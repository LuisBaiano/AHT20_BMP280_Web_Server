#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "web_server.h"
#include "config.h"
#include "wifi_credentials.h"

#define REQUEST_BUFFER_SIZE 2048
#define HTML_BUFFER_SIZE    8192

typedef struct TCP_SERVER_T {
    struct tcp_pcb *server_pcb;
    EstacaoMeteo_t *station_data;
} TCP_SERVER_T;

static TCP_SERVER_T *tcp_server_instance = NULL;

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void parse_user_request(char *request);


static void parse_user_request(char *request) {

    if (!request || strncmp(request, "POST /config", 12) != 0) {
        return;
    }
    
    char *body = strstr(request, "\r\n\r\n");
    if (body) {
        body += 4;
        sscanf(body,
               "temp_max=%f&temp_min=%f&temp_offset=%f&"
               "umid_max=%f&umid_min=%f&umid_offset=%f&"
               "press_offset=%f",
               &tcp_server_instance->station_data->config.limites.temp_max,
               &tcp_server_instance->station_data->config.limites.temp_min,
               &tcp_server_instance->station_data->config.offsets.temperatura,
               &tcp_server_instance->station_data->config.limites.umid_max,
               &tcp_server_instance->station_data->config.limites.umid_min,
               &tcp_server_instance->station_data->config.offsets.umidade,
               &tcp_server_instance->station_data->config.offsets.pressao
        );
        printf("CONFIG: Configuracoes salvas via formulario web.\n");
    }
}

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p || err != ERR_OK) {
        if (p) pbuf_free(p);
        tcp_close(tpcb);
        return err;
    }
    tcp_recved(tpcb, p->tot_len);

    char request_buffer[REQUEST_BUFFER_SIZE];
    pbuf_copy_partial(p, request_buffer, sizeof(request_buffer) - 1, 0);
    pbuf_free(p);
    
    parse_user_request(request_buffer);
    
    EstacaoMeteo_t *estacao = tcp_server_instance->station_data;
    
    char html[HTML_BUFFER_SIZE];
    snprintf(html, sizeof(html),
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
        "<!DOCTYPE html><html lang='pt-BR'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>Estacao Meteorologica</title>"
        "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
        "<style>"
            "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Helvetica,Arial,sans-serif;background-color:#f0f2f5;margin:0;padding:1rem;color:#333;}"
            ".container{max-width:900px;margin:auto;background:#fff;padding:1.5rem;border-radius:8px;box-shadow:0 4px 12px rgba(0,0,0,0.1);}"
            "h1,h2{text-align:center;color:#1a3a69;margin-bottom:1.5rem;}"
            "h1{margin-top:0;}"
            ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:1rem;margin-bottom:2rem;}"
            ".card{text-align:center;padding:1rem;background-color:#f8f9fa;border-radius:8px;border-left:5px solid #0d47a1; transition: transform 0.2s;}"
            ".card:hover{transform: translateY(-5px);}"
            ".card h4{margin-top:0;margin-bottom:0.5rem;font-size:1em;color:#495057;}"
            ".card p{margin:0;}"
            ".value{font-size:2rem;font-weight:700;color:#0d47a1;}"
            ".chart-grid{display:grid;grid-template-columns:1fr;gap:2rem;margin-top:1rem}"
            ".chart-container{height:250px;width:100%}"
            "form{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:1.5rem;margin-top:1rem;border-top:1px solid #eee;padding-top:1.5rem}"
            ".form-group{display:flex;flex-direction:column;gap:0.5rem;}"
            "label{font-weight:600;font-size:0.9em;}"
            "input{width:100%%;padding:10px;box-sizing:border-box;border:1px solid #ccc;border-radius:4px;font-size:1em;}"
            "button{grid-column:1/-1;padding:12px;background-color:#0d47a1;color:white;border:0;border-radius:5px;cursor:pointer;font-size:1.1rem;}"
            "button:hover{background-color:#1a3a69;}"
        "</style>"
        "</head><body><div class='container'>"
        "<div id='sensor-data' data-temp='%.2f' data-hum='%.1f' data-press='%.1f' style='display:none;'></div>"
        
        "<h1>Estacao Meteorologica</h1>"

        "<h2>Leituras Individuais</h2>"
        "<div class='grid'>"
            "<div class='card'><h4>Temp. AHT20</h4><p><span class='value'>%.1f</span>°C</p></div>"
            "<div class='card'><h4>Temp. BMP280</h4><p><span class='value'>%.1f</span>°C</p></div>"
            "<div class='card'><h4>Umidade</h4><p><span class='value'>%.1f</span>%%</p></div>"
            "<div class='card'><h4>Pressao</h4><p><span class='value'>%.0f</span> hPa</p></div>"
        "</div>"

        "<h2>Graficos de Leitura (Media / Calibrados)</h2>"
        "<div class='chart-grid'>"
            "<div class='chart-container'><canvas id='tempChart'></canvas></div>"
            "<div class='chart-container'><canvas id='humChart'></canvas></div>"
            "<div class='chart-container'><canvas id='pressChart'></canvas></div>"
        "</div>"
        
        "<h2>Configuracoes e Calibracao</h2>"
        "<form action='/config' method='post'>"
             "<div class='form-group'>"
                "<h4>Limites Temperatura (Média)</h4>"
                "<label>Max (°C):</label><input type='number' step='0.1' name='temp_max' value='%.1f'>"
                "<label>Min (°C):</label><input type='number' step='0.1' name='temp_min' value='%.1f'>"
                "<label>Offset (°C):</label><input type='number' step='0.1' name='temp_offset' value='%.1f'>"
            "</div>"
            "<div class='form-group'>"
                "<h4>Limites Umidade</h4>"
                "<label>Max (%%):</label><input type='number' step='0.1' name='umid_max' value='%.1f'>"
                "<label>Min (%%):</label><input type='number' step='0.1' name='umid_min' value='%.1f'>"
                "<label>Offset (%%):</label><input type='number' step='0.1' name='umid_offset' value='%.1f'>"
            "</div>"
            "<div class='form-group'>"
                "<h4>Calibracao Pressao</h4>"
                "<label>Offset (hPa):</label><input type='number' step='0.1' name='press_offset' value='%.1f'>"
            "</div>"
            "<button type='submit'>Salvar Alteracoes</button>"
        "</form>"
        "</div>"
        "<script>"
            "const MAX_POINTS=30;"
            "function createChart(id,label,data,color){const ctx=document.getElementById(id).getContext('2d');"
            "new Chart(ctx,{type:'line',data:{labels:data.labels,datasets:[{label:label,data:data.values,borderColor:color,backgroundColor:color+'33',fill:true,tension:0.2}]},"
            "options:{responsive:true,maintainAspectRatio:false,animation:false,plugins:{legend:{display:true}}}});}"
            "window.onload=()=>{const e=document.getElementById('sensor-data');"
            "const t={temp:parseFloat(e.dataset.temp),hum:parseFloat(e.dataset.hum),press:parseFloat(e.dataset.press)};"
            "let a=JSON.parse(sessionStorage.getItem('sensorHistory'))||{labels:[],temp_v:[],hum_v:[],press_v:[]};"
            "const n=new Date().toLocaleTimeString('pt-BR',{hour:'2-digit',minute:'2-digit',second:'2-digit'});"
            "a.labels.push(n);a.temp_v.push(t.temp);a.hum_v.push(t.hum);a.press_v.push(t.press);"
            "while(a.labels.length>MAX_POINTS){a.labels.shift();a.temp_v.shift();a.hum_v.shift();a.press_v.shift();}"
            "sessionStorage.setItem('sensorHistory',JSON.stringify(a));"
            "createChart('tempChart','Temperatura Media (°C)',{labels:a.labels,values:a.temp_v},'rgb(255, 99, 132)');"
            "createChart('humChart','Umidade (%%)',{labels:a.labels,values:a.hum_v},'rgb(54, 162, 235)');"
            "createChart('pressChart','Pressao (hPa)',{labels:a.labels,values:a.press_v},'rgb(75, 192, 192)');"
            "setTimeout(()=>{window.location.reload();},5000);};"
        "</script></body></html>",
        

        estacao->valores_calibrados.temperatura, 
        estacao->valores_calibrados.umidade, 
        estacao->valores_calibrados.pressao,
        

        estacao->valores_lidos.temp_aht20,
        estacao->valores_lidos.temp_bmp280,
        estacao->valores_calibrados.umidade,
        estacao->valores_calibrados.pressao,
        
        estacao->config.limites.temp_max, estacao->config.limites.temp_min, estacao->config.offsets.temperatura,
        estacao->config.limites.umid_max, estacao->config.limites.umid_min, estacao->config.offsets.umidade,
        estacao->config.offsets.pressao
    );

    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);
    tcp_close(tpcb);

    return ERR_OK;
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    if (err != ERR_OK || !newpcb) { return ERR_VAL; }
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

bool web_server_init(EstacaoMeteo_t *estacao) {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_BRAZIL)) return false;
    
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("ERRO: Falha ao conectar ao Wi-Fi.\n");
        estacao->status.wifi_conectado = false;
        return false;
    }
    
    estacao->status.wifi_conectado = true;
    strncpy(estacao->status.ip_address, ip4addr_ntoa(netif_ip4_addr(netif_default)), 20);
    printf("WIFI Conectado com sucesso! IP: %s\n", estacao->status.ip_address);
    
    tcp_server_instance = calloc(1, sizeof(TCP_SERVER_T));
    if (!tcp_server_instance) {
        printf("ERRO: Falha ao alocar memoria para o servidor TCP.\n");
        return false;
    }
    tcp_server_instance->station_data = estacao;
    
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (!pcb) {
        printf("ERRO: tcp_new falhou.\n");
        return false;
    }
    if (tcp_bind(pcb, NULL, 80) != ERR_OK) {
        printf("ERRO: tcp_bind falhou.\n");
        tcp_close(pcb);
        return false;
    }
    
    tcp_server_instance->server_pcb = tcp_listen(pcb);
    if (!tcp_server_instance->server_pcb) {
        printf("ERRO: tcp_listen falhou.\n");
        tcp_close(pcb);
        return false;
    }
    
    tcp_accept(tcp_server_instance->server_pcb, tcp_server_accept);
    
    printf("Servidor TCP aguardando conexoes na porta 80.\n");
    return true;
}

void web_server_deinit() {
    if (tcp_server_instance) {
        if (tcp_server_instance->server_pcb) {
            tcp_close(tcp_server_instance->server_pcb);
        }
        free(tcp_server_instance);
        tcp_server_instance = NULL;
    }
    cyw43_arch_deinit();
}
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <string.h>
#include "include/config.h"
#include "include/station_data.h"
#include "include/web_server.h"
#include "include/display.h"
#include "include/rgb_led.h"
#include "include/led_matrix.h"
#include "include/buzzer.h"
#include "include/buttons.h"
#include "include/external/aht20.h"
#include "include/external/bmp280.h"

// Crie um arquivo de credencias de wifi e defina WIFI_SSID e WIFI_PASSWORD
#include "include/wifi_credentials.h"


EstacaoMeteo_t estacao;
static ssd1306_t ssd_global;

void init_station_data();
void update_sensor_readings();
void check_and_trigger_alerts();

int main() {
    stdio_init_all();
    sleep_ms(2000); // Aguarda a serial USB estabilizar
    printf("Sistema da Estacao Meteorologica Iniciado.\n");

    // 1. Inicializa a estrutura de dados e periféricos internos
    init_station_data();
    rgb_led_init();
    led_matrix_init();
    buzzer_init();
    buttons_init();
    rgb_led_set_color(LED_COLOR_BLUE); // Sinaliza estado de inicialização

    // 2. Inicializa os barramentos I2C
    i2c_init(I2C0_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);

    i2c_init(I2C1_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SDA_PIN);
    gpio_pull_up(I2C1_SCL_PIN);

    display_init(&ssd_global);
    
    aht20_init();
    if (!detect_bmp280_address()) {
        rgb_led_set_color(LED_COLOR_RED);
        display_message(&ssd_global, "ERRO FATAL", "BMP280 Nao Encontrado");
        printf("ERRO FATAL: Sensor BMP280 nao detectado. Travando.\n");
        while (true);
    }
    bmp280_init();

    display_startup_screen(&ssd_global);
    display_message(&ssd_global, "Sistema OK", "Iniciando Rede...");
    
    if (web_server_init(&estacao)) {
        rgb_led_set_color(LED_COLOR_GREEN);
        display_message(&ssd_global, estacao.status.ip_address, "Servidor Online");
    } else {
        rgb_led_set_color(LED_COLOR_RED);
        display_message(&ssd_global, "ERRO FATAL", "Falha na Rede");
        printf("ERRO FATAL: Falha ao iniciar Web Server ou conectar ao Wi-Fi. Travando.\n");
        while (true); // Trava o programa
    }
    
    printf("Inicializacao completa. Entrando no loop principal...\n");
    uint32_t last_sensor_read = 0;
    uint32_t last_display_update = 0;

    while (true) {
        cyw43_arch_poll();
        
        uint32_t now = to_us_since_boot(get_absolute_time());
        
        if (now - last_sensor_read >= SENSOR_READ_INTERVAL_MS * 1000) {
            update_sensor_readings();
            check_and_trigger_alerts();
            last_sensor_read = now;
        }

        if (now - last_display_update >= DISPLAY_UPDATE_INTERVAL_MS * 1000) {
            display_update(&ssd_global, &estacao);
            last_display_update = now;
        }

        handle_button_presses(&estacao); 

        sleep_ms(10);
    }
    return 0;
}


void init_station_data() {
    memset(&estacao, 0, sizeof(EstacaoMeteo_t));

    // Configurações padrão de limites
    estacao.config.limites.temp_max = 30.0f;
    estacao.config.limites.temp_min = 15.0f;
    estacao.config.limites.umid_max = 70.0f;
    estacao.config.limites.umid_min = 30.0f;

    // Configurações padrão de calibração
    estacao.config.offsets.temperatura = 0.0f;
    estacao.config.offsets.umidade = 0.0f;
    estacao.config.offsets.pressao = 0.0f;
    
    // Estado inicial
    strcpy(estacao.status.ip_address, "N/A");
    estacao.status.wifi_conectado = false;
    estacao.status.em_alerta_temp = false;
    estacao.status.em_alerta_umid = false;
    estacao.status.tela_oled = 0;
}

void update_sensor_readings() {
    float temp_aht = 0.0f, hum_aht = 0.0f, temp_bmp = 0.0f, press_pa = 0.0f;

    if (aht20_read_data(&temp_aht, &hum_aht)) {
        estacao.valores_lidos.temp_aht20 = temp_aht;
        estacao.valores_lidos.umidade = hum_aht;
    }

    if (bmp280_read_data(&temp_bmp, &press_pa)) {
        estacao.valores_lidos.temp_bmp280 = temp_bmp;
        estacao.valores_lidos.pressao = press_pa / 100.0f;
    }

    estacao.valores_lidos.temperatura = estacao.valores_lidos.temp_aht20;

    estacao.valores_calibrados.temperatura = estacao.valores_lidos.temperatura + estacao.config.offsets.temperatura;
    estacao.valores_calibrados.umidade     = estacao.valores_lidos.umidade + estacao.config.offsets.umidade;
    estacao.valores_calibrados.pressao     = estacao.valores_lidos.pressao + estacao.config.offsets.pressao;
}

void check_and_trigger_alerts() {
    estacao.status.em_alerta_temp = (estacao.valores_calibrados.temperatura > estacao.config.limites.temp_max ||
                                     estacao.valores_calibrados.temperatura < estacao.config.limites.temp_min);
    
    estacao.status.em_alerta_umid = (estacao.valores_calibrados.umidade > estacao.config.limites.umid_max ||
                                     estacao.valores_calibrados.umidade < estacao.config.limites.umid_min);
                                     

                                     
    if (estacao.status.em_alerta_temp || estacao.status.em_alerta_umid) {
        rgb_led_set_color(LED_COLOR_RED);
        led_matrix_show_alert();
        buzzer_beep(BUZZER_ALERT_FREQ_HZ, BUZZER_ALERT_DURATION_MS);
    } else if (estacao.status.wifi_conectado) {
        rgb_led_set_color(LED_COLOR_GREEN); 
        led_matrix_clear(); 
    }
}
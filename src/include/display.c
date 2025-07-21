#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "display.h"
#include "config.h"

void ssd1306_clear_screen(ssd1306_t *ssd) {
    ssd1306_fill(ssd, false);
}

void ssd1306_update_screen(ssd1306_t *ssd) {
    ssd1306_send_data(ssd); 
}

// A função display_init, conforme sua especificação
void display_init(ssd1306_t *ssd) {
    i2c_init(I2C1_PORT, 400 * 1000); 
    gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SDA_PIN);
    gpio_pull_up(I2C1_SCL_PIN);

    ssd1306_init(ssd, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, DISPLAY_I2C_ADDR, I2C1_PORT);
    ssd1306_config(ssd);
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
    printf("Display inicializado.\n");
}

void display_startup_screen(ssd1306_t *ssd) {
    ssd1306_fill(ssd, false);
    uint8_t center_x_approx = ssd->width / 2;
    uint8_t start_y = 8;
    uint8_t line_height = 10;

    // Define as strings a serem exibidas
    const char *line1 = "ESTACAO";
    const char *line2 = "METEOROLOGICA";
    const char *line3 = "ATH + BMP";
    ssd1306_draw_string(ssd, line1, center_x_approx - (strlen(line1)*8)/2, start_y);
    ssd1306_draw_string(ssd, line2, center_x_approx - (strlen(line2)*8)/2, start_y + line_height);
    ssd1306_draw_string(ssd, line3, center_x_approx - (strlen(line3)*8)/2, start_y + 2*line_height);
    ssd1306_send_data(ssd);
    sleep_ms(2500);

    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}

void draw_main_screen_grid(ssd1306_t *ssd, EstacaoMeteo_t *estacao) {
    char buffer[32];

    ssd1306_draw_string(ssd, estacao->status.wifi_conectado ? estacao->status.ip_address : "Conectando...", 2, 2);
    ssd1306_hline(ssd, 0, 127, 12, true);


    ssd1306_vline(ssd, 63, 13, 63, true);   // Linha vertical central
    ssd1306_hline(ssd, 0, 127, 38, true);   // Linha horizontal central


    // Quadrante Superior Esquerdo: Temperatura AHT20
    ssd1306_draw_string(ssd, "T. AHT", 4, 16);
    snprintf(buffer, sizeof(buffer), "%.1f C", estacao->valores_lidos.temp_aht20);
    ssd1306_draw_string(ssd, buffer, 4, 26);
    if (estacao->status.em_alerta_temp) ssd1306_draw_string(ssd, "!", 52, 16);

    // Quadrante Superior Direito: Temperatura BMP280
    ssd1306_draw_string(ssd, "T. BMP", 68, 16);
    snprintf(buffer, sizeof(buffer), "%.1f C", estacao->valores_lidos.temp_bmp280);
    ssd1306_draw_string(ssd, buffer, 68, 26);

    // Quadrante Inferior Esquerdo: Umidade Calibrada
    ssd1306_draw_string(ssd, "Umidade", 4, 42);
    snprintf(buffer, sizeof(buffer), "%.1f %%", estacao->valores_calibrados.umidade);
    ssd1306_draw_string(ssd, buffer, 4, 52);
    if (estacao->status.em_alerta_umid) ssd1306_draw_string(ssd, "!", 52, 42);

    // Quadrante Inferior Direito: Pressão Calibrada
    ssd1306_draw_string(ssd, "Pressao", 68, 42);
    snprintf(buffer, sizeof(buffer), "%.0fhPa", estacao->valores_calibrados.pressao);
    ssd1306_draw_string(ssd, buffer, 68, 52);
}

void draw_config_screen(ssd1306_t *ssd, EstacaoMeteo_t *estacao) {
    char buffer[24];

    ssd1306_draw_string(ssd, "Offsets", 16, 2);
    ssd1306_hline(ssd, 0, 127, 12, true);

    snprintf(buffer, sizeof(buffer), "Temp: %+5.1f C", estacao->config.offsets.temperatura);
    ssd1306_draw_string(ssd, buffer, 4, 18);

    snprintf(buffer, sizeof(buffer), "Umid: %+5.1f %%", estacao->config.offsets.umidade);
    ssd1306_draw_string(ssd, buffer, 4, 32);

    snprintf(buffer, sizeof(buffer), "Pres: %+5.1fhPa", estacao->config.offsets.pressao);
    ssd1306_draw_string(ssd, buffer, 4, 46);
}

void display_update(ssd1306_t *ssd, EstacaoMeteo_t *estacao) {
    // Limpa o buffer de memória
    ssd1306_fill(ssd, false);

    // Desenha a tela correta no buffer
    switch(estacao->status.tela_oled) {
        case 0: // Tela Principal
            draw_main_screen_grid(ssd, estacao); // <<< CHAMA A NOVA FUNÇÃO COM GRADE
            break;
        case 1: // Tela de Configuração
            draw_config_screen(ssd, estacao);
            break;
        default:
            draw_main_screen_grid(ssd, estacao);
    }
    
    // Envia o buffer para a tela física
    ssd1306_send_data(ssd);
}

void display_message(ssd1306_t *ssd, const char *line1, const char *line2) {
    ssd1306_fill(ssd, false);
    ssd1306_rect(ssd, 0, 0, 127, 63, false, true);
    ssd1306_draw_string(ssd, line1, 2, 16);
    ssd1306_draw_string(ssd, line2, 2, 32);
    ssd1306_send_data(ssd);
}
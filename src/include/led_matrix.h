#ifndef LED_MATRIX_H
#define LED_MATRIX_H

// Inicializa o PIO para controlar a matriz de LEDs.
void led_matrix_init();

// Limpa todos os LEDs da matriz (apaga).
void led_matrix_clear();

// Exibe um ícone de alerta (um 'X' vermelho).
void led_matrix_show_alert();

// Exibe um ícone de "tudo OK" (um 'V' verde).
void led_matrix_show_ok();

#endif // LED_MATRIX_H
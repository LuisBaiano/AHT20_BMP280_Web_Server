#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

#include "buttons.h"
#include "config.h"
#include "buzzer.h"
#include "debouncer.h"

// Variáveis de estado para comunicação entre a ISR e o loop principal
static volatile bool btn_a_pressed = false;
static volatile bool btn_b_pressed = false;

// Timestamps para debouncing independente
static uint32_t last_press_time_A = 0;
static uint32_t last_press_time_B = 0;


// Callback de interrupção para os pinos GPIO dos botões
static void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_A_PIN) {
        if (check_debounce(&last_press_time_A, BUTTON_DEBOUNCE_US)) {
            btn_a_pressed = true;
        }
    }
    else if (gpio == BUTTON_B_PIN) {
        if (check_debounce(&last_press_time_B, BUTTON_DEBOUNCE_US)) {
            btn_b_pressed = true;
        }
    }
}


// Função de inicialização
void buttons_init() {
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}


// Função para processar os cliques no loop principal
void handle_button_presses(EstacaoMeteo_t *estacao) {
    if (btn_a_pressed) {
        btn_a_pressed = false; // Consome o evento
        
        // Alterna entre a tela de dados e a tela de configurações
        estacao->status.tela_oled = (estacao->status.tela_oled + 1) % 2; 
    }
    
    if (btn_b_pressed) {
        btn_b_pressed = false; // Consome o evento
    }
}
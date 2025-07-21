#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include <math.h>
#include <string.h>

#include "led_matrix.h"
#include "config.h"
#include "led_matrix.pio.h" 

static PIO pio_instance = pio0;
static uint pio_sm = 0;
static uint32_t pixel_buffer[MATRIX_SIZE]; 

typedef struct { 
    float r; 
    float g; 
    float b; 
} ws2812b_color_t;

static const ws2812b_color_t COLOR_BLACK = { 0.0f, 0.0f, 0.0f };
static const ws2812b_color_t COLOR_RED   = { 1.0f, 0.0f, 0.0f }; // Vermelho puro
static const ws2812b_color_t COLOR_GREEN = { 0.0f, 1.0f, 0.0f }; // Verde puro

static const uint8_t Leds_Matrix_postion[MATRIX_DIM][MATRIX_DIM] = {
    { 24, 23, 22, 21, 20 }, 
    { 15, 16, 17, 18, 19 }, 
    { 14, 13, 12, 11, 10 }, 
    {  5,  6,  7,  8,  9 }, 
    {  4,  3,  2,  1,  0 }  
};


static inline int map_pixel(int lin, int col) {
    if (lin >= 1 && lin <= MATRIX_DIM && col >= 1 && col <= MATRIX_DIM) {
        return Leds_Matrix_postion[lin - 1][col - 1];
    }
    return -1;
}

// Converte a cor float (R,G,B) + brilho para o formato de 32 bits do PIO (G-R-B)
static inline uint32_t color_to_pio_format(ws2812b_color_t color, float brightness) {
    float r = fmaxf(0.0f, fminf(1.0f, color.r * brightness));
    float g = fmaxf(0.0f, fminf(1.0f, color.g * brightness));
    float b = fmaxf(0.0f, fminf(1.0f, color.b * brightness));
    unsigned char R_val = (unsigned char)(r * 255.0f);
    unsigned char G_val = (unsigned char)(g * 255.0f);
    unsigned char B_val = (unsigned char)(b * 255.0f);
    return ((uint32_t)(G_val) << 24) | ((uint32_t)(R_val) << 16) | ((uint32_t)(B_val) << 8);
}


// Envia o conteúdo do buffer de memória para a matriz de LEDs via PIO
static void update_matrix() {
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        pio_sm_put_blocking(pio_instance, pio_sm, pixel_buffer[i]);
    }
}

// Coloca um pixel no buffer de memória na posição física correta
static void set_pixel_at(int lin, int col, uint32_t color) {
    int logical_index = map_pixel(lin, col);
    if (logical_index != -1) {
        pixel_buffer[logical_index] = color;
    }
}

// Limpa o buffer de memória (não atualiza o display)
static void clear_buffer() {
    uint32_t black = color_to_pio_format(COLOR_BLACK, 1.0f);
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        pixel_buffer[i] = black;
    }
}


void led_matrix_init() {
    pio_sm = pio_claim_unused_sm(pio_instance, true);
    uint offset = pio_add_program(pio_instance, &led_matrix_program);
    led_matrix_program_init(pio_instance, pio_sm, offset, MATRIX_WS2812_PIN);
    led_matrix_clear();
}

void led_matrix_clear() {
    clear_buffer();
    update_matrix();
}

void led_matrix_show_alert() {
    clear_buffer();
    uint32_t red = color_to_pio_format(COLOR_RED, 0.2f);

    // Desenha um 'X' na matriz
    for(int i = 1; i <= 5; i++) {
        set_pixel_at(i, i, red);     // Diagonal principal
        set_pixel_at(i, 6 - i, red); // Diagonal secundária
    }
    
    update_matrix();
}

void led_matrix_show_ok() {
    clear_buffer();
    uint32_t green = color_to_pio_format(COLOR_GREEN, 0.2f);

    // Desenha um 'V' de check
    set_pixel_at(3, 1, green);
    set_pixel_at(4, 2, green);
    set_pixel_at(3, 3, green);
    set_pixel_at(2, 4, green);
    set_pixel_at(1, 5, green);
    
    update_matrix();
}
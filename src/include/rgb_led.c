#include "pico/stdlib.h"
#include "rgb_led.h"
#include "config.h"

void rgb_led_init() {
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
}

void rgb_led_set_color(LedColor color) {
    bool r=0, g=0, b=0;
    switch (color) {
        case LED_COLOR_RED:     r = 1; break;
        case LED_COLOR_GREEN:   g = 1; break;
        case LED_COLOR_BLUE:    b = 1; break;
        case LED_COLOR_YELLOW:  r = 1; g = 1; break;
        case LED_COLOR_CYAN:    g = 1; b = 1; break;
        case LED_COLOR_MAGENTA: r = 1; b = 1; break;
        case LED_COLOR_OFF:     break;
    }
    gpio_put(LED_RED_PIN, r);
    gpio_put(LED_GREEN_PIN, g);
    gpio_put(LED_BLUE_PIN, b);
}
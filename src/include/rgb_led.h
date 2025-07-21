#ifndef RGB_LED_H
#define RGB_LED_H

typedef enum {
    LED_COLOR_OFF,
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_YELLOW,
    LED_COLOR_CYAN,
    LED_COLOR_MAGENTA
} LedColor;

void rgb_led_init();
void rgb_led_set_color(LedColor color);

#endif // RGB_LED_H
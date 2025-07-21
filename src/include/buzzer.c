#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>

#include "buzzer.h"
#include "config.h"

void buzzer_init() {
    gpio_set_function(BUZZER_PWM_PIN, GPIO_FUNC_PWM);
    pwm_set_enabled(pwm_gpio_to_slice_num(BUZZER_PWM_PIN), false);
    printf("Buzzer inicializado.\n");
}

void buzzer_beep(uint16_t freq_hz, uint16_t duration_ms) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PWM_PIN);
    
    // A função clock_get_hz está definida em "hardware/clocks.h"
    float divider = (float)clock_get_hz(clk_sys) / (freq_hz * 1000.0f); 
    if (divider < 1.f) divider = 1.f;
    uint16_t wrap = 1000-1;
    
    pwm_set_clkdiv(slice_num, divider);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(BUZZER_PWM_PIN), wrap / 2);
    
    pwm_set_enabled(slice_num, true);
    sleep_ms(duration_ms);
    pwm_set_enabled(slice_num, false);
}
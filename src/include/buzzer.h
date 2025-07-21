#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void buzzer_init();

void buzzer_beep(uint16_t freq_hz, uint16_t duration_ms);

#endif // BUZZER_H
#ifndef DISPLAY_H
#define DISPLAY_H

#include "station_data.h"
#include "lib/ssd1306/ssd1306.h"
#include "lib/ssd1306/font.h"

void display_init(ssd1306_t *ssd);

void display_startup_screen(ssd1306_t *ssd);

void display_message(ssd1306_t *ssd, const char *line1, const char *line2);

void display_update(ssd1306_t *ssd, EstacaoMeteo_t *estacao);

#endif // DISPLAY_H
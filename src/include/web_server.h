#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <stdbool.h>
#include "station_data.h"


bool web_server_init(EstacaoMeteo_t *estacao);

void web_server_deinit();

#endif // WEB_SERVER_H
#ifndef STATION_DATA_H
#define STATION_DATA_H

#include <stdbool.h>
#include <stdint.h>

// Sub-estrutura para os valores de calibração (offsets)
typedef struct {
    float temperatura;
    float umidade;
    float pressao;
} CalibracaoOffsets_t;

// Sub-estrutura para os limites de alerta definidos pelo usuário
typedef struct {
    float temp_max;
    float temp_min;
    float umid_max;
    float umid_min;
} LimitesAlerta_t;

// Estrutura Principal que agrega todos os dados da estação
typedef struct {
    // Valores lidos DIRETAMENTE dos sensores
    struct {
        float temperatura;
        float temp_aht20; 
        float temp_bmp280;
        float umidade;
        float pressao; // Em hPa
    } valores_lidos;

    // Valores finais após aplicar a calibração
    struct {
        float temperatura;
        float umidade;
        float pressao;
    } valores_calibrados;

    // Configurações do sistema
    struct {
        LimitesAlerta_t    limites;
        CalibracaoOffsets_t offsets;
    } config;

    // Estado do sistema
    struct {
        bool wifi_conectado;
        char ip_address[20];
        bool em_alerta_temp;
        bool em_alerta_umid;
        uint8_t tela_oled; // 0 para principal, 1 para config, etc.
    } status;

} EstacaoMeteo_t;

#endif
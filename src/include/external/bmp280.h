#ifndef BMP280_H
#define BMP280_H

#include <stdbool.h>
#include <stdint.h>

// Declara que esta variável global existe e será definida em outro lugar (no .c)
// Isso permite que main.c veja o endereço detectado, se necessário.
extern uint8_t bmp280_address;

/**
 * @brief Varre os endereços I2C 0x76 e 0x77 para encontrar o BMP280.
 *        Se encontrar, atualiza a variável global `bmp280_address`.
 * @return true se o sensor foi encontrado.
 */
bool detect_bmp280_address();

/**
 * @brief Inicializa o sensor BMP280 lendo os dados de calibração.
 *        Deve ser chamada DEPOIS de detect_bmp280_address().
 * @return true se a inicialização foi bem-sucedida.
 */
bool bmp280_init();

/**
 * @brief Lê os valores de temperatura e pressão compensados.
 * 
 * @param temperature Ponteiro para a variável que receberá a temperatura em °C.
 * @param pressure Ponteiro para a variável que receberá a pressão em Pascals (Pa).
 * @return true se a leitura foi bem-sucedida.
 */
bool bmp280_read_data(float *temperature, float *pressure);

#endif // BMP280_H
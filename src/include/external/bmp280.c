#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <string.h>
#include "bmp280.h"
#include "include/config.h"

// Estrutura interna para guardar os dados de calibração do chip
static struct bmp280_calib_params {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} calib;

// Variável de compensação fina de temperatura, usada pelos cálculos
static int32_t t_fine;
// Variável global para armazenar o endereço I2C detectado
uint8_t bmp280_address = 0xFF;


// --- Funções Privadas (static) ---

static float compensate_temp(int32_t adc_T) {
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) * ((int32_t)calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) * ((int32_t)calib.dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

static float compensate_pressure(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) + ((var1 * (int64_t)calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_P1) >> 33;
    if (var1 == 0) return 0;
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);
    return (float)p;
}

static void read_calibration_data() {
    uint8_t reg_addr = 0x88;
    uint8_t buffer[24];

    i2c_write_blocking(I2C0_PORT, bmp280_address, &reg_addr, 1, true);
    i2c_read_blocking(I2C0_PORT, bmp280_address, buffer, 24, false);

    calib.dig_T1 = (buffer[1] << 8) | buffer[0];
    calib.dig_T2 = (buffer[3] << 8) | buffer[2];
    calib.dig_T3 = (buffer[5] << 8) | buffer[4];
    calib.dig_P1 = (buffer[7] << 8) | buffer[6];
    calib.dig_P2 = (buffer[9] << 8) | buffer[8];
    calib.dig_P3 = (buffer[11] << 8) | buffer[10];
    calib.dig_P4 = (buffer[13] << 8) | buffer[12];
    calib.dig_P5 = (buffer[15] << 8) | buffer[14];
    calib.dig_P6 = (buffer[17] << 8) | buffer[16];
    calib.dig_P7 = (buffer[19] << 8) | buffer[18];
    calib.dig_P8 = (buffer[21] << 8) | buffer[20];
    calib.dig_P9 = (buffer[23] << 8) | buffer[22];
}

// --- Funções Públicas ---

bool detect_bmp280_address() {
    uint8_t reg_addr = 0xD0; // Registrador do Chip ID
    uint8_t chip_id;
    for (uint8_t addr = 0x76; addr <= 0x77; addr++) {
        i2c_write_blocking(I2C0_PORT, addr, &reg_addr, 1, true);
        int result = i2c_read_blocking(I2C0_PORT, addr, &chip_id, 1, false);
        if (result == 1 && (chip_id == 0x58 || chip_id == 0x60)) {
            bmp280_address = addr;
            printf("BMP280 encontrado no endereco 0x%02X\n", addr);
            return true;
        }
    }
    printf("ERRO: BMP280 nao foi detectado.\n");
    return false;
}

bool bmp280_init() {
    if (bmp280_address == 0xFF) {
        printf("ERRO BMP280: O sensor precisa ser detectado antes de inicializar.\n");
        return false;
    }

    read_calibration_data();

    // Configura o sensor: Normal mode, oversampling x16 para Temp e Pressão
    uint8_t config_data[] = {0xF4, 0b10110111};
    i2c_write_blocking(I2C0_PORT, bmp280_address, config_data, 2, false);
    
    printf("BMP280 inicializado com sucesso.\n");
    return true;
}

bool bmp280_read_data(float *temperature, float *pressure) {
    if (bmp280_address == 0xFF) return false;

    uint8_t reg_addr = 0xF7; // Endereço inicial dos dados
    uint8_t buffer[6];

    i2c_write_blocking(I2C0_PORT, bmp280_address, &reg_addr, 1, true);
    int result = i2c_read_blocking(I2C0_PORT, bmp280_address, buffer, 6, false);
    if (result < 6) return false;

    int32_t adc_P = (buffer[0] << 12) | (buffer[1] << 4) | (buffer[2] >> 4);
    int32_t adc_T = (buffer[3] << 12) | (buffer[4] << 4) | (buffer[5] >> 4);
    
    // As funções de compensação retornam valores que precisam ser divididos
    *temperature = compensate_temp(adc_T) / 100.0f;     // Retorna temp em °C
    *pressure = compensate_pressure(adc_P) / 256.0f;   // Retorna pressão em Pa
    
    return true;
}
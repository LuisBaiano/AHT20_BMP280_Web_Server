#ifndef CONFIG_H
#define CONFIG_H


// --- Configuração da Rede Wi-Fi ---
#define WIFI_SSID     "CLARO_2G86C2FE"
#define WIFI_PASSWORD "DE86C2FE"

#define TCP_PORT 80
#define TCP_SERVER_POLL_TIME_S 5
#define HTTP_REQUEST_BUFFER_SIZE 4096


// --- CONFIGURAÇÃO I2C ---
#define I2C_BAUDRATE        (400 * 1000)

// --- Barramento I2C 1: Para o Display OLED ---
#define I2C1_PORT           i2c1
#define I2C1_SDA_PIN        14 
#define I2C1_SCL_PIN        15 
#define DISPLAY_I2C_ADDR    0x3C
#define DISPLAY_WIDTH       128
#define DISPLAY_HEIGHT      64

// --- Barramento I2C 0: Apenas para os Sensores ---
#define I2C0_PORT           i2c0
#define I2C0_SDA_PIN        0
#define I2C0_SCL_PIN        1
#define AHT20_I2C_ADDR      0x38

// --- Configuração dos Pinos ---
// LED RGB
#define LED_RED_PIN         13
#define LED_GREEN_PIN       11
#define LED_BLUE_PIN        12

// Matriz de LEDs
#define MATRIX_WS2812_PIN   7
#define MATRIX_SIZE         25
#define MATRIX_DIM          5

// Botões
#define BUTTON_A_PIN        5
#define BUTTON_B_PIN        6

// Buzzer
#define BUZZER_PWM_PIN      10 

// --- Configuração de Temporização ---
#define SENSOR_READ_INTERVAL_MS 2000
#define DISPLAY_UPDATE_INTERVAL_MS 1000
#define BUTTON_DEBOUNCE_US 200000

// --- Configurações dos Alertas ---
#define BUZZER_ALERT_FREQ_HZ 1500
#define BUZZER_ALERT_DURATION_MS 100

#endif // CONFIG_H
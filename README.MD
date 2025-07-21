# Estação Meteorológica com Interface Web para BitDogLab (Pico W)

Este projeto consiste em uma estação meteorológica completa desenvolvida para a plataforma BitDogLab, baseada no Raspberry Pi Pico W. O sistema monitora temperatura, umidade e pressão atmosférica, exibindo os dados localmente em um display OLED e servindo uma interface web responsiva com gráficos e formulários de configuração.

## Funcionalidades Principais

- **Monitoramento de Sensores:** Leitura contínua de dados de dois sensores I2C:
  - **AHT20:** Temperatura e Umidade Relativa.
  - **BMP280:** Pressão Barométrica e Temperatura.
- **Exibição Local:**
  - Um **Display OLED SSD1306 (128x64)** exibe os dados em tempo real em uma grade organizada, mostrando as leituras individuais dos sensores, o status da conexão Wi-Fi e os offsets de calibração em uma tela secundária.
- **Interface Web Robusta:**
  - Servidor web embarcado acessível via Wi-Fi.
  - **Interface responsiva** que se adapta a telas de desktop e dispositivos móveis.
  - **Gráficos históricos** para Temperatura (média), Umidade e Pressão, renderizados com Chart.js. O histórico é mantido no navegador usando `sessionStorage`.
  - Formulário para **configuração de limites de alerta e offsets de calibração**, que são enviados ao microcontrolador via requisição `POST`.
- **Sistema de Alertas:**
  - **Visual:** O **LED RGB** integrado indica o estado do sistema (inicializando, conectado, erro, alerta). A **matriz de LEDs 5x5** exibe um ícone de alerta ('X') quando os valores ultrapassam os limites.
  - **Sonoro:** Um **buzzer** emite bipes intermitentes em caso de alerta.
- **Interação do Usuário:**
  - **Botões Físicos:** O botão 'A' alterna entre a tela de dados e a tela de offsets no display OLED. A lógica utiliza **interrupções e debounce** de software para uma resposta precisa.
- **Estabilização de Dados:**
  - Implementação de um **filtro passa-baixa (Média Móvel Exponencial)** para suavizar as leituras dos sensores, evitando oscilações bruscas e apresentando dados mais estáveis.

## Hardware Necessário

- Placa BitDogLab (ou um Raspberry Pi Pico W com os seguintes periféricos)
- Sensor de Temperatura e Umidade **AHT20** (I2C)
- Sensor de Pressão e Temperatura **BMP280** (I2C)
- Display OLED **SSD1306 128x64** (I2C)
- LED RGB, Matriz de LEDs WS2812 5x5, Buzzer e Botões (integrados na BitDogLab)
- Cabo Micro-USB

## Estrutura do Projeto

O código-fonte está organizado de forma modular dentro do diretório `src/`:

```
src/
├── main.c # Lógica principal, inicialização e loop de tarefas
├── config.h # Configurações centrais (pinos, WiFi, I2C, etc.)
├── station_data.h # Definição da struct de dados principal
├── web_server.c/.h # Lógica do servidor web e geração do HTML
├── display.c/.h # Lógica de alto nível para as telas do OLED
├── buttons.c/.h # Gerenciamento dos botões com interrupção
├── debouncer.c/.h # Módulo de debounce reutilizável
├── buzzer.c/.h # Driver do buzzer
├── rgb_led.c/.h # Driver do LED RGB
├── led_matrix.c/.h # Driver da matriz de LEDs com PIO
├── led_matrix.pio # Código PIO para a matriz WS2812
│
├── external/
│ ├── aht20.c/.h # Driver para o sensor AHT20
│ └── bmp280.c/.h # Driver para o sensor BMP280
│
└── lib/
└── ssd1306/ # (Local para a biblioteca de baixo nível do display)
```

## Configuração do Ambiente

1.  **Instale o Pico C/C++ SDK** e a toolchain ARM, seguindo o guia oficial da Raspberry Pi.
2.  Certifique-se de que `CMake`, `arm-none-eabi-gcc` e `Ninja` estão instalados. Em sistemas Debian/Ubuntu:
    ```bash
    sudo apt update
    sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential ninja-build
    ```
3.  Configure a variável de ambiente `PICO_SDK_PATH` ou use um `pico_sdk_import.cmake`.

## Como Compilar e Executar

1.  **Configurar o Wi-Fi:**
    - Abra o arquivo `src/config.h`.
    - Altere as macros `WIFI_SSID` e `WIFI_PASSWORD` com as credenciais da sua rede Wi-Fi.

2.  **Compilar o Projeto:**
    - Navegue até o diretório raiz do projeto no terminal.
    - Crie o diretório de build e gere os arquivos de compilação com o CMake:
      ```bash
      cmake -B build -S . -G Ninja
      ```
    - Execute a compilação com o Ninja:
      ```bash
      cmake --build build -j
      ```

3.  **Carregar o Firmware:**
    - Pressione e segure o botão `BOOTSEL` da sua Pico W e conecte-a ao computador via USB.
    - A placa será montada como um dispositivo de armazenamento em massa.
    - Arraste e solte o arquivo `estacao_meteorologica_web.uf2` (que está dentro da pasta `build/`) para a placa.
    - A placa irá reiniciar automaticamente e começar a executar o programa.

## Como Usar o Sistema

1.  **Inicialização:** Ao ligar, o LED RGB ficará azul. Após a conexão Wi-Fi, ele ficará verde. O display OLED mostrará uma tela de inicialização e, em seguida, o endereço IP da placa.
2.  **Display OLED:**
    - A tela principal exibe uma grade com as temperaturas lidas de cada sensor, a umidade e a pressão calibradas, e o status da conexão.
    - Pressione o **Botão A** para alternar para a tela de "Offsets Atuais", que mostra os valores de calibração que foram configurados via web.
3.  **Interface Web:**
    - Digite o endereço IP mostrado no display em um navegador.
    - A página exibirá as leituras individuais, gráficos históricos e o formulário de configuração.
    - Preencha os campos de limites e offsets e clique em "Salvar Alterações" para atualizar as configurações na placa.
4.  **Alertas:** Se qualquer valor de temperatura ou umidade (após calibração) sair dos limites definidos, o LED RGB ficará vermelho, a matriz de LEDs mostrará um 'X' e o buzzer soará intermitentemente. O display OLED também mostrará um '!' ao lado do valor fora do limite.

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

# Puxe o PICO_SDK_PATH do seu ambiente
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
if (NOT PICO_SDK_PATH)
    message(FATAL_ERROR "PICO_SDK_PATH não está definido. Por favor, configure o caminho para o SDK do Pico.")
endif ()

include(${PICO_SDK_PATH}/pico_sdk_import.cmake)
project(estacao_meteorologica_web C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${PROJECT_NAME}
    main.c
    include/external/bmp280.c
    include/external/aht20.c
    include/web_server.c
    include/display.c
    include/rgb_led.c
    include/led_matrix.c
    include/buzzer.c
    include/buttons.c
    include/debouncer.c
    include/lib/ssd1306/ssd1306.c
)

# Gere o arquivo C a partir do arquivo .pio
pico_generate_pio_header(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/include/pio/led_matrix.pio)

target_compile_definitions(${PROJECT_NAME} PUBLIC 
    PICO_CYW43_ARCH_LWIP_OPTS_FILE=lwipopts.h
)

target_link_libraries(${PROJECT_NAME} 
    pico_stdlib 
    hardware_i2c 
    hardware_adc 
    hardware_pio 
    hardware_pwm 
    pico_cyw43_arch_lwip_poll
    )

target_include_directories(estacao_meteorologica_web PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # Para main.c, web_server.c, etc. se estiverem na raiz de src/
    ${CMAKE_CURRENT_SOURCE_DIR}/libs # Para libs/ssd1306.h
    ${CMAKE_CURRENT_SOURCE_DIR}/external # Se tiver cabeçalhos lá
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/lwip/src/include/arch
    ${PICO_SDK_PATH}/lib/lwip/src/include/lwip
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/httpd.c
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/fs.c
)


# Ativa a saída printf via USB ou UART
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})
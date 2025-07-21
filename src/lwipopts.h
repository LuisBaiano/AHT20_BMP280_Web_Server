// Salve como: src/lwipopts.h (VERSÃO ROBUSTA PARA AJAX)

#ifndef LWIPOPTS_H
#define LWIPOPTS_H

// --- Configurações Principais ---
#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_IPV4                   1

// --- Configuração da Alocação de Memória ---
#define MEM_LIBC_MALLOC             0
#define MEMP_MEM_MALLOC             0
#define MEM_ALIGNMENT               4
// Aumentamos um pouco o heap geral.
#define MEM_SIZE                    (40 * 1024) 

// --- Módulos da Rede a Habilitar ---
#define LWIP_TCP                    1
#define LWIP_DHCP                   1

// --- Configurações de Buffers de Pacotes (PBUF) ---
// <<< AUMENTO SIGNIFICATIVO. Permite lidar com mais rajadas de pacotes.
#define PBUF_POOL_SIZE              32 
#define MEMP_NUM_PBUF               32 

// --- Configurações Específicas de TCP ---
#define TCP_MSS                     1460
// O buffer de envio grande continua sendo importante.
#define TCP_SND_BUF                 (4 * TCP_MSS) 
#define TCP_WND                     (2 * TCP_MSS)

// <<< AUMENTO SIGNIFICATIVO. Mais conexões e segmentos em trânsito.
// Permite que o servidor aceite mais conexões simultâneas (uma para HTML, uma para JSON, etc.)
#define MEMP_NUM_TCP_PCB            10   
// Permite enfileirar mais segmentos TCP, crucial para performance sob carga
#define MEMP_NUM_TCP_SEG            32   

#define SO_REUSE                    1

#endif /* LWIPOPTS_H */
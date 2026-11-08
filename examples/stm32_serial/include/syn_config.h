#ifndef SYN_CONFIG_H
#define SYN_CONFIG_H

/* Drivers */
#define SYN_USE_GPIO           1
#define SYN_USE_UART           1
#define SYN_USE_ADC            0

/* UART tuning */
#define SYN_UART_TX_BUF_SIZE   64
#define SYN_UART_RX_BUF_SIZE   64
#define SYN_UART_MAX_INSTANCES 1

/* Multitasking */
#define SYN_USE_PT                     1
#define SYN_USE_SCHED                  1
#define SYN_SCHED_PRIO_LEVELS          4
#define SYN_SCHED_MAX_WAIT_CRITICAL_MS 1U
#define SYN_SCHED_MAX_WAIT_HIGH_MS     5U
#define SYN_SCHED_MAX_WAIT_NORMAL_MS   15U
#define SYN_SCHED_MAX_WAIT_LOW_MS      50U
#define SYN_USE_TIMER                  0
#define SYN_USE_MUTEX                  0

/* Services */
#define SYN_USE_LOG            1
#define SYN_USE_CLI            1

/* Logging tuning */
#define SYN_LOG_LEVEL          1   /* DEBUG */
#define SYN_LOG_BUF_SIZE       64
#define SYN_LOG_TIMESTAMP      1
#define SYN_LOG_COLOR          0

/* CLI tuning */
#define SYN_CLI_LINE_BUF_SIZE  64
#define SYN_CLI_MAX_ARGS       6
#define SYN_CLI_HISTORY_DEPTH  0
#define SYN_CLI_CMD_ERRORS     0

/* Input / Output */
#define SYN_USE_LED            1

/* DSP / Filters */
#define SYN_USE_FSM            1

/* Utilities */
#define SYN_USE_FMT            1
#define SYN_CRC_USE_TABLE      1

#endif /* SYN_CONFIG_H */

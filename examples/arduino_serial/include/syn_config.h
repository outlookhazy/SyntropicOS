#ifndef SYN_CONFIG_H
#define SYN_CONFIG_H

/* Drivers */
#define SYN_USE_GPIO           1
#define SYN_USE_UART           1
#define SYN_USE_ADC            1
#define SYN_USE_EXTI           0

/* UART tuning */
#define SYN_UART_TX_BUF_SIZE   16  /* Kept small to conserve RAM on Uno */
#define SYN_UART_RX_BUF_SIZE   16
#define SYN_UART_MAX_INSTANCES 1

/* Multitasking */
#define SYN_USE_PT             1
#define SYN_USE_SCHED          1
#define SYN_USE_TIMER          0
#define SYN_USE_EVENT          0
#define SYN_USE_WATCHDOG       0
#define SYN_USE_SEQUENCER      0
#define SYN_USE_WORKQUEUE      0

/* Services */
#define SYN_USE_LOG            1
#define SYN_USE_CLI            1

/* Logging tuning */
#define SYN_LOG_LEVEL          1   /* DEBUG */
#define SYN_LOG_BUF_SIZE       32  /* Conserves RAM */
#define SYN_LOG_TIMESTAMP      1
#define SYN_LOG_COLOR          0

/* CLI tuning */
#define SYN_CLI_LINE_BUF_SIZE  64  /* Large enough for stats and debug commands */
#define SYN_CLI_MAX_ARGS       6
#define SYN_CLI_HISTORY_DEPTH  0

/* Input / Output */
#define SYN_USE_BUTTON         0
#define SYN_USE_ENCODER        0
#define SYN_USE_LED            1
#define SYN_USE_SOFT_PWM       0

/* Control & Motor */
#define SYN_USE_PID            0
#define SYN_USE_STEPPER        0
#define SYN_USE_SERVO          0
#define SYN_USE_DC_MOTOR       0
#define SYN_USE_MOTOR_CTRL     0

/* DSP / Filters */
#define SYN_USE_FILTER         1
#define SYN_USE_SIGNAL         1
#define SYN_USE_FSM            1

/* Communication */
#define SYN_USE_COBS           0
#define SYN_USE_MODBUS         0

/* Sensor */
#define SYN_USE_SENSOR         0

/* Storage */
#define SYN_USE_PARAM          0

/* System */
#define SYN_USE_BOOT           0
#define SYN_USE_ERRLOG         0

/* Debug */
#define SYN_USE_TRACE          0
#define SYN_USE_PROFILER       0

/* Utilities */
#define SYN_USE_FMT            1
#define SYN_CRC_USE_TABLE      0   /* Uses bitwise CRC to save Flash memory */

#endif /* SYN_CONFIG_H */

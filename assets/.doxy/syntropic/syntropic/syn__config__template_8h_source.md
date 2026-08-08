

# File syn\_config\_template.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**syn\_config\_template.h**](syn__config__template_8h.md)

[Go to the documentation of this file](syn__config__template_8h.md)


```C++

#ifndef SYN_CONFIG_H
#define SYN_CONFIG_H

/* ── Drivers ────────────────────────────────────────────────────────────── */

#define SYN_USE_GPIO 1     
#define SYN_USE_UART 1     
#define SYN_USE_ADC 1      
#define SYN_USE_EXTI 1     
#define SYN_USE_HPCLOCK 1  
#define SYN_USE_TIMESYNC 1 
#define SYN_USE_GEO 1      
#define SYN_GNSS_USE_FIXED_POINT \
    0 
/* Console serial (syn_port_serial) is always available — used by CLI/log.  */
/* I2C/SPI device helpers are header-only — always available.               */

/* ── UART tuning ────────────────────────────────────────────────────────── */

#define SYN_UART_TX_BUF_SIZE 128 
#define SYN_UART_RX_BUF_SIZE 128 
#define SYN_UART_MAX_INSTANCES 2 
/* ── Multitasking ───────────────────────────────────────────────────────── */

#define SYN_USE_PT 1        
#define SYN_USE_SCHED 1     
#define SYN_USE_TIMER 1     
#define SYN_USE_EVENT 1     
#define SYN_USE_WATCHDOG 1  
#define SYN_USE_SEQUENCER 1 
#define SYN_USE_WORKQUEUE 1 
#define SYN_USE_TICKLESS 0  
/* Mailbox is header-only — always available.                                */

/* ── Services ───────────────────────────────────────────────────────────── */

#define SYN_USE_LOG 1        
#define SYN_USE_CLI 1        
#define SYN_USE_WASM 1       
#define SYN_WASM_USE_FIXED 0 
/* ── Logging tuning ─────────────────────────────────────────────────────── */

#define SYN_LOG_LEVEL 1      
#define SYN_LOG_BUF_SIZE 192 
#define SYN_LOG_TIMESTAMP 1  
#define SYN_LOG_COLOR 0      
/* ── CLI tuning ─────────────────────────────────────────────────────────── */

#define SYN_CLI_LINE_BUF_SIZE 128 
#define SYN_CLI_MAX_ARGS 16       
#define SYN_CLI_HISTORY_DEPTH 0   
/* ── Input / Output ─────────────────────────────────────────────────────── */

#define SYN_USE_BUTTON 1   
#define SYN_USE_ENCODER 1  
#define SYN_USE_LED 1      
#define SYN_USE_SOFT_PWM 1 
/* ── Control & Motor ────────────────────────────────────────────────────── */

#define SYN_USE_PID 1        
#define SYN_USE_STEPPER 1    
#define SYN_USE_SERVO 1      
#define SYN_USE_DC_MOTOR 1   
#define SYN_USE_MOTOR_CTRL 1 
#define SYN_USE_AUTOTUNE 1   
#define SYN_USE_ACTUATOR 1   
/* ── DSP / Filters ──────────────────────────────────────────────────────── */

#define SYN_USE_FILTER 1         
#define SYN_USE_SIGNAL 1         
#define SYN_USE_BIQUAD 1         
#define SYN_USE_FFT 1            
#define SYN_FILTER_MAX_WINDOW 32 
/* ── State Machines / Concurrency ───────────────────────────────────────── */

#define SYN_USE_FSM 1 
#define SYN_USE_AO 1  
/* ── Cryptography ───────────────────────────────────────────────────────── */

#define SYN_USE_BLAKE2S 1          
#define SYN_USE_CHACHA20POLY1305 1 
#define SYN_USE_X25519 1           
/* ── Communication / Protocols ──────────────────────────────────────────── */

#define SYN_USE_COBS 1           
#define SYN_USE_ETHERCAT 1       
#define SYN_USE_LIN 1            
#define SYN_USE_DALI 1           
#define SYN_USE_SMBUS 1          
#define SYN_USE_PMBUS 1          
#define SYN_USE_MBUS 1           
#define SYN_USE_MODBUS 1         
#define SYN_USE_MQTT 1           
#define SYN_USE_HTTP 1           
#define SYN_USE_HTTPD 1          
#define SYN_USE_WEBSOCKET 1      
#define SYN_USE_COAP 1           
#define SYN_USE_DNS 1            
#define SYN_USE_ETH 1            
#define SYN_ETH_ARP_CACHE_SIZE 8 
#define SYN_USE_DHCP 1           
#define SYN_USE_ICMP 1           
#define SYN_USE_AUTOIP 1         
#define SYN_USE_NETCFG 1         
#define SYN_USE_IGMP 1           
#define SYN_USE_ROUTER 1         
#define SYN_USE_HEARTBEAT 1      
#define SYN_USE_SNTP 1           
#define SYN_USE_WG \
    1 
#define SYN_WG_MTU 1420         
#define SYN_USE_TRANSPORT_TCP 1 
/* ── Sensor ─────────────────────────────────────────────────────────────── */

#define SYN_USE_SENSOR 1 
/* ── Storage ────────────────────────────────────────────────────────────── */

#define SYN_USE_PARAM 1    
#define SYN_USE_SETTINGS 1 
#define SYN_USE_VFS 1      
#define SYN_USE_LFS 1      
#define SYN_USE_FAT 1      
/* ── Display / UI ───────────────────────────────────────────────────────── */

#define SYN_GFX_BACKEND_CANVAS 0               
#define SYN_GFX_BACKEND_DIRECT 1               
#define SYN_GFX_BACKEND SYN_GFX_BACKEND_CANVAS 
#define SYN_USE_CANVAS 1 
#define SYN_USE_MENU 1   
#define SYN_USE_IMGUI 1  
/* ── System ─────────────────────────────────────────────────────────────── */

#define SYN_USE_BOOT 1     
#define SYN_USE_ERRLOG 1   
#define SYN_USE_FAULT 1    
#define SYN_USE_HWWDT 1    
#define SYN_USE_POWER 1    
#define SYN_USE_COREDUMP 0 
/* #define SYN_COREDUMP_FLASH_ADDR 0x0803F800 */ 
#define SYN_COREDUMP_STACK_SIZE 128              
/* Version and Sleep are header-only — always available.                     */

/* ── Firmware Security ─────────────────────────────────────────────────── */

#define SYN_FW_USE_HMAC 0 
/* ── DMA ───────────────────────────────────────────────────────────────── */

#define SYN_USE_DMA 0 
/* ── Async Peripherals ─────────────────────────────────────────────────── */

#define SYN_USE_I2C_ASYNC 0 
#define SYN_USE_SPI_ASYNC 0 
/* ── Multicore (AMP) ───────────────────────────────────────────────────── */

#define SYN_USE_MULTICORE 0  
#define SYN_SPINLOCK_COUNT 4 
/* ── Debug ──────────────────────────────────────────────────────────────── */

#define SYN_USE_TRACE 1    
#define SYN_USE_PROFILER 1 
/* ── Logging ────────────────────────────────────────────────────────────── */

#define SYN_USE_DATALOG 1 
/* ── Utilities ──────────────────────────────────────────────────────────── */

#define SYN_USE_FMT 1           
#define SYN_USE_CBOR 1          
#define SYN_USE_JSON 1          
#define SYN_USE_SHA256 1        
#define SYN_USE_POOL 1          
#define SYN_USE_PUBSUB 1        
#define SYN_USE_RAMP 1          
#define SYN_USE_SCURVE 1        
#define SYN_USE_GOERTZEL 1      
#define SYN_USE_LZ4 1           
#define SYN_USE_USB_MIDI 1      
#define SYN_USE_MFCC 1          
#define SYN_USE_USB_MSC 1       
#define SYN_USE_NTP_SERVER 1    
#define SYN_USE_PROTOBUF 1      
#define SYN_LZ4_HASH_SIZE 4096U 
#define SYN_CRC_USE_TABLE 1     
/* Ring buffer, assert, bits, timeout, hysteresis, change filter, LUT,        */
/* Q-math, rate limiter, ping-pong, CRC, and mailbox are always available.    */

/* ── Assert configuration ───────────────────────────────────────────────── */

/* Uncomment to compile out all SYN_ASSERT() calls in release builds. */
/* #define SYN_DISABLE_ASSERT */

/* ── Audio configuration ────────────────────────────────────────────────── */
#ifndef SYN_AUDIO_MIXER_MAX_CHANNELS
#define SYN_AUDIO_MIXER_MAX_CHANNELS 4U 
#endif

#endif /* SYN_CONFIG_H */
```



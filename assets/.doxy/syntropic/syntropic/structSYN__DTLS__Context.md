

# Struct SYN\_DTLS\_Context



[**ClassList**](annotated.md) **>** [**SYN\_DTLS\_Context**](structSYN__DTLS__Context.md)



[More...](#detailed-description)

* `#include <syn_dtls.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**app\_rx\_buf**](#variable-app_rx_buf)  <br> |
|  size\_t | [**app\_rx\_len**](#variable-app_rx_len)  <br> |
|  uint8\_t | [**client\_app\_iv**](#variable-client_app_iv)  <br> |
|  uint8\_t | [**client\_app\_key**](#variable-client_app_key)  <br> |
|  uint8\_t | [**client\_app\_secret**](#variable-client_app_secret)  <br> |
|  uint64\_t | [**client\_seq\_num**](#variable-client_seq_num)  <br> |
|  [**SYN\_DTLS\_Config**](structSYN__DTLS__Config.md) | [**config**](#variable-config)  <br> |
|  uint8\_t | [**ecdhe\_priv**](#variable-ecdhe_priv)  <br> |
|  uint8\_t | [**ecdhe\_pub**](#variable-ecdhe_pub)  <br> |
|  [**SYN\_DTLS\_Epoch**](syn__dtls_8h.md#enum-syn_dtls_epoch) | [**epoch**](#variable-epoch)  <br> |
|  uint8\_t | [**master\_secret**](#variable-master_secret)  <br> |
|  uint8\_t | [**peer\_ecdhe\_pub**](#variable-peer_ecdhe_pub)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**pt**](#variable-pt)  <br> |
|  [**SYN\_DTLS\_ReplayWindow**](structSYN__DTLS__ReplayWindow.md) | [**replay\_window**](#variable-replay_window)  <br> |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  uint8\_t | [**server\_app\_iv**](#variable-server_app_iv)  <br> |
|  uint8\_t | [**server\_app\_key**](#variable-server_app_key)  <br> |
|  uint8\_t | [**server\_app\_secret**](#variable-server_app_secret)  <br> |
|  uint64\_t | [**server\_seq\_num**](#variable-server_seq_num)  <br> |
|  [**SYN\_DTLS\_State**](syn__dtls_8h.md#enum-syn_dtls_state) | [**state**](#variable-state)  <br> |
|  uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_buf\_size**](#variable-tx_buf_size)  <br> |
|  [**SYN\_Transport**](structSYN__Transport.md) \* | [**underlying\_transport**](#variable-underlying_transport)  <br> |












































## Detailed Description


DTLS 1.3 Engine Context (Caller-owned, zero-heap). 


    
## Public Attributes Documentation




### variable app\_rx\_buf 

```C++
uint8_t SYN_DTLS_Context::app_rx_buf[SYN_DTLS_RECORD_MAX_PAYLOAD];
```



Decrypted payload cache 


        

<hr>



### variable app\_rx\_len 

```C++
size_t SYN_DTLS_Context::app_rx_len;
```



Decrypted payload length 


        

<hr>



### variable client\_app\_iv 

```C++
uint8_t SYN_DTLS_Context::client_app_iv[12];
```



Client record base IV 


        

<hr>



### variable client\_app\_key 

```C++
uint8_t SYN_DTLS_Context::client_app_key[SYN_DTLS_SECRET_LEN];
```



Client record encryption key 


        

<hr>



### variable client\_app\_secret 

```C++
uint8_t SYN_DTLS_Context::client_app_secret[SYN_DTLS_SECRET_LEN];
```



Client application traffic secret 


        

<hr>



### variable client\_seq\_num 

```C++
uint64_t SYN_DTLS_Context::client_seq_num;
```



Outgoing 48-bit record sequence number 


        

<hr>



### variable config 

```C++
SYN_DTLS_Config SYN_DTLS_Context::config;
```



Engine configuration copy 


        

<hr>



### variable ecdhe\_priv 

```C++
uint8_t SYN_DTLS_Context::ecdhe_priv[32];
```



Local ephemeral ECDHE private key 


        

<hr>



### variable ecdhe\_pub 

```C++
uint8_t SYN_DTLS_Context::ecdhe_pub[32];
```



Local ephemeral ECDHE public key 


        

<hr>



### variable epoch 

```C++
SYN_DTLS_Epoch SYN_DTLS_Context::epoch;
```



Active DTLS record epoch 


        

<hr>



### variable master\_secret 

```C++
uint8_t SYN_DTLS_Context::master_secret[SYN_DTLS_SECRET_LEN];
```



Derived DTLS 1.3 master secret 


        

<hr>



### variable peer\_ecdhe\_pub 

```C++
uint8_t SYN_DTLS_Context::peer_ecdhe_pub[32];
```



Peer ephemeral ECDHE public key 


        

<hr>



### variable pt 

```C++
SYN_PT SYN_DTLS_Context::pt;
```



Non-blocking task state 


        

<hr>



### variable replay\_window 

```C++
SYN_DTLS_ReplayWindow SYN_DTLS_Context::replay_window;
```



Sliding window for incoming datagrams 


        

<hr>



### variable rx\_buf 

```C++
uint8_t* SYN_DTLS_Context::rx_buf;
```



Caller-owned RX record buffer 


        

<hr>



### variable rx\_buf\_size 

```C++
size_t SYN_DTLS_Context::rx_buf_size;
```



RX buffer capacity 


        

<hr>



### variable server\_app\_iv 

```C++
uint8_t SYN_DTLS_Context::server_app_iv[12];
```



Server record base IV 


        

<hr>



### variable server\_app\_key 

```C++
uint8_t SYN_DTLS_Context::server_app_key[SYN_DTLS_SECRET_LEN];
```



Server record encryption key 


        

<hr>



### variable server\_app\_secret 

```C++
uint8_t SYN_DTLS_Context::server_app_secret[SYN_DTLS_SECRET_LEN];
```



Server application traffic secret 


        

<hr>



### variable server\_seq\_num 

```C++
uint64_t SYN_DTLS_Context::server_seq_num;
```



Expected incoming sequence number 


        

<hr>



### variable state 

```C++
SYN_DTLS_State SYN_DTLS_Context::state;
```



Current handshake state 


        

<hr>



### variable tx\_buf 

```C++
uint8_t* SYN_DTLS_Context::tx_buf;
```



Caller-owned TX record buffer 


        

<hr>



### variable tx\_buf\_size 

```C++
size_t SYN_DTLS_Context::tx_buf_size;
```



TX buffer capacity 


        

<hr>



### variable underlying\_transport 

```C++
SYN_Transport* SYN_DTLS_Context::underlying_transport;
```



Wire transport (UDP, Serial, etc.) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dtls.h`


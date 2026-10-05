

# Struct SYN\_TLS\_Context



[**ClassList**](annotated.md) **>** [**SYN\_TLS\_Context**](structSYN__TLS__Context.md)



[More...](#detailed-description)

* `#include <syn_tls.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**app\_rx\_buf**](#variable-app_rx_buf)  <br> |
|  size\_t | [**app\_rx\_head**](#variable-app_rx_head)  <br> |
|  size\_t | [**app\_rx\_tail**](#variable-app_rx_tail)  <br> |
|  uint8\_t | [**client\_app\_iv**](#variable-client_app_iv)  <br> |
|  uint8\_t | [**client\_app\_key**](#variable-client_app_key)  <br> |
|  uint8\_t | [**client\_app\_secret**](#variable-client_app_secret)  <br> |
|  uint8\_t | [**client\_handshake\_secret**](#variable-client_handshake_secret)  <br> |
|  uint64\_t | [**client\_seq\_num**](#variable-client_seq_num)  <br> |
|  [**SYN\_TLS\_Config**](structSYN__TLS__Config.md) | [**config**](#variable-config)  <br> |
|  uint32\_t | [**handshake\_start\_ms**](#variable-handshake_start_ms)  <br> |
|  uint8\_t | [**master\_secret**](#variable-master_secret)  <br> |
|  uint8\_t | [**my\_privkey**](#variable-my_privkey)  <br> |
|  uint8\_t | [**my\_pubkey**](#variable-my_pubkey)  <br> |
|  uint8\_t | [**peer\_pubkey**](#variable-peer_pubkey)  <br> |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  uint8\_t | [**rx\_content\_type**](#variable-rx_content_type)  <br> |
|  size\_t | [**rx\_pos**](#variable-rx_pos)  <br> |
|  size\_t | [**rx\_record\_len**](#variable-rx_record_len)  <br> |
|  uint8\_t | [**server\_app\_iv**](#variable-server_app_iv)  <br> |
|  uint8\_t | [**server\_app\_key**](#variable-server_app_key)  <br> |
|  uint8\_t | [**server\_app\_secret**](#variable-server_app_secret)  <br> |
|  uint8\_t | [**server\_handshake\_secret**](#variable-server_handshake_secret)  <br> |
|  uint64\_t | [**server\_seq\_num**](#variable-server_seq_num)  <br> |
|  [**SYN\_TLS\_State**](syn__tls_8h.md#enum-syn_tls_state) | [**state**](#variable-state)  <br> |
|  [**SYN\_SHA256**](structSYN__SHA256.md) | [**transcript\_hash**](#variable-transcript_hash)  <br> |
|  uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_buf\_size**](#variable-tx_buf_size)  <br> |
|  [**SYN\_Transport**](structSYN__Transport.md) \* | [**underlying\_transport**](#variable-underlying_transport)  <br> |












































## Detailed Description


TLS 1.3 Engine Context (Caller-owned, zero-heap). 


    
## Public Attributes Documentation




### variable app\_rx\_buf 

```C++
uint8_t SYN_TLS_Context::app_rx_buf[SYN_TLS_RECORD_MAX_PAYLOAD];
```



Decrypted app data ring buffer 


        

<hr>



### variable app\_rx\_head 

```C++
size_t SYN_TLS_Context::app_rx_head;
```



App data head index 


        

<hr>



### variable app\_rx\_tail 

```C++
size_t SYN_TLS_Context::app_rx_tail;
```



App data tail index 


        

<hr>



### variable client\_app\_iv 

```C++
uint8_t SYN_TLS_Context::client_app_iv[12];
```



Cached client AEAD base IV 


        

<hr>



### variable client\_app\_key 

```C++
uint8_t SYN_TLS_Context::client_app_key[SYN_TLS_SECRET_LEN];
```



Cached client AEAD key 


        

<hr>



### variable client\_app\_secret 

```C++
uint8_t SYN_TLS_Context::client_app_secret[SYN_TLS_SECRET_LEN];
```



Client application traffic secret 


        

<hr>



### variable client\_handshake\_secret 

```C++
uint8_t SYN_TLS_Context::client_handshake_secret[SYN_TLS_SECRET_LEN];
```



Client handshake traffic secret 


        

<hr>



### variable client\_seq\_num 

```C++
uint64_t SYN_TLS_Context::client_seq_num;
```



Encryption record sequence counter 


        

<hr>



### variable config 

```C++
SYN_TLS_Config SYN_TLS_Context::config;
```



Engine configuration copy 


        

<hr>



### variable handshake\_start\_ms 

```C++
uint32_t SYN_TLS_Context::handshake_start_ms;
```



Handshake start timestamp 


        

<hr>



### variable master\_secret 

```C++
uint8_t SYN_TLS_Context::master_secret[SYN_TLS_SECRET_LEN];
```



Master secret 


        

<hr>



### variable my\_privkey 

```C++
uint8_t SYN_TLS_Context::my_privkey[SYN_TLS_SECRET_LEN];
```



Ephemeral X25519 private key 


        

<hr>



### variable my\_pubkey 

```C++
uint8_t SYN_TLS_Context::my_pubkey[SYN_TLS_SECRET_LEN];
```



Ephemeral X25519 public key 


        

<hr>



### variable peer\_pubkey 

```C++
uint8_t SYN_TLS_Context::peer_pubkey[SYN_TLS_SECRET_LEN];
```



Peer ephemeral X25519 public key 


        

<hr>



### variable rx\_buf 

```C++
uint8_t* SYN_TLS_Context::rx_buf;
```



Caller-owned RX record buffer 


        

<hr>



### variable rx\_buf\_size 

```C++
size_t SYN_TLS_Context::rx_buf_size;
```



RX record buffer capacity 


        

<hr>



### variable rx\_content\_type 

```C++
uint8_t SYN_TLS_Context::rx_content_type;
```



Current record content type 


        

<hr>



### variable rx\_pos 

```C++
size_t SYN_TLS_Context::rx_pos;
```



Bytes accumulated in rx\_buf 


        

<hr>



### variable rx\_record\_len 

```C++
size_t SYN_TLS_Context::rx_record_len;
```



Expected record payload length 


        

<hr>



### variable server\_app\_iv 

```C++
uint8_t SYN_TLS_Context::server_app_iv[12];
```



Cached server AEAD base IV 


        

<hr>



### variable server\_app\_key 

```C++
uint8_t SYN_TLS_Context::server_app_key[SYN_TLS_SECRET_LEN];
```



Cached server AEAD key 


        

<hr>



### variable server\_app\_secret 

```C++
uint8_t SYN_TLS_Context::server_app_secret[SYN_TLS_SECRET_LEN];
```



Server application traffic secret 


        

<hr>



### variable server\_handshake\_secret 

```C++
uint8_t SYN_TLS_Context::server_handshake_secret[SYN_TLS_SECRET_LEN];
```



Server handshake traffic secret 


        

<hr>



### variable server\_seq\_num 

```C++
uint64_t SYN_TLS_Context::server_seq_num;
```



Decryption record sequence counter 


        

<hr>



### variable state 

```C++
SYN_TLS_State SYN_TLS_Context::state;
```



Current handshake state 


        

<hr>



### variable transcript\_hash 

```C++
SYN_SHA256 SYN_TLS_Context::transcript_hash;
```



Running handshake transcript hash 


        

<hr>



### variable tx\_buf 

```C++
uint8_t* SYN_TLS_Context::tx_buf;
```



Caller-owned TX record buffer 


        

<hr>



### variable tx\_buf\_size 

```C++
size_t SYN_TLS_Context::tx_buf_size;
```



TX record buffer capacity 


        

<hr>



### variable underlying\_transport 

```C++
SYN_Transport* SYN_TLS_Context::underlying_transport;
```



Wire transport (TCP, UDP, Serial) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tls.h`


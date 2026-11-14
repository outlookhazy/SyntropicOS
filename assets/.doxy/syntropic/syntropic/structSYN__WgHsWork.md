

# Struct SYN\_WgHsWork



[**ClassList**](annotated.md) **>** [**SYN\_WgHsWork**](structSYN__WgHsWork.md)



_Staged handshake crypto scratchpad for cooperative X25519 yielding._ 

* `#include <syn_wg.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  uint8\_t | [**ck**](#variable-ck)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**crypto\_pt**](#variable-crypto_pt)  <br> |
|  uint8\_t | [**dh\_ok**](#variable-dh_ok)  <br> |
|  uint8\_t | [**enc\_static**](#variable-enc_static)  <br> |
|  uint8\_t | [**enc\_ts**](#variable-enc_ts)  <br> |
|  uint8\_t | [**eph\_priv**](#variable-eph_priv)  <br> |
|  uint8\_t | [**eph\_pub**](#variable-eph_pub)  <br> |
|  uint32\_t | [**expected\_idx**](#variable-expected_idx)  <br> |
|  uint8\_t | [**h**](#variable-h)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**hs\_pt**](#variable-hs_pt)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_rekey**](#variable-is_rekey)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_resp**](#variable-is_resp)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ok**](#variable-ok)  <br> |
|  uint32\_t | [**peer\_idx**](#variable-peer_idx)  <br> |
|  uint8\_t | [**tmp\_key**](#variable-tmp_key)  <br> |
|  [**SYN\_X25519\_Ctx**](structSYN__X25519__Ctx.md) | [**x25519**](#variable-x25519)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_WgHsWork::active;
```



True while handshake protothread is running 


        

<hr>



### variable ck 

```C++
uint8_t SYN_WgHsWork::ck[32];
```



Staged Noise chaining key 


        

<hr>



### variable crypto\_pt 

```C++
SYN_PT SYN_WgHsWork::crypto_pt;
```



Child protothread for syn\_x25519\_pt 


        

<hr>



### variable dh\_ok 

```C++
uint8_t SYN_WgHsWork::dh_ok;
```



Constant-time non-zero DH accumulator 


        

<hr>



### variable enc\_static 

```C++
uint8_t SYN_WgHsWork::enc_static[48];
```



Encrypted static key (48B) or resp tag (16B) 


        

<hr>



### variable enc\_ts 

```C++
uint8_t SYN_WgHsWork::enc_ts[28];
```



Encrypted TAI64N timestamp + AEAD tag 


        

<hr>



### variable eph\_priv 

```C++
uint8_t SYN_WgHsWork::eph_priv[32];
```



Staged ephemeral private key 


        

<hr>



### variable eph\_pub 

```C++
uint8_t SYN_WgHsWork::eph_pub[32];
```



Staged local or peer ephemeral public key 


        

<hr>



### variable expected\_idx 

```C++
uint32_t SYN_WgHsWork::expected_idx;
```



Initiator sender index (staged or expected) 


        

<hr>



### variable h 

```C++
uint8_t SYN_WgHsWork::h[32];
```



Staged Noise handshake hash 


        

<hr>



### variable hs\_pt 

```C++
SYN_PT SYN_WgHsWork::hs_pt;
```



Active handshake protothread continuation 


        

<hr>



### variable is\_rekey 

```C++
bool SYN_WgHsWork::is_rekey;
```



True when staged initiation is for rekey 


        

<hr>



### variable is\_resp 

```C++
bool SYN_WgHsWork::is_resp;
```



True for response consumption, false init 


        

<hr>



### variable ok 

```C++
bool SYN_WgHsWork::ok;
```



True when handshake PT completed cleanly 


        

<hr>



### variable peer\_idx 

```C++
uint32_t SYN_WgHsWork::peer_idx;
```



Responder sender index from response 


        

<hr>



### variable tmp\_key 

```C++
uint8_t SYN_WgHsWork::tmp_key[32];
```



Staged DH shared secret or AEAD key 


        

<hr>



### variable x25519 

```C++
SYN_X25519_Ctx SYN_WgHsWork::x25519;
```



Active cooperative X25519 scalar-mul context 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_wg.h`


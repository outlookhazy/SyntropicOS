

# Struct SYN\_OCPP\_Client



[**ClassList**](annotated.md) **>** [**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md)



_OCPP Client instance state (EVSE Role)._ 

* `#include <syn_ocpp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  char | [**active\_id\_tag**](#variable-active_id_tag)  <br> |
|  int32\_t | [**active\_transaction\_id**](#variable-active_transaction_id)  <br> |
|  [**SYN\_OCPP\_AuthorizationHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_authorizationhandler) | [**auth\_cb**](#variable-auth_cb)  <br> |
|  uint32\_t | [**connector\_id**](#variable-connector_id)  <br> |
|  [**SYN\_OCPP\_ChargePointStatus**](syn__ocpp_8h.md#enum-syn_ocpp_chargepointstatus) | [**connector\_status**](#variable-connector_status)  <br> |
|  uint32\_t | [**heartbeat\_interval\_sec**](#variable-heartbeat_interval_sec)  <br> |
|  uint32\_t | [**heartbeat\_timer\_ms**](#variable-heartbeat_timer_ms)  <br> |
|  uint32\_t | [**message\_counter**](#variable-message_counter)  <br> |
|  [**SYN\_OCPP\_RegistrationHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_registrationhandler) | [**reg\_cb**](#variable-reg_cb)  <br> |
|  [**SYN\_OCPP\_RegistrationStatus**](syn__ocpp_8h.md#enum-syn_ocpp_registrationstatus) | [**registration\_status**](#variable-registration_status)  <br> |
|  [**SYN\_OCPP\_RemoteStartHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_remotestarthandler) | [**remote\_start\_cb**](#variable-remote_start_cb)  <br> |
|  [**SYN\_OCPP\_RemoteStopHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_remotestophandler) | [**remote\_stop\_cb**](#variable-remote_stop_cb)  <br> |
|  [**SYN\_OCPP\_StartTxHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_starttxhandler) | [**start\_tx\_cb**](#variable-start_tx_cb)  <br> |
|  uint32\_t | [**tx\_start\_energy\_wh**](#variable-tx_start_energy_wh)  <br> |
|  void \* | [**user\_ctx**](#variable-user_ctx)  <br> |
|  [**SYN\_OCPP\_V2GMode**](syn__ocpp_8h.md#enum-syn_ocpp_v2gmode) | [**v2g\_mode**](#variable-v2g_mode)  <br> |
|  [**SYN\_OCPP\_Version**](syn__ocpp_8h.md#enum-syn_ocpp_version) | [**version**](#variable-version)  <br> |












































## Public Attributes Documentation




### variable active\_id\_tag 

```C++
char SYN_OCPP_Client::active_id_tag[32];
```



Active authorized RFID tag 


        

<hr>



### variable active\_transaction\_id 

```C++
int32_t SYN_OCPP_Client::active_transaction_id;
```



Active transaction ID (-1 if none) 


        

<hr>



### variable auth\_cb 

```C++
SYN_OCPP_AuthorizationHandler SYN_OCPP_Client::auth_cb;
```



Authorization response callback 


        

<hr>



### variable connector\_id 

```C++
uint32_t SYN_OCPP_Client::connector_id;
```



Active connector ID (e.g. 1) 


        

<hr>



### variable connector\_status 

```C++
SYN_OCPP_ChargePointStatus SYN_OCPP_Client::connector_status;
```



Current connector status 


        

<hr>



### variable heartbeat\_interval\_sec 

```C++
uint32_t SYN_OCPP_Client::heartbeat_interval_sec;
```



Heartbeat interval in seconds 


        

<hr>



### variable heartbeat\_timer\_ms 

```C++
uint32_t SYN_OCPP_Client::heartbeat_timer_ms;
```



Heartbeat countdown timer ms 


        

<hr>



### variable message\_counter 

```C++
uint32_t SYN_OCPP_Client::message_counter;
```



Monotonic message ID counter 


        

<hr>



### variable reg\_cb 

```C++
SYN_OCPP_RegistrationHandler SYN_OCPP_Client::reg_cb;
```



Registration response callback 


        

<hr>



### variable registration\_status 

```C++
SYN_OCPP_RegistrationStatus SYN_OCPP_Client::registration_status;
```



Central System registration state 


        

<hr>



### variable remote\_start\_cb 

```C++
SYN_OCPP_RemoteStartHandler SYN_OCPP_Client::remote_start_cb;
```



RemoteStart command callback 


        

<hr>



### variable remote\_stop\_cb 

```C++
SYN_OCPP_RemoteStopHandler SYN_OCPP_Client::remote_stop_cb;
```



RemoteStop command callback 


        

<hr>



### variable start\_tx\_cb 

```C++
SYN_OCPP_StartTxHandler SYN_OCPP_Client::start_tx_cb;
```



StartTx response callback 


        

<hr>



### variable tx\_start\_energy\_wh 

```C++
uint32_t SYN_OCPP_Client::tx_start_energy_wh;
```



Meter Wh at transaction start 


        

<hr>



### variable user\_ctx 

```C++
void* SYN_OCPP_Client::user_ctx;
```



User context pointer 


        

<hr>



### variable v2g\_mode 

```C++
SYN_OCPP_V2GMode SYN_OCPP_Client::v2g_mode;
```



ISO 15118-20 V2G mode (Scheduled / Dynamic) 


        

<hr>



### variable version 

```C++
SYN_OCPP_Version SYN_OCPP_Client::version;
```



Negotiated OCPP version (1.6 / 2.0.1 / 2.1) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ocpp.h`


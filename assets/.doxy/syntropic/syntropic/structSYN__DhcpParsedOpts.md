

# Struct SYN\_DhcpParsedOpts



[**ClassList**](annotated.md) **>** [**SYN\_DhcpParsedOpts**](structSYN__DhcpParsedOpts.md)



_Parsed DHCP option values extracted from a BOOTP options block._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**lease**](#variable-lease)  <br> |
|  uint8\_t | [**msg\_type**](#variable-msg_type)  <br> |
|  uint32\_t | [**router**](#variable-router)  <br> |
|  uint32\_t | [**server**](#variable-server)  <br> |
|  uint32\_t | [**subnet**](#variable-subnet)  <br> |












































## Public Attributes Documentation




### variable lease 

```C++
uint32_t SYN_DhcpParsedOpts::lease;
```



IP Address Lease Time in seconds (Option 51) 


        

<hr>



### variable msg\_type 

```C++
uint8_t SYN_DhcpParsedOpts::msg_type;
```



DHCP Message Type (Option 53) 


        

<hr>



### variable router 

```C++
uint32_t SYN_DhcpParsedOpts::router;
```



Default Router (Option 3) 


        

<hr>



### variable server 

```C++
uint32_t SYN_DhcpParsedOpts::server;
```



DHCP Server Identifier (Option 54) 


        

<hr>



### variable subnet 

```C++
uint32_t SYN_DhcpParsedOpts::subnet;
```



Subnet Mask (Option 1) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dhcp.c`


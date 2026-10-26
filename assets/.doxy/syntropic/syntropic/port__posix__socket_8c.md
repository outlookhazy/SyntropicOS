

# File port\_posix\_socket.c



[**FileList**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**posix**](dir_6d923334ab2bb155e44e019dd9da5e9c.md) **>** [**port\_posix\_socket.c**](port__posix__socket_8c.md)

[Go to the source code of this file](port__posix__socket_8c_source.md)

_POSIX socket implementation of the SyntropicOS socket port layer._ [More...](#detailed-description)


































































## Detailed Description


Provides UDP and TCP socket functions using the standard BSD/POSIX socket API. Works on any platform with POSIX sockets: Linux, macOS, ESP-IDF (lwIP), Zephyr, and any RTOS with a lwIP or POSIX socket shim.


Compile this file into your project alongside your platform's system port (e.g. [**port\_esp32.c**](port__esp32_8c.md) for GPIO/timer/UART) to get full networking support. 


    

------------------------------
The documentation for this class was generated from the following file `src/port/posix/port_posix_socket.c`


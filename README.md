# cpp-socket
> A C++ Wrapper to provide safer development using GCC socket library.

The motivation of this library is to leverage modern C++ feature to make socket usage less intimidating for developers (e.g. domain,transport and protocl options, client-server setup, numerous options parameters, different options groups, etc).

As of now, the library is providing concise type-safe way to create client and server. Setting up client and server is as simple as follows.
```cpp
using Client = Client<Domain::IPV4, Transport::TCP, Protocol::IP>
```
```cpp
using Server = Server<Domain::IPV4, Transport::TCP, Protocol::IP>
```

Also, configuring socket is not as clueless as C inet library as seen below.
```cpp
server.configure<Option::SocketOption>(Option::SocketOption::EnableReuseAddress, 1)
```
> DO note that configuration is not made type safe (yet). I am still sceptical of making trade-off for required flexibility.

The samples in `sample` directory contains c++ main code to provide basic usage of the library.

## Getting started
Simply clone this repository to your linux working directory
```
git clone git@github.com:dannyyws/cpp-socket.git
```

## Feature
- Provide a type-safe, concise method to setup server and client socket.
- Provide verbose importing of parameter via namespace hint.
- Currently working on single client-server using TCPIP,IPV4 and UDP,IPV4.

## Action Items
- Single client-server on TCPIP, IPV6
- Multi client-server on TCPIP
- Multicast on UDP (IPV6?)
- Think of a type-safe way to handle returning exception.

# License
This project is licensed under Unlicense license. This license does not require you to take the license with you to your project.

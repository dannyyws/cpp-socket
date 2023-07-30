#include <socketUtil/endpoint.h>

template<typename... TRANSPORT>
struct Client: SocketEndpoint<Client, TRANSPORT...>
{
    using SocketEndpoint<Client, TRANSPORT...>::socketPtr;
    using SocketEndpoint<Client, TRANSPORT...>::socket_address;

    [[nodiscard]]
    int connectServer(const IPV4_ADDRESS &ipAddress, IPVX_PORT port)
    {
        return socketPtr->connect(ipAddress, port);
    }
};
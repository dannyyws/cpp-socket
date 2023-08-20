#ifndef CLIENT_H
#define CLIENT_H

#include <socketUtil/endpoint.h>

template <typename DOMAIN,
          typename TRANSPORT,
          typename PROTOCOL>
struct Client : SocketEndpoint<Client, DOMAIN, TRANSPORT, PROTOCOL>
{
    using SocketEndpoint<Client, DOMAIN, TRANSPORT, PROTOCOL>::socketPtr;
    using SocketEndpoint<Client, DOMAIN, TRANSPORT, PROTOCOL>::socket_address;

    [[nodiscard]] int connectServer(const IPV4_ADDRESS &&ipAddress, IPPORT_TYPE port)
    {
        return socketPtr->connect(std::move(ipAddress), port);
    }

    [[nodiscard]] int connectServer(const IPV6_ADDRESS &&ipAddress, IPPORT_TYPE port)
    {
        return socketPtr->connect(std::move(ipAddress), port);
    }

    [[nodiscard]] int send(const BytesBuffer &message)
    {
        return socketPtr->send(message);
    }

    const BytesBuffer &receive()
    {
        return socketPtr->receive();
    }
};

#endif
#ifndef SERVER_H
#define SERVER_H

#include <socketUtil/endpoint.h>

template <typename DOMAIN,
          typename TRANSPORT,
          typename PROTOCOL>
struct Server : SocketEndpoint<Server, DOMAIN, TRANSPORT, PROTOCOL>
{
    using SocketEndpoint<Server, DOMAIN, TRANSPORT, PROTOCOL>::socketPtr;

    [[nodiscard]] int bindPort(const IPV4_ADDRESS &&ipAddress, IPPORT_TYPE port)
    {
        return socketPtr->binds(std::move(ipAddress), port);
    }

    [[nodiscard]] int bindPort(const IPV6_ADDRESS &&ipAddress, IPPORT_TYPE port)
    {
        return socketPtr->binds(std::move(ipAddress), port);
    }

    [[nodiscard]] int startListen(int maxConnection = SOMAXCONN)
    {
        return socketPtr->listen(maxConnection);
    }

    int acceptClient()
    {
        return socketPtr->accept();
    }

    /// @brief TCP must have targeted client id to execute
    /// @param clientId
    /// @param bytesBuffer
    /// @return
    [[nodiscard]] int send(int clientId, BytesBuffer &bytesBuffer)
    {
        static_assert(std::is_same_v<TRANSPORT, Transport::TCP>, "Server needs client ID for TCP connection.");
        return socketPtr->send(clientId, bytesBuffer);
    }

    /// @brief UDP do not need client id
    /// @param message
    /// @return
    [[nodiscard]] int send(const BytesBuffer &message)
    {
        static_assert(std::is_same_v<TRANSPORT, Transport::UDP>, "Server can send without client ID using UDP only.");
        return socketPtr->send(message);
    }

    /// @brief Receive buffer from connected socket with clientId fs.
    /// @param clientId
    /// @return Number of bytes read. Or error if connected socket is closed(normally).
    const BytesBuffer &receive(const int clientId)
    {
        static_assert(std::is_same_v<TRANSPORT, Transport::TCP>, "Server needs client ID for TCP connection.");
        return socketPtr->receive(clientId);
    }

    /// @brief UDP do not need client id
    /// @param message
    /// @return
    [[nodiscard]] const BytesBuffer &receive()
    {
        static_assert(std::is_same_v<TRANSPORT, Transport::UDP>, "Server can receive without client ID using UDP only.");
        return socketPtr->receive();
    }
};

#endif
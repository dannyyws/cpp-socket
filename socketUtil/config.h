#ifndef CONFIG_H
#define CONFIG_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <type_traits>

// Create socket
template <typename DOMAIN>
struct NetworkDomain
{
    struct tag
    {
    };
};
inline namespace Domain
{
    struct POSIX : NetworkDomain<POSIX>, std::integral_constant<int, AF_LOCAL>
    {
        using socket_address = void;
        using Tag = tag;
    };
    struct IPV4 : NetworkDomain<IPV4>, std::integral_constant<int, AF_INET>
    {
        using socket_address = sockaddr_in;
        using Tag = tag;
    };
    struct IPV6 : NetworkDomain<IPV6>, std::integral_constant<int, AF_INET6>
    {
        using socket_address = sockaddr_in6;
        using Tag = tag;
    };
}

template <typename TRANSPORT>
struct TransportProtocol
{
};
inline namespace Transport
{
    struct TCP : TransportProtocol<TCP>, std::integral_constant<int, SOCK_STREAM>
    {
    };
    struct UDP : TransportProtocol<UDP>, std::integral_constant<int, SOCK_DGRAM>
    {
    };
}

template <typename PROTOCOL>
struct NetworkProtocol
{
};
inline namespace Protocol
{
    struct IP : NetworkProtocol<IP>, std::integral_constant<int, IPPROTO_IP>
    {
    };
    struct UDP : NetworkProtocol<UDP>, std::integral_constant<int, IPPROTO_UDP>
    {
    };
}

// Configure socket option
template <typename OPT_LEVEL>
struct OptionLevel
{
};
inline namespace Option
{
    // Socket layer configuration
    struct SocketOption : OptionLevel<SocketOption>, std::integral_constant<int, SOL_SOCKET>
    {
        // static integer here due to option is more dynamic than level or socket type
        static constexpr int EnableReuseAddress{SO_REUSEADDR};
        static constexpr int EnableReusePort{SO_REUSEPORT};
        static constexpr int EnableKeepAlive{SO_KEEPALIVE};
        static constexpr int SendBufferSize{SO_SNDBUF};
        static constexpr int ReceiveBufferSize{SO_RCVBUF};
        static constexpr int EnableBroadcast{SO_BROADCAST};
        static constexpr int EnableLingerTxClose{SO_LINGER};
        static constexpr int EnableOutOfBandQueue{SO_OOBINLINE};
    };

    struct TcpOption : OptionLevel<TcpOption>, std::integral_constant<int, IPPROTO_TCP>
    {
        static constexpr int NoDelay{TCP_NODELAY};
        // <-- not advisable to configure in portable code -->
    };

    struct IpOption : OptionLevel<IpOption>, std::integral_constant<int, IPPROTO_IP>
    {
        enum TypeOfService
        {
            LowDelay = IPTOS_LOWDELAY,
            HighThroughput = IPTOS_THROUGHPUT,
            Reliable = IPTOS_RELIABILITY,
            // MinCost // What's the use model?
        };

        static constexpr int TimeToLive{IP_TTL};
        static constexpr int MulticastTimeToLive{IP_MULTICAST_TTL};
    };

    struct UdpOption : OptionLevel<UdpOption>, std::integral_constant<int, IPPROTO_UDP>
    {
        static constexpr int EnableCork{UDP_CORK};
    };
};

#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

#include <socketUtil/ipstruct.h>
#include <socketUtil/config.h>
#include <socketUtil/exception.h>

#include <util/buffer.h>

/// @Interface to create socket in a more friendly way while maintaining performant code 
/// @tparam ENDPOINT - Server / client 
/// @tparam TYPE - Network address type
/// @tparam PROTOCOL - Network protocol
/// @tparam MODE - Transport protocol
template<template <typename...> class ENDPOINT, 
         typename DOMAIN, 
         typename TRANSPORT, 
         typename PROTOCOL>
struct SocketEndpoint
{
    using socket_address = typename DOMAIN::socket_address;
    using socket_address_ptr = socket_address*;
    using address_size = socklen_t;
    using address_size_ptr = socklen_t*;

    SocketEndpoint(int bufferSize)
    {
        socketPtr = std::make_unique<Socket>(bufferSize);
    }

    template<typename OPTION_LEVEL>
    [[nodiscard]]
    int configure(int parameter, int value);
    
    [[nodiscard]]
    int bindPort(const IPV4_ADDRESS& ipAddress, IPVX_PORT port);

    [[nodiscard]]
    int bindPort(const IPV6_ADDRESS& ipAddress, IPVX_PORT port){ throw NotImplementedException{"Bind IPV6 Port."};}

    const BytesBuffer& receive(){
        return socketPtr->receive();
    }

    const BytesBuffer& receive(socket_address_ptr clientAddressPtr, address_size_ptr addressSizePtr){
        return socketPtr->receive(clientAddressPtr, addressSizePtr);
    }

protected:
    struct Socket
    {

        Socket(const int mtu)
        :m_address{ .sin_family = DOMAIN::value }, m_addressLength {sizeof (m_address)}
        {
            m_self = socket(DOMAIN::value, TRANSPORT::value, PROTOCOL::value);
            if (m_self == -1)
                throw CannotCreateSocketException{};
            m_bufferPtr = std::make_unique<BytesBuffer>(mtu);
        }

        ~Socket() noexcept;

        template <typename OPTION_LEVEL>
        [[nodiscard]]
        int configure(int parameter, int value)
        {
            static_assert(std::is_base_of<OptionLevel<OPTION_LEVEL>, OPTION_LEVEL>::value, "Option level is not supported");
            int status = setsockopt(m_self, OPTION_LEVEL::value, parameter, (char *)&value, sizeof(value));

            return status;
        }
        
        [[nodiscard]]
        int binds(const IPV4_ADDRESS& ipAddress, IPVX_PORT port)
        {
            static_assert(std::is_same<DOMAIN, Domain::IPV4>::value, "Cannot use IPV4 address for non-IPV4 domain");
            static_assert(std::is_same<PROTOCOL, Protocol::IP>::value || std::is_same<PROTOCOL, Protocol::UDP>::value, "Socket is not created with IP address.");
            m_address.sin_port  = htons(port),
            m_address.sin_addr.s_addr = static_cast<uint32_t>(ipAddress);

            int status = bind(m_self, reinterpret_cast<sockaddr*>(&m_address), sizeof(m_address));
            return status;
        }
        
        [[nodiscard]]
        int listen(int maxConnection)
        {
            static_assert(std::is_same<TRANSPORT, Transport::UDP>::value == false, "UDP does not listen for connection.");
            m_clients.resize(maxConnection);
            return ::listen(m_self, maxConnection);
        }

        [[nodiscard]]
        int accept()
        {
            static_assert(std::is_same<TRANSPORT, Transport::UDP>::value == false, "UDP does not accept connection");
            int acceptedId = ::accept(m_self, reinterpret_cast<sockaddr*>(&m_address), &m_addressLength);
            m_clients.push_back(acceptedId);

            return acceptedId;
        }

        [[nodiscard]]
        const BytesBuffer& receive()
        {
            socket_address clientAddress{};
            address_size addressSize {sizeof(socket_address)};

            return receive(&clientAddress, &addressSize);
        }

        [[nodiscard]]
        const BytesBuffer& receive(socket_address_ptr clientAddress, address_size_ptr addressSize)
        {
            int response = ::recvfrom(m_self,
                                      reinterpret_cast<void *>(m_bufferPtr->message()),
                                      m_bufferPtr->size(),
                                      0,
                                      reinterpret_cast<sockaddr *>(clientAddress),
                                      addressSize);
            if (response != 0)
            {
                m_bufferPtr->reset();
            }
            return *m_bufferPtr;
        }

    private:
        int m_self{};
        std::unique_ptr<BytesBuffer> m_bufferPtr{};
        std::vector<int> m_clients{};
        socklen_t m_addressLength{};
        struct DOMAIN::socket_address m_address{};
    };
    
protected:
    std::unique_ptr<Socket> socketPtr{};
    
    static_assert(std::is_base_of<NetworkDomain<DOMAIN>, DOMAIN>::value, "Network type is invalid.");
    static_assert(std::is_base_of<TransportProtocol<TRANSPORT>, TRANSPORT>::value, "Transport mode is invalid.");
    static_assert(std::is_base_of<NetworkProtocol<PROTOCOL>, PROTOCOL>::value, "Network protocol is invalid.");
};

template<template <typename...> class ENDPOINT, 
         typename DOMAIN, 
         typename TRANSPORT, 
         typename PROTOCOL>
SocketEndpoint<ENDPOINT, DOMAIN, TRANSPORT, PROTOCOL>::Socket::~Socket() noexcept
{
    //shutdown routine
    for (auto& client : m_clients)
        close(client);
    m_clients.clear();
    ::shutdown(m_self, SHUT_RDWR);
    printf("Socket is shutting down...\n");
}

template<template <typename...> class ENDPOINT, 
         typename DOMAIN, 
         typename TRANSPORT, 
         typename PROTOCOL>
template<typename OPTION_LEVEL>
int SocketEndpoint<ENDPOINT, DOMAIN, TRANSPORT, PROTOCOL>::configure(int parameter, int value)
{
    return socketPtr->template configure<OPTION_LEVEL>(parameter, value);
}

template<template <typename...> class ENDPOINT, 
         typename DOMAIN, 
         typename TRANSPORT, 
         typename PROTOCOL>
int SocketEndpoint<ENDPOINT, DOMAIN, TRANSPORT, PROTOCOL>::bindPort(const IPV4_ADDRESS& ipAddress, IPVX_PORT port)
{
    return socketPtr->binds(ipAddress, port);
}

template<typename... TRANSPORT>
struct Client: SocketEndpoint<Client, TRANSPORT...>
{
    using SocketEndpoint<Client, TRANSPORT...>::socketPtr;
};

template<typename... TRANSPORT>
struct Server: SocketEndpoint<Server, TRANSPORT...> 
{
    using SocketEndpoint<Server, TRANSPORT...>::socketPtr;
    
    [[nodiscard]]
    int startListen(int maxConnection=SOMAXCONN)
    {
        return socketPtr->listen(maxConnection);
    }
    
    int acceptClient()
    {
        return socketPtr->accept();
    }
};
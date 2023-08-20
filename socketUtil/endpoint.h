#ifndef ENDPOINT_H
#define ENDPOINT_H

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
#include <util/template.h>

/// @Interface to create socket in a more friendly way while maintaining performant code
/// @tparam ENDPOINT - Server / client
/// @tparam TYPE - Network address type
/// @tparam PROTOCOL - Network protocol
/// @tparam MODE - Transport protocol
template <template <typename...> class ENDPOINT,
          typename DOMAIN,
          typename TRANSPORT,
          typename PROTOCOL>
struct SocketEndpoint
{
    using socket_address = typename DOMAIN::socket_address;
    using socket_address_ptr = socket_address *;
    using address_size = socklen_t;
    using address_size_ptr = socklen_t *;

    SocketEndpoint(int bufferSize)
    {
        socketPtr = std::make_unique<Socket>(bufferSize, typename DOMAIN::Tag{});
    }

    template <typename OPTION_LEVEL>
    [[nodiscard]] int configure(int parameter, int value);

    const BytesBuffer &receive(socket_address_ptr clientAddressPtr, address_size_ptr addressSizePtr)
    {
        return socketPtr->receive(clientAddressPtr, addressSizePtr);
    }

    [[nodiscard]] int send(const BytesBuffer &message, const socket_address_ptr client, const address_size &size)
    {
        return socketPtr->send(message, client, size);
    }

protected:
    struct Socket
    {
        /// @brief Tag dispatch to setup different type of socket address
        /// @param bufferSize 
        /// @param  
        Socket(const int bufferSize, Domain::IPV4::Tag)
            : m_address{.sin_family = DOMAIN::value}, m_addressLength{sizeof(m_address)}
        {
            m_self = socket(DOMAIN::value, TRANSPORT::value, PROTOCOL::value);
            if (m_self == -1)
                throw CannotCreateSocketException{};
            m_bufferPtr = std::make_unique<BytesBuffer>(bufferSize);
        }

        Socket(const int bufferSize, Domain::IPV6::Tag)
            : m_address{.sin6_family = DOMAIN::value}, m_addressLength{sizeof(m_address)}
        {
            m_self = socket(DOMAIN::value, TRANSPORT::value, PROTOCOL::value);
            if (m_self == -1)
                throw CannotCreateSocketException{};
            m_bufferPtr = std::make_unique<BytesBuffer>(bufferSize);
        }

        ~Socket() noexcept;

        template <typename OPTION_LEVEL>
        [[nodiscard]] int configure(int parameter, int value)
        {
            static_assert(std::is_base_of<OptionLevel<OPTION_LEVEL>, OPTION_LEVEL>::value, "Option level is not supported");
            int status = setsockopt(m_self, OPTION_LEVEL::value, parameter, (char *)&value, sizeof(value));

            return status;
        }

        /// @brief Enforce owning a V4 address
        /// @param ipAddress
        /// @param port
        /// @return
        [[nodiscard]] int binds(const IPV4_ADDRESS &&ipAddress, IPPORT_TYPE port)
        {
            static_assert(std::is_same<DOMAIN, Domain::IPV4>::value, "Cannot use IPV4 address for non-IPV4 domain");
            static_assert(std::is_same<PROTOCOL, Protocol::IP>::value || std::is_same<PROTOCOL, Protocol::UDP>::value, "Socket is not created with IP address.");
            m_address.sin_port = htons(port),
            m_address.sin_addr.s_addr = static_cast<uint32_t>(ipAddress);

            int status = bind(m_self, reinterpret_cast<sockaddr *>(&m_address), sizeof(m_address));
            return status;
        }

        /// @brief Enforce owning a V6 address
        /// @param ipAddress
        /// @param port
        /// @return
        [[nodiscard]] int binds(const IPV6_ADDRESS &&ipAddress, IPPORT_TYPE port)
        {
            static_assert(std::is_same<DOMAIN, Domain::IPV6>::value, "Cannot use IPV6 address for non-IPV6 domain");
            static_assert(std::is_same<PROTOCOL, Protocol::IP>::value || std::is_same<PROTOCOL, Protocol::UDP>::value, "Socket is not created with IP address.");
            m_address.sin6_port = htons(port),
            m_address.sin6_addr = ipAddress.m_address.sin6_addr;

            int status = bind(m_self, reinterpret_cast<sockaddr *>(&m_address), sizeof(m_address));
            return status;
        }

        [[nodiscard]] int connect(const IPV4_ADDRESS &&ipAddress, IPPORT_TYPE port)
        {
            m_address.sin_port = htons(port),
            m_address.sin_addr.s_addr = static_cast<uint32_t>(ipAddress);

            int status = ::connect(m_self, reinterpret_cast<sockaddr *>(&m_address), sizeof(m_address));
            return status;
        }

        [[nodiscard]] int connect(const IPV6_ADDRESS &&ipAddress, IPPORT_TYPE port)
        {
            m_address.sin6_port = htons(port),
            m_address.sin6_addr = ipAddress.m_address.sin6_addr;

            int status = ::connect(m_self, reinterpret_cast<sockaddr *>(&m_address), sizeof(m_address));
            return status;
        }

        [[nodiscard]] int listen(int maxConnection)
        {
            static_assert(std::is_same<TRANSPORT, Transport::UDP>::value == false, "UDP does not listen for connection.");
            m_clients.resize(maxConnection);
            return ::listen(m_self, maxConnection);
        }

        [[nodiscard]] int accept()
        {
            static_assert(std::is_same<TRANSPORT, Transport::UDP>::value == false, "UDP does not accept connection");
            int acceptedId = ::accept(m_self, reinterpret_cast<sockaddr *>(&m_address), &m_addressLength);
            m_clients.push_back(acceptedId);

            return acceptedId;
        }

        [[nodiscard]] const BytesBuffer &receive()
        {
            return receive(m_self);
        }

        [[nodiscard]] const BytesBuffer &receive(const int clientId)
        {
            int response = ::read(clientId,
                                  m_bufferPtr->message(),
                                  m_bufferPtr->size());
            if (response <= 0)
            {
                m_bufferPtr->reset();
            }
            return *m_bufferPtr;
        }

        [[nodiscard]] const BytesBuffer &receive(socket_address_ptr clientAddress, address_size_ptr addressSize)
        {
            int response = ::recvfrom(m_self,
                                      reinterpret_cast<void *>(m_bufferPtr->message()),
                                      m_bufferPtr->size(),
                                      0,
                                      reinterpret_cast<sockaddr *>(clientAddress),
                                      addressSize);
            if (response <= 0)
            {
                m_bufferPtr->reset();
            }
            return *m_bufferPtr;
        }

        [[nodiscard]] int send(const BytesBuffer &message)
        {
            return send(m_self, message);
        }

        [[nodiscard]] int send(int clientId, const BytesBuffer &message)
        {
            return ::send(clientId, reinterpret_cast<const void *>(message.message()), strlen(message.message()), 0);
        }

        [[nodiscard]] int send(const BytesBuffer &message, const socket_address_ptr client, const address_size &size)
        {
            return ::sendto(m_self, reinterpret_cast<const void *>(message.message()), strlen(message.message()), 0,
                            reinterpret_cast<const sockaddr *>(client), size);
        }

    private:
        int m_self{};
        std::unique_ptr<BytesBuffer> m_bufferPtr{};
        std::vector<int> m_clients{};
        socklen_t m_addressLength{};
        struct DOMAIN::socket_address m_address
        {
        };
    };

protected:
    std::unique_ptr<Socket> socketPtr{};

    static_assert(std::is_base_of<NetworkDomain<DOMAIN>, DOMAIN>::value, "Network type is invalid.");
    static_assert(std::is_base_of<TransportProtocol<TRANSPORT>, TRANSPORT>::value, "Transport mode is invalid.");
    static_assert(std::is_base_of<NetworkProtocol<PROTOCOL>, PROTOCOL>::value, "Network protocol is invalid.");
};

template <template <typename...> class ENDPOINT,
          typename DOMAIN,
          typename TRANSPORT,
          typename PROTOCOL>
SocketEndpoint<ENDPOINT, DOMAIN, TRANSPORT, PROTOCOL>::Socket::~Socket() noexcept
{
    // shutdown routine
    for (auto &client : m_clients)
        close(client);
    m_clients.clear();
    ::shutdown(m_self, SHUT_RDWR);
    printf("Socket is shutting down...\n");
}

template <template <typename...> class ENDPOINT,
          typename DOMAIN,
          typename TRANSPORT,
          typename PROTOCOL>
template <typename OPTION_LEVEL>
int SocketEndpoint<ENDPOINT, DOMAIN, TRANSPORT, PROTOCOL>::configure(int parameter, int value)
{
    return socketPtr->template configure<OPTION_LEVEL>(parameter, value);
}

#endif
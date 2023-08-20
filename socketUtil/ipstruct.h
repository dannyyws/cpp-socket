#ifndef IP_STRUCT_H
#define IP_STRUCT_h

#include <arpa/inet.h>
#include <bits/types.h>
#include <type_traits>

#include "config.h"

struct IPV6_ADDRESS
{
public:
    IPV6_ADDRESS(const char (&addr)[])
    {
        if (inet_pton(Domain::IPV6::value, addr, &m_address.sin6_addr) <= 0)
        {
            perror("Invalid IPV6 addresss: ");
            perror(addr);
            exit(EXIT_FAILURE);
        }
    }

    IPV6_ADDRESS(const IPV6_ADDRESS &rhs) = delete;
    IPV6_ADDRESS &operator=(const IPV6_ADDRESS &rhs) = delete;

    Domain::IPV6::socket_address m_address{};
};

struct IPV4_ADDRESS
{
    using octet_type = unsigned char;
    octet_type m_octets[4]{};

    // Don't need to waste space
    IPV4_ADDRESS(const IPV4_ADDRESS &rhs) = delete;
    IPV4_ADDRESS &operator=(const IPV4_ADDRESS &rhs) = delete;
    // IPV4_ADDRESS(const IPV4_ADDRESS&& rhs) = delete;
    // IPV4_ADDRESS& operator= (const IPV4_ADDRESS&& rhs) = delete;

    operator uint32_t() const
    {
        return *((unsigned int *)(&m_octets[0]));
    }

    IPV4_ADDRESS(octet_type oct0, octet_type oct1, octet_type oct2, octet_type oct3)
    {
        m_octets[0] = oct0;
        m_octets[1] = oct1;
        m_octets[2] = oct2;
        m_octets[3] = oct3;
    }

private:
    IPV4_ADDRESS(){};
};

using IPPORT_TYPE = uint16_t;

#endif
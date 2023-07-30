#include <bits/types.h>
#include <type_traits>
#include <netinet/in.h>


struct IPV6_ADDRESS
{
    // using hexa_type = uint
};

struct IPV4_ADDRESS
{
    using octet_type = unsigned char;
    octet_type m_octets[4]{};

    //Don't need to waste space
    IPV4_ADDRESS(const IPV4_ADDRESS& rhs) = delete;
    IPV4_ADDRESS& operator= (const IPV4_ADDRESS& rhs) = delete;
    IPV4_ADDRESS(const IPV4_ADDRESS&& rhs) = delete;
    IPV4_ADDRESS& operator= (const IPV4_ADDRESS&& rhs) = delete;

    operator uint32_t() const
    {
        return *((unsigned int*)(&m_octets[0]));
    }

    IPV4_ADDRESS(octet_type oct0,octet_type oct1,octet_type oct2,octet_type oct3)
    {
        m_octets[0] = oct0;
        m_octets[1] = oct1;
        m_octets[2] = oct2;
        m_octets[3] = oct3;
    }
private:
    IPV4_ADDRESS(){};
};

using IPVX_PORT=uint16_t;
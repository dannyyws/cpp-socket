#include <socketUtil/endpoint.h>


template<typename... TRANSPORT>
struct Server: SocketEndpoint<Server, TRANSPORT...> 
{
    using SocketEndpoint<Server, TRANSPORT...>::socketPtr;

    [[nodiscard]] 
    int bindPort(const IPV4_ADDRESS &ipAddress, IPVX_PORT port)
    {
        return socketPtr->binds(ipAddress, port);
    }

    [[nodiscard]]
    int bindPort(const IPV6_ADDRESS& ipAddress, IPVX_PORT port){ throw NotImplementedException{"Bind IPV6 Port."};}

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
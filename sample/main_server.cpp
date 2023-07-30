// This library provides template meta-helper to allow type-rich, safer development
// - All generic integer parameters are encapsulated in user-defined types
// - Wrong substition of domain, transport or protocol will failed in compilation because they are essentially a variant of their own
// - All substitution is compile-level work which incur zero-runtime cost

#include <socketUtil/endpoint.h>

void testTCPIPServer()
{
    using Server = Server<Domain::IPV4, Transport::TCP, Protocol::IP>;
    Server server{1024};
    if (server.configure<Option::SocketOption>(Option::SocketOption::EnableReuseAddress, 1)) {
        printf("Detected: cannot configure socket properly\n");
    }
    if (server.bindPort(IPV4_ADDRESS{127,0,0,1}, 8080) != 0) {
        printf("Detected: cannot bind to port\n");
    }
    if (server.startListen(3) != 0) {
        printf("Detected: cannot listen to port\n");
    }
    if (server.acceptClient() == -1) {
        printf("Detected: fail to accept incoming connection\n");
    }
    while(true) {
        sleep(1);
    }
}

void testUDPServer()
{
    using Server = Server<Domain::IPV4, Transport::UDP, Protocol::IP>;
    Server server{1024};
    if (server.configure<Option::SocketOption>(Option::SocketOption::EnableReuseAddress, 1)) {
        printf("Detected: cannot configure socket properly\n");
    }
    if (server.bindPort(IPV4_ADDRESS{127,0,0,1}, 8080) != 0) {
        printf("Detected: cannot bind to port\n");
    }
    while(true) {
        sleep(1);
    }
}

int main(int argc, char const *argv[])
{
    // testTCPIPServer();
    testUDPServer();

    return 0;
}

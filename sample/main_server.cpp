// This library provides template meta-helper to allow type-rich, safer development
// - All generic integer parameters are encapsulated in user-defined types
// - Wrong substition of domain, transport or protocol will failed in compilation because they are essentially a variant of their own
// - All substitution is compile-level work which incur zero-runtime cost

#include <socketUtil/server.h>

void testTCPIPServer()
{
    using Server = Server<Domain::IPV4, Transport::TCP, Protocol::IP>;
    Server server{1024};
    int socket = -1;
    if (server.configure<Option::SocketOption>(Option::SocketOption::EnableReuseAddress, 1)) {
        printf("Detected: cannot configure socket properly\n");
    }
    if (server.bindPort(IPV4_ADDRESS{127,0,0,1}, 8080) != 0) {
        printf("Detected: cannot bind to port\n");
    }
    if (server.startListen(3) != 0) {
        printf("Detected: cannot listen to port\n");
    }
    socket = server.acceptClient();
    if (socket < 0) {
        printf("Detected: fail to accept incoming connection\n");
    }
    // unfortunately for TCP connection protocol, we may need to scan all connected socket
    while(true)
    {
        // continuously receive hello for testing
        const BytesBuffer& buffer = server.receive(socket);
        if (strlen(buffer.message()) > 0)
        {
            printf("Received [%d]Bytes: %s\n", (int)strlen(buffer.message()), buffer.message());
        }
        else
        {
            printf("Socket [%d] is disconnected.\n", socket);
            break;
        }
    }
}

void testUDPServer()
{
    using Server = Server<Domain::IPV4, Transport::UDP, Protocol::IP>;
    Server server{1024};
    if (server.configure<Option::SocketOption>(Option::SocketOption::EnableReuseAddress, 1)) {
        printf("Detected: cannot configure socket properly\n");
    }
    if (server.configure<Option::UdpOption>(Option::UdpOption::EnableCork, 1)) {
        printf("Detected: cannot configure socket properly\n");
    }
    if (server.bindPort(IPV4_ADDRESS{127,0,0,1}, 8080) != 0) {
        printf("Detected: cannot bind to port\n");
    }
    while(true)
    {
        const BytesBuffer& buffer = server.receive();
        if (strlen(buffer.message()) > 0)
            printf("Received [%d]Bytes: %s\n", (int)strlen(buffer.message()), buffer.message());
        else
        {
            printf("Socket [%d] is disconnected.\n", socket);
            break;
        }
    }
}

int main(int argc, char const *argv[])
{
    // testTCPIPServer();
    testUDPServer();

    return 0;
}

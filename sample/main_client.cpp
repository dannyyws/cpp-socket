// This library provides template meta-helper to allow type-rich, safer development
// - All generic integer parameters are encapsulated in user-defined types
// - Wrong substition of domain, transport or protocol will failed in compilation because they are essentially a variant of their own
// - All substitution is compile-level work which incur zero-runtime cost

#include <socketUtil/client.h>

void testTCPIPClient()
{
    using Client = Client<Domain::IPV4, Transport::TCP, Protocol::IP>;
    Client client{1024};
    while (client.connectServer(IPV4_ADDRESS{127,0,0,1}, 8080) != 0) {
        printf("Detected: cannot connect to port\n");
        sleep(2);
    }
    BytesBuffer bytesBuffer(1024);
    strcpy(bytesBuffer.message(), "Hello from client\0");
    // continuously send hello for testing
    for(int i = 0; i < 10; ++i)
    {
        if (client.send(bytesBuffer) == -1)
        {
            printf("Failed to send message across network.\n");
        }
        usleep(500000);
    }
}

void testUDPClient()
{
    using Client = Client<Domain::IPV4, Transport::UDP, Protocol::IP>;
    Client client{1024};
    while (client.connectServer(IPV4_ADDRESS{127,0,0,1}, 8080) != 0) {
        printf("Detected: cannot connect to port\n");
        sleep(2);
    }
    BytesBuffer bytesBuffer(1024);
    strcpy(bytesBuffer.message(), "Hello from client\0");
    // client.send(bytesBuffer);
    if (client.send(bytesBuffer) == -1)
    {
        printf("Failed to send message across network.");
    }
    sleep(2);
}

int main(int argc, char const *argv[])
{
    // testTCPIPClient();
    testUDPClient();

    return 0;
}

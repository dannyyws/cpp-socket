#include <socketUtil/client.h>
#include <socketUtil/server.h>
#include <socketUtil/config.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <atomic>
#include <cassert>
#include <mutex>
#include <thread>
#include <condition_variable>

//#define USE_IPV4
#define USE_IPV6

struct shared_testbed
{
public:
    void write(const char* serverData)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_isDataCompared.wait(lock, [&](){ return m_bufSize == 0;});
        m_bufSize = strlen(serverData);
        memcpy(m_buffer, serverData, m_bufSize);
        std::atomic_thread_fence(std::memory_order_release);
        m_isDataAvailable.notify_one();
    }

    bool readOk(const char* clientData)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_isDataAvailable.wait(lock, [&](){ return m_bufSize != 0;});
        bool match = memcmp(m_buffer, clientData, m_bufSize) == 0;
        m_bufSize = 0;
        std::atomic_thread_fence(std::memory_order_acquire);
        m_isDataCompared.notify_one();

        return match;
    }

private:
    std::mutex m_mutex{};
    std::condition_variable m_isDataAvailable{};
    std::condition_variable m_isDataCompared{};

    char m_buffer[1024];
    size_t m_bufSize{};
};

const char* SHOBJ = "shared-testbed-object";
const char* TEST_QWERTY = "QWERTYUIOP\0";
const char* CLOSE_INST = "clllooooseee";
namespace boostipc = boost::interprocess;

int main(int argc, char const *argv[])
{
    BytesBuffer bytesBuffer(1024);

    if (argc == 1)
    {
        struct shmemRemover
        {
            shmemRemover() { boostipc::shared_memory_object::remove(SHOBJ); }
            ~shmemRemover() { boostipc::shared_memory_object::remove(SHOBJ); }
        } remover;
        
        auto sharedObj = boostipc::shared_memory_object(boostipc::create_only, SHOBJ, boostipc::read_write);
        sharedObj.truncate(sizeof(shared_testbed));
        boostipc::mapped_region region (sharedObj, boostipc::read_write);
        void* ptr = region.get_address();
        shared_testbed* testbed = new (ptr) shared_testbed{};

#ifdef USE_IPV4
        using Server = Server<Domain::IPV4, Transport::TCP, Protocol::IP>;
#endif
#ifdef USE_IPV6
        using Server = Server<Domain::IPV6, Transport::TCP, Protocol::IP>;
#endif
        Server server{1024};
        if (server.configure<Option::SocketOption>(Option::SocketOption::EnableKeepAlive, 1) == -1)
            printf("Failed to set keep alive for server\n");

#ifdef USE_IPV4
        if (server.bindPort(IPV4_ADDRESS{127,0,0,1}, 8080) != 0)
#endif
#ifdef USE_IPV6
        if (server.bindPort(IPV6_ADDRESS{"::1"}, 8080))
#endif
        {
            printf("Failed to bind server to port.\n");
            return 1;
        }

        if (server.startListen(5) == -1)
            printf("Server has failed to listen port.\n");
        
        int clientId = server.acceptClient();
        while(clientId == -1) {
            printf("Server failed to host any client socket \n");
            clientId = server.acceptClient();
            sleep(2);
        }
        testbed->write(TEST_QWERTY);
        strcpy(bytesBuffer.message(), TEST_QWERTY);
        if (server.send(clientId, bytesBuffer) == -1)
            printf("Server failed to send message\n");
    }
    else if (strcmp(argv[1], "clear") == 0)
    {
        struct shmemRemover
        {
            shmemRemover() { boostipc::shared_memory_object::remove(SHOBJ); }
            ~shmemRemover() { boostipc::shared_memory_object::remove(SHOBJ); }
        } remover;
    }
    else
    {
        boostipc::permissions perm {};
        perm.set_unrestricted();
        auto sharedObj = boostipc::shared_memory_object(boostipc::open_only, SHOBJ, boostipc::read_write);
        boostipc::mapped_region region (sharedObj, boostipc::read_write);
        void* ptr = region.get_address();
        shared_testbed* testbed = static_cast<shared_testbed*>(ptr);

#ifdef USE_IPV4
        using Client = Client<Domain::IPV4, Transport::TCP, Protocol::IP>;
#endif
#ifdef USE_IPV6
        using Client = Client<Domain::IPV6, Transport::TCP, Protocol::IP>;
#endif
        Client client{1024};
#ifdef USE_IPV4
        if (client.connectServer(IPV4_ADDRESS{127,0,0,1}, 8080) != 0)
#endif
#ifdef USE_IPV6
        if (client.connectServer(IPV6_ADDRESS{"::1"}, 8080) != 0)
#endif
        {
            printf("Detected: cannot connect to port\n");
            return 1;
        }
        const BytesBuffer& buffer = client.receive();
        if (testbed->readOk(buffer.message()) == false)
            printf("Socket message mismatched\n");
        else
            printf("%s [MATCHED]\n", buffer.message());
    }
    return 0;
}

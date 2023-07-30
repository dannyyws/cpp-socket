#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    // Bind the socket to the address and port
    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1 in network byte order
    serverAddr.sin_port = htons(8080); // Port 8080 in network byte order

    if (bind(udpSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::cerr << "Failed to bind socket." << std::endl;
        close(udpSocket);
        return 1;
    }

    // Server setup complete, now listen for incoming messages
    std::cout << "UDP server listening on 127.0.0.1:8080..." << std::endl;

    char buffer[1024];
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    while (true) {
        // Receive data from client
        int bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer) - 1, 0,
                                reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);

        if (bytesRead == -1) {
            std::cerr << "Error receiving data." << std::endl;
            continue;
        }

        // Add null terminator to received data
        buffer[bytesRead] = '\0';

        // Process the received data (in this case, simply print it)
        std::cout << "Received from " << inet_ntoa(clientAddr.sin_addr) << ": " << buffer << std::endl;
    }

    // Close the socket
    close(udpSocket);

    return 0;
}
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <pthread.h>
#include <thread>
#include <vector>
#include <map>

struct ClientInfo {
    int port;
    std::string name;
};

std::map<int, ClientInfo> activeClients;

void sendActiveUsers(int clientSocket) {
    std::string activeUsers = "Active Users:\n";
    for (const auto& entry : activeClients) {
        activeUsers += entry.second.name + " (Port " + std::to_string(entry.second.port) + ")\n";
    }
    send(clientSocket, activeUsers.c_str(), activeUsers.size(), 0);
}

void handleClient(int clientSocket) {
    char buffer[1024];
    int bytesReceived;

    // Receive client information (port and name)
    bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0) {
        std::cerr << "Client disconnected" << std::endl;
        close(clientSocket);
        return;
    }
    buffer[bytesReceived] = '\0';

    int clientPort;
    char clientName[20];
    if (sscanf(buffer, "%d:%s", &clientPort, clientName) != 2) {
        std::cerr << "Invalid client information" << std::endl;
        close(clientSocket);
        return;
    }

    ClientInfo clientInfo;
    clientInfo.port = clientPort;
    clientInfo.name = clientName;
    std::cerr << "Client Connected: " << clientName << " (Port " << clientPort << ")" << std::endl;

    // Add the client to the active clients list
    activeClients[clientSocket] = clientInfo;

    while (true) {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Client disconnected: " << clientInfo.name << " (Port " << clientInfo.port << ")" << std::endl;
            // Remove the client from the active clients list
            activeClients.erase(clientSocket);
            break;
        }
        buffer[bytesReceived] = '\0';

        // Check if the received message is a command
        if (strcmp(buffer, "getActiveUsers") == 0) {
            sendActiveUsers(clientSocket);
        } else {
            std::cout << "Received from " << clientInfo.name << " (Port " << clientInfo.port << "): " << buffer << std::endl;
        }
    }

    close(clientSocket);
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int k = 0;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "*****CHAT ROOM CREATED SUCCESFULLY*****\n";
    while (true) {
        int new_socket;
        struct sockaddr_in clientAddr;
        int addrlen = sizeof(clientAddr);

        if ((new_socket = accept(server_fd, (struct sockaddr *)&clientAddr, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        std::thread clientThread(handleClient, new_socket);
        clientThread.detach();
    }

    return 0;
}

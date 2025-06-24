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
#include <string>
#include <regex>


char name[20];
int PORT;
std::vector<int> activePorts;
std::map<std::string, int> userPortMap;

void multisending();
void receiving(int server_fd);
void *receive_thread(void *server_fd);
void sending(std::string user);

void receiveMessages(int clientSocket) {
    char buffer[1024];
    int bytesReceived;
    while (true) {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Server disconnected" << std::endl;
            return;
        }
        buffer[bytesReceived] = '\0';
        std::cout << buffer;
        userPortMap.clear();

    std::regex portRegex("\\(Port (\\d+)\\)");

    // Create a vector to store the extracted port numbers
    std::string input;
    input.assign(buffer);
    std::istringstream iss(input);
    std::string line;
    std::regex userPortRegex("\\s*([^\\(]+)\\s*\\(Port\\s*(\\d+)\\)");
    while (std::getline(iss, line)) {
        // Search for matches using std::regex_search for each line
        std::smatch match;
        if (std::regex_search(line, match, userPortRegex)) {
            if (match.size() == 3) {
                std::string userName = match[1];
                userName.pop_back();
                int portNumber = std::stoi(match[2]);
                userPortMap[userName] = portNumber;
            }
        }
    }
}
}
void getActiveUsers(int serverSocket) {
    char buffer[1024];
    int bytesReceived;

    // Request the list of active users from the server
    std::string request = "getActiveUsers";
    send(serverSocket, request.c_str(), request.size(), 0);

    // Receive and parse the list of active ports from the server
}

int main(int argc, char const *argv[]) {
    std::cout << ">Enter name:";
    std::cin >> name;

    std::cout << ">Enter your port number:";
    std::cin >> PORT;

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
    address.sin_port = htons(PORT);

    // // Printed the server socket addr and port
    // std::cout << "IP address is: " << inet_ntoa(address.sin_addr) << std::endl;
    // std::cout << "port is: " << ntohs(address.sin_port) << std::endl;

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    int clientSocket;
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating client socket" << std::endl;
        return -1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080); // Change the port to match the server
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change the IP address to match the server

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to server" << std::endl;
        return -1;
    }

    std::cout << ">Connected to server" << std::endl;
    std::thread receiveThread(receiveMessages, clientSocket);
    receiveThread.detach();
    std::string message = std::to_string(PORT) + ":" + name; // Send port and name as a string
    send(clientSocket, message.c_str(), message.size(), 0);
    int ch;
    pthread_t tid;
    pthread_create(&tid, NULL, &receive_thread, &server_fd); // Creating thread to keep receiving message in real-time

    std::cout << "\n*****Press any of the following:*****\n3 ==> Broadcast\n2 ==> Get Current Active Users\n1 ==> Send message\n0 ==> Quit\n";
    std::cout << "\n>Enter choice:\n>";

    do {
        std::cin >> ch;
        std::string user;
        switch (ch) {
            case 3:
                getActiveUsers(clientSocket);
                sleep(1);
                multisending();
                break;
            case 2:
                getActiveUsers(clientSocket);
                sleep(1);
                std::cout<<"\n>Enter choice:\n>";
                break;
            case 1:
                getActiveUsers(clientSocket);
                sleep(1);
                std::cout << ">Enter UserName to send Private message: ";
                std::getline(std::cin >> std::ws,user);
                sending(user);
                break;
            case 0:
                std::cout << "\n>Leaving\n";
                break;
            default:
                std::cout << ">Wrong choice\n";
                std::cout<<"\n>Enter choice:\n>";
        }
    } while (ch);

    close(server_fd);
    return 0;
}

// Sending messages to port
void multisending() {
    char buffer[2000] = {0};

    if (userPortMap.empty()) {
        std::cout << "No active users to broadcast to." << std::endl;
        return;
    }

    char dummy[200];
    std::cout << ">Enter your message:";
    std::cin.ignore(); // Ignore newline character
    std::cin.getline(dummy, sizeof(dummy));

    for (auto port : userPortMap) {
        if (port.second == PORT) {
            // Skip sending to oneself
            continue;
        }

        int sock = 0;
        struct sockaddr_in serv_addr;

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cout << "\n Socket creation error \n";
            return;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY always gives an IP of 0.0.0.0
        serv_addr.sin_port = htons(port.second);

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cout << ">Connection to port " << port.second << " Failed \n";
            close(sock);
            continue;
        }

        snprintf(buffer, sizeof(buffer), "%s[Port:%d] Broadcasted: %s", name, PORT, dummy);
        send(sock, buffer, sizeof(buffer), 0);
        std::cout << ">Message sent to " << port.first << "[Port:" << port.second <<"]" <<std::endl;
        close(sock);
    }
    std::cout<<"\n>Enter choice:\n>";
    

}

void sending(std::string user) {
    char buffer[2000] = {0};
    char mess[200] = {0};
    if(userPortMap.find(user)==userPortMap.end())
    {
        std::cout<<">Invalid User\n\n";
        std::cout<<">Enter choice:\n>";
        return;
    }
    int destPort = userPortMap[user];
    // IN PEER WE TRUST
    std::cout << ">Enter your message:\n>";
    //std::cin.ignore(); // Ignore newline character
    std::cin.getline(mess, sizeof(mess));

    int sock = 0, valread;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "\n Socket creation error \n";
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY always gives an IP of 0.0.0.0
    serv_addr.sin_port = htons(destPort); // Use the destination port

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << ">Connection Failed \n\n";
        return;
    }

    snprintf(buffer, sizeof(buffer), "%s[PORT:%d] says: %s", name, PORT, mess);
    send(sock, buffer, sizeof(buffer), 0);
    std::cout << ">Message sent\n\n";
    close(sock);
    std::cout<<">Enter choice:\n>";

}

// Calling receiving every 2 seconds
void *receive_thread(void *server_fd) {
    int s_fd = *((int *)server_fd);
    while (1) {
        sleep(2);
        receiving(s_fd);
    }
}

// Receiving messages on our port
void receiving(int server_fd) {
    struct sockaddr_in address;
    int valread;
    char buffer[2000] = {0};
    int addrlen = sizeof(address);
    fd_set current_sockets, ready_sockets;

    // Initialize my current set
    FD_ZERO(&current_sockets);
    FD_SET(server_fd, &current_sockets);
    int k = 0;

    while (1) {
        k++;
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
            perror("Error");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &ready_sockets)) {
                if (i == server_fd) {
                    int client_socket;

                    if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    FD_SET(client_socket, &current_sockets);
                } else {
                    valread = recv(i, buffer, sizeof(buffer), 0);
                    std::string s= buffer;
                    s.push_back('\n');
                    std::cout << s+'>';
                    FD_CLR(i, &current_sockets);
                }
            }
        }

        if (k == (FD_SETSIZE * 2)) {
            break;
        }
    }
}


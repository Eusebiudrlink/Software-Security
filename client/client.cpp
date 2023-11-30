// ftp_client.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

void receiveFile(int serverSocket, const char* fileName) {
    int fileSize;
    recv(serverSocket, &fileSize, sizeof(fileSize), 0);

    ofstream file(fileName, ios::binary);

    char buffer[1024];
    int bytesRead;
    while (fileSize > 0) {
        bytesRead = recv(serverSocket, buffer, sizeof(buffer), 0);
        file.write(buffer, bytesRead);
        fileSize -= bytesRead;
    }

    file.close();
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "Error creating socket" << endl;
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, '0', sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        cerr << "Invalid address/ Address not supported" << endl;
        exit(EXIT_FAILURE);
    }

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Connection Failed" << endl;
        exit(EXIT_FAILURE);
    }

    char fileName[256];
    cout << "Enter file name: ";
    cin >> fileName;

    send(clientSocket, fileName, sizeof(fileName), 0);
    cout<<"Nume: "<<fileName<<" and"<<sizeof(fileName)<<endl;
    receiveFile(clientSocket, fileName);

    close(clientSocket);

    return 0;
}


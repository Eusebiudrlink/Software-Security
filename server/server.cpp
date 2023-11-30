// ftp_server.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>  // For inet_ntoa
#include <netinet/in.h> 

using namespace std;

void sendFile(int clientSocket, const char* fileName) {
    ifstream file(fileName, ios::binary);
    if (!file.is_open()) {
        cerr << "Error opening file: " << fileName << endl;
        return;
    }
    
	cout<<"File opened with succes"<<endl ;
	

    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.seekg(0, ios::beg);

    send(clientSocket, &fileSize, sizeof(fileSize), 0);

    char buffer[1024];
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        send(clientSocket, buffer, file.gcount(), 0);
    }

    file.close();
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Error creating socket" << endl;
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, '0', sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(8888);

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    listen(serverSocket, 5);

    cout << "FTP Server listening on port 8888..." << endl;

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        cout << "Connection accepted from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;

        char fileName[256];
        recv(clientSocket, fileName, sizeof(fileName), 0);
	cout<< "File name is:" <<fileName<<" Lungime:"<<sizeof(fileName)<<endl;
        sendFile(clientSocket, fileName);

        close(clientSocket);
    }

    return 0;
}


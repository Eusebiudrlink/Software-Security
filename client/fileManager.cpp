#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
using namespace std;

class FileManager {//client
private:
  


public:
    FileManager() {
        
    }

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
  
};


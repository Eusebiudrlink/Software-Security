#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
using namespace std;

class FileManager {
private:
  


public:
    FileManager() {
        
    }

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
  
};

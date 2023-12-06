#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
using namespace std;

class FileManager {
private:
  string typeOfTransfer="A";//I - binary and and A - ASCII

public:
    FileManager() {
        

    }

    void setType(int clientSocket,const std::string& type)
    {
        if(type=="A" || type=="I")
        {
            typeOfTransfer=type;  
            string response200 = "200 Type set to "+typeOfTransfer+"\r\n";
            
            send(clientSocket, response200.c_str(), response200.size(), 0); 
        }
      
        else{
            const char* response500 = "500 Syntax error, command unrecognized\r\n";
            send(clientSocket, response500, strlen(response500), 0);
            cout<<"Unrecognized type of transfer!"<<endl;
        }
    }
    void sendFile(int clientSocket,int passiveSocket, const char* fileName) {
        ifstream file(fileName, ios::binary);
        if (!file.is_open()) {
            cerr << "Error opening file: " << fileName << endl;
            const char* response550 = "550 File not found\r\n";
            send(clientSocket, response550, strlen(response550), 0);
            return;
        }
        
        const char* response150 = "150 Using BINARY mode data connection for filename\n";
        send(clientSocket, response150, strlen(response150), 0);
        cout<<"File opened with succes"<<endl ;

        file.seekg(0, ios::end);
        int fileSize = file.tellg();
        file.seekg(0, ios::beg);
        fileSize=htons(fileSize);
       int bytesSent =send(passiveSocket,&fileSize, sizeof(fileSize), 0);//passiv
         
        cout<<"SERVER: bytes sent to client(filesize): "<<bytesSent<<endl;

        char buffer[1024];
        // while (!file.eof()) {
        //     file.read(buffer, sizeof(buffer));
        //     send(passiveSocket, buffer, file.gcount(), 0);//passiv
        // }
        const char* response226 = "226 Transfer complete\n";
        send(clientSocket, response226, strlen(response226), 0);
        cout<<"File sent from server "<<fileName<<endl;

        file.close();
    }
  
};

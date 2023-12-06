#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
using namespace std;

class FileManager {//client
private:
 //test sebi 
  string typeOfTransfer="A";//I - binary and and A - ASCII

public:
    FileManager() {
        
    }
    void setType(const std::string& type)
    {
        if(type=="A" || type=="I")
        {
            typeOfTransfer=type;          
        }

    }

    void receiveFile(int serverSocket, const char* fileName) {
        int fileSize;
        cout<<"Receive file funct open"<<endl;
        int bytesRead = recv(serverSocket, &fileSize, sizeof(fileSize), 0);
            /* (bytesRead <= 0) {
                perror("Eroare la primirea răspunsului de la server.");
                close(clientSocket);
                return -1;
            }*/
        fileSize=ntohs(fileSize);
        if (bytesRead < 0) 
            {
                // Handle error using errno
                perror("Error while receiving data from the server");
                close(serverSocket);
               
            } 
        cout<<"NR bytes cititii este:"<<bytesRead<<endl;
        cout<<"Filesize ul este:"<<fileSize<<endl;
        ofstream file(fileName, ios::binary);

        // char buffer[1024];
      
        // while (fileSize > 0) {
        //     bytesRead = recv(serverSocket, buffer, sizeof(buffer), 0);
        //     file.write(buffer, bytesRead);
        //     fileSize -= bytesRead;
        // }
        cout<<"S a finalizat primirea!"<<endl;
        file.close();
}
  
};


#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
using namespace std;

class FileManager {
private:
  string typeOfTransfer="I";

public:
    FileManager() {}
    
    std::string convertToCRLF(const std::string& data) {
        // Replace '\n' with '\r\n'
        std::string convertedData = data;
        size_t pos = 0;
            while ((pos = convertedData.find('\n', pos)) != std::string::npos) {
                convertedData.replace(pos, 1, "\r\n");
                pos += 2; // Move past the replaced '\r\n'
            }
        return convertedData;
    }

    std::string convertToLF(const std::string& data) {
        // Replace '\r\n' with '\n'
        std::string convertedData = data;
        size_t pos = 0;
            while ((pos = convertedData.find("\r\n", pos)) != std::string::npos) {
                convertedData.replace(pos, 1, "\n");
                pos += 1; // Move past the replaced '\n'
            }
        return convertedData;
    }
    
    void setType(int clientSocket,const std::string& type)
    {
        if(type=="A" || type=="I")
        {
            typeOfTransfer=type;  
            string response200 = "200 Transfer type set to "+typeOfTransfer+"\r\n";
            
            send(clientSocket, response200.c_str(), response200.size(), 0); 
        }
      
        else{
            const char* response500 = "500 Syntax error, command unrecognized\r\n";
            send(clientSocket, response500, strlen(response500), 0);
            cout<<"Unrecognized type of transfer!"<<endl;
        }
    }
    string getTypeOfTransfer() {
        return typeOfTransfer;
    }
    void sendFile(int clientSocket,int passiveSocket, const char* fileName) {
        // Open the file.
        ifstream file(fileName, ios::binary);
        if (!file.is_open()) {
            cerr << "Error opening file: " << fileName << endl;
            const char* response550 = "550 File not found\r\n";
            send(clientSocket, response550, strlen(response550), 0);
            return;
        }
        
        const string response150 = "150 Using "+typeOfTransfer+" mode data connection for " + string(fileName) + "\n";
        send(clientSocket, response150.c_str(), response150.size(), 0);

        cout<<"File opened with succes!"<<endl ;

        // Read and send the file content in chunks
        const int bufferSize = 1024;
        char buffer[bufferSize];
    
        while (!file.eof()) 
        {
            file.read(buffer, bufferSize);
            int bytesRead = file.gcount();
            int bytesSent;
            if(typeOfTransfer == "A") {
                
                std::string asciiData = convertToCRLF(buffer);
                bytesSent = send(passiveSocket, asciiData.c_str(), asciiData.size(), 0);
            }
            else {
              // Send the data to the client using the data socket
                bytesSent=send(passiveSocket, buffer, bytesRead, 0);  
            }
            
            if (bytesSent < 0) {
            std::cerr << "Error sending data to server\n";
            break;
            }
            
            cout<<"Sent buffer:"<<buffer<<" with "<<bytesSent<<" bytesSent\n";
        }
        
        // Close the file and the data connection
        file.close();

        // Send an FTP response indicating successful file transfer
        const char* response226 = "226 File transfer successful.\r\n";
        send(clientSocket, response226, strlen(response226), 0);
        cout<<"Sent response226\n";       
        
    }
    
    void receiveFile(int clientSocket,int dataSocket, const char* fileName) {
    // Open the file for writing in binary mode
    // File is created in the currently working directory which is already set
    std::ofstream file(fileName, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << fileName << std::endl;
        return;
    }

    // Read data from the data connection and write it to the file
    const int bufferSize = 1024;
    char buffer[bufferSize];

    while (true) {
        int bytesRead = recv(dataSocket, buffer, bufferSize, 0);
        cout<<"Received bytes: "<<bytesRead<<endl;
        if (bytesRead < 0) {
            std::cerr << "Error receiving data from server\n";
            break;
        }

        if (bytesRead == 0) { // the socket connection is closed
            // End of file
            break;
        }

        if(typeOfTransfer == "A") {
            // Handle ASCII mode spceific for the Server OS (Linux)
            std::string asciiData = convertToLF(buffer);
            
            // Write the ASCII-transformed data to the file
            file.write(asciiData.c_str(), asciiData.size());
        }
        else {
        // Binary mode: write the original binary data to the file
        file.write(buffer, bytesRead);
        }
    }
    
    

    // Close the file and the data connection
    file.close();
    

    const int responseMaxSize = 1024;
    char response[responseMaxSize];
    
    // Send an FTP response indicating successful file transfer
    const char* response226 = "226 File transfer successful.\r\n";
    send(clientSocket, response226, strlen(response226), 0);
    cout<<"Sent response226\n"; 
}

  
};

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
using namespace std;

class FileManager {
private: 
  string typeOfTransfer="I"; // default is binary

public:
    FileManager() {
        
    }
    //I - binary and and A - ASCII
    void setType(const std::string& type)
    {
        if(type=="A" || type=="I")
        {
            typeOfTransfer=type;          
        }

    }
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
                convertedData.replace(pos, 2, "\n");
                pos += 1; // Move past the replaced '\n'
            }
        return convertedData;
    }
    string getTypeOfTransfer() {
       return typeOfTransfer;
    }

    void receiveFile(int clientSocket,int dataSocket, const char* fileName) {
    // Open the file for writing in binary mode
    std::ofstream file(fileName, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << fileName << std::endl;
        return;
    }
    cout<<"I opened file "<<fileName<<" for writing"<<endl;
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

        if (bytesRead == 0) {
            // End of file
            break;
        }
        
        if(typeOfTransfer == "A") 
        {
            // Handle ASCII mode spceific for the Server OS (Linux/Windows) or the TYPE asked
            std::string asciiData = convertToCRLF(buffer);
            
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
        
    // for the 226 response recieve is on the clientSocket
    int responseBytesRead = recv(clientSocket, response, responseMaxSize, 0);
    if (responseBytesRead <= 0) {
        perror("Eroare la primirea răspunsului de la server.");
        close(clientSocket);
        return;
    }
    response[responseBytesRead] = '\0';
    // Afișează răspunsul de la server
    std::cout << "Răspuns de la server: " << response << std::endl;
    std::cout << "File saved as: " << fileName << std::endl;
}

void sendFile(int clientSocket, int dataSocket, const char* fileName) {
    // Open the file for reading in binary mode
    std::ifstream file(fileName, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        close(dataSocket);
        return;
    }

    // Read and send the file content in chunks
    const int bufferSize = 1024;
    char buffer[bufferSize];

    
    while (!file.eof()) {
        file.read(buffer, bufferSize);
        int bytesRead = file.gcount();
        int bytesSent;
        if(typeOfTransfer == "A") {
            std::string lfData = convertToLF(buffer);
            std::string asciiData = convertToCRLF(lfData);
            bytesSent = send(dataSocket, asciiData.c_str(), asciiData.size(), 0);
        }
        else {
            // Send the data to the client using the data socket
            bytesSent=send(dataSocket, buffer, bytesRead, 0);  
        }
        
        if (bytesSent < 0) {
            std::cerr << "Error sending data to server\n";
            break;
        }
        cout<<"Sent buffer:"<<buffer<<" with "<<bytesSent<<" bytesSent\n";
    }

    // Close the file and the data connection on the client side
    file.close();
    close(dataSocket);

    // Wait for the server's response indicating the file transfer is complete
    const int responseMaxSize = 1024;
    char response[responseMaxSize];
    int responseBytesRead = recv(clientSocket, response, responseMaxSize, 0);
    if (responseBytesRead <= 0) {
        perror("Error receiving response from server");
        close(clientSocket);
        return;
    }
    response[responseBytesRead] = '\0';

    // Display the server's response
    std::cout << "Server Response: " << response << std::endl;
    std::cout << "File transfer complete.\n";
}
  
};


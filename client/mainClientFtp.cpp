#include <iostream>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include "fileManager.cpp"

using namespace std;

int createDataSocket(const char* ipAddress, int port) {
    cout<<"[Client]: Se creeaza socket pe IP: "<<ipAddress<<" si PORT: "<<port<<endl;
    int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (dataSocket == -1) {
        perror("Error creating data socket");
        return -1;
    }

    struct sockaddr_in dataAddress{};
    dataAddress.sin_family = AF_INET;
    dataAddress.sin_port = htons(port);

    if (inet_pton(AF_INET, ipAddress, &dataAddress.sin_addr) <= 0) {
        perror("Error converting data IP address");
        close(dataSocket);
        return -1;
    }

    if (connect(dataSocket, (struct sockaddr*)&dataAddress, sizeof(dataAddress)) == -1) {
        perror("Error connecting to data socket");
        close(dataSocket);
        return -1;
    }

    return dataSocket;
}


void extractValues(const char* input, int& h1, int& h2, int& h3, int& h4, int& p1, int& p2) {
    std::istringstream ss(input);
    char dummy; // To consume ',' and '('

    ss.ignore(256, '('); // Skip the opening '('
    ss >> h1 >> dummy >> h2 >> dummy >> h3 >> dummy >> h4 >> dummy >> p1 >> dummy >> p2;
}


int main() {
    const char* serverIP = "127.0.0.1"; // Adresa IP a serverului
    const int serverPort = 21;           // Portul standard FTP

    // Crează un socket pentru client
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Eroare la crearea socketului");
        return -1;
    }

    // Setează detaliile serverului (adresa IP și port)
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

    // Converteste adresa IP de la format text la format binar
    if (inet_pton(AF_INET, serverIP, &serverAddress.sin_addr) <= 0) {
        perror("Eroare la convertirea adresei IP");
        close(clientSocket);
        return -1;
    }

    // Conectează clientul la server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Eroare la conectarea la server");
        close(clientSocket);
        return -1;
    }

    std::string cmd;
    char buffer[1024];
    FileManager fileManager;
    
    int passiveDataSocket = -1; // Initialize to an invalid value

while(true)
{
    
    std::cout << "Scrieti o comanda pentru Serverul FTP:"<<endl;
    std::getline(std::cin, cmd);

    const char* parameter ="";
    if(cmd.size()>4) {
        parameter=cmd.c_str() + 5;
    }
    

    if (send(clientSocket, cmd.c_str(), cmd.size(), 0) == -1) {
            perror("Eroare la trimiterea comenzii ");
            close(clientSocket);
            return -1;
    }

    if (strncmp(cmd.c_str() , "TYPE", 4) == 0)
    {
         fileManager.setType(parameter);
         // Așteaptă răspunsul de la server
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            perror("Eroare la primirea răspunsului de la server");
            close(clientSocket);
            return -1;
        }

        // Afișează răspunsul de la server
        buffer[bytesRead] = '\0';
        std::cout << "Răspuns de la server: " << buffer<<endl;
    }
    
    else if(strncmp(cmd.c_str(), "PASV", 4) == 0) 
    {
            // Receive the response from the server
            int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) {
                perror("Error receiving PASV response");
                close(clientSocket);
                return -1;
            }

            // Print the PASV response
            buffer[bytesRead] = '\0';
            std::cout << "PASV Response: " << buffer << endl;

            cout<<"CLIENT: Extract passive mode information"<<endl;
            // Extract the passive mode information (h1, h2, h3, h4, p1, p2) from the response
            int h1, h2, h3, h4, p1, p2;
            // Call the function to extract values
            extractValues(buffer, h1, h2, h3, h4, p1, p2);

             // Create a new socket and connect to the server in passive mode
            passiveDataSocket = createDataSocket(
                (to_string(h1) + "." + to_string(h2) + "." + to_string(h3) + "." + to_string(h4)).c_str(),
                p1 * 256 + p2);
        
    }
    else if (strncmp(cmd.c_str(), "LIST", 4) == 0) {
        if (passiveDataSocket == -1) {
                std::cerr << "Error: PASV command must be executed first." << std::endl;
                continue;
            }
        
        // Așteaptă răspunsul de la server
        const int bufferSize = 1024;
        char buffer[bufferSize];
        std::string response;
        
        // for the 150 response recieve is on the clientSocket!!!
        int bytesRead = recv(clientSocket, buffer, bufferSize, 0);
        if (bytesRead <= 0) {
            perror("Eroare la primirea răspunsului de la server.");
            close(clientSocket);
            return -1;
        }
        buffer[bytesRead] = '\0';
        while (true) {
            int bytesRead = recv(passiveDataSocket, buffer, bufferSize, 0);
            if (bytesRead < 0) {
                perror("Eroare la primirea răspunsului de la server.");
                close(clientSocket);
                return -1;
            }
            if(bytesRead == 0) {
                // There is nothing else to read from the passiveDataSocket. The server closed the socket
                break;
            }

            // Null-terminate the received data
            buffer[bytesRead] = '\0';
            // Append the received data to the response string
            response.append(buffer, bytesRead);
        }
        bytesRead = recv(clientSocket, buffer, bufferSize, 0);
        if (bytesRead <= 0) {
            perror("Eroare la primirea răspunsului de la server.");
            close(clientSocket);
            return -1;
        }
        buffer[bytesRead] = '\0';
        response.append(buffer, bytesRead);
        passiveDataSocket = -1;
        
        if (fileManager.getTypeOfTransfer() == "A") {
            response = fileManager.convertToLF(response);
        } 
        // Afișează răspunsul de la server
        std::cout << "Răspuns de la server: " << response << std::endl;
    }
    else if (strncmp(cmd.c_str() , "RETR", 4) == 0)
    {
          if (passiveDataSocket == -1) {
                std::cerr << "Error: PASV command must be executed first." << std::endl;
                continue;
          }
          
          // Așteaptă răspunsul de la server
        const int bufferSize = 1024;
        char buffer[bufferSize];
        std::string response;
        
        // for the 150 response recieve is on the clientSocket!!!
        int bytesRead = recv(clientSocket, buffer, bufferSize, 0);
        if (bytesRead <= 0) {
            perror("Eroare la primirea răspunsului de la server.");
            close(clientSocket);
            return -1;
        }
        buffer[bytesRead] = '\0';
        cout<<"Server: "<<buffer<<endl;
        if(strncmp(buffer , "150", 3) == 0) {
            cout<<"Waiting for file " << parameter << endl;
        }
        else {
            continue;
        }
        fileManager.receiveFile(clientSocket,passiveDataSocket, parameter);
        passiveDataSocket = -1;
        
    }
    else if (strncmp(cmd.c_str(), "STOR", 4) == 0)
    {
        if (passiveDataSocket == -1) {
                std::cerr << "Error: PASV command must be executed first." << std::endl;
                continue;
          }
          
        // Așteaptă răspunsul de la server
        const int bufferSize = 1024;
        char buffer[bufferSize];
        std::string response;
        
        // for the 150 response recieve is on the clientSocket!!!
        int bytesRead = recv(clientSocket, buffer, bufferSize, 0);
        if (bytesRead <= 0) {
            perror("Eroare la primirea răspunsului de la server.");
            close(clientSocket);
            return -1;
        }
        buffer[bytesRead] = '\0';
        cout<<"Server: "<<buffer<<endl;
        
        cout<<"Trimitem file-ul " << parameter << endl;
        fileManager.sendFile(clientSocket,passiveDataSocket,parameter);
        passiveDataSocket = -1;
    }
    else if (strncmp(cmd.c_str(), "QUIT", 4) == 0) 
    {
        // Așteaptă răspunsul de la server
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            perror("Eroare la primirea răspunsului de la server");
            close(clientSocket);
            return -1;
        }

        // Afișează răspunsul de la server
        buffer[bytesRead] = '\0';
        std::cout << "Răspuns de la server: " << buffer << endl;

        // Închide socketul și iese din buclă
        close(clientSocket);
        std::cout << "Deconectare..." << endl;
        return 0;
        }
        else{
        // Așteaptă răspunsul de la server
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            perror("Eroare la primirea răspunsului de la server");
            close(clientSocket);
            return -1;
        }

        // Afișează răspunsul de la server
        buffer[bytesRead] = '\0';
        std::cout << "Răspuns de la server: " << buffer<<endl;
        }
       
}
    

    // Închide socketul
    close(clientSocket);

    return 0;
}

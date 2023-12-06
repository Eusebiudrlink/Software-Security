#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <thread>
#include <fstream>
#include <filesystem>
#include "users.cpp"
#include "fileManager.cpp"
using namespace std;

//namespace fs = std::experimental::filesystem;
namespace fs = std::filesystem;

/*
2 sockets are open on the server side for passive mode:

Control Socket: Listens on port 21 for command communication.
Data Socket: Listens on a dynamically assigned port for passive mode data transfer.
*/

    // Funcție pentru a gestiona un client într-un fir de execuție separat
    void clientHandler(int clientSocket, Users& users, FileManager& fileManager) {
        int passiveSocket = socket(AF_INET, SOCK_STREAM, 0);
        int passiveClientSocket = -1; // initialize to an invalid value
        bool passiveConnectionEstablished = false;

        string clientUsername;  
        fs::path currentPath = fs::current_path();

        // Navigate back one step
        fs::path parentPath = currentPath.parent_path();
        
        fs::path rootDirectory = parentPath / "files";
        string clientDirectory="";
        
        while(true) {
        // Buclă de ascultare pentru comenzi
        char buffer[1024];
        // Așteaptă și citește comanda de la client
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            // Conexiunea s-a închis sau a apărut o eroare
            break;
        }

        // Adaugă terminarea șirului la sfârșitul datelor primite pentru a le interpreta corect ca șir de caractere
        buffer[bytesRead] = '\0';   

        const char* parameter = buffer + 5; // Saltă peste comanda "USER PASS LIST"

        // Identifică comanda primită
        if (strncmp(buffer, "USER", 4) == 0) {
            // Procesează comanda USER
            std::cout << "Comanda USER primita: " << buffer << std::endl;
            if(users.checkUser(parameter))
            {
                  clientUsername = parameter;
                  const char* response331 = "331 User name okay, need password.\r\n";
                  send(clientSocket, response331, strlen(response331), 0);
            }
            else
            {
                  const char* response530 = "530 Not logged in. Wrong User\r\n";
                  send(clientSocket, response530, strlen(response530), 0);
            }
            
        } 
        else if (strncmp(buffer, "PASS", 4) == 0) {
            // Procesează comanda PASS
            std::cout << "Comanda PASS primita: " << buffer << std::endl;
            if(users.checkPass(parameter))
            {
                // if the password if ok, we can use update the fake root
                clientDirectory = rootDirectory / clientUsername;
                
                // Set the working directory to the client's directory
                chdir(clientDirectory.c_str());
            
                const char* response230 = "230 User logged in, proceed.\r\n";
                send(clientSocket, response230, strlen(response230), 0);
            }
            else
            {
                const char* response530 = "530 Not logged in.Wrong PASS\r\n";
                  send(clientSocket, response530, strlen(response530), 0);
            }

        }
        else if(!users.getStatusUser())//if it is not logged
        {
                const char* response530 = "530 Not logged in\r\n";
                send(clientSocket, response530, strlen(response530), 0);
        }
        else if (strncmp(buffer, "RETR", 4) == 0)
        {
            fileManager.sendFile(clientSocket,parameter);
            cout<<"Am trimis file ul "<<parameter<<endl;
        }
        else if (strncmp(buffer, "PASV", 4) == 0) 
        {
            // Procesează comanda PASV
            std::cout << "Comanda PASV primita: " << buffer << std::endl;
            
            // Set up the passive socket
            struct sockaddr_in passiveAddress{};
            passiveAddress.sin_family = AF_INET;
            passiveAddress.sin_addr.s_addr = INADDR_ANY;
            passiveAddress.sin_port = htons(0); // Let the system choose an available port

            // Bind and listen on the passive socket
            if(bind(passiveSocket, (struct sockaddr*)&passiveAddress, sizeof(passiveAddress
            )) == -1 || listen(passiveSocket, 1) == -1) {
                perror("Error setting up passive connection");
                close(passiveSocket); // Close the passive socket in case of an error
                return; // or handle the error in an appropriate way
            }

            // Get the passive socket port
            sockaddr_in passiveServerAddress{};
            socklen_t passiveServerAddrLen = sizeof(passiveServerAddress);
            getsockname(passiveSocket, (struct sockaddr*)&passiveServerAddress, &passiveServerAddrLen);

            cout << "SERVER: Passive socket IP: " << inet_ntoa(passiveServerAddress.sin_addr) << ", Port: " << ntohs(passiveServerAddress.sin_port) << endl;
            
            // Send the passive mode response to the client
            unsigned char h1, h2, h3, h4, p1, p2;
            memcpy(&h1, &passiveServerAddress.sin_addr.s_addr, 1);
            memcpy(&h2, ((char*)&passiveServerAddress.sin_addr.s_addr) + 1, 1);
            memcpy(&h3, ((char*)&passiveServerAddress.sin_addr.s_addr) + 2, 1);
            memcpy(&h4, ((char*)&passiveServerAddress.sin_addr.s_addr) + 3, 1);
            p1 = ntohs(passiveServerAddress.sin_port) / 256;
            p2 = ntohs(passiveServerAddress.sin_port) % 256;

            // Send the passive mode response to the client
            char responseBuffer[1024];
            snprintf(responseBuffer, sizeof(responseBuffer), "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n",
             h1, h2, h3, h4, p1, p2);

            // Send the response to the client
            send(clientSocket, responseBuffer, strlen(responseBuffer), 0);
          
            // Accept the data connection from the client
            sockaddr_in passiveClientAddress{};
            socklen_t passiveClientAddrLen = sizeof(passiveClientAddress);
            passiveClientSocket = accept(passiveSocket, (struct sockaddr*)&passiveClientAddress, &passiveClientAddrLen);
            passiveConnectionEstablished = true;
            cout << "SERVER: Accept connection from the client on the passive socket" << endl;

          
        }
        else if (strncmp(buffer, "LIST", 4) == 0) 
        {
            if(!passiveConnectionEstablished) {
                cout<<"Command not taken into consideration.\n";
                continue;
            }
            
            // Procesează comanda LIST
            std::cout << "Comanda LIST primita: " << buffer << std::endl;
            
            const char* response150 = "150 Here comes the directory listing.\n";
            send(clientSocket, response150, strlen(response150), 0);
            
            // if there was a directory provided it would be in the parameter variable
            // Construct the command to list files in the specified directory
            string directoryName = parameter;
            cout<<directoryName<<endl;
            std::string listCommand = "ls -l " + directoryName;
            FILE* pipe = popen(listCommand.c_str(), "r");
            // Implement the directory listing
            //FILE* pipe = popen("ls -l", "r");
            if (!pipe) {
                perror("Error executing command");
                const char* response500 = "500 Error executing LIST command.\r\n";
                send(clientSocket, response500, strlen(response500), 0);
            } 
            else 
            {
                char buffer[1024];
                while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                    int bytesSent = send(passiveClientSocket, buffer, strlen(buffer), 0);
                    cout<<"SERVER: bytes sent to client: "<<bytesSent<<endl;
                }
                pclose(pipe);
                const char* response226 = "226 Directory send OK.\r\n";
                send(clientSocket, response226, strlen(response226), 0);
            }

            // Close the passive data socket
            close(passiveClientSocket);
            passiveClientSocket = -1;  // Reset to an invalid value
            passiveConnectionEstablished=false;
            
        }

        else if (strncmp(buffer, "QUIT", 4) == 0) {
        // Procesează comanda QUIT
        std::cout << "Comanda QUIT primita: " << buffer << std::endl;

        const char* response221 = "221 Service closing connection for you.\r\n";
        send(clientSocket, response221, strlen(response221), 0);

        // Închide socket-ul clientului și iese din buclă
        close(clientSocket);
        break; // ptc iese din while si inchide conexiunea resp thread-ul clientului
        
        }

        else {
            const char* response500 = "500 Syntax error, command unrecognized.Sau neimplementata!\r\n";
                  send(clientSocket, response500, strlen(response500), 0);
        }
    
        }
    // Închide socketul clientului după ce termini operațiile
    close(clientSocket);
    close(passiveClientSocket);
    
    }
    
int main() {
    const int serverPort = 21;  // Portul standard FTP

    Users users ;
    FileManager fileManager;
    // Deschide un socket pentru conexiuni de la clienți
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Eroare la deschiderea socketului");
        return -1;
    }

    // Setează detaliile serverului (adresa IP și port)
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(serverPort);

    // Leagă socketul la adresa și portul specificate
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Eroare la legarea la adresa și port");
        close(serverSocket);
        return -1;
    }

    // Ascultă pentru conexiuni de la clienți
    if (listen(serverSocket, 5) == -1) {
        perror("Eroare la ascultarea pentru conexiuni");
        close(serverSocket);
        return -1;
    }

    std::cout << "Server FTP asteapta conexiuni pe portul " << serverPort << std::endl;

    while(true) {
    // Acceptă conexiuni de la clienți
    sockaddr_in clientAddress{};
    socklen_t clientAddrLen = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddrLen);
    if (clientSocket == -1) {
        perror("Eroare la acceptarea conexiunii de la client");
        close(serverSocket);
        return -1;
    }

    std::cout << "Conexiune acceptata de la " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << std::endl;
    
    // Creați un fir de execuție nou pentru a gestiona clientul
    std::thread(clientHandler, clientSocket, std::ref(users), std::ref(fileManager)).detach();

    }
    close(serverSocket);

    return 0;
}
    


    



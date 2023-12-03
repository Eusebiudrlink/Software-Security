#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
#include "users.cpp"
#include "fileManager.cpp"
using namespace std;




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

    
    // Buclă de ascultare pentru comenzi
    char buffer[1024];
    while (true) {
        // Așteaptă și citește comanda de la client
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            // Conexiunea s-a închis sau a apărut o eroare
            break;
        }

        // Adaugă terminarea șirului la sfârșitul datelor primite pentru a le interpreta corect ca șir de caractere
        buffer[bytesRead] = '\0';   

        const char* parameter = buffer + 5; // Saltă peste comanda "USER PASS"

        // Identifică comanda primită
        if (strncmp(buffer, "USER", 4) == 0) {
            // Procesează comanda USER
            std::cout << "Comanda USER primita: " << buffer << std::endl;
            if(users.checkUser(parameter))
            {
                  const char* response331 = "331 User name okay, need password.\r\n";
                  send(clientSocket, response331, strlen(response331), 0);
            }
                else{
                  const char* response530 = "530 Not logged in.Wrong User\r\n";
                  send(clientSocket, response530, strlen(response530), 0);
            }
            
        } else if (strncmp(buffer, "PASS", 4) == 0) {
            // Procesează comanda PASS
            std::cout << "Comanda PASS primita: " << buffer << std::endl;
            if(users.checkPass(parameter))
            {
                const char* response230 = "230 User logged in, proceed.\r\n";
                send(clientSocket, response230, strlen(response230), 0);
            }
            else{
                const char* response530 = "530 Not logged in.Wrong PASS\r\n";
                  send(clientSocket, response530, strlen(response530), 0);
            }

        }else if(!users.getStatusUser())//if it is not logged
            {
                const char* response530 = "530 Not logged in\r\n";
                  send(clientSocket, response530, strlen(response530), 0);
        }
        else if (strncmp(buffer, "RETR", 4) == 0)
        {
            fileManager.sendFile(clientSocket,parameter);
            cout<<"Am trimis file ul "<<parameter<<endl;
        }

        else {
            const char* response500 = "500 Syntax error, command unrecognized.Sau neimplementata!\r\n";
                  send(clientSocket, response500, strlen(response500), 0);
        }
    

        // Trimite răspuns la client (exemplu simplificat)
       // const char* response = "200 OK\r\n";
        //send(clientSocket, response, strlen(response), 0);
    }

    // Închide socketurile după ce termini operațiile
    close(clientSocket);
    close(serverSocket);

    return 0;
}


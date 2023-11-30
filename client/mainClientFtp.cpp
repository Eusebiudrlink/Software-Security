#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
using namespace std;

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

while(true)
{
     std::cout << "Scrieti o comanda pentru Serverul FTP:"<<endl; 
    std::getline(std::cin, cmd);
    if (send(clientSocket, cmd.c_str(), cmd.size(), 0) == -1) {
            perror("Eroare la trimiterea comenzii ");
            close(clientSocket);
            return -1;
        }

        // Așteaptă răspunsul de la server
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            perror("Eroare la primirea răspunsului de la server");
            close(clientSocket);
            return -1;
        }

        // Afișează răspunsul de la server
        buffer[bytesRead] = '\0';
        std::cout << "Răspuns de la server: " << buffer;
}
    

    // Închide socketul
    close(clientSocket);

    return 0;
}

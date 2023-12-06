#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
using namespace std;

class Users {
private:
    struct UserInfo {
        std::string username;
        std::string password;
    };

    std::vector<UserInfo> userList;
    std::string curentUser;
    bool loggedIn=false;

public:
    Users() {
        std::ifstream file("userlist.txt");

        if (!file.is_open()) {
            std::cerr << "Eroare la deschiderea fisierului.\n";
            exit(EXIT_FAILURE);
        }

        // Citeste lista de utilizatori și parole din fișier
        std::string line;
        while (std::getline(file, line)) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                UserInfo user;
                user.username = line.substr(0, pos);
                user.password = line.substr(pos + 1);
                userList.push_back(user);
            }
        }
        file.close();
    }

    bool checkUser(const std::string& username) {
        for (const auto& user : userList) {
            if (user.username == username) {
                curentUser = username;
                return true; // Utilizatorul a fost găsit
            }
        }
        return false; // Utilizatorul nu a fost găsit
    }

    bool checkPass(const std::string& password) {
      
        for (const auto& user : userList) {
            if ((curentUser== user.username) &&( password==user.password)) {
                loggedIn=true;
                return true; // Utilizatorul și parola au fost găsite
            }
        }
        curentUser = "";
        return false; // Utilizatorul sau parola nu au fost găsite
    }
    bool getStatusUser(){
        return loggedIn;
    }
};

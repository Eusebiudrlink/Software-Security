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
                 size_t lastNonSpace = user.password.find_last_not_of(" \t\r\n");
                if (lastNonSpace != std::string::npos) {
                user.password.erase(lastNonSpace + 1);
                } else {
                // Dacă parola constă doar din caractere de spațiu sau linie nouă, o eliminăm complet
                 user.password.clear();
                 }

                userList.push_back(user);
            }
        }
        file.close();
    }

    bool checkUser(const std::string& username) {
        for (const auto& user : userList) {
            cout<<username<<" and "<<user.username<<endl;
            if (username==user.username) {
                cout<<"VICTORY: "<<username<<" and "<<user.username<<endl;
                curentUser = username;
                return true; // Utilizatorul a fost găsit
            }
        }
        return false; // Utilizatorul nu a fost găsit
    }

    bool checkPass(const std::string password) {
      
        for (const auto& user : userList) {
            cout<<curentUser<<" and "<<user.username<<endl;
            cout<<password<<" and "<<user.password<<"."<<endl;
            
            if ((curentUser==user.username)&&( password==user.password)) {
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

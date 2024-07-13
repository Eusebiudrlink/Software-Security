# Software-Security
This software console app simulates the FTP working flow for transferring files between two PCs in the same network.
This app was done in collaboration with a colleague which was a good opportunity to learn about organization, planning, and distribution of tasks

How to compile this app:
Compilare client :  g++ mainClientFtp.cpp fileManager.cpp -o mainClientFtp <br>
Compilare server :  g++ mainServerFTP.cpp users.cpp  fileManager.cpp -o mainServerFTP  <br>

Rulare: ./mainClientFtp (la server adaugam in fata sudo for admin rights)

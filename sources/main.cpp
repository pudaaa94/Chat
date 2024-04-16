// Chat.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "main.h"

int main(int argc, char* argv[])
{
    // choosing if client or server
    
    if (argc > 1) {
        if (*argv[1] == 'c' || *argv[1] == 'C'){
            // client
            if (argc < 3) {
                std::cout << "No username provided" << std::endl;
            } else {
                Client client(argv[2]);
            }
        }
        else {
            // server
            Server server;
            server.start();        
        }
    }
    else {
        std::cout << "No arguments provided." << std::endl;
    }
    
    return 0;
}

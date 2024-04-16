#pragma once

#include "top_level.h"

class Server {
    public:        
        Server();
        ~Server();
        void start();
        void stop();

    private:

        boost::asio::io_service io_service;
        boost::asio::ip::tcp::acceptor acceptor;
        std::thread acceptThread;
        std::vector<std::thread> handlingThreads;
        std::vector<std::string> userNames;
        std::vector<std::string> messagesToBeSent;
        std::deque<boost::asio::ip::tcp::socket> sockets;
        std::mutex socketsMutex;
        std::mutex userNamesMutex;
        std::mutex timeToBroadcastMutex;
        std::mutex clientCounterMutex;
        std::mutex messagesToBeSentMutex;
        
        bool timeToBroadcast = false;        
        const int numOfhandlingThreads = MAX_NUM_OF_CLIENTS;
        int currentNumOfClients = 0;
        int broadcastCounter = 0;

        void acceptConnections();
        void handleCommunication();
};
#include "Server.h"

using namespace std;
using namespace boost::asio;

Server::Server() : acceptor(io_service, ip::tcp::endpoint(ip::tcp::v4(), 12345)) {}

Server::~Server() {
    stop();
}

void Server::start() {
    // Start accepting connections in a separate thread
    acceptThread = thread([this]() {  // [this] part captures the current object ('this') by reference, allowing the lambda to access Server class members
        acceptConnections();
        });

    // Handle reading and writing from/to clients in separate threads
    for (int i = 0; i < numOfhandlingThreads; ++i) {
        handlingThreads.emplace_back([this]() {
            handleCommunication();
            });
    }
}

void Server::stop() {
    // Cleanup and stop server
    // Join all threads
    acceptThread.join();
    for (auto& thread : handlingThreads) {
        thread.join();
    }
}

void Server::acceptConnections() {
    
    std::cout << "Listening for connections started" << std::endl;
    
    while (true) {
        ip::tcp::socket socket(io_service);
        acceptor.accept(socket);

        // Dispatch the new connection to a reading and writing threads
        {
            lock_guard<mutex> lock(socketsMutex);
            sockets.push_back(move(socket));
            {
                lock_guard<mutex> lock(clientCounterMutex);
                currentNumOfClients++;
            }            
            std::cout << "Connection accepted " << std::endl;
        }
    }
}

void Server::handleCommunication() {
   
    while (true) { // socket might be closed in first iteration, that's why we need while(true)
        
        /* local identificators, handles and temps */
        std::string userName;
        bool broadcastedAvailableUsers = false;
        ip::tcp::socket socket(io_service);

        {
            lock_guard<mutex> lock(socketsMutex);
            //std::cout << "Reading started for " << clientNum << std::endl;
            if (!sockets.empty()) {
                socket = move(sockets.front()); // Move the socket from the front of the deque to the local variable
                sockets.pop_front(); // Remove the socket from the deque             
            }
        }

        /* COMMUNICATE WHILE SOCKET IS OPEN */
        while (socket.is_open()) {
            
            /* local temps */
            std::string message;

            /* BROADCASTING ONLINE STATUS */
            {                
                /* broadcast who is online */                
                lock_guard<mutex> lock(timeToBroadcastMutex);                
                if (timeToBroadcast) {
                    std::cout << "User " << userName << " thread" << std::endl;
                    std::cout << "timeToBroadcast: " << timeToBroadcast << " broadcastedAvailableUsers: " << broadcastedAvailableUsers << std::endl;
                    if (!broadcastedAvailableUsers) {                        
                        message = "Online users:";
                        {
                            lock_guard<mutex> lock(userNamesMutex);
                            for (std::string element : userNames) {
                                message += element + ":";
                            }
                            try {
                                std::cout << message << std::endl;
                                boost::asio::write(socket, boost::asio::buffer(message));
                                broadcastedAvailableUsers = true;                                
                                broadcastCounter++; 
                                std::cout << "broadcastCounter: " << broadcastCounter << " currentNumOfClients: " << currentNumOfClients << std::endl;
                                if (broadcastCounter == currentNumOfClients) {
                                    broadcastCounter = 0;
                                    timeToBroadcast = false;

                                }
                            }
                            catch (const std::exception& e) {
                                std::cerr << "Error writing data to client: " << e.what() << std::endl;
                            }
                        }                        
                    }                      
                } else {
                    broadcastedAvailableUsers = false;
                }
            }

            /* ACCEPTING MESSAGES */
            if (socket.available() > 0) {                
                try {                    
                    boost::asio::streambuf buffer;

                    // Synchronously read data from the socket into the buffer
                    boost::asio::read(socket, buffer, boost::asio::transfer_at_least(1));

                    // Data has been read successfully, process it here
                    std::istream is(&buffer);                    
                    std::getline(is, message); // Read data from buffer
                    std::cout << "Received from client: " << message << std::endl;   
                    std::istringstream iss(message);
                    std::string token;
                    std::vector<std::string> parsed;                    
                    while (std::getline(iss, token, ':')) {
                        parsed.push_back(token);
                    }
                    if (parsed[0] == "reg") {
                        /* user registers for chat, server dedicates resources for it */
                        userName = parsed[1];                                              
                        {
                            lock_guard<mutex> lock(timeToBroadcastMutex);
                            timeToBroadcast = true;
                            {
                                lock_guard<mutex> lock(userNamesMutex);
                                userNames.push_back(parsed[1]);
                                std::cout << "User registered: " << parsed[1] << std::endl;
                                message = parsed[1] + " successfully registered";
                            }                        
                        }
                        try {
                            boost::asio::write(socket, boost::asio::buffer(message));
                        }
                        catch (const std::exception& e) {
                            std::cerr << "Error writing data to client: " << e.what() << std::endl;
                        }
                    } else {
                        /* user sent a message to someone, process it. It is received in form sender:receiver0:receiver1:...:data */
                        int i = 0;
                        while (i < parsed.size()-2) { // because the last field is message itself
                            {
                                lock_guard<mutex> lock(messagesToBeSentMutex);
                                messagesToBeSent.emplace_back(
                                    parsed[0]   + ":" +     // sender
                                    parsed[i+1] + ":" +     // next receiver
                                    parsed[parsed.size()-1] // data  
                                );                                                               
                                std::cout << "Message to be sent" << messagesToBeSent[i] << std::endl;
                                i++;
                            }                            
                        }
                    }                    
                }
                catch (const std::exception& e) {
                    // Error occurred while reading data
                    std::cerr << "Error reading data from client: " << e.what() << std::endl;
                }
            } else {               
                //std::cout << "Nothing to read" << std::endl;           
            }

            /* ROUTING MESSAGES TO RECIPIENTS */
            {
                lock_guard<mutex> lock(messagesToBeSentMutex);   
                int i = 0;
                while (i < messagesToBeSent.size()) {
                    std::istringstream iss(messagesToBeSent[i]);
                    std::string token;
                    std::vector<std::string> parsed;
                    while (std::getline(iss, token, ':')) {
                        parsed.push_back(token);
                    }
                    if (parsed[1] == userName) {
                        messagesToBeSent.erase(messagesToBeSent.begin() + i);
                        try {
                            boost::asio::write(socket, boost::asio::buffer(parsed[0] + ":" + parsed[2]));
                        }
                        catch (const std::exception& e) {
                            std::cerr << "Error writing data to client: " << e.what() << std::endl;
                        }
                    } else {
                        i++;
                    }
                }
            }
        }
    }
}
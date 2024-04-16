#include "Client.h"

using namespace std;
using namespace boost::asio;

Client::Client(std::string userName): userName(userName) {

    try {
        // Create a boost::asio::io_context object
        io_context io_context;

        // Create a TCP resolver object to resolve the server endpoint
        ip::tcp::resolver resolver(io_context);

        // Resolve the server endpoint
        ip::tcp::resolver::results_type endpoints = resolver.resolve("localhost", "12345");

        // Create a TCP socket
        ip::tcp::socket socket(io_context);

        // Connect to the server
       connect(socket, endpoints);
       try {           
            // register on network
            write(socket, buffer("reg:" + userName));      
       }
       catch (const exception& e) {
           cerr << "Exception in inputHandler thread: " << e.what() << endl;
       }

       // Start input handling thread
       thread inputHandler([&]() {
           try {
               while (true) {
                   // Read user input from stdin
                   string userInput;
                   getline(cin, userInput);

                   cout << "Sending: " << userInput << endl;
                   // Send user input to the server
                   write(socket, buffer(userName + ":" + userInput + "\n"));
               }
           }
           catch (const exception& e) {
               cerr << "Exception in inputHandler thread: " << e.what() << endl;
           }
           });

       // Start receive handling thread
       thread receiveHandler([&]() {
           try {
               while (true) {
                   // Buffer to store data received from the server
                   if (socket.available() > 0) {
                       //std::cout << "Entered" << std::endl;
                       boost::asio::streambuf buffer;

                       // Synchronously read data from the socket into the buffer
                       boost::asio::read(socket, buffer, boost::asio::transfer_at_least(1));

                       // Data has been read successfully, process it here
                       std::istream is(&buffer);
                       std::string message;
                       std::getline(is, message); // Read data from buffer
                       std::cout << "Received from server: " << message << std::endl;
                   }
               }
           }
           catch (const std::exception& e) {
               cerr << "Exception in receiveHandler thread: " << e.what() << endl;
           }
           });

       // Wait for both input handling thread and receive handling thread to finish
       inputHandler.join();
       receiveHandler.join();
    }
    catch (exception& e) {
        cerr << "Exception caught: " << e.what() << endl;
    }
}
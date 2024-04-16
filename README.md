Main purpose of this project was practicing of C++ skills.

PREREQUISITES

Installed Boost Asio library

HOW TO RUN?

Open multiple Command Prompts and in each of it run one of following Chat.exe commands (there should be exactly one command for starting of server and up to ten commands for starting of clients):

Chat.exe s
(this command will start a process which will represent a server for Chat platform)

Chat.exe c user0
(this command will start a process which will represent a first Client on Chat platform)

Chat.exe c user1
(this command will start a process which will represent a second Client on Chat platform. You can add up to 10 clients)

Each time some user joins the network, it is broadcasted to everyone on network who is currently on network and available for chat

Here is a snippet from user terminal after its successful joining to the network:

Chat.exe c user

Received from server: user successfully registered

Received from server: Online users:puda:user:

The last message says that there are two users on network: puda and user

In order to chat with puda, user will write following in its terminal:

puda:hello

Server will know that this message came from user and will route it to puda (it's terminal will look like this):

Received from server: user:hello

In order to respond, puda will write to its terminal:

user:hello to you too

Group chats are also supported, it is just needed to add multiple users before message:

user1:user2:user3:hello 

In terminals of user1, user2 and user3, message hello will appear

To be done:
disconnecting from Chat network
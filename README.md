# multiT-C-pServer
A multithread server that responds to client requests in C (POSIX).

## The project
**multiT-C-pServer** is a study repository of a client/server application developed in C. The project simulates a sittuation where multiple equipments make requisitions to the server throught commands in the terminal, and the server has to deal with all requisitions in parallel. To solve the problem, multiple threads are created, in the server side and in the client side. For each client connected to the server an independent thread is created, and every client have an independent thread to receive information from server at the same time as it can read the input.

## Structures
The server can connect with 10 clients at most, each client have its own Id and its clients list that are connected to the server, that list must be updated every time that the server adds or removes an client from its database.

## Commands
The **multiT-C-pServer** accepts the following command codes from the client(equipment):
- *close connection*: Client requests to close the connection with the server.
- *list equipments*: Printscreen the list of equipments on the client database.
- *request information from _Id*: Client requests information from other client connected to the server. The "information" returned is a random float number from 0 to 10.

## Compiling and execution instructions
**The following instructions only apply to linux based systems**

To generate the _equipment_ and _server_ output files run ```make``` in the root project directory. After build the binaries, the server can be runned by passing the port as an argument. Example:

```./server 5151```

Runs the server listen to localport 5151.

To connect the client to the server, run the _equipment_ file passing the full address and port as the parameters. Example:

```./equipment 127.0.0.1 5151```

Connects the equipment on the server listen to the address 127.0.0.1::5151.

After connecting, the client can make request to server using the command line. Below is a screenshot of the program running on 4 different terminals.
![image](https://user-images.githubusercontent.com/73205375/203443846-e02e9253-7f81-4c02-a133-3fc691034f60.png)

## Final considerations
Cool project, it was really fun to make.

## Author
| [<img src="https://avatars.githubusercontent.com/u/73205375?v=4" width=115><br><sub>João Pedro Fernandes Silva</sub>](https://github.com/JoaoP-Silva)

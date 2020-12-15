#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

int main() {
    char server_message[256] = "You have reached the server01!";

    // create the server socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // define the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(10001);
    // server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // bind the socket to our specified IP and port
    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    // 2nd arg - backlog: how many connections can be waiting for this socket.
    listen(server_socket, 5);

    int client_socket;
    // 2nd param - struct that contains address of the client connection, 3rd -
    // sizeof it. We'll leave it at NULL
    client_socket = accept(server_socket, NULL, NULL);

    // send the welcoming message
    // opt flags: 0
    send(client_socket, server_message, sizeof(server_message), 0);

    // receive data from the client
    char client_message[256];
    recv(client_socket, &client_message, sizeof(client_message), 0);

    // print out the client's msg
    printf("The client's message is: %s\n", client_message);

	// convert client msg to upper case
	int i = 0;
    while(client_message[i]) {
      client_message[i] = toupper(client_message[i]);
      i++;
    }

    printf("The client's modified message is: %s\n", client_message);

    // create a socket to connect to server2
    int network_socket;

    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in network_address;
    network_address.sin_family = AF_INET;

    network_address.sin_port = htons(10002);
    // network_address.sin_addr.s_addr = INADDR_ANY;
    network_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // cast server_address to different structure
    int connection_status = connect(network_socket, (struct sockaddr *) &network_address, sizeof(network_address));

    if (connection_status == -1) {
        printf("There was an error making a connection to the remote socket \n\n");
    }

    char server_response[256];

    // receive data from the server2 (welcome msg)
    recv(network_socket, &server_response, sizeof(server_response), 0);

    // print out the server's response
    printf("The server2 sent the data: %s\n", server_response);
	
    // send client's message to the server2
    send(network_socket, client_message, sizeof(client_message), 0);

    recv(network_socket, &server_response, sizeof(server_response), 0);
    printf("The server2 sent the data: %s\n", server_response);

    close(network_socket);

    // send modified message from server2 back to the client
    send(client_socket, server_response, sizeof(server_response), 0);
    close(server_socket);

    return 0;
}

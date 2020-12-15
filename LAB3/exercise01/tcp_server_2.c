#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

int main() {
    char server_message[256] = "You have reached the server02!";

    // create the server socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // define the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(10002);
    // server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // bind the socket to our specified IP and port
    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    // 2nd arg - backlog: how many connections can be waiting for this socket.
    // Set 5, bet doesn't matter
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

    char modified_client_message[256];
	// modify client msg (double each letter)
    int i = 0;
    int j = 0;
    while(client_message[i]) {
      modified_client_message[j++] = client_message[i];
      modified_client_message[j++] = client_message[i++];
    }

    printf("The client's modified message is: %s\n", modified_client_message);

    send(client_socket, modified_client_message, sizeof(modified_client_message), 0);
    close(server_socket);

    return 0;
}

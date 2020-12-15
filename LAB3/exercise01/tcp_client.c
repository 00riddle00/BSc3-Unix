#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <time.h>

void waitFor (unsigned int secs);

int main() {
    // create a socket
    int network_socket;

    // protocol = 0 (default: TCP)
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    // specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;

    // convert integer to network byte order
    server_address.sin_port = htons(10001);

    // sin_addr - a struct that contains another struct
    // INADRR_ANY = 0.0.0.0 (any address used on local machine)
    // server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // cast server_address to different structure
    int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    // check for error with the connection
    // 0 for no errors
    if (connection_status == -1) {
        printf("There was an error making a connection to the remote socket \n\n");
    }

    char server_response[256];

    // receive data from the server (welcome msg)
    // optional flags: 0
    recv(network_socket, &server_response, sizeof(server_response), 0);

    // print out the server's response
    printf("The server1 sent the data: %s\n", server_response);
    
    char client_message[256] = "hello";

    // send data to the server
    send(network_socket, client_message, sizeof(client_message), 0);

    waitFor(1);
    
    recv(network_socket, &server_response, sizeof(server_response), 0);
    printf("The server1 sent the data: %s\n", server_response);
    close(network_socket);

    return 0;
}

void waitFor (unsigned int secs) {
    unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime);               // Loop until it arrives.
}

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> // for close
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_STR_LENGTH 256

const char HOST_IP[] = "127.0.0.1";
//const char HOST_IP[] = "::1";
const char CLIENT_PORT[]= "10000";

void send_msg(char* sender_name, char* client_message, char* server_port);

int main() {
    /* ----- SETUP ---------*/

    // vars which will be params for getaddrinfo()
    struct addrinfo hints;
    struct addrinfo *res; // will point to the results

    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;

    char welcome_msg[MAX_STR_LENGTH] = "You have reached the ";

    /* FIRST SERVER ------------------------------------------------------------- */

    /* ----- ACT AS A SERVER ------- */

    char server1_name[MAX_STR_LENGTH] = "server01";
    char server1_ip[MAX_STR_LENGTH] = "20001";

    // create the server socket
    int server1_socket;

    getaddrinfo(HOST_IP, server1_ip, &hints, &res);

    // protocol = 0 (default: TCP)
    server1_socket = socket(res->ai_family, res->ai_socktype, 0);

    // bind the socket to our specified IP and port
    bind(server1_socket, res->ai_addr, res->ai_addrlen);

    // 2nd arg - backlog: how many connections can be waiting for this socket.
    // Set 5, but doesn't matter
    listen(server1_socket, 5);

    int client1_socket;
    // 2nd param - struct that contains address of the client connection,
    // 3rd - sizeof it. We'll leave it at NULL
    // DOES NOT PROCEED FURTHER UNTIL IT GETS A CONNECTION

    struct sockaddr_storage their_addr;
    socklen_t addr_size;

    addr_size = sizeof their_addr;

    client1_socket = accept(server1_socket, NULL, NULL);

    // print the welcoming message
    printf("[%s] %s%s!\n", server1_name, welcome_msg, server1_name);

    // receive data from the client
    char client1_message[MAX_STR_LENGTH];
    char hop_count[MAX_STR_LENGTH];
    recv(client1_socket, &client1_message, sizeof(client1_message), 0);
    recv(client1_socket, &hop_count, sizeof(hop_count), 0);

    // print out the client's msg
    printf("[%s] The client's message is: %s\n", server1_name, client1_message);
    printf("[%s] The hop count selected is %s\n", server1_name, hop_count);

    int hops = atoi(hop_count);

	// convert client msg to upper case
	int a = 0;
    while(client1_message[a]) {
      client1_message[a] = toupper(client1_message[a]);
      a++;
    }

    printf("[%s] The client's modified message is: %s\n", server1_name, client1_message);
    close(server1_socket);

    /* INTERMEDIARY SERVERS ------------------------------------------------------ */

    // same as server1_port
    char current_port[MAX_STR_LENGTH] = "20001";

    for (int i = 2; i <= 2+hops; i++) {

        char server_current_name[MAX_STR_LENGTH] = "server0";
        char server_prev_name[MAX_STR_LENGTH] = "server0";
        sprintf(server_current_name, "%s%d", server_current_name, (char)i);
        sprintf(server_prev_name, "%s%d", server_prev_name, (char)i-1);

        // create the server socket
        int server_socket;

        // increment port number by i
        current_port[strlen(current_port)-1] = '\0';
        sprintf(current_port, "%s%d", current_port, (char)i);

        getaddrinfo(HOST_IP, current_port, &hints, &res);

        server_socket = socket(res->ai_family, res->ai_socktype, 0);

        // bind the socket to our specified IP and port
        bind(server_socket, res->ai_addr, res->ai_addrlen);

        // 2nd arg - backlog: how many connections can be waiting for this socket.
        // Set 5, but doesn't matter
        listen(server_socket, 5);

        int client_socket;

        // ############## send the message from previous server  ####################
        send_msg(server_prev_name, client1_message, current_port);
        // ###########################################################################

        // 2nd param - struct that contains address of the client connection,
        // 3rd - sizeof it. We'll leave it at NULL
        client_socket = accept(server_socket, NULL, NULL);

        // print the welcoming message
        printf("[%s] %s%s!\n", server_current_name, welcome_msg, server_current_name);

        // receive data from the client
        char client2_message[MAX_STR_LENGTH];
        recv(client_socket, &client2_message, sizeof(client2_message), 0);

        // print out the client's msg
        printf("[%s] The received client's message is: %s\n", server_current_name, client1_message);

        if (i == 2+hops) {

            char modified_client1_message[MAX_STR_LENGTH];
            // convert client message
            int j = 0;
            int k = 0;
            while(client1_message[j]) {
                modified_client1_message[k++] = client1_message[j];
                modified_client1_message[k++] = client1_message[j++];
            }

            printf("[%s] The client's modified message is: %s\n", server_current_name, modified_client1_message);

            send_msg(server_current_name, modified_client1_message, CLIENT_PORT);
        }

        close(server_socket);
    }
    /* ----------------------------------------------------------------------- */

    return 0;
}

// create a client socket, connect it to server socket with given server_port, send the message,
// and close the client socket
void send_msg(char* sender_name, char* client_message, char* server_port) {

    /* ----- SETUP ---------*/

    // vars which will be params for getaddrinfo()
    struct addrinfo hints;
    struct addrinfo *res; // will point to the results

    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;

    /* ---------------------*/

    // create a socket
    int client_socket;

    getaddrinfo(HOST_IP, server_port, &hints, &res);

    // protocol = 0 (default: TCP)
    client_socket = socket(res->ai_family, res->ai_socktype, 0);

    // connect
    int connection_status = connect(client_socket, res->ai_addr, res->ai_addrlen);

    // check for error with the connection
    // 0 for no errors
    if (connection_status == -1) {
        printf("[%s] There was an error making a connection to the remote socket \n\n", sender_name);
    }
    // send data to the server
    send(client_socket, client_message, sizeof(client_message), 0);

    // and then close the socket
    close(client_socket);
}


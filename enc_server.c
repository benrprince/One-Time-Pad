/*********************************************************************
 *      Author: Ben Prince
 *      Assignment 5: One-Time Pad
 *      Date: 05/31/2021
 * 
 *      Encryption Server File
 * ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

// Error function used for reporting issues
// Design copied from server.c example file on assignment page
void error(const char *msg) {

    perror(msg);
    exit(0);

}

void encrypt(char *text, char *key, char *encryptedText) {

    int i;
    int textChar;
    int keyChar;
    int newChar;

    for(i=0; i < strlen(text); i++) {

        if(text[i] == ' ') {
            textChar = 26;
        } else {
            textChar = text[i] - 'A';
        }

        if(key[i] == ' ') {
            keyChar = 26;
        } else {
            keyChar = key[i] - 'A';
        }
    
        newChar = (textChar + keyChar) % 27;

        if(newChar == 26) {
            encryptedText[i] = ' ';
        } else {
            encryptedText[i] = newChar + 'A';
        }
    }

}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]) {
    int connectionSocket, charsRead;
    char buffer[1024];
    char text[10000];
    char key[10000];
    char encryptedText[10000];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress); 
    
    // Check arguments
    if (argc < 2) { 
        fprintf(stderr,"SERVER: Argument Error\n"); 
        exit(1);
    }

    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket, 
        (struct sockaddr *)&serverAddress, 
        sizeof(serverAddress)) < 0){
            error("ERROR on binding");
    }

    // Start listening for connetions. Allow up to 5 connections to queue up
    listen(listenSocket, 5); 

      // Accept a connection, blocking if one is not available until one connects
    while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectionSocket < 0){
        error("ERROR on accept");
    }

    printf("SERVER: Connected to client running at host %d port %d\n", 
                            ntohs(clientAddress.sin_addr.s_addr),
                            ntohs(clientAddress.sin_port));

    // Get the message from the client and display it
    memset(buffer, '\0', 1024);
    // Read the client's message from the socket
    printf("%c\n",buffer[strlen(buffer)-1]);
    memset(buffer, '\0', 1024);

    // Get Text
    while(charsRead = recv(connectionSocket, buffer, 1, 0)) {
        if(buffer[0] == '\n') {
            break;
        }
        strcat(text, buffer);
    }
    
    // Get Key
    while(charsRead = recv(connectionSocket, buffer, 1, 0)) {
        if(buffer[0] == '@') {
            break;
        }
        strcat(key, buffer);
    }
    if (charsRead < 0){
        error("ERROR reading from socket");
    }
    encrypt(text, key, encryptedText);
    printf("SERVER: I received this from the client: \"%s\"\n", text);
    printf("SERVER: I received this from the client: \"%s\"\n", encryptedText);
    // Send a Success message back to the client
    charsRead = send(connectionSocket, 
                    "I am the server, and I got your message", 39, 0); 
    if (charsRead < 0){
        error("ERROR writing to socket");
    }

    // Close the connection socket for this client
    close(connectionSocket); 
    }
    // Close the listening socket
    close(listenSocket); 
    return 0;

}
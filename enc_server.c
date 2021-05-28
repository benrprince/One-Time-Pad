/*********************************************************************
 *      Author: Ben Prince
 *      Assignment 5: One-Time Pad
 *      Date: 05/31/2021
 * 
 *      Encryption Server File
 *      Source: https://www.youtube.com/watch?v=9g_nMNJhRVk&list=PLPyaR5G9aNDvs6TtdpLcVO43_jvxp4emI&index=8
 * ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <sys/wait.h>   // waitpid()
#include <netdb.h>      // gethostbyname()
#include <errno.h>

// Error function used for reporting issues
// Design copied from server.c example file on assignment page
void error(const char *msg) {
/******************************************************************
 *  param: char * 
 *  return: NA
 *  description: Prints error message and exits
 * ***************************************************************/
    errno = EBADE;   // Tried to find Generic Wording for the error, "Invalid Exchange"
    perror(msg);
    exit(0);

}

void encrypt(char *text, char *key, char *encryptedText) {

    int i;
    int textChar;
    int keyChar;
    int newChar;

    for(i=0; i < strlen(text); i++) {

        // Convert text chars to int
        if(text[i] == ' ') {
            textChar = 26;
        } else {
            textChar = text[i] - 'A';
        }

        // Convert key chars to int
        if(key[i] == ' ') {
            keyChar = 26;
        } else {
            keyChar = key[i] - 'A';
        }

        // Get encrypted character int
        newChar = (textChar + keyChar) % 27;

        // Convert encrypted character int to a char value
        if(newChar == 26) {
            encryptedText[i] = ' ';
        } else {
            encryptedText[i] = newChar + 'A';
        }
    }
    // Add new line char at the end
    encryptedText[i] = '\n';
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

    // Set up varibales for socket and fork
    pid_t spawnpid = -5;
    int connectionSocket, childStatus;
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

        spawnpid = fork();
        switch(spawnpid) {
            case -1: {

                error("SERVER: fork() failed");
                exit(1);
                break;
            }
            
            case 0: {

                char buffer[1024];
                char fullText[1000000];
                char parsedKey[100000];
                char parsedText[100000];
                char encryptedText[100000];
                char *key;
                char *text;
                int fileLength, charsRead, fromEncryptionServer;

                //Read number of chars to expect
                read(connectionSocket, &fileLength, sizeof(int));

                // Read chars and set them in fullText
                charsRead = 0;
                while(charsRead <= fileLength) {

                    charsRead += recv(connectionSocket, buffer, 255, 0);
                    printf("%s", buffer);
                    strcat(fullText, buffer);
                    memset(buffer, '\0', 1024);

                }
                if (charsRead < 0){
                    error("ERROR reading from socket");
                }

                // text and key are separated by a @ char. strtok to split them.
                text = strtok(fullText, "@");
                strcpy(parsedText, text);
                key = strtok(NULL, "@");
                strcpy(parsedKey, key);

                // Call encrypt to put the encrypted text in encryptedText string
                encrypt(parsedText, parsedKey, encryptedText);

                // Send a 0 to client to prove that this is encryption not decryption
                fromEncryptionServer = 0;
                write(connectionSocket, &fromEncryptionServer, sizeof(int));

                // Send encrypted text back to client
                charsRead = 0;
                while(charsRead < strlen(encryptedText)) {
                    charsRead += send(connectionSocket, encryptedText, 1000, 0);
                }
                if (charsRead < 0){
                    error("ERROR writing to socket");
                }

                memset(buffer, '\0', 1024);
                memset(fullText, '\0', 100000);
                memset(parsedKey, '\0', 100000);
                memset(parsedText, '\0', 100000);
                memset(encryptedText, '\0', 100000);

                // Close the connection socket for this client
                close(connectionSocket);
                break;

            }
            default: {

                spawnpid = waitpid(spawnpid, &childStatus, WNOHANG);
            }
        }
    }

    // Close the listening socket
    close(listenSocket); 
    return 0;

}
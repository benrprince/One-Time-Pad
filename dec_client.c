/*********************************************************************
 *      Author: Ben Prince
 *      Assignment 5: One-Time Pad
 *      Date: 05/31/2021
 * 
 *      Decryption Client File
 *      Source: https://www.youtube.com/watch?v=9g_nMNJhRVk&list=PLPyaR5G9aNDvs6TtdpLcVO43_jvxp4emI&index=8
 * ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <errno.h>

// Error function used for reporting issues
// Design copied from client.c example file on assignment page
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

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber){

    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname("localhost"); 
    if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);

}

int main(int argc, char *argv[]) {

    // Initial Setup
    int socketFD, portNumber, charsWritten, charsRead, fileLength, keyLength, server;
    struct sockaddr_in serverAddress;
    FILE *plainText;
    FILE *key;   // was key
    char buffer[256];   // change to 1024
    int size = 256;
    char sendText[100000];
    char keyText[100000];
    char readText[100000];

    // Check usage & args
    if (argc < 4) { 
        fprintf(stderr,"USAGE: %s hostname port\n", argv[0]);
        fprintf(stderr,"Example: enc_client plaintext key port\n");
        exit(0); 
    } 

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0){

        error("CLIENT: ERROR opening socket");

    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]));

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){

        error("CLIENT: ERROR connecting");

    }
    
    //Send text to server
    plainText = fopen(argv[1], "r");
    if (plainText == NULL) {

        error("CLIENT: Couldn't find plain text file");

    }

    key = fopen(argv[2], "r");
    if (key == NULL) {

        error("CLIENT: Couldn't find key file");

    }

    memset(buffer, '\0', sizeof(buffer));

    char c;
    fileLength = 0;
    for(c = getc(plainText); c != EOF; c = getc(plainText)) {

        sendText[fileLength] = c;
        fileLength++;

    }
    sendText[strcspn(sendText, "\n")] = '\0';
    sendText[strcspn(sendText, "\n")] = '@';

    keyLength = 0;
    int tempLength = fileLength;
    while((c = getc(key)) != EOF) {
        
        sendText[fileLength] = c;
        fileLength++;
        keyLength++;

        if(keyLength > tempLength) {
            break;
        }

    }
    if(keyLength < (fileLength - keyLength)) {
        
        error("CLIENT Error: Key is too short");

    }

    sendText[strcspn(sendText, "\n")] = '\0';
    keyLength--;

    write(socketFD, &fileLength, sizeof(int));
    rewind(plainText);
    rewind(key);

    char ch;
    charsWritten = 0;
    while(charsWritten <= fileLength) {  // Possibly change this

        charsWritten += write(socketFD, sendText, 1000);

    } 

    fclose(plainText);
    fclose(key);

    // Get return message from server
    // Clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));

    // Check that we sent to correct server
    read(socketFD, &server, sizeof(int));

    if(server != 1) {

        error("CLIENT Error: Sent to incorrect Server");

    }

    // Read data from the socket, leaving \0 at end
    charsRead = 0;
    while(charsRead < fileLength) {

        charsRead += recv(socketFD, buffer, 10, 0);
        strcat(readText, buffer);
        memset(buffer, '\0', sizeof(buffer));

    }
    
    if (charsRead < 0){

        error("CLIENT: ERROR reading from socket");

    }

    // Send encrypted text to stdout
    printf("%s", readText);
    fflush(stdout);

    // Close the socket
    close(socketFD); 
    return 0;
}
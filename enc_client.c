/*********************************************************************
 *      Author: Ben Prince
 *      Assignment 5: One-Time Pad
 *      Date: 05/31/2021
 * 
 *      Encryption Client File
 * ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

// Error function used for reporting issues
// Design copied from client.c example file on assignment page
void error(const char *msg) {

    perror(msg);
    exit(0);

}

int getFileLength(FILE *file, char* buffer) {

    int count;
    char ch;

    // https://www.codevscolor.com/c-program-read-file-contents-character
    for (ch = getc(file); ch != EOF; ch = getc(file)) {
        
        buffer[count] = ch;
        // Increment count for this character
        count = count + 1;

    }


    return count;
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
    int socketFD, portNumber, charsWritten, charsRead, fileLength;
    struct sockaddr_in serverAddress;
    FILE *plainText;
    FILE *key;
    char buffer[256];

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
    
    // Send plain text file 
    // Open plain text file
    plainText = fopen(argv[1], "r");
    if (plainText == NULL) {

        error("CLIENT: Couldn't find plain text file");

    }

    memset(buffer, '\0', sizeof(buffer));
    fileLength = getFileLength(plainText, buffer);
    printf("%d\n", fileLength);                     //Here for Testing REMOVE!!!!!
    buffer[strcspn(buffer, "\n")] = '\0'; 
    printf("%s", buffer);                           //Here for Testing REMOVE!!!!!

    //Send message to server
    // Write to the server
    charsWritten = send(socketFD, buffer, strlen(buffer), 0); 
    if (charsWritten < 0){

        error("CLIENT: ERROR writing to socket");

    }
    if (charsWritten < strlen(buffer)){

        printf("CLIENT: WARNING: Not all data written to socket!\n");

    }
    fclose(plainText);

    // Get return message from server
    // Clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at end
    // charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
    // if (charsRead < 0){

    //     error("CLIENT: ERROR reading from socket");

    // }

    // printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

    // Close the socket
    close(socketFD); 
    return 0;
}
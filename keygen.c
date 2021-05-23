/*********************************************************************
 *      Author: Ben Prince
 *      Assignment 5: One-Time Pad
 *      Date: 05/31/2021
 * 
 *      Key Generation File
 *          - Takes one command line agrument of how many chars
 *            the key should be.
 * ******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h> // Used for more randomness

int main(int argc, char *argv[]) {

    // Handle incorrect arguments
    if (argc != 2) {
        fprintf(stderr, "How long does the key need to be\n", argv[0]);
        fprintf(stderr, "Example: %s 1024\n", argv[0]);
        exit(0);
    }

    // Get length that was passed in and initialize string of that length+1
    int length = atoi(argv[1]);
    char randString[length+1];

    // Assignm Random letters in string
    for(int i=0; i < length; i++) {

        srand(time(NULL)*i);
        char randomChar = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "[rand() % 27];
        randString[i] = randomChar;

    }
    // Append a new line char at the end of the string
    randString[length] = '\n';

    // Print out generated key/send to redirected file name
    printf("%s", randString);

    return 0;
}
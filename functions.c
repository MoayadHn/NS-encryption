#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>


#define MAX_SIZE 51200


/*****************************************************************
 This function is to convert digits into string as the itoa function is not implemented in C99 which i am using.
 taken from : http://www.strudel.org.uk/itoa/
 ****************************************************************/
void itoa(int value, char* result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; }
    else{
        char* ptr = result, *ptr1 = result, tmp_char;
        int tmp_value;
        
        do {
            tmp_value = value;
            value /= base;
            *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
        } while ( value );
        
        // Apply negative sign
        if (tmp_value < 0) *ptr++ = '-';
        *ptr-- = '\0';
        while(ptr1 < ptr) {
            tmp_char = *ptr;
            *ptr--= *ptr1;
            *ptr1++ = tmp_char;
        }
    }
}

//error message funciton with progrram quit
void fatalError(const char *msg){
    perror(msg);
    exit(1);
}
//function to encrypt using subsitution cypher
void encryptMsg(char *msg, char *secret, char *key, int len){
    int i =0;
    for (i=0;i<len;i++){
        secret[i] = (char)(msg[i] - key[i%16]) ;
    }
    
}
//function to decrypt using subsitution cypher
void decryptMsg(char *msg, char *secret, char *key, int len){
    int i =0;
    for (i=0;i<len;i++){
        msg[i] = (char)(secret[i] + key[i%16]);
    }
}

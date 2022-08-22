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


void error(const char *msg){
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

//Declerations
int main(int argc, char *argv[]){
    char *msg;
    msg = (char *) malloc(MAX_SIZE);
    char *secret;
    unsigned int addr_len;
    secret = (char *) malloc(MAX_SIZE);
    char myKey[16];
    char *sessionKey = (char *) malloc(sizeof(char) * 17);
    char *tempbuff= (char *) malloc(sizeof(char)*17);
    char *tempNouce = (char *) malloc(sizeof(char) * 33);
    char *payload;
    payload = (char *) malloc(33 * sizeof(char) );
    int buffSize;
    int sock;
    char clientName[20];
    char otherClinetName[20];
    struct sockaddr_in server_addr, client_addr;
    struct hostent *host;
    int portno;
    int sessionPort;
    int currentNonce = rand();
    int r;
    char send_data[MAX_SIZE];
    char line_input[256];
    
    struct timeval t0,t1,t2;
    struct tm *tm;
    double timetaken = 0.0;
    
    // packet structure
    typedef struct packet{
        char type[1];
        char from[20];
        char to[20];
        int nonce;
        int length;
        char payload[MAX_SIZE];
    } packet;
    
    packet buffpacket;
    
    
    
    if (argc <7 ){
        fprintf(stderr,"ERROR,usage:%s -s <serverName> -g <ServerPort> -n <userName> -p <sessionPort>\n",argv[0]);
        exit(1);
    }
    
    int in;
    for(in=1;in<8;in+=2){
        if (strncmp(argv[in],"-s",2)==0){
            host= (struct hostent *) gethostbyname(argv[in+1]);
        }
        else if (strncmp(argv[in],"-g",2)==0){
            portno = atoi(argv[in+1]);
        }
        else if (strncmp(argv[in],"-n",2)==0){
            strcpy(clientName, argv[in+1]);
        }
        else if (strncmp(argv[in],"-p",2)==0){
            sessionPort= atoi(argv[in+1]);
        }
        else {
            fprintf(stderr,"ERROR, Wrong argument type '%s'\n",argv[in]);
            fprintf(stderr,"ERROR,usage:%s -s <serverName> -g <communicationPort> -n <userName>-p <sessionPort>\n",argv[0]);
            
            exit(1);
        }
    }
    
    printf("----------------------------------------------\n");
    printf("              Client  aka %s           \n", clientName);
    printf("----------------------------------------------\n");
    
    /*
     Steps for starting the comunication
     1) request comunicaton to other client from the server
     2) communicate with client
     */
    //generating local key -39* 65)  % 27
    for (int i =0;i<16;i++){
        myKey[i] = clientName[i % strlen(clientName)] ;
    }
    printf("MyKey: %s \n \n", myKey);
    printf("... No Secure Session is available ...\n");
    int choice;
    while(1){
        printf("Please select from the folowwing options:\n");
        printf(" 1) request secure session to a party.\n");
        printf(" 2) wait for secure session to start.\n");
        printf( " : ");
        
        scanf("%d",&choice);
        if (choice ==1){
            
            printf("Type the name of the client: ");
            fflush(stdout);
            scanf("%s",otherClinetName);
            strcpy(buffpacket.to,otherClinetName);
            
            
            
            if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
            {
                perror("socket");
                exit(1);
            }
            
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(portno);
            server_addr.sin_addr = *((struct in_addr *)host->h_addr);
            bzero(&(server_addr.sin_zero),8);
            
            unsigned int addr_len = sizeof(struct sockaddr);
            //seting up the request packet
            buffpacket.type[0] = 'R';
            strcpy(buffpacket.from, clientName);
            buffpacket.nonce = currentNonce;
            
            //sending the request
            buffSize = buffpacket.length+sizeof(buffpacket.from)+sizeof(buffpacket.to)+sizeof(char)+ 2 * sizeof(int);
            int n = sendto(sock, &buffpacket,buffSize, 0,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));
            
            //getting the time of the send requset
            gettimeofday(&t0,NULL);
            tm = localtime(&t0.tv_sec);
            printf("reqenst send at %d:%02d:%02d:%d \n", tm->tm_hour,tm->tm_min,tm->tm_sec,t0.tv_usec/1000);
            printf("Sent Size: %d \n", n);
            printf("nonce:%d",currentNonce);
            printf("paket length:%d \n",buffpacket.length);
            bzero(buffpacket.payload, MAX_SIZE);
            
            printf("my Key: %s \n",myKey);
            
            
            while(1){
                gettimeofday(&t1,NULL);
                r = recvfrom(sock, &buffpacket,MAX_SIZE, 0,(struct sockaddr *)&server_addr, &addr_len);
                if (r<0){
                    error("Couldn't recive socket! \n");
                }
                
                //getting the time of the send requset
                gettimeofday(&t0,NULL);
                tm = localtime(&t0.tv_sec);
                printf("Packet recived at %d:%02d:%02d:%d \n", tm->tm_hour,tm->tm_min,tm->tm_sec,t0.tv_usec/1000);
                
                
                printf("Server IP: %s \n", inet_ntoa(server_addr.sin_addr));
                printf("paket length:%d \n",buffpacket.length);
                printf("Packet nonce #:%d ",buffpacket.nonce);
                
                if ( currentNonce == buffpacket.nonce){
                    printf(" -confirmed!\n");
                }
                else{
                    printf("-wrong!\n");
                    error("Couldn't confirm nonce! \n");
                }
                printf("payload length:%d \n",buffpacket.length);
                
                printf("payload: %s\n",buffpacket.payload);
                printf("\n");
                
                for(int i = 0;i<16;i++)
                {
                    tempbuff[i] = buffpacket.payload[i];
                }
                decryptMsg(sessionKey,tempbuff,myKey,16);
                bzero(tempbuff,sizeof(char)*17);
                for(int i = 0;i<16;i++)
                {
                    tempbuff[i] = buffpacket.payload[i+16];
                }
                printf("My Session Key: %s\n", sessionKey);
                printf("secret to send: %s\n", tempbuff);
                gettimeofday(&t2,NULL);
                timetaken +=  ((t2.tv_usec + t2.tv_sec * 1000000) - (t1.tv_usec + t1.tv_sec * 1000000));
                printf("douration: %lf ms\n", timetaken/1000);
                
                if(buffpacket.type[0]=='E'){
                    
                    printf("... Secure Session Key optained ...\n");
                    
                }
                //clearing the buffer
                bzero(buffpacket.payload,MAX_SIZE);
                //................sending secret session key to receptent .................
                printf("Type the IP address of %s to send the session key: ", otherClinetName);
                char *otherClinetIP = (char *) malloc(sizeof(char) * 30);
                scanf("%s", otherClinetIP);
                host= (struct hostent *) gethostbyname(otherClinetIP);
                bzero(&(server_addr.sin_zero),8);
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(sessionPort);
                server_addr.sin_addr = *((struct in_addr *)host->h_addr);
                buffpacket.type[0] = 'C';
                strcpy(buffpacket.from, clientName);
                strcpy(buffpacket.to, otherClinetName);
                strcpy(buffpacket.payload, tempbuff);
                buffpacket.length = 16;
                printf("paket length:%d \n",buffpacket.length);
                buffpacket.nonce = 0;
                //sending the secret key to the other client
                buffSize = sizeof(char) * buffpacket.length + sizeof(buffpacket.from) + sizeof(buffpacket.to) + sizeof(char) + 2 * sizeof(int) + 3;
                int n = sendto(sock, &buffpacket,buffSize, 0,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                gettimeofday(&t0,NULL);
                tm = localtime(&t0.tv_sec);
                printf("Packet sent at %d:%02d:%02d:%d \n", tm->tm_hour,tm->tm_min,tm->tm_sec,t0.tv_usec/1000);
                printf("Client IP: %s \n", inet_ntoa(client_addr.sin_addr));
                printf("Payload: %s \n",buffpacket.payload);
                //------- waiting responce to session estaplishment ------
                r = recvfrom(sock,&buffpacket,MAX_SIZE, 0,(struct sockaddr *)&client_addr, &addr_len);
                if (r<0){
                    error("Couldn't recive socket! \n");
                }
                if (buffpacket.type[0]=='C'){
                    printf("-- Request recived --\n");
                    printf("IP: %s \n", inet_ntoa(client_addr.sin_addr));
                    printf("From: %s\n", buffpacket.from);
                    printf("Payload: %s\n", buffpacket.payload);
                    bzero(tempbuff,sizeof(char)*17);
                    for(int i = 0;i<buffpacket.length;i++)
                    {
                        tempbuff[i] = buffpacket.payload[i];
                    }
                    
                    decryptMsg(msg,tempbuff,sessionKey,buffpacket.length);
                    currentNonce = atoi(msg);
                    
                    //-------Preparing nounce to be sent -------
                    bzero(buffpacket.payload,MAX_SIZE);
                    printf("Recived nonce: %d \n", currentNonce);
                    currentNonce += 1;
                    itoa(currentNonce,tempNouce,10);
                    encryptMsg(tempNouce,secret,sessionKey,10);
                    strcpy(buffpacket.payload,secret);
                    strcpy(buffpacket.from, clientName);
                    strcpy(buffpacket.to, otherClinetName);
                    buffpacket.length = 10;
                    printf("paket length:%d \n",buffpacket.length);
                    buffpacket.type[0] = 'C';
                    printf(" --Sending responce -- \n");
                    buffSize =sizeof(char) * buffpacket.length + sizeof(buffpacket.from) + sizeof(buffpacket.to) + sizeof(buffpacket.type)+ 2 * sizeof(int) + 3;
                    sendto(sock,&buffpacket,buffSize,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
                    
                    gettimeofday(&t0,NULL);
                    tm = localtime(&t0.tv_sec);
                    printf("Packet sent at %d:%02d:%02d:%d \n", tm->tm_hour,tm->tm_min,tm->tm_sec,t0.tv_usec/1000);
                    printf("Client IP: %s \n", inet_ntoa(client_addr.sin_addr));
                    printf("Payload: %s\n", buffpacket.payload);
                    bzero(buffpacket.payload, MAX_SIZE);
                    printf("-- wating for session to start --\n");
                }
                
                // --- Starting the secure cumnication  --------
                printf("The session will start when you recive respond, When you send message wait to get responce then you can send the next message\n\n");
                printf("------------------------------\n");
                while(1){
                    
                    r = recvfrom(sock,&buffpacket,MAX_SIZE, 0,(struct sockaddr *)&client_addr, &addr_len);
                    if (r<0){
                        error("Couldn't recive socket! \n");
                    }
                    
                    gettimeofday(&t0,NULL);
                    tm = localtime(&t0.tv_sec);
                    fflush(stdout);
                    printf("Recived at %d:%02d:%02d:%d  :\n", tm->tm_hour,tm->tm_min,tm->tm_sec,t0.tv_usec/1000 );
                    printf("Secret message: %s\n", buffpacket.payload);
                    strcpy(secret, buffpacket.payload);
                    decryptMsg(msg, secret, sessionKey, buffpacket.length);
                    printf("%s : %s \n", buffpacket.from, msg);
                    bzero(buffpacket.payload,MAX_SIZE);
                    bzero(msg,MAX_SIZE);
                    bzero(secret,MAX_SIZE);
                    printf("Me: ");
                    fgets(msg, MAX_SIZE,stdin);
                    if ((strlen(msg)>0) && (msg[strlen (msg) - 1] == '\n'))
                        msg[strlen (msg) - 1] = '\0';
                    encryptMsg(msg, secret, sessionKey, strlen(msg));
                    strcpy(buffpacket.payload, secret);
                    buffpacket.length = strlen(secret);
                    buffpacket.nonce = currentNonce;
                    strcpy(buffpacket.from,clientName);
                    strcpy(buffpacket.to, otherClinetName);
                    buffpacket.type[0] = 'C';
                    buffSize =sizeof(char) * buffpacket.length + sizeof(buffpacket.from) + sizeof(buffpacket.to) + sizeof(buffpacket.type) + 2 * sizeof(int) + 3;
                    sendto(sock,&buffpacket,buffSize,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
                    
                    bzero(buffpacket.payload,MAX_SIZE);
                    bzero(msg,MAX_SIZE);
                    bzero(secret,MAX_SIZE);
                    
                    
                }
                
            }
        }
        
        /*******************************************************************
         -------------- The other client side of code ----------------------
         *******************************************************************/
        else if( choice == 2){
            printf("port %d\n",sessionPort);
            if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
                perror("Socket");
                exit(1);
            }
            
            bzero(&(server_addr.sin_zero),8);
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(sessionPort);
            server_addr.sin_addr.s_addr = INADDR_ANY;
            
            
            if (bind(sock,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){
                perror("Bind");
                exit(1);
            }
            
            addr_len = sizeof(struct sockaddr);
            bzero(buffpacket.payload,MAX_SIZE);
            printf("-- Waiting for connection to start --\n");
            //---------reciving secret session key ---------
            r = recvfrom(sock,&buffpacket,MAX_SIZE, 0,(struct sockaddr *)&client_addr, &addr_len);
            if (r<0){
                error("Couldn't recive socket! \n");
            }
            if (buffpacket.type[0]=='C'){
                printf("-- Request recived --\n");
                printf("IP: %s \n", inet_ntoa(client_addr.sin_addr));
                printf("From: %s\n", buffpacket.from);
                strcpy(otherClinetName, buffpacket.from);
                printf("Payload: %s\n", buffpacket.payload);
                printf("paket length:%d \n",buffpacket.length);
                bzero(tempbuff,sizeof(char)*17);
                for(int i = 0;i<16;i++)
                {
                    tempbuff[i] = buffpacket.payload[i];
                }
                decryptMsg(sessionKey,tempbuff,myKey,16);
                printf("SessionKey: %s \n",sessionKey);
                //-------Preparing nounce to be sent -------
                bzero(buffpacket.payload,MAX_SIZE);
                currentNonce = rand();
                printf("nonce Sent:%d\n", currentNonce);
                itoa(currentNonce,tempNouce,10);
                printf ("text nonce: %s\n",tempNouce);
                encryptMsg(tempNouce,secret,sessionKey,10);
                printf("Encrypted Nonce: %s\n", secret);
                strcpy(buffpacket.payload,secret);
                strcpy(buffpacket.from, clientName);
                strcpy(buffpacket.to, otherClinetName);
                buffpacket.length = 33;
                printf("paket length:%d \n",buffpacket.length);
                buffpacket.type[0] = 'C';
                buffSize =sizeof(char) * buffpacket.length + sizeof(buffpacket.from) + sizeof(buffpacket.to) + sizeof(buffpacket.type) + 2 * sizeof(int) + 3;
                sendto(sock,&buffpacket,buffSize,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
                bzero(buffpacket.payload,MAX_SIZE);
                gettimeofday(&t0,NULL);
                tm = localtime(&t0.tv_sec);
                printf("Packet sent at %d:%02d:%02d:%d \n", tm->tm_hour,tm->tm_min,tm->tm_sec,t0.tv_usec/1000);
                printf("Client IP: %s \n", inet_ntoa(client_addr.sin_addr));
                printf("-- wating for confermation --\n");
            }
            //---------reciving secret session key ---------
            r = recvfrom(sock,&buffpacket,MAX_SIZE, 0,(struct sockaddr *)&client_addr, &addr_len);
            if (r<0){
                error("Couldn't recive socket! \n");
            }
            if (buffpacket.type[0]=='C'){
                printf("-- Respond recived --\n");
                printf("IP: %s \n", inet_ntoa(client_addr.sin_addr));
                printf("From: %s\n", buffpacket.from);
                printf("Payload: %s\n", buffpacket.payload);
                printf("paket length:%d \n",buffpacket.length);
                bzero(tempbuff,sizeof(char)*17);
                for(int i = 0;i<16;i++)
                {
                    tempbuff[i] = buffpacket.payload[i];
                }
                decryptMsg(msg,tempbuff,sessionKey,16);
                int rnonce = atoi(msg);
                printf("Reviced nounce: %d", rnonce);
                
                if ( rnonce == currentNonce + 1 ){
                    printf(" -confermed\n");
                }
                else
                    printf(" -faild\n");
                
            }
            // --- Starting the secure cumnication  --------
            printf(" \n\n ** Secure session is granted with %s **\n\n",otherClinetName);
            printf("When you send message wait to get responce then you can send the next message\n\n");
            printf("-----------------------------\n");
            while(1){
                printf("Me: ");
                fgets(msg, MAX_SIZE,stdin);
                if ((strlen(msg)>0) && (msg[strlen (msg) - 1] == '\n'))
                    msg[strlen (msg) - 1] = '\0';
                encryptMsg(msg, secret, sessionKey, strlen(msg));
                strcpy(buffpacket.payload, secret);
                buffpacket.length = strlen(secret);
                buffpacket.nonce = currentNonce;
                strcpy(buffpacket.from,clientName);
                strcpy(buffpacket.to, otherClinetName);
                buffpacket.type[0] = 'C';
                buffSize =sizeof(char) * buffpacket.length + sizeof(buffpacket.from) + sizeof(buffpacket.to) + sizeof(buffpacket.type) + 2 * sizeof(int) + 3;
                sendto(sock,&buffpacket,buffSize,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
                
                bzero(buffpacket.payload,MAX_SIZE);
                bzero(msg,MAX_SIZE);
                bzero(secret,MAX_SIZE);
                
                r = recvfrom(sock,&buffpacket,MAX_SIZE, 0,(struct sockaddr *)&client_addr, &addr_len);
                if (r<0){
                    error("Couldn't recive socket! \n");
                }
                
                gettimeofday(&t0,NULL);
                tm = localtime(&t0.tv_sec);
                fflush(stdout);
                printf("Recived at %d:%02d:%02d:%d \n", tm->tm_hour,tm->tm_min,tm->tm_sec,t0.tv_usec/1000 );
                printf("Secret message: %s\n", buffpacket.payload);
                strcpy(secret, buffpacket.payload);
                decryptMsg(msg, secret, sessionKey, buffpacket.length);
                printf("%s : %s\n", buffpacket.from, msg);
                bzero(buffpacket.payload,MAX_SIZE);
                bzero(msg,MAX_SIZE);
                bzero(secret,MAX_SIZE);
            }
            
        }
        
        else
            printf("Please enter 1 or 2 based on your choice!\n");
        
    }	
    return 0;
}

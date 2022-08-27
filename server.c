/* server.c    server take the arguments -p port -r rate -q seq_no -l length*/

#include "functions.c"

int main(int argc, char *argv[]){
    // Variables declerations 
    int sock,k;
    unsigned int addr_len;
    int bytes_read;
    struct sockaddr_in server_addr , client_addr;
    int portno= -1;
    int buffSize;
    // memory allocations 
    char *reciverKey = (char *) malloc(17*sizeof(char) );
    char *senderKey = (char *) malloc(17* sizeof(char));
    char *secretKey;
    secretKey = (char *) malloc(17 * sizeof(char));
    char *payload;
    payload = (char *) malloc(33 * sizeof(char) );
    char *secret1;
    secret1 = (char *) malloc(16 * sizeof(char));
    char *secret2;
    secret2 = (char *) malloc(16 * sizeof(char));

    struct tm *tm;
    struct timeval t0;
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
    
    printf("--------------------------------------------------\n");
    printf("|           Trusted Server  aka Kathy             |\n");
    printf("--------------------------------------------------\n");
    
    if (argc <3 ){
        fprintf(stderr,"ERROR,usage:%s -p <port> \n",argv[0]);
        exit(1);
    }
    
    int in;
    if (strncmp(argv[1],"-p",2)==0){
        portno = atoi(argv[2]);
    }
    else {
        fprintf(stderr,"ERROR, Wrong argument type '%s'\n",argv[in]);
        fprintf(stderr,"ERROR,usage:%s -p <port>",argv[0]);
        
        exit(1);
    }
    
    
    //printing info
    printf("port %d\n",portno);
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero),8);
    
    if (bind(sock,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){
        perror("Bind");
        exit(1);
    }
    
    addr_len = sizeof(struct sockaddr);
    //printf("\nServer Waiting for client on port %d",portno);
    
    while(1){
        bzero(buffpacket.payload,MAX_SIZE);
        printf("-- Waiting for requests --\n");
        bytes_read = recvfrom(sock,&buffpacket,MAX_SIZE, 0,(struct sockaddr *)&client_addr, &addr_len);
        if (buffpacket.type[0]=='R'){
            printf("-- Request recived --\n");
            printf("IP: %s \n", inet_ntoa(client_addr.sin_addr));
            printf("From: %s\n", buffpacket.from);
            printf("To: %s\n", buffpacket.to);
            
            //keys for both ends of cummunications

            //generating local key
            for (int i =0;i<16;i++){
                senderKey[i] = buffpacket.from[i % strlen(buffpacket.from)] ;
            }
            for (int i =0;i<16;i++){
                reciverKey[i] = buffpacket.to[i % strlen(buffpacket.to)] ;
            }
            printf (" ALice Key: %s\n",senderKey);


            // Generating session key for both
            for (int i =0;i<16;i++){
                secretKey[i] = rand()%255;
            }
            printf(" Bob key: %s \n",secretKey);

            //encrypting the key for the reciver
            encryptMsg(secretKey, secret1, reciverKey, 16);
            //encrypting the key for the sender
            encryptMsg(secretKey, secret2, senderKey, 16);

            for (int i=0;i<16;i++)
            {
                payload[i]=secret2[i];
            }
            for (int i=0;i<16;i++)
            {
                payload[i+16]=secret1[i];
            }
            
            printf("-- Generating responce --\n");
            int nonce = buffpacket.nonce ;
            printf("payload: %s", payload);
            
            //generating replay packet
            buffpacket.type[0] = 'R';
            strcpy(buffpacket.to,buffpacket.from);
            strcpy(buffpacket.from,"Server");
            buffpacket.nonce = nonce;
            buffpacket.length = 32;
            strcpy(buffpacket.payload, payload);
            
            buffSize =sizeof(char)* 32 + sizeof(buffpacket.from) + sizeof(buffpacket.to) + sizeof(buffpacket.type) + 2 * sizeof(int) + 3;
            sendto(sock,&buffpacket,buffSize,0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
            
            bzero(payload, sizeof (char) * 33 );
            
            gettimeofday(&t0,NULL);
            tm = localtime(&t0.tv_sec);
            printf("Packet sent at %d:%02d:%02d:%d \n", tm->tm_hour,tm->tm_min,tm->tm_sec,t0.tv_usec/1000);
            printf("Client IP: %s \n", inet_ntoa(client_addr.sin_addr));
            
        }
        
        
        printf("-- Secret key have been Sent! --\n");
        
        
        fflush(stdout);
        
    }

    return 0;
}

# NS-encryption
Needham and Schroeder encryption messaging client-server application
### Underlying process:
1. Alice send request to communicatee with bob to the server plus nonce.
2. The server respond with payload of 2 versions of the session key, one encrypted with Alice key, the other is encryptd with Bob key, and put the same nonce alice sent.
3. Alice send Bob section from the payload to Bob along.
4. Bob receive and decrypt the session key with his key, generate a nonce and encrypt it with the session key, then send to Alice.
5. Alice receive the nonce, decrypt with the session key, add 1 to it then encrypt with the session key and send to bob.
6. bob receive and confirm the nonce.
7. session is confirmed and both can send messages encrypted with the session key

###Sequence Diagram
                    
```seq
Alice->Server: connect me to Bob 
Note right of Server: server validate\nAlice nonce 
Server-->Alice: encrypted Session keys 
Note right of Alice: Alice validate\nServer nonce 
Alice->Bob: Are you Bob?!
Note right of Bob: Bob decrypt\nand validate
Bob-->Alice: Yes!
Note right of Alice: Alice validate\nBob nonce
Alice->Bob: Are you sure?!
Note right of Bob: Bob validate\nnew nonce
Bob-->Alice: Yes, I am sure we can talk!
```
###End

### Compile
To Compile the program simple run make command
Note: "I used simple encryption and not AES, there might be some bugs in the communication, for example right when the session start the nonce will be sent again. The good part there is no known major problem, yet!."
to run the program:
1) run the server with argument -p <port number>:
>sudo ./server -p 445
2) in separated window or a computer run a client with arguments -s <Server name> -g <server port> -n <client name> -p <secure channel port>
>sudo ./client -s localhost -g 445 -n Alice -p 2225 
\n Chose option 2 to wait for connection
3) in separated window  or a computer run a client with arguments -s <Server name> -g <server port> -n <client name> -p <secure channel port>
>sudo ./client -s localhost -g 445 -n Bob -p 2225 
chose option 1 to request communication
when prompted put the name of the other party ( case sensitive, the server will use this name to know the other person encryption key) for example here Alice
when prompted put the server name or IP address of the other party for example here localhost
When the secure channel is granted start chatting when prompted

Arguments:
-s followed by server name or IP address: define the server
-g followed by a port number: define the server port
-n followed by username: to define a name for the client
-p followed bz a port number: define the client port number

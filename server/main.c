#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define PORT 13337 
#define SA struct sockaddr 
  
// Function designed for chat between client and server. 
void func(int sockfd) 
{ 
    int bufSize = 132;
    char buf[132] = { 0 };
    int n; 
    // infinite loop for chat
    read(sockfd, buf, bufSize);

    char charName[20];
    memcpy(charName, buf, 20);
    FILE* pFile;
    char* filePath;
    sprintf(filePath, "%s.DnD", charName);

    pFile = fopen(filePath,"wb");
    
    if (pFile){
        fwrite(buf, bufSize, 1, pFile);
        puts("Wrote to file!");
    }
    else{
        puts("Something wrong writing to File.");
    }

    fclose(pFile);
} 
  
// Driver function 
int main() 
{ 

    pid_t PID = fork();
    if(PID == 0) {
        // IN CHILD - PORT 13339 CHECK NAME
        int sockfd, connfd, len; 
        struct sockaddr_in servaddr, cli; 
    
        // socket create and verification 
        sockfd = socket(AF_INET, SOCK_STREAM, 0); 
        if (sockfd == -1) { 
            printf("socket creation failed...\n"); 
            exit(0); 
        } 
        else
            printf("Socket successfully created..\n"); 
        bzero(&servaddr, sizeof(servaddr)); 
    
        // assign IP, PORT 
        servaddr.sin_family = AF_INET; 
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
        servaddr.sin_port = htons(13339); 
    
        // Binding newly created socket to given IP and verification 
        if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
            printf("socket bind failed...\n"); 
            exit(0); 
        } 
        else
            printf("Socket successfully binded..\n"); 
    
        // Now server is ready to listen and verification 
        if ((listen(sockfd, 5)) != 0) { 
            printf("Listen failed...\n"); 
            exit(0); 
        } 
        else
            printf("Server listening..\n"); 
        len = sizeof(cli); 
        while(1) {
            // Accept the data packet from client and verification 
            connfd = accept(sockfd, (SA*)&cli, &len); 
            if (connfd < 0) { 
                printf("server acccept failed...\n"); 
                exit(0); 
            } 
            else
                printf("server acccept the client...\n"); 
        
            // Function for chatting between client and server 
            

            // GET NAME AND RETURN 0 or 1!
            char name[20];
            int exists = 1;
            
            recv(connfd, &name, 20, 0);
            printf("Received name\n");

            char filename[100];
            sprintf(filename, "%s.DnD", name);
            if (access(filename, F_OK) != -1) {
                // Name exists
                printf("Got to exists\n");
                exists = 1;
                send(connfd, &exists, sizeof(int), 0);
                printf("Sent exists\n");
            } else {
                // Name does not exist
                exists = 0;
                printf("Got to not exists\n");
                send(connfd, &exists, sizeof(int), 0);
                printf("Sent does not exist\n");
            }

        
            // After chatting close the socket 
            close(connfd);
        }

    }

    PID = fork();
    if(PID == 0) {
        // IN CHILD - PORT 13338 SEND BACK FILE
        int sockfd, connfd, len; 
        struct sockaddr_in servaddr, cli; 
    
        // socket create and verification 
        sockfd = socket(AF_INET, SOCK_STREAM, 0); 
        if (sockfd == -1) { 
            printf("socket creation failed...\n"); 
            exit(0); 
        } 
        else
            printf("Socket successfully created..\n"); 
        bzero(&servaddr, sizeof(servaddr)); 
    
        // assign IP, PORT 
        servaddr.sin_family = AF_INET; 
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
        servaddr.sin_port = htons(13338); 
    
        // Binding newly created socket to given IP and verification 
        if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
            printf("socket bind failed... Socket: %d\n", 13338); 
            exit(0); 
        } 
        else
            printf("Socket successfully binded..\n"); 
    
        // Now server is ready to listen and verification 
        if ((listen(sockfd, 5)) != 0) { 
            printf("Listen failed...\n"); 
            exit(0); 
        } 
        else
            printf("Server listening..\n"); 
        len = sizeof(cli); 
        while(1) {
            // Accept the data packet from client and verification 
            connfd = accept(sockfd, (SA*)&cli, &len); 
            if (connfd < 0) { 
                printf("server acccept failed...\n"); 
                exit(0); 
            } 
            else
                printf("server acccept the client...\n"); 
            
            // GET NAME, RETURN FILE
            char name[20];

            recv(connfd, &name, 20, 0);
            printf("Received Name: %s\n", name);

            char filename[150] = { '\0' };
            //printf("%s\n", filename);
            char cwd[100];
            getcwd(cwd, 100);
            sprintf(filename, "%s/%s.DnD", cwd, name);
            printf("%s\n", filename);
            char sendBuffer[132];
            FILE* pFile;
            errno = 0;
            pFile = fopen(filename,"r+b");
            //printf("%d", pFile);
            
            if (pFile){
                fread(&sendBuffer, sizeof(sendBuffer), 1, pFile);
                puts("Read From File");
            }
            else{
                printf("Error is: %d\n", errno);
                puts("Something wrong writing to File.");
            }

            fclose(pFile);

            send(connfd, sendBuffer, 132, 0);
            printf("Sent character file\n");
            close(connfd);
        }
    }
    
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 

    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 

    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 

    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 

    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    len = sizeof(cli); 
    while(1) {
        // Accept the data packet from client and verification 
        connfd = accept(sockfd, (SA*)&cli, &len); 
        if (connfd < 0) { 
            printf("server acccept failed...\n"); 
            exit(0); 
        } 
        else
            printf("server acccept the client...\n"); 
    
        // Function for chatting between client and server 
        func(connfd); 
    
        // After chatting close the socket 
        close(connfd); 
    }
    close(sockfd);
}  
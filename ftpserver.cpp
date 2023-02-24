#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h> 
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

#define BUFSIZ 1024

int main(int argc, char const* argv[])
{
    if (argc < 2){
        printf("\n Please enter the server port number.");
        return -1;
    }
    const int PORT = atoi(argv[1]);
    int server_fd, sock, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFSIZ] = { 0 };
    char* ack = "ack";
 
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket failed");
        return -1;
    }

    printf("Socket file descriptor created\n");
 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
    // attaching socket to the port
    if (bind(server_fd, (struct sockaddr*)&address,
             addrlen) < 0) {
        printf("bind failed");
        return -1;
    }
    
    if (listen(server_fd, 3) < 0) {
        printf("listen");
        exit(EXIT_FAILURE);
    }
    printf("Listening on port %d\n", PORT);
    
    if ((sock = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        printf("accept");
        return -1;
    }

    while(sock >= 0){
        valread = read(sock, buffer, BUFSIZ);
        printf("Command received - %s\n", buffer);
        send(sock, ack, strlen(ack), 0);
        printf("Command ack sent\n");

        string input, filename, command;
        input = buffer;
        string delimitter = " ";
        
        command = input.substr(0, input.find(delimitter));
        filename = input.substr(input.find(delimitter)+1, input.size());

        if (command == "upload"){
            string newFilename = "new" + filename;

            valread = read(sock, buffer, BUFSIZ);
            printf("file size is  %s\n", buffer);
            send(sock, ack, strlen(ack), 0);
            printf("File size ack sent\n");

            long fileSize = atol(buffer);
            long remainDataSize = fileSize;
            long lengthRead;

            FILE* upload_fp = fopen(newFilename.c_str(), "wb");
            if(upload_fp == NULL){
                printf("something went wrong!! couldn't open the file");
                return 1;
            }

            printf("\nStrarting to receive the file\n");
            while ((remainDataSize > 0) && ((lengthRead = recv(sock, buffer, BUFSIZ, 0)) > 0))
            {
                    fwrite(buffer, sizeof(char), lengthRead, upload_fp);
                    remainDataSize -= lengthRead;
                    // fprintf(stdout, "Receive %ld bytes and we hope :- %ld bytes\n", lengthRead, remainDataSize);
            }
            printf("\nFile received completely\n");
            fclose(upload_fp);
        }
        else if (command == "get"){
            // Open the file
            FILE* fp = fopen(filename.c_str(), "rb");
            if(fp == NULL){
                printf("something went wrong!! couldn't open the file");
                return 1;
            }

            // Seek to the end of file to get the total size
            fseek(fp, 0 , SEEK_END);
            long fileSize = ftell(fp);
            long remainDataSize = fileSize;
            long lengthRead;
            const char* fileSizeToSend = to_string(fileSize).c_str();
            send(sock, fileSizeToSend, strlen(fileSizeToSend), 0);
            printf("File size sent - %ld\n", fileSize);
            valread = read(sock, buffer, 1024);
            printf("File size ack received - %s\n", buffer);

            // Seek back to the begining to start sending the actual file
            fseek(fp, 0 , SEEK_SET);
            printf("\nStrarting to sent the file data \n");
            while( (remainDataSize > 0) && (lengthRead = fread(buffer, 1, sizeof(buffer), fp))>0 ){
                send(sock, buffer, lengthRead, 0);
                remainDataSize -= lengthRead;
            }
            printf("\nDone sending the file \n");
            fclose(fp);
        }
        
    }
    // closing the connected socket
    close(sock);

    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}
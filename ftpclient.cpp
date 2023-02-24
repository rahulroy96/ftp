#include <iostream>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

#define BUFSIZ 1024
 
int main(int argc, char const* argv[])
{
    if (argc < 2){
        printf("\n Please enter the server port number. use ./ftpclient <<PORT>> to run");
        return -1;
    }
    const int PORT = atoi(argv[1]);

    char* ack = "ack";

    int sock = 0, valread, client_fd;
    char buffer[BUFSIZ] = { 0 };

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    if ((client_fd
         = connect(sock, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1){
        printf("Please use the below commands to upload or download a file:\n");
        printf("1. upload <<filename>>\n");
        printf("2. get <<filename>>\n");
        printf("3. exit\n");
        string input, filename, command;
        string delimitter = " ";
        getline(cin, input);
        
        command = input.substr(0, input.find(delimitter));
        filename = input.substr(input.find(delimitter)+1, input.size());
        
        if (command == "exit"){
            break;
        }

        printf("command = %s, filename - %s", command.c_str(), filename.c_str());

        send(sock, input.c_str(), input.length(), 0);
        printf("Command message sent\n");
        valread = read(sock, buffer, BUFSIZ);
        printf("%s\n", buffer);
        if (command == "upload") {

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
            
            valread = read(sock, buffer, BUFSIZ);
            printf("File size ack received - %s\n", buffer);

            // Seek back to the begining to start sending the actual file
            fseek(fp, 0 , SEEK_SET);
            printf("\n Strarting to sent the file\n");
            while( (remainDataSize > 0) && (lengthRead = fread(buffer, 1, sizeof(buffer), fp))>0 ){
                send(sock, buffer, lengthRead, 0);
                remainDataSize -= lengthRead;
            }
            printf("\nDone uploading the file \n");

            fclose(fp);

        } else if (command == "get"){
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

            printf("\n Strarting to receive the file\n");
            while ((remainDataSize > 0) && ((lengthRead = recv(sock, buffer, BUFSIZ, 0)) > 0))
            {
                    fwrite(buffer, sizeof(char), lengthRead, upload_fp);
                    remainDataSize -= lengthRead;
            }
            printf("\nDone getting the file \n");
            fclose(upload_fp);
        }

    }

    
 
    // closing the connected socket
    close(client_fd);
    return 0;
}
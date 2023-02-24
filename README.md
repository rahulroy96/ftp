# Simple FTP Client Server
A  simple  version  of  ftp  client/server  software. The  user  can  issue  a  command  at  the  client  side:  “get  <filename>”, 
which is to retrieve a file from the server, or “upload < filename>”, which is to upload a file to the server. The file is split into chunks of 1K bytes and send in a loop.

### To run the server
1. g++ ftpserver.cpp -o ftpserver
2. ./ftpserver < PORT NUMBER >

### To run the client
1. g++ ftpclient.cpp -o ftpclient
2. ./ftpclient < PORT NUMBER >
3. Enter get < filename > to download file with filename from server
4. Enter upload < filename > to upload a file with filename to server

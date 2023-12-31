// Server side implementation of UDP client-server model
#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>
using namespace std;

//#define PORT	 8080
#define MAXLINE 1500

// Driver code
int main(int arc, char* argv[]) {
	int sockfd;
	char buffer[MAXLINE];
	const char *hello = "Hello from server";
	int port = atoi(argv[1]);
	char* name = argv[2];
	struct sockaddr_in servaddr, cliaddr;
	
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(port);
	
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr,
			sizeof(servaddr)) < 0 )
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	
	socklen_t len;
	int n;

	len = sizeof(cliaddr); //len is value/result
	ofstream myFile (name, ios::out | ios::binary);
    			
	while(1){
		//memset(buffer,'\0',MAXLINE);
		//taking out msg_waitall
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
					0, ( struct sockaddr *) &cliaddr,
					&len);
		if(strncmp (buffer, "Done",4) == 0){
			break;
		}

		else{
    			myFile.write (&buffer[1], n-1);
		}
//buffer[n] = '\0';
		unsigned char seq = (unsigned char) buffer[0];
		//printf("Client : %d\n", seq);
		sendto(sockfd, buffer, 1,
			MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
				len);
		//std::cout<<"Hello message sent."<<std::endl;
	}
	myFile.close();
	close(sockfd);
	return 0;
}


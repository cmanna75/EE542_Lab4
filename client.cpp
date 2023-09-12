// Client side implementation of UDP client-server model
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
#define PORT	 8080
#define MAXLINE 1024
#define SERVER_IP_ADDR "192.168.2.17"


// Driver code
int main(int argc, char* argv[]) {

	//command line input: file name, port number, ip address, ? directory if we have time)
	int sockfd;
	char buffer[MAXLINE];
	const char *hello = "Done";
	struct sockaddr_in	 servaddr;

	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	
	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);
	
	int n;
	socklen_t len;
	


	char buf[1500];
	ifstream myFile ("test/data.bin", ios::in | ios::binary);
    	while(myFile.read (buf, 1500)){


		sendto(sockfd, buf, strlen(buf),
			MSG_CONFIRM, (const struct sockaddr *) &servaddr,
				sizeof(servaddr));
		std::cout<<"Binary Sent."<<std::endl;
			
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
					MSG_WAITALL, (struct sockaddr *) &servaddr,
					&len);
		//buffer[n] = '\0';
		//std::cout<<"Server :"<<buffer<<std::endl;
	}
	sendto(sockfd, (const char *) hello, strlen(hello),
		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
			sizeof(servaddr));
	std::cout<<"done"<<std::endl;
	close(sockfd);
	return 0;
}


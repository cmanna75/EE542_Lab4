#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ctime>
#include <sys/mman.h>
#include <fcntl.h>
#include <arpa/inet.h> //need for htons and such
#include <netdb.h> //needed for gethostbyname

using namespace std;

//1500 mtu
#define PAYLOAD 1464
//9000 mtu
//#define PAYLOAD 8964

// Mutex for thread synchronization
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int filesize = 0;
int errnoVal = -1;
char *fileName;
char *data;

struct sockaddr_in serv_addr;
struct stat statbuf;

int totalPacketsArrived = 0;

struct dgram {
    uint32_t seqNum;
    uint32_t dataSize;
    char buf[PAYLOAD];
};

pthread_t thrd[1];
int sockfd;

// Function to handle errors
void error(const char *msg);

// Function to open and read a file
char *openFile(const char *str);

// Thread function for checking client
void *checkClient(void *arg);

int main(int argc, char *argv[]) {
    int portno, fileID, n;
    struct hostent *server;

    // Check for correct command line arguments
    if (argc < 5) {
        cout << "usage " << argv[0] << " hostname port fileID filename\n";
        exit(1);
    }

    portno = atoi(argv[2]);
    fileID = atoi(argv[3]);

    fileName = argv[4];

    // Create a socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("Error opening socket\n");
    }
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        cout << "No such host\n";
        exit(0);
    }

    // Initialize server address structure
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    long int sndsize = 50000000;

    // Set socket options for sending and receiving buffers
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *) &sndsize, (long int) sizeof(sndsize)) == -1) {
        cout << "Error setting socket\n";
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &sndsize, (long int) sizeof(sndsize)) == -1) {
        cout << "Error setting socket\n";
    }

    unsigned char buffer[PAYLOAD];
    unsigned char *chptr = NULL;
    int seq = 0;

    FILE *fp = NULL;

    // Initialize buffer with null bytes , added char casting
    if ((chptr = (unsigned char*)memset(buffer, '\0', PAYLOAD)) == NULL) {
        cout << "Failed memset";
    }

    // Open the file for reading
    if ((fp = fopen(fileName, "r")) == NULL) {
        error("Failed to open");
    }

    // Get the size of the file
    if (stat(fileName, &statbuf) == 0) {
        filesize = statbuf.st_size;
        cout << "The size " << filesize << " bytes\n";
    }

    // Send the file size to the server multiple times for redundancy
    for (int i = 0; i < 10; i++) {
        n = sendto(sockfd, &filesize, sizeof(filesize), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
        if (n < 0)
            error("Failed to send");
    }

    // Lock mutex to read the file
    pthread_mutex_lock(&lock);

    // Read the file into memory
    data = openFile(fileName);
    int datasize = statbuf.st_size;

    // Unlock mutex
    pthread_mutex_unlock(&lock);

    // Create a thread to check for client responses
    if ((errnoVal = pthread_create(&thrd[0], 0, checkClient, (void *) 0))) {
        cout << "pthread_create[0] " << strerror(errnoVal) << "\n";
        pthread_exit(0);
    }

    // Loop to send data in packets
    while (datasize > 0) {
        int chunk, share;
        dgram sendDG;
        memset(&sendDG, 0, sizeof(dgram));

        share = datasize;
        chunk = PAYLOAD;

        if (share - chunk < 0) {
            chunk = share;
        } else {
            share = share - chunk;
        }
        // Lock mutex to access shared data
        pthread_mutex_lock(&lock);

        // Copy data into packet buffer
        memcpy(sendDG.buf, &data[seq * PAYLOAD], chunk);

        // Unlock mutex
        pthread_mutex_unlock(&lock);

        sendDG.seqNum = seq;
        sendDG.dataSize = chunk;

        usleep(100);

        // Send the packet to the server
        sendto(sockfd, &sendDG, sizeof(sendDG), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
        seq++;
        datasize -= chunk;
    }

    // Wait for the checkClient thread to finish
    pthread_join(thrd[0], 0);

    // Clean up and exit
    munmap(data, statbuf.st_size);
    fclose(fp);
    close(sockfd);
    return 0;
}

// Function to handle errors
void error(const char *msg) {
    cout << msg;
    exit(0);
}

// Function to open and read a file
char *openFile(const char *str) {
    int fp, pagesize;
    char *data;

    fp = open(str, O_RDONLY);
    if (fp < 0) {
        error("Failed to open file\n");
    }

    if (fstat(fp, &statbuf) < 0) {
        error("Failed file status");
    }

    // Map the file into memory
    data = (char *) mmap((caddr_t) 0, statbuf.st_size, PROT_READ, MAP_SHARED, fp, 0);

    if (data == MAP_FAILED) {
        error("Failed to map\n");
    }

    return data;
}

// Thread function for checking client
void *checkClient(void *arg) {
    int totalseq, lefbyte, size_p;
    dgram sendDG;
    int packetmiss, errnoVal;
    socklen_t servlen;
    servlen = sizeof(serv_addr);
    cout << "Client running\n";

    while (1) {
        ssize_t n;
        n = recvfrom(sockfd, &packetmiss, sizeof(int), 0, (struct sockaddr *) &serv_addr, &servlen);
        if (n < 0) {
            error("Failed to receive");
        }
        if (packetmiss < 0) {
            cout << "All packets sent !!!\n";
            pthread_exit(0);
        }

        totalseq = filesize / PAYLOAD;
        lefbyte = filesize % PAYLOAD;

        size_p = PAYLOAD;

        if (lefbyte != 0 && totalseq == packetmiss) {
            size_p = lefbyte;
        }
        // Lock mutex to access shared data
        pthread_mutex_lock(&lock);

        // Copy data into packet buffer
        memcpy(sendDG.buf, &data[packetmiss * PAYLOAD], size_p);
        pthread_mutex_unlock(&lock);

        sendDG.seqNum = packetmiss;
        sendDG.dataSize = size_p;

        // Send the packet to the server
        n = sendto(sockfd, &sendDG, sizeof(dgram), 0, (struct sockaddr *) &serv_addr, servlen);
        if (n < 0) {
            error("Error failed to send");
        }
    }
}

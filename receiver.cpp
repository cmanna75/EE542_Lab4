#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>
#include <cmath>
#include <fcntl.h>
#include <cstdint>
#include <arpa/inet.h> //need for htons and such
#include <netdb.h> //needed for gethostbyname

using namespace std;

//1500 MTU
#define MAXBUFSIZE 1464
//9000 MTU
//#define MAXBUFSIZE 8964

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Define a structure to represent datagrams
typedef struct dgram {
    uint32_t seqNum;
    uint32_t dataSize;
    char buf[MAXBUFSIZE];
} dgram;

char *fileName;        // Filename for the received data
char *rcvDGRAMS;       // Array to track received datagrams
int iteratingptr;      // Iterator for tracking received datagrams

pthread_t thrd[1];     // Thread for checking the server
int sockfd;            // Socket file descriptor
int filesize;          // Total size of the file being received
struct sockaddr_in serv_addr; // Server address

int totalPacketArrived = 0; // Total packets received
int totalSeq = 0;           // Total expected sequence numbers

void error(const char *msg);
int findUnavailDGRAM();
void *checkServer(void *arg);
int isNewDGRAM(int seqNum);

int main(int argc, char *argv[]) {
    int portNo, fileID, i = 0;
    socklen_t fromlen;
    int n;
    FILE *fp;
    int errnoVal = 0;

    dgram recvDG;
    int ack = -1;

    int datasize = 0;

    if (argc < 3) {
        cout << "usage " << argv[0] << " port fileID\n";
        exit(1);
    }

    fileID = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("Error failed to open socket\n");
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portNo = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNo);

    long int sndsize = 50000000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&sndsize, (long int)sizeof(sndsize)) == -1) {
        cout << "Error failed socket setup\n";
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&sndsize, (long int)sizeof(sndsize)) == -1) {
        cout << "Error failed setting the socket\n";
    }

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("Error binding socket\n");
    }
    fromlen = sizeof(struct sockaddr_in);

    n = recvfrom(sockfd, &filesize, sizeof(filesize), 0, (struct sockaddr *)&serv_addr, &fromlen);
    totalSeq = ceil(filesize / MAXBUFSIZE);
    cout << "Total datagram " << totalSeq << endl;

    fileName = (char *)malloc(sizeof(char) * 11);
    sprintf(fileName, "r%d.data.bin", fileID);
    fp = fopen(fileName, "w+");

    if ((errnoVal = pthread_create(&thrd[0], 0, checkServer, (void *)0))) {
        cerr << "pthread_create[0] " << strerror(errnoVal) << endl;
        pthread_exit(0);
    }

    rcvDGRAMS = (char *)calloc(totalSeq, sizeof(char));

    while (1) {
        n = recvfrom(sockfd, &recvDG, sizeof(dgram), 0, (struct sockaddr *)&serv_addr, &fromlen);

        if (n == sizeof(int)) {
            continue;
        }

        if (n < 0) {
            error("Error failed to recv\n");
        }

        pthread_mutex_lock(&lock);

        if (isNewDGRAM(recvDG.seqNum)) {
            fseek(fp, MAXBUFSIZE * recvDG.seqNum, SEEK_SET);
            fwrite(&recvDG.buf, recvDG.dataSize, 1, fp);
            fflush(fp);
            totalPacketArrived++;
        }

        pthread_mutex_unlock(&lock);

        if (totalPacketArrived == totalSeq + 1) {
            cout << "File received " << fileName << endl;
            cout << "Send acks" << endl;

            for (i = 0; i < 20; i++) {
                n = sendto(sockfd, &ack, sizeof(int), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
                if (n < 0) {
                    error("Error failed to send\n");
                }
            }
            break;
        }
    }

    free(fileName);

    pthread_join(thrd[0], 0);

    fclose(fp);

    close(sockfd);

    return 0;
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Find an unavailable datagram slot
int findUnavailDGRAM() {
    int i;
    if (rcvDGRAMS == NULL) {
        return -1;
    }

    for (i = iteratingptr; i <= totalSeq; i++) {
        if (rcvDGRAMS[i] == 0) {
            if (i == totalSeq)
                iteratingptr = 0;
            else
                iteratingptr = i + 1;
            return i;
        }
    }
    iteratingptr = 0;
    return -1;
}

// Function to check the server for missing datagrams
void *checkServer(void *arg) {
    int n = 0;
    int requestIndex;
    cout << "Server running" << endl;
    while (1) {
        if (totalPacketArrived == totalSeq + 1) {
            cout << "Total data received" << endl;
            pthread_exit(0);
        }
        usleep(100);
        pthread_mutex_lock(&lock);
        requestIndex = findUnavailDGRAM();

        pthread_mutex_unlock(&lock);

        if (requestIndex >= 0 && requestIndex <= totalSeq) {
            n = sendto(sockfd, &requestIndex, sizeof(int), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

            if (n < 0) {
                error("Error failed to send\n");
            }
        }
    }
}

// Check if a datagram is new or has been received before
int isNewDGRAM(int seqNum) {
    if (seqNum >= 0 && seqNum <= totalSeq) {
        if (rcvDGRAMS[seqNum] == 0) {
            rcvDGRAMS[seqNum] = 1;
            return 1;
        } else
            return 0;
    }
    return 0;
}

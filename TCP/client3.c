#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 4455
#define MAX_BUFFER_SIZE 1024

void bubbleSort(int arr[], int n) {
         
	for(int i=0;i<n-1;i++) {
		for(int j=0;j<n-i-1;j++) {
			if(arr[j] > arr[j+1]) {
				int temp = arr[j];
				arr[j] = arr[j+1];
				arr[j+1] = temp;
			}
		}
	}
}

int createTcpIPV4socket() {

	return socket(AF_INET, SOCK_STREAM, 0); // returns the file descriptor
}

struct sockaddr_in *createIPV4address(char *ip, int port) {

	struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
	address->sin_family = AF_INET;
	address->sin_port = htons(PORT);
        address->sin_addr.s_addr = inet_addr(ip);
	return address;
}

void handleClient(int clientSocket) {
    int numElements;
    int elements[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];
    if (recv(clientSocket, &numElements, sizeof(numElements), 0) <= 0) {
        perror("[-]Failed to receive number of elements");
        close(clientSocket);
        return;
    }
    numElements = ntohl(numElements); // convert data from network to host order 
    
    if (recv(clientSocket, elements, numElements * sizeof(int), 0) <= 0) {
        perror("[-]Failed to receive elements");
        close(clientSocket);
        return;
    }
    printf("Received data:----------\n");
    printf("The number of elements:\t%d\n",numElements);
    printf("The elements are:\t");
    for(int i=0;i<numElements;i++)
    {
        printf("%d\t",elements[i]);
    }
    printf("\n");
    // Sorting the elements
    bubbleSort(elements, numElements);
    // Send the sorted elements back to the client
    if (send(clientSocket, elements, numElements * sizeof(int), 0) <= 0) {
        perror("[-]Failed to send sorted elements");
    }

    close(clientSocket);
}

int main(int argc, char *argv[]) {
    int serverSocketFD, clientSocketFD;
    struct sockaddr_in *serverAddr, clientAddr;
    socklen_t addr_size;

    serverSocketFD = createTcpIPV4socket();
    if (serverSocketFD < 0) {
        perror("[-]Socket creation failed");
        exit(1);
    }
    printf("[+]Server socket created\n");
    char *ip = argv[1];
    int port =atoi(argv[2]);
    
    serverAddr = createIPV4address(ip, port);

    if (bind(serverSocketFD, (struct sockaddr *)serverAddr, sizeof(*serverAddr)) < 0) {
        perror("[-]Binding failed");
        close(serverSocketFD);
        exit(1);
    }

    free(serverAddr);
    printf("[+]Binding successful\n");
    if (listen(serverSocketFD, 10) < 0) { //Backlog is set to 10 means max 10 processes can be there in queue
        perror("[-]Listening failed");
        close(serverSocketFD);
        exit(1);
    }
    printf("[+]Server is listening at port 4455...\n");
    while (1) {
        addr_size = sizeof(clientAddr);
        clientSocketFD = accept(serverSocketFD, (struct sockaddr*)&clientAddr, &addr_size); // clientAddr will be overwritten with the address of the client and for each client one unique file descriptor(clientSocketFD) will be created in server
        if (clientSocketFD < 0) {
            perror("[-]Connection failed");
            continue;
        }
        printf("[+]Client connected\n");
        handleClient(clientSocketFD);
    }
    close(serverSocketFD);
    return 0;
}
// barcode.c

// Barcode Scanner Reader
// Dustin Holliday
// 02/03/2022

// This code opens a TCP/IP socket to a barcode scanner device and listens
// for barcode data to be passed back.  When data is recieved, it is printed
// back to stdout.

// Barcode scanner is model number BC9180
// Default IP address: 192.168.187.31
// Default Port:       51000

// TCP/IP Socket code adapted from Brian "Beej Jorgensen" Hall's
// wonderful guide "Beej's Guide to Network Programming"
// https://beej.us/guide/bgnet/html/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>


// Define the connection parameters that we will connect to
#define IP "192.168.187.31"
#define PORT "51000"

// Max number of bytes we can recieve at once
#define MAXDATASIZE 100

void *get_in_addr(struct sockaddr *);
int scanner_connect(int *, int);
int scanner_read(int *, int);
void scanner_close(int *);
void scanner_reconnect(int *, int);

int main(int argc, char* argv[])
{
	int sockfd;
	int connect_error;
	int read_error;

	// Verbosity command line switch
	int verbose = 0;

	if (argc > 1)
	{
		if (strcmp(argv[1], "-v") == 0)
		{
			verbose = 1;
		}
	}

	// Attempt to connect to the host
	// Try to reconnect every 5 seconds until successful
	connect_error = scanner_connect(&sockfd, verbose);

	if (connect_error)
	{
		if (verbose)
		{
			printf("client: retrying in 5 seconds...\n");
		}
		sleep(5);
		scanner_reconnect(&sockfd, verbose);
	}

	// Attempt to read from the host
	// If scanner_read() returns a 2 this means the server dropped the connection
	// Try to reconnect every 5 seconds until successful
	// If scanner_read() returns a 1, something horrible went wrong
	// and we exit the program
	while (1)
	{
		read_error = scanner_read(&sockfd, verbose);

		if (read_error == 2)
		{
			scanner_reconnect(&sockfd, verbose);
		}
		else if (read_error == 1)
		{
			scanner_close(&sockfd);
			exit(1);
		}
	}
}

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Function that connects to a specified socket
// determined by global defines IP and PORT
// Returns 0 if successful, 1 otherwise
int scanner_connect(int *sockfd, int verbose)
{
	int error = 0;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	int total;

	// Allocate data for our addrinfo struct
	memset(&hints, 0, sizeof hints);

	// Set socket parameters
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream type socket

	if (verbose)
	{
		printf("client: connecting...\n");
	}

	// Catch any errors thrown by getaddrinfo()
	// If successful, our structs will be filled
	if ((rv = getaddrinfo(IP, PORT, &hints, &servinfo)) != 0)
	{
		if (verbose)
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		}
		error = 1;
	}

	// Loop through the linked list of servers populated by
	// getaddrinfo() and connect to the first one that is successful
	for (p = servinfo; p!= NULL; p = p->ai_next)
	{
		// Get socket descriptor if socket() is successful
		// otherwise catch it's error value of -1
		// and continue to the next server in the linked list
		if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			if (verbose)
			{
				perror("client: socket");
			}
			continue;
		}

		// Connect to the socket descriptor if possible,
		// and catch it's error value of -1 if not
		if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			if (verbose)
			{
				perror("client: connect");
			}
			return 1;
		}
		break;
	}

	if (p == NULL)
	{
		if (verbose)
		{
			fprintf(stderr, "client: failed to connect\n");
		}
		return 1;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

	freeaddrinfo(servinfo);

	return error;
}

// Loop while the server keeps the connection alive
// and print all data recieved from the server to stdout
// Return 1 if recv() throws an error
// Return 2 if the server closes the connection
int scanner_read(int *sockfd, int verbose)
{
	int error = 0;
	int total = 0;
	int numbytes;
	char buf[MAXDATASIZE];

	while((numbytes = recv(*sockfd, &buf, sizeof (buf) - total, 0)) > 0)
	{
		total += numbytes;

		buf[numbytes] = '\0';

		printf("%s", buf);
	}

	if (numbytes == -1)
	{
		if (verbose)
		{
			perror("recv");
		}
		error = 1;
	}

	if (numbytes == 0)
	{
		if (verbose)
		{
			printf("server: connection closed\n");
		}
		error = 2;
	}

	return error;
}

// Close the socket
void scanner_close(int *sockfd)
{
	close(*sockfd);
}

// Attempt to reconnect every 5 seconds
void scanner_reconnect(int *sockfd, int verbose)
{
	int connect_error;
	
	do
	{
		connect_error = scanner_connect(sockfd, verbose);
		if (connect_error)
		{
			if (verbose)
			{
				printf("client: retrying in 5 seconds...\n");
			}
			sleep(5);
		}
	}
	while (connect_error);
	if (verbose)
	{
		printf("client: connection re-established\n");
	}
}

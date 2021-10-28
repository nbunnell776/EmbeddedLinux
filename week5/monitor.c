/*

        File: monitor.c

    A Posix thread to monitor console input for parameter changes
    Also includes functions to create and terminate the thread called
    from main() in the thermostat.c file

    -------------------------------------------------------

    Author: Nathan Bunnell
    Date: 10/24/2021
    Course: ECE-40105, Embedded Linux
    Assignment: Week 5 -- Network Programming

*/

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// Import #includes from netserve.c for network functions
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "thermostat.h"

const char *delims = " \t,=\n";

#define BUFLEN 80

// Func prototype for createServer()
int createServer(void);

//int client_socket, tempRead;
int tempRead;

pthread_mutex_t paramMutex;
pthread_t monitorT;

void *monitor (void *arg)
{
    char text[BUFLEN], *cmd;
    unsigned int value;

    // create server, test return value (socketNumber) for error
    int socketNumber = createServer();
    if (socketNumber == -1)
    {
    	printf("\n ERROR: return code from createServer() is %d\n", socketNumber);
    	exit(1);
    }

    while (1)
    {
    	// fgets (text, BUFLEN, stdin);
    	read (socketNumber, (void *)text, BUFLEN);
        cmd = strtok (text, delims);

        while (cmd)
        {
        	// Responses for valid and invalid entries; temperature read response buffer
			char okResponse[] = "OK";
			char invalidResponse[] = "???";
			char buffer[100];

			// Test moving this call into the cases we expect a value in to avoid segfaults
            //value = atoi (strtok (NULL, delims));
            
            pthread_mutex_lock (&paramMutex);   //get exclusive access to parameters 

            switch (*cmd)
            {
                case 's':
                	value = atoi (strtok (NULL, delims));
                	setpoint = value;
                	// Add response to parameter changes
                	write (socketNumber, okResponse, strlen(okResponse));
                    break;
                case 'l':
                	value = atoi (strtok (NULL, delims));
                	limit = value;
                	// Add response to parameter changes
					write (socketNumber, okResponse, strlen(okResponse));
					break;
                case 'd':
                	value = atoi (strtok (NULL, delims));
                	deadband = value;
                	// Add response to parameter changes
					write (socketNumber, okResponse, strlen(okResponse));
					break;
                case 'x':
                	// Add response for temp value query
                	snprintf(buffer, sizeof(buffer), "Temperature is: %d\n", tempRead);
                	write(socketNumber, buffer, sizeof(buffer));
                	break;

                // Add case to read current param values
                case 'p':
                	snprintf(buffer, sizeof(buffer), "SP is: %d\nLimit is: %d\nDB is: %d\n", setpoint, limit, deadband);
					write(socketNumber, buffer, strlen(buffer));
					break;

                default:
                	// Add response for invalid query
                	write(socketNumber, invalidResponse, strlen(invalidResponse));

                    break;
            }
            pthread_mutex_unlock (&paramMutex);     // release the parameters
            
            cmd = strtok (NULL, delims);
        }
    }
    return NULL;
}

#define CHECK_ERROR if (error) { \
        printf ("%s\n", strerror (error)); \
        return 1; }

int createThread ()
/*
    Creates the mutex and starts up the monitor thread
*/
{
    int error;
/*
    Create the Posix objects
*/
    error = pthread_mutex_init (&paramMutex, NULL);
    CHECK_ERROR;
    
    error = pthread_create (&monitorT, NULL, monitor, NULL);
    CHECK_ERROR;
    
    return 0;
}

void terminateThread (void)
/*
    Cancel and join the monitor thread
*/
{
	void *thread_val;
	
    pthread_cancel (monitorT);
    pthread_join (monitorT, &thread_val);
}

int createServer(void)
{
    int server_socket, client_socket, client_len;
//	int server_socket, client_len;
	struct sockaddr_in server_addr, client_addr;
//  char text[80];
//  int len, result;
    int result;

// Create unnamed socket and give it a "name"
    server_socket = socket (PF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    result = inet_aton (SERVER, &server_addr.sin_addr);
    if (result == 0)
    {
        printf ("inet_aton failed\n");
        exit (1);
    }
    server_addr.sin_port = htons (PORT);

// Bind to the socket
    result = bind (server_socket, (struct sockaddr *) &server_addr, sizeof (server_addr));
    if (result != 0)
    {
        perror ("bind");
        exit (1);
    }

// Create a client queue
    result = listen (server_socket, 1);
    if (result != 0)
    {
        perror ("listen");
        exit (1);
    }
    printf ("Network server running\n");

// Accept a connection
    client_len = sizeof (client_addr);
    client_socket = accept (server_socket, (struct sockaddr *) &client_addr, (socklen_t * __restrict__)&client_len);

    printf ("Connection established to %s\n", inet_ntoa (client_addr.sin_addr));

    return (client_socket);

}

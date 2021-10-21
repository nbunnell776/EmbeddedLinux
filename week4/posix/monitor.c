/*

        File: monitor.c

    A Posix thread to monitor console input for parameter changes
    Also includes functions to create and terminate the thread called
    from main() in the thermostat.c file

*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include "thermostat.h"

const char *delims = " \t,=";

#define BUFLEN 80

pthread_mutex_t paramMutex;
pthread_t monitorT;

void *monitor (void *arg)
{
    char text[BUFLEN], *cmd;
    unsigned int value;

    while (1)
    {
        fgets (text, BUFLEN, stdin);
        cmd = strtok (text, delims);

        while (cmd)
        {
            value = atoi (strtok (NULL, delims));
            
            pthread_mutex_lock (&paramMutex);   //get exclusive access to parameters 
            switch (*cmd)
            {
                case 's':
                    setpoint = value;
                    break;
                case 'l':
                    limit = value;
                    break;
                case 'd':
                    deadband = value;
                    break;
                default:
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

/*

        File: measure.c

    A simple data acquisition program that reads ADC channel 0
    and sends it to stdout. Default loop time is 2 seconds.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "libmc-pcf8591.h"
//#include "driver.h"

extern int errno;

int running = 1;

void done (int sig)
/*
    Signal handler to stop the program gracefully
*/
{
    running = 0;
}

int main (int argc, char *argv[])
{
    int value, fd;
    unsigned int wait, sample = 0;

    signal (SIGINT, done);  // set up signal handler
    if (argc > 1)           // get wait time
        sscanf (argv[1], "%d", &wait);
    else
        wait = 2;

    if ((fd = init_AD (0)) < 0)
    {
        printf ("Couldn't initialize A/D converter\n");
        exit (2);
    }
    while (running)
    {
        sleep (wait);
        if (read_AD (fd, &value) == 0)
        {
			printf ("Sample %d = %d\n", sample, value);
			sample++;
        }
    }
    printf ("Goodbye\n");
    close_AD (fd);
    return 0;
}

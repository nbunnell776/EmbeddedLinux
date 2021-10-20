/*

        File: devices.c

    This program provides simulated A to D input and digital outputs
    using the curses library.  Data is placed in shared memory.
*/
#define DRIVER

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <curses.h>

#include "driver.h"

shmem_t *p;
static const int led_mask[] = {1 << 13, 1 << 6, 1 << 26, 1 << 19};

const char *led (unsigned int which)
/*
    Returns a character string representing the state
    of 'which' led.
*/
{
	static const char *blanks = "  ";
	static const char *on = "ON";

	if ((p->GPIOmask & which))
        return on;
    return blanks;
}

void do_leds (int sig)
/*
    Signal handler invoked whenever a digital output is changed. In
    this case, we're only interested in the four outputs that are
    connected to LEDs.
*/
{
    int y, x;

    getyx (stdscr, y, x);
    move (3, 25);
    printw ("Cooler  %s    Alarm %s    LED3 %s    LED4 %s",
    			led (led_mask[COOLER]), led (led_mask[ALARM]),
				led (led_mask[2]), led (led_mask[3]));
    move (y, x);
    refresh();
}

int main (int argc,char *argv[])
/*
    After setup, the main loop just waits for keyboard input.
    Output is handled by the signal handler.
*/
{
    int id, running, status;
    struct sigaction act;

// Initialize and attach shared memory region
    if ((id = shmget ((key_t) SHM_KEY, sizeof (shmem_t), 0666 | IPC_CREAT)) < 0)
    {
        printf ("Can't get shared memory region\n");
        return 1;
    }
    if ((p = (shmem_t *) shmat (id, (void *) 0, 0)) == (shmem_t *) -1)
    {
        printf ("Can't attach shared memory region\n");
        return 2;
    }
    p->a2d = 50;    // initialize to mid range

// Initialize curses and screen labels
    initscr();
    move (1, 20);
    printw ("Thermostat Device Simulation");
    move (3, 5);
    printw ("A/D in:");

// Set upt the signal handler
    act.sa_handler = do_leds;
    sigemptyset (&act.sa_mask);
    act.sa_flags = SA_RESTART;
    status = sigaction (SIGUSR1, &act, 0);
    p->pid = getpid();  // so the device driver knows where to
                        // send the signal
    do
    {
        move (3, 15);
        running = scanw ("%d", &p->a2d);
    }
    while (running);

// All Done.  Detach shared memory region.
    shmdt (p);
// Detach curses
    endwin();

    return 0;
}




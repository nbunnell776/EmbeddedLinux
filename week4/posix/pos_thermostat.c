/*

        File: pos_thermostat.c

    A simple data acquisition program that reads ADC channel 0
    and sends it to stdout. Default loop time is 2 seconds.

    -------------------------------------------------------

    Author: Nathan Bunnell
    Date: 10/21/2021
    Course: ECE-40105, Embedded Linux
    Assignment: Week 4 -- Thermostat

    NOTES ON MODIFIED PROGRAM pos_thermostat.c

    - will implement a simple state machine using measure.c
    	as a reference with the addition of feedback in the
    	form of the target board's LEDs

	- set points will be hardcoded under the #defines section

	- state machine states will be defined as follows:
		- NORMAL_STATE = 0
			- tempRead < (setpoint - deadband)
			- COOLER_LED and ALARM_LED will be off
		- HIGH_STATE = 1
			- tempRead > (setpoint + deadband)
			- COOLER_LED will be on; ALARM_LED will be off
		- LIMIT_STATE = 2
			- tempRead > limit
			- COOLER_LED and ALARM_LED will be on
		- state transitions will be defined in state machine switch code

	- LED/states will manifest as follows:
		- COOLER_LED indicated by the BLUE LED will indicate
    		if the simulated tempRead value from the on-board
    		pot is above the (setpoint + deadband) value
    	- ALARM_LED indicated by the RED LED will indicate
    	 	if the simulated tempRead value from the on-board
    		pot is above limit

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <stdbool.h>

#include <pthread.h>
#include "thermostat.h"

#include "libmc-gpio.h"
#include "libmc-pcf8591.h"

#include "driver.h"

// Macro defs for states, LEDs, analog limit values, and the loop delay
#define NORMAL_STATE 0
#define HIGH_STATE 1
#define LIMIT_STATE 2

#define COOLER_LED 2	// Blue LED
#define ALARM_LED 0		// Red LED

// Overridden by variable defs setpoint, limit, deadband;
//#define SETPOINT_VAL 45
//#define LIMIT_VAL 55
//#define DEADBAND_VAL 2

// This should normally be 1 for a second delay in the loop
#define DELAY 1

// Debug message enable flag
#define DEBUG 0

// End macro defs

extern int errno;

unsigned int setpoint, limit, deadband;

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
	// Local variable declarations
	int tempRead, fd;
    unsigned int wait;

    // Create monitor thread and mutex, store result code in
    //  statusCode variable to test for success
    int statusCode = createThread();

	//  Test results; message and exit on error
	if (statusCode != 0)
	{
		printf("\n ERROR: return code from pthread_create is %d\n", statusCode);
        exit(1);
	}

    // Declare variable to track state machine, init to
    //  NORMAL_STATE for first pass
    unsigned int currentState = NORMAL_STATE;

    // Toggle flag used to blink alarm LED
    bool toggleFlag = 0;

    // Counter variable will be used to replace the wait-delay
    unsigned int counterVal = 0;

    signal (SIGINT, done);  // set up signal handler
    if (argc > 1)           // get wait time
        sscanf (argv[1], "%d", &wait);
    else
        wait = 2;

    // Import LED init call, referenced from ../led/led.c
    if ((fd = init_leds (REV_A)) < 0)
	{
		printf ("Couldn't initialize LEDs\n");
		return -1;
	}

    if ((fd = init_AD (0)) < 0)
	{
		printf ("Couldn't initialize A/D converter\n");
		exit (2);
	}

    while (running)
    {
    	if (DEBUG) printf ("DEBUG: Loop count is %d\n", counterVal);

    	/*
    	 * Test if the counter has delayed a reading long enough
    	 *   Skip the first pass when counter is equal to 0
    	 */
    	if (((counterVal % wait) == 0) && (counterVal != 0))
    	{
			if (read_AD (fd, &tempRead) == 0)
			{

				// Divide tempRead by 4 to increase granularity on the potentiometer
				//tempRead = tempRead / 4;

				printf ("Loop count is %d; Reading is %d\n", counterVal, tempRead);

				// Lock out our mutex to keep access exclusive to the thread during the switch
				pthread_mutex_lock (&paramMutex);

				// Main state machine logic
				switch(currentState)
				{
					/*
					 * NORMAL_STATE def:
					 * 		If tempRead > (setpoint + deadband)
					 * 			goto HIGH_STATE
					 * 			turn on COOLER_LED
					 * 			break
					 * 		Else If tempRead > (limit)
					 * 			goto LIMIT_STATE
					 * 			turn on COOLER_LED
					 * 			turn on ALARM_LED
					 * 			break
					 * 		Else
					 * 			no change of state
					 * 			break
					 */
					case(NORMAL_STATE):
						if (DEBUG) printf ("DEBUG: IN NORMAL_STATE\n");
						if ((tempRead > (setpoint + deadband)) && (tempRead < limit))
						{
							currentState = HIGH_STATE;
							ledON (COOLER_LED);
							break;
						}
						if (tempRead > limit)
						{
							currentState = LIMIT_STATE;
							ledON (COOLER_LED);
							ledON (ALARM_LED);
							break;
						}
						break;

					/*
					 * HIGH_STATE def:
					 * 		If tempRead < (setpoint - deadband)
					 * 			goto NORMAL_STATE
					 * 			turn off COOLER_LED
					 * 			break
					 * 		Else If tempRead > (limit)
					 * 			goto LIMIT_STATE
					 * 			turn on ALARM_LED
					 * 			break
					 * 		Else
					 * 			no change of state
					 * 			break
					 */
					case(HIGH_STATE):
						if (DEBUG) printf ("DEBUG: IN HIGH_STATE\n");
						if (tempRead < (setpoint - deadband))
						{
							currentState = NORMAL_STATE;
							ledOFF (COOLER_LED);
							break;
						}
						if (tempRead > limit)
						{
							currentState = LIMIT_STATE;
							ledON (ALARM_LED);
							break;
						}
						break;

					/*
					 * LIMIT_STATE def:
					 * 		If tempRead < (setpoint - deadband)
					 * 			goto NORMAL_STATE
					 * 			turn off COOLER_LED
					 * 			turn off ALARM_LED
					 * 			break
					 * 		Else If tempRead < (setpoint + deadband)
					 * 			goto HIGH_STATE
					 * 			turn off ALARM_LED
					 * 			break
					 * 		Else
					 * 			no change of state
					 * 			break
					 */
					case(LIMIT_STATE):
						if (DEBUG) printf ("DEBUG: IN LIMIT_STATE\n");
						if (tempRead < (setpoint - deadband))
						{
							currentState = NORMAL_STATE;
							ledOFF (COOLER_LED);
							ledOFF (ALARM_LED);
							break;
						}
						if ((tempRead > (setpoint + deadband)) && (tempRead < limit))
						{
							currentState = HIGH_STATE;
							ledON (COOLER_LED);
							ledOFF (ALARM_LED);
							break;
						}
						break;

					/*
					 * Catch if we're somehow outside of the defined states w/ a boundary test
					 */
					default:
						assert ((currentState >= NORMAL_STATE) && (currentState <= LIMIT_STATE));
						break;
				}

				// Release the mutex after exiting the switch
				pthread_mutex_unlock (&paramMutex);
			}
    	}

        /*
         * External to the bulk of the state machine logic, perform
         *   housekeeping operations:
         *   - Toggle ALARM_LED, if needed
         *   - Increment counterVal
         *   - Delay loop
         */

        // Test currentState, toggle ALARM_LED and toggleFlag if needed
    	//  I had attempted to use the getGPIOValue() function in place
    	//  of a flag but was unable to implement it in time
		if (currentState == LIMIT_STATE)
		{
			if (toggleFlag == 0)
			{
				ledON (ALARM_LED);
				toggleFlag = 1;
			}
			else
			{
				ledOFF (ALARM_LED);
				toggleFlag = 0;
			}
		}

		// Increment counterVal
		counterVal++;

		// Delay loop for DELAY seconds
		sleep (DELAY);

    }

    printf ("\nGoodbye!\n");
    close_AD (fd);
    close_leds();

	// Kill the main() thread prior to exiting
	terminateThread();

	return 0;

}

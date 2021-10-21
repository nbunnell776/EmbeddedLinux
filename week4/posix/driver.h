/*
    Driver functions for the "thermostat"
*/
int initAD (void);
int readAD (unsigned int channel);
void closeAD (void);

int initDigIO (void);
void setDigOut (unsigned int bit);
void clearDigOut (unsigned int bit);
void writeDigOut (unsigned int value);
unsigned int getDigIn (void);
void closeDigIO (void);
/*
	For simulation
*/
#ifdef DRIVER
#define SHM_KEY 1234		// shared memory key
typedef struct {
	unsigned int a2d;
	unsigned int leds;
	pid_t pid;
} shmem_t;
#endif
/*
    Digital I/O definitions
*/
#define LED1    0x1
#define LED2    0x2
#define LED3    0x4
#define LED4    0x8

#define SW1     0x1
#define SW2     0x8
#define SW3     0x20
#define SW4     0x40
#define SW5     0x80
#define SW6     0x800
/*
    Thermostat application definitions
*/
#define COOLER  LED1
#define ALARM   LED2


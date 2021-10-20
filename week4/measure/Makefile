
#  make measure  -- build measure.c for the target
#  make          -- build the simulation version of thermostat.c
#  make TARGET=1 -- build the target version of thermostat.c
#  make devices	 -- build the simulated devices program

#  Note that the only target whose build changes with the TARGET symbol
#  is thermostat.o.  Everything else builds in the appropriate environment

CFLAGS += -Wall -g -Og
INC := ../includes

ifdef TARGET
all: thermostat_t
thermostat.o: thermostat.c $(INC)/driver.h		# compile for the ARM
	$(CC) $(CFLAGS) -c -I$(INC) thermostat.c
else
all: thermostat_s
thermostat.o: thermostat.c $(INC)/driver.h	# compile for x86
	gcc $(CFLAGS) -c -I$(INC) thermostat.c
endif

thermostat_s : thermostat.o
	gcc -o $@ -lsim -L../sim-lib $^

devices : devices.c $(INC)/driver.h
	gcc -g -o $@ -I$(INC) -lncurses devices.c

measure : measure.o
	$(CC) -o $@ -L../pi-lib -lmchw $^
measure.o : measure.c $(INC)/driver.h
	$(CC) $(CFLAGS) -c  -I$(INC) measure.c

thermostat_t : thermostat.o
	$(CC) -o  $@ -L../pi-lib -lmchw $^

clean :
	rm -f *.o *_t *_s *~ core

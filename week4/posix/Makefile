#  Posix threads version of pos_thermostat

#  make          -- build the simulation version of pos_thermostat.c
#  make TARGET=1 -- build the target version of pos_thermostat.c

CFLAGS += -Wall -g -Og
INC := ../includes

ifdef TARGET
all: clean pos_thermostat_t
pos_thermostat.o: pos_thermostat.c $(INC)/driver.h $(INC)/thermostat.h		# compile for the ARM
	$(CC) $(CFLAGS) -c -I$(INC) pos_thermostat.c
monitor.o : monitor.c $(INC)/driver.h $(INC)/thermostat.h
	$(CC) $(CFLAGS) -c  -I$(INC) monitor.c
else
all: clean pos_thermostat_s
pos_thermostat.o: pos_thermostat.c $(INC)/driver.h $(INC)/thermostat.h	# compile for x86
	gcc $(CFLAGS) -c -I$(INC) pos_thermostat.c
monitor.o : monitor.c $(INC)/driver.h $(INC)/thermostat.h
	gcc $(CFLAGS) -c  -I$(INC) monitor.c
endif

pos_thermostat_s : pos_thermostat.o monitor.o
	gcc -o $@ $^ -lpthread -lsim -L../sim-lib

pos_thermostat_t : pos_thermostat.o monitor.o
	$(CC) -o $@ $^ -lpthread -lmchw -L../pi-lib

clean :
	rm -f *.o *_t *_s *~ core

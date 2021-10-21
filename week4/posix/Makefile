#  Posix threads version of thermostat

#  make          -- build the simulation version of thermostat.c
#  make TARGET=1 -- build the target version of thermostat.c

CFLAGS += -Wall -g
CC_ARM := arm-linux-gcc
INC_ARM := ../../include

ifdef TARGET
all: thermostat_t
thermostat.o: thermostat.c driver.h thermostat.h		# compile for the ARM
	$(CC_ARM) $(CFLAGS) -c -I$(INC_ARM) thermostat.c
monitor.o : monitor.c driver.h thermostat.h
	$(CC_ARM) $(CFLAGS) -c  -I$(INC_ARM) monitor.c
else
all: thermostat_s
thermostat.o: driver.h thermostat.h	# use the implicit rule for x86 compilation
monitor.o : monitor.c driver.h thermostat.h
endif

# Use the driver code in the measure directory and assume it's compiled.

thermostat_s : thermostat.o monitor.o ../measure/simdrive.o
	$(CC) -o $@ $^ -lpthread

thermostat_t : thermostat.o monitor.o ../measure/trgdrive.o
	$(CC_ARM) -o $@ $^ -lpthread

clean :
	rm -f *.o *_t *_s *~ core

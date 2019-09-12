
srcdir = .
LDFLAGS =  -Wl,-elf2flt="-r -z -s 32768"
LIBS =  
CFLAGS = -O2 -fomit-frame-pointer -fno-builtin -fpic -msingle-pic-base


# Change these if necessary

CC = arm-elf-g++ 
CPP = arm-elf-g++ -E

FILES = Watchdog
 
all:	$(FILES)
	chmod a+x $(FILES)

Watchdog:	MoxaWatchdog.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS) -D_STLP_UNIX

clean:
	rm -f *.o Watchdog *.gdb $(FILES) 



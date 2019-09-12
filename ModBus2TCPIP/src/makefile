

srcdir = .
LDFLAGS =  
LIBS =  
CFLAGS = -g

# Change these if necessary

CC = g++ 
CPP = g++

FILES = MoxaGateway GatewayDemon Watchdog Web_wd rebootd logger
 
all:	$(FILES)
	chmod 755 $(FILES)
MoxaGateway:	
	arm-elf-g++ -I../openssl-1.0.2c/include -L../openssl-1.0.2c/ -L../../openssl-1.0.2c/lib Utility.cpp  ModbusController.cpp ServerController.cpp GatewayCom.cpp LodamModbusProject.cpp -o MoxaGateway -Wl,-elf2flt="-r -z -s 32768" -lssl -lcrypto -D_STLP_UNIX -lpthread -lmoxalib -static

GatewayDemon:
	 arm-elf-gcc  -O2 -fomit-frame-pointer -fno-builtin -fpic -msingle-pic-base  -Wl,-elf2flt="-r -z -s 32768"  GatewayDemon.c -o GatewayDemon

Watchdog:
	arm-elf-g++ -O2  -Wl, -elf2flt -o Watchdog Watchdog.c -D_STLP_UNIX -lpthread -lmoxalib

Web_wd:
	arm-elf-g++ -O2  -elf2flt -o Web_wd Web_wd.c -D_STLP_UNIX -lpthread -lmoxalib

rebootd:
	arm-elf-g++ -Wl, -elf2flt -o rebootd rebootd.c -D_STLP_UNIX -lpthread

logger:
	arm-elf-g++ -O2  -elf2flt -o logger logger.c -D_STLP_UNIX -lpthread

install:
	cp GatewayDemon /home/Moxa/
	cp Watchdog /home/Moxa/
	cp MoxaGateway /home/Moxa/
kill:
	killall -9 MoxaGateway;killall -9  GatewayDemon;killall -9  Watchdog;
clean:
	rm -f *.o MoxaGateway logger *.gdb $(FILES) 

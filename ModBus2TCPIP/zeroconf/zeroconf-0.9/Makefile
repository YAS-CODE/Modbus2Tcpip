
Q=@

CWARN= -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wchar-subscripts -Wcomment -Wformat=2 -Wno-format-extra-args -Wimplicit-int -Werror-implicit-function-declaration -Wmain -Wmissing-braces -Wparentheses -Wswitch -Wundef -Wshadow -Wwrite-strings

CFLAGS= -MMD -O2 -g

SRCS=zeroconf.c delay.c

OBJS= $(SRCS:.c=.o)

TARGET=zeroconf

all: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS) *.d

uclinux-build:
	arm-elf-gcc -O2 -fomit-frame-pointer -fno-builtin -fpic -msingle-pic-base  -Wl,-elf2flt="-r -z -s 32768" delay.c zeroconf.c -o zeroconf

zeroconf: zeroconf.o delay.o
	$(Q)echo "Creating $@"
	$(Q)$(CC) $(CFLAGS) -o $@ $^ $(LIBS)


# Automatic dependency generation
# make the 'deps' variable equal to all *.c files
# and replace the '*.c' with '*.d' in the file name
#
deps := ${patsubst %.c,%.d,${wildcard *.c}}

# Make '*.d' dependant on '*.c' and specify how to
# invoke the compiler to turn one ('*.c') into another ('*.d')
${deps}: %.d : %.c
	$(Q)$(CC) -MM $(INCLUDES) $< > $@

ifneq (${MAKECMDGOALS},clean)
-include ${deps}
endif

# replace the inbuilt compilation rule so we get nice output
%.o: %.c
	$(Q)echo "Compiling $<"
	$(Q)$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 

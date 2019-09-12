EXEC = mDNSResponder
#OBJS = Responder.o mDNSPosix.o GenLinkedList.o mDNSUNP.o mDNS.o DNSCommon.o mDNSDebug.o uDNS.o DNSDigest.o
OBJS = mDNSPosix.o GenLinkedList.o mDNSUNP.o mDNS.o DNSCommon.o mDNSDebug.o uDNS.o DNSDigest.o Responder.o PlatformCommon.o

CFLAGS += -DPID_FILE=\"/var/run/mdnsd.pid\" -DMDNS_UDS_SERVERPATH=\"/var/run/mdnsd\"
CFLAGS += -DNOT_HAVE_SA_LEN -DUSES_NETLINK -DHAVE_LINUX -DNOT_HAVE_DAEMON
CFLAGS += -Wall -g

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS_$@))

clean:
	rm -f $(EXEC) *.elf *.gdb *.o

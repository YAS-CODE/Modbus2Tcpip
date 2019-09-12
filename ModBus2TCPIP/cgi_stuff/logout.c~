#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>



#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include <sys/signal.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/statvfs.h>
#include <sys/reboot.h>




void showlogin(){
	puts("Content-type: text/html\n");
	puts("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">");
	puts("<HTML>");
	puts("<HEAD>");
	puts("<TITLE></TITLE>");
	puts("<META http-equiv=Refresh Content=\"0; Url=login.cgi\">");
	puts("</HEAD>");
	puts("</HTML>");
}
int main(int argc, char* argv[])
{

	FILE *ptr;
	// struct ifreq ifr;
	

	//if( access( "/home/etc/auth", F_OK ) == -1 ) {
		remove("/home/etc/auth");
		
	//}
	showlogin();
	return 0;
}

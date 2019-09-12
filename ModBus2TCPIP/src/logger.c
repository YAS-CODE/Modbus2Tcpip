/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg) {
	perror(msg);
	//exit(1);
}

unsigned get_file_size (const char * file_name)
{
	struct stat sb;
	if (stat (file_name, & sb) != 0) {
		//exit (EXIT_FAILURE);
		return 0;
	}
	//printf("yas %d: file_size=%ld func=%s, file=%s\n",__LINE__,sb.st_size,__FUNCTION__,__FILE__);
	return sb.st_size;
}

/*int setlogname(char logfilename, int logfilecount){
	do{
		sprintf(logfilename, "Gateway%d.log",logfilecount);
	}while(access( "/home/etc/dailyreset", F_OK ) != -1);
}*/

int main(int argc, char **argv) {
	int sockfd; /* socket */
	int portno; /* port to listen on */
	unsigned clientlen; /* byte size of client's address */
	struct sockaddr_in serveraddr; /* server's addr */
	struct sockaddr_in clientaddr; /* client addr */
	struct hostent *hostp; /* client host info */
	char buf[BUFSIZE]; /* message buf */
	char *hostaddrp; /* dotted decimal host addr string */
	int optval; /* flag value for setsockopt */
	int n; /* message byte size */
	FILE * fdev;
	char timestamp[50];
	long logfilecount=0;
	char logfilename[50];

	/* 
	 * check command line arguments 
	 */
	portno = atoi("48000");

	/* 
	 * socket: create the parent socket 
	 */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	/* setsockopt: Handy debugging trick that lets 
	 * us rerun the server immediately after we kill it; 
	 * otherwise we have to wait about 20 secs. 
	 * Eliminates "ERROR on binding: Address already in use" error. 
	 */
	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

	/*
	 * build the server's Internet address
	 */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);

	/* 
	 * bind: associate the parent socket with a port 
	 */
	if (bind(sockfd, (struct sockaddr *) &serveraddr,sizeof(serveraddr)) < 0) 
		error("ERROR on binding");

	/* 
	 * main loop: wait for a datagram, then echo it
	 */
	clientlen = sizeof(clientaddr);
	/*fdev = fopen ("Gateway_log.log","wb");
	  if (fdev == NULL)
	  {
	  printf("Error opening file! /home/Moxa/last_devs\n");
	//fclose(fmac);
	return 1;

	}*/

	mkdir("/mnt/sd/gateway_logs", 0755);

	time_t t;
	struct tm tm;

	sprintf(logfilename, "/mnt/sd/Gateway%d.log",logfilecount++);


	while (1) {


		/*
		 * recvfrom: receive a UDP datagram from a client
		 */
		bzero(buf, BUFSIZE);
		n = recvfrom(sockfd, buf, BUFSIZE, 0,(struct sockaddr *) &clientaddr, &clientlen);
		if (n < 0)
			error("ERROR in recvfrom");

		t = time(NULL);

		tm = *localtime(&t);
		sprintf(timestamp,"%d-%d-%d %d:%d:%d :-", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);


		if( access( logfilename, F_OK ) != -1 ){ // if file exist 

			if( get_file_size (logfilename)>90000){
				do{
					sprintf(logfilename, "/mnt/sd/Gateway%d.log",logfilecount++);
				}while(access( logfilename, F_OK ) != -1);
				fdev = fopen (logfilename,"wb");
			}
			else
				fdev = fopen (logfilename,"ab");
		}
		else
			fdev = fopen (logfilename,"wb");
		if (fdev == NULL)
		{
			sleep(1);
			//printf("Error opening file! Gateway_log.log\n");
			//fclose(fmac);
			continue;

		}


		fwrite(timestamp, 1, strlen(timestamp), fdev);
		fwrite(buf, 1, n, fdev);
		fwrite("\n", 1, 1, fdev);
		fclose(fdev);
		/* 
		 * gethostbyaddr: determine who sent the datagram
		 */
#if 0
		hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		if (hostp == NULL)
			error("ERROR on gethostbyaddr");
		hostaddrp = inet_ntoa(clientaddr.sin_addr);
		if (hostaddrp == NULL)
			error("ERROR on inet_ntoa\n");
		printf("server received datagram from %s (%s)\n",hostp->h_name, hostaddrp);
		printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);

		/* 
		 * sendto: echo the input back to the client 
		 */
		n = sendto(sockfd, buf, strlen(buf), 0,	(struct sockaddr *) &clientaddr, clientlen);
		if (n < 0) 
			error("ERROR in sendto");
#endif
	}
}

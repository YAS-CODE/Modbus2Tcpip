/*
   ============================================================================
Name        : MoxaWatchdog.c
Author      : Imitaz Ahmed
Version     :
Copyright   : Your copyright notice
Description : Hello World in C, Ansi-style
============================================================================
 */


#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
//#include <sstream>
#include <iostream>
#include <stdio.h>

#include <stdlib.h>     /* strtoul */
#include <sys/stat.h>
#include <errno.h>
#include <sys/reboot.h>




bool wait_for_web=true;


char mDNSCommand[100];
char hostcommand[50];


unsigned char mac_address[6];
char last_devices[50];

int readMacAddr(unsigned char* mac){
	FILE * fmac;
	long length;
	fmac = fopen ("/home/Moxa/last_mac","rb");
	if (fmac == NULL)
	{
		fprintf(stderr,"Error opening file! data_cashe_temp\n");
		//fclose(fmac);
		return 1;

	}
	fseek (fmac, 0, SEEK_END);
	length = ftell (fmac);
	fseek (fmac, 0, SEEK_SET);
	if (length==6)
		fread (mac, 1, length, fmac);
	else{
		fclose(fmac);
		return 1;
	}
	fclose(fmac);
	return 0;
}

unsigned char* getMACAddress() {

	struct ifreq ifr;
	struct ifconf ifc;
	const int buf_length = 256;
	char buf[buf_length];
	int success = 0;
	int sock;
	int i;


	//zero out the array
	for(i = 0; i < 6; i++)
	{
		mac_address[i] = 0;
	}

	if(readMacAddr(mac_address)){
		do{
			sleep(1);
			sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
			if (sock == -1) { /* handle error*/
				continue;// return mac_address;
			};

			ifc.ifc_len = buf_length;
			ifc.ifc_buf = buf;
			if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { /* handle error */
				continue;//return mac_address;
			}

			struct ifreq* it = ifc.ifc_req;
			const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

			for (; it != end; ++it) {
				strcpy(ifr.ifr_name, it->ifr_name);
				if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
					if (!(ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
						if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
							success = 1;
							break;
						}
					}
				} else { /* handle error */
				}
			}
			//free(it);
			//free(end);
			fprintf(stderr,"yas %d: success=%d func=%s, file=%s\n",__LINE__,success,__FUNCTION__,__FILE__);
		}while(!success);
		if (success) {
			memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);

		}
	}
	
	memset(mDNSCommand,0,100);
	sprintf(mDNSCommand,"/home/Moxa/mDNSResponder -t _http._tcp. -d local -n %02hhx%02hhx%02hhx%02hhx%02hhx%02hhx &",mac_address[0],mac_address[1],mac_address[2],mac_address[3],mac_address[4],mac_address[5]);

	fprintf(stderr,"yas %d: success=%d mac_address=%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx func=%s, file=%s\n",__LINE__,success,mac_address[0],mac_address[1],mac_address[2],mac_address[3],mac_address[4],mac_address[5],__FUNCTION__,__FILE__);

	return mac_address;

}

/**
 * Start the process
 */
void startProcess()
{

	printf("\n\rStarting mDNS..");
	system("killall -9 mDNSResponder");

	printf("starting mDNS command:- %s\n",mDNSCommand);
	sleep(5);
	system(mDNSCommand);

}

/**
 * Check if the process is running.
 */
int checkProcess()
{

	FILE *fp;
	char path[500];
	char* dnsApp = "mDNSResponder";

	int bFound = 0;

	/* Open the command for reading. */
	fp = popen("/bin/ps | grep mDNSResponder", "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		exit(1);
	}

	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path)-1, fp) != NULL) {
	  if (strstr(path, dnsApp) != NULL && strstr(path, "grep mDNSResponder ") == NULL ) {
		  //printf("\n\r %s", path);
		  bFound = 1;
		  break;
	  }
	}


	/* close */
	pclose(fp);

	return bFound;

}



void *webServer(void*)
{
	time_t current,boottime;
	double sec;
	time(&boottime);
	do{
	time(&current);
	sec = difftime(current,boottime);
	}while(sec<(60*5));
	system("killall -9 boa");
	sleep(5);
	wait_for_web=false;
	//system("sh /etc/iptable_modules;sh /etc/iptables.rules");
	//sleep(10);
}
void *wd_hang(void*){
	int optval = 1 ;
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[200];
	char cmd[50];

	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		//printf("Could not create socket");
	}
	//puts("Socket created");
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8636 );


	if (setsockopt(socket_desc,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int)) == -1) {
		perror("[Error] Socket configration Error") ;
	}
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("Watchdog bind failed. Error");
	}

	//puts("bind done");
	listen(socket_desc , 1);
	//Accept and incoming connection
	//puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while(1){
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if (client_sock < 0)
		{
			perror("accept failed");
		}
		//puts("Connection accepted");
		struct timeval timeout;
		timeout.tv_sec = 1000;
		timeout.tv_usec = 0;

		if (setsockopt (client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
			perror("[Error] Socket configration Error") ;
		write(client_sock , "getpid" , strlen("getpid"));
		memset(client_message,0,sizeof(client_message));

		while( (read_size = recv(client_sock , client_message , 200 , 0)) > 0 )
		{
			
			if(strncmp(client_message,"reboot_now=",strlen("reboot_now="))==0){
				sleep(5);
				reboot(RB_AUTOBOOT);
				printf("yas %d: its should reboot now ... func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__);
			}

			//        write(client_sock , client_message , strlen(client_message));
		}
		if(read_size == 0)
		{
			//puts("web client disconnected");
			//fflush(stdout);
		}
		else if(read_size == -1)
		{
			//perror("web client recv failed");
		}
		close(client_sock);

		sleep(1);
	}
}

int main( int argc, char *argv[] )
{
	pthread_t wd;
	pthread_t timer_thread;
	printf("\n\r****************************************\n");
	printf("Moxa Webserver Watchdog \n" );
	printf( "Version 0.0.4 \n");
	printf( "****************************************\n");
	
	getMACAddress();

	memset(hostcommand,0,50);
	sprintf(hostcommand,"hostname %02hhx%02hhx%02hhx%02hhx%02hhx%02hhx &",mac_address[0],mac_address[1],mac_address[2],mac_address[3],mac_address[4],mac_address[5]);
	printf("starting hostname command:- %s\n",hostcommand);
	system(hostcommand);
	pthread_create(&timer_thread, NULL, webServer, NULL);
	
	pthread_create(&wd, NULL, wd_hang, NULL);
	//while(wait_for_web);

	do
	{

		if(checkProcess() == 0)
		{
			startProcess();
		}
		else
		{
			//printf("\n\rMoxaGateway Process found running..\n\r");
			sleep(5);
		}


		sleep(5);
	}
	while(wait_for_web);
	
	//system("killall -9 mDNSResponder");
	//sleep(3);


	return 0;
}


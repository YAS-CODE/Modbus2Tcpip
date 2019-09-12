/*
   ============================================================================
Name        : MoxaWatchdog.c
Author      : Imitaz Ahmed
Version     :
Copyright   : Your copyright notice
Description : Hello World in C, Ansi-style
============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>


#include<sys/socket.h>
#include <sys/reboot.h>
#include<arpa/inet.h> //inet_addr
#include <moxadevice.h>
#include <stdarg.h>

#define CRASH_LIMIT 6

int total_crashes=0;
char moxagate[50];
long logfilecount=0;
FILE * log_f;

int fileExist(const char * pathname)
{
	struct stat st = {0};
	if (stat((char *)pathname, &st) != -1) {
		return 1;
	}
	else
		return 0;

}

unsigned get_file_size (const char * file_name)
{
	struct stat sb;
	if (stat (file_name, & sb) != 0) {
		fprintf (stderr, "'stat' failed for '%s': %s.\n",
				file_name, strerror (errno));
		//exit (EXIT_FAILURE);
		return 0;
	}
	
	return sb.st_size;
}


int stringlog(const char *buf, int len,...){
#if 1    
	if(1){
		char logbuf[1000];
		va_list arg;
		va_start(arg,buf);
		vsprintf(logbuf,buf,arg);
		va_end(arg);
		//sendto(loggerfd, logbuf, strlen(logbuf), 0,(struct sockaddr *) &loggeraddr, (unsigned)sizeof(loggeraddr));
		char timestamp[50];
		char logfilename[50];
		//mkdir("/mnt/sd/gateway_logs", 0755);

		if (fileExist("/mnt/sd/gateway_logs")) {
			time_t t;
			struct tm tm;


			sprintf(logfilename, "/mnt/sd/gateway_logs/Watchdog%ld.log",logfilecount);
			t = time(NULL);

			tm = *localtime(&t);
			sprintf(timestamp,"%d-%d-%d %d:%d:%d :-", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

			do{

				if( access( logfilename, F_OK ) != -1 ){ // if file exist 

					if( get_file_size (logfilename)>90000){
						do{
							sprintf(logfilename, "/mnt/sd/gateway_logs/Watchdog%ld.log",logfilecount++);
						}while(access( logfilename, F_OK ) != -1);
						log_f = fopen (logfilename,"wb");
					}
					else
						log_f = fopen (logfilename,"ab");
				}
				else
					log_f = fopen (logfilename,"wb");
			}while(log_f == NULL);

			if(len==0){
				len=strlen(logbuf);
			}

			fwrite(timestamp, 1, strlen(timestamp), log_f);
			fwrite(logbuf, 1, len, log_f);
			fwrite("\n", 1, 1, log_f);
			fclose(log_f);
		}
	}
#endif
}

int readCrashStatus(){
	char buffer[50];
	long length;
	FILE * f = fopen ("/home/Moxa/crashes", "r+");
	memset(buffer,0,50);
	if (f)
	{
		{ 
			//int err=fscanf(f,"%s",buffer);
			fseek(f, 0, SEEK_END);
			long fsize = ftell(f);
			fseek(f, 0, SEEK_SET);
			fread(buffer, fsize, 1, f);
			if (fsize>0)
			{
				total_crashes=atoi(buffer);
				printf("yas %d: buffer=%s length=%d crashes=%d func=%s, file=%s\n",__LINE__,buffer, length,total_crashes,__FUNCTION__,__FILE__);
			}
		}
		//printf("yas %d: buffer=%s length=%d crashes=%d func=%s, file=%s\n",__LINE__,buffer, length,total_crashes,__FUNCTION__,__FILE__);
		fclose (f);
	}
	else
		total_crashes=0;
}

int addCrash(){	
	char buf[10];
	readCrashStatus();
	FILE * f = fopen ("/home/Moxa/crashes", "w+");
	if (f){
		sprintf(buf,"%d",++total_crashes);
		//printf("yas %d: buffer=%s crashes=%d func=%s, file=%s\n",__LINE__,buf, total_crashes,__FUNCTION__,__FILE__);
		stringlog(" Gateway Client has been Crashed, Totol crashes = %d",0,total_crashes);
		fprintf(f,"%s",buf);
	}
	fclose (f);
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
		printf("Could not create socket");
	}
	puts("Socket created");
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8635 );


	if (setsockopt(socket_desc,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int)) == -1) {
		perror("[Error] Socket configration Error") ;
	}
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("Watchdog bind failed. Error");
	}

	puts("bind done");
	listen(socket_desc , 1);
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while(1){
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if (client_sock < 0)
		{
			perror("accept failed");
		}
		puts("Connection accepted");
		struct timeval timeout;
		timeout.tv_sec = 1000;
		timeout.tv_usec = 0;

		if (setsockopt (client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
			perror("[Error] Socket configration Error") ;
		write(client_sock , "getpid" , strlen("getpid"));
		memset(client_message,0,sizeof(client_message));

		while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
		{
			if(strncmp(client_message,"getpid=",7)==0){
				/*char cpid_buff[10];
				  strncpy(cpid_buff,client_message+7,strlen(client_message)-7);
				  cpid=atol(cpid_buff);
				  printf("yas %d: mesg=%s cpid=%d func=%s, file=%s\n",__LINE__,client_message,cpid,__FUNCTION__,__FILE__);
				  int ret=Enable (cpid);
				  if(ret<0){
				  close(client_sock);
				  kill(cpid, SIGQUIT); 
				  exit(EXIT_SUCCESS);
				  }*/


			}
			if(strncmp(client_message,"reboot_now=",strlen("reboot_now="))==0){
				sleep(5);
				reboot(RB_AUTOBOOT);
				printf("yas %d: its should reboot now ... func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__);
			}
			//        write(client_sock , client_message , strlen(client_message));
		}
		if(read_size == 0)
		{
			puts("GatewayDemon disconnected");
			//fflush(stdout);
		}
		else if(read_size == -1)
		{
			perror("GatewayDemon recv failed");
		}
		close(client_sock);
		//readCrashStatus();
		//		addCrash();
		sprintf(cmd,"killall -9 %s",moxagate);
		system(cmd);
		sleep(1);
	}
}


/**
 * Start the process
 */
void startProcess()
{
	addCrash();
	readCrashStatus();
	char complete_path[100];
	memset(moxagate,0,50);
	memset(complete_path,0,100);
	if(total_crashes<CRASH_LIMIT && access( "/home/Moxa/MoxaGateway", F_OK ) != -1){
		sprintf(moxagate,"MoxaGateway");
	}
	else{
		if( access( "/home/Moxa/MoxaGateway_backup", F_OK ) != -1 ) {
			sprintf(moxagate,"MoxaGateway_backup");
		} else {
			// file doesn't exist
			printf("MoxaGateway_backup does not exist\n");
			sprintf(moxagate,"MoxaGateway");
		}
	}
	sprintf(complete_path,"/home/Moxa/%s &",moxagate);
	printf("\n\rStarting %s..",complete_path);
	//system("/home/Moxa/GatewayDemon &");
	system(complete_path);
	sleep(10);
}

/**
 * Check if the process is running.
 */
int checkProcess()
{

	FILE *fp;
	char path[1035];
	//char* moxaApp = "GatewayDemon";
	char cmd[100];

	int bFound = 0;

	/* Open the command for reading. */
	memset(cmd,0,100);
	sprintf(cmd,"/bin/ps | grep %s",moxagate);
	//fp = popen("/bin/ps | grep MoxaGateway", "r");
	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		exit(1);
	}

	/* Read the output a line at a time - output it. */
	memset(cmd,0,100);
	sprintf(cmd,"grep %s ",moxagate);
	while (fgets(path, sizeof(path)-1, fp) != NULL) {
		if (strstr(path, moxagate) != NULL && strstr(path, cmd) == NULL ) {
			//		  printf("\n\r %s", path);
			bFound = 1;
			break;
		}
	}
	/* close */
	pclose(fp);
	return bFound;
}
int reboot_required = 0;
void timeToReboot()
{
	// current date/time based on current system
	time_t now = time(0);
	tm *ltm = localtime(&now);
	// print various components of tm structure.
	//printf("UTC:   %s", asctime(ltm));
	if (ltm->tm_hour == 1 && ltm->tm_min == 00)
	{
		//set the reboot bit to true so that it reboots at 1:01 AM
		reboot_required = 1;
	}
	else if(reboot_required == 1)
	{
		if( access( "/home/etc/dailyreset", F_OK ) != -1 ) {
			printf( "Reboot initiated..\n");
			system("reboot &");
			sleep(60);
		}


		//exit(0);
	}
}

int main( int argc, char *argv[] )
{
	pthread_t wd;
	printf("\n\r****************************************\n");
	printf("Moxa Client Watchdog \n" );
	printf( "Version 1.0.4.2 \n");
	printf( "****************************************\n");


	int fd;
	fd = swtd_open();
	if ( fd < 0 ) 
		printf("Open sWatchDog device fail !\n");
	else
		swtd_enable(fd, 60000);
	//	sleep(5);

	//	signal(SIGCHLD, childHandler);

	if( access( "/home/Moxa/MoxaGateway", F_OK ) != -1 ) {
		sprintf(moxagate,"MoxaGateway");
	}
	else
		sprintf(moxagate,"MoxaGateway_backup");



	if (!fileExist("/mnt/sd/gateway_logs"))
		mkdir("/mnt/sd/gateway_logs", 0755);


	pthread_create(&wd, NULL, wd_hang, NULL);

	readCrashStatus();
	memset(moxagate,0,50);
	if(total_crashes<CRASH_LIMIT && access( "/home/Moxa/MoxaGateway", F_OK ) != -1)
		sprintf(moxagate,"MoxaGateway");
	else{
		if( access( "/home/Moxa/MoxaGateway_backup", F_OK ) != -1 ) {
			sprintf(moxagate,"MoxaGateway_backup");
		} else {
			// file doesn't exist
			printf("MoxaGateway_backup does not exist\n");
			sprintf(moxagate,"MoxaGateway");
		}
	}
	do
	{
		if ( fd > 0 )
			swtd_ack(fd);
		if(checkProcess() == 0)
		{
			startProcess();
		}
		else{
			//printf("\n\GatewayDemon Process found running..\n\r");
			sleep(10);
		}

		//fflush(stdin);
		//fflush(stdout);
		timeToReboot();
	}
	while(1);
	if ( fd > 0 )	
		swtd_close(fd);

	return 0;
}


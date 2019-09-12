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



/**
 * Start the process
 */
void startProcess()
{

	printf("\n\rStarting MoxaGateway..");
	system("/home/MoxaGateway &");

}

/**
 * Check if the process is running.
 */
int checkProcess()
{

	FILE *fp;
	char path[1035];
	char* moxaApp = "MoxaGateway";

	int bFound = 0;

	/* Open the command for reading. */
	fp = popen("/bin/ps | grep MoxaGateway", "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		exit(1);
	}

	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path)-1, fp) != NULL) {
	  if (strstr(path, moxaApp) != NULL && strstr(path, "grep MoxaGateway ") == NULL ) {
		  //printf("\n\r %s", path);
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
		  printf( "Reboot initiated..\n");
		  system("reboot &");
		  exit(0);
	  }

}


int main( int argc, char *argv[] )
{

	printf("\n\r****************************************\n");
	printf("Moxa Client Watchdog \n" );
	printf( "Version 1.0.1 \n");
	printf( "****************************************\n");

	sleep(5);

	do
	{

		if(checkProcess() == 0)
		{
			startProcess();
		}
		else
		{
			//printf("\n\rMoxaGateway Process found running..\n\r");
			sleep(10);
		}

		timeToReboot();

	}
	while(1);

  return 0;
}

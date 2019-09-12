#define _POSIX_C_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>


#define CRASH_LIMIT 6
pid_t cpid;    
volatile sig_atomic_t done = 0;

static pid_t watchdog_pid = -1;

int total_crashes=0;

char spawned_name[]="Moxa";

void alarmHandler(int signum)
{
	if (kill(cpid, SIGTERM) != -1)
		printf("kill signal sent to child from parent\n");
	else
		if (errno == ESRCH)
			printf("kill could not find child, must already be dead\n");
		else
		{
			perror("kill");
			exit(EXIT_FAILURE);
		}
}

int readCrashStatus(){
	char buffer[50];
	long length;
	
	FILE * f = fopen ("/home/Moxa/crashes", "r+");
	memset(buffer,0,50);

	if (f)
	{

		{ 
			int err=fscanf(f,"%s",buffer);
			if (err>0)
			{
				total_crashes=atoi(buffer);
				
				printf("yas %d: buffer=%s length=%d crashes=%d func=%s, file=%s\n",__LINE__,buffer, length,total_crashes,__FUNCTION__,__FILE__);
				//fprintf(f,"%d",crashes);
			}
			
		}

		printf("yas %d: buffer=%s length=%d crashes=%d func=%s, file=%s\n",__LINE__,buffer, length,total_crashes,__FUNCTION__,__FILE__);

		fclose (f);

	}
	else
				total_crashes=0;
}

int addCrash(){	
	FILE * f;
	char buf[10];
	readCrashStatus();
	f = fopen ("/home/Moxa/crashes", "w+");
	if (f){
		sprintf(buf,"%d",++total_crashes);
		printf("yas %d: buffer=%s crashes=%d func=%s, file=%s\n",__LINE__,buf, total_crashes,__FUNCTION__,__FILE__);
		fprintf(f,"%s",buf);
	}
	fclose (f);
}
void childHandler(int signum)
{
	pid_t childpid;
	int status;

	watchdog_pid = getpid();
	printf("yas %d: cpid=%ld func=%s, file=%s\n",__LINE__,cpid,__FUNCTION__,__FILE__);
	/*	while ((childpid = waitpid( cpid, &status, WNOHANG)) > 0)
		{    
		if (WIFEXITED(status))
		printf("Child %d exited naturally\n", childpid);

		if (WIFSIGNALED(status))
		printf("Child %d exited because of signal\n", childpid);
		}*/

	/* Go to sleep until something happens with the child process (ie: if it

	   exits or is signaled). */
#if 1
	while ((childpid = waitpid(cpid, &status, WUNTRACED)) > 0 ){



		/* Check if the child process terminated due to uncaught signal.  

		   If so, restart it. */

		if(WIFSIGNALED(status)) {

			fprintf(stderr,

					"WATCHDOG (%d): %s (%d) exited due to SIGNAL (code = %d).\n", 

					watchdog_pid, spawned_name, cpid, WTERMSIG(status));

			//readCrashStatus();
			addCrash();
			kill(cpid, SIGKILL);
			if(total_crashes<CRASH_LIMIT)
				system("killall -9 MoxaGateway");
			else
				system("killall -9 MoxaGateway_backup");
			sleep(1);

			/* Go to top of loop and start over. */

			continue;

		}



		/* Check if the child process was stopped or suspended.  If so, kill 

		   it and restart it. */

		if(WIFSTOPPED(status)) {

			fprintf(stderr, "WATCHDOG (%d): %s (%d) was STOPPED "

					"(code = %d).  Killing process.\n", watchdog_pid, 

					spawned_name, cpid, WSTOPSIG(status));

			/* Kill the child process. */
			//readCrashStatus();
			addCrash();
			kill(cpid, SIGKILL);
			if(total_crashes<CRASH_LIMIT)
				system("killall -9 MoxaGateway");
			else
				system("killall -9 MoxaGateway_backup");
			sleep(1);

			/* Go to top of loop and start over. */

			continue;

		}



		/* Check if the child process exited (ie: return; exit(int); etc.).  

		   If so and the code != 0, restart it. */

		if(WIFEXITED(status)) {

			/* If the child exited with code == 0, it wanted to terminate.

			   WATCHDOG quits. */

			if(WEXITSTATUS(status) == 0) {

				fprintf(stderr, "WATCHDOG (%d): %s (%d) exited CLEANLY."

						"  Terminating watchdog.\n", watchdog_pid, spawned_name,

						cpid);

				/* WATCHDOG quits. */
				

				//exit(0);

			}

			/* ...otherwise restart the child. */

			else {

				fprintf(stderr, 

						"WATCHDOG (%d): %s (%d) exited UNCLEANLY (code = %d).\n", 

						watchdog_pid, spawned_name, cpid, WEXITSTATUS(status));

									//readCrashStatus();
			addCrash();
			kill(cpid, SIGKILL);
			if(total_crashes<CRASH_LIMIT)
				system("killall -9 MoxaGateway");
			else
				system("killall -9 MoxaGateway_backup");
			sleep(1);
				/* Go to top of loop and start over. */

				//continue;

			}

		}

	}
#endif
	if (childpid == -1 && errno != ECHILD)
	{
		perror("waitpid");
		exit(EXIT_FAILURE);
	}

	done = 1;
}

int main (int argc, char *argv[])
{
	int sleepSecs;
	int timeoutSecs;
	char moxagate[50];
	
	printf("Initiating GatewayDemon....\n ");
	system("killall -9 MoxaGateway");
	sleep(1);
	system("killall -9 MoxaGateway_backup");
	sleep(1);
	readCrashStatus();
	
	

	/*    if (argc < 3)
	      {
	      printf("\nusage: %s sleep-seconds timeout-seconds\n\n", argv[0]);
	      exit(EXIT_FAILURE);
	      }

	      sscanf(argv[1], "%d", &sleepSecs);
	      sscanf(argv[2], "%d", &timeoutSecs);*/

	//sscanf(argv[1], "%ld", &cpid);
	//printf("yas %d: cpid=%ld func=%s, file=%s\n",__LINE__,cpid,__FUNCTION__,__FILE__);

	do {
		
		if(total_crashes<CRASH_LIMIT && access( "/home/Moxa/MoxaGateway", F_OK ) != -1)
			sprintf(moxagate,"/home/Moxa/MoxaGateway");
		else{
			if( access( "/home/Moxa/MoxaGateway_backup", F_OK ) != -1 ) {
				sprintf(moxagate,"/home/Moxa/MoxaGateway_backup");
		} else {
		// file doesn't exist
						printf("MoxaGateway_backup does not exist\n");
						sprintf(moxagate,"/home/Moxa/MoxaGateway");
		}
			}
		signal(SIGCHLD, childHandler);
		//   signal(SIGALRM, alarmHandler);
		

		done = 0;
		if ((cpid = vfork()) == -1)

		{
			printf("%d : failed to start child process.\n", errno);
			perror("fork");
			exit( -1);
		}

		if (cpid == 0) //child
		{
			printf("Demon starting %s...\n",moxagate);

				execl(moxagate, moxagate, (char *) NULL);
			

			perror("execl");
			exit(EXIT_FAILURE);
		}
		else //parent
		{
			//        alarm(timeoutSecs);

			while (! done)
			{
				sleep(1); // or do something useful instead
			}

			//exit(0);
		}
		sleep(5);
	}while(1);
}


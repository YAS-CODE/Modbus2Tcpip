

#include <stdio.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main( int argc, char *argv[] )
{
	
	int delay;
	char buffer[50];
	long length;
	long fsize;
	FILE * f;// = fopen ("/home/Moxa/crashes", "r+");
	printf("rebootd process ID : %d\n", getpid());
	

	char buf[10];

	f = fopen ("/var/run/rebootd.pid", "w+");
	if (f){
		sprintf(buf,"%d",getpid());
		fprintf(f,"%s",buf);
	}
	fclose (f);


	while(1){
	f = fopen ("/var/reboot", "r+");
	memset(buffer,0,50);
	delay=0;
	if (f)
	{
		{ 
			//int err=fscanf(f,"%s",buffer);
                        fseek(f, 0, SEEK_END);
                        fsize = ftell(f);
                        fseek(f, 0, SEEK_SET);
                        fread(buffer, fsize, 1, f);
                        if (fsize>0)
			{
				delay=atoi(buffer);
				printf("Going to reset by Gateway web....\n");
				sleep(delay);
				reboot(RB_AUTOBOOT);
				
			}
		}
		fclose (f);
	}
	}

	
	return 0;
}


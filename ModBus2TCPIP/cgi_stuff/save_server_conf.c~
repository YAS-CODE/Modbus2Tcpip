#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>


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
#include <pthread.h>



#define MAX_LEN 256


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
int wd_sock;
void reboot(){

	char buf[10];
	int reboot_pid;
	long length;
	long fsize;
	FILE * f;

	f = fopen ("/var/run/rebootd.pid", "r+");
	memset(buf,0,10);
	if (f)
	{
		{ 
			fseek(f, 0, SEEK_END);
			fsize = ftell(f);
			fseek(f, 0, SEEK_SET);
			fread(buf, fsize, 1, f);
			if (fsize>0)
			{
				reboot_pid=atoi(buf);

			}
		}
		fclose (f);
	}


	if (kill(reboot_pid, 0) == 0) {
		/* process is running or a zombie */
		f = fopen ("/var/reboot", "w+");
		if (f){
			sprintf(buf,"%d",10);

			fprintf(f,"%s",buf);
		}
		fclose (f);

	}  else {
		sleep(1);
		reboot(RB_AUTOBOOT);
	}

}

int findpattren(char *str, char *find, int strln,int findln,int str_p){
	int a=0,b;

	//int substr_len=strlen(find);

	if(findln==1){
		for(a=str_p;a<strln;a++){
			if(str[a]==*(find))
				break;
		}
	}
	else{	
		for(a=str_p;a<strln;a++){
			if(str[a]==*(find)){
				for(b=1;b<findln;b++){
					if(str[a+b]!=*(find+b)){
						break;
					}
				}
				if(b==findln)
					break;
			}
		}
	}
	if(a==strln)
		return -1;
	return a;
}

int getIdVal(char *buffer,char *key,char *val){
	int index, length;
	index=findpattren(buffer,key,strlen(buffer),strlen(key),0);
	length=0;
	index=index+strlen(key)+1;
	while(buffer[index+length]!='&')
		length++;
	strncpy(val,buffer+index,length);
	fprintf(stderr,"yas %d: val=%s buffer+index=%s func=%s, file=%s\n",__LINE__,val,buffer+index,__FUNCTION__,__FILE__);
}

int main(int argc, char* argv[])
{
	int fd;
	pthread_t timer_thread;
	// struct ifreq ifr;
	char *query_string, *request_method, *content_length;
	char gw_server_adr[100],command[100];
	printf("Content-type: text/html\n\n");
	request_method = getenv("REQUEST_METHOD");
	content_length = getenv("CONTENT_LENGTH");
	query_string = getenv("QUERY_STRING");

	if( access( "/home/etc/auth", F_OK ) == -1 ) {
		showlogin();
		return 0;
	}
#if 0
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, "eth0", 16-1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);
#endif



	if(query_string == NULL){
		printf("<p>Error! Error in passing data from form to script.</p>"); 
	}
	else{
		//		int num=sscanf(query_string, "lan_type=%*s&lan_ip=%s&lan_mask=%s&lan_gateway=%s&save=Apply+Changes",lan_ip,lan_mask,lan_gateway);


		fprintf(stderr,"here gw_server_adr=%s\n", gw_server_adr);
		mkdir("../etc", 0755);
		memset(gw_server_adr,0,100);
		getIdVal(query_string,"gw_server_adr",gw_server_adr);
		FILE *ptr;
		//ptr=fopen("../etc/config.txt", "w+");
		ptr=fopen("/home/Moxa/config.txt", "w+");
		//system("pwd >> ../Moxa/config.txt");
		sprintf(command, "server_address=%s\n",gw_server_adr);
		fwrite(command, 1, strlen(command), ptr);
		fclose(ptr);


		printf("<html>\n");
		printf(" <head>\n");
		printf("  <script type=\"text/javascript\">\n");
		printf("      function submitForm()\n");
		printf("         {\n");
		printf("      document.getElementById(\"ismForm\").submit();\n");
		printf("      }      \n");
		printf("      }\n");
		printf("   </script> \n");
		printf("  <script type=\"text/javascript\">\n");
		printf("      var interval;\n");
		printf("          var minutes = 0;\n");
		printf("      var seconds = 60; \n");
		printf("          window.onload = function() \n");
		printf("      {\n");
		printf("       countdown('countdown');\n");
		printf("          }\n");
		printf("          function countdown(element) \n");
		printf("      {\n");
		printf("       interval = setInterval(function() {\n");
		printf("           var el = document.getElementById(element);\n");
		printf("           if(seconds == 0) {\n");
		printf("               if(minutes == 0) {\n");

		printf("                 (el.innerHTML = \"Device sucessfully configured!\");\n");
		printf("                   clearInterval(interval);\n");
		printf("                   return;\n");
		printf("               } else {\n");
		printf("                   minutes--;\n");
		printf("                   seconds = 60;\n");
		printf("               }\n");
		printf("           }\n");
		printf("           if(minutes > 0) {\n");
		printf("               var minute_text = minutes + (minutes > 1 ? ' minutes' : ' minute');\n");
		printf("           } else {\n");
		printf("               var minute_text = '';\n");
		printf("           }\n");
		printf("           var second_text = seconds > 1 ? 'seconds' : 'second';\n");
		printf("           el.innerHTML = minute_text + ' ' + seconds + ' ' + second_text + ' remaining to Reboot Device';\n");
		printf("           seconds--;      \n");
		printf("       }, 1000);\n");
		printf("      }\n");
		printf("   </script> \n");
		printf(" </head> \n");
		printf(" <body> \n");
		printf("  <center> \n");
		printf("   <div id=\"countdown\"></div> \n");
		printf("  </center> \n");
		printf("  <br>\n");
		printf("  <br> \n");
		printf(" </body>\n");
		printf("</html>\n");
	}

	free(query_string);
	free(request_method);
	free(content_length);

	remove("/home/etc/auth");
	reboot();

	return 0;
}

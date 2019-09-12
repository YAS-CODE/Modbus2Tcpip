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



#define MAX_LEN 256
/////////////////////////////////////////////////////Web Watchdog/////////////////////////////////////////////

int wd_sock;




void rebootWatchdog(void) {

	struct sockaddr_in server;
	wd_sock=0;

	//Create socket

	wd_sock = socket(AF_INET , SOCK_STREAM , 0);
	if (wd_sock == -1)
	{
		fprintf(stderr,"Could not create socket");
	}

	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( 8635 );
	int optval = 1 ;
	if (setsockopt(wd_sock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int)) == -1) {

		perror("[Error] Socket configration Error") ;
		//	exit(-1) ;
	}

	while(connect(wd_sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//perror("connect failed. Error");
	}


	write(wd_sock, "reboot_now=", strlen("reboot_now="));
	sleep(1);
	close(wd_sock);

}
//////////////////////////////////////////////////////end Web Watchdog///////////////////////////////////////////




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

void showHeader(){
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


}

void showFooter(){
	printf("  </center> \n");
	printf("  <br>\n");
	printf("  <br> \n");
	printf(" </body>\n");
	printf("</html>\n");

}

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


int main(int argc, char* argv[])
{
	int fd,index,length;
	FILE *ptr;
	// struct ifreq ifr;
	char *query_string, *request_method, *content_length;
	char old_pass[16],new_pass[16],retry_pass[16],command[100],orignal_pass[16];
	printf("Content-type: text/html\n\n");
	request_method = getenv("REQUEST_METHOD");
	content_length = getenv("CONTENT_LENGTH");
	query_string = getenv("QUERY_STRING");
	fprintf(stderr,"yas %d: query_string=%s func=%s, file=%s\n",__LINE__,query_string,__FUNCTION__,__FILE__);

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



		memset(old_pass,0,16);
		memset(new_pass,0,16);
		memset(retry_pass,0,16);	
		memset(orignal_pass,0,16);
		getIdVal(query_string,"old_pass",old_pass);
		getIdVal(query_string,"new_pass",new_pass);
		getIdVal(query_string,"retype_new_pass",retry_pass);

		ptr=fopen("/home/etc/pass", "r");

		if (ptr!=NULL){
			while (fgets(command, sizeof(command), ptr)) {
				index=0;
				length=strlen(command);
				while(command[index]!='=')
					index++;
				if(strncmp(command,"PASS",strlen("PASS"))==0)
					strncpy(orignal_pass,command+index+1,length-index-2);
			
			}
			fclose(ptr);
			if(strcmp(old_pass,orignal_pass)!=0){
			showHeader();
			fprintf(stderr,"\nThe password is incorrect !! %s,%s\n", orignal_pass,old_pass);
			printf("   <div>The password is incorrect !!</div> \n");
			showFooter();
			return 0;
			}
			
				
		}


		if(strcmp(new_pass,retry_pass)!=0){
			showHeader();
			printf("   <div>The password and the retype password must be the same.</div> \n");
			showFooter();
			return 0;
		}
			
		fprintf(stderr,"\n\nhere old_pass=%s, new_pass=%s, retype_new_pass=%s\n", old_pass,new_pass,retry_pass);
		mkdir("../etc", 0755);
		
					//DHCP Enabled

		ptr=fopen("/home/etc/pass", "w+");
		sprintf(command, "PASS=%s\n",new_pass);
		fwrite(command, 1, strlen(command), ptr);
		fclose(ptr);
		


		//printf("<H2> %s </H2>",command);
		//printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	}

#if 1	
	/* display result */

	showHeader();
	printf("   <div id=\"countdown\"></div> \n");
	showFooter();
	
	free(query_string);
	free(request_method);
	free(content_length);

	remove("/home/etc/auth");
	reboot();
	//reboot(RB_AUTOBOOT);
	//rebootWatchdog();
#endif




	return 0;
}

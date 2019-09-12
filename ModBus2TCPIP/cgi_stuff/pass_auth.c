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


void showHome(){
	puts("Content-type: text/html\n");
	puts("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">");
	puts("<HTML>");
	puts("<HEAD>");
	puts("<TITLE></TITLE>");
	puts("<META http-equiv=Refresh Content=\"0; Url=loadiplan.cgi\">");
	puts("</HEAD>");
	puts("</HTML>");
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
	int fd,index,length;
	FILE *ptr;
	// struct ifreq ifr;
	char *query_string, *request_method, *content_length;
	char command[100],orignal_pass[16],pass[16],account[16];
	
	request_method = getenv("REQUEST_METHOD");
	content_length = getenv("CONTENT_LENGTH");
	query_string = getenv("QUERY_STRING");
	fprintf(stderr,"yas %d: query_string=%s func=%s, file=%s\n",__LINE__,query_string,__FUNCTION__,__FILE__);



	if(query_string == NULL){
		printf("<p>Error! Error in passing data from form to script.</p>"); 
	}
	else{



			
		memset(orignal_pass,0,16);
		memset(pass,0,16);
		memset(account,0,16);
		getIdVal(query_string,"password",pass);
		getIdVal(query_string,"email",account);

		if(strcmp(account,"admin")!=0){
		showlogin();
		return 0;
		}

		ptr=fopen("/home/etc/pass", "r");

		if (ptr!=NULL){
			fprintf(stderr,"yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__);
			while (fgets(command, sizeof(command), ptr)) {
				index=0;
				length=strlen(command);
				while(command[index]!='=')
					index++;
				if(strncmp(command,"PASS",strlen("PASS"))==0)
					strncpy(orignal_pass,command+index+1,length-index-2);
			
			}
			fclose(ptr);
			if(strcmp(pass,orignal_pass)==0){
			ptr=fopen("/home/etc/auth", "w+");
			if (ptr!=NULL){
			fwrite("1", 1, 1, ptr);
			fclose(ptr);
			}
			showHome();
			return 0;
			}
			
				
		}
		else if(strcmp(pass,"admin")==0){
			ptr=fopen("/home/etc/auth", "w+");
			if (ptr!=NULL){
			fwrite("1", 1, 1, ptr);
			fclose(ptr);
			}
			showHome();
			return 0;
		}
		
		
			
	}
	showlogin();
		
	free(query_string);
	free(request_method);
	free(content_length);

	return 0;
}

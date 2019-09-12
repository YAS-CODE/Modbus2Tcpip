#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LEN 256




int main(int argc, char* argv[])
{
	char *query_string, *request_method, *content_length;
	char lan_ip[50],lan_mask[50],lan_gateway[50],command[100];

	        request_method = getenv("REQUEST_METHOD");
		  content_length = getenv("CONTENT_LENGTH");
		  query_string = getenv("QUERY_STRING");
		  printf("REQUEST_METHOD: {%s}<br>", request_method);
		  printf("CONTENT_LENGTH: {%s}<br>", content_length);
		  printf("QUERY_STRING: {%s}<br>", query_string);
		  if(query_string == NULL){
		  printf("<p>Error! Error in passing data from form to script.</p>");
		  }
		  else{
	//              int num=sscanf(query_string, "lan_type=%*s&lan_ip=%s&lan_mask=%s&lan_gateway=%s&save=Apply+Changes",lan_ip,lan_mask,lan_gateway);    
	getIdVal(query_string,"lan_ip",lan_ip);
	getIdVal(query_string,"lan_mask",lan_mask);
	getIdVal(query_string,"lan_gateway",lan_gateway);
	FILE *ptr;
	ptr=fopen("network", "w+");
	sprintf(command, "ifconfig eth0 %s netmask %s;route add default gw %s dev eth0", lan_ip,lan_mask,lan_gateway);
	fwrite(command, 1, strlen(command), ptr);
	fclose(ptr);
	printf("<H2> %s </H2>",command);
	}

	free(query_string);
	free(request_method);
	free(content_length);

	return 0;
}



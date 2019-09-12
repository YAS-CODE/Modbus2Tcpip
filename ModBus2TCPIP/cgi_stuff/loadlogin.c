
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <stdio.h>

#include <stdlib.h>     /* strtoul */
#include <sys/stat.h>
#include <errno.h>



#define MAX_LEN 256


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
#if 0
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

	fprintf(stderr,"yas %d: success=%d mac_address=%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx func=%s, file=%s\n",__LINE__,success,mac_address[0],mac_address[1],mac_address[2],mac_address[3],mac_address[4],mac_address[5],__FUNCTION__,__FILE__);

	return mac_address;

}
#endif

int readDevList(){
	FILE * fdev;
	char buff[50];
	memset(last_devices,0,50);
	fdev = fopen ("/home/Moxa/last_devs","r");
	if (fdev == NULL)
	{

		printf("Error opening file! data_cashe_temp\n");
		//fclose(fmac);
		return 1;

	}
	fseek (fdev, 0, SEEK_END);
	long length = ftell (fdev);
	fseek (fdev, 0, SEEK_SET);
	if (length>0)
		fread (buff, 1, length, fdev);
	else{
		fclose(fdev);
		return 1;
	}
	fclose(fdev);
	
	strncpy(last_devices,buff,length-1);
	
	//strncpy(m_device_numbers,buff,length-1);
	return 0;

}


int main(int argc, char* argv[])
{
	char *query_string, *request_method, *content_length;
	char lan_type[10],lan_ip[50],lan_mask[50],lan_gateway[50],header_background_color[10],navigation_color[10],server_address[100],command[100],temp[10],header_logo_background_color[10];
	int disable_serverp,disable_about,disable_contact;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	request_method = getenv("REQUEST_METHOD");
	content_length = getenv("CONTENT_LENGTH");
	query_string = getenv("QUERY_STRING");
	fprintf(stderr,"REQUEST_METHOD: {%s}<br>\n", request_method);
	fprintf(stderr,"CONTENT_LENGTH: {%s}<br>\n", content_length);
	fprintf(stderr,"QUERY_STRING: {%s}<br>\n", query_string);


	//getMACAddress();
	readDevList();
	/// char const* const fileName = argv[1]; /* should check that argc > 1 */
	
	// char line[256];

	int index,length;

	memset(lan_ip,0,50);
	memset(lan_mask,0,50);
	memset(lan_gateway,0,50);
	memset(lan_type,0,10);
	memset(temp,0,10);
	memset(server_address,0,100);
	strcpy(server_address,"0.0.0.0");
	FILE* file = fopen("/home/httpd/lastconf", "r"); /* should check the result */
	if (file!=NULL){
		while (fgets(command, sizeof(command), file)) {
			/* note that fgets don't strip the terminating \n, checking its
			   presence would allow to handle lines longer that sizeof(line) */

			index=0;
			while(command[index]!='=')
				index++;

			length=strlen(command);
			if(strncmp(command,"LAN_MASK",strlen("LAN_MASK"))==0)
				strncpy(lan_mask,command+index+1,length-index-2);

			if(strncmp(command,"LAN_IP",strlen("LAN_IP"))==0)
				strncpy(lan_ip,command+index+1,length-index-2);

			if(strncmp(command,"LAN_GATEWAY",strlen("LAN_GATEWAY"))==0)
				strncpy(lan_gateway,command+index+1,length-index-2);

			if(strncmp(command,"LAN_TYPE",strlen("LAN_TYPE"))==0)
				strncpy(lan_type,command+index+1,length-index-2);
			fprintf(stderr,"parse results lan_type=%s lan_ip=%s,lan_mask=%s,lan_gateway=%s\n",lan_type, lan_ip,lan_mask,lan_gateway);
		}
		fclose(file);
	}
	
#if 1
	//file = fopen("../etc/config.txt", "r");
	file = fopen("/home/Moxa/config.txt", "r");
	if (file!=NULL){
		while (fgets(command, sizeof(command), file)) {
			/* note that fgets don't strip the terminating \n, checking its
			   presence would allow to handle lines longer that sizeof(line) */

			index=0;
			while(command[index]!='=')
				index++;

			length=strlen(command);
			if(strncmp(command,"server_address",strlen("server_address"))==0)
				strncpy(server_address,command+index+1,length-index-2);


			fprintf(stderr,"parse results server_address=%s \n",server_address);
		}
		fclose(file);
	}
	
#endif	
	memset(header_background_color,0,10);
	memset(navigation_color,0,10);
	memset(header_logo_background_color,0,10);
	//strcpy(header_background_color,"#B51A1D");
	strcpy(header_background_color,"#808080");
	strcpy(header_logo_background_color,"#808080");
	disable_about=0;
	disable_serverp=0;
	disable_contact=0;

	strcpy(navigation_color,"red");

	//system("hello >>../etc/boa-test");
	file=fopen("/home/etc/customer.profile", "r");

	if (file!=NULL){
		while (fgets(command, sizeof(command), file)) {
			index=0;
			length=strlen(command);
			while(command[index]!='=')
				index++;
			if(strncmp(command,"HEADER_BACK_COL",strlen("HEADER_BACK_COL"))==0)
				strncpy(header_background_color,command+index+1,length-index-2);
			if(strncmp(command,"HEADER_LOGO_BACK_COL",strlen("HEADER_LOGO_BACK_COL"))==0)
				strncpy(header_logo_background_color,command+index+1,length-index-2);
			if(strncmp(command,"NAV_TEXT_COL",strlen("NAV_TEXT_COL"))==0){
				strncpy(navigation_color,command+index+1,length-index-2);

			}
			if(strncmp(command,"DISABLE_ABOUT",strlen("DISABLE_ABOUT"))==0){
				strncpy(temp,command+index+1,length-index-2);
				disable_about=atoi(temp);
			}
			if(strncmp(command,"DISABLE_CONTACT",strlen("DISABLE_CONTACT"))==0){
				strncpy(temp,command+index+1,length-index-2);
				disable_contact=atoi(temp);
			}
			if(strncmp(command,"DISABLE_SERVER",strlen("DISABLE_SERVER"))==0){
				strncpy(temp,command+index+1,length-index-2);
				disable_serverp=atoi(temp);
			}
		}
		fclose(file);
	}



	puts("Content-type: text/html\n");
	puts("<!DOCTYPE html>");
	puts("<html>");
	puts("   <head>");
	puts("      <title>Remote Caretaking Gateway</title>");
	puts("      ");
	puts("      <script src=\"/jquery.min.js\"></script>");
	printf("<link rel=\"stylesheet\" type=\"style/css\" >\n");


	printf("<style>\n");
	printf("   body {\n");
	printf("   background-color: #EEE;\n");
	printf("   font-family: Helvetica, Arial, sans-serif;\n");
	printf("   }\n");
	printf("   a {\n");
	printf("   text-decoration: none;\n");
	printf("   color: %s;\n",navigation_color);
	printf("   }\n");
	printf("   h1, h2, h3 {\n");
	printf("   margin: 0;\n");
	printf("   }\n");
	printf("   #nav ul {\n");
	printf("   list-style-type: none;\n");
	printf("   padding: 0px;\n");
	printf("   }\n");
	printf("   #header {\n");
	printf("   padding: 0px;\n");
	printf("   background-color: %s;\n",header_background_color);
	printf("   color: white;\n");
	printf("   height: 110px;\n");
	printf("   /*    text-align: center;*/\n");
	printf("   }\n");
	printf("   #TopMenuRight{\n");
	printf("   width: 85%%;\n");
	printf("   float:left;\n");
	printf("   font: 14px \"Arial\",Helvetica,Arial,sans-serif;\n");
	printf("   }\n");
	printf("   .top-headline {\n");
	printf("   color: #fff;\n");
	printf("   display: inline-block;\n");
	printf("   margin-top: 45px;\n");
	printf("   }\n");
	printf("   .h2m {\n");
	printf("   padding-top: 8px;\n");
	printf("   font-size: 21px;\n");
	printf("   font-weight: bold;\n");
	printf("   padding-left: 80px;\n");
	printf("   }\n");
	printf("   #TopMenuLeft{\n");
	printf("   width:15%%;\n");
	printf("   background-color: %s;\n",header_logo_background_color);
	printf("   height: 110px;\n");
	printf("   float:left;\n");
	printf("   }\n");
	printf("   #TopMenuLogo\n");
	printf("   {\n");
	printf("   background:url(\"/images/nilan_logo.svg\") no-repeat center center;\n");
	printf("   background-size: 75%%;\n");
	printf("   height:110px;\n");
	printf("   }\n");
	printf("   #container {\n");
	printf("   background-color: white;\n");
	printf("   width: 800px;\n");
	printf("   margin-left: auto;\n");
	printf("   margin-right: auto;\n");
	printf("   }\n");
	printf("   #content {\n");
	printf("   padding: 10px;\n");
	printf("   }\n");
	puts("#nav {");
	puts("    width: 180px;");
	puts("    float: top;");
	puts("    height: 100%;");
	puts("  bottom: 0;");
	puts("}");
	puts("#nav ul li a {");
	puts("display: inline-block;");
	puts("padding: 10px 15px;");
	puts("}");
	printf("   #main {\n");
	printf("   width: 600px;\n");
	printf("   float: right;\n");
	printf("   }\n");
	printf("   #footer {\n");
	printf("   clear: both;\n");
	printf("   padding: 0px;\n");
	printf("   background-color: #999;\n");
	printf("   color: white;\n");
	printf("   text-align: right;\n");
	printf("   }\n");
	printf("   #nav .selected {\n");
	printf("   font-weight: bold;\n");
	printf("   }\n");

/*	printf("table, th, td {");
	printf("    border: 1px solid black;");
	printf("    border-collapse: collapse;");
	printf("}");
	printf("th, td {");
	printf("    padding: 5px;");
	printf("}");*/
	printf("</style>\n");



	puts("      <script>");
	puts("         $(document).ready(function(){");
	puts("           // Set trigger and container variables");
	puts("           var trigger = $(\'#nav ul li a\'),");
	puts("               container = $(\'#main\');");
	puts("         //alert(\"hello ready!!!!\");");
	puts("           ");
	puts("           // Fire on click");
	puts("           trigger.on(\'click\', function(){");
	puts("             // Set $this for re-use. Set target from data attribute");
	puts("             var $this = $(this),");
	puts("               target = $this.data(\'target\');       ");
	puts("             ");
	puts("             // Load target page into container");
	puts("             //container.load(target + \'.php\');");
	puts("//             container.load(\'about.php\');");
	puts("		if(target==\'GW\'){");
	puts("		$(\"#main\").hide();");
	puts("		$(\"#content-about\").hide();");
	puts("		$(\"#content-contact\").hide();");
	puts("		$(\"#gw_server\").show();");
	puts("        $(\"#network_set\").hide();");
	puts("		$(\"#pass_set\").hide();");
	puts("		}");
	puts("		else if(target==\'about\'){");
	puts("		$(\"#main\").hide();");
	puts("		$(\"#content-contact\").hide();");
	puts("		$(\"#gw_server\").hide();");
	puts("		$(\"#content-about\").show();");
	puts("        $(\"#network_set\").hide();");
	puts("		$(\"#pass_set\").hide();");
	puts("		}");
	puts("		else if(target==\'home\'){");
	puts("		$(\"#content-about\").hide();");
	puts("		$(\"#content-contact\").hide();");
	puts("		$(\"#gw_server\").hide();");
	puts("		$(\"#main\").show();");
	puts("        $(\"#network_set\").hide();");
	puts("		$(\"#pass_set\").hide();");
	puts("		}");
	puts("		else if(target==\'contact\'){");
	puts("		$(\"#main\").hide();");
	puts("		$(\"#content-about\").hide();");
	puts("		$(\"#gw_server\").hide();");
	puts("		$(\"#content-contact\").show();");
	puts("        $(\"#network_set\").hide();");
	puts("		$(\"#pass_set\").hide();");
	puts("		}");
	puts("        else if(target==\'NS\'){");
	puts("		$(\"#main\").hide();");
	puts("		$(\"#content-about\").hide();");
	puts("		$(\"#gw_server\").hide();");
	puts("		$(\"#content-contact\").hide();");
	puts("        $(\"#network_set\").show();");
	puts("		$(\"#pass_set\").hide();");
	puts("		}");
	puts("        else if(target==\'PS\'){");
	puts("		$(\"#main\").hide();");
	puts("		$(\"#content-about\").hide();");
	puts("		$(\"#gw_server\").hide();");
	puts("		$(\"#content-contact\").hide();");
	puts("        $(\"#network_set\").hide();");
	puts("		$(\"#pass_set\").show();");
	puts("		}");
	puts("  			");
	puts("             // Stop normal link behavior");
	puts("             return false;");
	puts("           });");
	puts("         });");
	puts("      </script>");

	puts("   </head>");
	puts("   <body>");
	puts("      <div id=\"container\">");
	puts("         <div id=\"header\">");
	puts("            <div id=\"TopMenuRight\" >");
	puts("               <!-- Check for the directive  -->");
	puts("               <span class=\"top-headline h2m ng-binding\" ng-bind=\"title\">Remote Caretaking Gateway</span>");
	puts("            </div>");
	puts("            <div id=\"TopMenuLeft\">");
	puts("               <div id=\"TopMenuLogo\"></div>");
	puts("            </div>");
	puts("         </div>");
	puts("         <div id=\"content\">");
	puts("             <table>");
	puts("  <tr>");
	puts("    <td valign=\"top\">");
	puts("	<div id=\"nav\">");
	puts("               <ul>");
	puts("		<li><a href=\"#\" data-target=\"home\">Home</a></li>");
	puts("        <li><a href=\"#\" data-target=\"NS\">Network Settings</a></li>");

	if(!disable_serverp)
		puts("        	<li><a href=\"#\" data-target=\"GW\">Gateyways Server</a></li>");
	puts("        <li><a href=\"#\" data-target=\"PS\">Password</a></li>");
	if(!disable_about)
		puts("		<li><a href=\"#\" data-target=\"about\">About</a></li>");
	if(!disable_contact)
		puts("        	<li><a href=\"#\" data-target=\"contact\">Contact</a></li>");

	puts("	       </ul>");
	puts("            </div>");
	puts("	</td>");
	puts("    <td>");
	puts("	<div>");
	puts("            <div id=\"main\">");
	puts("               <!--<blockquote>-->");
	puts("               <big>System Information</big>");
	puts("               <table border=0 width=550 cellspacing=0 cellpadding=0>");
	puts("                  <tr>");
	puts("                     <td>");
	puts("                        <hr size=1 noshade >");
	puts("                     </td>");
	puts("                  </tr>");
	puts("                  <tr>");
	puts("                     <td><small>");
	puts("                        This page show Gateway`s general imformation.");
	puts("                        </small>");
	puts("                     </td>");
	puts("                  </tr>");
	puts("                  <tr>");
	puts("                     <td>");
	puts("                        <hr size=1 noshade >");
	puts("                     </td>");
	puts("                  </tr>");
	puts("               </table>");
	puts("               <!---form action=\"cgi-bin/save_reboot.cgi\" method=\"GET\"--->");
	puts("                  <table border=0 width=550 cellspacing=0 cellpadding=0>");
	puts("                     ");
	puts("                     <tr>");
	puts("                        <td>&nbsp;</td>");
	puts("                        <td></td>");
	puts("                     </tr>");
	puts("                     <tr>");
	puts("                        <td>&nbsp;</td>");
	puts("                        <td></td>");
	puts("                     </tr>");
	puts("                     <tr>");
	puts("                         <td width=\"30%\" valign=top>MAC Address :</td>");
	memset(mac_address,0,6);
	readMacAddr(mac_address);
	
	printf("                        <td width=\"70%%\">%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx </td>",mac_address[0],mac_address[1],mac_address[2],mac_address[3],mac_address[4],mac_address[5]);
	puts("                     </tr>");
	puts("                     <tr>");
	puts("                        <td>&nbsp;</td>");
	puts("                        <td></td>");
	puts("                     </tr>");
	puts("                     <tr>");
	puts("                        <td width=\"30%\" valign=top>Server status:</td>");
//	printf("                        <td width=\"70%%\">%s</td>",last_devices);
		file=fopen("/home/logs/Gateway_conn_stat", "r");

	if (file!=NULL){
		while (fgets(command, sizeof(command), file)) {
			index=0;

				if(strncmp("0",command,1)==0)
					printf("                        <td width=\"70%%\">OFFLINE</td>");
				else if(strncmp("1",command,1)==0)
					printf("                        <td width=\"70%%\">ONLINE</td>");
				else
					printf("                        <td width=\"70%%\">...</td>");

		}
		fclose(file);
	}
	else
		printf("                        <td width=\"70%%\">...</td>");


	puts("                     </tr>");
	puts("                     <tr>");
	puts("                        <td>&nbsp;</td>");
	puts("                        <td></td>");
	puts("                     </tr>");
	puts("                     <tr>");
	puts("                        <td width=\"30%\" valign=top>Gateway`s date/time :</td>");
	printf("                        <td width=\"70%%\"> %d-%d-%d %d:%d:%d</td>",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	puts("                     </tr>");
	puts("                     <tr>");
	puts("                        <td>&nbsp;</td>");
	puts("                        <td></td>");
	puts("                     </tr>");
	puts("                     <tr>");
	puts("                        <td>&nbsp;</td>");
	puts("                        <td></td>");
	puts("                     </tr>");

	puts("                     <tr>");
	puts("                        <td>&nbsp;</td>");
	puts("                        <td></td>");
	puts("                     </tr>");
	puts("                     <tr>");
	puts("                        <td></td>");
#if 0
	puts("                        <!--td>");
	puts("                           <input type=\"submit\" name=\"save\" value=\"Save & Reboot\" onclick=\"return foo();\" />");
	puts("                           &nbsp;&nbsp;");
	puts("                           <input type=\"reset\" value=\"Reset\" name=\"reset\" onClick=\"resetClick()\">");
	puts("                        </td-->");
#endif
	puts("                     </tr>");
	puts("                  </table>");
#if 1
	puts("	<table style=\"width:70%%\" border=\"1\" valign=top> ");
	puts("  <tr>");
	puts("    <th>Device Id</th> ");
	puts("    <th>Device Status</th>		");
	puts("  </tr> ");
/*	puts("  <tr> ");
	puts("    <td>Jill</td> ");
	puts("    <td>Smith</td>		");
	puts("  </tr> ");*/
	file=fopen("/home/logs/devs_state", "r");

	if (file!=NULL){
		while (fgets(command, sizeof(command), file)) {
			index=0;
			length=strlen(command);
			while(command[index]!='=')
				index++;
				puts("  <tr> ");
				strncpy(temp,command,index);				
				printf("    <td valign=top>%s</td> ",temp);
				memset(temp,0,10);
				strncpy(temp,command+index+1,length-index-2);
				if(strncmp("0",temp,1)==0)
					printf("    <td>OFFLINE</td>		");
				else if(strncmp("1",temp,1)==0)
					printf("    <td>ONLINE</td>		");
				else
					printf("    <td>...</td>		");
				puts("  </tr> ");
		}
		fclose(file);
	}
	puts(" </table>" );
#endif
	puts("                  <br>");
	puts("               <!--/form-->");
	puts("            </div>");
	puts("            <div id=\"network_set\" hidden>");
	puts("               <!--<blockquote>-->");
	puts("               <big>LAN Interface Setup</big>");
	puts("               <table border=0 width=550 cellspacing=0 cellpadding=0>");
	puts("                  <tr>");
	puts("                     <td>");
	puts("                        <hr size=1 noshade >");
	puts("                     </td>");
	puts("                  </tr>");
	puts("                  <tr>");
	puts("                     <td><small>");
	puts("                        This page allows configuration of static or dynamic IP address assignment");
	puts("                        </small>");
	puts("                     </td>");
	puts("                  </tr>");
	puts("                  <tr>");
	puts("                     <td>");
	puts("                        <hr size=1 noshade >");
	puts("                     </td>");
	puts("                  </tr>");
	puts("               </table>");
	printf("   <form action=\"save_ip_settings.cgi\" method=\"GET\">\n");
	printf("   <table border=0 width=550 cellspacing=0 cellpadding=0>\n");
	printf("   	<tr>\n");
	printf("   		<td width=\"30%%\" valign=top> Gateway IP :</td>\n");
	printf("   		<td width=\"70%%\">\n");
	printf("   			<select id=\"lan_type\" name=\"lan_type\"  onChange=\"changetextbox()\">\n");
	printf("   			<%%  choice = getIndex(\"lan_type\");\n");
	printf("   			if ( choice == 0 ) {\n");
	if (strcmp(lan_type,"0")==0){
		printf("   			write( \"<option selected value=0>Static</option>\" );\n");
		printf("   			write( \"<option value=1>Automatic</option>\" );\n");
	}
	else{
		printf("   			write( \"<option  value=0>Static</option>\" );\n");
		printf("   			write( \"<option selected value=1>Automatic</option>\" );\n");
	}
	printf("   			}\n");
	printf("   			%%>\n");
	printf("   			</select>\n");
	printf("   		</td>\n");
	printf("   	</tr>\n");
	printf("   	<tr><td>&nbsp;</td><td></td></tr>\n");
	printf("   	<tr><td>&nbsp;</td><td></td></tr>\n");
	if (strcmp(lan_type,"0")==0){
		printf("      <tr>\n");
		printf("         <td width=\"30%%\" valign=top>IP Address :</td>\n");
		printf("         <td width=\"70%%\"><input type=\"text\" id=\"lan_ip\" name=\"lan_ip\" size=\"15\" maxlength=\"15\" value=\"%s\" maxlength=\"16\"> </td>\n", lan_ip);
		printf("     </tr>\n");
		printf("   	<tr><td>&nbsp;</td><td></td></tr>\n");
		printf("   	<tr>\n");
		printf("         <td width=\"30%%\" valign=top>Subnet Mask :</td>\n");
		printf("         <td width=\"70%%\"><input type=\"text\" id=\"lan_mask\" name=\"lan_mask\" size=\"15\" maxlength=\"15\" value=\"%s\" maxlength=\"16\"> </td>\n",lan_mask);
		printf("     </tr>\n");
		printf("   	<tr><td>&nbsp;</td><td></td></tr>\n");
		printf("   	<tr>\n");
		printf("   		<td width=\"30%%\" valign=top>Default Gateway :</td>\n");
		printf("   		<td width=\"70%%\"><input type=\"text\" id=\"lan_gateway\" name=\"lan_gateway\" size=\"15\" maxlength=\"15\" value=\"%s\" maxlength=\"16\"> </td>\n",lan_gateway);
		printf("   	</tr>\n");
	}
	else{
		printf("      <tr>\n");
		printf("         <td width=\"30%%\" valign=top>IP Address :</td>\n");
		printf("         <td width=\"70%%\"><input type=\"text\" id=\"lan_ip\" name=\"lan_ip\" size=\"15\" maxlength=\"15\" value=\"0.0.0.0\" maxlength=\"16\" disabled> </td>\n");
		printf("     </tr>\n");
		printf("   	<tr><td>&nbsp;</td><td></td></tr>\n");
		printf("   	<tr>\n");
		printf("         <td width=\"30%%\" valign=top>Subnet Mask :</td>\n");
		printf("         <td width=\"70%%\"><input type=\"text\" id=\"lan_mask\" name=\"lan_mask\" size=\"15\" maxlength=\"15\" value=\"0.0.0.0\" maxlength=\"16\" disabled> </td>\n");
		printf("     </tr>\n");
		printf("   	<tr><td>&nbsp;</td><td></td></tr>\n");
		printf("   	<tr>\n");
		printf("   		<td width=\"30%%\" valign=top>Default Gateway :</td>\n");
		printf("   		<td width=\"70%%\"><input type=\"text\" id=\"lan_gateway\" name=\"lan_gateway\" size=\"15\" maxlength=\"15\" value=\"0.0.0.0\" maxlength=\"16\" disabled> </td>\n");
		printf("   	</tr>\n");
	}
	printf("   	<tr><td>&nbsp;</td><td></td></tr>\n");
	printf("   <tr><td>&nbsp;</td><td></td></tr>\n");
	printf("   <tr><td>&nbsp;</td><td></td></tr>\n");
	printf("     <tr>\n");
	printf("         <td></td>\n");
	printf("         <td>\n");
	printf("   	<input type=\"submit\" name=\"save\" value=\"Save & Reboot\" onclick=\"return foo();\" />\n");
	printf("   	&nbsp;&nbsp;\n");
	printf("         <input type=\"reset\" value=\"Reset\" name=\"reset\" onClick=\"resetClick()\">\n");
	printf("         </td>\n");
	printf("     </tr>\n");
	printf("     </table>\n");
	printf("     <br>      \n");
	printf("    </form>\n");
	puts("            </div>");


	puts("            <div id=\"gw_server\" hidden>");
	puts("               <!--<blockquote>-->");
	puts("               <big>Gateway Server Setup</big>");
	puts("               <table border=\"0\" width=\"550\" cellspacing=\"0\" cellpadding=\"0\">");
	puts("                  <tbody>");
	puts("                     <tr>");
	puts("                        <td>");
	puts("                           <hr size=\"1\" noshade=\"\">");
	puts("                        </td>");
	puts("                     </tr>");
	puts("                     <tr>");
	puts("                        <td><small>");
	puts("                           This page allows configuration of gateway Server.");
	puts("                           </small>");
	puts("                        </td>");
	puts("                     </tr>");
	puts("                     <tr>");
	puts("                        <td>");
	puts("                           <hr size=\"1\" noshade=\"\">");
	puts("                        </td>");
	puts("                     </tr>");
	puts("                  </tbody>");
	puts("               </table>");
	puts("               <form action=\"save_server_conf.cgi\" method=\"GET\">");
	puts("                  <table border=\"0\" width=\"550\" cellspacing=\"0\" cellpadding=\"0\">");
	puts("                     <tbody>");
	puts("                        <tr>");
	puts("                           <td>&nbsp;</td>");
	puts("                           <td></td>");
	puts("                        </tr>");
	puts("                        <tr>");
	puts("                           <td width=\"30%\" valign=\"top\">Gateway Server Address:</td>");
	puts("                           <td width=\"70%\">");
	printf("<input type=\"text\" id=\"gw_server_adr\" name=\"gw_server_adr\" size=\"30\" maxlength=\"100\" value=\"%s\" >  </td>",server_address);
	puts("                        </tr>");
	puts("                        <tr>");
	puts("                           <td>&nbsp;</td>");
	puts("                           <td></td>");
	puts("                        </tr>");
	puts("                        ");
	puts("                        <tr>");
	puts("                           <td>&nbsp;</td>");
	puts("                           <td></td>");
	puts("                        </tr>");
	puts("                        <tr>");
	puts("                           <td>&nbsp;</td>");
	puts("                           <td></td>");
	puts("                        </tr>");
	puts("                        <tr>");
	puts("                           <td>&nbsp;</td>");
	puts("                           <td></td>");
	puts("                        </tr>");
	puts("                        <tr>");
	puts("                           <td></td>");
	puts("                           <td>");
	puts("                              <input type=\"submit\" name=\"save_gw_server\" value=\"Save &amp; Reboot\" onclick=\"return setGWServer();\">");
	puts("                              &nbsp;&nbsp;");
	puts("                              <input type=\"reset\" value=\"Reset\" name=\"reset\" onclick=\"resetClick()\">");
	puts("                           </td>");
	puts("                        </tr>");
	puts("                     </tbody>");
	puts("                  </table>");
	puts("                  <br>");
	puts("               </form>");
	puts("            </div>");

	puts("            <div id=\"pass_set\" hidden>");
	puts("               <!--<blockquote>-->");
	puts("               <big>Web Password Setup</big>");
	puts("               <table border=\"0\" width=\"550\" cellspacing=\"0\" cellpadding=\"0\">");
	puts("                  <tbody>");
	puts("                     <tr>");
	puts("                        <td>");
	puts("                           <hr size=\"1\" noshade=\"\">");
	puts("                        </td>");
	puts("                     </tr>");
	puts("                     <tr>");
	puts("                        <td><small>");
	puts("                           This page allows User to change user password.....");
	puts("                           </small>");
	puts("                        </td>");
	puts("                     </tr>");
	puts("                     <tr>");
	puts("                        <td>");
	puts("                           <hr size=\"1\" noshade=\"\">");
	puts("                        </td>");
	puts("                     </tr>");
	puts("                  </tbody>");
	puts("               </table>");
	puts("               <form action=\"save_pass_conf.cgi\" method=\"GET\">");
	puts("                  <table border=\"0\" width=\"550\" cellspacing=\"0\" cellpadding=\"0\">");
	puts("                     <tbody>");
	puts("                        <tr>");
	puts("                           <td>&nbsp;</td>");
	puts("                           <td></td>");
	puts("                        </tr>");
	printf("   	<tr><td>&nbsp;</td><td></td></tr>\n");
	printf("   	<tr><td>&nbsp;</td><td></td></tr>\n");
	if( access( "/home/etc/pass", F_OK ) != -1 ) {
	printf("      <tr>\n");

		printf("         <td width=\"30%%\" valign=top>Old Password :</td>\n");
	 

		printf("         <td width=\"70%%\"><input type=\"password\" id=\"old_pass\" name=\"old_pass\" size=\"15\" maxlength=\"15\" value=\"\" maxlength=\"16\"> </td>\n");
		printf("     </tr>\n");
	}
	printf("   	<tr><td>&nbsp;</td><td></td></tr>\n");
	printf("   	<tr>\n");
	printf("         <td width=\"30%%\" valign=top>New Password :</td>\n");
	printf("         <td width=\"70%%\"><input type=\"password\" id=\"new_pass\" name=\"new_pass\" size=\"15\" maxlength=\"15\" value=\"\" maxlength=\"16\"> </td>\n");
	printf("     </tr>\n");
	printf("   	<tr><td>&nbsp;</td><td></td></tr>\n");
	printf("   	<tr>\n");
	printf("   		<td width=\"30%%\" valign=top>Retype New Password:</td>\n");
	printf("   		<td width=\"70%%\"><input type=\"password\" id=\"retype_new_pass\" name=\"retype_new_pass\" size=\"15\" maxlength=\"15\" value=\"\" maxlength=\"16\"> </td>\n");
	printf("   	</tr>\n");
	printf("   	<tr><td>&nbsp;</td><td></td></tr>\n");
	puts("                        <tr>");
	puts("                           <td>&nbsp;</td>");
	puts("                           <td></td>");
	puts("                        </tr>");
	puts("                        ");
	puts("                        <tr>");
	puts("                           <td>&nbsp;</td>");
	puts("                           <td></td>");
	puts("                        </tr>");
	puts("                        <tr>");
	puts("                           <td>&nbsp;</td>");
	puts("                           <td></td>");
	puts("                        </tr>");
	puts("                        <tr>");
	puts("                           <td>&nbsp;</td>");
	puts("                           <td></td>");
	puts("                        </tr>");
	puts("                        <tr>");
	puts("                           <td></td>");
	puts("                           <td>");
	puts("                              <input type=\"submit\" name=\"save_gw_server\" value=\"Save &amp; Reboot\" onclick=\"return setGWServer();\">");
	puts("                              &nbsp;&nbsp;");
	puts("                              <input type=\"reset\" value=\"Reset\" name=\"reset\" onclick=\"resetClick()\">");
	puts("                           </td>");
	puts("                        </tr>");
	puts("                     </tbody>");
	puts("                  </table>");
	puts("                  <br>");
	puts("               </form>");
	puts("            </div>");	



	puts("	        <div id=\"content-about\" hidden>");
#if 1	
	file=fopen("/home/etc/customer.profile.about", "r");

	if (file!=NULL){
		while (fgets(command, sizeof(command), file)) {
			printf("%s",command);
		}
		fclose(file);
	}
	else {
#endif		

		puts("                            	<div class=\"section\">");
		puts("                                <h2>ABOUT GSHARE</h2>");
		puts("                                <p style=\"padding-top:50px;\">");
		puts("Gshare is an offshore software house with many expertises that will benefit your company. You can get your own dedicated software teams that will cover your software needs and get your software projects done by highly educated experts. All this at a very affordable price.</p>");
		puts("<p>");
		puts("We work hard to offer a very high standard in all our fields and we do our best to get the highest out of our teams. We not only gather our teams to have the best qualifications but also good colleagues that work in a friendly and professional environment.</p>");
		puts("<p>Gshare simplifies the outsourcing of your software projects.</p>                          ");
		puts("							</div>");
	}
	puts("                           ");
	puts("                         ");
	puts("                        ");
	puts("                            ");
	puts("                            </div>");
	puts("	        <div id=\"content-contact\" hidden>");
	file=fopen("/home/etc/customer.profile.contact", "r");

#if 1
	if (file!=NULL){
		while (fgets(command, sizeof(command), file)) {
			printf("%s",command);
		}
		fclose(file);
	}
	else {
#endif		
		puts("                            	<div class=\"section\">");
		puts("                                <h2>CONTACT US</h2>");
		puts("                                <div class=\"col\" style=\"left:5em;\">");
		puts("                                    <h4>Danish Headquarter</h4>");
		puts("                                    	GShare ApS<br> ");
		puts("                                      Bellisvej 15<br>");
		puts("                                      6430 Nordborg <br>");
		puts("                                      Denmark <br><br>");
		puts("                                      ");
		puts("                                      Phone: +45 7349 0000 <br>");
		puts("                                      Email: ak@gshare.dk<br>");
		puts("                                      Skypeid: allankacz");
		puts("                                </div>");
		puts("                                    <div class=\"col\" style=\"left:16em;\">");
		puts("                                    <h4>Pakistani Division</h4>");
		puts("                                    Global Share (SMC-Pvt) Ltd <br>");
		puts("				    Office No. 11/12, Al-Rehman Mall, Plot 33, G-11 Markaz./<br>");
		puts("                                    Islamabad Pakistan<br>");
		puts("                                    <br><b>Country Manager</b><br>");
		puts("                                    Kaif Haqqi<br>");
		puts("                                    <br>");
		puts("                                    Phone:  +92-51-8736092<br>");
		puts("                                    Email: kaif.haqqi@gshare.dk<br>");
		puts("                                    Skype:kaifhaqqi");
		puts("                                    </div>          ");
		puts("							</div>");
	}
	puts("                         ");
	puts("                         ");
	puts("                        ");
	puts("                            ");
	puts("                            </div>");
	puts("                 </div>");
	puts("</td>		");
	puts("  </tr>");
	puts("</table>");
	puts("         </div>");
	puts("         <div id=\"footer\">");
	puts("            Copyright &copy; Lodam");
	puts("         </div>");
	puts("      </div>");
	puts("   </body>");
	puts("   <script>");
	puts("      function foo() {");
	puts("          //alert(\"Submit button clicked!\");");
	puts("          if (document.getElementById(\"lan_type\").value == \"0\")");
	puts("          {");
	puts("          if(!checkIsIPV4(document.getElementById(\"lan_gateway\").value)){");
	puts("      	    alert(\"Try Again,Default Gateway Wrong!\");");
	puts("      	return false;");
	puts("      	}");
	puts("          if(!checkIsIPV4(document.getElementById(\"lan_ip\").value)){");
	puts("      	    alert(\"Try Again,IP Address Wrong!\");");
	puts("      	return false;");
	puts("      	}");
	puts("          if(!checkIsIPV4(document.getElementById(\"lan_mask\").value)){");
	puts("      	    alert(\"Try Again,Subnet Mask Wrong!\");");
	puts("      	return false;");
	puts("      	}");
	puts("          return true;");
	puts("      	}");
	puts("      	else");
	puts("      	return true;");
	puts("          }");
	puts("      function myFunction() {");
	puts("          alert(\"Page is loaded\");");
	puts("      }");
	puts("      function checkIsIPV4(entry) {");
	puts("        ");
	puts("        var blocks = entry.split(\".\");");
	puts("        if(blocks.length === 4) {");
	puts("          return blocks.every(function(block) {");
	puts("            return parseInt(block,10) >=0 && parseInt(block,10) <= 255;");
	puts("          });");
	puts("        }");
	puts("        return false;");
	puts("      }");
	puts("      function changetextbox()");
	puts("      {");
	puts("          if (document.getElementById(\"lan_type\").value == \"0\") {");
	puts("              document.getElementById(\"lan_ip\").removeAttribute(\"disabled\");");
	puts("      	document.getElementById(\"lan_mask\").removeAttribute(\"disabled\");");
	puts("      	document.getElementById(\"lan_gateway\").removeAttribute(\"disabled\");");
	puts("      ");
	puts("          } else {");
	puts("              document.getElementById(\"lan_ip\").disabled=\'true\';");
	puts("              document.getElementById(\"lan_mask\").disabled=\'true\';");
	puts("              document.getElementById(\"lan_gateway\").disabled=\'true\';");
	puts("          }");
	puts("      }");
	puts("   </script>");
	puts("</html>");

	free(query_string);
	free(request_method);
	free(content_length);
	//system("pwd >> testcat");


	return 0;
}



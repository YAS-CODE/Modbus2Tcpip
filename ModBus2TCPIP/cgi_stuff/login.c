#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*****
*
*	This file was generated with html2c 0.2. It uses puts() function calls
*	for speed. Wherever you need to insert dynamic data, change the call
*	to printf().
*
*	NOTE: If the line has '%' symbols as part of table sizing, printf()
*	will get confused. Split the line at the point where the data
*	should be inserted and use a printf() just for that. Then
*	continue outputting the line with a puts() call on another line.
*
*****/
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


int main(void)
{

	char header_background_color[10],navigation_color[10],command[100],temp[10],header_logo_background_color[10];


	int index,length;
	if( access( "/home/etc/auth", F_OK ) != -1 ) {
		showHome();
		return 0;
	}

	memset(header_background_color,0,10);
	memset(navigation_color,0,10);
	memset(header_logo_background_color,0,10);
	//strcpy(header_background_color,"#B51A1D");
	strcpy(header_background_color,"#808080");
	strcpy(header_logo_background_color,"#808080");

	//strcpy(navigation_color,"red");
	strcpy(navigation_color,"#333");

	//system("hello >>../etc/boa-test");
	FILE* file=fopen("/home/etc/customer.profile", "r");

	if (file!=NULL){
		while (fgets(command, sizeof(command), file)) {
			index=0;
			length=strlen(command);
			while(command[index]!='=')
				index++;
			if(strncmp(command,"HEADER_BACK_COL",strlen("HEADER_BACK_COL"))==0)
				strncpy(header_background_color,command+index+1,length-index-2);
			if(strncmp(command,"NAV_TEXT_COL",strlen("NAV_TEXT_COL"))==0){
				strncpy(navigation_color,command+index+1,length-index-2);

			}
			if(strncmp(command,"HEADER_LOGO_BACK_COL",strlen("HEADER_LOGO_BACK_COL"))==0)
				strncpy(header_logo_background_color,command+index+1,length-index-2);
		}
		fclose(file);
	}


	// The next line may not be needed depending on the application.
	puts("Content-type: text/html\n");
	puts("<!DOCTYPE html>");
	puts("<html>");
	puts("   <head>");
	puts("      <title>Remote Caretaking Gateway</title>");
	puts("      <script src=\"/jquery.min.js\"></script>");
	puts("      <link rel=\"stylesheet\" type=\"style/css\" >");
	puts("      <style>");
	puts("         body {");
	puts("         background-color: #EEE;");
	puts("         font-family: Helvetica, Arial, sans-serif;");
	puts("         }");
	puts("         a {");
	puts("         text-decoration: none;");
	printf("   color: %s;\n",navigation_color);
	puts("         }");
	puts("         h1, h2, h3 {");
	puts("         margin: 0;");
	puts("         }");
	puts("         #nav ul {");
	puts("         list-style-type: none;");
	puts("         padding: 0px;");
	puts("         }");
	puts("         #header {");
	puts("         padding: 0px;");
	printf("   background-color: %s;\n",header_background_color);
	puts("         color: white;");
	puts("         height: 110px;");
	puts("         /*    text-align: center;*/");
	puts("         }");
	puts("         #TopMenuRight{");
	puts("         width: 85%;");
	puts("         float:left;");
	puts("         font: 14px \"Arial\",Helvetica,Arial,sans-serif;");
	puts("         }");
	puts("         .top-headline {");
	puts("         color: #fff;");
	puts("         display: inline-block;");
	puts("         margin-top: 45px;");
	puts("         }");
	puts("         .h2m {");
	puts("         padding-top: 8px;");
	puts("         font-size: 21px;");
	puts("         font-weight: bold;");
	puts("         padding-left: 80px;");
	puts("         }");
	puts("         #TopMenuLeft{");
	puts("         width:15%;");
	printf("   background-color: %s;\n",header_logo_background_color);
	puts("         height: 110px;");
	puts("         float:left;");
	puts("         }");
	puts("         #TopMenuLogo");
	puts("         {");
	puts("         background:url(\"/images/nilan_logo.svg\") no-repeat center center;");
	puts("         background-size: 75%;");
	puts("         height:110px;");
	puts("         }");
	puts("         #container {");
	puts("         background-color: white;");
	puts("         width: 800px;");
	puts("         margin-left: auto;");
	puts("         margin-right: auto;");
	puts("         }");
	puts("         #content {");
	puts("         padding: 10px;");
	puts("         }");
	puts("         #nav {");
	puts("         width: 180px;");
	puts("         float: top;");
	puts("         height: 100%;");
	puts("         bottom: 0;");
	puts("         }");
	puts("         #nav ul li a {");
	puts("         display: inline-block;");
	puts("         padding: 10px 15px;");
	puts("         }");
	puts("         #main {");
	puts("         width: 600px;");
	puts("         float: right;");
	puts("         }");
	puts("         #footer {");
	puts("         clear: both;");
	puts("         padding: 0px;");
	puts("         background-color: #999;");
	puts("         color: white;");
	puts("         text-align: right;");
	puts("         }");
	puts("         #nav .selected {");
	puts("         font-weight: bold;");
	puts("         }");
	puts("          h2{");
	puts("text-align: center;");
	puts("font-size: 24px;");
	puts("}");
	puts("hr{");
	puts("margin-bottom: 30px;");
	puts("}");
	puts("div.container{");
	puts("width: 960px;");
	puts("height: 610px;");
	puts("margin:50px auto;");
	puts("font-family: \'Droid Serif\', serif;");
	puts("position:relative;");
	puts("}");
	puts("div.main{");
	puts("width: 320px;");
	puts("margin-top: 80px;");
	puts("float:left;");
	puts("padding: 10px 55px 40px;");
	puts("border: 15px solid white;");
	puts("font-size: 13px;");
	puts("}");
	puts("input[type=text],[type=password] {");
	puts("width: 97.7%;");
	puts("height: 34px;");
	puts("padding-left: 5px;");
	puts("margin-bottom: 20px;");
	puts("margin-top: 8px;");
	puts("border: 2px solid ;");
	puts("color: #4f4f4f;");
	puts("font-size: 16px;");
	puts("}");
	puts("label{");
	puts("color: #464646;");
	puts("text-shadow: 0 1px 0 #fff;");
	puts("font-size: 14px;");
	puts("font-weight: bold;");
	puts("}");
	puts("#login {");
	puts("width: 100%;");
	printf("   background-color: %s;\n",header_background_color);
	puts("font-size: 20px;");
	puts("margin-top: 15px;");
	puts("padding: 8px;");
	puts("font-weight: bold;");
	puts("cursor: pointer;");
	puts("color: white;");
	puts("text-shadow: 0px 1px 0px #13506D;");
	puts("}");
	puts("      </style>");
	puts("     ");
	puts("   </head>");
	puts("   <body>");
	puts("<div id=\"container\">");
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
	puts("            </div>");
	puts("	</td>");
	puts("    <td>");
	puts("	<div>");
	puts("            ");
	puts("                        <div class=\"container\">");
	puts("<div class=\"main\">");
	printf("   <form action=\"auth.cgi\" method=\"GET\">\n");
	puts("<label>Account :</label>");
	puts("<input type=\"text\" name=\"demail\" id=\"email\" maxlength=\"16\" >");
	puts("<label>Password :</label>");
	puts("<input type=\"password\" name=\"password\" id=\"password\"size=\"15\" maxlength=\"15\"  maxlength=\"16\">");
	puts("<input type=\"submit\" name=\"login\" id=\"login\" value=\"Login\">");
	puts("</form>");
	puts("</div>");
	puts("</div>");
	puts("            ");
	puts("	        ");
	puts("	        ");
	puts("    </div>");
	puts("</td>		");
	puts("  </tr>");
	puts("</table>");
	puts("         </div>");
	puts("         <div id=\"footer\">");
	puts("            Copyright &copy; Lodam");
	puts("         </div>");
	puts("      </div>");
	puts("   </body>");
	puts("   ");
	puts("</html>");
	return 0;
}


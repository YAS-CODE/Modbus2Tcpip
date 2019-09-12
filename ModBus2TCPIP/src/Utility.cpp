/*
 * Utility.cpp
 *
 *  Created on: Mar 2, 2015
 *      Author: imtiazahmed
 */

#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include "Utility.h"
#include <stdlib.h>     /* strtoul */
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>

Utility::Utility() {

	m_server_port = 5001;
#if DEVICE == 1
	m_server_address = "datacenter.lodam.com"; //"datacenter.lodam.com";  "192.168.40.78"   "lodamapp-dev-env.elasticbeanstalk.com"
#else
	m_server_address = "192.168.40.78";
#endif


	m_configuration_read = false;
	m_config_file_name = "/home/Moxa/config.txt";

#if DEVICE == 1
	m_device_numbers = "28,29,30";
#else
	m_device_numbers = "31";
#endif
	modbus_address= "/dev/ttyM0";
	m_devices_count = 10;//8;
	m_allow_device = "0";
	log_enabled=1;
	modbus_delay=35;

	have_last_config=false;
	logfilecount=0;

}

Utility::~Utility() {

}

void Utility::loadConfiguration() {


	FILE *fp;
	char buf[100];

	string str;


	fp = fopen(m_config_file_name.c_str(), "r");

	if( fp == NULL )
	{
		perror("Error while opening the file.\n");
		return;
	}

	DEBUG_PRINT(("Configuration file found. Reading contents form %s  :\n", m_config_file_name.c_str()));

	while (fgets(buf,1000, fp) != NULL){

		string line (buf);

		if (line.find("server_address=") != string::npos) {
			int start = line.find("=") + 1;
			int end = line.size()  - start - 1;
			str = line.substr(start, end);
			m_server_address = str.substr();
			cout << m_server_address << endl;
		}
		else if (line.find("modbus_address=") != string::npos) {
			int start = line.find("=") + 1;
			int end = line.size()  - start - 1;
			str = line.substr(start, end);
			modbus_address = str.substr();
			cout << modbus_address << endl;
		}
		else if (line.find("server_port_1_0_4=") != string::npos) {
			str = line.substr(line.find("=") + 1);
			m_server_port = atoi(str.c_str());
			//				cout << m_server_port << endl;
		}
		else if (line.find("logging_enabled=") != string::npos) {
			str = line.substr(line.find("=") + 1);
			log_enabled = atoi(str.c_str());
			//				cout << m_server_port << endl;
		}
		else if (line.find("modbus_read_write_delay=") != string::npos) {
			str = line.substr(line.find("=") + 1);
			modbus_delay = atoi(str.c_str());
			//				cout << m_server_port << endl;
		}
		else if (line.find("device_numbers=") != string::npos) {
			str = line.substr(line.find("=") + 1);
			m_device_numbers = str;
			//				cout << m_device_numbers << endl;
		}
		else if (line.find("allow_modbus_id=") != string::npos) {
			str = line.substr(line.find("=") + 1);
			m_allow_device = str;
			//				cout << m_device_numbers << endl;
		}
	}


	fclose(fp);
	readDevList();

	m_configuration_read = true;

}

/**
 * Get the address of server to connect to.
 */
string Utility::getServerAddress() {
	return m_server_address;
}

/**
 * Get the address of Modbus to connect to.
 */
string Utility::getModbusAddr() {
	return modbus_address;
}
/**
 * Get the Allowed Device id only.
 */
string Utility::getAllowedDev() {
	return m_allow_device;
}
/**
 * Get the address of server to connect to.
 */
int Utility::getServerPort() {
	return m_server_port;
}

/**
 *  returns the devices ids in string
 */
string Utility::getDevicesIDs() {
	return m_device_numbers;
}


int Utility::getDevicesCount() {
	return m_devices_count;
}


int Utility::getModbusDelay() {
	return modbus_delay;
}
int Utility::getSDCardLoggingState() {
	return log_enabled;
}
/*find string from another string
parameter:
str: string from whcih pattern needed to be find out
find: pattern which needed to find out
strln: length of string
findln: pattern length
str_p: startng point from where patter should start to find out
output:
index in string where pattern has been find out, -1 on unable to find
 */
int Utility::findpattren(unsigned char *str, unsigned char *find, int strln,int findln,int str_p){
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

int Utility::readDevList(){
	FILE * fdev;
	char buff[500];
	char temp[500];
	fdev = fopen ("/home/Moxa/last_devs","r");
	if (fdev == NULL)
	{
		have_last_config=false;
		DEBUG_PRINT(("Error opening file! data_cashe_temp\n"));
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
	memset(temp,0,500);
	strncpy(temp,buff,length-1);
	m_device_numbers=temp;
	have_last_config=true;
	//strncpy(m_device_numbers,buff,length-1);
	return 0;

}


int Utility::saveDevConf(uint8_t* pack, int len,struct s_dev_config* dev_config, int total_dev){
	FILE * fdev;
	char buff[500];
	char temp[10];
	fdev = fopen ("/home/Moxa/last_devs","w");
	if (fdev == NULL)
	{
		DEBUG_PRINT(("Error opening file! /home/Moxa/last_devs\n"));
		//fclose(fmac);
		return 1;

	}
	memset(buff, 0, 500);
	for (int d=0; d<total_dev; d++){
		sprintf(temp,"%d,",dev_config[d].device_id);
		strcat(buff,temp);
	}	
	fwrite(buff, 1, strlen(buff), fdev);
	fclose(fdev);

	fdev = fopen ("/home/Moxa/last_dev_conf","wb");
	if (fdev == NULL)
	{
		DEBUG_PRINT(("Error opening file! /home/Moxa/last_devs\n"));
		//fclose(fmac);
		return 1;

	}
	fwrite(pack, 1, len, fdev);
	fclose(fdev);
	return 0;

}


int Utility::saveGatewayConnStat(int gateway_stat){
	FILE * fdev;
	char buff[5];

	fdev = fopen ("/home/logs/Gateway_conn_stat","w");
	if (fdev == NULL)
	{
		DEBUG_PRINT(("Error opening file! /home/Moxa/last_devs\n"));
		//fclose(fmac);
		return 1;

	}
	memset(buff, 0, 5);

	sprintf(buff,"%d",gateway_stat);
	fwrite(buff, 1, strlen(buff), fdev);
	fclose(fdev);
	return 0;

}

/**
 * Get the address of server to connect to.
 */
bool Utility::loadDevices(struct s_dev_config* dev_config) {


	int max_size = 10;
	int current_position = 0;
	int last_position = 0;
	int def_register_ids[] = { 203, 221, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 1000, 1001, 1002, 1003, 1004 };
	int def_register_type[] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 4, 3 };
	int def_utc_register_ids[]={300,301,302,303,304,305};
	m_devices_count = 0;

	int device_id = 0;

	if(m_device_numbers.size() != 0)
	{

		while(true)
		{

			current_position = m_device_numbers.find(",", current_position);
			device_id = atoi(m_device_numbers.substr(last_position, current_position).c_str());

			if(device_id < 255)
			{
				//				devices_array[m_devices_count++] = device_id;

				dev_config[m_devices_count].device_id=device_id;
				dev_config[m_devices_count].status_reg.total_registers_count=17;
				for(int r=0; r<dev_config[m_devices_count].status_reg.total_registers_count; r++){
					dev_config[m_devices_count].status_reg.register_address[r]=def_register_ids[r];
					dev_config[m_devices_count].status_reg.register_type[r]=def_register_type[r];
				}

				dev_config[m_devices_count].utc_reg.total_registers_count=6;
				for(int r=0; r<dev_config[m_devices_count].utc_reg.total_registers_count; r++){
					dev_config[m_devices_count].utc_reg.register_address[r]=def_utc_register_ids[r];
					dev_config[m_devices_count].utc_reg.register_type[r]=3;
				}

				m_devices_count++;
				//				cout << devices_array[m_devices_count - 1] << endl;
				//printf("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__);
			}

			if(current_position == -1)
			{
				break;
			}


			if(m_devices_count >= max_size)
			{
				break;
			}

			last_position = current_position + 1;
			current_position++;

		}
	}

	return (m_devices_count != 0);
}

int Utility::setMacAddr(unsigned char* mac){
	FILE * fmac;
	fmac = fopen ("/home/Moxa/last_mac","wb");
	if (fmac == NULL)
	{
		DEBUG_PRINT(("Error opening file! data_cashe_temp\n"));
		//fclose(fmac);
		return 1;

	}
	fwrite(mac, 1, 6, fmac);
	fclose(fmac);
	return 0;

}

int Utility::readMacAddr(unsigned char* mac){
	FILE * fmac;
	long length;
	fmac = fopen ("/home/Moxa/last_mac","rb");
	if (fmac == NULL)
	{
		DEBUG_PRINT(("Error opening file! data_cashe_temp\n"));
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

unsigned char* Utility::getMACAddress() {

	struct ifreq ifr;
	struct ifconf ifc;
	const int buf_length = 256;
	char buf[buf_length];
	int success = 0;
	int sock;


	//zero out the array
	for(int i = 0; i < 6; i++)
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
			DEBUG_PRINT(("yas %d: success=%d func=%s, file=%s\n",__LINE__,success,__FUNCTION__,__FILE__));
		}while(!success);
		if (success) {

			memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
			setMacAddr(mac_address);


		}
	}
	DEBUG_PRINT(("yas %d: success=%d mac_address=%s func=%s, file=%s\n",__LINE__,success,mac_address,__FUNCTION__,__FILE__));

	return mac_address;

}


bool Utility::clearBytes(unsigned char* array, int start, int end) {


	if(start > end)
	{
		return false;
	}

	for (int i = start; i < end; i++)
		array[i] = 0;


	return true;
}



void Utility::printBytes(unsigned char* array, int start, int end) {

	//#if DEBUG == 1

	if(start > end)
	{
		DEBUG_PRINT(("Wrong start and end values given for printing bytes \n"));
	}


	DEBUG_PRINT(("\n Data = "));

	int i;

	for ( i = start; i < end; i++){
		if (i > 0) {
			DEBUG_PRINT((":"));
		}

		DEBUG_PRINT(("%02X", array[i]));

	}

	//#endif

}



/**
 * Write given value in big Endian notation
 * @param value  Value to write
 * @param array  Buffer to write to
 * @param start_position   Position to write at
 */
void Utility::loadIntInArray(int value, unsigned char* array, int start_position) {

	for (int i = 0; i < 2; i++){
		array[start_position + 1 - i] = (value >> (i * 8));
		//printf("%d, %02X, \n", start_position + i, array[start_position + i]);
	}

	//	printf("\n");
}


/**
 * Read an int value from given array, starting from given position
 */
int Utility::getIntFromByte(unsigned char* array,
		int start_position) {

	return  array[start_position + 1] | array[start_position] << 8;

}


void Utility::loadLongInArray(unsigned long int longInt, unsigned char* byteArray, int start_position){
	// convert from an unsigned long int to a 4-byte array
	/*	byteArray[start_position] = (int)((longInt >> 24) & 0xFF) ;
		byteArray[start_position+1] = (int)((longInt >> 16) & 0xFF) ;
		byteArray[start_position+2] = (int)((longInt >> 8) & 0XFF);
		byteArray[start_position+3] = (int)((longInt & 0XFF));*/
	unsigned char byte;
	for ( int index = start_position; index < start_position+8; index ++ ) {
		byte = longInt & 0xff;
		byteArray [ index ] = byte;
		longInt = (longInt - byte) / 256 ;
	}
}

long int Utility::getLongIntFromByte(unsigned char* byteArray, int start_position){
	/*unsigned long int anotherLongInt = ( (byteArray[start_position] << 24) 
	  + (byteArray[start_position+1] << 16) 
	  + (byteArray[start_position+2] << 8) 
	  + (byteArray[start_position+3] ) );
	  return anotherLongInt;*/

	long int value = 0;
	for ( int i = (start_position+8) - 1; i >= (start_position); i--) {
		value = (value * 256) + byteArray[i];
	}

	return value;
}



void Utility::loadIntInArrayLE(unsigned int Int, unsigned char* byteArray, int start_position){
	// convert from an unsigned long int to a 4-byte array
	/*      byteArray[start_position] = (int)((longInt >> 24) & 0xFF) ;
		byteArray[start_position+1] = (int)((longInt >> 16) & 0xFF) ;
		byteArray[start_position+2] = (int)((longInt >> 8) & 0XFF);
		byteArray[start_position+3] = (int)((longInt & 0XFF));*/
	unsigned char byte;
	for ( int index = start_position; index < start_position+2; index ++ ) {
		byte = Int & 0xff;
		byteArray [ index ] = byte;
		Int = (Int - byte) / 256 ;
	}
}

int Utility::getIntFromArray(unsigned char* byteArray, int start_position){
	/*unsigned long int anotherLongInt = ( (byteArray[start_position] << 24) 
	  + (byteArray[start_position+1] << 16) 
	  + (byteArray[start_position+2] << 8) 
	  + (byteArray[start_position+3] ) );
	  return anotherLongInt;*/

	long int value = 0;
	for ( int i = (start_position+2) - 1; i >= (start_position); i--) {
		value = (value * 256) + byteArray[i];
	}

	return value;
}

unsigned Utility::get_file_size (const char * file_name)
{
	struct stat sb;
	if (stat (file_name, & sb) != 0) {
		fprintf (stderr, "'stat' failed for '%s': %s.\n",
				file_name, strerror (errno));
		//exit (EXIT_FAILURE);
		return 0;
	}
	DEBUG_PRINT(("yas %d: file_size=%ld func=%s, file=%s\n",__LINE__,sb.st_size,__FUNCTION__,__FILE__));
	return sb.st_size;
}


unsigned Utility::runBashCom (const char * com)
{
	FILE* com_fd = popen(com, "r"); 

	if (com_fd == NULL) 

	{ 

		DEBUG_PRINT(("FAIL!\n")); 

		return 1; 

	} 



	char buffer[1028]; 



	while (fgets(buffer, 1028, com_fd) != NULL) 

	{ 

		DEBUG_PRINT(("%s",buffer)); 

	} 



	pclose(com_fd); 

	DEBUG_PRINT(("Success.\n")); 



	return 0;
}

int Utility::fileExist(const char * pathname)
{
	struct stat st = {0};
	if (stat((char *)pathname, &st) != -1) {
		return 1;
	}
	else
		return 0;

}

int Utility::stringlog(const char *buf, int len,...){
#if 1    
	if(log_enabled){
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

		
		sprintf(logfilename, "/mnt/sd/gateway_logs/Gateway%ld.log",logfilecount);
		t = time(NULL);

		tm = *localtime(&t);
		sprintf(timestamp,"%d-%d-%d %d:%d:%d :-", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		do{

		if( access( logfilename, F_OK ) != -1 ){ // if file exist 

			if( get_file_size (logfilename)>90000){
				do{
					sprintf(logfilename, "/mnt/sd/gateway_logs/Gateway%ld.log",logfilecount++);
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

int Utility::initlogger(){
#if 0
	int portno, n;
    struct hostent *logserver;
    char hostname[50];


    if(log_enabled){
    strcpy(hostname,"127.0.0.1");
    portno=48000;
    /* socket: create the socket */
    loggerfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (loggerfd < 0) 
        return 0;

    /* gethostbyname: get the logserver's DNS entry */
    logserver = gethostbyname(hostname);
    if (logserver == NULL) {
       return 0;
    }

    /* build the logserver's Internet address */
    bzero((char *) &loggeraddr, sizeof(loggeraddr));
    loggeraddr.sin_family = AF_INET;
    bcopy((char *)logserver->h_addr, (char *)&loggeraddr.sin_addr.s_addr, logserver->h_length);
    loggeraddr.sin_port = htons(portno);
	}
#endif
}

#if 0
void Utility::process_mem_usage(double& vm_usage, double& resident_set)
{

	using std::ios_base;
	using std::ifstream;
	using std::string;

	vm_usage     = 0.0;
	resident_set = 0.0;

	// 'file' stat seems to give the most reliable results
	//
	ifstream stat_stream("/proc/self/stat",ios_base::in);

	// dummy vars for leading entries in stat that we don't care about
	//
	string pid, comm, state, ppid, pgrp, session, tty_nr;
	string tpgid, flags, minflt, cminflt, majflt, cmajflt;
	string utime, stime, cutime, cstime, priority, nice;
	string O, itrealvalue, starttime;

	// the two fields we want
	//
	unsigned long vsize;
	long rss;

	stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
		>> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
		>> utime >> stime >> cutime >> cstime >> priority >> nice
		>> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

	stat_stream.close();

	long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
	vm_usage     = vsize / 1024.0;
	resident_set = rss * page_size_kb;

}
#endif

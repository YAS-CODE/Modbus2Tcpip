/*
 * LodamModbusProject.cpp
 *
 *  Created on: Feb 14, 2015
 *      Author: imtiazahmed
 */

#include <iostream>
#include "ModbusController.h"
#include "ServerController.h"
#include "GatewayCom.h"
#include "Utility.h"
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


using namespace std;

pid_t cpid; 

int wd_sock;

struct sockaddr_in server;

//bool mod_bus_ready=false;


//int total_devices_count = 11;

//int t_register_ids[] = { 300,301,302,303,304,305 };


//int total_register_count = 17;
string serverAddress = "";
string modbusAddress = "";

int Allow_dev_only = 0;
int serverPort = 0;
int modbus_interval = 0;
bool show_output = true;
//unsigned long long ctr = 0;

Utility utility;

unsigned char* mac_address;

static struct s_timeout_info timeout_info[10];

static struct s_dev_config dev_config[MAX_NUM_DEV];
//static struct gateway_logger logger;
//unsigned char m_reply_message[MAX_MESSAGE_LENGTH];



//time_t last_update_t;

bool wd_connected=false;

ModbusController modbusController;
ServerController serverController;

GatewayCom gateway_com;

static struct s_dev_data dev_data[MAX_NUM_DEV];
time_t last_log_t;

#define MAX_BUF 1024


int updateTimeOut(struct s_dev_config* dev_config,int curr_dev);
int logTimeOut();

/**
 * Loop and read status from devices using Modbus protocol
 * and send data to server
 */
int work() {


	try {

		int total_bytes = 0;
		int change_counter;
		int timer = 0;
		FILE * fdev;
		char dev_log_buff[500];
		char log_temp[10];
		//serverController.setIDForCommunication(mac_address);
		if (!modbusController.establishConnection(modbusAddress)) {
			DEBUG_PRINT((" Can not connect to USB \n"));
			//			return 0;
		}

		bool connected = false;
		bool cacheNow = false;
		int read_length = 0;
		unsigned char* data_from_server;
		unsigned char* data_from_modbus;
		int reply_length;
		//time(&last_update_t);
		bool update_timeout=true;
		last_log_t=0;
		gateway_com.mod_bus_ready=utility.have_last_config;
		DEBUG_PRINT(("yas %d: utility.have_last_config=%d func=%s, file=%s\n",__LINE__,utility.have_last_config,__FUNCTION__,__FILE__));
		do
		{
			show_output = false;
			if(wd_sock && wd_connected)
				write(wd_sock, "ping", 4);
			connected=serverController.isConnected();

			//			fdev = fopen ("/home/logs/devs_state_c","w+");
			utility.saveGatewayConnStat(connected);
			strcpy(dev_log_buff,"");

			//if not connected, try to connect to server
			if (connected!=true) {
				memset( dev_data ,0,sizeof(s_dev_data)* MAX_NUM_DEV);
				connected = serverController.connectWithServer(serverAddress,serverPort,wd_sock,wd_connected);

				if (connected) {
					DEBUG_PRINT(("yas %d: connected=%d func=%s, file=%s\n",__LINE__,connected,__FUNCTION__,__FILE__));
					serverController.setIDForCommunication(mac_address);
					//send welcome message, the server will either return success
					//or will return with the values to be written to the registers
					serverController.sendWelcomeMessage(timeout_info);
					gateway_com.mod_bus_ready=false;	
					last_log_t=0;					

				} else {
					if (gateway_com.mod_bus_ready==false || gateway_com.total_devices_count<1){
						sleep(1);
						continue;
					}
					//connection with server failed, exit the program
					//watch dog will start it again
					//cout << "CONNECTION WITH SERVER FAILED..." << endl;
					//	return 0;
#if 1				
					time_t log_t = time(0);
					cacheNow=logTimeOut();
					for (int device_ctr = 0; device_ctr < gateway_com.total_devices_count;
							device_ctr++) {

						//if device id on given index is 0, move to next one
						if (dev_config[device_ctr].device_id == 0) {
							continue;
						}
#if 0
						if(Allow_dev_only){
							if(Allow_dev_only!=dev_config[device_ctr].device_id)
								continue;
						}
#endif
						timer = 0;
						change_counter = 0;
						if (show_output) {
							cout << endl
								<< "==============================================";
							cout << endl << "Device_id = " << /*device_ids[device_ctr]*/dev_config[device_ctr].device_id
								<< endl;
							cout << endl;
						}
						gateway_com.cache_file_index=10;
						total_bytes = modbusController.readRegistersOfADevice(dev_config[device_ctr],dev_data[device_ctr],change_counter,OPT_ZERO_VAL);                                      
						DEBUG_PRINT(("yas %d: Device_id =%d  gateway_com.total_devices_count=%d total_bytes=%d func=%s, file=%s\n",__LINE__,dev_config[device_ctr].device_id,gateway_com.total_devices_count,total_bytes,__FUNCTION__,__FILE__));	
						if(total_bytes>0 && cacheNow){
							serverController.logDevicesStatus(dev_data[device_ctr], total_bytes,log_t);
							utility.stringlog(" Going for cache for id=%d ",strlen(" Going for cache for id=%d "),dev_config[device_ctr].device_id);
							cout << "CONNECTION WITH SERVER FAILED,LOG DATA NOW..." << endl;
						}

						if(fdev!=NULL){
							//memset(dev_log_buff, 0, 50);
							if(total_bytes)
								sprintf(log_temp,"%d=%d\n",dev_config[device_ctr].device_id,1);
							else
								sprintf(log_temp,"%d=%d\n",dev_config[device_ctr].device_id,0);
							strcat(dev_log_buff,log_temp);
						}

					}
#endif					
				}
			}

			//if connected, loop through the devices and send the data to server
			if (connected) {

				update_timeout=0;
				//check if value of any register is changed - if so send the values of all registers for that device only
				if (gateway_com.mod_bus_ready==false || gateway_com.total_devices_count<1){
					sleep(1);
					continue;
				}

				for (int device_ctr = 0; device_ctr < gateway_com.total_devices_count;
						device_ctr++) {

					//if device id on given index is 0, move to next one
					if (dev_config[device_ctr].device_id == 0) {
						DEBUG_PRINT(("yas %d: dev_config[%d].device_id=%d gateway_com.total_devices_count=%d func=%s, file=%s\n",__LINE__,device_ctr,dev_config[device_ctr].device_id,gateway_com.total_devices_count,__FUNCTION__,__FILE__));
						continue;
					}
					timer = 0;
					DEBUG_PRINT(("yas %d: Device_id =%d  gateway_com.total_devices_count=%d func=%s, file=%s\n",__LINE__,dev_config[device_ctr].device_id,gateway_com.total_devices_count,__FUNCTION__,__FILE__));
					change_counter = 0;

					if (show_output) {
						cout << endl
							<< "==============================================";
						cout << endl << "Device_id = " << /*device_ids[device_ctr]*/dev_config[device_ctr].device_id
							<< endl;
						cout << endl;
					}
#if 0
					if(Allow_dev_only){
						if(Allow_dev_only!=dev_config[device_ctr].device_id)
							continue;
					}
#endif
					total_bytes = modbusController.readRegistersOfADevice(dev_config[device_ctr],dev_data[device_ctr],change_counter,OPT_DATA_CHANGE_VAL);

					if(!change_counter)
						update_timeout=updateTimeOut(dev_config,device_ctr);

					//DEBUG_PRINT(("yas %d: update_timeout=%d,change_counter=%d dev_config[device_ctr].device_id=%d func=%s, file=%s\n",__LINE__,update_timeout,change_counter,dev_config[device_ctr].device_id,__FUNCTION__,__FILE__));
					if (update_timeout || change_counter > 0 /*1*/) {
						if (show_output) {
							cout << "\nSending data to server ";
						}
						//more than one register changed, send the data to server
						//bool status = serverController.sendDevicesStatus(devices_data[device_ctr], total_bytes);
						bool status = serverController.sendDevicesStatus(dev_config[device_ctr],dev_data[device_ctr], total_bytes);
						time(&dev_config[device_ctr].last_update_t);	
					}
					if(fdev!=NULL){
						//memset(dev_log_buff, 0, 50);
						if(total_bytes)
							sprintf(log_temp,"%d=%d\n",dev_config[device_ctr].device_id,1);
						else
							sprintf(log_temp,"%d=%d\n",dev_config[device_ctr].device_id,0);
						strcat(dev_log_buff,log_temp);
					}
					if (show_output) {
						cout << "=============================================="
							<< endl;
					}
				}
			}

			sleep(modbus_interval);
			fdev = fopen ("/home/logs/devs_state","w+");
			if (fdev != NULL){
				fwrite(dev_log_buff, 1, strlen(dev_log_buff), fdev);
				fclose(fdev);
			}
		}while (true);
		modbusController.releaseConnection();
	} catch (exception &e) {
		cout << "Standard exception: " << e.what() << endl;
	} catch (...) {
	}
	return 0;
}
int updateTimeOut(struct s_dev_config* dev_config,int curr_dev){
	time_t current;
	time(&current);
	double sec = difftime(current,dev_config[curr_dev].last_update_t);
	if(sec>20){
		dev_config[curr_dev].last_update_t=current;
		return 1;
	}
	else
		return 0;
}

int logTimeOut(){
	time_t current;
	time(&current);
	double sec = difftime(current,last_log_t);

	DEBUG_PRINT(("log_timeout_sec=%.f\n",sec));
	if(sec>/*896*/ 540){
		last_log_t=current;
		return 1;
	}
	else
		return 0;
}
void *Recevier(void*)
{

	try {

		bool connected = false;

		while(1){
			connected=serverController.isConnected();

			if (connected) {
				gateway_com.comParser(mac_address,timeout_info,dev_config);
			}
		}
	} catch (exception &e) {
		cout << "Standard exception: " << e.what() << endl;
	} catch (...) {
	}
}

void *startWatchdog(void*) {

	FILE *fp;
	char path[1035];
	const char *watchdogApp = "Watchdog";
	char client_message[500];
	int read_size;
	bool bFound = false;
	//	DEBUG_PRINT("yas %d: wd_sock=%d func=%s, file=%s\n",__LINE__,wd_sock,__FUNCTION__,__FILE__);
	fp = popen("/bin/ps | grep Watchdog", "r");
	if (fp == NULL) {
		DEBUG_PRINT(("Failed to run command\n"));
	}

	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path) - 1, fp) != NULL) {
		if (strstr(path, watchdogApp) != NULL
				&& strstr(path, "grep Watchdog ") == NULL) {
			bFound = true;
			break;
		}
	}
	/* close */
	pclose(fp);
	if (bFound == false) {
		DEBUG_PRINT(("\n\rStarting Watchdog..\n\r"));
		//system("Watchdog &");
	}
	wd_sock=0;
	while(1){
		//Create socket
		wd_connected=false;
		wd_sock = socket(AF_INET , SOCK_STREAM , 0);
		if (wd_sock == -1)
		{
			DEBUG_PRINT(("Could not create socket"));
		}
		puts("Socket created");
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
		server.sin_family = AF_INET;
		server.sin_port = htons( 8635 );
		int optval = 1 ;
		if (setsockopt(wd_sock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int)) == -1) {

			perror("[Error] Socket configration Error") ;
			//	exit(-1) ;
		}
		DEBUG_PRINT(("yas %d: wd_sock=%d func=%s, file=%s\n",__LINE__,wd_sock,__FUNCTION__,__FILE__));
		while(connect(wd_sock , (struct sockaddr *)&server , sizeof(server)) < 0)
		{
			//perror("connect failed. Error");
		}
		wd_connected=true;
		puts("WD Connected\n");
		while( (read_size = recv(wd_sock , client_message , 500 , 0)) > 0 )
		{
			DEBUG_PRINT(("Received: %s\n", client_message));
			if(strncmp("getpid",client_message,strlen("getpid"))==0){
				memset(client_message,0,sizeof(client_message));
				sprintf(client_message,"getpid=%d",cpid);
				write(wd_sock, client_message, strlen(client_message));
			}
		}
		if(read_size == 0)
		{
			puts("WD disconnected");
			//fflush(stdout);
		}
		else if(read_size == -1)
		{
			perror("Server recv failed");
		}
		close(wd_sock);
		wd_sock=0;
	}
}

void *Timer(void*)
{

	try {
		int timeout = 0;
		int timeout_limit=0;
		int well_timeout=0;
		int well_timeout_limit=0;
		bool connected;

		while(1){
			connected=serverController.isConnected();
			//if not connected, try to connect to server
			if (connected==true) {
				if(timeout_info[0].timeout_status==true){
					if(timeout_info[0].timeout_cycle==true)
						timeout=0;
					else{
						timeout++;
						timeout_limit++;
					}

					if(timeout>30){
						timeout=0;
						utility.printBytes(timeout_info[0].timeout_packet,0,timeout_info[0].timeout_len);
						if(timeout_info[0].timeout_len>0)
							serverController.sendCommandReply(timeout_info[0].timeout_packet,timeout_info[0].timeout_len);
					}

					if(timeout_limit>300)
						timeout_info[0].timeout_status=false;

					timeout_info[0].timeout_cycle=false;
				}
				//send welcome message, the server will either return success
				//or will return with the values to be written to the registers

#if 0 // disabled wellcome packet timer
				if(timeout_info[1].timeout_status==true){
					if(timeout_info[1].timeout_cycle==true)
						well_timeout=0;
					else{
						well_timeout++;
						well_timeout_limit++;
					}

					if(well_timeout>5){
						well_timeout=0;
						utility.printBytes(timeout_info[1].timeout_packet,0,timeout_info[1].timeout_len);
						if(timeout_info[1].timeout_len>0)
							serverController.socketWrite(timeout_info[1].timeout_packet,timeout_info[1].timeout_len);
						//serverController.sendCommandReply(timeout_info[0].timeout_packet,timeout_info[0].timeout_len);
					}

					if(well_timeout_limit>15)
						timeout_info[1].timeout_status=false;

					timeout_info[1].timeout_cycle=false;
				}
#endif			
			}
			else{
				timeout_info[0].timeout_status=false;
			}


			sleep(1);
		}
	} catch (exception &e) {
		cout << "Standard exception: " << e.what() << endl;
	} catch (...) {

	}


	//	pthread_exit(NULL);
}


int main(int argc, char **argv) {

	pthread_t timer_thread, wd,rec_thread;
	cout << endl;
	cout << endl;
	cout << "****************************************" << endl;
	cout << "Moxa Client Application " << endl;
	cout << "Version 1.0.5.2 " << endl;
	cout << "****************************************" << endl;
	cout << endl;

	cout << "Moxa client starting....." << endl;

	//Wait for a while to ensure the connections are up before start working
#if DEVICE == 1
#endif

	mkdir("/mnt/sd/gateway_logs", 0755);
	mkdir("/home/Moxa/remote_cache/", 0755);
	mkdir("/home/Moxa/cache_data/", 0755);
	mkdir("/home/logs/", 0755);


	if(utility.getSDCardLoggingState()){
		if (utility.fileExist("/mnt/sd/gateway_logs")) {
			struct statvfs fiData;

			if((statvfs("/mnt/sd/",&fiData)) < 0 ) {
				cout << "\nFailed to stat: /mnt/sd/"  ;
			} else {
				DEBUG_PRINT(( "\nDisk: %s",  "/mnt/sd/"));
				DEBUG_PRINT(("\nBlock size: %ld", fiData.f_bsize));
				DEBUG_PRINT(("\nTotal no blocks: %ld", fiData.f_blocks));
				DEBUG_PRINT(("\nFree blocks: %ld", fiData.f_bfree));
				DEBUG_PRINT(("\navailable space: %ld", fiData.f_bfree*fiData.f_frsize/1024));
				if((fiData.f_bfree*fiData.f_frsize/1024)<1000){
					utility.runBashCom("rm -rf /mnt/sd/gateway_logs/*");
				}
			}

		}
	}
#if 1

	gateway_com.setconnections(modbusController, serverController,utility);
	cpid=getpid();
	DEBUG_PRINT(("yas %d: cpid=%d func=%s, file=%s\n",__LINE__,cpid,__FUNCTION__,__FILE__));
	pthread_create(&wd, NULL, startWatchdog, NULL);
	//startWatchdog();
	modbusController.setUtilityobj(utility);
	serverController.setUtilityobj(utility);
	//utility.initlogger();
	utility.stringlog("Moxa Client Application Version 1.0.5.3  has been started......", 0);
#endif



	//signal(SIGPIPE, SIG_IGN);
	memset(dev_config,0,sizeof(struct s_dev_config)*8);
	utility.loadConfiguration();
	utility.loadDevices(dev_config);



	mac_address = utility.getMACAddress();


	if (mac_address[0] == 0 && mac_address[1] == 0) {
		cout << "NO MAC ADDRESS FOUND.. EXITING " << endl;
		return 0;
	}

	gateway_com.total_devices_count = utility.getDevicesCount();
	gateway_com.setLastDevicesConfig(dev_config);

	serverAddress = utility.getServerAddress();
	serverPort = utility.getServerPort();
	modbusAddress = utility.getModbusAddr();
	Allow_dev_only = atoi(utility.getAllowedDev().c_str());
	cout << endl;
	cout << "Server Address =  " << serverAddress << endl;
	cout << "Server Port    =  " << serverPort << endl;
	cout << "Modbus Port    =  " << modbusAddress << endl;
	cout << "Allowed Devices    =  " << Allow_dev_only << endl ;
	cout << "Modbus Read/Write Delay = "<<utility.getModbusDelay()<<endl;
	cout << "Local Devices  =  " << utility.getDevicesIDs() << endl;
	cout << "logging state =  " << utility.getSDCardLoggingState() << endl;

	utility.stringlog("Server Address =  %s ",0,serverAddress.c_str());
	utility.stringlog("Server Port =  %d ",0,serverPort);

	utility.stringlog("MAC Address    =  %02X:%02X:%02X:%02X:%02X:%02X\n",0, mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4],
			mac_address[5]);
	DEBUG_PRINT(("MAC Address    =  %02X:%02X:%02X:%02X:%02X:%02X\n", mac_address[0],
				mac_address[1], mac_address[2], mac_address[3], mac_address[4],
				mac_address[5]));
	utility.stringlog("Modbus Read Write Delay =  %d ",0,utility.getModbusDelay());




	memset(timeout_info,0,sizeof(timeout_info[10]));
	pthread_create(&timer_thread, NULL, Timer, NULL);
	pthread_create(&rec_thread, NULL, Recevier, NULL);

	do {
		if (work() == 0) {
			break;
		}
		cout << endl << " " << endl;
	} while (true);

	cout << endl << " Done..";

	return 0;
}


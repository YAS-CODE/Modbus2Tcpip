/*
 * Utility.h
 *
 *  Created on: Mar 2, 2015
 *      Author: imtiazahmed
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <fstream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <strings.h>

#include <stdint.h>
using namespace std;

#define DEVICE 1
#define DEBUG  0

const int UPDATE_APPLICATION_DATASIZE= 1024;//72;
const int UPDATA_APPLICATION_FRAMESIZE=50;
const int CRCSIZE =2;

const int CACHE_RECORD_LIMIT = 160;//99;
const int CACHE_SIZE_LIMIT = 10000;

const int MAX_MESSAGE_LENGTH = 2100;//315;
const int MAX_MODBUS_MESSAGE_LENGTH = 315;


const int MAX_NUM_DEV = 11;
const int MAX_REGISTER_DATA_SIZE = 5 + 68;

const int OPT_ZERO_VAL = 0;
const int OPT_DATA_CHANGE_VAL = 1;
const int OPT_DATA_CHANGE_AND_ZERO_VAL = 2;

#define STATUS_REG_LIST_TYPE 0
#define UTC_REG_LIST_TYPE 1


struct s_timeout_info {
                 bool timeout_status;
                 unsigned char timeout_packet[50];
				 int timeout_len;
                 bool timeout_cycle;
              };

struct s_reg_list {
	int list_type;
    int register_address[50];
	int register_type[50];
	int total_registers_count ;
    };

struct s_dev_config{
	int device_id;
	int total_reg_list;
	struct s_reg_list status_reg;
	struct s_reg_list utc_reg;
	time_t last_update_t;
	};
struct s_dev_data{
	int device_id;
	unsigned char reg_data[MAX_REGISTER_DATA_SIZE];
	unsigned char prev_reg_data[MAX_REGISTER_DATA_SIZE];
	bool change_status[MAX_REGISTER_DATA_SIZE];
	unsigned char optim_reg_data[MAX_REGISTER_DATA_SIZE];
	};


struct gateway_logger{
    unsigned loggerfd;
    struct sockaddr_in loggeraddr;	
};

#if DEBUG != 0
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x)
#endif	

class Utility {
public:
	Utility();
	virtual ~Utility();


	/**
	 * Return mac address of the device
	 */
	unsigned char* getMACAddress();


	/**
	 * Load the configuration from config file
	 */
	void loadConfiguration();


	/**
	 * read the server address from file and return
	 */
	string getServerAddress() ;


	/**
	 * returns the server port
	 */
	int getServerPort();

	/**
	 * returns the devices ids in string
	 */
	string getDevicesIDs();

	/**
	 * Load the device numbers to given array
	 */
	bool loadDevices(struct s_dev_config* dev_config);


	/**
	 * Return the count of devices
	 */
	int getDevicesCount();

	/**
	 * Load integer to a given location in char array
	 */
	void loadIntInArray(int value, unsigned char* array, int start_position);


	/**
	 * Zero out the given array
	 */
	bool clearBytes(unsigned char* array, int start, int end);

	/**
	 * Print out the given array
	 */
	void printBytes(unsigned char* array, int start, int end);


	/**
	 * Get Int from byte array
	 */

	string getModbusAddr();
	string getAllowedDev();
	int getIntFromByte(unsigned char* array,
			int start_position);
			
	int findpattren(unsigned char *str, unsigned char *find, int strln,int findln,int str_p);

	void loadLongInArray(unsigned long int longInt, unsigned char* byteArray, int start_position);
	
	long int getLongIntFromByte(unsigned char* byteArray, int start_position);

	unsigned get_file_size (const char * file_name);

	unsigned runBashCom (const char * com);
	
        static bool packet_timeout[10];
        static unsigned char timeout_packet[10][100];
	int getIntFromArray(unsigned char* byteArray, int start_position);
	void loadIntInArrayLE(unsigned int Int, unsigned char* byteArray, int start_position);

#if 1	
	double vm;
	double rss;
	void process_mem_usage(double& vm_usage, double& resident_set);
	int fileExist(const char * pathname);
	int getSDCardLoggingState();
	
#endif
	int readDevList();
	int saveDevConf(uint8_t* pack, int len,struct s_dev_config* dev_config, int total_dev);

	int saveGatewayConnStat(int gateway_stat);

	int initlogger();
	int stringlog(const char *buf, int len,...);//char *buf, int len);

	int getModbusDelay();

	unsigned loggerfd;
	struct sockaddr_in loggeraddr;

	bool have_last_config;


private:

	ifstream m_config_file;

	/**
	 * Server address i.e
	 * datacenter.lodam.com
	 */
	string m_server_address;
	string modbus_address;

	/**
	 * Port number of server. i.e
	 * 5000
	 */
	int m_server_port;

	/**
	 * Indicator if configuration file was read successfully
	 */
	bool m_configuration_read;

	/**
	 * Name of configuration file
	 */
	string m_config_file_name;

	/**
	 * Comma separated device numbers to be read from the configuration file
	 */
	string m_device_numbers;

	/**
	 * Count of devices attached to this moxa box
	 */
	int m_devices_count;

	/**
	 * MAC address of this moxa box
	 */
	unsigned char mac_address[6];

	int log_enabled;

	int modbus_delay;


	string m_allow_device;
	
	int readMacAddr(unsigned char* mac);

	int setMacAddr(unsigned char* mac);

	FILE * log_f;
	long logfilecount;
	

};

#endif /* UTILITY_H_ */

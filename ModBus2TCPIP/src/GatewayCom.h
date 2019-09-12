/*
 * GatewayCom.h
 *
 *
 *  Created on: Feb 14, 2015
 *      Author: imtiaz ahmed
 *
   ============================================
 	Communicates with server with sockets
   ============================================
 *
 *	Commands
 *
 *		Command Code	= 1
 *		Sender			= Gateway/Master
 *		Receiver		= Web Server
 *		Description  	= Asks server to create a master_id and return it to this gateway/master, to use for future communication
 *		Reply			= 4 bytes with unsigned long master id
 *		Date added		= Feb 19, 2015
 *
 *
 *		Command Code	= 3
 *		Sender			= Web Server
 *		Receiver		= Gateway/Master
 *		Description  	= Execute a modbus command
 *		Reply			= Result of the command
 *
 *		Incoming Data	= master_id (4 bytes) + command id (1 byte) + modbus command length (1 byte) + modbus command
 *		Reply Data		= master_id (4 bytes) + command id (1 byte) + modbus reply length (1 byte) + modbus reply
 *
 *		Date added		= Feb 19, 2015
 *
 *
 */


#include "Utility.h"
#include "ServerController.h"
#include "ModbusController.h"
//#include <resolv.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include <sys/statvfs.h>


#ifndef GATEWAYCOMM_H_
#define GATEWAYCOMM_H_

using namespace std;

class GatewayCom {

	public:
		GatewayCom();
		virtual ~GatewayCom();


		/**
		 * Set mac address as gateway ID to make it part of message header for
		 * server communcation
		 *
		 */
		void comParser();

		bool mod_bus_ready;
		
		int cache_file_index;

		int total_devices_count;
		/**
		 * Send command 40 to server
		 */

	bool setconnections(ModbusController &md, ServerController &sd,Utility &uti );
	int comParser(unsigned char* mac_address,struct s_timeout_info* timeout_info,struct s_dev_config* dev_config);
	int parseCommands(unsigned char* buff, int length,unsigned char* mac_address,struct s_timeout_info* timeout_info,struct s_dev_config* dev_config);
	void create_reply(unsigned char* mac,int commandid, bool success,int cmd_ver);
	void create_reply_and_send(unsigned char* mac,int commandid, int success,int cmd_ver);
	int create_remote_file_size_reply_and_send(uint8_t* buffer, int len,unsigned char* mac,int commandid, bool success);
	int request_cmd(uint8_t* buffer, int len,unsigned char* mac,int commandid);
	void send_install_log_reply_and_send(unsigned char* mac,int commandid, bool success);
	void start_script_and_send (unsigned char* mac,int commandid, bool success);
	int remote_update(uint8_t* buffer, int len,struct s_timeout_info * timeout_info);
	int mk_cache_slice(int size,int file_num);
	int getGatewayCashe(unsigned char* mac);
	int disk_space_request(uint8_t* buffer, int len);
	int removeFile(char * name);
	int setGatewayConf(uint8_t* pack, int len,struct s_dev_config* dev_config);
	int setLastDevicesConfig(struct s_dev_config* dev_config);
	
	private:


		/**
		 * Clear the message byte array - used before preparing a command to be sent to server
		 */
		void clearMessage();

		unsigned char packet[2100];
		int m_reply_length;

		
		ModbusController *modcon;
		ServerController *sercon;
		Utility *util;

		unsigned char m_reply_message[MAX_MESSAGE_LENGTH];
};



#endif /* GATEWAYCOMM_H_ */

/*
 * ServerController.h
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

#include <netdb.h>
#include <string>
#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "Utility.h"

//#include <resolv.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/pem.h"
#include "openssl/x509.h"
#include "openssl/x509_vfy.h"


#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>



#ifndef SERVERCONTROLLER_H_
#define SERVERCONTROLLER_H_

using namespace std;


const int COMMAND_INDEX = 6;
const int COMMAND_STATUS_INDEX = 7;
const int MESSAGE_LENGTH_INDEX = 8;
const int CONNECT_MESSAGE_COMMAND = 40;	//Shakehand message by gateway to server.
const int GET_STATUS_OF_ALL_SLAVES_COMMAND = 50; //Reads all sensors of all slaves and returns the values.
const int GET_STATUS_OF_A_SLAVE_COMMAND = 51;
const int UPDATE_SLAVE_IDS_COMMAND = 52;
const int REPORT_SLAVE_STATUS_COMMAND = 53;
const int UPDATE_SYSTEM_TIME_COMMAND = 54;
const int UPDATE_APPLICATION_COMMAND = 100;
#define VERIFY_DEPTH_S 3

class ServerController {

	public:
		ServerController();
		virtual ~ServerController();


		/**
		 * Set mac address as gateway ID to make it part of message header for
		 * server communcation
		 *
		 */
		void setIDForCommunication(unsigned char* mac_address);


		/**
		 * Send command 40 to server
		 */
		void sendWelcomeMessage(struct s_timeout_info* timeout_info);


		/**
		 * Returns the last reply from server
		 */
		unsigned char* getLastReplyFromServer(int& length_of_data);

		/**
		 * Send status of devices to server
		 */
		bool sendDevicesStatus(struct s_dev_config& dev_config,struct s_dev_data& dev_data, int length);

		/**
		 * Connect to server
		 */
		bool connectWithServer(string server_ip, int server_port,int& wd_socket,bool& wds);

		/**
		 * Disconnect from server
		 */
		int disconnect();


		/**
		 * Parse the command and write the values to registers
		 */
		void sendCommandReply(unsigned char* packets,int length);

		bool isConnected();

		void setcurrentver();
		
		bool logDevicesStatus(struct s_dev_data& dev_data, int length,time_t now);

		int logDeviceStatus (unsigned char* data, int length,time_t now);
		
		ssize_t socketWrite(void *sock_buf, size_t sock_count);
		ssize_t socketRead(void *sock_buf, size_t sock_count);
		bool setUtilityobj(Utility &uti ) ;

	private:


		/**
		 * Clear the message byte array - used before preparing a command to be sent to server
		 */
		void clearMessage();


		/**
		 * Send prepared m_communication_message to server.
		 * Before calling this method, the data in m_communication_message needs to be prepared first.
		 *
		 * write_length indicates the number of bytes to send from m_communication_message
		 * read_length indicates the number of bytes to read from incoming reply
		 */
		int sendPreparedBytesToServer(unsigned int write_length, unsigned int read_length);

		void sendModbusCommand(unsigned char *array);

		unsigned long getLongFromByte(unsigned char *array, int start_position);

		void getArrayFromLong(unsigned long number, unsigned char* array, int start_position);

		void updateCommandLength(int length);


		void printBytes(bool out_going, int length);


		//IP of the server to connect to
		string m_server_ip;

		//Port of the server to connect to
		int m_port;

		//set timeout for socket
		struct timeval timeout;



		//socket file descriptor
		int m_socket_file_descriptor;

		//length reporting while writing on sockets
		int m_length;

		//length reporting while reading on sockets
		int m_reading_length;

		int s_sock_status;

		//server address
		struct sockaddr_in m_serv_addr;

		//server descriptor
		struct hostent *m_server;

		string m_server_url;

		//m_keep_listening
		bool m_keep_listening;


		//maximum message length - note this is only for demo where we
		//know the registers, this needs to be changed after demo
		unsigned char m_communication_message[MAX_MESSAGE_LENGTH];

		const static int m_buffer_length = 1000;

		//buffer to read the bytes in
		unsigned char m_buffer[m_buffer_length];

		int insertRecord(FILE * f, unsigned char* data, int length,time_t now);
		
		BIO              *certbio;
		BIO               *outbio;
		X509                *cert;
		X509_NAME       *certname;
		
		SSL_CTX *ctx;
		SSL *ssl;
		void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile);
		void ShowCerts(SSL* ssl);

		pthread_mutex_t cs_mutex;
		time_t last_log_t;
		
		int logTimeOut();
		Utility *s_utility;
		EVP_PKEY *expected_rsa_key;

};



#endif /* SERVERCONTROLLER_H_ */

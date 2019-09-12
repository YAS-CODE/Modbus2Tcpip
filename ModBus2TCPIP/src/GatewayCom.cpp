/*
 * ServerController.cpp
 *
 *  Created on: Feb 14, 2015
 *      Author: imtiazahmed
 */

#include "GatewayCom.h"


GatewayCom::GatewayCom() {
	mod_bus_ready=false;
	cache_file_index=10;

}

GatewayCom::~GatewayCom() {
}

bool GatewayCom::setconnections(ModbusController &md, ServerController &sd,Utility &uti ) {

	modcon=&md;
	sercon=&sd;
	util=&uti;
}
int GatewayCom::comParser(unsigned char* mac_address,struct s_timeout_info* timeout_info,struct s_dev_config* dev_config){
	int buffer_length = 2000;

	int read_length = 0;

	int reply_length;


	unsigned char buffer[buffer_length];

	reply_length=sercon->socketRead(buffer,buffer_length);
	if (reply_length>0){

		reply_length = parseCommands(buffer, reply_length,mac_address,timeout_info,dev_config);
	}
}


int GatewayCom::parseCommands(unsigned char* buff, int length,unsigned char* mac_address,struct s_timeout_info* timeout_info,struct s_dev_config* dev_config){

	//Sample data coming in
	//
	//00 :0C :29 :AF :71 :3B :31 :02 :00 :07 :01 :1F :01 :03 :E9 :00 :01
	//
	//0-5    MAC ID
	//6th    Command number, must be 49

	int device_id = 0;
	int total_registers_count = 0;
	//int total_devices_count = 0;
	int data_pointer = 6;
	int register_address = 0;
	int register_value = 0;
	int reply_state;
	int num_reply;

	int ret;
	unsigned char reply_packet[MAX_MESSAGE_LENGTH];

	uint16_t result[1];
	result[0] = 0;


	//	unsigned char data[2100];
	int packet_length=0;
	int pakcet_index=0;


	memset(m_reply_message,0,MAX_MESSAGE_LENGTH);
	m_reply_length=0;
	packet_length=util->findpattren(buff, mac_address, length,6,pakcet_index);
	if(packet_length<0)
		return 0;
	do{
		data_pointer = 6;
		memset(packet,0,1000);
		packet_length=util->findpattren(buff, mac_address, length,6,pakcet_index+6);
		if(packet_length<0)
			packet_length=length-pakcet_index;
		else{
			packet_length=packet_length-pakcet_index;

		}

		memcpy(packet,buff+pakcet_index,packet_length);

		/*		DEBUG_PRINT(("yas %d: packet index=%d packet end=%d total_length=%d pakcet_len=%d func=%s, file=%s\n",__LINE__,pakcet_index,pakcet_index+packet_length,length,packet_length,__FUNCTION__,__FILE__));
				printBytes(packet,packet_length);
				DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));*/

		int command=packet[data_pointer++];
		int status=packet[data_pointer++];
		switch(command){
			case 40:
				timeout_info[1].timeout_cycle=true;
				DEBUG_PRINT(("yas %d: Connect Message Status=%d func=%s, file=%s\n",__LINE__,status,__FUNCTION__,__FILE__));
				if(status==2){
					reply_state=setGatewayConf(packet, packet_length,dev_config);

					if(reply_state){
						timeout_info[1].timeout_status=false;
						timeout_info[1].timeout_cycle=false;
						mod_bus_ready=true;

						getGatewayCashe(mac_address);
					}
					else
						create_reply_and_send(mac_address,command,reply_state,0);
				}
				break;
			case 99:
				DEBUG_PRINT(("yas %d: Cache Message Status=%d func=%s, file=%s\n",__LINE__,status,__FUNCTION__,__FILE__));
				if(status)
					removeFile((char *)"/home/Moxa/cashe_lines");
				getGatewayCashe(mac_address);
				//create_reply_and_send(mac_address,command,reply_state,1);
				break;
			case 100:
				if(packet[8]==0){
					DEBUG_PRINT(("yas %d: Update Application Status=%d func=%s, file=%s\n",__LINE__,status,__FUNCTION__,__FILE__));
					reply_state=remote_update(packet, packet_length,timeout_info);
					DEBUG_PRINT(("yas %d: reply_state=%d func=%s, file=%s\n",__LINE__,reply_state,__FUNCTION__,__FILE__));
					create_reply_and_send(mac_address,command,reply_state,0);

					timeout_info[0].timeout_cycle=true;
					timeout_info[0].timeout_len=m_reply_length;

					memcpy(timeout_info[0].timeout_packet,m_reply_message,m_reply_length);
				}

				if(packet[8]==1){	// Check File Size
					DEBUG_PRINT(("yas %d: Update Application Status=%d func=%s, file=%s\n",__LINE__,status,__FUNCTION__,__FILE__));
					create_remote_file_size_reply_and_send(packet,packet_length,mac_address,command,reply_state);

				}
				if(packet[8]==2){ // install unzip and isntall remote updates
					//                                        create_remote_file_size_reply(mac_address,command,reply_state);
					DEBUG_PRINT(("yas %d: Update Application Status=%d func=%s, file=%s\n",__LINE__,status,__FUNCTION__,__FILE__));
					start_script_and_send(mac_address,command,reply_state);
				}
				if(packet[8]==3){ // Request Firmware Version Number
					DEBUG_PRINT(("yas %d: Update Application Status=%d func=%s, file=%s\n",__LINE__,status,__FUNCTION__,__FILE__));
					send_install_log_reply_and_send(mac_address,command,reply_state);
				}
				if(packet[8]==4){ // Request Firmware Version Number
					DEBUG_PRINT(("yas %d: Update Application Status=%d func=%s, file=%s\n",__LINE__,status,__FUNCTION__,__FILE__));
					sleep(3);
					//exit(0); 
					system("reboot");
					sleep(3);
				}
				if(packet[8]==5){ // Request Firmware Version Number
					DEBUG_PRINT(("yas %d: Update Application Status=%d func=%s, file=%s\n",__LINE__,status,__FUNCTION__,__FILE__));
					util->runBashCom("rm -rf /home/Moxa/remote_cache/*");
					create_reply_and_send(mac_address,100,1,5);
				}
				break;
			case 6:
				disk_space_request(packet, packet_length);
				break;
			case 7:
				request_cmd(packet,packet_length,mac_address,command);
				break;

			default:
				break;


		}


		int remaining_packet_length = util->getIntFromByte(packet, data_pointer);



		//move the pointer forward as last remaining_packet_length is of two bytes.
		if(command==49){
			memset(reply_packet,0,MAX_MESSAGE_LENGTH);
			memcpy(reply_packet,mac_address,6);
			reply_packet[6]=command;
			ret=modcon->parseCommand(packet,packet_length,reply_packet+7,num_reply);
			DEBUG_PRINT(("yas %d: COmmand Reply=%d func=%s, file=%s\n",__LINE__,ret,__FUNCTION__,__FILE__));
			//create_reply(mac_address,command,ret,0);
			sercon->socketWrite(reply_packet,7+num_reply);
		}
		else if(command==27){
			memset(reply_packet,0,MAX_MESSAGE_LENGTH);
			memcpy(reply_packet,mac_address,6);
			reply_packet[6]=command;
			ret=modcon->readCommand(packet,packet_length,reply_packet+7,num_reply);
			DEBUG_PRINT(("yas %d: COmmand Reply=%d func=%s, file=%s\n",__LINE__,ret,__FUNCTION__,__FILE__));
			//create_reply(mac_address,command,ret,0);
			sercon->socketWrite(reply_packet,7+num_reply);
		}

		pakcet_index=pakcet_index+packet_length;
	}while(pakcet_index<length);
	return 0;
}


void GatewayCom::create_reply(unsigned char* mac,int commandid, bool success,int cmd_ver){
	memcpy(m_reply_message+m_reply_length,mac,6);
	m_reply_message[m_reply_length+6]=commandid;
	if(success)
		m_reply_message[m_reply_length+7]=1;
	else
		m_reply_message[m_reply_length+7]=0;
	m_reply_message[m_reply_length+8]=cmd_ver;
	m_reply_message[m_reply_length+9]=0;
	m_reply_length=m_reply_length+10;
}

void GatewayCom::create_reply_and_send(unsigned char* mac,int commandid, int success,int cmd_ver){
	char reply_packet[MAX_MESSAGE_LENGTH];
	memcpy(reply_packet,mac,6);
	reply_packet[6]=commandid;
	reply_packet[7]=success;
	reply_packet[8]=cmd_ver;
	reply_packet[9]=0;
	sercon->socketWrite(reply_packet,10);
}


int GatewayCom::create_remote_file_size_reply_and_send(uint8_t* buffer, int len,unsigned char* mac,int commandid, bool success){
	uint8_t filename[UPDATE_APPLICATION_DATASIZE];
	char abs_path[UPDATE_APPLICATION_DATASIZE];
	FILE * pFileTXT;
	int buff_length;

	if (buffer[7]!=2){
		DEBUG_PRINT(("Not a request!!!\n"));
		return 0;
	}

	//      unsigned char reply_packet[MAX_MESSAGE_LENGTH];
	memset(m_reply_message,0,MAX_MESSAGE_LENGTH);
	m_reply_length=0;
	//create_reply(mac,commandid,success,1);

	memcpy(m_reply_message+m_reply_length,mac,6);
	m_reply_message[m_reply_length+6]=commandid;
	m_reply_message[m_reply_length+8]=1; // for file size reply
	m_reply_message[m_reply_length+9]=1; // reply packet


	memset(filename,0,UPDATE_APPLICATION_DATASIZE);
	int filenamelen = util->getIntFromArray(buffer, 10);

	memcpy(filename,buffer+12,filenamelen);
	sprintf(abs_path,"/home/Moxa/remote_cache/%s",(char *)filename);
	if(buffer[9]==1){
		m_reply_message[m_reply_length+10]=1;
		struct stat st = {0};

		DEBUG_PRINT(("yas %d: file_name=%s func=%s, file=%s\n",__LINE__,abs_path,__FUNCTION__,__FILE__));
		if (stat((char *)abs_path, &st) == -1) 
			m_reply_message[m_reply_length+7]=0;//return 0

		else
			m_reply_message[m_reply_length+7]=1;//return 1;

	}
	else{
		m_reply_message[m_reply_length+10]=0;
		unsigned long int byte_received=util->get_file_size(abs_path);
		DEBUG_PRINT(("yas %d: file_name=%s byte_received=%ld func=%s, file=%s\n",__LINE__,abs_path,byte_received,__FUNCTION__,__FILE__));
		if(byte_received>0)
			m_reply_message[m_reply_length+7]=1;
		else
			m_reply_message[m_reply_length+7]=0;
		util->loadLongInArray(byte_received, m_reply_message, 11);

	}

	m_reply_length=m_reply_length+20;
	sercon->socketWrite(m_reply_message,m_reply_length);

	DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));
	modcon->printBytes(m_reply_message,m_reply_length);
	DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));



}



int GatewayCom::request_cmd(uint8_t* buffer, int len,unsigned char* mac,int commandid){

#if 0
	char cmd[UPDATE_APPLICATION_DATASIZE];
	char cmd_res[UPDATE_APPLICATION_DATASIZE];
	FILE * pFileTXT;
	int buff_length;

	if (buffer[7]!=2){
		DEBUG_PRINT(("Not a request!!!\n"));
		return 0;
	}

	//      unsigned char reply_packet[MAX_MESSAGE_LENGTH];
	memset(m_reply_message,0,MAX_MESSAGE_LENGTH);
	m_reply_length=0;
	//create_reply(mac,commandid,success,1);

	memcpy(m_reply_message+m_reply_length,mac,6);
	m_reply_message[m_reply_length+6]=commandid;
	m_reply_message[m_reply_length+8]=1; // for file size reply
	m_reply_message[m_reply_length+9]=1; // reply packet
	m_reply_length=10;


	memset(cmd,0,UPDATE_APPLICATION_DATASIZE);
	int cmdlen = util->getIntFromArray(buffer, 10);

	memcpy(cmd, buffer+12,cmdlen);

	printf("\n\n cmd has been commanded as :%s!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",cmd);
	//sprintf(abs_path,"/home/Moxa/remote_cache/%s",(char *)filename);
	FILE* com_fd = popen(cmd, "r"); 

	if (com_fd == NULL) 

	{ 
		DEBUG_PRINT(("FAIL!\n")); 

		return 1; 

	} 

	m_reply_length=12;

	util->stringlog(" Command Request :-%s ",0,cmd);
	while (fgets(cmd_res, UPDATE_APPLICATION_DATASIZE, com_fd) != NULL) 

	{ 

		DEBUG_PRINT(("%s",buffer)); 
		cmdlen= strlen(cmd_res);
		//m_reply_message[10]=cmdlen;
		memcpy(m_reply_message+m_reply_length,cmd_res,cmdlen);
		m_reply_length=m_reply_length+cmdlen;


		if(m_reply_length>500){
			util->loadIntInArray(m_reply_length,m_reply_message, 10);
			//m_reply_length=m_reply_length+strlen(cmd_res);
			sercon->socketWrite(m_reply_message,m_reply_length);
			util->stringlog(" cmd: %s reply :-%s ",0,cmd,m_reply_message);
			m_reply_length=12;
		}

	} 
	util->loadIntInArray(m_reply_length,m_reply_message, 10);
	//m_reply_length=m_reply_length+strlen(cmd_res);
	sercon->socketWrite(m_reply_message,m_reply_length);
	pclose(com_fd); 
	util->stringlog(" cmd: %s reply :-%s ",0,cmd,m_reply_message);
#endif
}


void GatewayCom::send_install_log_reply_and_send(unsigned char* mac,int commandid, bool success){

	memset(m_reply_message,0,MAX_MESSAGE_LENGTH);
	m_reply_length=0;
	char buffer[500];
	long length;
	FILE * f = fopen ("/home/Moxa/install_log", "rb");

	if (f)
	{
		fseek (f, 0, SEEK_END);
		length = ftell (f);
		fseek (f, 0, SEEK_SET);
		fread (buffer, 1, length, f);
		fclose (f);
	}

	if (strlen(buffer)>0)
	{
		// start to process your data / extract strings here...
		create_reply(mac,commandid,1,3);
		m_reply_message[9]=length;
		memcpy(m_reply_message+10,buffer,length);
		util->stringlog(" Remote Update install_log:%s ",length,buffer);
		m_reply_length=m_reply_length+length;
	}
	else{
		create_reply(mac,commandid,0,3);
	}
	sercon->socketWrite(m_reply_message,m_reply_length);

}
void GatewayCom::start_script_and_send (unsigned char* mac,int commandid, bool success){

	//		util->runBashCom("echo \"start installing...\" > /home/Moxa/installing");
	//util->runBashCom("rm -rf /home/Moxa/remote_cache");
	//util->runBashCom("mkdir /home/Moxa/remote_cache");
	util->runBashCom("chmod 755 /home/Moxa/* 2> /home/Moxa/install_log");
	//util->runBashCom("mv /home/Moxa/recv_file /home/Moxa/remote_cache/remote_update.tar.gz 2>> /home/Moxa/install_log");
	//util->runBashCom("tar xvf /home/Moxa/remote_cache/remote_update.tar.gz -C /home/Moxa/remote_cache/ 2>> /home/Moxa/install_log");
	util->runBashCom("sh /home/Moxa/remote_cache/install.sh 2>> /home/Moxa/install_log");
	//		util->runBashCom("rm /home/Moxa/installing");
	//		util->runBashCom("");

	if(util->getSDCardLoggingState()){
		if (util->fileExist("/mnt/sd/gateway_logs")) {
			util->runBashCom("cp /home/Moxa/remote_cache/install.sh /mnt/sd/gateway_logs/last_install.sh");
			util->runBashCom("cp /home/Moxa/install_log /mnt/sd/gateway_logs/last_install_log");
		}
	}
	create_reply_and_send(mac,commandid,1,2);
}

int GatewayCom::remote_update(uint8_t* buffer, int len,struct s_timeout_info * timeout_info) {

	uint8_t data_buf[UPDATE_APPLICATION_DATASIZE];
	uint8_t filename[UPDATE_APPLICATION_DATASIZE];
	char abs_path[UPDATE_APPLICATION_DATASIZE];
	FILE * pFileTXT;
	int buff_length;

	if (buffer[7]!=2){
		DEBUG_PRINT(("Not a request!!!\n"));
		return 0;
	}
	//	int packet_num = util->getIntFromByte(buffer, 8);
	long crc=modcon->crc16(buffer, len-2);
	long packet_crc = util->getIntFromByte(buffer, len-2);
	if(crc!=packet_crc){
		DEBUG_PRINT(("yas %d: Wrong crc patcket_crc=%ld crc=%ld  len=%d func=%s, file=%s\n",__LINE__, packet_crc,crc,len,__FUNCTION__,__FILE__));
		return 0;
	}
	memset(filename,0,UPDATE_APPLICATION_DATASIZE);
	int filenamelen = util->getIntFromArray(buffer, 10);
	//filenamelen=util->getLongIntFromByte(buffer,10);
	memcpy(filename,buffer+12,filenamelen);
	long int packet_num=  util->getLongIntFromByte(buffer,filenamelen+12);
	long int total_packets=  util->getLongIntFromByte(buffer,filenamelen+20);
	sprintf(abs_path,"/home/Moxa/remote_cache/%s",(char *)filename);
	if(buffer[9]==1){
		DEBUG_PRINT(("yas %d: creating  file_name=%s func=%s, file=%s\n",__LINE__,abs_path,__FUNCTION__,__FILE__));
		mkdir((char *)abs_path, 0755);
		return 1;

	}
	buff_length =util->getIntFromArray(buffer, filenamelen+28);
	//	buff_length=len-(UPDATA_APPLICATION_FRAMESIZE+CRCSIZE);

	DEBUG_PRINT(("yas %d: pcket_num=%ld buf_len=%d func=%s, file=%s\n",__LINE__,packet_num,buff_length,__FUNCTION__,__FILE__));
	memcpy(data_buf,buffer+filenamelen+30,buff_length);
	if(packet_num<1){
		pFileTXT = fopen (abs_path,"w");
		util->stringlog(" Remote Update writing on file %s ",0,abs_path);
		(timeout_info[0].timeout_status)=true;
	}
	else
		pFileTXT = fopen (abs_path,"a");// use "a" for append, "w" to overwrite, previous content will be deleted

	int fret=fwrite(data_buf, sizeof(data_buf[0]), buff_length , pFileTXT);
	fclose (pFileTXT); // must close after opening
	if(packet_num>=(total_packets-1)){
		(timeout_info[0].timeout_status)=false;
	}
	if(fret<1){
		DEBUG_PRINT(("yas %d: pcket_num=%ld buf_len=%d Write Error=%d func=%s, file=%s\n",__LINE__,packet_num,buff_length,fret,__FUNCTION__,__FILE__));
		return 3;
	}
	else
		return 1;

}

int GatewayCom::mk_cache_slice(int size,int file_num){

	FILE * cashe;
	FILE * cashe_temp;
	FILE * cashe_lines;
	int count_bytes=0;
	char cached[50];
	char cached_temp[50];

	char cached_lines[]={"/home/Moxa/cashe_lines"};
	int cache_lines=0;
	int count_lines=0;
	bool cache_end=false;
	char ch[3];
	time_t now = time(0);
	memset(cached,0,50);
	memset(cached_temp,0,50);
	sprintf(cached,"/home/Moxa/cache_data/%d",file_num);
	sprintf(cached_temp,"/home/Moxa/cache_data/%d_t",file_num);
	cashe = fopen (cached,"rb");
	if (cashe == NULL)
	{	
		return 1;
	}
	memset(ch,0,3);
	while(count_bytes<size)
	{
		if(feof(cashe)){
			cache_end=true;
			break;
		}
		ch[0] = fgetc(cashe);
		if(ch[0] == ';' && ch[1] == ';' && ch[2] == ';')
		{
			cache_lines++;
		}
		ch[2]=ch[1];
		ch[1]=ch[0];
		count_bytes++;
	}
	rewind(cashe);

	cashe_temp = fopen (cached_temp,"wb");
	if (cashe_temp == NULL)
	{
		DEBUG_PRINT(("Error opening file! data_cashe_temp\n"));
		fclose(cashe);
		return 1;
	}
	cashe_lines = fopen (cached_lines,"wb");
	if (cashe_lines == NULL)
	{
		fclose(cashe);
		fclose(cashe_temp);
		DEBUG_PRINT(("Error opening file! cashe_lines\n"));
		return 1;
	}
	memset(ch,0,3);
	while(!feof(cashe))
	{
		ch[0] = fgetc(cashe);

		if(count_lines<cache_lines)
			fputc(ch[0], cashe_lines);
		else
			fputc(ch[0], cashe_temp);

		if(ch[0] == ';' && ch[1] == ';' && ch[2] == ';')
		{
			count_lines++;
		}
		ch[2]=ch[1];
		ch[1]=ch[0];
	}
	fclose(cashe_temp);
	fclose(cashe);	
	fclose(cashe_lines);	

	if(remove(cached) == 0)
		DEBUG_PRINT(("File %s  deleted.\n", cached));
	else
		fprintf(stderr, "Error deleting the file %s.\n", cached);
	if(!cache_end){
		if(rename(cached_temp, cached) == 0)
		{
			DEBUG_PRINT(("%s has been rename %s.\n", cached_temp, cached));
		}
		else
		{
			fprintf(stderr, "Error renaming %s.\n", cached_temp);
		}
	}
	else{
		if(remove(cached_temp) == 0)
			DEBUG_PRINT(("File %s  deleted.\n", cached_temp));
		else
			fprintf(stderr, "Error deleting the file %s.\n", cached_temp);
	}

	return 0;

}


int GatewayCom::getGatewayCashe(unsigned char* mac){
	int cache_succ=0;
	FILE * cashe_lines;
	int frame_size=10;
	unsigned char cache_replay[1000];
	DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));
	cashe_lines = fopen( "/home/Moxa/cashe_lines", "rb" ) ;
	if(  cashe_lines == NULL ) // checks to see if file dogs exists
	{ 
		DEBUG_PRINT(("File not found\n"));
		do{
			cache_succ=mk_cache_slice(500,cache_file_index);
			if(cache_succ!=0)
				cache_file_index--;
			if(cache_file_index<0)
				break;
		}while(cache_succ!=0);
		if(cache_succ==0){
			cashe_lines = fopen( "/home/Moxa/cashe_lines", "rb" ) ;
			if(  cashe_lines == NULL ) { // seems cache has been trasmited

				return 0;
			}
		}
		else{
			return 0;
		}

	}

	memset(cache_replay,0,1000);
	memcpy(cache_replay,mac,6);
	cache_replay[6]=99;
	cache_replay[7]=2;
	cache_replay[8]=0;


	long length=0;
	//FILE * f = fopen ("cashe_lines", "rb");

	//memset(buffer,0,900);
	if (cashe_lines)
	{
		while(!feof(cashe_lines))
		{
			cache_replay[frame_size+length++] = fgetc(cashe_lines);

		}
		fclose (cashe_lines);
	}
	DEBUG_PRINT(("yas %d: length=%ld func=%s, file=%s\n",__LINE__,length,__FUNCTION__,__FILE__));
	if (length>0)
	{

		cache_replay[9]=length;
		long crc=modcon->crc16(cache_replay+10, length);
		length=frame_size+length+2;
		util->loadIntInArray(crc,cache_replay, length);
		length=length+2;
		sercon->socketWrite(cache_replay,length);

	}

}
int GatewayCom::disk_space_request(uint8_t* buffer, int len) {

	uint8_t data_buf[UPDATE_APPLICATION_DATASIZE];
	char filename[UPDATE_APPLICATION_DATASIZE];
	struct statvfs fiData;
	char abs_path[UPDATE_APPLICATION_DATASIZE];
	uint8_t space_reply[UPDATE_APPLICATION_DATASIZE];
	FILE * pFileTXT;
	int buff_length;
	int packet_index=0;

	if (buffer[7]!=2){
		DEBUG_PRINT(("Not a request!!!\n"));
		return 0;
	}

	//	int packet_num = util->getIntFromByte(buffer, 8);

	long packet_crc = util->getIntFromByte(buffer, 10);
	buffer[10]=0;
	buffer[11]=0;
	long crc=modcon->crc16(buffer, len);
	if(crc!=packet_crc){
		DEBUG_PRINT(("yas %d: Wrong crc patcket_crc=%ld crc=%ld  len=%d func=%s, file=%s\n",__LINE__, packet_crc,crc,len,__FUNCTION__,__FILE__));
		return 0;
	}
	memset(space_reply,0,UPDATE_APPLICATION_DATASIZE);
	memcpy(space_reply,buffer,12);

	space_reply[7]=1;

	int num_paths=buffer[9];

	packet_index=12;
	int reply_index=12;
	for (int i=0; i < num_paths; i++){
		memset(filename,0,UPDATE_APPLICATION_DATASIZE);


		int filenamelen = util->getIntFromArray(buffer, packet_index);

		memcpy(space_reply+reply_index,buffer+packet_index,filenamelen+2);

		packet_index=packet_index+2;
		memcpy(filename,buffer+packet_index,filenamelen);
		packet_index=packet_index+filenamelen;

		reply_index=filenamelen+2+reply_index;

		if((statvfs(filename,&fiData)) < 0 ) {
			space_reply[reply_index]=0;
			cout << "\nFailed to stat:"  << filename;
		} else {
			space_reply[reply_index++]=3;
			util->loadLongInArray(fiData.f_bsize, space_reply, reply_index);
			reply_index=reply_index+8;
			util->loadLongInArray(fiData.f_blocks, space_reply, reply_index);
			reply_index=reply_index+8;
			util->loadLongInArray(fiData.f_bfree, space_reply, reply_index);
			reply_index=reply_index+8;
			DEBUG_PRINT(( "\nDisk: %s",  filename));
			DEBUG_PRINT(("\nBlock size: %ld", fiData.f_bsize));
			DEBUG_PRINT(("\nTotal no blocks: %ld", fiData.f_blocks));
			DEBUG_PRINT(("\nFree blocks: %ld", fiData.f_bfree));
		}

	}

	crc=modcon->crc16(space_reply, reply_index);
	util->loadIntInArray(crc,space_reply, 10);

	DEBUG_PRINT(("\n\n\n\nyas %d:  func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));
	modcon->printBytes(space_reply,reply_index);
	sercon->socketWrite(space_reply,reply_index);

}
int GatewayCom::removeFile(char * name){
	if(remove(name) == 0)
		DEBUG_PRINT(("File %s  deleted.\n", name));
	else
		fprintf(stderr, "Error deleting the file %s.\n", name);
}

int GatewayCom::setGatewayConf(uint8_t* pack, int len,struct s_dev_config* dev_config){

	time_t rawtime;
  struct tm * timeinfo;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
	int list_type=0;
	DEBUG_PRINT(("\n\n\n\nyas %d: length=%d func=%s, file=%s\n",__LINE__,len,__FUNCTION__,__FILE__));
	modcon->printBytes(pack,len);
	DEBUG_PRINT(("yas %d: length=%d func=%s, file=%s\n",__LINE__,len,__FUNCTION__,__FILE__));
	long packet_crc = util->getIntFromByte(pack, len-2);
	long crc=modcon->crc16(pack, len-2);
	if(crc==packet_crc){
		memset(dev_config,0,sizeof(struct s_dev_config)*10);
		int index=9;
		int total_dev=pack[index++];
		total_devices_count=total_dev;
		int config_size=util->getIntFromArray(pack, 10);
		index=index+2;
		DEBUG_PRINT(("yas %d: total_dev=%d pack[index++]=%d config_size=%d func=%s, file=%s\n",__LINE__,total_dev,pack[index],config_size,__FUNCTION__,__FILE__));
		int d;

		for (d=0; d<total_dev; d++){
			dev_config[d].device_id=util->getIntFromArray(pack, index);
			index=index+2;
			dev_config[d].total_reg_list=pack[index++];

			for (int l=0; l<dev_config[d].total_reg_list; l++){
				list_type=pack[index++];
				if(list_type==STATUS_REG_LIST_TYPE){

					dev_config[d].status_reg.list_type=STATUS_REG_LIST_TYPE;
					DEBUG_PRINT(("yas %d: dev_config[%d].device_id=%d func=%s, file=%s\n",__LINE__,d,dev_config[d].device_id,__FUNCTION__,__FILE__));
					dev_config[d].status_reg.total_registers_count=pack[index++];
					for (int r =0; r < dev_config[d].status_reg.total_registers_count ; r++){
						dev_config[d].status_reg.register_address[r]=util->getIntFromArray(pack, index);

						index=index+2;
						dev_config[d].status_reg.register_type[r]=pack[index++];
						DEBUG_PRINT(("yas %d: dev_config[%d].register_ids[%d]=%d type=%d func=%s, file=%s\n",__LINE__,d,r,dev_config[d].status_reg.register_address[r],dev_config[d].status_reg.register_type[r],__FUNCTION__,__FILE__));
					}
				}
				else if(list_type==UTC_REG_LIST_TYPE){

					dev_config[d].utc_reg.list_type=UTC_REG_LIST_TYPE;

					DEBUG_PRINT(("yas %d: dev_config[%d].device_id=%d func=%s, file=%s\n",__LINE__,d,dev_config[d].device_id,__FUNCTION__,__FILE__));
					dev_config[d].utc_reg.total_registers_count=pack[index++];
					for (int r =0; r < dev_config[d].utc_reg.total_registers_count ; r++){
						dev_config[d].utc_reg.register_address[r]=util->getIntFromArray(pack, index);

						index=index+2;
						dev_config[d].utc_reg.register_type[r]=pack[index++];
						DEBUG_PRINT(("yas %d: dev_config[%d].register_ids[%d]=%d type=%d func=%s, file=%s\n",__LINE__,d,r,dev_config[d].utc_reg.register_address[r],dev_config[d].utc_reg.register_type[r],__FUNCTION__,__FILE__));
					}
				}
			}
		}
		util->saveDevConf(pack,len,dev_config, d); 
		return 1;
	}
	else
		DEBUG_PRINT(("yas %d: Connect Message Status incorrent crc=%ld packet_crc=%ld func=%s, file=%s\n",__LINE__,crc,packet_crc,__FUNCTION__,__FILE__));
	return 0;

}


int GatewayCom::setLastDevicesConfig(struct s_dev_config* dev_config){
	FILE * confFile;
	uint8_t conf_buff[1000];
	confFile = fopen( "/home/Moxa/last_dev_conf", "rb" ) ;

	if(  confFile == NULL ) // checks to see if file dogs exists
	{ 
		return 0;
	}

	long length=0;

	if (confFile)
	{
		while(!feof(confFile))
		{
			conf_buff[length++] = fgetc(confFile);

		}
		fclose (confFile);
		setGatewayConf(conf_buff, length-1,dev_config);
	}
	return 1;
}

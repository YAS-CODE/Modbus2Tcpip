/*
 * ServerController.cpp
 *
 *  Created on: Feb 14, 2015
 *      Author: imtiazahmed
 */

#include "ServerController.h"
#include <sstream>
#include <iostream>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>     /* strtoul */
#include <errno.h>
#include <iostream>
#include <ctime>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <fcntl.h>
int verify_callback(int pre_verify_ok, X509_STORE_CTX *ctx) {
	char buf[256];
	X509 *err_cert;
	int err, depth;

	depth = X509_STORE_CTX_get_error_depth(ctx);
	DEBUG_PRINT(( "verify_callback: depth = %d\n",depth));
	if ( depth > VERIFY_DEPTH_S ) {
		DEBUG_PRINT(( "tls_init: verify_callback: cert chain too long ( depth > VERIFY_DEPTH_S)\n"));
		pre_verify_ok=0;
	}

	if( pre_verify_ok ) {
		DEBUG_PRINT(( "tls_init: verify_callback: preverify is good: verify return: %d\n", pre_verify_ok));
		return pre_verify_ok;
	}

	err_cert = X509_STORE_CTX_get_current_cert(ctx);
	err = X509_STORE_CTX_get_error(ctx);    
	X509_NAME_oneline(X509_get_subject_name(err_cert),buf,sizeof buf);

	DEBUG_PRINT(( "tls_init: verify_callback: subject = %s\n", buf));
	DEBUG_PRINT(( "tls_init: verify_callback: verify error:num=%d:%s\n", err, X509_verify_cert_error_string(err)));  
	DEBUG_PRINT(( "tls_init: verify_callback: error code is %d\n", ctx->error));

	switch (ctx->error) {
		case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			X509_NAME_oneline(X509_get_issuer_name(ctx->current_cert),buf,sizeof buf);
			DEBUG_PRINT(( "tls_init: verify_callback: issuer= %s\n",buf));
			break;

		case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
		case X509_V_ERR_CERT_NOT_YET_VALID:
			DEBUG_PRINT(( "tls_init: verify_callback: notBefore\n"));
			break;

		case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
		case X509_V_ERR_CERT_HAS_EXPIRED:
			DEBUG_PRINT(( "tls_init: verify_callback: notAfter\n"));
			break;

		case X509_V_ERR_CERT_SIGNATURE_FAILURE:
		case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			DEBUG_PRINT(( "tls_init: verify_callback: unable to decrypt cert signature\n"));
			break;

		case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
			DEBUG_PRINT(( "tls_init: verify_callback: unable to decode issuer public key\n"));
			break;

		case X509_V_ERR_OUT_OF_MEM:
			DEBUG_PRINT(("tls_init: verify_callback: Out of memory \n"));
			break;

		case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
		case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			DEBUG_PRINT(( "tls_init: verify_callback: Self signed certificate issue\n"));
			break;

		case X509_V_ERR_CERT_CHAIN_TOO_LONG:
			DEBUG_PRINT(( "tls_init: verify_callback: certificate chain too long\n"));
			break;
		case X509_V_ERR_INVALID_CA:
			DEBUG_PRINT(( "tls_init: verify_callback: invalid CA\n"));
			break;
		case X509_V_ERR_PATH_LENGTH_EXCEEDED:
			DEBUG_PRINT(( "tls_init: verify_callback: path length exceeded\n"));
			break;
		case X509_V_ERR_INVALID_PURPOSE:
			DEBUG_PRINT(( "tls_init: verify_callback: invalid purpose\n"));
			break;
		case X509_V_ERR_CERT_UNTRUSTED:
			DEBUG_PRINT(( "tls_init: verify_callback: certificate untrusted\n"));
			break;
		case X509_V_ERR_CERT_REJECTED:
			DEBUG_PRINT(( "tls_init: verify_callback: certificate rejected\n"));
			break;

		default:
			DEBUG_PRINT(( "tls_init: verify_callback: something wrong with the cert ... error code is %d (check x509_vfy.h)\n", ctx->error));
			break;
	}

	DEBUG_PRINT(( "tls_init: verify_callback: verify return:%d\n", pre_verify_ok));
	return(pre_verify_ok);
}


//#include "ModbusController.h"

using namespace std;


void ServerController::LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
	/* set the local certificate from CertFile */
	if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
	{
		ERR_print_errors_fp(stderr);
		abort();
	}
	/* set the private key from KeyFile (may be the same as CertFile) */
	if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
	{
		ERR_print_errors_fp(stderr);
		abort();
	}
	/* verify private key */
	if ( !SSL_CTX_check_private_key(ctx) )
	{
		fprintf(stderr, "Private key does not match the public certificate\n");
		abort();
	}
}
ServerController::ServerController() {
	m_server_ip = "";
	m_socket_file_descriptor = 0;

	s_sock_status=1;

	int i;

	for (i = 0; i < MAX_MESSAGE_LENGTH; i++) {
		m_communication_message[i] = 0;
	}

	for (i = 0; i < m_buffer_length; i++) {
		m_buffer[i] = 0;
	}
	if (pthread_mutex_init(&cs_mutex, NULL) != 0)
	{
		DEBUG_PRINT(("\n mutex init failed\n"));

	}
	certbio = NULL;
	outbio = NULL;
	cert = NULL;
	certname = NULL;
	const SSL_METHOD *method;

	DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));
	OpenSSL_add_all_algorithms();
	ERR_load_BIO_strings();
	ERR_load_crypto_strings();
	SSL_load_error_strings();
	DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));
	/* ---------------------------------------------------------- *
	 * Create the Input/Output BIO's.                             *
	 * ---------------------------------------------------------- */
	certbio = BIO_new(BIO_s_file());
	//outbio  = BIO_new_fp(stdout, BIO_NOCLOSE);

	/* ---------------------------------------------------------- *
	 * initialize SSL library and register algorithms             *
	 * ---------------------------------------------------------- */
	if(SSL_library_init() < 0)
		DEBUG_PRINT(("Could not initialize the OpenSSL library !\n"));


	method = TLSv1_2_client_method();
	DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));
	/* ---------------------------------------------------------- *
	 * Try to create a new SSL context                            *
	 * ---------------------------------------------------------- */
	if ( (ctx = SSL_CTX_new(method)) == NULL)
		DEBUG_PRINT(("Unable to create a new SSL context structure.\n"));

	/* ---------------------------------------------------------- *
	 * Disabling SSLv2 will leave v3 and TSLv1 for negotiation    *
	 * ---------------------------------------------------------- */
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv3);

	//LoadCertificates(ctx,(char *) "/home/Moxa/ca1-cert.pem", (char *)"/home/Moxa/ca1-key.pem"); 


	if(! SSL_CTX_load_verify_locations(ctx, "/home/Moxa/cacert.pem", NULL))
	{
		/* Handle failed load here */
		DEBUG_PRINT(( "Faild load verify locations\n"));

	}
#if 1	
	DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));
	/* ---------------------------------------------------------- *
	 * Create new SSL connection state object                     *
	 * ---------------------------------------------------------- */
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);

	/*
	 * Let the verify_callback catch the verify_depth error so that we get
	 * an appropriate error in the logfile.
	 */
	SSL_CTX_set_verify_depth(ctx,1);
#endif
#if 0
	ssl = SSL_new(ctx);

	const char cert_filestr[] = "/home/Moxa/servercert.pem";
	//expected_rsa_key =  { 0 };
	BIO              *expected_server_certbio = NULL;
	BIO               *outbio = NULL;
	X509                *expected_server_cert = NULL;
	int ret;

	/* ---------------------------------------------------------- *
	 * These function calls initialize openssl for correct work.  *
	 * ---------------------------------------------------------- */
	DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));

	/* ---------------------------------------------------------- *
	 * Create the Input/Output BIO's.                             *
	 * ---------------------------------------------------------- */
	expected_server_certbio = BIO_new(BIO_s_file());
	//outbio  = BIO_new_fp(stdout, BIO_NOCLOSE);

	/* ---------------------------------------------------------- *
	 * Load the certificate from file (PEM).                      *
	 * ---------------------------------------------------------- */
	ret = BIO_read_filename(expected_server_certbio, cert_filestr);
	if (! (expected_server_cert = PEM_read_bio_X509(expected_server_certbio, NULL, 0, NULL))) {
		DEBUG_PRINT(( "Error loading cert into memory\n"));
		exit(-1);
	}

	/* ---------------------------------------------------------- *
	 * Extract the certificate's public key data.                 *
	 * ---------------------------------------------------------- */
	if ((expected_rsa_key = X509_get_pubkey(expected_server_cert)) == NULL)
		DEBUG_PRINT(( "Error getting public key from certificate"));

	/* ---------------------------------------------------------- *
	 * Print the public key information and the key in PEM format *
	 * ---------------------------------------------------------- */
	/* display the key type and size here */
	if (expected_rsa_key) {
		switch (expected_rsa_key->type) {
			case EVP_PKEY_RSA:
				DEBUG_PRINT(("%d bit RSA Key\n\n", EVP_PKEY_bits(expected_rsa_key)));
				break;
			case EVP_PKEY_DSA:
				DEBUG_PRINT(( "%d bit DSA Key\n\n", EVP_PKEY_bits(expected_rsa_key)));
				break;
			default:
				DEBUG_PRINT(("%d bit non-RSA/DSA Key\n\n", EVP_PKEY_bits(expected_rsa_key)));
				break;
		}
	}
#endif	
	SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");


	/* the client doesn't have to send it's certificate */
	//SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, krx_ssl_verify_peer);

	/* enable srtp */
	SSL_CTX_set_tlsext_use_srtp(ctx, "SRTP_AES128_CM_SHA1_80");

	//s_utility->initlogger();
	//time(&last_log_t);

}

ServerController::~ServerController() {
}


bool ServerController::setUtilityobj(Utility &uti ) {
	s_utility=&uti;
}
bool ServerController::connectWithServer(string server_ip, int server_port,int& wd_socket,bool& wds) {

	int code;
	int connect_tries=0;
	int sret;
	//char buf[100];
	X509 *received_cert;
	EVP_PKEY *received_pubkey;

	time_t current;

	BIO	*bp_public = NULL;

	s_sock_status=1;
	m_port = server_port;
	m_server_ip = server_ip; //server_ip;
	time(&last_log_t);


	do{

		time(&current);
		//m_server = gethostbyname(m_server_ip.c_str()); //server_ip
		struct addrinfo *res;
		int ret;
		bzero((char *) &m_serv_addr, sizeof(m_serv_addr));
		res_init();
		//_res.options |= (RES_DEBUG|RES_USEVC);
		ret = getaddrinfo(m_server_ip.c_str(), NULL, NULL, &res);
		if (ret) {
			freeaddrinfo(res);
			DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));
			s_utility->stringlog(" Getaddrinfo failed! ",21);
			//sleep(1);
			if(logTimeOut()){

				return false;
			}
			else
				continue;
		}

		if (res->ai_family == PF_INET)
			memcpy(&m_serv_addr, res->ai_addr, sizeof(struct sockaddr_in));
		else if (res->ai_family == PF_INET6)
			memcpy(&m_serv_addr, res->ai_addr, sizeof(struct sockaddr_in6));
		else{

			freeaddrinfo(res);
			DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));	
			sleep(1);
			if(logTimeOut()){

				return false;
			}
			else
				continue;
		}


		freeaddrinfo(res);

		if(m_socket_file_descriptor)
			close(m_socket_file_descriptor);
		m_socket_file_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

#if DEBUG  == 1
		cout << "Server = " << server_ip << " Port = " << server_port << endl;
#endif
		if(wd_socket>0 && wds)
			write(wd_socket, "ping", strlen("ping"));
		if (m_socket_file_descriptor < 0) {
			cout << ("ERROR opening socket\n") << endl;
			s_utility->stringlog("ERROR opening socket",20);
			sleep(1);
			if(logTimeOut()){
				//connect_tries=0;
				return false;
			}
			else
				continue;
		}


		//bzero((char *) &m_serv_addr, sizeof(m_serv_addr));

		m_serv_addr.sin_family = AF_INET;

		//bcopy((char *) m_server->h_addr, (char *)&m_serv_addr.sin_addr.s_addr, m_server->h_length);

		m_serv_addr.sin_port = htons(m_port);


		do{

			code = connect(m_socket_file_descriptor,
					(struct sockaddr *) &m_serv_addr, sizeof(m_serv_addr));

			if (code < 0) {


				DEBUG_PRINT(("Connect failed %d:   %s \n",errno, strerror(errno)));
				//sprintf(buf,"Connect failed %d:   %s \n",errno, strerror(errno));
				s_utility->stringlog("Connect failed %d:   %s \n",0,errno, strerror(errno));
				sleep(1);
				//			//DEBUG_PRINT("yas %d: wd_socket=%d func=%s, file=%s\n",__LINE__,wd_socket,__FUNCTION__,__FILE__);
				if(wd_socket>0 && wds)
					write(wd_socket, "ping", strlen("ping"));

				if(logTimeOut()){

					return false;
				}

			} 
		}while(code<0);

#if 1
		timeout.tv_sec = 30;
		timeout.tv_usec = 0;

		if (setsockopt (m_socket_file_descriptor, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
					sizeof(timeout)) < 0){
			DEBUG_PRINT(("setsockopt failed\n"));
			return false;
		} else {
#if DEBUG  == 1
			cout << "Connected with server!" << endl;
#endif
			//		return true;
		}

		if (setsockopt (m_socket_file_descriptor, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout)) < 0){
			DEBUG_PRINT(("setsockopt failed\n"));
			return false;
		}
#endif

		int a = 65535;
		if (setsockopt(m_socket_file_descriptor, SOL_SOCKET, SO_RCVBUF, &a, sizeof(int)) == -1) {
			fprintf(stderr, "Error setting socket opts: %s\n", strerror(errno));
		}

		//s_sock_status=0;
		DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));



#if 1	
		///////////////////////////////ssl///////////////////////////////////////////////////

		if (ssl)
		{
			SSL_shutdown (ssl);
			SSL_free (ssl);
		}
		ssl = SSL_new(ctx);
		SSL_set_fd(ssl, m_socket_file_descriptor);
		//SSL_set_mode(ssl, SSL_MODE_RELEASE_BUFFERS);
		DEBUG_PRINT(("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));
		/* ---------------------------------------------------------- *
		 * Try to SSL-connect here, returns 1 for success             *
		 * ---------------------------------------------------------- */
		//SSL_connect(ssl) ;
		sret=SSL_connect(ssl);
		if ( sret != 1 ){
			sret=SSL_get_error(ssl,sret);
			DEBUG_PRINT(( "Error: Could not build a SSL session .with error:%d:%s\n",sret,ERR_error_string(sret,NULL)));
			//sprintf(buf,"Error: Could not build a SSL session .with error:%d:%s\n",sret,ERR_error_string(sret,NULL));
			//s_utility->stringlog(buf,strlen(buf));
			s_utility->stringlog("Error: Could not build a SSL session .with error:%d:%s\n",0,sret,ERR_error_string(sret,NULL));
			close(m_socket_file_descriptor);
			goto ERROR;//continue;
		}
		else
			printf("Successfully enabled SSL/TLS session \n");



#if 0
		received_cert = SSL_get_peer_certificate(ssl);
		received_pubkey = X509_get_pubkey(received_cert);


		//print_certificate(received_cert);
		if (EVP_PKEY_type(received_pubkey->type) != EVP_PKEY_RSA)
			DEBUG_PRINT(("yas %d: Error func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__));//error();

		//EVP_PKEY expected_pubkey = { 0 };
		//EVP_PKEY_assign_RSA(&expected_pubkey, expected_rsa_key);
		sret=EVP_PKEY_cmp(received_pubkey, expected_rsa_key);
		PEM_write_PrivateKey(stdout,received_pubkey,NULL,NULL,0,NULL, NULL);
		//tls_session_verify_fingerprint();
		DEBUG_PRINT(("yas %d: sret=%d func=%s, file=%s\n",__LINE__,sret,__FUNCTION__,__FILE__));
		PEM_write_PrivateKey(stdout,expected_rsa_key,NULL,NULL,0,NULL, NULL);
		if (sret == 1)
			printf("verified Successfully\n");//return true; // identity verified!
		else
			printf("verified Unsuccessfully\n");//return true; // identity verified!return false
		EVP_PKEY_free(received_pubkey);
		X509_free(received_cert);
#endif	

		if (SSL_get_verify_result(ssl) != X509_V_OK)
			sret=-1;

ERROR: 
		if (sret != 1)
			DEBUG_PRINT(( "Error: Could not build a SSL session .with error:%d:%s\n",sret,ERR_error_string(sret,NULL)));

	}while(sret != 1);
	/* ---------------------------------------------------------- *
	 * Get the remote certificate into the X509 structure         *
	 * ---------------------------------------------------------- */

#endif	
	///////////////////////////////end ssl/////////////////////////////////////////////////
	s_sock_status=0;
	last_log_t=0;
	s_utility->stringlog("Successfully enabled SSL/TLS session with Gatewayserver",55);
#if 1
	struct linger		ling;
ling.l_onoff = 1;		/* cause RST to be sent on close() */
	ling.l_linger = 0;
	setsockopt(m_socket_file_descriptor, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));

                        int flags;
                        /* Set socket to non-blocking */

                        if ((flags = fcntl(m_socket_file_descriptor, F_GETFL, 0)) < 0)
                        {
                                /* Handle error */
                                DEBUG_PRINT(("F_GETFL, %d:   %s \n",errno, strerror(errno)));
                        }


                        if (fcntl(m_socket_file_descriptor, F_SETFL, flags | O_NONBLOCK) < 0)
                        {
                                /* Handle error */
                                DEBUG_PRINT(("F_SETFL, failed %d:   %s \n",errno, strerror(errno)));
                        }
#endif

	return true;

}
int ServerController::logTimeOut(){
	
#if 0
	time_t current;
	time(&current);
	double sec = difftime(current,last_log_t);

	/*DEBUG_PRINT(("log_timeout_sec=%.f\n",sec));
	if(sec>/*896*//*540*/10){
		last_log_t=current;
		return 1;
	}
	else
#endif
		return 1;
}

int ServerController::disconnect() {

	s_sock_status=1;
	if (m_socket_file_descriptor > 0) {
#if DEBUG  == 1
		//cout << "Disconnected successfully" << endl;
#endif
		return close(m_socket_file_descriptor);

	} else {
		cout << "Socket was not connected" << endl;
		return -1;
	}

}

void ServerController::setIDForCommunication(unsigned char* mac_address) {

	//copy 6 bytes of mac address at the beginning of the message
	for (int i = 0; i < 6; i++) {
		m_communication_message[i] = mac_address[i];
	}

}

/**
 * Send welcome command
 */
void ServerController::sendWelcomeMessage(struct s_timeout_info* timeout_info) {

	clearMessage();

	m_communication_message[COMMAND_INDEX] = CONNECT_MESSAGE_COMMAND;
	setcurrentver();
	updateCommandLength( 3 + 4 + 4);

	//for this command the read and write length is same (9)
	int bytesToWrite = MESSAGE_LENGTH_INDEX + 2 + 3 + 4 + 4;
	timeout_info[1].timeout_status=true;
	timeout_info[1].timeout_cycle=false;
	timeout_info[1].timeout_len=bytesToWrite;

	memcpy(timeout_info[1].timeout_packet,m_communication_message,bytesToWrite);
	sendPreparedBytesToServer(bytesToWrite, bytesToWrite);

}
ssize_t ServerController::socketWrite(void *sock_buf, size_t sock_count){
	int ret=0;
	pthread_mutex_lock( &cs_mutex );
	//DEBUG_PRINT("yas %d: ret=%d errno=%d func=%s, file=%s\n",__LINE__,ret,errno,__FUNCTION__,__FILE__);
	if(s_sock_status==0)
		ret=SSL_write(ssl, sock_buf,sock_count);
	pthread_mutex_unlock( &cs_mutex );
	//DEBUG_PRINT("yas %d: ret=%d errno=%d func=%s, file=%s\n",__LINE__,ret,errno,__FUNCTION__,__FILE__);
	/* If we've got data to write then try to write it*/              
	if(ret<0)	
		DEBUG_PRINT(("yas %d: ret=%d errno=%d func=%s, file=%s\n",__LINE__,ret,errno,__FUNCTION__,__FILE__));
	//ret=write(m_socket_file_descriptor, sock_buf,sock_count);
	return ret;
}
ssize_t ServerController::socketRead(void *sock_buf, size_t sock_count){

#if DEBUG == 1
	printBytes(true, m_length);
	DEBUG_PRINT(("Total bytes written on socket %d",  m_length));
#endif

#if 0
	if(m_socket_file_descriptor<1)
		return 0;

	m_reading_length=SSL_read(ssl, sock_buf, MAX_MESSAGE_LENGTH);  


	if(m_reading_length == 0 || errno==11)
	{
		DEBUG_PRINT(("yas %d: m_reading_length=%d errno=%d func=%s, file=%s\n",__LINE__,m_reading_length,errno,__FUNCTION__,__FILE__));
		s_sock_status=1;
		close(m_socket_file_descriptor);
		puts("Client disconnected");
	}
	else if(m_reading_length == -1)
	{
		s_sock_status=1;
		close(m_socket_file_descriptor);
		DEBUG_PRINT(("[Error] Socket read failed%d:	%s",errno, strerror(errno)));
	}
	else {

#if DEBUG == 1

		printBytes(false, m_reading_length);

		DEBUG_PRINT(("Total bytes read on socket %d",  m_reading_length));
#endif
	}
#endif
	int m_reading_length = SSL_read(ssl, sock_buf, MAX_MESSAGE_LENGTH);
	if (m_reading_length == 0) {
		long error = ERR_get_error();
		s_sock_status=1;
                close(m_socket_file_descriptor);
                puts("Client disconnected");
		//const char* error_str = ERR_error_string(error, NULL);
		//printf("could not SSL_read (returned 0): %s\n", error_str);
		return m_reading_length;//BREAK;
	} else if (m_reading_length < 0) {
		int ssl_error = SSL_get_error(ssl, m_reading_length);
		if (ssl_error == SSL_ERROR_WANT_WRITE) {
			//printf("SSL_read wants write\n");
			//                        *wants_tcp_write = 1;
			//                        *call_ssl_read_instead_of_write = 1;
			return m_reading_length;//CONTINUE;
		}

		if (ssl_error == SSL_ERROR_WANT_READ) {
			//printf("SSL_read wants read\n");
			// wants_tcp_read is always 1;
			return m_reading_length;//CONTINUE;
		}

		long error = ERR_get_error();
		const char* error_string = ERR_error_string(error, NULL);
		//printf("could not SSL_read (returned -1) %s\n", error_string);
		return m_reading_length;//BREAK;
	} else {
		//printf("read %d bytes\n", m_reading_length);
	}
	return m_reading_length;
}

/**
 * Send prepared m_communication_message to server.
 * Before calling this method, the data in m_communication_message needs to be prepared first.
 *
 * write_length indicates the number of bytes to send from m_communication_message
 * read_length indicates the number of bytes to read from incoming reply
 */
int ServerController::sendPreparedBytesToServer(unsigned int write_length,
		unsigned int read_length) {

	try {

		m_length = socketWrite(m_communication_message,write_length);

		if (m_length < 0) {
			//DEBUG_PRINT("yas %d: m_length=d errno=%d func=%s, file=%s\n",__LINE__,m_length,errno,__FUNCTION__,__FILE__);
			cout << "ERROR writing to socket" << endl;
			m_length = 0;
			s_sock_status=1;
			close(m_socket_file_descriptor);

		} else {
#if 0
#if DEBUG == 1
			printBytes(true, m_length);
			cout << "Total bytes written on socket" << m_length << endl;
#endif
			//Bytes written on socket, now going to read the server reply
			m_reading_length = read(m_socket_file_descriptor, m_buffer,
					MAX_MESSAGE_LENGTH);

			if(m_reading_length == 0 || errno==11)
			{
				s_sock_status=1;
				close(m_socket_file_descriptor);
				puts("Client disconnected");
			}
			else if(m_reading_length == -1)
			{
				s_sock_status=1;
				close(m_socket_file_descriptor);
				DEBUG_PRINT(("[Error] Socket read failed%d:	%s\n",errno, strerror(errno)));
			}
			else {

#if DEBUG == 1

				printBytes(false, m_reading_length);
				cout << "Total bytes read from Server" << m_reading_length << endl;
#endif
			}
#endif
		}

	} catch (...) {
		m_length = 0;
	}

	return m_length;

}

unsigned long ServerController::getLongFromByte(unsigned char* array,
		int start_position) {

	return ((array[start_position + 3] << 24)
			| (array[start_position + 2] << 16)
			| (array[start_position + 1] << 8) | (array[start_position]));

}

bool ServerController::logDevicesStatus(struct s_dev_data& dev_data, int length,time_t now) {

	//clearMessage();
	int bytesToWrite;
	int byteToRead;

	memset(m_communication_message,6,6);

	//clearMessage();
	if(length>0){
		m_communication_message[COMMAND_INDEX] = GET_STATUS_OF_ALL_SLAVES_COMMAND;
		updateCommandLength(length);

		//for this command write length comes as a parameter
		bytesToWrite = MESSAGE_LENGTH_INDEX + 1 + length;
		byteToRead = 9;

		for (int i = 0; i < MAX_MODBUS_MESSAGE_LENGTH; i++) {
			m_communication_message[MESSAGE_LENGTH_INDEX + i] = dev_data.optim_reg_data[i];
		}	
	}
	else{
		m_communication_message[COMMAND_INDEX] = REPORT_SLAVE_STATUS_COMMAND;
		updateCommandLength(4);

		//for this command write length comes as a parameter
		bytesToWrite = MESSAGE_LENGTH_INDEX + 1 + 6;
		byteToRead = 9;
		m_communication_message[10]=1;
		m_communication_message[11]=dev_data.optim_reg_data[3];

	}
	DEBUG_PRINT(("yas %d: length=%d func=%s, file=%s\n",__LINE__,length,__FUNCTION__,__FILE__));
	printBytes(true,bytesToWrite);
	DEBUG_PRINT(("yas %d: length=%d func=%s, file=%s\n",__LINE__,length,__FUNCTION__,__FILE__));
	logDeviceStatus(m_communication_message+6, bytesToWrite-6,now);

	//        return sendPreparedBytesToServer(bytesToWrite, byteToRead) > 0;
}
bool ServerController::sendDevicesStatus(struct s_dev_config& dev_config,/*unsigned char* data*/struct s_dev_data& dev_data, int length) {

	//clearMessage();
	int bytesToWrite;
	int byteToRead;

	if(length>0){
		m_communication_message[COMMAND_INDEX] = GET_STATUS_OF_ALL_SLAVES_COMMAND;
		updateCommandLength(length);

		//for this command write length comes as a parameter
		bytesToWrite = MESSAGE_LENGTH_INDEX + 1 + length;
		byteToRead = 9;

		for (int i = 0; i < MAX_MODBUS_MESSAGE_LENGTH; i++) {
			m_communication_message[MESSAGE_LENGTH_INDEX + i] = dev_data.optim_reg_data[i];// data[i];
		}
	}
	else{
		m_communication_message[COMMAND_INDEX] = REPORT_SLAVE_STATUS_COMMAND;
		updateCommandLength(4);

		//for this command write length comes as a parameter
		bytesToWrite = MESSAGE_LENGTH_INDEX + 1 + 6;
		byteToRead = 9;
		m_communication_message[10]=1;
		m_communication_message[11]=dev_config.device_id;//dev_data.reg_data[3];//data[3];

	}

	return sendPreparedBytesToServer(bytesToWrite, byteToRead) > 0;

}

void ServerController::clearMessage() {

	for (int i = COMMAND_STATUS_INDEX; i < MAX_MESSAGE_LENGTH; i++) {
		m_communication_message[i] = 0;
	}

}

bool ServerController::isConnected(){
	if(s_sock_status==0)
		return true;
	else
		return false;

}

void ServerController::setcurrentver() {


	char buffer[50];
	long length;
	FILE * f = fopen ("/home/Moxa/version", "rb");
	memset(buffer,0,50);

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
		int index=0,len=0;
		char num[10];
		//index=m_utility.findpattren(buffer, '.', length,1,0);	
		while(buffer[index]!='.' && index < length )
			index++;
		strncpy(num,buffer,index);
		m_communication_message[10]=atoi(num);

		index++;
		len=index;

		//index=m_utility.findpattren(buffer, '.', length,1,index);
		while(buffer[index]!='.' && index < length )
			index++;
		strncpy(num,buffer+len,index-len);
		m_communication_message[11]=atoi(num);


		index++;
		len=index;

		//index=m_utility.findpattren(buffer, '.', length,1,index);
		while(buffer[index]!='.' && index < length )
			index++;
		strncpy(num,buffer+len,index-len);
		m_communication_message[12]=atoi(num);

	}
	else{

		m_communication_message[10]=0;
		m_communication_message[11]=0;
		m_communication_message[12]=0;

	}

	m_communication_message[13]=1;
	m_communication_message[14]=0;
	m_communication_message[15]=4;
	m_communication_message[16]=0;

	m_communication_message[17]=1;
	m_communication_message[18]=0;
	m_communication_message[19]=5;
	m_communication_message[20]=4;


}
void ServerController::updateCommandLength(int length) {

	//Set the status to 2 to indicate that this message is request.
	m_communication_message[COMMAND_STATUS_INDEX] = 2;

	for (int i = MESSAGE_LENGTH_INDEX; i < MESSAGE_LENGTH_INDEX + 2; i++)
		m_communication_message[i] = (length >> (i * 8));

}

void ServerController::getArrayFromLong(unsigned long number,
		unsigned char* array, int start_position) {

	for (int i = start_position; i < 4; i++)
		array[i] = (number >> (i * 8));

}

unsigned char* ServerController::getLastReplyFromServer(int& length_of_data) {

	length_of_data = m_reading_length;
	return m_buffer;

}

void ServerController::printBytes(bool out_going, int length) {

	//#if DEBUG == 1
	DEBUG_PRINT(("\nPrinting bytes = "));
	int i;

	for (i = 0; i < length; i++) {
		if (i > 0) {
			DEBUG_PRINT((":"));
		}
		if(out_going)
		{
			DEBUG_PRINT(("%02X", m_communication_message[i]));
		}
		else
		{
			DEBUG_PRINT(("%02X", m_buffer[i]));
		}
	}
	DEBUG_PRINT(("\n"));
	//#endif

}

void ServerController::sendCommandReply(unsigned char* packets,int length) {

	/*        DEBUG_PRINT("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__);
		  printBytes(packets,length);
		  DEBUG_PRINT("yas %d: func=%s, file=%s\n",__LINE__,__FUNCTION__,__FILE__);*/
	int n=socketWrite(packets,length);
	if (n < 0){ 
		DEBUG_PRINT(("ERROR writing to socket"));
		s_sock_status=1;
		close(m_socket_file_descriptor);
	}

	//TODO implement the code to reply back the result of writing
	//to registers
}
int ServerController::insertRecord(FILE * f, unsigned char* data, int length,time_t now){
	//time_t now = time(0);
	char term[3] ;//= ':';
	char now_str[10];

	sprintf(now_str,"%ld",now);

	fwrite(now_str, 1, 10, f);
	//Some calculations to fill a[]
	DEBUG_PRINT(("yas %d: now=%ld now_str=%s func=%s, file=%s\n",__LINE__,now,now_str,__FUNCTION__,__FILE__));
	for (int i =0;i <3 ; i++)
		term[i] = ':';
	fwrite(&term, 1, 3, f);

	fwrite(data, 1, length, f);
	usleep(35 * 500);

	for (int i =0;i <3 ; i++)
		term[i] = ';';
	fwrite(&term, 1, 3, f);
	usleep(35 * 500);



}

int ServerController::logDeviceStatus (unsigned char* data, int length,time_t now)
{

	FILE * cashe;
	FILE * cashe_temp;
	char cached[]={"/home/Moxa/cache_data/0"};
	int lines=0;
	char ch[3];

	cashe = fopen (cached,"rb");
	if (cashe == NULL)
	{

		cashe = fopen (cached,"wb");
		if (cashe == NULL){
			DEBUG_PRINT(("Error opening file! data_cashe \n"));
			return -1;
		}

		//fprintf(cashe,"%ld:%s;",now,data);
		insertRecord(cashe,data,length,now);
		fclose(cashe);
		return 0;

	}
	memset(ch,0,3);
	while(!feof(cashe))
	{
		ch[0] = fgetc(cashe);
		if(ch[0] == ';' && ch[1] == ';' && ch[2] == ';')
		{
			lines++;
			//	DEBUG_PRINT(("yas %d: current line num=%d func=%s, file=%s\n",__LINE__,lines,__FUNCTION__,__FILE__));
		}

		ch[2]=ch[1];
		ch[1]=ch[0];
	}
	DEBUG_PRINT(("Number of Records=%d Added \n",lines));
	fclose(cashe);
	cashe = fopen (cached,"ab");
	if (cashe == NULL){
		cashe = fopen (cached,"wb");
		if (cashe == NULL)
			DEBUG_PRINT(("Error opening file! data_cashe \n"));
		return -1;
	}
	//fprintf(cashe,  "%ld:%s\n",now,data);
	insertRecord(cashe,data,length,now);
	fclose(cashe);
	unsigned long int cache_size=s_utility->get_file_size("/home/Moxa/cache_data/0");
	if(lines>CACHE_RECORD_LIMIT  || cache_size>CACHE_SIZE_LIMIT){

		char cache_part_num[50];
		char prev_cache_part_n[50];
		for(int i=11;i>0;i--){
			sprintf(cache_part_num,"/home/Moxa/cache_data/%d",i);
			sprintf(prev_cache_part_n,"/home/Moxa/cache_data/%d",i-1);
			if(rename(prev_cache_part_n, cache_part_num) == 0)
			{
				DEBUG_PRINT(("%s has been rename %s.\n", prev_cache_part_n, cache_part_num));
			}
		}
		if(remove("/home/Moxa/cache_data/11") == 0)
			DEBUG_PRINT(("File /home/Moxa/cache_data/11  deleted.\n"));

	}

	return 0;
}

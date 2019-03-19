/**
* Mobius Software LTD
* Copyright 2015-2018, Mobius Software LTD
*
* This is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 2.1 of
* the License, or (at your option) any later version.
*
* This software is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, write to the Free
* Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA, or see the FSF site: http://www.fsf.org.
*/

#include "../tcp_listener.h"
#include "../account.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <netdb.h>

#define BUFFER_SIZE          (1<<16)

static char file_name_template [] = "/tmp/c_client_temp.XXXXXX";
static char * filename = NULL;
static const char * cert_password = NULL;

static SSL *ssl;
int fd;
static pthread_t worker;
static char in_buf[BUFFER_SIZE];
static struct TcpListener * listener;
static int interrupted = 0;

static int passwd_cb(char *buf,int size, int rwflag,void *userdata)
{

  int password_length;

  password_length = strlen(cert_password);

  if ((password_length + 1) > size) {
    printf("Password specified by environment variable is too big\n");
    return 0;
  }

  strcpy(buf,cert_password);
  return password_length;

}

static void * update_dtls(void *threadid)
{
	int len = 0;
	while (interrupted == 0) {
		len = SSL_read(ssl, in_buf, sizeof(in_buf));
		if(len > 0) {
			listener->prd_pt(in_buf, len);
		}
	}
	cert_password = NULL;
	close(fd);
	printf("DTLS connection closed.\n");
	return 0;
}

static char* hostname_to_ip(char * hostname)
{
	struct hostent *he;
	struct in_addr **addr_list;
	int i;

	if ( (he = gethostbyname( hostname ) ) == NULL)
	{
		// get the host info
		printf("Cannot get ip from hostname : %s\n", hostname);
		return NULL;
	}

	addr_list = (struct in_addr **) he->h_addr_list;

	for(i = 0; addr_list[i] != NULL; i++)
	{
		//Return the first one;
		return inet_ntoa(*addr_list[i]);
	}

	return NULL;
}

static int start_client(char *remote_address, int port, const char * cert) {

	char * remote_ip = hostname_to_ip(remote_address);
	if(remote_ip == NULL)
		return -1;

	union {
		struct sockaddr_storage ss;
		struct sockaddr_in s4;
	} remote_addr;

	SSL_CTX *ctx;
	BIO *bio;
	struct timeval timeout;
	memset((void *) &remote_addr, 0, sizeof(struct sockaddr_storage));
	inet_pton(AF_INET, remote_ip, &remote_addr.s4.sin_addr);
	remote_addr.s4.sin_family = AF_INET;
	remote_addr.s4.sin_port = htons(port);

	fd = socket(remote_addr.ss.ss_family, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket");
		exit(-1);
	}

	OpenSSL_add_ssl_algorithms();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(DTLSv1_2_client_method());
	if(cert != NULL) {

		filename = malloc((strlen(file_name_template) + 1) * sizeof(char));
		strcpy(filename, file_name_template);
		int file_d = mkstemp(filename);
		if (file_d == -1) {
			printf("cannot open file!!!\n");
			return -1;
		}
		int size_data = write(file_d, cert, strlen(cert));
		if(size_data <= 0) {
			printf("Error : Cannot write certificate in file!!!\n");
			close(file_d);
			unlink(filename);
			return -1;
		}
		close(file_d);

		if (!SSL_CTX_use_certificate_file(ctx, filename, SSL_FILETYPE_PEM))
			printf("\nERROR: no certificate found!");

		if(cert_password != NULL) {
			SSL_CTX_set_default_passwd_cb(ctx, passwd_cb);
		}

		if (!SSL_CTX_use_PrivateKey_file(ctx, filename, SSL_FILETYPE_PEM))
		{
			printf("\nERROR: no private key found!");
			return -1;
		}



		if (!SSL_CTX_check_private_key (ctx))
		{
			printf("\nERROR: invalid private key!");
			return -1;
		}
	}

	SSL_CTX_set_verify_depth (ctx, 2);
	SSL_CTX_set_read_ahead(ctx, 1);

	ssl = SSL_new(ctx);

	/* Create BIO, connect and set to already connected */
	bio = BIO_new_dgram(fd, BIO_CLOSE);

	connect(fd, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr_in));

	BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, &remote_addr.ss);

	SSL_set_bio(ssl, bio, bio);
	int connection_result = SSL_connect(ssl);
	if (connection_result < 0) {
		return -1;
	}

	/* Set and activate timeouts */
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_RECV_TIMEOUT, 0, &timeout);

	//start listener

	long t = 111;
	int rc = pthread_create(&worker, NULL, update_dtls, (void *)t);
	if (rc){
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		return -1;
	 }
	if(filename!= NULL)
		unlink(filename);

	return 0;
}

int init_dtls(const char * host, int port, struct TcpListener * _listener, const char * cert, const char * _cert_password) {

	listener = _listener;
	cert_password = _cert_password;
	interrupted = 0;
	return start_client((char*)host, port, cert);

}

void dtls_fire(char * buf, int length) {

	SSL_write(ssl, buf, length);

}

void stop_dtls_net_service() {
	interrupted = 1;
}


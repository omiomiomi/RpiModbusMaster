/*
 * Copyright © 2008-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/*
 *	Includes
 */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <modbus.h>

/*
 *	Defines
 */
#define HOST "www.cc.puv.fi"
#define POST_URL "/~e1201336/JSON/print.php"
#define PORT 80
#define SEND_RQ(MSG) {fprintf(stderr,"%s",MSG); send(sockfd,MSG,strlen(MSG),0);}
#define LOOP             1
#define SERVER_ID        1
#define FOREVER		while(1)


/* 
 *	Prototypes
 */
uint8_t http_post(unsigned char *host, unsigned char *url, unsigned char *message);
uint8_t get_len(void);
uint8_t mb_send(uint8_t len);

uint8_t main(
	void
)	{	
	uint8_t	len = 0;
	unsigned char *host = HOST;
	unsigned char *url = POST_URL;
	unsigned char *message = "{\"time\":237}";

	FOREVER	{	
		len = get_len();
		mb_send(len);	
		http_post(host , url , message);
	}
}

uint8_t mb_send(
	uint8_t len	
)	{
	modbus_t *ctx;
	uint8_t rc;
	uint8_t nb_fail;
	uint8_t addr = 0;

	/* RTU */
	ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);
	modbus_set_slave(ctx, SERVER_ID);

	if (modbus_connect(ctx) == -1) {
		fprintf(stderr, "Connection failed: %s\n",
		modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
	
	nb_fail = 0;

	/* SINGLE REGISTER */
	rc = modbus_write_register(ctx, addr, len);
	if (rc != 1) {
		 printf("ERROR modbus_write_register (%d)\n", rc);
		 printf("Address = %d, value = %d (0x%X)\n",
				addr, len, len);
		 nb_fail++;
	}

	 /* Close the connection */
	 modbus_close(ctx);
	 modbus_free(ctx);

	if (nb_fail)
		return 0;
	else
		return 1;
}

uint8_t get_len(
	void
)	{
	char *len;
	len = getenv("QUERY_STRING");
//	printf("%s/r/n",len);
	return atoi(len);	
}

uint8_t http_post(
	unsigned char *host, unsigned char *url, unsigned char *message
)  {

    unsigned char recbuf[255];
    int sockfd, portno=PORT, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char domain,sender,receiver,request,address,check_sum,data;
    char c1[1];
    int l,line_length,count=0;
    int loop = 1;
    int bHeader = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    error("ERROR opening socket");
    server = gethostbyname(HOST);

    if (server == NULL) {
    error("ERROR, no such host\n");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
    (char *)&serv_addr.sin_addr.s_addr,
    server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    error("ERROR connecting");

    SEND_RQ("POST ");
    SEND_RQ(url);

    SEND_RQ(" HTTP/1.0\r\n");

    char content_header[100];
    sprintf(content_header,"Content-Length: %d\r\n",strlen(message));
    SEND_RQ(content_header);

    SEND_RQ("Host: ");
    SEND_RQ(host);
    SEND_RQ("\r\n");

    SEND_RQ("Content-Type: application/x-www-form-urlencoded\r\n");
    SEND_RQ("\r\n");

    SEND_RQ(message);
    SEND_RQ("\r\n");

    bzero(recbuf,255);
	line_length=0;

    l = recv(sockfd, c1, 1, 0);

    while(l!=0) {

    fprintf(stderr,"%c",c1[0]);
    *(recbuf+line_length)=c1[0];
    line_length++;
    l = recv(sockfd, c1, 1, 0);
    }

    bzero(recbuf,255);
    bHeader = 1;

    if(bHeader) {
    fprintf(stderr,"####BODY####\n") ;

    l = recv(sockfd, c1, 1, 0);
    while(l!=0) {

    fprintf(stderr,"%c",c1[0]);
    *(recbuf+line_length)=c1[0];
    line_length++;
    l = recv(sockfd, c1, 1, 0);
    }

    } else {
    return -102;
    }

    close(sockfd);

    return 1;
}


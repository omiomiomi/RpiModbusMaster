/*
 *	Include
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>

/*
 *	Defines
 */
#define PORT 80
#define HOST "www.cc.puv.fi"
#define POST_URL "/~e1201336/JSON/print.php"
#define SEND_RQ(MSG) {fprintf(stderr,"%s",MSG); send(sockfd,MSG,strlen(MSG),0);}

/*
 *	Prototypes
 */
int main(void);
int http_send(unsigned char *host, unsigned char *url, unsigned char *message);

/*
 *	MAIN
 */
int main(
	void
)	{
	unsigned char *host = HOST;
	unsigned char *url = POST_URL;
	unsigned char *message = "{\"time\":237}";
	

	if(http_send(host, url, message) != 1) {
		error("HTTP SEND NOT SUCCESFUL");
	}else{
		printf("SUCCESS!\r\n");
	}
	return 0;
}

int http_send(unsigned char *host, unsigned char *url, unsigned char *message)	{
	
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

	/* 
	 *	After this point change the variable message to rec_buf
	 */	
	

	fprintf(stderr,"########### Reveiving #############\n");

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

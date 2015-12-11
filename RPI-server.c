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
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

/*
 *	Defines
 */
#define HOST "www.cc.puv.fi"
#define LOCALHOST "localhost"
#define PAGE "/"
#define POST_URL "/~e1201336/JSON/print.php"
#define PORT 80
#define USERAGENT "HTMLGET 1.0"
#define SEND_RQ(MSG) {fprintf(stderr,"%s",MSG); send(sockfd,MSG,strlen(MSG),0);}

/* 
 *	Prototypes
 */
uint8_t create_tcp_socket();
uint8_t http_post
uint8_t http_get();
char *get_ip(char *host);
char *build_get_query(char *host, char *page);

uint8_t main(
	void
)	{
	unsigned char *host = HOST;
	unsigned char *url = POST_URL;
	unsigned char *message = "{\"time\":237}";

}

uint8_t create_tcp_socket(
	
)	{
	uint8_t sock;
	if((sock == socket(AF_INET, SOCK_STREAM , IPPROTO_TCP)) < 0)	{
		perror("Can't create TCP socket");	
		exit(1);
	}
	return sock;
}

char *get_ip(
	char *host
)	{
  struct hostent *hent;
  uint8_t iplen = 15;
  char *ip = (char *)malloc(iplen+1);
  memset(ip, 0, iplen+1);
  if((hent = gethostbyname(host)) == NULL)
  {
    herror("Can't get IP");
    exit(1);
  }
  if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
  {
    perror("Can't resolve host");
    exit(1);
  }
  return ip;
}

#ifdef HTTP_GET

uint8_t http_get(

)	{
  struct sockaddr_in *remote;
  uint8_t sock;
  uint8_t tmpres;
  char *ip;
  char *get;
  char buf[BUFSIZ+1];
  char *host;
  char *page;
 
  host = LOCALHOST;
  page = PAGE;

  sock = create_tcp_socket();
  ip = get_ip(host);
  remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
  remote->sin_family = AF_INET;
  tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));
  
  if( tmpres < 0)	{
    perror("Can't set remote->sin_addr.s_addr");
    exit(1);
  }else if(tmpres == 0)	{
    fprintf(stderr, "%s is not a valid IP address\n", ip);
    exit(1);
  }

  remote->sin_port = htons(PORT);
 
  if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0){
    perror("Could not connect");
    exit(1);
  }

  get = build_get_query(host, page);
  //Send the query to the server
  uint8_t sent = 0;
  while(sent < strlen(get))	{
    tmpres = send(sock, get+sent, strlen(get)-sent, 0);
    if(tmpres == -1){
      perror("Can't send query");
      exit(1);
    }
    sent += tmpres;
  }

  //now it is time to receive the page
  memset(buf, 0, sizeof(buf));
  uint8_t htmlstart = 0;
  char * htmlcontent;
  while((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0){
    if(htmlstart == 0)
    {
      /* Under certain conditions this will not work.
      * If the \r\n\r\n part is splitted into two messages
      * it will fail to detect the beginning of HTML content
      */
      htmlcontent = strstr(buf, "\r\n\r\n");
      if(htmlcontent != NULL){
        htmlstart = 1;
        htmlcontent += 4;
      }
    }else{
      htmlcontent = buf;
    }
    if(htmlstart){
      fprintf(stdout, htmlcontent);
    } 
 
    memset(buf, 0, tmpres);
  }
  if(tmpres < 0)
  {
    perror("Error receiving data");
  }
  free(get);
  free(remote);
  free(ip);
  close(sock);
  return 0;
}

char *build_get_query(
	char *host, char *page
)	{
	char *query;
	char *getpage = page;
	char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
	if(getpage[0] == '/'){ getpage = getpage + 1;
	fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", page, getpage);
	}
	// -5 is to consider the %s %s %s in tpl and the ending \0
	query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
	sprintf(query, tpl, getpage, host, USERAGENT);
	return query;
}
#endif
uint8_t http_post(unsigned char *host, unsigned char *url, unsigned char *message)  {

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


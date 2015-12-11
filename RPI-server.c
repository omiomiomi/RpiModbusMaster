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
#define HOST "localhost"
#define PAGE "/"
#define PORT 80
#define USERAGENT "HTMLGET 1.0"

/* 
 *	Prototypes
 */
uint8_t create_tcp_socket();
uint8_t com_via_socket();
char *get_ip(char *host);
char *build_get_query(char *host, char *page);

uint8_t main(
	void
)	{
	com_via_socket();	
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

uint8_t com_via_socket(

)	{
  struct sockaddr_in *remote;
  uint8_t sock;
  uint8_t tmpres;
  char *ip;
  char *get;
  char buf[BUFSIZ+1];
  char *host;
  char *page;
 
  host = HOST;
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
  if(getpage[0] == '/'){
    getpage = getpage + 1;
    fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", page, getpage);
  }
  // -5 is to consider the %s %s %s in tpl and the ending \0
  query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
  sprintf(query, tpl, getpage, host, USERAGENT);
  return query;
}

/*
 * port:10000
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>

void show(char **lis){
	printf("# show()\n");
	char **p = lis;
	while(*p){
		printf("%s", *p);
		p++;
	}
	printf("# end of show()\n");
}

int get_req(char *src, char **req){
	printf("# get_req()\n");
	char **q;
	q = req;
	q[10] = 0;
	int fd;
	int rd;
	char buf[BUFSIZ];
	fd = open(src, O_RDONLY);
	if(fd < 0){
		perror("open"); exit(1);
	}
	while( (rd = read(fd, buf, BUFSIZ-1)) > 0){
		buf[strlen(buf)] = '\0';
		*q = (char *)malloc(sizeof(char)*BUFSIZ);
		strcpy(*q, buf);
		q++;
//		printf("%s", *(q-1));
	}
	*q = 0;
	close(fd);
	return 0;
}

int main(){
	int sockfd, nbytes;
	char buf[BUFSIZ];
	char *mesg = "Hello, World";
	
	char **req = (char **)malloc(sizeof(char *)*11);
	char *filename = "http_req.txt";
	get_req(filename, req);
	show(req);

	struct sockaddr_in servaddr;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket"); exit(1);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(10000);
	// IP address of localhost is 127.0.0.1
	if(inet_pton(AF_INET, "127.0.0.1:10000", &servaddr.sin_addr) < 0){
		perror("inet_pton"); exit(1);
	}
	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("connect"); exit(1);
	}
	// nbytes wont check this code (sometimes cant send all of message)
	// printf("%s",*req);
	nbytes = write(sockfd, *req, strlen(*req)+1);
	nbytes = read(sockfd, buf, sizeof(buf)-1);
	buf[nbytes] = '\0';		// add null escape at the end of string
	puts(buf);
	close(sockfd);
	return 0;
}

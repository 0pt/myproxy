#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>

void handler(int signo){
	if(waitpid(-1, NULL, WNOHANG) < 0){
		perror("waitpid"); exit(1);
	}
}

int append(char *dst, char *src){
	while(*src){
		*dst++ = *src++;
	}
	return 1;
}

int main(void){
	int listenfd, connfd, nbytes;
	char buf[BUFSIZ], msg[BUFSIZ];
	struct sockaddr_in servaddr;
	pid_t pid;
	int flag;
	int portnum;
	char *type, *url, *ver;
	char *protocol, *host, *path, *pathport, *port;
	struct hostent *servhost;
	struct servent *servprot;
	int servfd;

	signal(SIGCHLD, handler);
	if( (listenfd = socket(AF_INET, SOCK_STREAM, 0))<0 ){
		perror("socket"); exit(1);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(10000);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	flag = 1;
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) < 0){
		printf("%s: fail set REUSEADDR(%d)\n",__func__, errno);
		exit(1);
	}

	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
		perror("bind"); exit(1);
	}
	if(listen(listenfd, 5)<0){
		perror("listen"); exit(1);
	}
	while(1){
		if( (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL))<0){
			perror("accept"); exit(1);
		}

		if((pid = fork()) < 0){
			perror("fork");
		}
		if(pid == 0){
			close(listenfd);
			if( (nbytes = read(connfd, buf, sizeof(buf)))<0 ){
				perror("read"); exit(1);
			}else if(nbytes == 0){
				// printf("nbytes is 0\n");
				exit(1);
			}
			printf("--- received ---\n");
			printf("%s",buf);
			printf("---------------\n");
			type = strtok(buf, " ");
			url = strtok(NULL, " ");
			ver = strtok(NULL, "\r\n");
			protocol = strtok(url, "://");
			host = strtok(NULL, "/");
			pathport = strtok(NULL, " ");
			path = strtok(pathport, ":");
			port = strtok(pathport, " ");
			
			strcat(msg, type);
			strcat(msg, " ");
			// strcat(msg, protocol);
			// strcat(msg, "://");
			// strcat(msg, host);
			strcat(msg, "/");
			strcat(msg, path);
			strcat(msg, " ");
			strcat(msg, "HTTP/1.0");
			// strcat(msg, ver);
			strcat(msg, "\r\nHost: ");
			strcat(msg, host);
			strcat(msg, "\r\n\r\n");
			printf("--- requested ---\n");
			printf("%s", msg);
			printf("-----------------\n");
			
			if(port != NULL){
				portnum = atoi(port);
			}else{
				portnum = 80;
			}

			if( (servprot = getservbyname(protocol, "tcp")) == NULL){
				perror("getservbyname"); exit(1);
			}
			// printf("%s\n",host);
			if( (servhost = gethostbyname(host)) == NULL){
				perror("gethostbyname"); exit(1);
			}

			if( (servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
				perror("socket"); exit(1);
			}
			bzero(&servaddr, sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			portnum = 80;
			servaddr.sin_port = htons(portnum);
		
			// printf("%s\n",servhost->h_addr);
			// servaddr.sin_addr = *(struct in_addr*)servhost->h_addr;
			bcopy(servhost->h_addr, &servaddr.sin_addr, servhost->h_length);
			// memcpy((char *)&servaddr.sin_addr, servhost->h_addr, servhost->h_length);
			if(connect(servfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
				perror("connect"); exit(1);
			}
			if( (nbytes = write(servfd, msg, strlen(msg))) < 0){
				perror("write"); exit(1);
			}
			while(nbytes){
				bzero(&buf, sizeof(buf));
				if( (nbytes = read(servfd, buf, sizeof(buf))) < 0){
					perror("read"); exit(1);
				}
				write(connfd, buf, nbytes);
//				buf[nbytes] = '\0';
//				printf("%s",buf);
			}
			
			close(servfd);
			close(connfd);
		exit(1);
		}
	}
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/select.h>

#define MFD (16)

struct getform{
	char *type;
	char *url;
	char *ver;
};
struct urlinfo{
	char *prot;
	char *host;
	char *pathport;
	char *path;
	char *port;
	int portnum;
};

struct sessions{
	int servfd;
	int connfd;
	struct getform form;
	struct urlinfo uinfo;
	char buf[BUFSIZ];
	char msg[BUFSIZ];
	struct sockaddr_in servaddr;
	int nbytes;
	struct hostent *servhost;
	struct servent *servprot;
};

// void handler(int signo){
// 	if(waitpid(-1, NULL, WNOHANG) < 0){
// 		perror("waitpid"); exit(1);
// 	}
// }

// int append(char *dst, char *src){
// 	while(*src){
// 		*dst++ = *src++;
// 	}
// 	return 1;
// }

int main(void){
	int listenfd, connfd, nbytes;
	char buf[BUFSIZ], msg[BUFSIZ];
	struct sockaddr_in servaddr;
	pid_t pid;
	int flag;
	// int portnum;
	// char *type, *url, *ver;
	// char *protocol, *host, *path, *pathport, *port;
	// struct hostent *servhost;
	// struct servent *servprot;
	// int servfd;
	
	int ik;
	int cfds[MFD];
	int ncl;
	fd_set mfds, rfds;
	int maxfd;
	struct timeval tick;

	// signal(SIGCHLD, handler);
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

	tick.tv_sec = 0;
	tick.tv_usec = 300*1000;
	FD_ZERO(&mfds);
	FD_SET(listenfd, &mfds);
	rfds = mfds;
	ncl = 0;
	struct sessions sess[MFD];
	for(int i=0; i<MFD; i++){
		sess[i].connfd = -1;
	}

	while(1){
		maxfd = -1;
		// printf("aaa %d\n", ncl);
		for(int i=0; i<MFD; i++){
			if(sess[i].connfd > maxfd){
				maxfd = sess[i].connfd;
				printf("%d\n",maxfd);
			}
		}
		rfds = mfds;
		ik = select(maxfd+1, &rfds, NULL, NULL, &tick);
		if(FD_ISSET(listenfd, &rfds)){
			for(int i=0; i<MFD; i++){
				if(sess[i].connfd < 0){
					if( (sess[i].connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) < 0){
						perror("connect"); exit(1);
					}
					if(ncl < MFD){
						printf("connfd: %d\n",sess[i].connfd);
						FD_SET(sess[i].connfd, &mfds);
						ncl++;
					}else{
						close(sess[i].connfd);
						sess[i].connfd = -1;
					}
				}
			}
		}

		for(int i=0; i<MFD; i++){
			if(sess[i].connfd < 0){
				continue;
			}
			if(FD_ISSET(sess[i].connfd, &rfds)){
				printf("read %d\n",sess[i].connfd);
				sess[i].nbytes = read(sess[i].connfd, sess[i].buf, sizeof(sess[i].buf));
				if(sess[i].nbytes > 0){
					printf("--- received ---\n");
					printf("%s",sess[i].buf);
					printf("---------------\n");
					sess[i].form.type = strtok(sess[i].buf, " ");
					sess[i].form.url = strtok(NULL, " ");
					sess[i].form.ver = strtok(NULL, "\r\n");
					sess[i].uinfo.prot = strtok(sess[i].form.url, "://");
					sess[i].uinfo.host = strtok(NULL, "/");
					sess[i].uinfo.pathport = strtok(NULL, " ");
					sess[i].uinfo.path = strtok(sess[i].uinfo.pathport, ":");
					sess[i].uinfo.port = strtok(NULL, "");
					strcat(sess[i].msg, sess[i].form.type);
					strcat(sess[i].msg, " ");
					// strcat(msg, protocol);
					// strcat(msg, "://");
					// strcat(msg, host);
					strcat(sess[i].msg, "/");
					strcat(sess[i].msg, sess[i].uinfo.path);
					strcat(sess[i].msg, " ");
					strcat(sess[i].msg, "HTTP/1.0");	// required 1.0 or 0.9
					// strcat(msg, ver);
					strcat(sess[i].msg, "\r\nHost: ");
					strcat(sess[i].msg, sess[i].uinfo.host);
					strcat(sess[i].msg, "\r\n\r\n");
					printf("--- requested ---\n");
					printf("%s", sess[i].msg);
					printf("-----------------\n");
					if( (sess[i].nbytes = read(sess[i].connfd, sess[i].buf, sizeof(sess[i].buf)))<0 ){
						perror("read"); exit(1);
					}else if(sess[i].nbytes == 0){
						// printf("nbytes is 0\n");
						exit(1);
					}
					if(sess[i].uinfo.port != NULL){
						sess[i].uinfo.portnum = atoi(sess[i].uinfo.port);
					}else{
						sess[i].uinfo.portnum = 80;
					}
					if( (sess[i].servprot = getservbyname(sess[i].uinfo.prot, "tcp")) == NULL){
						perror("getservbyname"); exit(1);
					}
					// printf("%s\n",host);
					if( (sess[i].servhost = gethostbyname(sess[i].uinfo.host)) == NULL){
						perror("gethostbyname"); exit(1);
					}

					if( (sess[i].servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
						perror("socket"); exit(1);
					}
					bzero(&(sess[i].servaddr), sizeof(sess[i].servaddr));
					sess[i].servaddr.sin_family = AF_INET;
					sess[i].uinfo.portnum = 80;
					sess[i].servaddr.sin_port = htons(sess[i].uinfo.portnum);
					bcopy(sess[i].servhost->h_addr, &(sess[i].servaddr.sin_addr), sess[i].servhost->h_length);
					if(connect(sess[i].servfd, (struct sockaddr*)&(sess[i].servaddr), sizeof(sess[i].servaddr)) < 0){
						perror("connect"); exit(1);
					}
					if( (sess[i].nbytes = write(sess[i].servfd, sess[i].msg, strlen(sess[i].msg))) < 0){
						perror("write"); exit(1);
					}
					while(sess[i].nbytes){
						bzero(&(sess[i].buf), sizeof(sess[i].buf));
						if( (sess[i].nbytes = read(sess[i].servfd, sess[i].buf, sizeof(sess[i].buf))) < 0){
							perror("read"); exit(1);
						}
						write(sess[i].connfd, sess[i].buf, sess[i].nbytes);
						sess[i].buf[sess[i].nbytes] = '\0';
						printf("%s",sess[i].buf);
					}
					close(sess[i].servfd);
				}else{
					close(sess[i].connfd);
					FD_CLR(sess[i].connfd, &mfds);
					sess[i].connfd = -1;
				}
			}
		}
	}
	return 0;
}

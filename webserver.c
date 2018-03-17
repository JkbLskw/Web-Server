#define _POSIX_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include "request_processor.h"


#define BUFFER_LEN 20
#define SERVERPATH_LEN 1
#define LOGFILE "server.log"
#define PIDFILE "server.pid"

char *serverpath;


/* signal handler zum löschen der prozess-soehne */
void signalHandler(int sig){
	int status;
	wait(&status);
}

/* funktion zum forken in einen sohn-prozess  */
void sub(int serversockfd, int newsockfd, int logfd){
  int pid;
  pid = fork();
  if(pid == -1){
    perror("fork\n");
    exit(-1);
  } else if(pid == 0){
    /* serversocket wird im sohn geschlossen */
    close(serversockfd);        
    /* request wird im sohn weitergeleitet */
    process_request(newsockfd, newsockfd, logfd, serverpath);
    exit(0);
  } else{
    /* close(newsockfd); */
    signal(SIGCHLD, signalHandler);
  }
	
}

/* tcp server */
void do_serv(int port, int logfd){
    char *log_buffer;
    int log_spacing_size = 3;
    int log_semaphore = 0;
	int sockfd = 0;
	int newsockfd = 0;
	socklen_t clientlen;
	struct sockaddr_in servaddr, clientaddr;
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	
    /* server-socket wird erzeugt */
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		exit(-1);
	}
    
    /* server-socket wird an die adresse und port gebunden*/
	if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in)) < 0){
		perror("bind");
		exit(-1);
	}
    
    /* startet listen des server-sockets */
	if(listen(sockfd, 5) < 0){
		perror("listen");
		exit(-1);
	}

	write(0, "webserver started\n", 18);
	
    /* endlos-schleife zum akzeptieren von eingehenden verbindungen*/
	for(;;){
        /* laenge des address-structs*/
		clientlen = sizeof(struct sockaddr);
        log_semaphore = 1;
        /* neuer client socket wird erzeugt, wenn eine anfrage reinkommt*/
		newsockfd = accept(sockfd, (struct sockaddr*)&clientaddr, &clientlen);
		if(newsockfd > 0 && log_semaphore == 1){
            log_semaphore = 0;
            /* init log_buffer */
            log_buffer = calloc(strlen(inet_ntoa(clientaddr.sin_addr)) + log_spacing_size, sizeof(char));
            /* loggt die ip der eingehenden Anfrage */
            sprintf(log_buffer, "%s - ", inet_ntoa(clientaddr.sin_addr));
            write(logfd, log_buffer, strlen(log_buffer));	
            write(0, log_buffer, strlen(log_buffer));
            free(log_buffer);
            /* gibt die file-descriptoren an den sohn-prozess weiter*/
            sub(sockfd, newsockfd, logfd);
		}
		
	}
    /* schliessen des server-sockets*/
	close(sockfd);
}

int main(int argc, char *argv[]){
    /* standard-port */
	int port = 8080;
	
	if(argc == 1){
        /* server-pfad wenn kein pfad vom nutzer angegeben wurde */
		serverpath = malloc(SERVERPATH_LEN);
		serverpath[0] = '.';
	}
	if(argc >= 2){
        
        /* wenn stop als parameter mitgegeben wurde */
		if(strcmp(argv[1],"stop") == 0){
			
			/* oeffnen des pid-files */
			int pidfd = open(PIDFILE, O_RDONLY, 0644);
			int pid = 0;
			
			if(pidfd < 0){
				perror("can't open server.pid");
				exit(-1);
			}
			
			/* lesen des pid-strings */
			char readbuffer[BUFFER_LEN];
			int reading = read(pidfd, readbuffer, BUFFER_LEN);
			if(reading < 0){
				perror("can't read pid in pid-file");
				exit(-1);
			}
			
			/* pid-string in int umwandeln */
			pid = atoi(readbuffer);
			
            /* den process hinter der pid killen */
			kill(pid, SIGINT);
            
            /* den pid-file-descriptor schliessen */
			close(pidfd);
			
			/* loeschen des pid-files */
			if(remove(PIDFILE) < 0){
			  perror("Can't remove Pid-File");
			}
			
			printf("Stopped Server at %d\n", pid);
            
            /* schliessen der anwendung */
			exit(0);
		}else{
            /* vom nutzer angegebener server-pfad */
            serverpath = argv[1];
        }
	}
	
    /* vom nutzer angegebener port */
	if(argc >= 3){
		port = atoi(argv[2]);
	}

	/* erzeugt den log-file-descriptor */
	int logfd = open(LOGFILE, O_CREAT|O_APPEND, 0644);
	if(logfd < 0){
		perror("can't open server.log");
		exit(-1);
	}
	
	/* oeffnen des pid-files */
	int pidfd = open(PIDFILE, O_CREAT|O_TRUNC|O_WRONLY, 0644);
	if(pidfd < 0){
		perror("can't open server.pid");
		exit(-1);
	}
    /* schreibt die momentane pid in das pid-file */
	char pidarray[BUFFER_LEN];
	int pid = getpid();
	sprintf(pidarray, "%d", pid);
	write(pidfd, pidarray, strlen(pidarray));
    
    /* schliessen des pid-file-descriptors*/
	close(pidfd);
	
    /* ruft die funktion zum empfangen von anfragen auf */
	do_serv(port, logfd);

	if(argc == 1){
	  free(serverpath);
	}
	return 0;
}

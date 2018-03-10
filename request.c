#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include "prozent.h"
#include "anfrage.h"
#include "auftrag.h"

#define REQUEST_BUFFER_SIZE 2048
#define BUFFER_SIZE 200

void process_request(int infd, int outfd, int logfd, char *serverpath){
	
	/* erstellt ein felder struct fuer die request-teile */
	struct felder *parts = malloc(sizeof(struct felder));
	
	/* definitionen der char-buffer */
	char buffer[BUFFER_SIZE];
	char request_buffer[REQUEST_BUFFER_SIZE];
	char log_buffer[BUFFER_SIZE];
	char time_buffer[BUFFER_SIZE];
	char first_line[BUFFER_SIZE];

	/* fill the buffers with zeros */
	memset(buffer,0,BUFFER_SIZE);
	memset(request_buffer,0,REQUEST_BUFFER_SIZE);
	memset(log_buffer,0,BUFFER_SIZE);
	memset(time_buffer,0,BUFFER_SIZE);
	memset(first_line,0,BUFFER_SIZE);
	
	/* current_time setzen */
	time_t t;
	struct tm *current_time;
	t = time(NULL);
	current_time = localtime(&t);
	
	/* zeiger auf first_line*/
	char *ptr_first_line = first_line;
	int is_first = 1;
	int gelesen;
	
	/* liesst den request string */
	while(1){
		gelesen = read(infd, buffer, BUFFER_SIZE);
		if(gelesen < 0){
			perror("read request error");
		}else{
			/* appended den buffer des requests-lesens in den requestbuffer */
		  strncat(request_buffer, buffer, BUFFER_SIZE);
		  /* wenn die zeile die erste ist */
		  if(is_first == 1){
		    /* kopiert den buffer des request-lesens in den first_line-buffer */
		    strncpy(first_line, buffer, BUFFER_SIZE);
		    is_first = 0;
		  }
			
		  /* bircht die while-schleife ab, wenn im request-buffer zwei linefeeds enthalten sind */
		  if(strstr(request_buffer, "\r\n\r\n") != NULL){
		    break;
		  }
		}
	}
	
	/* setzt den pointer der first_line auf das erste \r */
	while(*ptr_first_line != '\r'){
		ptr_first_line += 1;
	}
	/* ersetzt das \r mit \0 */
	*ptr_first_line = '\0';
	
	/* zerlegt die first_line in drei einzelne teile*/
	auftragaufdroeseln(first_line, parts);
	
	/* get time in time_buffer*/
	strftime(time_buffer, BUFFER_SIZE, "%d.%m.%Y %T", current_time);
	sprintf(log_buffer, "%s - %s%s", time_buffer, parts->data, "\n");
	
	/* log request */
	write(logfd, log_buffer, BUFFER_SIZE);
	write(0, log_buffer, BUFFER_SIZE);
	/* write response */
	do_get(parts->id, serverpath, outfd);

	/* free of allocs */
	free(parts);
	
	/* schliesst input file discriptor */
	close(infd);
}

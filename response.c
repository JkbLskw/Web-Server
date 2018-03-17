#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include "response.h"

/* definitionen der html tags */
char html_start_tag[6] = "<html>";
char html_end_tag[7] = "</html>";
char new_line[1] = "\n";
char list_start_tag[4] = "<ul>";
char list_end_tag[5] = "</ul>";
char li_start[4] = "<li>";
char li_end[5] = "</li>";
char a_start[9] = "<a href=\"";
char a_start_close[3] = "\">";
char a_end[4] = "</a>";
char dir_divider[1] = "/";

/* erzeugt den response-header */
void write_response_header(int outfd, char *status, int status_length, char *content_type, int content_type_length, char *content_size, int content_size_length){
	char header[9] = "HTTP/1.0 ";
	char line_feed[2] = "\r\n";
	/* schreibt den header */
	write(outfd, header, sizeof(header));
	
	/* schreibt den status */
	write(outfd, status, status_length);
	
	/* schreibt new line */
	write(outfd, line_feed, sizeof(line_feed));
	
	/* schreibt den content type */
	write(outfd, content_type, content_type_length);
	
	/* schreibt die content size */
	write(outfd, content_size, content_size_length);
	
	/* schreibt leere zeile */
	write(outfd, line_feed, sizeof(line_feed));
}

/* gibt den inhalt einer datei aus */
void write_response_content(int read_fd, int outfd){
    
    /* buffer zum lesen der datei */
	char buffer[1000];
	int gelesen;
	
    /* schreibt solange bis die datei zu ende ist */
	while((gelesen = read(read_fd, buffer, 1000)) != 0){
        write(outfd, buffer, gelesen);
	}
    
    /* schliesst den read-file-descriptor */
	close(read_fd);
}

/* erzeugt einen eintrag der ordner-auflistung */
void write_response_eintrag(int outfd, struct dirent *eintrag, char *dir_path){

    /* setzt den path-zeiger hinter das erste zeichen (/) */
	dir_path += 1;

    /* schreibt des start-tag des li und des a */
	write(outfd, li_start, sizeof(li_start));
	write(outfd, a_start, sizeof(a_start));

    /* schreibt das verzeichnis */
	write(outfd, dir_path, strlen(dir_path));
    
    /* wenn der ordner-pfad mehr als ein zeichen hat wird ein '/' dahinter geschrieben*/
	if(strlen(dir_path) > 1){
		write(outfd, dir_divider, sizeof(dir_divider));
	}
	
    /* schreibt den namen des files als href des a-tags */
	write(outfd, eintrag->d_name, strlen(eintrag->d_name));
    /* schreibt das ende des starts des a-tags */
	write(outfd, a_start_close, sizeof(a_start_close));
    /* schreibt den namen des files als value des a-tags */
	write(outfd, eintrag->d_name, strlen(eintrag->d_name));
    /* schreibt das schliessende-tag des a */
	write(outfd, a_end, sizeof(a_end));
    /* schreibt das schliessende tag des li */
	write(outfd, li_end, sizeof(li_end));
    /* schreibt eine neue zeile */
	write(outfd, new_line, sizeof(new_line));
}

/* gibt die laenge des erzeugen htmls (liste der files) zurueck */
int count_dir_file_length(const char *respath, char *dir_path){
	DIR *dir;
	struct dirent *eintrag;
	int dir_response_length = 0;
	
	/* setzt den path-zeiger hinter das erste zeichen (/) */
	dir_path += 1;
	
	dir_response_length += sizeof(html_start_tag); /* len start tag html */
	dir_response_length += sizeof(new_line); /* len new line */
	dir_response_length += sizeof(list_start_tag); /* len start tag list*/
	dir_response_length += sizeof(new_line); /* len new line */
	
	dir = opendir(respath);
	while((eintrag = readdir(dir)) != NULL){
		if(strncmp(eintrag->d_name, ".", 1) == 0 || strncmp(eintrag->d_name, "..", 2) == 0){
			continue;
		}
		dir_response_length += sizeof(li_start); /* len li_start */
		dir_response_length += sizeof(a_start); /* len a_start */
		dir_response_length += strlen(dir_path); /* len dir_path */
		if(strlen(dir_path) > 1){
			dir_response_length += sizeof(dir_divider); /* wenn der ordner-pfad mehr als ein zeichen hat wird die laenge des dividers addiert*/
		}
		dir_response_length += strlen(eintrag->d_name); /* len des namens des files */
		dir_response_length += sizeof(a_start_close); /* len a_start_close */
		dir_response_length += strlen(eintrag->d_name); /* len des namens des files */
		dir_response_length += sizeof(a_end); /* len a_end */
		dir_response_length += sizeof(li_end); /* len li_end */
	}
	dir_response_length += sizeof(list_end_tag); /* len end tag list */
	dir_response_length += sizeof(new_line); /* len new line */
	dir_response_length += sizeof(html_end_tag); /* len end tag html*/
	dir_response_length += sizeof(new_line); /* len new line */

	closedir(dir);
	return dir_response_length;
}

/* verarbeitet die anfrage und gibt je nachdem ein generiertes html oder das file zurück */
void do_get(char *respath, const char *server_path, int outfd){
    
    /* file-descriptor für das lesen eines files */
	int read_fd;
    /* groesse des contents-strings */
    int content_size_count;
    /* groesse des types-strings */
    int content_type_count;
    /* id des mimes eines files*/
	int mime_count = 0;
    /* char-zeiger der endung eines file-names */
	char *endung;
    
    /* struct-zeiger des stats */
	struct stat st;
    /* dir-zeiger des ordners */
	DIR *dir;
    /* struct-zeiger eines eintrags */
	struct dirent *eintrag;
	
    /* auflistung der mimes */
	char *mime[][2] = {
		{".html", "text/html"},
		{".txt", "text/plain"},
		{".png", "image/png"},
		{".css", "text/css"},
		{".js", "text/javascript"},
		{".xml", "text/xml"},
		{NULL, NULL}
	};
	
    /* definition der status */
	char *status[2] = {"200 OK","404 Not Found"};
    
    /* definition des not-found-html */
	char html_not_found[27] = "<html>404 Not Found</html>\n";
    
    /* char-arrays zum halten der header-strings */
	char content_size[30];
	char content_type[30];
    
    /* char-arrays zum halten der pfade */
	char path[30];
	char path_html_list[30];
	
	
	/* wenn der serverpath der aktuelle pfad (.) ist wird nur der path des requests genommen 
	 * wenn der serverpath ein spezifischer (leangerer) pfad ist wird der serverpath und der requestpath zusammengefuegt */
	if(strcmp(server_path, ".") != 0){
		sprintf(path_html_list, "%s", respath);
	}else{
		sprintf(path_html_list, "%s%s", server_path, respath);
	}
	
	/* der pfad von serverpath und requestpath zusammengefügt für das lesen der datei oder des ordners */
	sprintf(path, "%s%s", server_path, respath);
	
    /* oeffnen des pfades */
	read_fd = open(path, O_RDONLY);
    
    /* wenn der pfad nicht existiert wird die 404-meldung generiert und geschrieben */
	if(read_fd < 0){
		content_type_count = sprintf(content_type,"Content-Type: %s\r\n", mime[0][1]);
		content_size_count = sprintf(content_size,"Content-Length: %lu\r\n", sizeof(html_not_found));
		write_response_header(outfd, status[1], strlen(status[1]), content_type, content_type_count, content_size, content_size_count);
		write(outfd, html_not_found, sizeof(html_not_found));
	}
	else{
        /* wenn stat nicht auf den pfad zugreifen kann */
		if(stat(path, &st) == -1){
			perror("Stat");
		}
		else{
            /* wenn der pfad ein ordner beschreibt */
			if(S_ISDIR(st.st_mode)){
                
                /* header-felder werden erzeugt und geschrieben */
				content_type_count = sprintf(content_type,"Content-Type: %s\r\n", mime[0][1]);
				content_size_count = sprintf(content_size,"Content-Length: %i\r\n", count_dir_file_length(path, path_html_list));
				write_response_header(outfd, status[0], strlen(status[0]), content_type, content_type_count, content_size, content_size_count);
				
                /* der ordner-pfad wird geschrieben */
				dir = opendir(path);
				
                /* schreibt eine neue zeile */
				write(outfd, html_start_tag, sizeof(html_start_tag));
                /* schreibt das start-tag des html */
				write(outfd, new_line, sizeof(new_line));
                /* schreibt das start-tag der liste (ul) */
				write(outfd, list_start_tag, sizeof(list_start_tag));
                /* schreibt eine neue zeile */
				write(outfd, new_line, sizeof(new_line));
                
                /* liest eintraege aus dem ordnerpfad bis zum ende */
				while((eintrag = readdir(dir)) != NULL){
                    /* wenn '.' oder '..' als eintrag vorkommen werden diese vernachlaessigt */
					if(strncmp(eintrag->d_name, ".", 1) == 0 || strncmp(eintrag->d_name, "..", 2) == 0){
						continue;
					}else{
                        /* erzeugt und schreibt das html für den eintrag */
						write_response_eintrag(outfd, eintrag, path_html_list);
					}
				}
				
				closedir(dir);
                /* schreibt das end-tag der liste (ul) */
				write(outfd, list_end_tag, sizeof(list_end_tag));
                /* schreibt eine neue zeile */
				write(outfd, new_line, sizeof(new_line));
                /* schreibt das end-tag des html */
				write(outfd, html_end_tag, sizeof(html_end_tag));
                /* schreibt eine neue zeile */
				write(outfd, new_line, sizeof(new_line));
			}
            /* wenn der pfad eine datei beschreibt */
			else{
                /* wenn es eine ausfuehrbare datei ist */
                if(st.st_mode & S_IXUSR){                    
                    char *args[] = {"./", NULL};
                    dup2(outfd, 1);
                    dup2(outfd, 0);
                    execv(path, args);
                }else{
					/* die endung wird aus dem datei-pfad extrahiert */
					endung = strrchr(path, '.');
					
					/* endlostschleife mit abbruch bei null */
					while(1){
						/* wenn die schleife durchgelaufen ist wird der mime-count auf 1 (text/plain) gestellt */
						if(mime[mime_count][0] == NULL){
							mime_count = 1;
							break;
						}
						else{
							/* wenn die mime-endung mit der datei-endung uebereinstimmt bleibt der mime-count so stehen */
							if(strcmp(endung, mime[mime_count][0]) == 0){
								break;
							}
						}
						mime_count++;
					}
					
					/* header-felder werden erzeugt und geschrieben */
					content_type_count = sprintf(content_type,"Content-Type: %s\r\n", mime[mime_count][1]);
					content_size_count = sprintf(content_size,"Content-Length: %lu\r\n", st.st_size);
					write_response_header(outfd, status[0], strlen(status[0]), content_type, content_type_count, content_size, content_size_count);
					/* schreibt den content der datei */
					write_response_content(read_fd, outfd);
				}
			}
			
		}
		
	}
}

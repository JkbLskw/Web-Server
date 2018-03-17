#include <dirent.h>
void write_response_header(int, char *, int, char *, int, char *, int);
void write_response_content(int, int);
void write_response_eintrag(int, struct dirent *, char *);
int count_dir_file_length(const char *, char *);
void do_get(char *, const char *, int);

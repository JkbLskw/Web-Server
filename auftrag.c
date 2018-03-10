#include <stdio.h>
#include <stdlib.h>
#include "auftrag.h"
#include "prozent.h"

/* teilt die drei teile eines HTTP-GET und schreibt sie in ein struct */
void auftragaufdroeseln(char *auftrag, struct felder *droesel){
	
	/* der ganze string decoded */
	char *decoded_string_ptr = prozentdecode(auftrag);
	/* der ganze string als teile decoded */
	char *decoded_parts_ptr = prozentdecode(auftrag);
	
	/* setzen des data-pointers auf den ganzen decoded-string */
	droesel->data = decoded_string_ptr;
	
	/* setzen des art-pointers auf den ersten teil des decoded-strings */
	droesel->art = decoded_parts_ptr;
	
	/* die beiden anderen teile werden gesetzt */
	int teil = 2;
	while(*decoded_parts_ptr){
		if(*decoded_parts_ptr == ' ' && teil < 3){
			if(teil == 2){
				droesel->id = &decoded_parts_ptr[1];
			}
			else if(teil == 1){
				droesel->modus = &decoded_parts_ptr[1];
			}
            teil--;
            *decoded_parts_ptr = '\0';
		}
		decoded_parts_ptr++;
	}
}

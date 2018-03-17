#include <stdio.h>
#include <stdlib.h>
#include "splitter.h"
#include "percent_processor.h"

/* teilt die drei teile eines HTTP-GET und schreibt sie in ein struct */
void split_request(char *request, struct fields *splitted_fields){
	
	/* der ganze string decoded */
	char *decoded_string_ptr = percentdecode(request);
	/* der ganze string als teile decoded */
	char *decoded_parts_ptr = percentdecode(request);
	
	/* setzen des data-pointers auf den ganzen decoded-string */
	splitted_fields->data = decoded_string_ptr;
	
	/* setzen des art-pointers auf den ersten teil des decoded-strings */
	splitted_fields->art = decoded_parts_ptr;
	
	/* die beiden anderen teile werden gesetzt */
	int split_count = 2;
	while(*decoded_parts_ptr){
		if(*decoded_parts_ptr == ' ' && split_count < 3){
			if(split_count == 2){
				splitted_fields->id = &decoded_parts_ptr[1];
			}
			else if(split_count == 1){
				splitted_fields->modus = &decoded_parts_ptr[1];
			}
            split_count--;
            *decoded_parts_ptr = '\0';
		}
		decoded_parts_ptr++;
	}
}

#include <stdio.h>
#include <stdlib.h>
#include "percent_processor.h"

/* wandelt einen hex wert in einen int wert */
int hex_to_int(char c){
	return c-(c <= '9' && c >= '0' ? '0' : 'A' - 10);
}

/* wandelt einen int wert in einen hex wert */
char * int_to_hex(int i){
	int temp = i;
	int quotient = 16;
	int digits = 2;
	
	/* neuer rueckgabe-string */
	char *hex_string = malloc(sizeof(char)*digits);
	char *start = hex_string;
	hex_string += digits - 1;
	
	/* standard hex to int anweisungen */
	int q = temp / quotient;
	
	while(q != 0){
		*hex_string = temp % quotient;
		
		if(*hex_string >= 10){
			*hex_string += 65 - 10;
		}else{
			*hex_string += 48;
		}
		
		q = temp / quotient;
		temp = q;
		
		hex_string--;
	}
	hex_string = start;
	return hex_string;
}

/* wandelt alle prozentzeichen und deren zwei zeichen danach in lesbaren string */
char * percentdecode(const char *input){
	int decode_size = 0;
	const char *start = input;
		
	/* decode_size ist neue groesse des rueckgabestrings */
	while(*input){
		if(*input == '%'){
			input += 2;
		}
		decode_size++;
		input++;
	}
		
	/* mallocen des neuen rueckgabe-strings */
	char *result = malloc(sizeof(char) * decode_size);
		
	input = start;
	
	/* umwandlung von prozent-zeichen und deren zwei nachkommenden zeichen */
	while(*input){
		if(*input == '%'){
			/* umwandndlung einer hex zahl in einen char */
			char c = hex_to_int(input[1]) << 4 | hex_to_int(input[2]);
			*result = c;
			input += 2;
		}else{
			*result = *input;
		}
		result++;
		input++;
	}	
	result -= decode_size;
	return result;
}

/* wandelt einen string mit einem zucodieren teil in einen mit prozentzeichen und hexwert */
char * percentencode(const char *input, const char * encode_count){
	int encode_size = 0;
	int encode_count_rewind = 0;
	const char *start = input;
	int encoded = 0;
	
	/* zaehlt die neue groesse des strings */
	while(*input){
		while(*encode_count){
			if(*input == *encode_count){
				encode_size += 2;
			}
			encode_count++;
			encode_count_rewind++;
		}
		encode_count -= encode_count_rewind;
		encode_count_rewind = 0;
		encode_size++;
		input++;
	}
	
	/* setzt den pointer zurueck */
	input = start;
	
	char * result = malloc(sizeof(char) * encode_size);
	
	/* wandelt alle stellen an denen der zucodieren-string
	 * uebereinstimmt in den entsprechenden hex wert mit
	 * prozent-zeichen davor um */
	while(*input){
		while(*encode_count){
			if(*input == *encode_count){
				
				char *hex_string = int_to_hex(*input);
				
				result[0] = '%';
				result[1] = hex_string[0];
				result[2] = hex_string[1];
				
				free(hex_string);
				
				result += 2;
				encoded = 1;
			}
			encode_count++;
			encode_count_rewind++;
		}
		encode_count -= encode_count_rewind;
		encode_count_rewind = 0;
		
		if(encoded == 0){
			*result = *input;
		}
		result++;
		input++;
		encoded = 0;
	}
	
	/* rewind des pointers des result-strings */
	result -= encode_size;
	return result;
}


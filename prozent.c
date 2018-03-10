#include <stdio.h>
#include <stdlib.h>
#include "prozent.h"

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
char * prozentdecode(const char *eingabe){
	int decode_size = 0;
	const char *start = eingabe;
		
	/* decode_size ist neue groesse des rueckgabestrings */
	while(*eingabe){
		if(*eingabe == '%'){
			eingabe += 2;
		}
		decode_size++;
		eingabe++;
	}
		
	/* mallocen des neuen rueckgabe-strings */
	char *result = malloc(sizeof(char) * decode_size);
		
	eingabe = start;
	
	/* umwandlung von prozent-zeichen und deren zwei nachkommenden zeichen */
	while(*eingabe){
		if(*eingabe == '%'){
			/* umwandndlung einer hex zahl in einen char */
			char c = hex_to_int(eingabe[1]) << 4 | hex_to_int(eingabe[2]);
			*result = c;
			eingabe += 2;
		}else{
			*result = *eingabe;
		}
		result++;
		eingabe++;
	}	
	result -= decode_size;
	return result;
}

/* wandelt einen string mit einem zucodieren teil in einen mit prozentzeichen und hexwert */
char * prozentencode(const char *eingabe, const char * zucodieren){
	int encode_size = 0;
	int zucodieren_rewind = 0;
	const char *start = eingabe;
	int encoded = 0;
	
	/* zaehlt die neue groesse des strings */
	while(*eingabe){
		while(*zucodieren){
			if(*eingabe == *zucodieren){
				encode_size += 2;
			}
			zucodieren++;
			zucodieren_rewind++;
		}
		zucodieren -= zucodieren_rewind;
		zucodieren_rewind = 0;
		encode_size++;
		eingabe++;
	}
	
	/* setzt den pointer zurueck */
	eingabe = start;
	
	char * result = malloc(sizeof(char) * encode_size);
	
	/* wandelt alle stellen an denen der zucodieren-string
	 * uebereinstimmt in den entsprechenden hex wert mit
	 * prozent-zeichen davor um */
	while(*eingabe){
		while(*zucodieren){
			if(*eingabe == *zucodieren){
				
				char *hex_string = int_to_hex(*eingabe);
				
				result[0] = '%';
				result[1] = hex_string[0];
				result[2] = hex_string[1];
				
				free(hex_string);
				
				result += 2;
				encoded = 1;
			}
			zucodieren++;
			zucodieren_rewind++;
		}
		zucodieren -= zucodieren_rewind;
		zucodieren_rewind = 0;
		
		if(encoded == 0){
			*result = *eingabe;
		}
		result++;
		eingabe++;
		encoded = 0;
	}
	
	/* rewind des pointers des result-strings */
	result -= encode_size;
	return result;
}


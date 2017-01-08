/*
 * File: rsa.h
 * Created by Hamza ESSAYEGH (Querdos)
 */

#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>
#include <string.h>

#define BASE_SAVE 		61
#define MAX_CHARS_LINES 50

/**
 * Write chars (i.e. n_str, d_str, e_str) to a file (fp)
 */
void write_chars(char *str, int count, FILE *fp) {
	int i;
	
	for (i=0; i<strlen(str); i++) {
		fputc(str[i], fp);
		count++;
		
		if (count == MAX_CHARS_LINES) {
			count = 0;
			fputc('\n', fp);
		}
	}
}

/**
 * Save the private and public key pair to a new dir .rsa
 * 
 * return -1 if an error occured
 */
int save_keypair(mpz_t n, mpz_t e, mpz_t d) {
	// vars
	FILE *fp_rsa;
	char *n_str, *e_str, *d_str;
	int i, count;
	
	// saving public key
	fp_rsa = fopen(".rsa/rsa.pub", "w");
	if (NULL == fp_rsa) {
		printf("Unable to open '.rsa/rsa.pub' for write operation. Aborting.\n");
		return -1;
	}
	
	// allocating
	e_str = malloc((mpz_sizeinbase(e, BASE_SAVE)+2) * sizeof(char));
	mpz_get_str(e_str, BASE_SAVE, e);
	
	fputs("--- BEGIN PUBLIC KEY ---\n", fp_rsa);	
	fputs(e_str, fp_rsa);
	fputs("/", fp_rsa);
	free(e_str);
	
	// allocating
	n_str = malloc((mpz_sizeinbase(n, BASE_SAVE)+2) * sizeof(char));
	mpz_get_str(n_str, BASE_SAVE, n);
	
	count += strlen(e_str) + 1;
	write_chars(n_str, count, fp_rsa);
	
	fputs("\n--- END PUBLIC KEY ---", fp_rsa);
	fclose(fp_rsa);

	// saving private key
	fp_rsa = fopen(".rsa/rsa.priv", "w");
	if (NULL == fp_rsa) {
		printf("Unable to open '.rsa/rsa.priv' for write operation. Aborting.\n");
		return -1;
	}
	
	fputs("--- BEGIN PRIVATE KEY ---\n", fp_rsa);
	
	// allocating
	d_str = malloc((mpz_sizeinbase(d, BASE_SAVE)+2) * sizeof(char));
	mpz_get_str(d_str, BASE_SAVE, d);
	
	count = 0;
	write_chars(d_str, count, fp_rsa);
	free(d_str);
	
	fputs("/", fp_rsa);
	count++;
	write_chars(n_str, count, fp_rsa);
	
	free(n_str);
	fputs("\n--- END PRIVATE KEY ---", fp_rsa);

	// closing file
	fclose(fp_rsa);
	
	return 0;
}

/**
 * Load private key into n and d
 */
int load_priv(mpz_t n, mpz_t d) {
	// vars
	FILE *fp_rsa, *lines_cmd;
	char *n_str, *d_str, line[MAX_CHARS_LINES];
	int lines, i, step, count;
	
	// public key
	fp_rsa = fopen(".rsa/rsa.priv", "r");
	
	// existance
	if (fp_rsa == NULL) {
		printf("File '.rsa/rsa.priv' doesn't exists. Aborting.\n");
		return -1;
	}
	
	// counting lines
	lines_cmd = popen("wc -l < .rsa/rsa.priv", "r");
	fgets(line, 10, lines_cmd);
	lines = atoi(line) - 2; // removing first and last line
	pclose(lines_cmd);
	
	// allocating
	n_str 	 = malloc(50 * lines * sizeof(char));
	d_str 	 = malloc(50 * lines * sizeof(char));
	
	// init 
	memset(n_str, '\0', sizeof(n_str));
	memset(d_str, '\0', sizeof(d_str));
	
	// first line
	fgets(line, MAX_CHARS_LINES+2, fp_rsa);
	fgets(line, MAX_CHARS_LINES+2, fp_rsa); // +2 : \n and null char
	
	step = 1;
	count = 0;
	while (strcmp(line, "--- END PRIVATE KEY ---") != 0 ) {
		for (i=0; i<MAX_CHARS_LINES; i++) {
			if (line[i] == '/') {
				step = 2;
				count = 0;
				continue;
			}
			
			if (step == 1) {
				d_str[count] = line[i];
			} else {
				n_str[count] = line[i];
			}
			
			count++;
		}
		
		fgets(line, MAX_CHARS_LINES+2, fp_rsa);
	}
	fclose(fp_rsa);
	
	// inits
	mpz_inits(n, d, NULL);
	
	// converting 
	mpz_set_str(n, n_str, BASE_SAVE);
	mpz_set_str(d, d_str, BASE_SAVE);
	
	// closing
	free(n_str);
	free(d_str);
	
	return 0;
}
/**
 * Load public key into n and e
 */
int load_pub(mpz_t n, mpz_t e) {
	// vars
	FILE *fp_rsa, *lines_cmd;
	char *n_str, *e_str, line[MAX_CHARS_LINES];
	int lines, i, step, count;
	
	// public key
	fp_rsa = fopen(".rsa/rsa.pub", "r");
	
	// existance
	if (fp_rsa == NULL) {
		printf("File '.rsa/rsa.pub' doesn't exists. Aborting.\n");
		exit(1);
	}
	
	// counting lines
	lines_cmd = popen("wc -l < .rsa/rsa.pub", "r");
	fgets(line, 10, lines_cmd);
	lines = atoi(line) - 2; // removing first and last line
	//pclose(lines_cmd);
	
	// allocating
	n_str 	 = malloc(50 * lines * sizeof(char));
	e_str 	 = malloc(50 * lines * sizeof(char));
	
	// init 
	memset(n_str, 	 '\0', sizeof(n_str));
	memset(e_str,  	 '\0', sizeof(e_str));
	
	// first line
	fgets(line, MAX_CHARS_LINES+2, fp_rsa);
	fgets(line, MAX_CHARS_LINES+2, fp_rsa);
	
	step = 1;
	count = 0;
	while (strcmp(line, "--- END PUBLIC KEY ---") != 0 ) {
		for (i=0; i<MAX_CHARS_LINES; i++) {
			if (line[i] == '/') {
				step = 2;
				count = 0;
				continue;
			}
			
			if (step == 1) {
				e_str[count] = line[i];
			} else {
				n_str[count] = line[i];
			}
			
			count++;
		}
		
		fgets(line, MAX_CHARS_LINES+2, fp_rsa);
	}
	fclose(fp_rsa);
	
	// inits
	mpz_inits(n, e, NULL);
	
	// converting 
	mpz_set_str(n, n_str, BASE_SAVE);
	mpz_set_str(e, e_str, BASE_SAVE);
	
	// TODO: See why problem with free(n/e_str)
	return 0;
}
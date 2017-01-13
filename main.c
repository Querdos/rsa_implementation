/*
 * File: main.c
 * Created by Hamza ESSAYEGH (Querdos)
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <gmp.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "rsa.h"
#include "rsa_keys.h"

#define BASE_SAVE 		61
#define MAX_CHARS_LINES 50

int main(int argc, char** argv) {
	// init time
	srand(time(NULL));
	
	// checking number of arguments
	if (argc > 3 || argc == 1) {
		printf("Usage: %s --[decrypt, encrypt, generate-key-pair] file\nUsage: %s --generate-key-pair\n\n", argv[0], argv[0]);
		return EXIT_FAILURE;
	}
	
	// for encryption
	if (strcmp(argv[1], "--encrypt") == 0) {		
		// vars
		FILE *fp_plain, *size_of_file;
		mpz_t n, e;
		unsigned char *encrypted, *plain;
		unsigned long size;
		char command_wc[11], count[3], *filename;
		
		// generating the key pair
		if (load_pub(n, e) == -1) {
			return EXIT_FAILURE;
		}
	
		// opening the file (not encrypted)
		fp_plain = fopen(argv[2], "r");
		
		// exception
		if (NULL == fp_plain) {
			printf("Unable to open the file '%s'\n", argv[2]);
			mpz_clears(n, e, NULL);
			return EXIT_FAILURE;
		}
	
		// retreiving size of *plain
		sprintf(command_wc, "wc -c < %s", argv[2]);
		size_of_file = popen(command_wc, "r");
		
		fgets(count, 3, size_of_file);
		size = atoi(count);
		pclose(size_of_file);
	
		// allocating memory for string
		plain = malloc(size * sizeof(unsigned char));
		
		// retrieving plain text
		fgets(plain, size, fp_plain);
		fclose(fp_plain);
		
		// encrypting plain text
		filename = malloc(10 * strlen(argv[2]) * sizeof(char));
		memset(filename, '\0', sizeof(filename));
		
		sprintf(filename, "%s.enc", argv[2]);
		if (-1 == rsaes_pkcs1_encrypt(n, e, plain, filename)) {
			mpz_clears(n, e, NULL);
			free(filename);
			return EXIT_FAILURE;
		}
		
		// cleaning
		free(filename);
		mpz_clears(n, e, NULL);
	}
	
	// for decryption
	else if (strcmp(argv[1], "--decrypt") == 0) {
		// vars
		FILE *chars_count, *fp_encrypted;
		unsigned char *encrypted, *decrypted;
		char *wc_command, chars_result[5];
		mpz_t n, d;
		int chars, i;
		
		// trying to open the file
		fp_encrypted = fopen(argv[2], "r");
		if (NULL == fp_encrypted) {
			printf("File doesn't exists. Aborting.\n");
			return EXIT_FAILURE;
		}
		
		// retrieving rsa key pair (private)
		if (-1 == load_priv(n, d)) {
			mpz_clears(n, d, NULL);
			return EXIT_FAILURE;
		}
		
		// number of chars in encrypted file
		wc_command = malloc((strlen(argv[2]) + 8) * sizeof(char));
		sprintf(wc_command, "wc -c < %s", argv[2]);
		
		chars_count = popen(wc_command, "r");
		fgets(chars_result, 5, chars_count);
		chars = atoi(chars_result);
		
		free(wc_command);
		pclose(chars_count);
		
		// retreiving encrypted text
		encrypted = malloc(chars * sizeof(unsigned char));
		for (i=0; i<chars; i++) {
			encrypted[i] = fgetc(fp_encrypted);
		}
		fclose(fp_encrypted);
		
		// decrypting
		if (-1 == rsads_pkcs1_decrypt(decrypted, n, d, chars, encrypted, "decrypted")) {
			mpz_clears(n, d, NULL);
			free(encrypted);
			return EXIT_FAILURE;
		}
		
		// clearing
		mpz_clears(n, d, NULL);
		free(encrypted);
	}
	
	// key pair generation
	else if (strcmp(argv[1], "--generate-key-pair") == 0) {
		int dir_exists;
		mpz_t n, e, d;
		
		// checking .rsa existance
		dir_exists = mkdir(".rsa", 0755);
		
		// .rsa exists
		if (-1 == dir_exists) {
			char user_choice;
			printf("Directory exists. Generate new key pair? [y|n] ");
			user_choice = getchar();
			
			if (user_choice != 'y' && user_choice != 'n') {
				printf("Aborting.\n");
				return EXIT_FAILURE;
			}
			
			if ('y' == user_choice) {
				// init
				mpz_inits(n, e, d, NULL);
				
				// generating key pair
				printf("Generating key pair...");
				generate_keypair(n, e, d);
				printf(" Done.\n");
				
				// saving
				if (-1 == save_keypair(n, e, d)) {
					mpz_clears(n, e, d, NULL);
					return EXIT_FAILURE;
				}
				
				// cleaning
				mpz_clears(n, e, d, NULL);
			}
		}
		
		// .rsa doesn't exists
		else {
			// inits
			mpz_inits(n, e, d, NULL);
			
			// generating
			printf("Generating key pair...");
			generate_keypair(n, e, d);
			printf(" Done.\n");
			
			// saving
			save_keypair(n, e, d);
			
			// cleaning
			mpz_clears(n, e, d, NULL);
		}
	}
	
	// option not recognized
	else {
		printf("Usage: %s --[decrypt, encrypt, generate-key-pair] file\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

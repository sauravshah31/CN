/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "decrypt.h"
#include <string.h>


int
decrypt_prog_1(char *host, char* inp, char* sec)
{
	printf("sending...\n");
	CLIENT *clnt;
	int  *result_1;
	input  check_hash_1_arg;
	strcpy(check_hash_1_arg.inp, inp);
	strcpy(check_hash_1_arg.secret, sec);

#ifndef	DEBUG
	clnt = clnt_create (host, DECRYPT_PROG, DECRYPT_VERS, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
#endif	/* DEBUG */

	result_1 = check_hash_1(&check_hash_1_arg, clnt);
	if (result_1 == (int *) NULL) {
		clnt_perror (clnt, "call failed");
	}
#ifndef	DEBUG
	clnt_destroy (clnt);
#endif	 /* DEBUG */
	return *result_1;
}


int
main (int argc, char *argv[])
{

	if(argc != 4){
		fprintf(stderr, "Usage: %s host input secret",argv[0]);
		return -1;
	}
	char *host;

	if (argc < 2) {
		printf ("usage: %s server_host\n", argv[0]);
		exit (1);
	}
	host = argv[1];
	int flg = decrypt_prog_1 (host, argv[2], argv[3]);
	if(flg){
		printf("correct\n");
	}else{
		printf("incorrect\n");
	}
exit (0);
}

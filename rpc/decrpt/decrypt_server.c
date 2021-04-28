/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "decrypt.h"

int *
check_hash_1_svc(input *argp, struct svc_req *rqstp)
{
	static int  result;

	/*
	 * insert server code here
	 */
	printf("request received\n");
	int flg1 = 0, flg2=0;

	for(int i=0;i< strlen(argp->inp); i++)
		flg1 ^= argp->inp[i];
	
	for(int i=0;i< strlen(argp->secret); i++)
		flg2 ^= argp->secret[i];

	if(flg1 ^ flg2)
		result = 1;
	else
		result = 0;

	return &result;
}
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


int calc_red_pos(const long int i, int *out,long int rb){
    long int tot = 0;
    memset(out, 0, rb * sizeof(int));
    for(long int j=0; j<rb;j++){
        if(i & 1<<j){
            out[j] = 1;
            tot += 1;
        }
    }

    if(tot == 1)
        return 0;

    return 1;
}

int 
main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(stderr,"usage : %s message_bit\n",argv[0]);
        return -1;
    }

    char *msg = argv[1];

    const long int mb = strlen(msg);
    long int rb = ceil(log2(mb));
    if(pow(2,rb) < (mb+rb+1)){
        rb += 1;
    }

    char encoded_msg[mb+rb+1];
    int redundant_bits[rb];
    long int red_pos[rb];
    int rcurr = 2;
    memset(redundant_bits, 0, rb*sizeof(int));

    red_pos[0] = 1;
    red_pos[1] = 2;
    int curr_pos = 0;
    int out[rb];
    for(long int i=3;i <(mb+rb+1); i++){
        if(calc_red_pos(i, out, rb)){
            encoded_msg[i] = msg[curr_pos++];
            for(long int j=0;j<rb;j++){
                redundant_bits[j] ^= (out[j] & (encoded_msg[i]-'0'));
            }
        }else{
            red_pos[rcurr++] = i;
        }
    }
    for(long int j=0;j<rb;j++){
        encoded_msg[red_pos[j]] = redundant_bits[j]+'0';
    }
    encoded_msg[mb+rb+1] = '\0';
    
    printf("original message\n");
    printf("%s\n",msg);
    printf("encoded message\n");
    printf("%s\n",encoded_msg+1);
    
    return 0;
}
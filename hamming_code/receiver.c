#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int 
main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(stderr,"usage : %s message_bit\n",argv[0]);
        return -1;
    }
    
    int n = strlen(argv[1]);
    char *encoded = argv[1];

    long int rb = ceil(log2(n));
    long int pos = 0;
    for(long int i=0; i<rb; i++){
        long int diff = 1<<i;
        long int start = diff;
        int flag = 0;
        while(start < n+1){
            long int end = start + diff;
            for(start; start< end && start < n+1; start+=1){
                flag ^= (encoded[start-1] - '0');
            }

            start += diff;
        }

        if(flag){
            pos |= 1<<i;
        }
    }

    if(pos){
        printf("error deteced at position %ld\n",pos);
        pos--;
        encoded[pos] = encoded[pos] == '0'?'1':'0';
    }


    char decoded[n];
    int curr = 0;

    for(long int i=0;i<n;i++){
        double d = log2(i+1);
        if((d - (long int)d) != 0.0){
            decoded[curr++] = encoded[i];
        }
    }
    decoded[curr] = '\0';
    printf("decoded message\n");
    printf("%s\n",decoded);



    return 0;
}
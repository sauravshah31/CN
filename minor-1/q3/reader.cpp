#include <iostream>

#include <unistd.h>
using namespace std;
int main(){
    
    char *buf[1028];
    while( read(STDIN_FILENO, buf, 1024)>0){
        cout<<"Reader Process : "<<getpid()<<endl;
        cout<<buf;
    }
}
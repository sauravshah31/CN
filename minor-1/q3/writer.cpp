#include <iostream>
#include <string>
#include <unistd.h>
using namespace std;

int main(){
    
    string buf;
    while(true){
        cout<<"Writer Process : "<<(getpid())<<endl;
        cin>>buf;
        cout<<buf;
    }
}
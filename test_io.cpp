 
#include "mpio.hpp"
#include<iostream>
#include <vector>
#include "RecIO.hpp"
#include <string>
using namespace std;


int party,port;


int main(int argc,char **argv){
    const int n=3;
    if(argc!=3){
        puts("./test_io <party> <port>");
        return 0;
    }
    sscanf(argv[1],"%d",&party);
    sscanf(argv[2],"%d",&port);

    vector<string>ip;
    for(int i=0;i<=n;i++)
        ip.push_back(string("127.0.0.1"));
    
    

    MPIO<RecIO,n> *io=new MPIO<RecIO,n>(party,ip,port);

    
    if(party==1){
        io->send_data(2,"hi2",3);
        io->send_data(3,"hi3",3);
    }
    if(party==2){
        char tmp[4];memset(tmp,0,sizeof(tmp));
        io->recv_data(1,tmp,3);
        puts(tmp);
    }
    if(party==3){
        char tmp[4];memset(tmp,0,sizeof(tmp));
        io->recv_data(1,tmp,3);
        puts(tmp);
    }
}
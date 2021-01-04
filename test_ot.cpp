#include "RecIO.hpp"
#include "mpio.hpp"
#include "mpot.hpp"
#include<iostream>
#include <vector>
#include <string>
using namespace std;


int party,port;


int main(int argc,char **argv){
    const int n=3;
    if(argc!=3){
        puts("./test_ot <party> <port>");
        return 0;
    }
    sscanf(argv[1],"%d",&party);
    sscanf(argv[2],"%d",&port);

    vector<string>ip;
    for(int i=0;i<=n;i++)
        ip.push_back(string("127.0.0.1"));
    

    /*if(party==1){
        NetIO *io=new NetIO("127.0.0.1",port);
        io->send_data("hi2",3);
    
    }
    if(party==2){
        NetIO *io=new NetIO(NULL,port);
        io->accepting();
        char tmp[4];memset(tmp,0,sizeof(tmp));
        io->recv_data(tmp,3);
        puts(tmp);
    }*/
    
    

    MPIO<RecIO,n> *io=new MPIO<RecIO,n>(party,ip,port);
    MPOT<MPIO<RecIO,n>,n> *ot=new MPOT<MPIO<RecIO,n>,n>(io);
    
    if(party==1){
        int len=8;
        bool data0[10],data1[10];
        for(int i=0;i<len;i++){
            data0[i]=rand()%2;
            data1[i]=rand()%2;
            cout<<data0[i]<<","<<data1[i]<<endl;
        }
        
        ot->send(2,data0,data1,len);
    }
    if(party==2){
        int len=8;
        bool data[10],b[10];
        for(int i=0;i<len;i++){
            b[i]=rand()%2;
        }
        ot->recv(1,data,b,len);
        for(int i=0;i<len;i++)
            cout<<data[i]<<" "<<b[i]<<endl;
        cout<<endl;
    }

}
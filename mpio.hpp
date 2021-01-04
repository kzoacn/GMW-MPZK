#ifndef MP_IO_HPP
#define MP_IO_HPP

#include "RecIO.hpp" 
#include "constant.h"
#include <vector>
#include <string>
#include "group.hpp"
using namespace std;

template<class IO,int n>
class MPIO{
public:
    int party;
    IO* send_io[n+1];
    IO* recv_io[n+1];

    MPIO(int party,vector<string> ip,int port,bool quiet=false){
        this->party=party;
        for(int i=1;i<=n;i++)
        for(int j=1;j<=n;j++)if(i!=j){
            if(i==party){
                send_io[j]=new IO(ip[j].c_str(),port+(i-1)*n+j,quiet);
            }
            if(j==party){
                recv_io[i]=new IO(NULL,port+(i-1)*n+j,quiet);
            }
        }
    }

    ~MPIO(){
        for(int i=1;i<=n;i++)if(i!=party){
            delete send_io[i];
            delete recv_io[i];
        }
    }
    void flush(){

    }
    void send_data(int i,const void *data,int len){
        send_io[i]->send_data(data,len);
        send_io[i]->flush();
    }
    void recv_data(int i,void *data,int len){
        recv_io[i]->recv_data(data,len);
    } 

    void send_Bool(int i,bool b){
        send_data(i,&b,sizeof(b));
    }
    void recv_Bool(int i,bool &b){
        recv_data(i,&b,sizeof(b));
    }

    void send_pt(int i,Point *p){
        int size=p->size();
        unsigned char *buf=new unsigned char[size+1];
        p->to_bin(buf,size);
        send_data(i,buf,size); 
        delete[] buf;
    }

    void recv_pt(int i,Group *G,Point *p){
        int size=G->get_generator().size();
        unsigned char *buf=new unsigned char[size+1]; 
        recv_data(i,buf,size); 
        p->from_bin(G,buf,size);
        delete[] buf;
    }
    
};



#endif
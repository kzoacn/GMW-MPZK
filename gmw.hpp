#ifndef _BGW__HPP
#define _BGW__HPP
 
#include "mpio.hpp"
#include <cassert>
#include "constant.h"  
#include "prg.hpp"
#include "mpot.hpp"
class Bool{
public:
    bool val;
    Bool(bool b=false):val(b){

    }
};


class MPC{
public:
    virtual void set(Bool &c,bool a,int p)=0;

    virtual void onot(Bool &c,const Bool &a)=0;
    virtual void oxor(Bool &c,const Bool &a,const Bool &b)=0;
    virtual void oand(Bool &c,const Bool &a,const Bool &b)=0;
    
    virtual bool reveal(const Bool &a)=0;
};

template<class IO,int n>
class GMW : public MPC{

public:
    int party;
    MPIO<IO,n> *io;
    PRG prng;

    MPOT<MPIO<IO,n> > *ot;

    int xor_cnt;
    int and_cnt;

    GMW(MPIO<IO,n> *io,int party){
        xor_cnt=and_cnt=0;
        this->io=io;
        this->party=party;
        ot=new MPOT<MPIO<IO,n>>(io);
    }

    ~GMW(){

    }

    void set(Bool &c,bool a,int p){
        c=share(a,p);
    }

    Bool share(bool a,int p){
        Bool c;
        if(p==0){ // constant
            if(party==1)
                c.val=a;
            else
                c.val=0;
        }else{
            if(p==party){
                bool sum=0;
                
                for(int i=1;i<=n;i++){

                    int r=rand()%2; //TODO
                    
                    
                    if(i!=party){
                        io->send_Bool(i,r);    
                        sum^=r;
                    }
                }

                c.val=sum^a;

            }else{
                io->recv_Bool(p,c.val);
            }
        }    
        return c;    
    }


    void oxor(Bool &c,const Bool &a,const Bool &b){
       c.val=a.val^b.val;
       xor_cnt++;
    }

    void onot(Bool &c,const Bool &a){
        if(party==1)
            c.val=a.val^1;
    }
    void oand(Bool &c,const Bool &a,const Bool &b){
        and_cnt++;
        
        Bool res=a.val&b.val;

/*
  r->   ------   <- b 
       |  OT  |  
 r+a->  ------   -> r+ab

*/
        for(int i=1;i<=n;i++)
        for(int j=1;j<=n;j++){
            if(i==j)continue;
            if(i==party){
                bool r=rand()%2;
                bool data0=r,data1=r^a.val;
                ot->send(j,&data0,&data1,1);
                res.val^=r;
            }
            if(j==party){
                bool data,s=b.val;
                ot->recv(i,&data,&s,1);
                res.val^=data;
            }
        }
        c=res;
    }
    

    bool reveal(const Bool &a){
        bool point[n+1];
        point[party]=a.val;

        for(int i=1;i<=n;i++)
        for(int j=1;j<=n;j++)if(i!=j){
            if(i==party){
                io->send_Bool(j,a.val);
            }
            if(j==party){
                io->recv_Bool(i,point[i]);
            }
        } 
        bool ret=0;
        for(int i=1;i<=n;i++)
            ret^=point[i];
        
        return ret;
    }

};


 
template<int n>
struct View{
    vector<Bool> inputs;
    //PRG prng;
    vector<vector<char> >trans;
    void from_bin(unsigned char *in){
        /*int size=0;
        int sz;
        memcpy(&sz,in,4);
        size+=4;
        inputs.resize(sz);
        for(int i=0;i<sz;i++){
            int sz;
            memcpy(&sz,in+size,4);
            size+=4;
            inputs[i].from_bin(in+size);
            size+=sz;    
        }

        memcpy(prng.seed,in+size,sizeof(prng.seed));
        size+=sizeof(prng.seed);
        trans.resize(n+1);
        for(int i=1;i<=n;i++){
            int sz=0;
            memcpy(&sz,in+size,4);
            size+=4;
            
            trans[i].resize(sz);
            
            memcpy(trans[i].data(),in+size,sz);
            size+=sz;
        }*/
    }
    void to_bin(unsigned char *out){
        /*int size=0;
        int sz=inputs.size();
        memcpy(out,&sz,4);
        size+=4;
        for(int i=0;i<inputs.size();i++){
            int sz=inputs[i].size();
            memcpy(out+size,&sz,4);
            size+=4; 
            inputs[i].to_bin(out+size);
            size+=sz;    
        }
        
        memcpy(out+size,prng.seed,sizeof(prng.seed));
        size+=sizeof(prng.seed);
        for(int i=1;i<=n;i++){
            int sz=trans[i].size();
            memcpy(out+size,&sz,4);
            size+=4;
            memcpy(out+size,trans[i].data(),sz);
            size+=sz;
        }*/
    }
    int size(){
        /*int size=0;
        size+=4;
        for(int i=0;i<inputs.size();i++){
            int sz=inputs[i].size();
            size+=4; 
            size+=sz;    
        }  
        size+=sizeof(prng.seed);
        for(int i=1;i<=n;i++){
            int sz=trans[i].size();
            size+=4;
            size+=sz;
        }
        return size;*/
    }
    void digest(char *out){
        /*Hash view_hash;
        unsigned char *tmp=new unsigned char[size()];
        memset(tmp,0,size());
        to_bin(tmp);
        view_hash.put(tmp,size());
        delete []tmp;
        view_hash.digest(out);*/
    }
};

bool check_perm(int *perm){
    
    int cnt[n/3];
    memset(cnt,0,sizeof(cnt));
    for(int i=1;i<=open_num;i++){
        cnt[perm[i]%3]++;
    }
    for(int i=0;i<n/3;i++)
        if(cnt[i]==3)
            return false;
    return true;
}
 
#endif
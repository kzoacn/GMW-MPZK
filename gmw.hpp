#ifndef _BGW__HPP
#define _BGW__HPP
 
#include "mpio.hpp"
#include <cassert>
#include "constant.h"  
#include "prg.hpp"
#include "mpot.hpp"
#include <deque>
class Bool{
public:
    bool val;
    Bool(bool b=0):val(b){

    }
};


template<int n>
struct View{
    vector<boolean> inputs;
    PRG prg;

    vector<vector<boolean> >trans;
    vector<int>cur;


    int mode;
    View(){
        mode=1;
        trans.resize(n+1); 
        cur.resize(n+1);
    } 
    /*View(const View &view){
        this->inputs=view.inputs;
        this->prg=view.prg;
        this->trans=view.trans;
        this->cur=view.cur;
        this->mode=view.mode;
    }*/

    void clear(){
        cur.clear();
        cur.resize(n+1);
        prg.counter=0;
    }
 
    void recv_bool(int i,bool &c){
        if(mode==1){
            trans[i].push_back(c);
        }else{
            c=trans[i][cur[i]++];
        }
    }



    void from_bin(unsigned char *in){
        int size=0;
        int sz;
        memcpy(&sz,in,4); 
        size+=4;
        inputs.resize(sz);
        for(int i=0;i<sz;i++){
            inputs[i]=*(in+size);
            size++;    
        }


        memcpy(prg.key,in+size,sizeof(prg.key));
        size+=sizeof(prg.key);
        trans.resize(n+1);
        for(int i=1;i<=n;i++){
            int sz=0;
            memcpy(&sz,in+size,4);
            size+=4;
            
            trans[i].resize(sz);
            int bytes=byte_size(trans[i]);
            from_bytes(in+size,trans[i]);
            size+=bytes;
        }
    }
    void to_bin(unsigned char *out){
        int size=0;
        int sz=inputs.size();
        memcpy(out,&sz,4); 
        size+=4;
        for(int i=0;i<inputs.size();i++){
            *(out+size)=inputs[i];
            size++;
        }
        
        memcpy(out+size,prg.key,sizeof(prg.key));
        size+=sizeof(prg.key);
        for(int i=1;i<=n;i++){
            int sz=trans[i].size();
            memcpy(out+size,&sz,4);
            size+=4;
            int bytes=byte_size(trans[i]);
            to_bytes(out+size,trans[i]);
            size+=bytes; 
        }
    }

    int byte_size(const vector<boolean> &trans){
        return (trans.size()+7)/8;
    }
    void to_bytes(unsigned char *out,const vector<boolean> &trans){
        int bytes=byte_size(trans);
        for(int i=0;i<bytes;i++){
            *(out+i)=0;
            for(int j=0;j<8&&i*8+j<(int)trans.size();j++){
                if(trans[i*8+j])
                    *(out+i)|=1<<j;
            }
        }
    }
    void from_bytes(unsigned char *in,vector<boolean>&trans){
        int bytes=byte_size(trans);
        for(int i=0;i<bytes;i++){
            for(int j=0;j<8&&i*8+j<(int)trans.size();j++){
                trans[i*8+j]=(*(in+i))>>j&1;
            }
        }
    }

    int size(){
        int size=0;
        size+=4;
        for(int i=0;i<inputs.size();i++){
            size++;    
        }  
        size+=sizeof(prg.key);
        for(int i=1;i<=n;i++){ 
            size+=4;
            int bytes=byte_size(trans[i]);
            size+=bytes;
        }
        return size;
    }
    void digest(char *out){
        Hash view_hash;
        unsigned char *tmp=new unsigned char[size()];
        memset(tmp,0,size());
        to_bin(tmp);
        view_hash.put(tmp,size());
        delete []tmp;
        view_hash.digest(out);
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
    PRG prg;

    MPOT<MPIO<IO,n> > *ot;

    int xor_cnt;
    int and_cnt; 
    View<n> view;

    GMW(MPIO<IO,n> *io,int party){
        xor_cnt=and_cnt=0;
        this->io=io;
        this->party=party;
        ot=new MPOT<MPIO<IO,n>>(io);
        view.prg=prg;
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

                    int r=prg.rand()%2; //TODO
                    
                    
                    if(i!=party){
                        io->send_Bool(i,r);    
                        sum^=r;
                    }
                }

                c.val=sum^a;

            }else{
                io->recv_Bool(p,c.val);
                view.recv_bool(p,c.val);
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
        else
            c.val=a.val;
       xor_cnt++;
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
                bool r=prg.rand()%2;
                bool data0=r,data1=r^a.val;
                ot->send(j,&data0,&data1,1);
                res.val^=r;
            }
            if(j==party){
                bool data,s=b.val;
                ot->recv(i,&data,&s,1);
                view.recv_bool(i,data);
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
                view.recv_bool(i,point[i]);
            }
        } 
        bool ret=0;
        for(int i=1;i<=n;i++)
            ret^=point[i];
        
        return ret;
    }

};




class Channel{
public:
    vector<boolean>vec;
    int cur;
    Channel(){
        cur=0;
    }

    void send(bool x){
        vec.push_back(x);
    }
    void recv(bool &x){
        x=vec[cur++];
    }
    void send_ot(bool data0,bool data1){
        vec.push_back(data0);
        vec.push_back(data1);
    }
    void recv_ot(bool &data,bool b){
        bool d[2];
        d[0]=vec[cur++];
        d[1]=vec[cur++];
        data=d[b%2];
    }
};


template<int n>
class ReGMW : public MPC{


public:
    int party;
    
    View<n> *view;
    vector<Channel*> channels;
    ReGMW(View<n> *view,vector<Channel*> channels,int party){
        this->view=view;
        this->party=party;
        this->channels=channels;
        view->mode=0;
        view->clear();
    }

    ~ReGMW(){

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

                    int r=view->prg.rand()%2; //TODO
                    
                    
                    if(i!=party){
                        //io->send_Bool(i,r); 
                        channels[i]->send(r);   
                        sum^=r;
                    }
                }

                c.val=sum^a;

            }else{
                //io->recv_Bool(p,c.val);
                view->recv_bool(p,c.val);
            }
        }    
        return c;    
    }


    void oxor(Bool &c,const Bool &a,const Bool &b){
       c.val=a.val^b.val; 
    }

    void onot(Bool &c,const Bool &a){
        if(party==1)
            c.val=a.val^1;
        else
            c.val=a.val;
    }
    void oand(Bool &c,const Bool &a,const Bool &b){ 
        Bool res=a.val&b.val; 
        for(int i=1;i<=n;i++)
        for(int j=1;j<=n;j++){
            if(i==j)continue;
            if(i==party){
                bool r=view->prg.rand()%2;
                bool data0=r,data1=r^a.val;
                //ot->send(j,&data0,&data1,1);
                channels[j]->send_ot(data0,data1);
                res.val^=r;
            }
            if(j==party){
                bool data,s=b.val;
                //ot->recv(i,&data,&s,1);
                view->recv_bool(i,data);
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
                //io->send_Bool(j,a.val);
                channels[j]->send(a.val);
            }
            if(j==party){
                //io->recv_Bool(i,point[i]);
                view->recv_bool(i,point[i]);
            }
        } 
        bool ret=0;
        for(int i=1;i<=n;i++)
            ret^=point[i];
        
        return ret;    
    }


};



template<int n>
class FinalGMW : public MPC{


public:
    int party;
    
    View<n> *view;
    vector<Channel*> channels;
    vector<boolean> open;
    FinalGMW(View<n> *view,vector<Channel*> channels,vector<boolean> open,int party){
        this->view=view;
        this->party=party;
        this->channels=channels;
        this->open=open;
        view->mode=0;
        view->clear();
    }

    ~FinalGMW(){

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

                    int r=view->prg.rand()%2; //TODO
                    
                    
                    if(i!=party){
                        //io->send_Bool(i,r); 
                        sum^=r;
                    }
                }

                c.val=sum^a;

            }else{
                //io->recv_Bool(p,c.val);
                view->recv_bool(p,c.val);
                if(open[p]){
                    bool tmp;
                    channels[p]->recv(tmp);
                    if(tmp!=c.val)perror("inconsistent share");
                }
            }
        }    
        return c;    
    }


    void oxor(Bool &c,const Bool &a,const Bool &b){
       c.val=a.val^b.val; 
    }

    void onot(Bool &c,const Bool &a){
        if(party==1)
            c.val=a.val^1;
        else
            c.val=a.val;
    }
    void oand(Bool &c,const Bool &a,const Bool &b){ 
        Bool res=a.val&b.val; 
        for(int i=1;i<=n;i++)
        for(int j=1;j<=n;j++){
            if(i==j)continue;
            if(i==party){
                bool r=view->prg.rand()%2;
                bool data0=r,data1=r^a.val;
                //ot->send(j,&data0,&data1,1);
                res.val^=r;
            }
            if(j==party){
                bool data,s=b.val;
                //ot->recv(i,&data,&s,1);
                view->recv_bool(i,data);
                

                if(open[i]){
                    bool tmp;
                    channels[i]->recv_ot(tmp,s);
                    if(tmp!=data)perror("inconsistent and");
                }
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
                //io->send_Bool(j,a.val);
            }
            if(j==party){
                //io->recv_Bool(i,point[i]);
                view->recv_bool(i,point[i]);

                if(open[i]){
                    bool tmp;
                    channels[i]->recv(tmp);
                    if(tmp!=point[i])perror("inconsistent reveal");
                }
            }
        } 
        bool ret=0;
        for(int i=1;i<=n;i++)
            ret^=point[i];
        
        return ret;    
    }


};





bool check_perm(int *perm){
    
    int cnt[n/3];
    memset(cnt,0,sizeof(cnt));
    for(int i=1;i<=open_num;i++){
        cnt[(perm[i]-1)/3]++;
    }
    for(int i=0;i<n/3;i++)
        if(cnt[i]==3)
            return false;
    return true;
}
 
#endif
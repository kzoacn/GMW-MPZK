#include "RecIO.hpp" 
#include "gmw.hpp"
#include <iostream>
#include <vector>
#include "constant.h"
#include "program.hpp"

using namespace std; 

int party,port;



int main(int argc,char **argv){
    if(argc!=3){
        puts("./main <party> <port>");
        return 0;
    }
    sscanf(argv[1],"%d",&party);
    sscanf(argv[2],"%d",&port);

    vector<string>ip;
    for(int i=0;i<=n+1;i++)
        ip.push_back(string("127.0.0.1"));
/*    
    MPIO<RecIO,n> *io=new MPIO<RecIO,n>(party,ip,port,true);
    
        
        GMW<RecIO,n> *gmw=new GMW<RecIO,n>(io,party);
        vector<boolean>inputs;
        //inputs.push_back(party==2?false:true);
        for(int i=0;i<32;i++)
            inputs.push_back(party>>i&1);
        auto res=compute(party,inputs,gmw);


    int out[32];
    for(int i=0;i<32;i++)
        out[i]=gmw->reveal(res[i]);
    
    if(party==1){
        for(int i=0;i<32;i++)
            cout<<out[i]%2;
        cout<<endl;
    }
*/
    
 
    Hash view_all;
    vector<vector<char> >view_n;

    vector<vector<char> >views_hash;
    vector<View<n> >views;

    for(int it=0;it<REP;it++){
        cerr<<"proving "<<it<<endl;
        MPIO<RecIO,n> *io=new MPIO<RecIO,n>(party,ip,port,true);
        
        
        GMW<RecIO,n> *gmw=new GMW<RecIO,n>(io,party);
        vector<boolean>inputs;
        for(int i=0;i<32;i++){
            unsigned long long x=party;
            inputs.push_back(x>>i&1);
        }
        gmw->view.inputs=inputs;
        auto res=compute(party,inputs,gmw);
        vector<boolean>output;
        for(int i=0;i<(int)res.size();i++)
            output.push_back(gmw->reveal(res[i]));
        if(party==1){
            for(int i=0;i<(int)output.size();i++)
                cout<<output[i];
            cout<<endl;
        }

        views.push_back(gmw->view);
        views_hash.push_back(vector<char>());
        views_hash[it].resize(Hash::DIGEST_SIZE);
        views[it].digest(views_hash[it].data());
        
        view_all.put(views_hash[it].data(),views_hash[it].size());

        delete io;
        delete gmw;
    }
 


    view_n.resize(n+1);
    for(int i=1;i<=n;i++)
        view_n[i].resize(Hash::DIGEST_SIZE);
    view_all.digest(view_n[party].data());
    
    MPIO<RecIO,n> *io=new MPIO<RecIO,n>(party,ip,port);
    for(int i=1;i<=n;i++)
    for(int j=1;j<=n;j++)if(i!=j){
        if(i==party){
            io->send_data(j,view_n[i].data(),view_n[i].size());
        
        }
        if(j==party){
            io->recv_data(i,view_n[i].data(),view_n[i].size());
        }
    }


    delete io;

    view_all.reset();
    for(int i=1;i<=n;i++)
        view_all.put(view_n[i].data(),view_n[i].size());
    
    char r[Hash::DIGEST_SIZE];
    view_all.digest(r);
    PRG prg;
    prg.reseed((unsigned char*)r);
    static int perm[n+1];
    for(int i=1;i<=n;i++)
        perm[i]=i;

    

    string name="view_"+to_string(party)+".bin";
    FILE *fp=fopen(name.c_str(),"wb");
    for(int it=0;it<REP;it++){
        fwrite(views_hash[it].data(),1,views_hash[it].size(),fp);
    }


    for(int it=0;it<REP;it++){
        do{
            for(int i=2;i<=n;i++){
                int x=prg.rand()%(i-1)+1;//TODO
                swap(perm[i],perm[x]);
            }
        }while(!check_perm(perm));
        

        for(int i=1;i<=open_num;i++){ 
            if(party==perm[i]){
                 //printf("view size %d\n",(int)views[it].inputs.size());
                int size=views[it].size();
                //cout<<party<<" "<<size<<endl;
                unsigned char *tmp=new unsigned char[size];
                views[it].to_bin(tmp);
                fwrite(&size,1,4,fp);
                fwrite(tmp,1,size,fp);
            }
        }
    }
    fclose(fp);
 

    return 0;
}
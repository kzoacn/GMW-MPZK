#include "RecIO.hpp" 
#include "gmw.hpp"
#include <iostream>
#include <vector>
#include "constant.h"
#include "program.hpp"

using namespace std;

 
bool verify(){

    FILE *fp[n+1];
    for(int i=1;i<=n;i++){
        string name="view_"+to_string(i)+".bin";
        fp[i]=fopen(name.c_str(),"rb");
    }
    
    vector<vector<char> >view_n;
    vector<vector<vector<char> > >views_hash;


    view_n.resize(n+1);
    views_hash.resize(n+1);
    for(int i=1;i<=n;i++){
        Hash view_all;
        views_hash[i].resize(REP);
        for(int it=0;it<REP;it++){
            views_hash[i][it].resize(Hash::DIGEST_SIZE);

            fread(views_hash[i][it].data(),1,Hash::DIGEST_SIZE,fp[i]);
    
            view_all.put(views_hash[i][it].data(),Hash::DIGEST_SIZE);
        }
        view_n[i].resize(Hash::DIGEST_SIZE);
        view_all.digest(view_n[i].data());
    }

    Hash view_all;
    for(int i=1;i<=n;i++)
        view_all.put(view_n[i].data(),view_n[i].size());
    
    char r[Hash::DIGEST_SIZE];
    view_all.digest(r);
    PRG prg;
    prg.reseed((unsigned char*)r);
    static int perm[n+1];
    for(int i=1;i<=n;i++)
        perm[i]=i;

    
    for(int it=0;it<REP;it++){
        cerr<<"checking "<<it<<endl;
        do{
            for(int i=2;i<=n;i++){
                int x=prg.rand()%(i-1)+1;
                swap(perm[i],perm[x]);
            }
        }while(!check_perm(perm));
        
        vector<View<n> >views;
        views.resize(n+1);
        for(int i=1;i<=open_num;i++){
            int x=perm[i];
            static unsigned char tmp[MAX_SIZE];
            int size;
            fread(&size,1,4,fp[x]);
            fread(tmp,1,size,fp[x]);
            views[x].from_bin(tmp);
        }


        vector<vector<Channel> >all_channel;
        all_channel.resize(n+1);
        for(auto &vec:all_channel)
            vec.resize(n+1);

        for(int i=1;i<=open_num;i++){
            int x=perm[i];
            vector<Channel*>channels;
            for(int j=0;j<=n;j++)
                channels.push_back(&all_channel[x][j]);

            ReGMW<n> *gmw=new ReGMW<n>(&views[x],channels,x);
            auto res=compute(x,views[x].inputs,gmw);
            vector<boolean>output;
            for(int i=0;i<(int)res.size();i++)
                output.push_back(gmw->reveal(res[i]));
            delete gmw;
        }

        vector<boolean>open;
        open.resize(n+1);
        for(int i=1;i<=open_num;i++)
            open[perm[i]]=1;
        
        for(int i=1;i<=open_num;i++){
            int x=perm[i];
            vector<Channel*>channels;
            for(int j=0;j<=n;j++)
                channels.push_back(&all_channel[j][x]);

            FinalGMW<n> *gmw=new FinalGMW<n>(&views[x],channels,open,x);
            auto res=compute(x,views[x].inputs,gmw);
            vector<boolean>output;
            for(int i=0;i<(int)res.size();i++)
                output.push_back(gmw->reveal(res[i]));
            

            // check output
            if(it+1==REP){
                for(int i=0;i<(int)output.size();i++)
                    cout<<(int)output[i];
                cout<<endl;
            }
            
            delete gmw;
        }

    }

    for(int i=1;i<=n;i++)
        fclose(fp[i]);

    return true;
}

int main(int argc,char **argv){

    
   if(verify()){
        puts("Yes");
    }else{
        puts("No");
    }

    

    return 0;
}
#ifndef PRG_HPP__
#define PRG_HPP__

#include <memory>
#include <algorithm>
#include <random> 
#include "hash.hpp"
 
class PRG { public:
	uint64_t counter = 0;
	uint64_t key[2];
    Hash hash;

	PRG(const void * seed = nullptr) {	
		if (seed != nullptr) {
			reseed((const unsigned char *)seed);
		} else {
			unsigned int *v=(unsigned int*)key;
            std::random_device rand_div;
            for(int i=0;i<sizeof(key)/sizeof(unsigned int);i++)
                v[i]=rand_div();
			counter=0;
		}
	}
	void reseed(const unsigned char* seed) {
        memcpy(key,seed,sizeof(key));
		counter = 0;
	}

	void random_data(void *data, int nbytes) { 
        unsigned char tmp[Hash::DIGEST_SIZE];
        uint64_t ctr[3];
        memcpy(ctr,key,sizeof(key));
        int left=nbytes;
        int cur=0;
        while(left>0){
            ctr[2]=counter;
            Hash::hash(tmp,ctr,sizeof(ctr));
            memcpy(data+cur*Hash::DIGEST_SIZE,tmp,std::min(left,Hash::DIGEST_SIZE));
            counter++;    
            cur++;
            left-=Hash::DIGEST_SIZE;
        }
	}

    unsigned int rand(){
        unsigned int res;
        random_data(&res,sizeof(res));
        return res;
    } 
};

#endif// PRP_H__
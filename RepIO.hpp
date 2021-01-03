#ifndef REPLAY_IO_CHANNEL
#define REPLAY_IO_CHANNEL

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string> 
using std::string;
  
 

const int RECORD_MODE=0;
const int REPLAY_MODE=1;

class RepIO{ public: 

	std::vector<char>recv_rec;
	int recv_cur;

	RepIO(const char * address, int port,bool quiet = false) {
        
        recv_cur=0;

        if(!quiet)
			std::cout << "replaying\n";
	}

	void sync() {

	}

	~RepIO(){

	}

	void set_nodelay() {

	}

	void set_delay() {

	}

	void flush() {

	}
 
    
	void send_data(const void * data, int len) {
		//send_hash.put(data,len);
	}

	void recv_data(void  * data, int len) {
		if(recv_cur+len>(int)recv_rec.size())
			perror("RepIO recv_data error!\n");
        //recv_hash.put(data,len);
		for(int i=0;i<len;i++){
			((unsigned char*)data)[i]=(unsigned char)recv_rec[recv_cur+i];
		}
		recv_cur+=len;
	}
}; 
 
#endif  //NETWORK_IO_CHANNEL

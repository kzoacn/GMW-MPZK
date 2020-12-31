#ifndef IO_CHANNEL_H__
#define IO_CHANNEL_H__

#ifdef WIN32
 #error "can not use it on windows"
#endif

#include <string> 
const int NETWORK_BUFFER_SIZE=65536;


template<typename T>
class IOChannel {
public:
	void send_data(const void * data, int nbyte) {
		derived().send_data(data, nbyte);
	}
	void recv_data(void * data, int nbyte) {
		derived().recv_data(data, nbyte);
	}
	
	void send_string(const std::string &j) {
		int size=j.length();
		derived().send_data(&size, 4);
		derived().send_data(j.c_str(), size);
	}
	void recv_string(std::string &j) {
		int size;
		derived().recv_data(&size, 4);
		char *str=new char[size+1];
		str[size]=0;
		derived().recv_data(str, size);
		j=std::string(str);
		delete[] str;
	}
  

private:
	T& derived() {
		return *static_cast<T*>(this);
	}
};


#endif// IO_CHANNEL_H__


#ifndef NETWORK_IO_CHANNEL
#define NETWORK_IO_CHANNEL

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string> 
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
using std::string;


class NetIO: public IOChannel<NetIO> { 
public:
	bool is_server;
	sockaddr_in addr;
	int port;
	uint64_t counter = 0;
	char * buffer = NULL;
	int buffer_ptr = 0;
	int buffer_cap = NETWORK_BUFFER_SIZE;
	bool has_send = false; 
	int sock;
	int client_sock;
	sockaddr client_addr;
	NetIO(const char * address, int port, bool quiet = false) {
		
		is_server=(address==NULL);
		this->port = port;
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		memset(&addr, 0, sizeof(addr));  

		
		addr.sin_family = AF_INET;  
		addr.sin_addr.s_addr = inet_addr(is_server? "0.0.0.0" : address);
		addr.sin_port = htons(port);	
		bind(sock, (sockaddr*)&addr, sizeof(addr));
		
		if(is_server){
			listen(sock,SOMAXCONN);
			socklen_t addrlen;
			client_sock=accept(sock,&client_addr,&addrlen);
		}else{
			if(connect(sock,(sockaddr*)&addr,sizeof(addr))<0)
				puts("connect error");
		} 
	}
	void sync() {
		int tmp = 0;
		if(is_server) {
			send_data(&tmp, 1);
			recv_data(&tmp, 1);
		} else {
			recv_data(&tmp, 1);
			send_data(&tmp, 1);
			flush();
		}
	}

	~NetIO() {
		flush();
		delete[] buffer;
	}

	void set_nodelay() { 
	}

	void set_delay() { 
	}

	void flush() { 
	}

	void send_data(const void * data, int len) {

		if(is_server){
			write(client_sock,data,len);
		}else{
			write(sock,data,len);
		} 
	}

	void recv_data(void  * data, int len) {
		if(is_server){
			read(client_sock,data,len);
		}else{
			read(sock,data,len);
		} 
	}
};



#endif  //NETWORK_IO_CHANNEL
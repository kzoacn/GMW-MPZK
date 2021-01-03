#ifndef MP_OT_HPP
#define MP_OT_HPP

#include <vector>
#include <string> 
#include "group.hpp"
#include "hash.hpp"

template<class MIO,int n>
class MPOT{
public:
	MIO *io;

	Group *G = nullptr; 
    MPOT(MIO *io){
		this->io=io;
		G = new Group();
    }
 
	~MPOT() {
		delete G;
	}

	void send(int id,const bool *data0, const bool *data1,int length) {
		BigInt a;
		Point A, AaInv;
		unsigned char res[2][Hash::DIGEST_SIZE];
		Point * B = new Point[length];
		Point * BA = new Point[length];

		G->get_rand_bn(a);
		A = G->mul_gen(a);
		io->send_pt(id,&A);
		AaInv = A.mul(a);
		AaInv = AaInv.inv();

		for(int i = 0; i < length; ++i) {
			io->recv_pt(id,G, &B[i]);
			B[i] = B[i].mul(a);
			BA[i] = B[i].add(AaInv);
		}
		io->flush();
		for(int i=0;i<length;i++){
			Hash::KDF(&res[0][0],B[i]);
			res[0][0]^=data0[i];
			Hash::KDF(&res[1][0],BA[i]);
			res[1][0]^=data1[i];


			io->send_data(id,res[0], sizeof(res[0]));
			io->send_data(id,res[1], sizeof(res[1]));
		}
		io->flush();

		delete[] BA;
		delete[] B;
	}

	void recv(int id,bool *data, bool *b,int length) {
		BigInt * bb = new BigInt[length];
		Point * B = new Point[length],
				* As = new Point[length],
				A;

		for(int i = 0; i < length; ++i)
			G->get_rand_bn(bb[i]);

		io->recv_pt(id,G, &A);

		for(int i = 0; i < length; ++i) {
			B[i] = G->mul_gen(bb[i]);
			if (b[i]) 
				B[i] = B[i].add(A);
			io->send_pt(id,&B[i]);
		}
		io->flush();


		for(int i = 0; i < length; ++i)
			As[i] = A.mul(bb[i]);

		for(int i=0;i<length;i++){
			unsigned char res[2][Hash::DIGEST_SIZE];
			unsigned char tmp[Hash::DIGEST_SIZE];

			io->recv_data(id,res[0], sizeof(res[0]));
			io->recv_data(id,res[1], sizeof(res[1]));

			Hash::KDF(tmp,As[i]);

			if(b[i])
				data[i] = tmp[0] ^ res[1][0];
			else
				data[i] = tmp[0] ^ res[0][0];
		}
		delete[] bb;
		delete[] B;
		delete[] As;
	}
	

	 /*void 
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
   */ 
};



#endif
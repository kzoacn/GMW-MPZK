#ifndef EMP_IKNP_H__
#define EMP_IKNP_H__
#include "emp-ot/cot.h"
#include "emp-ot/co.h"

namespace emp {

/*
 * IKNP OT Extension
 * [REF] Implementation of "Extending oblivious transfers efficiently"
 * https://www.iacr.org/archive/crypto2003/27290145/27290145.pdf
 *
 * [REF] With optimization of "More Efficient Oblivious Transfer and Extensions for Faster Secure Computation"
 * https://eprint.iacr.org/2013/552.pdf
 * [REF] With optimization of "Better Concrete Security for Half-Gates Garbling (in the Multi-Instance Setting)"
 * https://eprint.iacr.org/2019/1168.pdf
 */
template<typename T>
class OTExtension: public COT<T> { public:
	using COT<T>::io;
	using COT<T>::Delta;

	OTCO<T> * base_ot = nullptr;
	bool setup = false, *extended_r = nullptr;

	const static int block_size = 1024*2;
	block local_out[block_size];
	bool s[128], local_r[256];
	PRG prg, G0[128], G1[128];
	bool malicious = false;
	block k0[128], k1[128]; 
	OTExtension(T * io, bool malicious = false): malicious(malicious) {
		this->io = io;
	}
	~OTExtension() {
		delete_array_null(extended_r);
	}

	void setup_send(const bool* in_s = nullptr, block * in_k0 = nullptr) {
		setup = true;
		if(in_s == nullptr)
			prg.random_bool(s, 128);
		else 
			memcpy(s, in_s, 128);
		
		if(in_k0 != nullptr) {
			memcpy(k0, in_k0, 128*sizeof(block));
		} else {
			this->base_ot = new OTCO<T>(io);
			base_ot->recv(k0, s, 128);
			delete base_ot;
		}
		for(int i = 0; i < 128; ++i)
			G0[i].reseed(&k0[i]);

		Delta = bool_to_block(s);
	}

	void setup_recv(block * in_k0 = nullptr, block * in_k1 =nullptr) {
		setup = true;
		if(in_k0 !=nullptr) {
			memcpy(k0, in_k0, 128*sizeof(block));
			memcpy(k1, in_k1, 128*sizeof(block));
		} else {
			this->base_ot = new OTCO<T>(io);
			prg.random_block(k0, 128);
			prg.random_block(k1, 128);
			base_ot->send(k0, k1, 128);
			delete base_ot;
		}
		for(int i = 0; i < 128; ++i) {
			G0[i].reseed(&k0[i]);
			G1[i].reseed(&k1[i]);
		}
	}
	void send_pre(block * out, int length) {
		if(!setup)
			setup_send();
		int j = 0;
		for (; j < length/block_size; ++j)
			send_pre_block(out + j*block_size, block_size);
		int remain = length % block_size;
		if (remain > 0) {
			send_pre_block(local_out, remain);
			memcpy(out+j*block_size, local_out, sizeof(block)*remain);
		} 
	}

	void send_pre_block(block * out, int len) {
		block t[block_size];
		block tmp[block_size];
		int local_block_size = (len+127)/128*128;
		io->recv_block(tmp, local_block_size);
		for(int i = 0; i < 128; ++i) {
			G0[i].random_data(t+(i*block_size/128), local_block_size/8);
			if (s[i])
				xorBlocks_arr(t+(i*block_size/128), t+(i*block_size/128), tmp+(i*local_block_size/128), local_block_size/128);
		}
		sse_trans((uint8_t *)(out), (uint8_t*)t, 128, block_size);
	}

	void recv_pre(block * out, const bool* r, int length) {
		if(not setup)
			setup_recv();

		block *block_r = new block[(length+127)/128];
		for(int i = 0; i < (length+127)/128; ++i)
			block_r[i] = bool_to_block(r+i*128);
		
		int j = 0;
		for (; j < length/block_size; ++j)
			recv_pre_block(out+j*block_size, block_r + (j*block_size/128), block_size);
		int remain = length % block_size;
		if (remain > 0) {
			recv_pre_block(local_out, block_r + (j*block_size/128), remain);
			memcpy(out+j*block_size, local_out, sizeof(block)*remain);
		} 
		delete[] block_r;
	}
	void recv_pre_block(block * out, block * r, int len) {
		block t[block_size];
		block tmp[block_size];
		int local_block_size = (len+127)/128 * 128;
		for(int i = 0; i < 128; ++i) {
			G0[i].random_data(t+(i*block_size/128), local_block_size/8);
			G1[i].random_data(tmp, local_block_size/8);
			xorBlocks_arr(tmp, t+(i*block_size/128), tmp, local_block_size/128);
			xorBlocks_arr(tmp, r, tmp, local_block_size/128);
			io->send_data(tmp, local_block_size/8);
		}

		sse_trans((uint8_t *)(out), (uint8_t*)t, 128, block_size);
	}

	void send_cot(block * data, int length) override{
		send_pre(data, length); 
	}
	void recv_cot(block* data, const bool * b, int length) override {
		recv_pre(data, b, length); 
	}
 
};

}//namespace
#endif
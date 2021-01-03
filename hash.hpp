#ifndef HASH_HPP__
#define HASH_HPP__

#include <openssl/sha.h>
#include <cstdio>
#include <cstring>
#include "group.hpp"
class Hash { public:
	SHA256_CTX hsh;
    static const int HASH_BUFFER_SIZE=65536;
	char buffer[HASH_BUFFER_SIZE];
	int size = 0;
	static const int DIGEST_SIZE = 32;
	Hash() {
		SHA256_Init(&hsh);
	}
	~Hash() {
	}
	void put(const void * data, int nbyte) {
		if (nbyte > HASH_BUFFER_SIZE)
			SHA256_Update(&hsh, data, nbyte);
		else if(size + nbyte < HASH_BUFFER_SIZE) {
			memcpy(buffer+size, data, nbyte);
			size+=nbyte;
		} else {
			SHA256_Update(&hsh, (char*)buffer, size);
			memcpy(buffer, data, nbyte);
			size = nbyte;
		}
	}
	void digest(char * a) {
		if(size > 0) {
			SHA256_Update(&hsh, (char*)buffer, size);
			size=0;
		}
		SHA256_Final((unsigned char *)a, &hsh);
	}
	void reset() {
		SHA256_Init(&hsh);
		size=0;
	}
	static void hash(void * digest, const void * data, int nbyte) {
		(void )SHA256((const unsigned char *)data, nbyte, (unsigned char *)digest);
	}
	/*#ifdef __x86_64__
	__attribute__((target("sse2")))
	#endif
	static block hash_for_block(const void * data, int nbyte) {
		char digest[DIGEST_SIZE];
		hash_once(digest, data, nbyte);
		return _mm_load_si128((__m128i*)&digest[0]);
	}*/

	static void KDF(unsigned char *out, Point &in) {
		size_t len = in.size();
		in.group->resize_scratch(len);
		unsigned char * tmp = in.group->scratch;
		in.to_bin(tmp, len);
        hash(out,tmp,len);
	}
};

#endif// HASH_H__
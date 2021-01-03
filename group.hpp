#ifndef EMP_GROUP_H__
#define EMP_GROUP_H__

#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <string>
#include <cstring> 

using std::string;


BN_CTX *default_ctx=BN_CTX_new();

class BigInt { public:
	BIGNUM *n = nullptr;
	BigInt();
	BigInt(const BigInt &oth);
	BigInt(unsigned long long oth);
	BigInt &operator=(BigInt oth);
	~BigInt();

	int size()const;
	void to_bin(unsigned char * in)const;
	void from_bin(const unsigned char * in, int length);
    void from_dec(const char *str);
    void from_hex(const char *str);
    void from_ulong(unsigned long long x);
	void print()const;

	BigInt add(const BigInt &oth);
	BigInt sub(const BigInt &oth);
	BigInt mul(const BigInt &oth, BN_CTX *ctx = default_ctx);
	BigInt mod(const BigInt &oth, BN_CTX *ctx = default_ctx);
	BigInt add_mod(const BigInt & b, const BigInt& m, BN_CTX *ctx = default_ctx);
	BigInt sub_mod(const BigInt & b, const BigInt& m, BN_CTX *ctx = default_ctx);
	BigInt mul_mod(const BigInt & b, const BigInt& m, BN_CTX *ctx = default_ctx);
	BigInt pow_mod(const BigInt &k, const BigInt& m, BN_CTX *ctx = default_ctx);
	BigInt inv_mod(const BigInt& m, BN_CTX *ctx = default_ctx);
};	


inline BigInt::BigInt() {
	n = BN_new();
}
inline BigInt::BigInt(const BigInt &oth) {
	n = BN_new();
	BN_copy(n, oth.n);
}
inline BigInt::BigInt(unsigned long long oth) {
	from_ulong(oth);
}
inline BigInt& BigInt::operator=(BigInt oth) {
	std::swap(n, oth.n);
	return *this;
}
inline BigInt::~BigInt() {
	if (n != nullptr)
		BN_free(n);
}

inline int BigInt::size() const{
	return BN_num_bytes(n);
}

inline void BigInt::to_bin(unsigned char * in) const{
	BN_bn2bin(n, in);
}

inline void BigInt::from_bin(const unsigned char * in, int length) {
	BN_free(n);
	n = BN_bin2bn(in, length, nullptr);
}
inline void BigInt::from_dec(const char *str) {
	BN_free(n);
	n=NULL;
    BN_dec2bn(&n,str);
}
inline void BigInt::from_hex(const char *str) {
	BN_free(n);
	n=NULL;
    BN_hex2bn(&n,str);
}
inline void BigInt::from_ulong(unsigned long long x) {
	string s=std::to_string(x);
    from_dec(s.c_str());
}

inline void BigInt::print() const{// memory leak ?
	puts(BN_bn2dec(n));
}

inline BigInt BigInt::add(const BigInt &oth) {
	BigInt ret;
	BN_add(ret.n, n, oth.n);
	return ret;
}
inline BigInt BigInt::sub(const BigInt &oth) {
	BigInt ret;
	BN_sub(ret.n, n, oth.n);
	return ret;
}

inline BigInt BigInt::mul_mod(const BigInt & b, const BigInt &m,  BN_CTX *ctx) {
	BigInt ret;
	BN_mod_mul(ret.n, n, b.n, m.n, ctx);
	return ret;
}

inline BigInt BigInt::add_mod(const BigInt & b, const BigInt &m,  BN_CTX *ctx) {
	BigInt ret;
	BN_mod_add(ret.n, n, b.n, m.n, ctx);
	return ret;
}
inline BigInt BigInt::sub_mod(const BigInt & b, const BigInt &m,  BN_CTX *ctx) {
	BigInt ret;
	BN_mod_sub(ret.n, n, b.n, m.n, ctx);
	return ret;
}
inline BigInt BigInt::pow_mod(const BigInt & b, const BigInt &m,  BN_CTX *ctx) {
	BigInt ret;
	BN_mod_exp(ret.n,n,b.n,m.n,ctx);
	return ret;
}

inline BigInt BigInt::inv_mod(const BigInt &m,  BN_CTX *ctx) {
	BigInt ret;
	//BN_mod_exp(ret.n,n,b.n,m.n,ctx);
	BN_mod_inverse(ret.n,n,m.n,ctx);
	return ret;
}

inline BigInt BigInt::mul(const BigInt &oth, BN_CTX *ctx) {
	BigInt ret;
	BN_mul(ret.n, n, oth.n, ctx);
	return ret;
}

inline BigInt BigInt::mod(const BigInt &oth, BN_CTX *ctx) {
	BigInt ret;
	BN_mod(ret.n, n, oth.n, ctx);
	return ret;
}


class Group;
class Point {
	public:
		EC_POINT *point = nullptr;
		Group * group = nullptr;
		Point (Group * g = nullptr);
		~Point();
		Point(const Point & p);
		Point& operator=(Point p);

		void to_bin(unsigned char * buf, size_t buf_len)const;
		size_t size()const;
		void from_bin(Group * g, const unsigned char * buf, size_t buf_len);

		Point add(Point & rhs);
//		Point sub(Point & rhs);
//		bool is_at_infinity();
//		bool is_on_curve();
		Point mul(const BigInt &m);
		Point inv();
		bool operator==(Point & rhs);
};

class Group { public:
	EC_GROUP *ec_group = nullptr;
	BN_CTX * bn_ctx = nullptr;
	BigInt order;
	unsigned char * scratch;
	size_t scratch_size = 256;
	Group();
	~Group();
	void resize_scratch(size_t size);
	void get_rand_bn(BigInt & n);
	Point get_generator();
	Point mul_gen(const BigInt &m);
};
 

inline Point::Point (Group * g) {
	if (g == nullptr) return;
	this->group = g;
	point = EC_POINT_new(group->ec_group);
}

inline Point::~Point() {
	if(point != nullptr)
		EC_POINT_free(point);
}

inline Point::Point(const Point & p) {
	if (p.group == nullptr) return;
	this->group = p.group;
	point = EC_POINT_new(group->ec_group);
	int ret = EC_POINT_copy(point, p.point);
	if(ret == 0) perror("ECC COPY");
}

inline Point& Point::operator=(Point p) {
	std::swap(p.point, point);
	std::swap(p.group, group);
	return *this;
}

inline void Point::to_bin(unsigned char * buf, size_t buf_len) const{
	int ret = EC_POINT_point2oct(group->ec_group, point, POINT_CONVERSION_UNCOMPRESSED, buf, buf_len, group->bn_ctx);
	if(ret == 0) perror("ECC TO_BIN");
}

inline size_t Point::size() const{
	size_t ret = EC_POINT_point2oct(group->ec_group, point, POINT_CONVERSION_UNCOMPRESSED, NULL, 0, group->bn_ctx);
	if(ret == 0) perror("ECC SIZE_BIN");
	return ret;
}

inline void Point::from_bin(Group * g, const unsigned char * buf, size_t buf_len) {
	if (point == nullptr) {
		group = g;
		point = EC_POINT_new(group->ec_group);
	}
	int ret = EC_POINT_oct2point(group->ec_group, point, buf, buf_len, group->bn_ctx);
	if(ret == 0) perror("ECC FROM_BIN");
}

inline Point Point::add(Point & rhs) {
	Point ret(group);
	int res = EC_POINT_add(group->ec_group, ret.point, point, rhs.point, group->bn_ctx);
	if(res == 0) perror("ECC ADD");
	return ret;
}

inline Point Point::mul(const BigInt &m) {
	Point ret (group);
	int res = EC_POINT_mul(group->ec_group, ret.point, NULL, point, m.n, group->bn_ctx);
	if(res == 0) perror("ECC MUL");
	return ret;
}

inline Point Point::inv() {
	Point ret (*this);
	int res = EC_POINT_invert(group->ec_group, ret.point, group->bn_ctx);
	if(res == 0) perror("ECC INV");
	return ret;
}
inline bool Point::operator==(Point & rhs) {
	int ret = EC_POINT_cmp(group->ec_group, point, rhs.point, group->bn_ctx);
	if(ret == -1) perror("ECC CMP");
	return (ret == 0);
}


inline Group::Group() {
	ec_group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);//NIST P-256
	bn_ctx = BN_CTX_new();
	EC_GROUP_precompute_mult(ec_group, bn_ctx);
	EC_GROUP_get_order(ec_group, order.n, bn_ctx);
	scratch = new unsigned char[scratch_size];
}

inline Group::~Group(){
	if(ec_group != nullptr)
		EC_GROUP_free(ec_group);

	if(bn_ctx != nullptr)
		BN_CTX_free(bn_ctx);

	if(scratch != nullptr)
		delete[] scratch;
}

inline void Group::resize_scratch(size_t size) {
	if (size > scratch_size) {
		delete[] scratch;
		scratch_size = size;
		scratch = new unsigned char[scratch_size];
	}
}

inline void Group::get_rand_bn(BigInt & n) {
	BN_rand_range(n.n, order.n);
}

inline Point Group::get_generator() {
	Point res(this);
	int ret = EC_POINT_copy(res.point, EC_GROUP_get0_generator(ec_group));
	if(ret == 0) perror("ECC GEN");
	return res;
}

inline Point Group::mul_gen(const BigInt &m) {
	Point res(this);
	int ret = EC_POINT_mul(ec_group, res.point, m.n ,NULL, NULL, bn_ctx);
	if(ret == 0) perror("ECC GEN MUL");
	return res;
}



#endif

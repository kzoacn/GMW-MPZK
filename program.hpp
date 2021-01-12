#ifndef PROGRAM_HPP_
#define PROGRAM_HPP_

#include "gmw.hpp"
#include <iostream>
#include <vector>
#include "constant.h" 

using namespace std;


//#define HASH

//#define YAO

#define AVG

vector<Bool> adder(vector<Bool> a,vector<Bool> b,MPC *gmw){
    vector<Bool>res;
    assert(a.size()==b.size());
    Bool c;
    gmw->set(c,0,0);
    for(int i=0;i<a.size();i++){
        Bool p,q,r,t1,t2;

        gmw->oxor(t1,a[i],c);
        gmw->oxor(t2,b[i],c);
        gmw->oxor(r,t1,b[i]);

        gmw->oand(t1,t1,t2);
        gmw->oxor(c,c,t1);
 

        res.push_back(r);
    }
    return res;
}
vector<Bool> neg(vector<Bool> a,MPC *gmw){
    vector<Bool>c,one;
    c.resize(a.size());
    Bool zero;
    gmw->set(zero,0,0);
    for(int i=0;i<a.size();i++){
        gmw->onot(c[i],a[i]);
        one.push_back(zero);    
    }
    gmw->set(one[0],1,0);
    c=adder(c,one,gmw);
    return c;
}


vector<Bool> suber(vector<Bool> a,vector<Bool> b,MPC *gmw){
    vector<Bool>b2;
    b2=neg(b,gmw);
    auto res=adder(a,b2,gmw);
    return res;
}
Bool less_than(vector<Bool> a,vector<Bool> b,MPC *gmw){
    auto res=suber(a,b,gmw);
    return res[res.size()-1];
}


const int AND_GATE=0;
const int XOR_GATE=1;
const int NOT_GATE=2;
class Circuit { public:
	int num_gate, num_wire, n1, n2, n3;
	vector<int> gates;
	vector<Bool> wires;
	Circuit(const char * file) {
		int tmp;
		FILE * f = fopen(file, "r");
		(void)fscanf(f, "%d%d\n", &num_gate, &num_wire);
		(void)fscanf(f, "%d%d%d\n", &n1, &n2, &n3);
		(void)fscanf(f, "\n");
		char str[10];
		gates.resize(num_gate*4);
		wires.resize(num_wire);
		for(int i = 0; i < num_gate; ++i) {
			(void)fscanf(f, "%d", &tmp);
			if (tmp == 2) {
				(void)fscanf(f, "%d%d%d%d%s", &tmp, &gates[4*i], &gates[4*i+1], &gates[4*i+2], str);
				if (str[0] == 'A') gates[4*i+3] = AND_GATE;
				else if (str[0] == 'X') gates[4*i+3] = XOR_GATE;
			}
			else if (tmp == 1) {
				(void)fscanf(f, "%d%d%d%s", &tmp, &gates[4*i], &gates[4*i+2], str);
				gates[4*i+3] = NOT_GATE;
			}
		}
		fclose(f);
	}

	void compute(Bool *out, Bool *in1, Bool *in2,MPC *mpc) {
		memcpy(wires.data(), in1, n1*sizeof(Bool));
		memcpy(wires.data()+n1, in2, n2*sizeof(Bool));
		for(int i = 0; i < num_gate; ++i) {
            if(i%10000==0)printf("%d / %d\n",i,num_gate);
			if(gates[4*i+3] == AND_GATE) {
				mpc->oand(wires[gates[4*i+2]],wires[gates[4*i]], wires[gates[4*i+1]]);
			}
			else if (gates[4*i+3] == XOR_GATE) {
				mpc->oxor(wires[gates[4*i+2]],wires[gates[4*i]], wires[gates[4*i+1]]);
			}
			else  
				mpc->onot(wires[gates[4*i+2]],wires[gates[4*i]]); 
		}
		memcpy(out, wires.data()+(num_wire-n3), n3*sizeof(Bool));
	}
};


Bool equal(vector<Bool> res,const char *s,MPC *gmw){
	Bool ans;
	gmw->set(ans,true,0);
	for(int i=0;i<res.size();i++){
		Bool tmp;
		tmp=res[i];
		if(s[i]=='0'){
			gmw->onot(tmp,res[i]);
		}
		gmw->oand(ans,ans,tmp);
	}
	return ans;
}

vector<Bool> compute(int party,vector<boolean> inputs,MPC *gmw){

#ifdef HASH
	vector<Bool>res,in;
	res.resize(256);
	in.resize(512);
	for(int i=0;i<512;i++)
		gmw->set(in[i],0,0);
	for(int j=1;j<=n;j++){
		Bool tmp;
		for(int i=0;i<inputs.size();i++){
			gmw->set(tmp,inputs[i],j);
			gmw->oxor(in[i],in[i],tmp);
		}
	}
    Circuit circuit("sha256Final.txt");
    circuit.compute(res.data(),in.data(),in.data(),gmw);
	const char output[]="1000100011010011010100110101000010110001100001000110111011001100001101001101011011010000010010100001000000110101010110101101100110101000111000010010010100101110100111010111111100111010111100010011000000011000011010110100111110101111000110101001100000110010";
	
	vector<Bool> ans;
	ans.push_back(equal(res,output,gmw));

    return ans;
#endif

#ifdef YAO
	const char output1[]="1101101001010110100110001011111000010111101110011011010001101001011000100011001101010111100110010111011110011111101111101100101010001100111001011101010010010001110000001101001001100010010000111011101011111110111110011110101000011000001101111010100111011000";
	const char output2[]="1000100011010011010100110101000010110001100001000110111011001100001101001101011011010000010010100001000000110101010110101101100110101000111000010010010100101110100111010111111100111010111100010011000000011000011010110100111110101111000110101001100000110010";
	vector<Bool>res,in;
	vector<Bool> ans;
	for(int k=0;k<2;k++){
		res.resize(256);
		in.resize(512);
		for(int i=0;i<512;i++)
			gmw->set(in[i],0,0);
		for(int j=3*k+1;j<=3*k+3;j++){
			Bool tmp;
			for(int i=0;i<inputs.size();i++){
				gmw->set(tmp,inputs[i],j);
				gmw->oxor(in[i],in[i],tmp);
			}
		}
		Circuit circuit("sha256Final.txt");
		circuit.compute(res.data(),in.data(),in.data(),gmw);

		if(k==0)
			ans.push_back(equal(res,output1,gmw));
		else
			ans.push_back(equal(res,output2,gmw));
	}


    vector<Bool>s;
    vector<Bool>bits[n+1];

    s.resize(inputs.size());
    for(int i=0;i<s.size();i++)
        gmw->set(s[i],0,0);

    for(int i=1;i<=n;i++){
        bits[i].resize(inputs.size());
        for(int j=0;j<inputs.size();j++){
            gmw->set(bits[i][j],inputs[j],i);
        }
        s=adder(s,bits[i],gmw);
    }

    vector<Bool> th;

    unsigned long long t=20;
    for(int i=0;i<(int)s.size();i++){
        Bool b;
        gmw->set(b,t>>i&1,0);
        th.push_back(b);
    }

    Bool bit;
    bit=less_than(th,s,gmw);
    ans.push_back(bit);

	return ans;

#endif


#ifdef AVG
	const char output[]="00100110000000000000000000000000";


	vector<Bool> ans;
    vector<Bool>s; 

    s.resize(inputs.size());
    for(int i=0;i<s.size();i++)
        gmw->set(s[i],0,0);


	vector<Bool>res,in; 
	for(int k=0;k<n/3;k++){ 
		in.resize(inputs.size());
		for(int i=0;i<inputs.size();i++)
			gmw->set(in[i],0,0);
		for(int j=3*k+1;j<=3*k+3;j++){
			Bool tmp;
			for(int i=0;i<inputs.size();i++){
				gmw->set(tmp,inputs[i],j);
				gmw->oxor(in[i],in[i],tmp);
			}
		}
		s=adder(s,in,gmw);
	}

	//ans=s;
	ans.push_back(equal(s,output,gmw));

	return ans;


#endif

}


#endif
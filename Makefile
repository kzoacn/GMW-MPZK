prover: prover.cpp *.hpp *.h
	g++ $< -o $@ -lcrypto -O2
verifier: verifier.cpp *.hpp *.h
	g++ $< -o $@ -lcrypto -O2
clean:
	rm prover verifier
all: prover verifier

hmm: hmm.c hmmcalc.c hmmcalc.h
	gcc -g -o hmm hmm.c simclist.c hmmcalc.c -lm
clean: 
	@rm -f ./hmm
hmm: hmm.c hmmcalc.c hmmcalc.h
	gcc -o hmm hmm.c simclist.c hmmcalc.c -lm
viterbi: viterbi.c hmmcalc.c hmmcalc.h
	gcc -o viterbi viterbi.c simclist.c hmmcalc.c -lm

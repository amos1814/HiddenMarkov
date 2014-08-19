Contents

Make hmm program using "make hmm". 
Run using ./hmm for a single iteration. 
"./hmm -v" will give the verbose output for 1 iteration
"./hmm -i" '<number of iterations>' will specify the number of iterations
"./hmm -st" will self-terminate when the sum probability reaches a local maximum.

english10k.txt	10,000 English words

input.txt	Smaller, sample input. HMM results are always consistent and predictable.

simclist.c	Used this library for manipulating lists

hmmcalc.h	Summary of functions here
hmmcalc.c	This contains the meat of the calculations for the HMM

hmm.c		Parses file and runs HMM.

graph.pdf	Graph of transitions between states, using Gnuplot 

output.txt	Results of 20 separate runs of HMM, used to make graph.pdf
------------------------------------------------------------------------------------

The algorithm and notation used is described thoroughly in "Elements of Statistical Natural Language Processing" by Christopher D. Manning and Hinrich Schütze.

Expectation Maximization:
To decide when to stop iterating, I experimented with both specifying the number of iterations, and deciding dynamically when to stop iterating. The latter was done by summing the probability of individual words across entire corpus. When this number stopped increasing, the program terminates and returns the current transition probabilities, as well as the log ratio of the emission probabilities between states. These can be seen by looking at a single instance in the output.txt file. I noticed that sometimes the sum probability decreases before increasing once again, so I set a lower limit of 10 iterations. One possible problem with this is that if sum probability continues decreasing, the data returned is not a local maximum. This can be solved by throwing away runs where this happens.

Local Maxima:
For the english language, we expect letters to be divided into states corresponding to vowels and consonants. A vowel usually follows a consonant, and a consonant usually follows a vowel. Therefore the transitions (0, 1) and (1, 0) should have higher probability. This is true almost all of the time when considering a small data set (something like "babi#, anan#" will always split into vowels and consonants, with the word boundary being somewhere in between). In the results of output.txt, this is not always the case. We get expected results half of the time, with transitions between states being more likely than those returning to the same state. Much of the time, however, the transitions from both states favor a single state, increasing as the model is iterated more and more. These unexpected results may be due to the fact that the model assigns the word boundary to a single state. I found that these undesirable results were reduced when I set initial probabilities and transition probabilities each to 50%. Transitions that are skewed one way or another tend to become more so as the model is iterated.

Fairly often, all five vowels are favored by a single state. This state usually includes the word boundary, and sometimes also includes several consonants. In almost all cases, the letters e, i, o, and u are strongly favored by one state.


Plotting results of 20 runs of HMM on English Corpus:
The plot in output.pdf shows the probability of transition from state 0 to state 1 on one axis, and the probability of transition from state 1 to state 0 on the other axis.

If we assume that one state corresponds to vowels and the other corresponds to consonants, both of these transitions should be greater than 50%, since vowels and consonants tend to alternate. They should not be too high, however, since double vowels and consonants are not uncommon. There are several points in the graph towards the top right that showcase the desired result. Most of the time, one transition probability is much higher than the other. This means that the model goes to one state and gets "stuck" there. This result suggests that the vowels and consonants are not being divided between the states as desired. This may be related to the fact that the model assigns the word boundary to one of the states, while it should be divided between the two.

Assigned to each point is the sum over the corpus of word probabilities using the results from the last iteration of the HMM. This ranges from 0.021-0.045. The magnitude of this value is not very important, because this number depends on the size of the corpus. Comparing the values between runs shows that a higher sum probability corresponds to "less desirable" results. This does not necessarily mean that those results are more likely. The sum probability has more to do with which letters are assigned to which states. 
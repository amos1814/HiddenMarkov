#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "simclist.h"
#include <time.h>

typedef struct letter_count{
  int index;
  char letter;
  int count;
} letter_count;

typedef struct letter_double{
	int letter;
	double d;
} letter_double;

typedef struct state{
  int index;
  double *transitions;
  double *emissions;
  char *letters;
} state;


// Comparator function for log probabilities of emission
int compare_log_prob(const void *el1, const void *el2);

// Stores alpha values for a given time
// NOTE: If we add new states, increase the size of this dynamically.
typedef struct alphas{
  double a1;
  double a2;
} alphas;

// Stores soft counts for a letter
typedef struct softcounts{
  char letter;
  double sc00;
  double sc01;
  double sc10;
  double sc11;
} softcounts;

// Seeker function for the following function
int seeker(const void *el, const void *key);

// Searches a letter_count struct for a letter
void search_letter_count(list_t *letters, char letter);

void print_letter_count(list_t *list);

// Gets the alphabet and letter count from a file
list_t parse_file(FILE *f);

// Simple sort 
void sort(double *array, int size);

//Randomizes the order of an array
void shuffle(char *array, int n);

// I use this function to randomize the order of the word list so that
// the model does not get biased by reading many similar words in order
void shuffle_list(list_t list, int n);

// Randomly selects initial probabilities
state *initialize_state(int state_num, list_t alphabet, int perturb);

// Gives the emission and transition probabilities of a state
void print_state(state *st, list_t alphabet);

// Returns the emission probability given a character and a state
double ep(state *st, char c);

// Initializes alpha list for time = 0.
// Assume there are two states again
list_t alpha_init(double* Pi);

// Does a single step of the alpha value calculations
list_t alpha_step(list_t alpha_list, int time, char c, state *state_0, state *state_1);

// calculates an alpha array for a given word
list_t alpha_calc(list_t init, char* word, state *state_0, state *state_1);

// Initializes beta list for time = 0.
// Assume there are two states again
list_t beta_init();

// Does a single step of the beta value calculations
list_t beta_step(list_t beta_list, int time, char c, state *state_0, state *state_1);

// calculates the array for beta
list_t beta_calc(list_t init, char* word, state *state_0, state *state_1);

// Prints all alpha and beta values
void print_alpha_beta(char* word, double* Pi, state *state_0, state *state_1, int verbose);

//Gives the probability of a word
double probability(char* word, double *Pi, state *state_0, state *state_1);

// Get the soft count for a word and a transition from i to j, where i and j are 0 or 1
double soft_count(char* word, int pos, double *Pi, state *state_0, state *state_1, int i, int j);

// Gets the first n letters of a string as a string
char* get_first_n(char* word, int n);

// Stores the total soft counts for each letter in each state
list_t sum_soft_counts(list_t alphabet, list_t words, double *Pi, state *state_0, state *state_1, int verbose);

// Performs one iteration of the HMM.
// This function returns the total probability, so that it can be used
// to terminate the loop if desired.
double iterate_hmm(list_t alphabet, list_t words, double *Pi, state *state_0, state *state_1, int verbose, int print);

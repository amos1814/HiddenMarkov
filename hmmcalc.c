#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "simclist.h"
#include "hmmcalc.h"
#include <time.h>

double sum(double* array, int size){
  double ret = 0;
  int i;
  for (i=0; i<size; i++){
    ret += array[i];
  }
}

int seeker(const void *el, const void *key){
  const letter_count* lc = (letter_count*)el;
  return (lc->letter == *(char*)key);
}

void search_letter_count(list_t *letters, char letter){
  int i = 0;
  int size = 0;
  letter_count *lc = (letter_count*)malloc(sizeof(letter_count));
  list_attributes_seeker(letters, seeker);
  lc = (letter_count*)list_seek(letters, &letter);
  if (lc != NULL){
    // printf("found %c\n", letter);
    i = lc->index;
    lc = list_extract_at(letters, i);
    lc->count++;
    list_insert_at(letters, lc, i);
  }
  else {
    // printf("not found %c\n", letter);
    lc = (letter_count*)malloc(sizeof(letter_count));
    size = list_size(letters);
    lc->index = size;
    lc->letter = letter;
    lc->count = 1;
    list_insert_at(letters, lc, list_size(letters));
  }
}

void print_letter_count(list_t *list){
  letter_count *temp = (letter_count*)malloc(sizeof(letter_count));
  int i;
  for (i=0; i<list_size(list); i++){
    temp = list_get_at(list, i);
     //printf("%c %d\n", temp->letter, temp->count);
  }
}

list_t parse_file(FILE *f){
  char c;
  list_t list;
  list_init(&list);
  while ((c = fgetc(f)) != EOF){
    if (c == '\n' || c == ' ')
      c = '#';
    // printf("looking for %c\n", c);
    search_letter_count(&list, c);
  }
  print_letter_count(&list);
  return list;
}

void sort(double *array, int size){
  int i;
  int j;
  double temp;
  for (i = 0 ; i < (size - 1); i++)
  {
    for (j = 0 ; j < (size - i - 1); j++)
    {
      if (array[j] < array[j+1])
      {
        temp       = array[j];
        array[j]   = array[j+1];
        array[j+1] = temp;
      }
    }
  }
}

// Borrowed this function from user "Asmodiel" on SOF
void shuffle(char *array, int n)
{
  if (n > 1)
  {
    size_t i;
    for (i = 0; i < n - 1; i++)
    {
      size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}


// I use this function to randomize the order of the word list so that
// the model does not get biased by reading many similar words in order
void shuffle_list(list_t list, int n){
	printf("%d\n", list_size(&list));
	if (n > 1)
	{
		int i;
		// Max word size = 32
		char* temp = (char*)malloc(32);
		for (i = 0; i < n - 1; i++){
    	  int j = i + rand() / (RAND_MAX / (n - i) + 1);
    	  temp = (char*)list_extract_at(&list, j);
    	  list_insert_at(&list, temp, n-1);
    }
  }
	printf("%d\n", list_size(&list));
}


// Randomly selects initial probabilities.
state *initialize_state(int state_num, list_t alphabet, int perturb){
  int alphabet_size = list_size(&alphabet);
  state *ret = (state*)malloc(sizeof(state));
  
  // Set transition probabilities
  // If more states are added, need to allocate more memory
  double *transitions = (double*)calloc(sizeof(double), 2);
  //transitions[0] = (((double) (rand() % 1000000))/(double)1000000);
	if (perturb){
  		transitions[0] = 0.50;
  		transitions[1] = 1 - transitions[0];
	} else {
		transitions[0] = 0.50;
		transitions[1] = 1 - transitions[0];
	}

  ret->transitions = transitions;

  // Set emission probabilities
  double *emissions = (double*)malloc(alphabet_size*sizeof(double));
  int i;
  double sum = 0;
  double random;
  for (i=0; i<alphabet_size; i++){
    // I take modulo because I don't want overflow when the sum is taken
    random = rand() % 1000000;
    emissions[i] = random;
    sum += random;
  }
  for (i=0; i<alphabet_size; i++){
    emissions[i] = emissions[i]/sum;
  }
  // Sort by highest weight first
  sort(emissions, alphabet_size);
  ret->emissions = emissions;
  
  // Randomize order of alphabet
  char* letters = (char*)malloc(alphabet_size);
  letter_count *lc = (letter_count*)malloc(sizeof(letter_count));
  for (i=0; i<alphabet_size; i++){
    lc = list_get_at(&alphabet, i);
    letters[i] = lc->letter;
  }
  
  shuffle(letters, alphabet_size);
  
  ret->letters = letters;
  ret->index = state_num;
  return ret;
}

void print_state(state *st, list_t alphabet){
  int num_letters = list_size(&alphabet);
  
  printf("Creating State %d\n", st->index);
  
  printf("Transition probabilities\n\tState\t0\t%lf\n\tState\t1\t%lf\n\n", st->transitions[0], st->transitions[1]);
  
  int i;
  letter_count *lc = (letter_count*)malloc(sizeof(letter_count));
  printf("Emission probabilities\n");
  for (i=0; i<num_letters; i++){
    lc = list_get_at(&alphabet, i);
    printf("\tLetter\t%c\t%lf\n", st->letters[i], st->emissions[i]);
  }
  printf("\tTotal: %lf", sum(st->emissions, num_letters));
  printf("\n");
}

// Returns the emission probability given a character and a state
double ep(state *st, char c){
  int i;
  for (i = 0; i < strlen(st->letters); i++){
    if (c == st->letters[i]){
      return st->emissions[i];
    }
  }
  fprintf(stderr, "Emission probability not found\n");
  exit(1);
}

// Initializes alpha list for time = 0.
// Assume there are two states again
list_t alpha_init(double* Pi){
  list_t ret;
  list_init(&ret);
  alphas *as = (alphas*)malloc(sizeof(alphas));
  int i;
  as->a1 = Pi[0];
  as->a2 = Pi[1]; 
  list_append(&ret, as);
  return ret;
}

// Does a single step of the alpha value calculations
list_t alpha_step(list_t alpha_list, int time, char c, state *state_0, state *state_1){
  list_t ret;
  list_init(&ret);
  ret = alpha_list;
  alphas *new = (alphas*)malloc(sizeof(alphas));
  alphas *temp;
  temp = (alphas*)list_get_at(&alpha_list, time);
  new->a1 = temp->a1 * state_0->transitions[0] * ep(state_0, c) + temp->a2 * state_1->transitions[0] * ep(state_1, c);
  new->a2 = temp->a1 * state_0->transitions[1] * ep(state_0, c) + temp->a2 * state_1->transitions[1] * ep(state_1, c);

  list_insert_at(&ret, new, time+1);
  return ret;
}

// calculates an alpha array for a given word
list_t alpha_calc(list_t init, char* word, state *state_0, state *state_1){
  list_t ret;
  list_init(&ret);
  ret = init;
  int length = strlen(word);
  int time;
  for (time=0; time<length; time++){
    ret = alpha_step(ret, time, word[time], state_0, state_1);
  }
  return ret;
};

// Initializes beta list for time = 0.
// Assume there are two states again
list_t beta_init(){
  list_t ret;
  list_init(&ret);
  alphas *as = (alphas*)malloc(sizeof(alphas));
  int i;
  as->a1 = (double) 1;
  as->a2 = (double) 1; 
  list_append(&ret, as);
  return ret;
}

// Does a single step of the beta value calculations
list_t beta_step(list_t beta_list, int time, char c, state *state_0, state *state_1){
  list_t ret;
  list_init(&ret);
  ret = beta_list;
  alphas *new = (alphas*)malloc(sizeof(alphas));
  alphas *temp;
  temp = (alphas*)list_get_at(&beta_list, time);
  new->a1 = temp->a1 * state_0->transitions[0] * ep(state_0, c) + temp->a2 * state_0->transitions[1] * ep(state_0, c);
  new->a2 = temp->a1 * state_1->transitions[0] * ep(state_1, c) + temp->a2 * state_1->transitions[1] * ep(state_1, c);

  list_insert_at(&ret, new, time+1);
  return ret;
}

// calculates the array for beta
list_t beta_calc(list_t init, char* word, state *state_0, state *state_1){
  list_t ret;
  list_init(&ret);
  ret = init;
  int length = strlen(word);
  int time;
  for (time=0; time<length; time++){
    ret = beta_step(ret, time, word[strlen(word)-time-1], state_0, state_1);
  }
  return ret;
};


void print_alpha_beta(char* word, double* Pi, state *state_0, state *state_1, int verbose)
{
  // Initialize alphas
  list_t alpha_array;
  list_init(&alpha_array);
  alpha_array = alpha_init(Pi);
  alpha_array = alpha_calc(alpha_array, word, state_0, state_1);
  int time;
  alphas *as;
  if (verbose){
    printf("\n\n*** word: %s ***\n\n", word);
    for (time = 0; time < 1 + strlen(word); time++){
      printf("time %d:", time);
      if (time != 0) 
        printf("'%c'\n", word[time-1]);
      else
        printf("\n");
      as = list_get_at(&alpha_array, time);
      printf("\tto state:0\n\t\tAlpha: %lf\n", as->a1);
      printf("\tto state:1\n\t\tAlpha: %lf\n", as->a2);
      printf("\tTotal at this time: %lf\n", as->a1 + as->a2);
    }
  }
  
  // Initialize betas
  list_t beta_array;
  list_init(&beta_array);
  beta_array = beta_init();
  beta_array = beta_calc(beta_array, word, state_0, state_1);
  
  // Print the totals
  printf("Alpha:\n");
  for (time = 0; time < 1 + strlen(word); time++){
  as = list_get_at(&alpha_array, time);
  	printf("Time\t%d\tState\t0:\t%lf\tState\t1:\t%lf\n", time, as->a1, as->a2);
  }
	printf("Beta:\n");
  for (time = 0; time < 1 + strlen(word); time++){
  as = list_get_at(&beta_array, strlen(word)-time);
  	printf("Time\t%d\tState\t0:\t%lf\tState\t1:\t%lf\n", time, as->a1, as->a2);
  }
}

//Gives the probability of a word
double probability(char* word, double *Pi, state *state_0, state *state_1){
	int length = strlen(word);
	list_t alpha_array;
  list_init(&alpha_array);
  alpha_array = alpha_init(Pi);
  alpha_array = alpha_calc(alpha_array, word, state_0, state_1);
  alphas *as;
  as = list_get_at(&alpha_array, length);
  return as->a1 + as->a2;
}

// Get the soft count for a word and a transition from i to j, where i and j are 0 or 1
double soft_count(char* word, int pos, double *Pi, state *state_0, state *state_1, int i, int j){
	int length = strlen(word);
	double prob = probability(word, Pi, state_0, state_1);
	list_t alpha_array;
  list_init(&alpha_array);
  alpha_array = alpha_init(Pi);
  alpha_array = alpha_calc(alpha_array, word, state_0, state_1);
  list_t beta_array;
  list_init(&beta_array);
  beta_array = beta_init();
  beta_array = beta_calc(beta_array, word, state_0, state_1);
  
  alphas *as;
  alphas *bs;
	as = list_get_at(&alpha_array, pos);
  bs = list_get_at(&beta_array, length-pos-1);
	double alpha;
	double a; 

	double beta;
	double b; 
	if (!i){
		alpha = as->a1;
		b = ep(state_0, word[pos]);
		if (!j){
      beta = bs->a1;
			a = state_0->transitions[0];
    }
		else {
      beta = bs->a2;
			a = state_0->transitions[1];
    }
	}
	else {
		alpha = as->a2;
		b = ep(state_1, word[pos]);
		if (!j){
      beta = bs->a1;
			a = state_1->transitions[0];
    }
		else {
      beta = bs->a2;
			a = state_1->transitions[1];
    }
	}
	return ((alpha * a * b * beta) / prob);
}

char* get_first_n(char* word, int n){
	char *new = (char*)malloc(strlen(word)*sizeof(word));
	strncpy(new, word, n);
	new[n] = '\0';
	return new;
}

void print_soft_counts(char* word, double *Pi, state *state_0, state *state_1){
	int length = strlen(word);
	int m, n, k;
  printf("\n\n---------------------------------\n- Soft Counts: %s -\n---------------------------------\n", word);
	for (m=0; m<length; m++){
		printf("Letter:\t%c\n", word[m]);
		for(n=0; n<2; n++){
			printf("\tFrom state:\t%d\n", n);
			for(k=0; k<2; k++){
				printf("\t\tto state:\t%d\t%lf;\n", k, soft_count(word, m, Pi, state_0, state_1, n, k));
			}
		}
	}
}

int seeker2(const void *el, const void *key){
  const softcounts* sc = (softcounts*)el;
  return (sc->letter == *(char*)key);
}

// Comparator function for log probabilities of emission
int compare_log_prob(const void *el1, const void *el2){
	const letter_double *ld = (letter_double*) el1;
	const letter_double *ld1 = (letter_double*) el2;
	return ((int)floor(ld->d - ld1->d));
}

// Stores the total soft counts for each letter in each state
// Also updates the emission, transition, for each state, as well as Pi.
list_t sum_soft_counts(list_t alphabet, list_t words, double *Pi, state *state_0, state *state_1, int verbose){
  int length;
  int i, j, k;
  char letter;
  char* word_buf = (char*)malloc(64);
  list_t ret;
  list_init (&ret);
  
  // Divide raw soft count by this
  double tot_count = 0;
  
  // Divide transitions by these
  double trans_0 = 0;
  double trans_1 = 0;
  
  double sc00, sc01, sc10, sc11;
  
  // This will update the new emission probabilities
  softcounts *sc = (softcounts*)malloc(sizeof(softcounts));
  softcounts *temp = (softcounts*)malloc(sizeof(softcounts));
  
  // This will update the new initial probabilities
  double *new_Pi = (double*)calloc(sizeof(double), 2);
  
  // This will update the new transition probabilities
  
  for (i=0; i < list_size(&alphabet); i++){
    sc = (softcounts*)malloc(sizeof(softcounts));
    sc->letter = ((letter_count*)list_get_at(&alphabet, i))->letter;
    // printf("Adding soft counts: %c\n", sc->letter);
    sc->sc00 = 0;
    sc->sc01 = 0;
    sc->sc10 = 0;
    sc->sc11 = 0;
    list_append(&ret, sc);
  }
  // printf("Size: %d\n", list_size(&ret));
  for (i=0; i<list_size(&words); i++){
    word_buf = list_get_at(&words, i);
    //printf("Processing word: %s\n", word_buf);
    for (j=0; j<strlen(word_buf); j++){
      
      letter = word_buf[j];
      // printf("Letter: %c\n", letter);
      list_attributes_seeker(& ret, seeker2);
      temp = (softcounts*)list_seek(&ret, &letter);
      // printf("Found: %c\n", temp->letter);
      temp = (softcounts*)list_seek(&ret, &word_buf[j]);
      
      sc00 = soft_count(word_buf, j, Pi, state_0, state_1, 0, 0);
      sc01 = soft_count(word_buf, j, Pi, state_0, state_1, 0, 1);
      sc10 = soft_count(word_buf, j, Pi, state_0, state_1, 1, 0);
      sc11 = soft_count(word_buf, j, Pi, state_0, state_1, 1, 1);
      temp->sc00 += sc00;
      temp->sc01 += sc01;
      temp->sc10 += sc10;
      temp->sc11 += sc11;
      if (j==0){
        new_Pi[0] += (sc00 + sc01);
        new_Pi[1] += (sc10 + sc11);
      }
    }
  }
  new_Pi[0] = new_Pi[0]/list_size(&words);
  new_Pi[1] = new_Pi[1]/list_size(&words);

  // NOTE: This definition of updated Pi prevents rounding error from 
  // changing the sum of Pi values.
  
  Pi[0] = 1 - new_Pi[1];
  Pi[1] = new_Pi[1];
  
  // Reset transition probabilities for each state
  for (i=0; i<2; i++){
    state_0->transitions[i] = 0;
    state_1->transitions[i] = 0;
  }
  if (verbose)
    printf("\n\nEmission raw counts\n\tFrom State 0\n");
  tot_count = 0;
  for (j=0; j<list_size(&ret); j++){
    temp = (softcounts*)list_get_at(&ret, j);
    if (verbose){
      printf("\t\tLetter:%c\n\t\t\tto state 0\t%lf\n", temp->letter, temp->sc00);
      printf("\t\t\tto state 1\t%lf\n", temp->sc01);
    }
    tot_count += temp->sc00 + temp->sc01;
  }
  // printf("Total %lf\n", tot_count);
  if (verbose)
    printf("Emission probabilities\n");
  for (j=0; j<list_size(&ret); j++){
    temp = (softcounts*)list_get_at(&ret, j);
    if (verbose){
      printf("letter:\t%c\tprobability: %lf\n", temp->letter, (temp->sc00+temp->sc01)/tot_count);
    }
    state_0->transitions[0] += temp->sc00;
    state_0->transitions[1] += temp->sc01;
    
    // Update state_0
    for (i=0; i<list_size(&alphabet); i++){
      if (state_0->letters[i] == temp->letter)
        state_0->emissions[i] = (temp->sc00+temp->sc01)/tot_count;
    }
  }
  tot_count = 0;
  if (verbose){
    printf("\n\tFrom State 1\n");
  }
  for (j=0; j<list_size(&ret); j++){
    temp = (softcounts*)list_get_at(&ret, j);
    if (verbose){
      printf("\t\tLetter:%c\n\t\t\tto state 0\t%lf\n", temp->letter, temp->sc10);
      printf("\t\t\tto state 1\t%lf\n", temp->sc11);
    }
    tot_count += temp->sc10 + temp->sc11;
  }
  if (verbose){
    printf("Emission probabilities\n");
  }
  for (j=0; j<list_size(&ret); j++){
    temp = (softcounts*)list_get_at(&ret, j);
    if (verbose){
      printf("letter:\t%c\tprobability: %lf\n", temp->letter, (temp->sc10+temp->sc11)/tot_count);
    }
    state_1->transitions[0] += temp->sc10;
    state_1->transitions[1] += temp->sc11;
    
    // Update state_1
    for (i=0; i<list_size(&alphabet); i++){
      if (state_1->letters[i] == temp->letter)
        state_1->emissions[i] = (temp->sc10+temp->sc11)/tot_count;
    }
  }
  // printf("Transitions: %lf %lf %lf %lf\n", state_0->transitions[0], state_0->transitions[1], state_1->transitions[0], state_1->transitions[1]);
  for (i=0; i<2; i++){
    trans_0 += state_0->transitions[i];
    trans_1 += state_1->transitions[i];
  }
  // printf("Transitions: %lf %lf %lf %lf\n", state_0->transitions[0], state_0->transitions[1], state_1->transitions[0], state_1->transitions[1]);
  for (i=0; i<2; i++){
    state_0->transitions[i] /= trans_0;
    state_1->transitions[i] /= trans_1;
  }
  free(new_Pi);
  return ret;
}


// Performs one iteration of the HMM.
// This function returns the total probability, so that it can be used
// to terminate the loop if desired.
double iterate_hmm(list_t alphabet, list_t words, double *Pi, state *state_0, state *state_1, int verbose, int print){
	double prob = 0;
  int i;
  if (verbose){
    for (i=0; i<list_size(&words); i++){
      char* word = (char*)malloc(sizeof(char));
      word = list_get_at(&words, i);
      print_alpha_beta(word, Pi, state_0, state_1, verbose);
    }
  }
  
  for (i=0; i<list_size(&words); i++){
  	char* word = (char*)malloc(sizeof(char));
  	word = list_get_at(&words, i);
  	// printf("Total probability of %s = %lf\n", word, probability(word, Pi, state_0, state_1));
  	// printf("Total: %lf\n", total);
    
  }
  if (verbose){
    for (i=0; i<list_size(&words); i++){
      char* word = (char*)malloc(sizeof(char));
      word = list_get_at(&words, i);
      print_soft_counts(word, Pi, state_0, state_1);
    }
  }
  
  list_t total_soft_counts;
  list_init(&total_soft_counts);
  total_soft_counts = sum_soft_counts(alphabet, words, Pi, state_0, state_1, verbose);
  if (print){
 		printf("------------------------------------------------------------------\n- End of Iteration Summary \n------------------------------------------------------------------\n");
  
  	printf("--------------------------------------------\n- Pi -\n--------------------------------------------\n");
 		printf("State\t0\t%lf\nState\t1\t%lf\n", Pi[0], Pi[1]);
  
  	printf("--------------------------------------------\n- Emission Probabilities -\n--------------------------------------------\n");
 	 	printf("State 0\t\t\tState 1\n");
  	for (i=0; i<list_size(&alphabet); i++){
    	printf("%c\t%lf\t\%c\t%lf\n", state_0->letters[i], state_0->emissions[i], state_1->letters[i], state_1->emissions[i]);
  	}
  	printf("--------------------------------------------\n- Transition Probabilities -\n--------------------------------------------\nFrom\t To:\tState 0\t\tState 1\nState 0\t\t%lf\t%lf\nState 1\t\t%lf\t%lf\n", state_0->transitions[0], state_0->transitions[1], state_1->transitions[0], state_1->transitions[1]);
  }
  for(i = 0; i<list_size(&words); i++){
  		prob += probability((char*)list_get_at(&words, i), Pi, state_0, state_1);
  }
  printf("Total probability: %.12lf\n", prob);
  return prob;
}

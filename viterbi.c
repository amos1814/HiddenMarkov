#include "hmmcalc.h"

void viterbi(char* word, double* Pi, state* state_0, state* state_1){
	
}

int main(int argc, char *argv[]){
  double total_prob = 0.00000001;
  double total_prob_prev = 0.0;
	int verbose = 0;
	// Max word size is 32
	char* word = (char*)malloc(32);
	if (argc > 1){
  		if (!strcmp(argv[1], "-v")){
   		 verbose = 1;
	    	 printf("VERBOSE OUTPUT\n");
		} else {
  			fprintf(stderr, "Invalid arguments\n");
  			exit(1);
		}
	}
	puts("Enter input file name:");
  char file_name[64];
  scanf("%s", file_name);
  FILE *file;
  file = fopen(file_name, "r");
  list_t alphabet;
  list_init(& alphabet);
  if (file == NULL){
    fprintf(stderr, "%s: No such file\n", file_name);
    exit(1);
  }

	puts("Enter word for Viterbi parse:");
  scanf("%s", word);

  // Get the list of letters from the file.
  alphabet = parse_file(file);
	int num_letters;
  num_letters = list_size(&alphabet);
  state *state_0 = (state*)malloc(sizeof(state));
  state *state_1 = (state*)malloc(sizeof(state));
  
  state_0 = initialize_state(0, alphabet, 1);
  state_1 = initialize_state(1, alphabet, 0);
  
  // Randomly generate initial probabilities.
  srand((unsigned)time(NULL));

  double *Pi = (double*)malloc(2*sizeof(double));
  // Pi[0] = ((double)rand()/(double)RAND_MAX);
  Pi[0] = 0.5;
  Pi[1] = 1 - Pi[0];
  //printf("Pi:\n\tState\t0\t%lf\n\tState\t1\t%lf\n", Pi[0], Pi[1]);

  // Max word size 32
  int i=0;
  char *buffer = (char*)malloc(32);
  list_t words;
  list_init(&words);
  fclose(file);
  file = fopen(file_name, "r");
  while (fscanf(file, "%s ", buffer) != EOF){
  strcat(buffer, "#");
    list_prepend(& words, strdup(buffer));
  }
	shuffle_list(words, list_size(&words));

	int iteration = 0;
	while(total_prob_prev < total_prob){
		printf("%lf %lf\n", total_prob_prev, total_prob);
 	 	printf("Running iteration number %d\n", iteration);
		total_prob_prev = total_prob;
  		total_prob = iterate_hmm(alphabet, words, Pi, state_0, state_1, verbose, 0);
  		iteration++;
 	}
	printf("%lf %lf\n", total_prob_prev, total_prob);
	iterate_hmm(alphabet, words, Pi, state_0, state_1, verbose, 0);
	viterbi(word, Pi, state_0, state_1);

	free(word);
	return 0;
}

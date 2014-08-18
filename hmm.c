#include "hmmcalc.h"

int main(int argc, char *argv[])
{
  int verbose = 0;
  int iterations = 1;
  int iterating = 0;
  int self_terminate = 0;
  double total_prob = 0.00000001;
  double total_prob_prev = 0.0;
	char c;
  if (argc > 1){
    if (!strcmp(argv[1], "-v")){
      verbose = 1;
      printf("VERBOSE OUTPUT\n");
    } else if (!strcmp(argv[1], "-i")){
    		if (argc == 2){
    			fprintf(stderr, "enter number of iterations\n");
    			exit(1);
    		} else if (argc == 3){
    			iterations = atoi(argv[2]);
    			iterating = 1;
    			printf ("Running %d iterations\n", iterations);
			}
    	} else if (!strcmp(argv[1], "-st")){
    		self_terminate = 1;
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
  // Get the list of letters from the file.
  alphabet = parse_file(file);
  
  // Randomly generate initial probabilities.
  srand((unsigned)time(NULL));
  
  int num_letters;
  num_letters = list_size(&alphabet);
  state *state_0 = (state*)malloc(sizeof(state));
  state *state_1 = (state*)malloc(sizeof(state));
  
  state_0 = initialize_state(0, alphabet, 1);
  state_1 = initialize_state(1, alphabet, 0);
  
    printf("---------------------------------\n- Initialization -\n---------------------------------\n");
  
    print_state(state_0, alphabet);
    print_state(state_1, alphabet);
  
    printf("\n---------------------------------\n");
  
  
  double *Pi = (double*)malloc(2*sizeof(double));
  // Pi[0] = ((double)rand()/(double)RAND_MAX);
  Pi[0] = 0.5;
  Pi[1] = 1 - Pi[0];
  printf("Pi:\n\tState\t0\t%lf\n\tState\t1\t%lf\n", Pi[0], Pi[1]);
  
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
	//shuffle_list(words, list_size(&words));
  
// Display word list
//  char *temp;
//  for (i=0; i<list_size(&words); i++){
//    temp = (char*)list_get_at(&words, i);
//			printf("%s\n", temp);
//  }
  int iteration = 0;
  if (!self_terminate){
  		while(iteration < iterations){
  			printf("Running iteration number %d\n", iteration);
  			if (!iterating){
   			iterate_hmm(alphabet, words, Pi, state_0, state_1, verbose, 1);
 		   	printf("\n\n\n\n");
  	  		} else if (iteration == iterations - 1){
   		 	iterate_hmm(alphabet, words, Pi, state_0, state_1, verbose, 1);
 	 	  } else {
    			iterate_hmm(alphabet, words, Pi, state_0, state_1, verbose, 1);
    		}
    		iteration++;
    }
  } else {
  		while((iteration < 10) || total_prob_prev < total_prob){
			printf("%lf %lf\n", total_prob_prev, total_prob);
  			printf("Running iteration number %d\n", iteration);
  			total_prob_prev = total_prob;
  			total_prob = iterate_hmm(alphabet, words, Pi, state_0, state_1, verbose, 1);
  			iteration++;
  			/* If total probability is not increasing much, return that iteration's information.
  			It doesn't make a big difference if we run one more information after the maximum,
  			so I don't output the previous iteration */
 	 	}
		printf("%lf %lf\n", total_prob_prev, total_prob);
		iterate_hmm(alphabet, words, Pi, state_0, state_1, verbose, 1);
  }
//	for (i=0; i<list_size(&alphabet); i++){
//		c = ((letter_count*)list_get_at(&alphabet, i))->letter;
//		printf("Log probability of %c: %lf\n", c, log(ep(state_0, c)/ep(state_1, c)));
//	}
	list_t freq0;
	list_t freq1;
	list_init(&freq0);
	list_init(&freq1);
	letter_double* ld;
	for (i=0; i<list_size(&alphabet); i++){
		c = ((letter_count*)list_get_at(&alphabet, i))->letter;
		ld = (letter_double*)malloc(sizeof(letter_double));
		ld->letter = c;
		ld->d = log(ep(state_0, c)/ep(state_1, c));
		if (log(ep(state_0, c)/ep(state_1, c)) > 0){
			list_append(&freq0, ld);
		}
	}
	list_attributes_comparator(&freq0, compare_log_prob);
	list_sort(&freq0, 1);
	printf("Letters most preferred by State 0:\n");
	for (i=0; i<list_size(&freq0); i++){
		ld = ((letter_double*)list_get_at(&freq0, i));
			printf("%c: %lf\n", ld->letter, ld->d);
	}
	for (i=0; i<list_size(&alphabet); i++){
		c = ((letter_count*)list_get_at(&alphabet, i))->letter;
		ld = (letter_double*)malloc(sizeof(letter_double));
		ld->letter = c;
		ld->d = log(ep(state_0, c)/ep(state_1, c));
		if (log(ep(state_0, c)/ep(state_1, c)) < 0){
			list_append(&freq1, ld);
		}
	}
	list_attributes_comparator(&freq1, compare_log_prob);
	list_sort(&freq1, 1);
	printf("\nLetters most preferred by State 1:\n");
	for (i=0; i<list_size(&freq1); i++){
		ld = ((letter_double*)list_get_at(&freq1, i));
			printf("%c: %lf\n", ld->letter, ld->d);
	}
	printf("\n");
  
  fclose(file);
  return 0;
}

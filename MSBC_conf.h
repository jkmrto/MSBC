#ifndef H_MSBC_CONF
#define H_MSBC_CONF

#include <stdint.h>
//#include <sys/types.h>
#include <stdbool.h>

#include "MSBC_macro.h"

#define ONE_PARTITION 1
#define SQUARE_PARTITION 2

#define TOTAL_PROBABILITY 2
#define MAX_PROBABILITY 1

typedef struct MSBC_variables{
	int clusters_required;
	
	int Kmeans_model;
	int calculate_probability; 

	float minimum_error_recoverable;
	float step_modifier;
	float fixed_shift;
	
	float min_variance;
	float no_gaussian_available;
	uint16_t normalized_size;

	bool euclidean_mean;
	bool euclidean_shift;

	char signature_file[MAX_BUFFER];

	bool print_map_file;
	uint16_t num_protocols_train;

}MSBC_variables;

extern MSBC_variables MSBCv;


int init_MSBC_default_conf();
int load_MSBC_conf();
int print_MSBC_conf();

#endif
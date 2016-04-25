#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "NormalizeFunctions.h"
#include "MSBC_macro.h"
#include "MSBC_train.h"

typedef struct cluster{
	float center;
	float variance;
	uint32_t values_content;
	float weight;
}cluster;

typedef struct clusters_trained{
	struct cluster *clusters;
	uint8_t num_clusters;
}clusters_trained;

int assign_belong(int32_t *values,int *array_belong,int num_values,struct cluster *clusters,int clusters_active);
int average_estimation(int32_t *values,int *array_belong,int num_values,struct cluster *clusters,int clusters_active);
int init_Kmeans(int32_t *values,int num_values, struct cluster *clusters,int *clusters_active);
int make_square_partition_shift(struct cluster *clusters,int *clusters_active);
int make_one_shift(float *center_reference,float variance,float *center_empty,int *clusters_active);
int spread_cluster(struct cluster *clusters,int clusters_active);
int get_position_empty_cluster(struct cluster *clusters,int *clusters_active);
float estimate_error(struct cluster *clusters,int n_clusters_active, int num_values);
int print_debug_information(int32_t *values,int num_values,int *array_belong,struct cluster *clusters, int n_clusters_active);
int K_means_algorithm(struct values_to_train *v,struct clusters_trained *clusters_result);
int estimate_clusters_weight(const uint32_t total_values, struct clusters_trained *clusters_result);
float get_new_cluster(float center,float var,int mode);

//int make_shift(struct cluster *clusters,float shift,int clusters_active);

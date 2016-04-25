#include <string.h>
#include <stdio.h>

#include <common/common.h>

#include "MSBC_macro.h"
#include "MSBC_conf.h"

#define MSBC_conf_name "MSBC_conf.txt"


int init_MSBC_default_conf(){

	MSBCv.clusters_required = 15;
	MSBCv.Kmeans_model = SQUARE_PARTITION;
	MSBCv.minimum_error_recoverable = 0.0000002; //Value Normalized 
	MSBCv.step_modifier = 0.1;
	MSBCv.min_variance = 0.0000001;
	MSBCv.no_gaussian_available = 0;
	MSBCv.fixed_shift = 0.1;
	MSBCv.normalized_size = 512;

	MSBCv.euclidean_mean = false;
	MSBCv.euclidean_shift = false;

	sprintf(MSBCv.signature_file,"own");

	MSBCv.print_map_file = true;
	MSBCv.num_protocols_train = 408;
	MSBCv.calculate_probability = TOTAL_PROBABILITY;
	return 0;
}

int load_MSBC_conf(){
	FILE *fp_conf; 
	char *field;
    char *row = malloc(MAX_BUFFER * sizeof(char));  /* Buffer to store file rows */
    char *sptr = NULL;
	char path_map_conf[MAX_BUFFER];
	
	sprintf(path_map_conf ,"%s/plugins/%s_%s/%s", tie_path, NAME, "1.1",MSBC_conf_name);

	if ((fp_conf = fopen(path_map_conf, "r")) == NULL) {
        printf("\nERROR: Unable to open %s file!\n\n", path_map_conf);
        return -1;
    } else { 
        printf("\nFile %s opened!\n\n", path_map_conf); 
    }

    while(fgets(row, MAX_BUFFER,fp_conf)){
    	 if (row[0] == '#' || row[0] == ' ')
            continue;

        field = strtok_r(row, "\t", &sptr);

        if(strcmp(field,"Number_protocolos_to_train") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	       	MSBCv.num_protocols_train = atoi(field);    
	    }

	    if(strcmp(field,"Clusters_Required") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	       	MSBCv.clusters_required = atoi(field);    
	    }


	    if(strcmp(field,"Normalized_size") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	       	MSBCv.normalized_size = atoi(field);    
	    }

	    if(strcmp(field,"Signature_file_name") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	      	sprintf(MSBCv.signature_file,"%s",field);    
	    }

		 if(strcmp(field,"Print_map_file") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	       	if(strcmp(field,"Yes") == 0)
	       		MSBCv.print_map_file = true;   //by default false 
	    }

	    if(strcmp(field,"Kmeans_model") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	       	if(strcmp(field,"one_partition") == 0){
	       		MSBCv.Kmeans_model = ONE_PARTITION;   //by default false 
	       	}else if((strcmp(field,"square_partition") == 0)){
	       		MSBCv.Kmeans_model = SQUARE_PARTITION;
	       	}else{
	       		printf("No selected any partition model for Kmeans");
	       	}
	    }

		if(strcmp(field,"Calculate_probability") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	       	if(strcmp(field,"max") == 0){
	       		MSBCv.calculate_probability = MAX_PROBABILITY;   //by default false 
	       	}else if((strcmp(field,"total") == 0)){
	       		MSBCv.calculate_probability = TOTAL_PROBABILITY;
	       	}else{
	       		printf("No selected any partition model for Kmeans");
	       	}
	    }    

	    if(strcmp(field,"Kmeans_Euclidean_mean") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	       	if(strcmp(field,"Yes") == 0){
	       		MSBCv.euclidean_mean = true;   //by default false 
	       	}
	    }


	    if(strcmp(field,"Kmeans_Euclidean_shift") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	       	if(strcmp(field,"Yes") == 0)
	       		MSBCv.euclidean_shift = true;   //by default false 
	    }

	    if(strcmp(field,"No_gaussian_min_probability") == 0 ){
	       	field = strtok_r(NULL, " ;", &sptr);
	       	MSBCv.no_gaussian_available = atof(field);    
	    }

	    if(strcmp(field,"Kmeans_fixed_shift") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	       	MSBCv.fixed_shift = atof(field);    
	    }

	    if(strcmp(field,"Kmeans_dynamic_step_modifier") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	       	MSBCv.step_modifier = atof(field);    
	    }

	    if(strcmp(field,"Error_threshold") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	       	MSBCv.minimum_error_recoverable = atof(field);    
	    }

	    if(strcmp(field,"Min_gaussian_variance") == 0 ){
	       	field = strtok_r(NULL, ";", &sptr);
	       	MSBCv.min_variance = atof(field);    
	    }
	    



	}

	return 0;
}

int print_MSBC_conf(){
	printf("\nClusters required: %d\n",MSBCv.clusters_required);
	printf("Normalized size: %d\n",MSBCv.normalized_size);
	printf("Min probability given for a cluster without gaussian: %1.10f\n",MSBCv.no_gaussian_available);
	printf("Euclidean_mean: %s\n",MSBCv.euclidean_mean ? "Yes":"No");
	printf("Euclidean_shift: %s\n",MSBCv.euclidean_shift ? "Yes":"No");
	printf("Kmeans model: %d,\tone|square\n",MSBCv.Kmeans_model);
	printf("Kmeans_fixed_shift: %1.4f\n",MSBCv.fixed_shift);
	printf("Kmeans_dynamic_step_modifier: %1.4f\n",MSBCv.step_modifier);
	printf("Error_threshold: %1.10f\n",MSBCv.minimum_error_recoverable);
	printf("Min_gaussian_variance: %1.10f\n",MSBCv.min_variance);
	printf("Print_map_file: %s\n",MSBCv.print_map_file ? "Yes":"No");
	printf("Signature_file_name: %s\n",MSBCv.signature_file);
	printf("Number of protocolo to train: %d\n",MSBCv.num_protocols_train);
	printf("Kmeans calculate probability : %d,\tmax|total\n",MSBCv.calculate_probability);
}





#include "Kmeans.h"

#include "MSBC_train.h"
#include "NormalizeFunctions.h"
#include <string.h>

#include "MSBC_conf.h"


/*


*/
int assign_belong(int32_t *values,int *array_belong,int num_values,struct cluster *clusters,int clusters_active){
int i,j,cluster_nearest;
float current_distance;

	for (i=0;i<num_values;i++){
			current_distance = distance_normalized((float)values[i],(clusters[0].center));
			cluster_nearest = 0;
		for (j = 1;j < clusters_active; j++){
			if (current_distance > distance_normalized((float)values[i],clusters[j].center)){
				current_distance = distance_normalized((float )values[i],clusters[j].center);
				cluster_nearest = j;	
			}
		}
		array_belong[i] = cluster_nearest;

	}

return 0;
}


int average_estimation(int32_t *values,int *array_belong,int num_values,struct cluster *clusters,int clusters_active){
int i,j;
float *cluster_sum_temp, *cluster_variance_temp;
int *cluster_num_values_content;
float variance_temp_value;

/*Use calloc in order to init by 0*/
cluster_sum_temp = calloc(clusters_active,sizeof(float));
cluster_num_values_content = calloc(clusters_active,sizeof(int));
cluster_variance_temp = calloc(clusters_active,sizeof(float));


	if(MSBCv.euclidean_mean){
		/*Evaluate the sum of the values belong to each cluster*/
		for (i=0;i<num_values;i++){
			cluster_sum_temp[array_belong[i]] = (values[i]) + cluster_sum_temp[array_belong[i]]; 
			cluster_num_values_content[array_belong[i]] = cluster_num_values_content[array_belong[i]]+1;
		}
		//Establish de mean
		for(i=0;i<clusters_active;i++){
			clusters[i].center = (cluster_sum_temp[i]/cluster_num_values_content[i]);
			clusters[i].values_content = cluster_num_values_content[i];
		}
	}else{
		for (i=0;i<num_values;i++){
			cluster_sum_temp[array_belong[i]] = size_normalized(values[i]) + cluster_sum_temp[array_belong[i]]; 
			cluster_num_values_content[array_belong[i]] = cluster_num_values_content[array_belong[i]]+1;
		}
		//Establish de mean
		for(i=0;i<clusters_active;i++){
			clusters[i].center = denormalize_size((cluster_sum_temp[i]/cluster_num_values_content[i]));
			clusters[i].values_content = cluster_num_values_content[i];
		}	
	}


	/*THIS LAST LOOP COULD BE LEAVED FOR THE LAST ITERACITON OF K-MEANS ALGORITHM
	IN ORDER TO REDUCE THE COMPUTE REQUIREMENTS*/
	/*WE NEED THIS TO GET THE SPREAD OF THE CLUSTERS, EVERY ITERACTION*/
	/*Establish the variance for each cluster*/
	for (i=0;i<num_values;i++){
		variance_temp_value = pow(distance_normalized(clusters[array_belong[i]].center,(float)values[i]),2);
		cluster_variance_temp[array_belong[i]] = variance_temp_value + cluster_variance_temp[array_belong[i]];
	}

	for (i=0;i<clusters_active;i++){
		clusters[i].variance = cluster_variance_temp[i] / clusters[i].values_content;
	}

free(cluster_num_values_content);
free(cluster_sum_temp);
free(cluster_variance_temp);

return 0;	
}

int init_Kmeans(int32_t *values,int num_values, struct cluster *clusters, int *clusters_active){
	int i;
	float temp_init_cluster = 0;
	float variance_temp_value = 0;

	if (num_values > 0 ){
		for (i=0;i<num_values;i++){
			//printf("%d\n",values[i]);
			temp_init_cluster = temp_init_cluster + values[i];
			//printf("%f\n",temp_init_cluster);
			
		}


		clusters[0].center = temp_init_cluster/num_values;
		clusters[0].values_content = num_values;
		
		/*loop to get the variance*/
		for (i=0;i<num_values;i++){
			variance_temp_value = variance_temp_value + pow(distance_normalized(clusters[0].center,(float)values[i]),2);
		}
		clusters[0].variance = variance_temp_value / clusters[0].values_content;
		(*clusters_active)++;
	}
	return 0;
}

int make_square_partition_shift(struct cluster *clusters,int *clusters_active){
	int i;

	int fixed_clusters_active = *clusters_active; 

	for(i=0;i<(fixed_clusters_active);i++){
		make_one_shift(&clusters[i].center,clusters[i].variance,&clusters[fixed_clusters_active+i].center,clusters_active);
	}


	return 0;
}

int make_one_shift(float *center_reference,float variance,float *center_empty,int *clusters_active){

	if(MSBCv.euclidean_shift){
		*center_empty = *center_reference  - MSBCv.fixed_shift;
		*center_reference = *center_reference  + MSBCv.fixed_shift; 

	}else{
		*center_empty = get_new_cluster(*center_reference,variance,1); /*Positive abs SHIFT*/
		*center_reference = get_new_cluster(*center_reference,variance,2); /*Negative abs SHIFTI//FT*/
	}
		
	(*clusters_active)++;

#ifdef DEBUG_MSBC
	printf("New centers: %1.10f, %1.10f\n",*center_empty,*center_reference);
#endif	
	
	return 0;
}

	bool euclidean_mean;
	
/*This function returns the position of the most dispersed cluster*/
int spread_cluster(struct cluster *clusters,int clusters_active){
	int i , pos_most_dispersed_cluster = 0;

	for (i = 1;i < clusters_active;i++){
		if(clusters[pos_most_dispersed_cluster].variance < clusters[i].variance){
			pos_most_dispersed_cluster = i;
		//	printf("cluster pos: %d\n",i);
		}
	}

	return pos_most_dispersed_cluster;
}



/*This function returns the position of the next cluster empty*/
int get_position_empty_cluster(struct cluster *clusters,int *clusters_active){
	//int i = *last_empty_cluster_pos;
	int i = 0; 
	int stay = 1;
	int empty_cluster_pos = -1;

	while(stay == 1){
		if(i == *clusters_active){
			stay = 0;	
		}else{
			if(clusters[i].values_content == 0){
				empty_cluster_pos = i;
				stay = 0;
			}
		}
	i++;	
	}


	if (empty_cluster_pos < 0){
	    empty_cluster_pos = *clusters_active;
	}else{
	    (*clusters_active)--; //to avoid increase the number of cluster actives cause we are reusing one
	}

	return empty_cluster_pos;
}

float estimate_error(struct cluster *clusters,int n_clusters_active,int num_values){
	int i;
	float estimated_error = 0.0;
	int total_values = 0;

		for(i = 0;i<n_clusters_active;i++){
			if(clusters[i].values_content > 0)		
				estimated_error = (clusters[i].variance * clusters[i].values_content / num_values)  + estimated_error;
				
	}

	return estimated_error;
}

int print_debug_information(int32_t *values,int num_values,int *array_belong,struct cluster *clusters, int n_clusters_active){
	 int i;

	printf("Number of clusters: %d\n",n_clusters_active);

	for (i = 0 ; i < num_values ; i++){
        printf("v=%d c=%d,\t",values[i],array_belong[i]);
    }
    printf("\n");
    for (i = 0; i < n_clusters_active; i++){
        printf("%d) center:%1.4f, variance:%1.4f, values_content: %d\n",i,clusters[i].center,clusters[i].variance,clusters[i].values_content);
    }
    printf("\n");

    return 0;
}

int estimate_clusters_weight(const uint32_t total_values, struct clusters_trained *clusters_result){
	int i;
	
	for(i = 0;i < clusters_result->num_clusters; i++)
	clusters_result->clusters[i].weight = ((float)clusters_result->clusters[i].values_content)/total_values; 


	return 0;
}

float get_new_cluster(float center,float var,int mode){
	float desv = sqrt(var);
	float new_center = 0.0; 
	float abs_center = abs(center);
	float step = MSBCv.step_modifier  * desv;
	/*positive abs SHIFT*/	
	if (mode == 1){
		new_center = (abs_center + step * (MSBCv.normalized_size + abs_center))/( 1 - step * (1 + abs_center/MSBCv.normalized_size));
	/*negative abs SHIFT*/	
	}else if(mode == 2){
		new_center = (abs_center - step * (MSBCv.normalized_size + abs_center))/( 1 + step * (1 + abs_center/MSBCv.normalized_size));
	}else{
		printf("Error getting the new clusters\n");
	}

	//Change the sign/
	if (center < 0){
		new_center = - new_center;
	}

	return new_center;
}




int K_means_algorithm(struct values_to_train *v,struct clusters_trained *clusters_result)
	{
	int array_belong[v->num_values];
	struct cluster *clusters;  
	clusters = clusters_result->clusters;

	int n_clusters_active = 0;

	int i;
    int empty_cluster_pos,most_dispersed_cluster_pos;
    float last_estimated_error, estimated_error,up_last_estimated_error;

	
    init_Kmeans(v->value,v->num_values,clusters,&(n_clusters_active));
    assign_belong(v->value,array_belong,v->num_values,clusters,n_clusters_active);

#ifdef DEBUG_MSBC
    //print_debug_information(v->value,v->num_values,array_belong,clusters,n_clusters_active);
#endif

   // printf ("Modelo de k means: %d,   %d, %d \n",MSBCv.Kmeans_model,ONE_PARTITION,SQUARE_PARTITION);

    most_dispersed_cluster_pos = spread_cluster(clusters,n_clusters_active);
    
    estimated_error = estimate_error(clusters,n_clusters_active,v->num_values);
    up_last_estimated_error = 2 * MSBCv.minimum_error_recoverable + estimated_error;

   while((n_clusters_active < MSBCv.clusters_required) && (clusters[most_dispersed_cluster_pos].variance > MSBCv.min_variance) 
   	&& (up_last_estimated_error > (MSBCv.minimum_error_recoverable + estimated_error))){

   	up_last_estimated_error = estimated_error;
	
	if(MSBCv.Kmeans_model == SQUARE_PARTITION){
	    if (2 * n_clusters_active > MSBCv.clusters_required){

	       	empty_cluster_pos = get_position_empty_cluster(clusters,&n_clusters_active);

	#ifdef DEBUG_MSBC
	        printf("Empty cluster index:%d\n",empty_cluster_pos);
	        printf("Dispersed cluster index:%d\n\n",most_dispersed_cluster_pos);
	 #endif       

	    make_one_shift(&clusters[most_dispersed_cluster_pos].center,clusters[most_dispersed_cluster_pos].variance,&clusters[empty_cluster_pos].center,&n_clusters_active);

	    }else{
	    	make_square_partition_shift(clusters,&n_clusters_active);
	    }

	}else if(MSBCv.Kmeans_model == ONE_PARTITION){

		empty_cluster_pos = get_position_empty_cluster(clusters,&n_clusters_active);

#ifdef DEBUG_MSBC
	printf("Empty cluster index:%d\n",empty_cluster_pos);
	printf("Dispersed cluster index:%d\n\n",most_dispersed_cluster_pos);
#endif 
		make_one_shift(&clusters[most_dispersed_cluster_pos].center,clusters[most_dispersed_cluster_pos].variance,&clusters[empty_cluster_pos].center,&n_clusters_active);

	}
        //First average estimation for these clusters
        assign_belong(v->value,array_belong,v->num_values,clusters,n_clusters_active);
        average_estimation(v->value,array_belong,v->num_values,clusters,n_clusters_active);
        estimated_error = estimate_error(clusters,n_clusters_active,v->num_values);   
        last_estimated_error = 2 * MSBCv.minimum_error_recoverable + estimated_error;


#ifdef  DEBUG_MSBC
   //print_debug_information(v->value,v->num_values,array_belong,clusters,n_clusters_active);
#endif        


        most_dispersed_cluster_pos = spread_cluster(clusters,n_clusters_active);



 /*       sprintf(string_term,"set term wxt %d \n",terminal_index);
        terminal_index++;
        fprintf(gnuplotPipe,string_term);
        fprintf(gnuplotPipe,"reset\n");

        for (i=0;i<n_clusters_active;i++){
            sprintf(prueba,"set object circle at %1.8f,5 size 0.5 fc rgb 'navy'\n",clusters[i].center);
             fprintf(gnuplotPipe, prueba);
        }
        for (i=0; i < 3; i++)
        {
        fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]); //Send commands to gnuplot one by one.
        }

        fprintf(FILE_estimated_error, "%d %1.10f \n",n_clusters_active,last_estimated_error); //Write the data to a temporary file
    }
    sprintf(string_term,"set term wxt %d \n",terminal_index);
    terminal_index++;
    fprintf(gnuplotPipe,string_term);
    fprintf(gnuplotPipe,"reset\n");
    fprintf(gnuplotPipe,"set title \"Representacion de los valores\"\n");
    fprintf(gnuplotPipe,"plot 'Estimated_Error.temp'\n");
    fprintf(gnuplotPipe,"reset\n");
	*/
/*	}
}*/

	}
/*
	char * commandsForGnuplot[] = {"set title \"Hola corazones\"", 

	"PI=3.14; s=1; m=1; a=1",
	"gauss(x) = a/(2*PI*s**2)**0.5*exp(-(x-m)**2/(2*s**2)) ",
	 "fit gauss(x)  via s, m, a",
	 "set multiplot",
	"set yrange [-1:2]",
	  "plot gauss(x)",
	  "unset ylabel",
	 "plot 'data.temp'",
	 "unset multiplot"};
   //double xvals[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    //double yvals[5] = {5.0 ,3.0, 1.0, 3.0, 5.0};
	int num_commands = 10;
    FILE * temp = fopen("data.temp", "w");
    /*Opens an interface that one can use to send commands as if they were typing into the
     *     gnuplot command line.  "The -persistent" keeps the plot open even after your
     *     C program terminates.
     */
  /*  FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
    for (i=0; i < v->num_values ; i++)
    {
    	printf("%d", v->value[i]);
    fprintf(temp, "%d %d \n", v->value[i],0); //Write the data to a temporary file
    }

    for (i=0; i < num_commands; i++)
    {
    fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]); //Send commands to gnuplot one by one.
    }
    return 0;*/

	

	clusters_result->num_clusters = n_clusters_active ;
	
	/*Once the K-means algorith has ended, we estimate the weight of each cluster*/
	if (estimate_clusters_weight(v->num_values,clusters_result) != 0){
		printf("Error estimating the weight of the clusters");
	}
	return 1;

}


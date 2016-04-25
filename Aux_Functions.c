#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "plugins/MSBC/Aux_Functions.h"
#include "NormalizeFunctions.h"
#include "MSBC_macro.h"


/*
This function implement the gaussian distribution
*/
float gaus_function(float value,float mean,float variance){
	float value_returned , distNormalized;
	float argument_exponential;
	

	//value_returned = 1/(sqrt((variance)*M_PI*2)); Gaussian value 
		
	//printf("factor dividendo:%f pi:%f variance:%f\n",value_returned,M_PI,variance);

	distNormalized = distance_normalized(value,mean); 

	//printf("Distancia normalizada %f de %f %f\n",distNormalized,value,mean);

	argument_exponential = (-(distNormalized * distNormalized))/(2 * (variance));
	value_returned = exp(argument_exponential);

	return value_returned;
}

/*loat distance_normalized(const float *value1,const float *value2){
	float value_returned, v1,v2;

	v1 = size_normalized(value1);
	v2 = size_normalized(value2);
	value_returned = abs(v1-v2);
	return value_returned;
}

float size_normalized(const float *value){
	float value_returned;

	value_returned = abs((*value)/(NORMALIZED_SIZE+abs(*value)));

	return value_returned;
}*/

int  find_max(float *num,int size,struct value_looked **found){
	int i;
	(*found)->value = 0.0; 

	for (i = 0; i < size ;i++){
		if((*found)->value < num[i]){
			(*found)->value = num[i];
			(*found)->pos = i;
		}
	}
	
	return 0;
}


float max_probability(int value, struct MSBC_model *MSBC_temp){
	float value_returned;
	struct value_looked *value_found_temp;
	int i;

	float array_probability[MSBC_temp->size];

	for(i = 0;i < MSBC_temp->size;i++){
		array_probability[i] = gaus_function((float) value,(MSBC_temp->means[i]),(MSBC_temp->variance[i])) * MSBC_temp-> weight[i];
		//printf("Probability %1.15f    %i\n",array_probability[i],i );
	}
	value_found_temp = calloc(1,sizeof(struct value_looked));
	
	if(find_max(array_probability,MSBC_temp->size,&value_found_temp) != 0){
		printf("Error finding max_probabiltity value");
	}
	value_returned = value_found_temp->value;

	free(value_found_temp); 

	return value_returned;
}

float get_total_probability(int value,struct MSBC_model *MSBC_temp){
	float value_returned = 0.0;
	int i;

	for(i = 0;i < MSBC_temp->size;i++){
		value_returned = gaus_function((float) value,(MSBC_temp->means[i]),(MSBC_temp->variance[i])) * MSBC_temp->weight[i] + value_returned;
		//printf("Probability %1.15f    %i\n",array_probability[i],i );
	}

	return value_returned;
}




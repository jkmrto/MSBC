#include "NormalizeFunctions.h"
#include "MSBC_macro.h"
#include "MSBC_conf.h"

#include <math.h>

float distance_normalized(float value1,float value2){
	float value_returned, v1,v2;

	v1 = size_normalized(value1);
	//printf("tamaño normalizado %f de %f\n",v1,value1);
	v2 = size_normalized(value2);
	//printf("tamaño normalizado %f de %f\n",v2,value2);
	value_returned = fabs(v1-v2);
	return value_returned;
}

float size_normalized(float value){
	float value_returned;

	value_returned = (value)/(MSBCv.normalized_size + fabs(value));

	return value_returned;
}

float denormalize_size(float value){
	float denormalize_value;

	denormalize_value = MSBCv.normalized_size * value;

	if (value > 0){
		denormalize_value = denormalize_value/(1 - value);
	}else{
		denormalize_value = denormalize_value/(1 + value);
	}

	return denormalize_value;
}
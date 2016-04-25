
#ifndef AUX_FUNCTIONS_H_   /* Include guard */
#define AUX_FUNCTIONS_H_

/*
 * Types
 */

typedef struct MSBC_model {
    //each array has the properties of any cluster
    float *means;      
    float *weight;
    float *variance; //array of variances
    u_int8_t size; //amount of clusters
}MSBC_model;

typedef struct TIE_app_id {
    u_int16_t app_id;
    u_int16_t sub_id;
}TIE_app_id;


typedef struct MSBC_info {
    struct MSBC_model *upstream;
    struct MSBC_model *downstream;
    struct TIE_app_id *TIE_app_table;
}MSBC_info;

/*This struct will be returned by any function which looks 
for a value in an array*/
typedef struct value_looked{
	int pos;
    float value;
}value_looked;


float gaus_function(float value,float mean,float variance);
int  find_max(float *num,int size,struct value_looked **found);
float max_probability(int value, struct MSBC_model *MSBC_temp);
float get_total_probability(int value,struct MSBC_model *MSBC_temp);

/*
This function implement the gaussian distribution
*/
#endif
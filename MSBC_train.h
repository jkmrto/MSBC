#ifndef MSBC_TRAIN_H_   /* Include guard */
#define MSBC_TRAIN_H_

#include <stdint.h>


typedef struct tie_app_sub_id_files{
    FILE *sub_app_file;
    uint32_t number_sessions_stored;
    int detected;
}tie_app_sub_id_files;


typedef struct tie_app_id_files {
    tie_app_sub_id_files *sub_id;
    uint16_t num_app_sub;
}tie_app_id_files;

typedef struct values_to_train{
	int32_t *value;
	uint32_t num_values;
}values_to_train;

typedef struct values_direction{
    values_to_train *downstream;
    values_to_train *upstream; 
}values_direction;

int init_train(struct tie_app_id_files **p_files_table);



#endif
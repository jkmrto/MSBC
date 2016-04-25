/*
 *  src/plugins/dummy/class_dummy.c - Component of the TIE v1.0.0-beta3 platform 
 *
 *  Copyright (C) 2007-2011 Alberto Dainotti, Walter de Donato,
 *                            Antonio Pescape', Alessio Botta.
 *  Email: alberto@unina.it, walter.dedonato@unina.it,
 *         pescape@unina.it, a.botta@unina.it 
 *
 *  DIS - Dipartimento di Informatica e Sistemistica (Computer Science Department)
 *  University of Naples Federico II
 *  All rights reserved.
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * The filename MUST begin with "class_" prefix and its name must be equal to
 * its folder under 'plugins' directory.
 * To enable the compilation of the plug-in add its name into enabled_plugins.
 *
 * Code HAVE TO be modified where the comment "Insert your code here" appears
 */
/*4
 * Dependences
 */
#include <string.h>
#include <stdlib.h>

#include "../plugin.h"
#include <sys/types.h>
#include <common/common.h>
#include <plugins/plugin.h>
#include <common/pkt_macros.h>

#include "plugins/MSBC/Aux_Functions.h"
#include "plugins/MSBC/MSBC_train.h"
#include "plugins/MSBC/Kmeans.h"
#include "plugins/MSBC/MSBC_conf.h"

/*
 * Constants and Macros
 */

#define MSBC_SIGN_FILE  "MSBC_signature_file.txt"
#define MSBC_MAP_FILE   "MSBC_map_file.txt"
#define MSBC_CONF_MAP_FILE   "MSBC_conf_map.txt" 


/*Hold the extern variable*/
 MSBC_variables MSBCv;
/*
 * Private functions
 */
int p_disable();
int p_enable();
bool p_is_session_classifiable(void *sess);
int p_load_signatures(char *error);
int p_train(char* path);
class_output* p_classify_session(void *sess);
int p_dump_statistics(FILE *fp);
int p_session_sign(void *sess, void *packet);

/*
 * Global variables
 */
char *name = NULL; /* Name is taken from filename and assigned by plug-in loader */
u_int32_t flags = 0;
u_int32_t stat_hits = 0;
u_int32_t stat_miss = 0;
u_int16_t MSBC_numProtocols_available;
/*This structure will be used to save the clusters trained for
each one of the protocols available for MSBC*/

/*Table to load the clusters of the protocols*/
struct MSBC_info *MSBC_table = NULL;

/*Table to save the direction of the files which store 
the size of the messages used to train the model*/
struct tie_app_id_files   *files_table = NULL;
int invert_down_upstream;  



/*
 * This is the external function called to initialize the classification engine
 */
int class_init(classifier *id)
{
    id->disable = p_disable;
    id->enable = p_enable;
    id->load_signatures = p_load_signatures;
    id->train = p_train;
    id->is_session_classifiable = p_is_session_classifiable;
    id->classify_session = p_classify_session;
    id->dump_statistics = p_dump_statistics;
    id->session_sign = p_session_sign;
    id->flags = &flags;
    id->version = VERSION;
    name = id->name;

    return (1);
}

/*
 * This is the classification engine algorithm
 *
 * This routine is called once for each session
 * when the p_is_session_classifiable() function
 * returns "true"
 */
class_output *p_classify_session(void *sess)
{

    class_output *result = malloc(sizeof(class_output));
    
    switch (pv.stype) {
        case SESS_TYPE_FLOW: {
            break;
        }
        case SESS_TYPE_BIFLOW: {
            struct biflow *s; 
            struct value_looked *min_dist_protocol;
            float probability_protocol[MSBC_numProtocols_available];
            int i,j;
            s = sess;
           // printf("numProtocols available:%d\n",MSBC_numProtocols_available);

            for (i = 0; i < MSBC_numProtocols_available;i++){
                probability_protocol[i] = 1;
            }

                 probability_protocol[0] = 0.0;

            for (i = 0 ;i < MSBC_numProtocols_available;i++){
                for(j = 0; j < pv.message_count; j++){
                    if((s->message_count[j]*invert_down_upstream) > 0){
                        //printf("positivo\n");
                        if(MSBC_table[i].upstream[j].size > 0){
                            if(MSBCv.calculate_probability = MAX_PROBABILITY){
                                probability_protocol[i] = probability_protocol[i]*max_probability(s->message_count[j],&MSBC_table[i].upstream[j]);
                            }else{
                                probability_protocol[i] = probability_protocol[i] * get_total_probability(s->message_count[j],&MSBC_table[i].upstream[j]);
                            }
                        }else{
                            probability_protocol[i] = MSBCv.no_gaussian_available * probability_protocol[i];  
                        }
                    }else{
                        if(MSBC_table[i].downstream[j].size > 0){
                            if(MSBCv.calculate_probability = MAX_PROBABILITY){
                                probability_protocol[i] = probability_protocol[i]*max_probability(s->message_count[j],&MSBC_table[i].downstream[j]);
                            }else{
                                probability_protocol[i] = probability_protocol[i] * get_total_probability(s->message_count[j],&MSBC_table[i].downstream[j]);    
                            }                        
                        }else{
                            probability_protocol[i] = MSBCv.no_gaussian_available * probability_protocol[i];          
                        }
                    }         
                }
            }

# ifdef DEBUG_MSBC
            printf("Numero de protocolos disponibles: %d",MSBC_numProtocols_available);
            for(i = 0;i < MSBC_numProtocols_available;i++){
                printf("\nDEBUG probability protocols\n");
                printf("Protocol ID:%d\t Protocolo SUBID:%d\t Protocol Confidence:%1.10f\t\n",MSBC_table[i].TIE_app_table->app_id,
                    MSBC_table[i].TIE_app_table->sub_id,probability_protocol[i]);
            }
#endif
            min_dist_protocol = calloc(1,sizeof(value_looked));
            find_max(probability_protocol,MSBC_numProtocols_available,&min_dist_protocol);
            result->id = MSBC_table[min_dist_protocol->pos].TIE_app_table->app_id;
            result->subid = MSBC_table[min_dist_protocol->pos].TIE_app_table->sub_id;
            result->confidence = probability_protocol[min_dist_protocol->pos]*100;
            stat_hits++;

            result->flags = 0;

            free(min_dist_protocol);

#ifdef DEBUG_MSBC
             printf("Protocol ID :%d\t",result->id);
             printf("Protocol SUBID: %d\n",result->subid);
             printf("Protocol Confidence %d\n",result->confidence);
#endif


        if ((s->message_count_length < pv.message_count) || 
            ((s->message_count_length == pv.message_count) && !s->prev_packet_end && (s->f_tuple.l4proto == L4_PROTO_TCP))){
            SET_BIT(s->flags, SESS_RECLASSIFY, 1);
        }else{
            SET_BIT(s->flags, SESS_CLASSIFIED,1);
        }

        break;
    }

        case SESS_TYPE_HOST: {
            break;
        }
    }

    return result;
}

/*
 * This function decides a session is ready to be classified
 *
 * This routine is called for each packet of a session that
 * has not yet been classified
 */
bool p_is_session_classifiable(void *sess)
{
    bool value_returned = false;

    switch (pv.stype) {
    case SESS_TYPE_FLOW: {
        struct flow *s;
        s = sess;

        break;
    }
    case SESS_TYPE_BIFLOW: {
        struct biflow *s;
        s = sess;

        if((s->prev_packet_end) == true){
            value_returned = true;

#ifdef DEBUG_MSBC
            printf("Session classifiable\n");
#endif 
        }
        
        break;
    }
    case SESS_TYPE_HOST: {
        struct host *s;
        s = sess;
        /* Insert your code here */
        break;
    }
    }

    return value_returned;
}

/*
 * This function disables the classifier
 */
int p_disable()
{
    SET_BIT(flags, CLASS_ENABLE, 0);
    return 1;
}

/*
 * This function enables the classifier
 *
 * This function is called just after class_init() and should
 * control if the plugin has enough information to classify.
 */
int p_enable()
{

    SET_BIT(flags, CLASS_ENABLE, 1); /* enable plugin by default */


    if(pv.message_count > 0) {

     if(pv.message_count != 5){
            printf("The recomendable number message per session is 5\n");
        }   
    }else{
        printf("MSBC needs to know how many message will be studied per session 5 recomendable (-U option)\n");
    SET_BIT(flags, CLASS_ENABLE, 0);   
    }



    if(pv.min_PDU > 0) {
        if(pv.min_PDU != 512){
            printf("The recomendable minmum PDU is 512\n");
        }
    }else{
        //printf("min PDU %d",pv.min_PDU);    
        printf("MSBC needs to establish the minimum PDU for considering that payload \nas the last of a message (-u option)\n");
    SET_BIT(flags, CLASS_ENABLE, 0);   
    }

        if (TEST_BIT(flags, CLASS_ENABLE, 1)){
           init_MSBC_default_conf();
            load_MSBC_conf();
            print_MSBC_conf();
 
    }


    return 1;
}

/*
 * This function loads signatures needed for classification
 * This function is called after the plugin has been enabled.
 * Here the plugin should read from some files its fingerprints
 * and store them somewhere in memory.
 */
int p_load_signatures(char *error)
{  
    FILE *fp_map; 
    FILE *fp_sign;
    char *field;
    char *row = malloc(MAX_BUFFER * sizeof(char));  /* Buffer to store file rows */
    char *sptr = NULL;
    char path[MAX_BUFFER], path_map_file[MAX_BUFFER],path_signature_file[MAX_BUFFER];
  	struct TIE_app_id *TIE_app_table;
  	struct MSBC_model *MSBC_model_temp;
    u_int16_t index = 0,Pos_Message;
    u_int16_t numProtocols_map;
    u_int16_t MSBC_ID_protocol;
    u_int8_t state_map = 0,state_sign = 0, Current_Gaus = 0;
    bool upstream;

    /*Init the conf of the plugin*/

    
    //printf("\nClusters:%d\n",MSBCv.clusters_required);

    if (strcmp(MSBCv.signature_file ,"own") != 0){
        sprintf(path_map_file ,"%s/plugins/%s_%s/%s_%s", tie_path, name, VERSION, "foreign",MSBC_MAP_FILE); 
        sprintf(path_signature_file ,"%s/plugins/%s_%s/%s_%s", tie_path, name, VERSION, "foreign",MSBC_SIGN_FILE); 
        invert_down_upstream = -1;
    }else{
        sprintf(path_map_file ,"%s/plugins/%s_%s/%s_%s", tie_path, name, VERSION, "own",MSBC_MAP_FILE);
        sprintf(path_signature_file ,"%s/plugins/%s_%s/%s_%s", tie_path, name, VERSION, "own",MSBC_SIGN_FILE);      
        invert_down_upstream = 1;
    }


    if ((fp_map = fopen(path_map_file, "r")) == NULL) {
        printf("\nERROR: Unable to open %s file!\n\n", path_map_file);
        return -1;
    } else { 
        printf("\nFile %s opened!\n\n", path_map_file); 
    }

    while(fgets(row, MAX_BUFFER,fp_map)){
    	 if (row[0] == '#' || row[0] == ' ')
            continue;
       switch(state_map){       		case(0):
       		field = strtok_r(row, " ", &sptr);
       		if(strcmp(field,"IdentifiedProtocols") == 0 ){
       			field = strtok_r(NULL, " ", &sptr);
       			numProtocols_map = atoi(field);
	       		TIE_app_table = calloc(numProtocols_map, sizeof(struct TIE_app_id));
                state_map=1;
                }
                break;
       		case(1):

       			field = strtok_r(row, ",", &sptr);
       			index = atoi(field);
       			SKIP_CHAR(sptr, '\t');
       			field = strtok_r(NULL, ",", &sptr);
				TIE_app_table[index].app_id = atoi(field);
				
				SKIP_CHAR(sptr, '\t');
				field = strtok_r(NULL, ",", &sptr);
                TIE_app_table[index].sub_id = atoi(field);
#if DEBUG > 1
				printf("%d \t",index);
				printf("%d \t",TIE_app_table[index].app_id);
				printf("%d \n",TIE_app_table[index].sub_id);	
#endif
       			break;

       }
    }


    if ((fp_sign = fopen(path_signature_file, "r")) == NULL) {
        printf("\nERROR: Unable to open %s file!\n\n", path_signature_file);
        return -1;
    } else { 
        printf("\nFile %s opened!\n\n", path_signature_file); 
    }
    
/*Index use to allocate the array of the MSBC_TABLE*/
index = 0;

while (fgets(row, MAX_BUFFER, fp_sign)) {
        /* Skip commented rows */
        if (row[0] == '#')
            continue;

        field = strtok_r(row, " ", &sptr);
 	
        switch (state_sign){
            case (0):
                if(strcmp(field,"NumProtocols")== 0 ){
                	field = strtok_r(NULL, " ", &sptr);
                    MSBC_numProtocols_available = atoi(field);
             		MSBC_table = calloc(MSBC_numProtocols_available, sizeof(struct MSBC_info));
                    state_sign++;
                }else{
                	//printf("error reading the MSBC_mapping_file.txt (NumProtocols)");
                }
                break;
            case (1):

            	if(strcmp(field,"Protocol")==0){
                	field = strtok_r(NULL, " ", &sptr);
                    MSBC_ID_protocol = atoi(field);
                    MSBC_table[index].TIE_app_table = calloc(pv.message_count, sizeof(struct TIE_app_id));
             		(*MSBC_table[index].TIE_app_table).app_id = TIE_app_table[MSBC_ID_protocol].app_id;
             		(*MSBC_table[index].TIE_app_table).sub_id = TIE_app_table[MSBC_ID_protocol].sub_id;
#ifdef DEBUG
                    printf("MSBC ID: %d \t",MSBC_ID_protocol);
                    printf("TIE app_id: %d \t",(*MSBC_table[index].TIE_app_table).app_id);
                    printf("TIE sub_id: %d \n",(*MSBC_table[index].TIE_app_table).sub_id);
#endif  
                    state_sign++;


                }else{
                	printf("Error reading the MSBC_mapping_file.txt (Protocol)");
                }
                break;
            case (2):

            	if(strcmp(field,"Upstream") == 0 ){
#ifdef DEBUG
            		printf("Upstream = %s\n",field);
#endif             	
            		upstream = true;
            		state_sign++;	
            		MSBC_table[index].upstream = calloc(pv.message_count, sizeof(struct MSBC_model));

            	}else if(strcmp(field,"Downstream") == 0 ){
#ifdef DEBUG
            		printf("Downstream = %s\n",field);
#endif
            		MSBC_table[index].downstream = calloc(pv.message_count, sizeof(struct MSBC_model));
                    upstream = false;
            		state_sign++;	
            	}else{
                	printf("Error reading the MSBC_mapping_file.txt (Upstream)");
            	}
                break;
             case (3):

#ifdef DEBUG
                    printf("Pos = %s\t",field);
#endif
            	if(strcmp(field,"Pos")== 0 ){

            		field = strtok_r(NULL, " ", &sptr);
#ifdef DEBUG
                    printf("\tPosicion = %s\n",field);
#endif
                    Pos_Message = atoi(field);
                   
            		if(upstream){
                        //&(MSBC_table[index].upstream[Pos_Message]) = calloc(1, sizeof(MSBC_model));
            			MSBC_model_temp = &(MSBC_table[index].upstream[Pos_Message]);
            		}else{
                        //&(MSBC_table[index].downstream[Pos_Message]) = calloc(1, sizeof(MSBC_model));
            			MSBC_model_temp = &(MSBC_table[index].downstream[Pos_Message]);
            		}

            		state_sign++;
            	}else{
                	printf("error reading the MSBC_mapping_file.txt (Pos)\n");
                }
                break;
            case (4):
            	if(strcmp(field,"NumGaussians") == 0 ){            		
            		field = strtok_r(NULL, " ", &sptr);
                    MSBC_model_temp->size = atoi(field);
#ifdef DEBUG 
            		printf("NumGaussians = %d",MSBC_model_temp->size);
                    if(upstream){
                        printf("\tNumGaussians = %d\n",MSBC_table[index].upstream[Pos_Message].size);
                    }else{
                      printf("\tNumGaussians = %d\n",MSBC_table[index].downstream[Pos_Message].size);  
                    }
#endif
            		if(atoi(field) == 0){ 
            			printf("Empty cluster found at protocol number: %d,  message: %d-%s \n",MSBC_ID_protocol, Pos_Message, upstream ? "upstream" : "downstream");
            			if( Pos_Message != (pv.message_count - 1)){
	       					state_sign = 3;
	       				}else{
		       				if(upstream){
			       				state_sign = 2;
			       			}else{
			       				state_sign = 1;
                                index++;
			       			}
			       		}		
		       		
            		}else{
                     MSBC_model_temp->means = calloc(MSBC_model_temp->size, sizeof(u_int32_t));
                     MSBC_model_temp->weight = calloc(MSBC_model_temp->size, sizeof(float));
                     MSBC_model_temp->variance = calloc(MSBC_model_temp->size, sizeof(float));
                     Current_Gaus = 0;
#ifdef DEBUG                       
            		printf("size %d \n",MSBC_model_temp->size);
#endif
            		state_sign++;
            		}
            	}else{
                	printf("error reading the (NumGaussians,%s)\n", MSBC_SIGN_FILE);
                }
                break;
       		case(5):
       			MSBC_model_temp->means[Current_Gaus] = atof(field);
       			field = strtok_r(NULL, " ", &sptr);
       			MSBC_model_temp->weight[Current_Gaus] = atof(field);
       			field = strtok_r(NULL, " ", &sptr);
       			MSBC_model_temp->variance[Current_Gaus] = atof(field);
#ifdef DEBUG 
       			printf("%f\t", MSBC_model_temp->means[Current_Gaus]);
       			printf("%f\t", MSBC_model_temp->variance[Current_Gaus]);
       			printf("%f\n", MSBC_model_temp->weight[Current_Gaus]);
#endif
       			Current_Gaus++;

				/* If it is the last element for this position of the message,
				we test if another protocol will come or will be the downstream cluster*/
       			
	       		if(Current_Gaus == MSBC_model_temp->size){
	       			if( Pos_Message != pv.message_count - 1){
	       				state_sign = 3;
	       			}else{
		       			if(upstream){
			       			state_sign = 2;
			       		}else{
			       			state_sign = 1;
                            index++;
			       		}	
		       		}
	       		}
	           
	            break;
           			     	
        }
 }           
        
        #ifdef DEBUG  
                /*for (i = 0; i < MSBC_numProtocols_available; i++){
                    printf("TIE app_id: %d \t",(*MSBC_table[i].TIE_app_table).app_id);
                    printf("TIE sub_id: %d \n",(*MSBC_table[i].TIE_app_table).sub_id);

                    printf("Upstream\n");
                    for (k = 0; k < NUMBER_MESSAGES; k++){ 
                        printf("Pos %d\n",k);
                        printf("NumGaussians: %d\n", MSBC_table[i].upstream[k].size);
                        for (j = 0; j < MSBC_table[i].upstream[k].size; j++){
                            //printf("%f\t", MSBC_table[i].upstream[k].means[j]);
                            //printf("%f\t", MSBC_table[i].upstream[k].variance[j]);
                            //printf("%f\n", MSBC_table[i].upstream[k].weight[j]);

                        }
                    }

                    printf("Downstream\n");
                    for (k = 0; k< NUMBER_MESSAGES;k++ ){ 
                        printf("Pos %d\n",k);
                        printf("NumGaussians: %d\n", MSBC_table[i].downstream[k].size);
                        for (j = 0; j < MSBC_table[i].downstream[k].size; j++){    
                            printf("%f\t", MSBC_table[i].downstream[k].means[j]);
                            printf("%f\t", MSBC_table[i].downstream[k].variance[j]);
                            printf("%f\n", MSBC_table[i].downstream[k].weight[j]);

                        }
                    }

             }*/
#endif

    /* Close file and free buffer */
    fclose(fp_sign);
    fclose(fp_map);
    free(row);




    return 0;
}

/*
 * This function saves signatures obtained by p_session_sign to file
 *
 * This function is called at the end of tie execution and should
 * store in some files the results of the fingerprint collection
 * in a format readable by load_signatures() function.
 */

 /*Queda por definir como se establece el peso*/
int p_train(char *path)
{
    FILE *fp_training_output; /*File to write the training results*/
    char *row = malloc(MAX_BUFFER * sizeof(char));  /* Buffer to store file rows */
    char *sptr = NULL;

    char path1[MAX_BUFFER];
    char *field;
    int i,j,k,l;

    int value_payload = 0;
    /*This values are neccessary to load the signature file*/
    int count_detected_protocols = 0,count_protocol = 0;
    
    /*struct to identify where is the file which
    contain the sign of the procol*/
    struct tie_app_sub_id_files *training_values;
    /*To load the values to train for each protocol*/
    struct values_direction temp_values_direction;
    struct values_to_train *temp_values;
    
    /*Used for the K-means algorithm*/
    int n_clusters_required = 15;
    int temp_n_clusters_required;
    
    /*Used for the gnuplot*/
    //int terminal_index = 0;

    /*Use in the case to avoid repeat te code*/
    int state_direction = 0; 

    struct clusters_trained *temp_clusters_result;

    sprintf(path1 ,"%s/plugins/%s_%s/Training_output/own_MSBC_signature_file.txt", tie_path, name,VERSION);
    if ((fp_training_output = fopen(path1, "w+")) == NULL) {
        printf("\nERROR: Unable to open %s file!\n\n", path1);
         return -1;
     } 

     /*Leaving a line to indicate the number of protocols studied*/
     fprintf(fp_training_output,"NumProtocols         \n");

    for (j = 0; j< MSBCv.num_protocols_train;j++){    
        for (i = 0;i < (files_table)[j].num_app_sub;i++){
            //printf("j:%d,i:%d\n",j,i);
            training_values = &((files_table)[j].sub_id[i]);
/*Count the number of protocols in order to make this implementation compatible
with the other, which used a map file to engage id and sub_id with the numer of protocol*/
                count_protocol++;
            if((files_table[j]).sub_id[i].detected == 1){ 
            //printf("j:%d,i:%d\n",j,i);       
                fprintf(fp_training_output,"Protocol %d\n",count_protocol-1);
                count_detected_protocols++;
                
                temp_values_direction.downstream = calloc((pv.message_count),sizeof(struct values_to_train));
                temp_values_direction.upstream = calloc(pv.message_count,sizeof(struct values_to_train));
                        
                for(k = 0;k < pv.message_count;k++){
                    temp_values_direction.downstream[k].value = calloc((training_values->number_sessions_stored),sizeof(uint32_t));
                    temp_values_direction.upstream[k].value = calloc((training_values->number_sessions_stored),sizeof(uint32_t));
                }
                    rewind(training_values->sub_app_file);

                    while(fgets(row, MAX_BUFFER,training_values->sub_app_file)){
                        if (row[0] == '#' || row[0] == ' ')
                            continue;
                        
                        for(k = 0; k < pv.message_count;k++){
                            if(k == 0){
                                field = strtok_r(row, ",", &sptr);
                            }else{
                                field = strtok_r(NULL, ",", &sptr);
                            }
                            value_payload = atoi(field);
                            //printf("temp_value:%d\n",value_payload);
                            if(value_payload > 0){
                                temp_values_direction.upstream[k].value[temp_values_direction.upstream[k].num_values++] = value_payload;
                                //printf("%d\t",temp_values_direction.upstream[k].value[temp_values_direction.upstream[k].num_values-1]);
                            }else{
                                temp_values_direction.downstream[k].value[temp_values_direction.downstream[k].num_values++] = value_payload;     
                                //printf("%d\t",temp_values_direction.downstream[k].value[temp_values_direction.downstream[k].num_values-1]);
                            }
                        SKIP_CHAR(sptr, '\t');    
                        }
                        //printf("\n");
                    }
                    

                    for (state_direction = 0; state_direction < 2; state_direction++){
                        
                        switch(state_direction){
                            case(0):{
                                fprintf(fp_training_output,"Upstream \n"); 
                                temp_values = temp_values_direction.upstream;
                                break;
                            }
                            case(1):{
                                fprintf(fp_training_output,"Downstream \n");
                                temp_values = temp_values_direction.downstream;
                                break;
                            }
                        }
        
                         
                        for(k = 0; k < pv.message_count;k++){


                                fprintf(fp_training_output,"Pos %d\n",k);

                                temp_clusters_result = calloc(1,sizeof(struct clusters_trained));   
                                temp_clusters_result->clusters = malloc(MSBCv.clusters_required * sizeof(struct cluster));
        
                           if(K_means_algorithm(&temp_values[k],temp_clusters_result) == 1){
                                fprintf(fp_training_output,"NumGaussians %d\n",temp_clusters_result->num_clusters);
                                 for(l = 0;l < temp_clusters_result->num_clusters;l++){
                                        if(temp_clusters_result->clusters[l].variance < MSBCv.min_variance){
                                            temp_clusters_result->clusters[l].variance = MSBCv.min_variance;
                                        }
                                        fprintf(fp_training_output,"%1.10f %1.10f %1.10f\n",temp_clusters_result->clusters[l].center, 
                                            temp_clusters_result->clusters[l].weight, temp_clusters_result->clusters[l].variance);
                                }
                            }
                        free(temp_clusters_result->clusters);
                        free(temp_clusters_result);
                        }
                    } 
            for(k = 0;k < pv.message_count;k++){
                free(temp_values_direction.downstream[k].value); 
                free(temp_values_direction.upstream[k].value);
            }
            free(temp_values_direction.downstream); 
            free(temp_values_direction.upstream);
            
            }

            fclose(training_values->sub_app_file);
           // free(training_values);
        }
         free(files_table[j].sub_id);
    }
    rewind(fp_training_output);  
    fprintf(fp_training_output,"NumProtocols %d",count_detected_protocols);
    fclose(fp_training_output);

    free(row);
    free(files_table);

    return 0;

}

/*
 * This function prints some statistics on classification
 * to the file pointed by fp
 */
int p_dump_statistics(FILE *fp)
{
    if (pv.class) {
        fprintf(fp, "%s\t| %d\t| %d\n", name, stat_hits, stat_miss);
    }
    return 0;
}

/*
 * This function is called for each packet with PS > 0 of a session until
 * the flag SESS_SIGNED is set. It should be used to store information
 * about sessions useful to fingerprint collection
 */
int p_session_sign(void *sess, void *packet)
{
    switch (pv.stype) {
        case SESS_TYPE_FLOW: {

            break;
        }
        case SESS_TYPE_BIFLOW: {
            struct tie_app_sub_id_files *sub_app_id_temp;
            struct biflow *s;
            s = sess;

            uint16_t app_id_temp;
            int i;

            if( files_table == NULL){
                if(init_train(&files_table) != 0){
                    printf("Error initializing the training");
                }
            }

            if((s->prev_packet_end) && (s->message_count_length == pv.message_count)){
                //printf("\nl.700 Momento para clasificar");
                //printf("\n%d, %d\n",s->app.id,s->app.subid);
      
                sub_app_id_temp = &(files_table[s->app.id].sub_id[s->app.subid]);
                sub_app_id_temp->number_sessions_stored++;
                sub_app_id_temp->detected = 1; 
                 for(i = 0; i < pv.message_count;i++){
                    fprintf(sub_app_id_temp->sub_app_file,"%d,\t",s->message_count[i]);
                 }

                 fprintf(sub_app_id_temp->sub_app_file,"\n");
                 SET_BIT(s->flags, SESS_SIGNED, 1);
            }
            
            break;
        }
        case SESS_TYPE_HOST: {

            break;
        }
    }
    return 0;
}



 
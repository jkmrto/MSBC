
#include <string.h>
#include <stdlib.h>

#include <common/common.h>
#include <sys/types.h>

#include "MSBC_train.h"
#include "MSBC_conf.h"

#ifndef MAX_BUFFER
#define MAX_BUFFER  255
#endif

#ifndef name
#define name "MSBC"
#endif

#ifndef VERSION
#define VERSION "1.1"
#endif

#define MSBC_TRAIN_CONF_FILE "MSBC_train_conf_file.txt"
#define OWN_MSBC_MAP_FILE 		"own_MSBC_map_file.txt"
#define MSBC_TIE_APP_ID      "TIE_apps.txt"

#define SKIP_CHAR(str,c)    while (*str == c) { str++; }

int init_train(struct tie_app_id_files **p_files_table){
   FILE *fp_conf_train_file; 
   FILE *fp_own_map_file = NULL;
   FILE *fp_apps_id;
   struct tie_app_id_files   *files_table;

   //bool print_map_file = false;
   int count_protocol= 0;
   uint16_t app_id_temp;

   struct tie_app_sub_id_files *sub_app_id_temp;

   char *field;
   char *row = malloc(MAX_BUFFER * sizeof(char));  /* Buffer to store file rows */
   char *sptr = NULL;
   char path[MAX_BUFFER];

   int i,j,k;

//Init the own_MSBC_map.txt
  sprintf(path ,"%s/plugins/%s_%s/%s", tie_path, name, VERSION, OWN_MSBC_MAP_FILE);
   if(MSBCv.print_map_file){
         if ((fp_own_map_file = fopen(path, "w")) == NULL) {
               printf("\nERROR: Unable to open %s file!\n\n", path);
                  return -1;
         }else{
               fprintf(fp_own_map_file,"IdentifiedProtocols          \n"); /*Leave the gap for the number of protocol counted*/
               fprintf(fp_own_map_file,"#N\tApp_ID\tSub_ID\n"); 
         }
   }

//Init the number of structs for the application
   if(MSBCv.num_protocols_train== 0){
      printf("Error loading the number of protocols");
   }else{
      files_table = calloc((MSBCv.num_protocols_train),sizeof(struct tie_app_id_files));   
      if(files_table == NULL){
         printf("Error getting memory for the direction of the temporal files of the protocols (MSBC trainning)");
      }
   }

   sprintf(path ,"%s/plugins/%s_%s/%s", tie_path, name, VERSION, MSBC_TIE_APP_ID);
   
// Loop to get the amount of subaplications
   if ((fp_apps_id = fopen(path, "r+")) == NULL) {
      printf("\nERROR: Unable to open %s file!\n\n", path);
   	return -1;
   }else{ 
		printf("\nFile %s opened!\n\n", path); 
                    
      while(fgets(row, MAX_BUFFER,fp_apps_id)){

      if (row[0] == '#' || row[0] == ' ')
         continue;

         field = strtok_r(row, ",", &sptr);
         app_id_temp = atoi(field);
         
         if(MSBCv.print_map_file)     
            fprintf(fp_own_map_file,"%d,\t%d,\t",count_protocol,atoi(field));
                       
         SKIP_CHAR(sptr, '\t');
         field = strtok_r(NULL, ",", &sptr);
         files_table[app_id_temp].num_app_sub =  atoi(field)+1;   
         
         if(MSBCv.print_map_file){
            fprintf(fp_own_map_file,"%d,\n",atoi(field));
            count_protocol++;
         }
      }
   }

   /*Print the number of identified protocol to the map file*/
   if(MSBCv.print_map_file){
      rewind(fp_own_map_file);
      fprintf(fp_own_map_file,"IdentifiedProtocols  %d",count_protocol);
      fclose(fp_own_map_file);
   }
            
   fclose(fp_apps_id);
             
   //Make the directory of the temp files
   sprintf(path,"mkdir %s/plugins/%s_%s/Training_output",tie_path, name, VERSION);
   system(path);

   sprintf(path,"mkdir %s/plugins/%s_%s/Training_output/Temp_files",tie_path, name,VERSION);
   system(path);

   /*Creating all the temp files for the protocols*/
   for(i = 0;i < MSBCv.num_protocols_train;i++){
      files_table[i].sub_id = calloc(files_table[i].num_app_sub,sizeof(struct tie_app_sub_id_files));
      for(j = 0;j < files_table[i].num_app_sub;j++){                      
      	sprintf(path ,"%s/plugins/%s_%s/Training_output/Temp_files/%d_%d_app.txt", tie_path, name, VERSION,i,j);
      	files_table[i].sub_id[j].sub_app_file = fopen(path, "w+");
      	sub_app_id_temp = &(files_table[i].sub_id[j]);

      	if(sub_app_id_temp->sub_app_file == NULL){
         	printf("\nError al generar el fichero: %s",path);
      	}else{
         	fprintf(sub_app_id_temp->sub_app_file,"#Temporal file used to store the sizes of the messages\n"); 
         	for(k = 0; k < pv.message_count; k++){
    	      	fprintf(sub_app_id_temp->sub_app_file,"#Pos: %d\t",k);
         	}
      	fprintf(sub_app_id_temp->sub_app_file,"\n");
      	} 
      }
   }

   free(row);
   //fclose(fp_conf_train_file);

   *p_files_table = files_table;

   return 0;
}









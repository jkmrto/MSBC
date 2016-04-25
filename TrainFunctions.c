#define MSBC_TRAIN_CONF_FILE "MSBC_train_conf_file.txt"

#include <string.h>
#include <stdlib.h>


int init_train(int *numProtocols_train,struct tie_app_id_files *files_table){
   FILE *fp_conf_train_file;  

   char *field;
   char *row = malloc(MAX_BUFFER * sizeof(char));  /* Buffer to store file rows */
   char *sptr = NULL;
   char path[MAX_BUFFER];

    fp_conf_train_file

    printf("\nCargando el fichero de configuracion");
    sprintf(path ,"%s/plugins/%s_%s/%s", tie_path, name, VERSION, MSBC_TRAIN_CONF_FILE);

    if ((fp_conf_train_file = fopen(path, "r")) == NULL) {
        printf("\nERROR: Unable to open %s file!\n\n", MSBC_TRAIN_CONF_FILE);
    return -1;
        } else { 
        printf("\nFile %s opened!\n", MSBC_TRAIN_CONF_FILE); 
    }
                
   while(fgets(row, MAX_BUFFER,fp_conf_train_file)){
      if (row[0] == '#' || row[0] == ' ')
         continue;
      if(i == 0 ){
         field = strtok_r(row, "\t ", &sptr);
         }else{
         field = strtok_r(row, "\t ", &sptr) ;
      }

      if (strcmp(field,"NumProtocols") == 0){
         field = strtok_r(NULL, "\t ", &sptr);
         *numProtocols_train = atoi(field);
         printf("\nNumero de protocolos:%s\n",field);
         }
                        if (strcmp(field,"PrintMapping") == 0){
                            field = strtok_r(NULL, "\t ", &sptr);
                                if (strcmp(field,"Yes") == 0){
                                    print_map_file = true;
                                    printf("Print map file\n\n");
                                    sprintf(path ,"%s/plugins/%s_%s/%s", tie_path, name, VERSION, OWN_MSBC_MAP_FILE);
                                    if ((fp_own_map_file = fopen(path, "w")) == NULL) {
                                        printf("\nERROR: Unable to open %s file!\n\n", path);
                                        return -1;
                                    }else{
                                        fprintf(fp_own_map_file,"IdentifiedProtocols          \n"); /*Leave the space for the number of protocol counted*/
                                        fprintf(fp_own_map_file,"#N\tApp_ID\tSub_ID\n"); 
                                    }
                                }
                        }   
                    }

                if(numProtocols_train == 0){
                    printf("Error loading the number of protocols");
                }else{
                    files_table = calloc((numProtocols_train),sizeof(struct tie_app_id_files));   
                    if(files_table == NULL){
                        printf("Error getting memory for the direction of the temporal files of the protocols (MSBC trainning)");
                    }
                }

                printf("\nCargando la identidad de las app\n");
                sprintf(path ,"%s/plugins/%s_%s/%s", tie_path, name, VERSION, MSBC_TIE_APP_ID);

                if ((fp_apps_id = fopen(path, "r+")) == NULL) {
                   printf("\nERROR: Unable to open %s file!\n\n", path);
                    return -1;
                } else { 

                    printf("\nFile %s opened!\n\n", path); 
                    
                    while(fgets(row, MAX_BUFFER,fp_apps_id)){

                       if (row[0] == '#' || row[0] == ' ')
                            continue;

                        field = strtok_r(row, ",", &sptr);
                        app_id_temp = atoi(field);
                        //printf("app_id:%d\t",app_id_temp);
                        if(print_map_file)     
                            fprintf(fp_own_map_file,"%d,\t%d,\t",count_protocol,atoi(field));
                        SKIP_CHAR(sptr, '\t');
                        
                        field = strtok_r(NULL, ",", &sptr);
                        files_table[app_id_temp].num_app_sub =  atoi(field)+1;   
                        //printf("num_app_sub_id:%d\n",files_table[app_id_temp].num_app_sub);
                        if(print_map_file){
                            fprintf(fp_own_map_file,"%d,\n",atoi(field));
                            count_protocol++;
                        }
                     }
                }

                 rewind(fp_own_map_file);
                 fprintf(fp_own_map_file,"IdentifiedProtocols  %d",count_protocol);
                 
                 fclose(fp_own_map_file);
                 fclose(fp_apps_id);
             
                    //Make the directory
                    sprintf(path,"mkdir %s/plugins/%s_%s/Training_output",tie_path, name, VERSION);
                    system(path);

                    sprintf(path,"mkdir %s/plugins/%s_%s/Training_output/Temp_files",tie_path, name,VERSION);
                    system(path);

                   for(i = 0;i < numProtocols_train;i++){
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
                    
                    /*Close the files use to init the train*/
                    FREE(row);
                    FREE(row);
                    fclose(fp_conf_train_file);
}









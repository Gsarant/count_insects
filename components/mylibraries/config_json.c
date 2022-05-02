#include "confing_json.h"
void write_default_values_to_rec_config_file(){
    strcpy(config_json_string, "{  \"index\": 1, \"version_config\": 0.13, \"DeviceID\": \"ad803f444d93f80e\",\
         \"url_send_data\": \"http://insectronics.net/binaries/ad803f444d93f80e\",\
         \"version_firmware\": 0.01,\
         \"url_update_ota_firmware\": \"https://firmware.insectronics.net/count_insect_gsm_02.bin\",\
         \"url_update_config\": \"http://insectronics.net/settings/ad803f444d93f80e\",\
         \"time_sleep\": 86400, \"flash_power\": 5, \"flash_time\": 1000,\
          \"detect\":1, \"sendphotos\":1,  \
          \"wifi_ssid\":\"sonic\", \"wifi_password\":\"tr3l0k0m10\" }");
    root = cJSON_Parse(config_json_string);

}

int get_int_value(const char *name){
    return cJSON_GetObjectItem(root,name)->valueint;
}
double get_double_value(const char *name){
    return cJSON_GetObjectItem(root,name)->valuedouble;
}
char *get_string_value(const char *name){
    return cJSON_GetObjectItem(root,name)->valuestring;
}

esp_err_t set_int_value(const char *name,int newvalue){
    if (cJSON_AddNumberToObject(root, name, newvalue)==NULL) {
        return ESP_FAIL;
    }
    else{
        return ESP_OK;
    }
}
esp_err_t set_double_value(const char *name,double newvalue){
    if (cJSON_AddNumberToObject(root, name, newvalue)==NULL) {
        return ESP_FAIL;
    }
    else{
        return ESP_OK;
    }
}
esp_err_t set_string_value(const char *name,char *newvalue){
    if (cJSON_AddStringToObject(root, name, newvalue)==NULL) {
        return ESP_FAIL;
    }
    else{
        return ESP_OK;
    }
}


esp_err_t read_config_file_json(const char *filename){
    
    struct stat st;
    if (stat(filename, &st) != 0) {
        ESP_LOGE(TAGCONFIG_FILE, "Failed to open file for reading");
        return ESP_FAIL;
    }        
    if((int)st.st_size<300){
        ESP_LOGE(TAGCONFIG_FILE, "Probably Corrupted config file is too small");
        return ESP_FAIL;
    }

    ESP_LOGI(TAGCONFIG_FILE,"Open read file %s", filename);
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        ESP_LOGE(TAGCONFIG_FILE, "Failed to open file for reading");
        return ESP_FAIL;
    }
    else{
        //char config_json_string[(int)st.st_size];
        fread(config_json_string, (int)st.st_size, 1, f);
        fclose(f); 
        //ESP_LOGI(CONFIG_FILE_NAME, "Read file Data %s size of file %d",config_json_string,(int)st.st_size);
        root = cJSON_Parse(config_json_string);
        ESP_LOGI(TAGCONFIG_FILE, "\n Read  Config file from SD size =%d",strlen(config_json_string));
        return ESP_OK;
    }    
};




esp_err_t write_config_file_json(const char *filename){
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        ESP_LOGE(TAGCONFIG_FILE, "Failed to open file %s for writing ",filename);
        return ESP_FAIL;
    }
    else{
	    strcpy(config_json_string, cJSON_Print(root));
        if(strlen(config_json_string)==0){
            ESP_LOGE(TAGCONFIG_FILE, "Failed to convert rec_config_file to string ");
            return ESP_FAIL;
        }
        //ESP_LOGD(TAGCONFIG_FILE,"\n config_json_string  len=%d  \n%s",strlen(config_json_string),config_json_string);
        ESP_LOGI(TAGCONFIG_FILE, "open file %s for writing ",filename);
        //size_t write_size=fwrite(config_json_string,strlen(config_json_string),1,f);
        size_t write_size=strlen(config_json_string);
        if(write_size<=0){
            ESP_LOGE(TAGCONFIG_FILE, "Failed to write in file %s size %d", filename,write_size);
            fclose(f); 
            return ESP_FAIL;
        }
        fclose(f); 
        ESP_LOGI(TAGCONFIG_FILE, "\nWrite Config  file to SD size=%d",strlen(config_json_string));
        return ESP_OK;
    }

};


esp_err_t read_config_nvs(){
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE_CONFIG, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAGCONFIG_FILE,"Error Open Confing nvs");
        return err;
    }
    size_t tmp_configsize=0;
    err = nvs_get_blob(my_handle, key_con,NULL , &tmp_configsize);
    if(tmp_configsize==0){
        ESP_LOGE(TAGCONFIG_FILE,"Read nvs Nothin in Config to read ");
        return ESP_FAIL;
    }
    else{
        char config_json_string[(int)tmp_configsize];
        err = nvs_get_blob(my_handle, key_con,config_json_string , &tmp_configsize);
        if (err != ESP_OK) {
            ESP_LOGE(TAGCONFIG_FILE,"Error Get Blob  Config nvs");
            return err;
        }
        root = cJSON_Parse(config_json_string);

    }
    nvs_close(my_handle);
    ESP_LOGI(TAGCONFIG_FILE,"\n Read  Config nvs size= %d \n ",sizeof(config_json_string));
    return ESP_OK;
}

esp_err_t save_config_nvs(){
     nvs_handle_t my_handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE_CONFIG, NVS_READWRITE, &my_handle);
    if (err != ESP_OK){
        ESP_LOGE(TAGCONFIG_FILE,"Error Open  Config nvs");        
        return err;
    } 
    
    strcpy(config_json_string, cJSON_Print(root));
    

    if(strlen(config_json_string)<=0){
        ESP_LOGE(TAGCONFIG_FILE,"Nothing to Save");
        return ESP_FAIL;
    }else{
        err = nvs_set_blob(my_handle, key_con, config_json_string, strlen(config_json_string));   
        if (err != ESP_OK) {
            ESP_LOGE(TAGCONFIG_FILE,"Can not write Config nvs");
            return err;
        }
        err = nvs_commit(my_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAGCONFIG_FILE,"Can not commit Config nvs !!!!!!!!!!!");
            return err;
        }
    }
    nvs_close(my_handle);
    ESP_LOGI(TAGCONFIG_FILE,"\n Save Config nvs \n");
    return ESP_OK;
}


void getSubstring(char *dst,  const char *src, char *start, char *end)
{
    char *p=NULL;
    p=strchr(src,start);
    if(p!=NULL){
        char *p2=NULL;
        p2=strchr(p,end);
        if(p2!=NULL){
            size_t len   = strlen(p)- strlen(p2)+1;
            strncpy(dst,p,len);
        }
        else{
            dst=NULL;    
        }
    }
    else{
        dst=NULL;
    }
}

void check_update_config_task(void *pvParameter) {
		ESP_LOGI(TAGCONFIG_FILE,"Looking for a new Configuration ...\n");
		Rec_parameters_config_task *rec_parameters_config_task;
    	rec_parameters_config_task=(Rec_parameters_config_task*)pvParameter;
        
        ESP_LOGD(TAGCONFIG_FILE,"try to Connect URL %s checking for a new config file",rec_parameters_config_task->url_update_config_json);
    	// configure the esp_http_client
		esp_http_client_config_t config = {
            .url = rec_parameters_config_task->url_update_config_json,
            .method=HTTP_METHOD_GET,
            .transport_type=HTTP_TRANSPORT_OVER_TCP,
		};
		esp_http_client_handle_t client = esp_http_client_init(&config);
         
        //esp_http_client_set_header(client, "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8");
        //esp_http_client_set_header(client, "Accept-Encoding", "gzip, deflate, br");
        esp_http_client_set_header(client, "Accept-Encoding", "identity");
        esp_http_client_set_header(client, "Accept-Language", "en-US,en;q=0.5");
	    esp_http_client_set_header(client, "Cache-Control","no-cache");
		
        // downloading the json file
		//esp_err_t err = esp_http_client_perform(client);
        esp_err_t err = esp_http_client_open(client,0);
        if (err != ESP_OK) {
            ESP_LOGE(TAGCONFIG_FILE, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        } else {
            int MAX_HTTP_OUTPUT_BUFFER=2000;
            char output_buffer[MAX_HTTP_OUTPUT_BUFFER]; 
            int data_read=0;
            int content_length = esp_http_client_fetch_headers(client);
            if (content_length < 0) {
                ESP_LOGE(TAGCONFIG_FILE, "HTTP client fetch headers failed");
            } else {
                data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
                if (data_read >= 0) {
                    output_buffer[data_read]=0;
                    //ESP_LOGV(TAGCONFIG_FILE,"\n Downloaded config file  len is %d  string is %s \n ",data_read,output_buffer);
                    char json_buffer[MAX_HTTP_OUTPUT_BUFFER];
                    memset( json_buffer, 0, sizeof(json_buffer) );
                    getSubstring(json_buffer,output_buffer,'{','}');

                    ESP_LOGV(TAGCONFIG_FILE,"\n json_buffer len %d string is %s \n ",strlen(json_buffer),json_buffer);
                  	
                    if(json_buffer!=NULL)
                    {
                        cJSON *json2 = cJSON_Parse(json_buffer);
                        
                        if(json2 == NULL) {
                            ESP_LOGE(TAGCONFIG_FILE,"downloaded file config json is not a valid json, aborting...\n");
                        }else {	
                            cJSON *version = cJSON_GetObjectItemCaseSensitive(json2, "version_config");
                            
                            double new_version = version->valuedouble;
                            double prev_version=get_double_value("version_config");
                            if(new_version > prev_version) {
                                
                                strcpy(config_json_string,json_buffer);
                                root = cJSON_Parse(config_json_string);
                                double cur_version=get_double_value("version_config");
                                ESP_LOGI(TAGCONFIG_FILE," Update new Config Version %.4lf... \n",cur_version);
                            }else{
                                ESP_LOGI(TAGCONFIG_FILE," Keep Config Version %.4lf... \n",prev_version);
                            }
                        }
                    }

                } else {
                    ESP_LOGE(TAGCONFIG_FILE, "Failed to read response");
                }
            }
        }
        esp_http_client_cleanup(client);
        esp_http_client_close(client);
        xEventGroupSetBits(http_config_event_group, BIT0);
        vTaskDelete(NULL);
}
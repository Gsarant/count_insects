#include "confing_file.h"
void write_default_values_to_rec_config_file(Rec_config_file_t *rec_config_file){
    rec_config_file->index=0;
    rec_config_file->version_config=0.1;
    //strcpy(rec_config_file->last_image_file_name,"none");
    
    strcpy(rec_config_file->DeviceID,"123");
    strcpy(rec_config_file->url_send_data, "http://");
    strcpy(rec_config_file->url_update_ota_json , "http://");
    strcpy(rec_config_file->url_update_config , "http://");
    
    rec_config_file->time_sleep = 60;
    rec_config_file->flash_power=10;
    rec_config_file->flash_time=1000;
    rec_config_file->sendphotos=1;
    rec_config_file->detect=1;
    strcpy(rec_config_file->wifi_ssid , "ssid");
    strcpy(rec_config_file->wifi_password , "password");
}

void read_config_string(const char *config_json_string,Rec_config_file_t *rec_config_file){
    //ESP_LOGI(TAGCONFIG_FILE, "read_config_string input string %s",config_json_string);
    cJSON *root = cJSON_Parse(config_json_string);
    rec_config_file->index = cJSON_GetObjectItem(root,"index")->valueint;
    rec_config_file->version_config= cJSON_GetObjectItem(root,"version_config")->valuedouble;
    strcpy(rec_config_file->DeviceID , cJSON_GetObjectItem(root,"DeviceID")->valuestring);
    //strcpy(rec_config_file->last_image_file_name , cJSON_GetObjectItem(root,"last_image_file_name")->valuestring);
    strcpy(rec_config_file->url_send_data , cJSON_GetObjectItem(root,"url_send_data")->valuestring);
    strcpy(rec_config_file->url_update_ota_json , cJSON_GetObjectItem(root,"url_update_ota_json")->valuestring);
    strcpy(rec_config_file->url_update_config , cJSON_GetObjectItem(root,"url_update_config")->valuestring);
    rec_config_file->time_sleep = cJSON_GetObjectItem(root,"time_sleep")->valueint;
    rec_config_file->flash_power=cJSON_GetObjectItem(root,"flash_power")->valueint;
    rec_config_file->flash_time=cJSON_GetObjectItem(root,"flash_time")->valueint;
    rec_config_file->detect=cJSON_GetObjectItem(root,"detect")->valueint;
    rec_config_file->sendphotos=cJSON_GetObjectItem(root,"sendphotos")->valueint;
    sprintf(rec_config_file->wifi_ssid , cJSON_GetObjectItem(root,"wifi_ssid")->valuestring);
    sprintf(rec_config_file->wifi_password , cJSON_GetObjectItem(root,"wifi_password")->valuestring);
  //  ESP_LOGI(TAGCONFIG_FILE, "read_config_string output string %s",cJSON_Print(root));
    cJSON_Delete(root);
}

esp_err_t read_config_file_json(const char *filename, Rec_config_file_t *rec_config_file){
    
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
        char config_json_string[(int)st.st_size];
        fread(config_json_string, (int)st.st_size, 1, f);
        fclose(f); 
        //ESP_LOGI(CONFIG_FILE_NAME, "Read file Data %s size of file %d",config_json_string,(int)st.st_size);
        read_config_string(config_json_string,rec_config_file);
        ESP_LOGI(TAGCONFIG_FILE, "\n Read  Config file from SD size =%d",strlen(config_json_string));
        return ESP_OK;
    }    
};

void write_config_string(char *config_json_string,Rec_config_file_t *rec_config_file){
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "index", rec_config_file->index);
    cJSON_AddNumberToObject(root, "version_config", rec_config_file->version_config);
   // cJSON_AddStringToObject(root, "last_image_file_name", rec_config_file->last_image_file_name);
    cJSON_AddStringToObject(root, "DeviceID", rec_config_file->DeviceID);
    cJSON_AddStringToObject(root, "url_send_data", rec_config_file->url_send_data);
    cJSON_AddStringToObject(root, "url_update_ota_json", rec_config_file->url_update_ota_json);
    cJSON_AddStringToObject(root, "url_update_config", rec_config_file->url_update_config);
    cJSON_AddNumberToObject(root, "time_sleep", rec_config_file->time_sleep);
    
    cJSON_AddNumberToObject(root, "flash_power", rec_config_file->flash_power);
    cJSON_AddNumberToObject(root, "flash_time", rec_config_file->flash_time);
    cJSON_AddNumberToObject(root, "detect", rec_config_file->detect);

    cJSON_AddNumberToObject(root, "sendphotos", rec_config_file->sendphotos);
    cJSON_AddStringToObject(root, "wifi_ssid", rec_config_file->wifi_ssid);
    cJSON_AddStringToObject(root, "wifi_password", rec_config_file->wifi_password);
    strcpy(config_json_string, cJSON_Print(root));

    //ESP_LOGI(TAGCONFIG_FILE, "my_json_string\n%s",config_json_string);
    cJSON_Delete(root);

}

esp_err_t write_config_file_json(const char *filename,Rec_config_file_t *rec_config_file){
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        ESP_LOGE(TAGCONFIG_FILE, "Failed to open file %s for writing ",filename);
        return ESP_FAIL;
    }
    else{
	    char config_json_string[600];
        write_config_string(config_json_string,rec_config_file);
        
        if(strlen(config_json_string)==0){
            ESP_LOGE(TAGCONFIG_FILE, "Failed to convert rec_config_file to string ");
            return ESP_FAIL;
        }
        //ESP_LOGD(TAGCONFIG_FILE,"\n config_json_string  len=%d  \n%s",strlen(config_json_string),config_json_string);
        ESP_LOGI(TAGCONFIG_FILE, "open file %s for writing ",filename);
        size_t write_size=fwrite(config_json_string,strlen(config_json_string),1,f);
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


esp_err_t read_config_nvs(Rec_config_file_t *rec_config_file){
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
        read_config_string(config_json_string,rec_config_file);
    }
    nvs_close(my_handle);
    ESP_LOGI(TAGCONFIG_FILE,"\n Read  Config nvs size= %d \n ",sizeof(rec_config_file));
    return ESP_OK;
}

esp_err_t save_config_nvs( Rec_config_file_t *rec_config_file){
     nvs_handle_t my_handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE_CONFIG, NVS_READWRITE, &my_handle);
    if (err != ESP_OK){
        ESP_LOGE(TAGCONFIG_FILE,"Error Open  Config nvs");        
        return err;
    } 
    char config_json_string[800];
    write_config_string(config_json_string,rec_config_file);
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
    char *p=strchr(src,start);
    char *p2=strchr(p,end);
    size_t len   = strlen(p)- strlen(p2)+1;
    strncpy(dst,p,len);
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
                        cJSON *json = cJSON_Parse(json_buffer);
                        
                        if(json == NULL) {
                            ESP_LOGE(TAGCONFIG_FILE,"downloaded file config json is not a valid json, aborting...\n");
                        }else {	
                            cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version_config");
                            
                            double new_version = version->valuedouble;
                            if(new_version > rec_parameters_config_task->rec_config_file_t->version_config) {
                                read_config_string(json_buffer,rec_parameters_config_task->rec_config_file_t);
                                ESP_LOGI(TAGCONFIG_FILE," Update new Config Version %.4lf... \n",rec_parameters_config_task->rec_config_file_t->version_config);
                            }else{
                                ESP_LOGI(TAGCONFIG_FILE," Keep Config Version %.4lf... \n",rec_parameters_config_task->rec_config_file_t->version_config);
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
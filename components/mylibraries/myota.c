#include "myota.h"


void check_update_task(void *pvParameter) {
		ESP_LOGI(TAG_MYOTA,"Looking for a new firmware...\n");
		Rec_parameters_ota_task *rec_parameters_ota_task;
    	rec_parameters_ota_task=(Rec_parameters_ota_task*)pvParameter;
    
    	// configure the esp_http_client
		esp_http_client_config_t config = {
        .url = rec_parameters_ota_task->url_update_ota_json,
		.method=HTTP_METHOD_GET,
        .transport_type=HTTP_TRANSPORT_OVER_TCP,
		//.transport_type=HTTP_TRANSPORT_OVER_SSL,
		//.skip_cert_common_name_check=true,
		//.cert_pem = (char *)server_cert_pem_start
		};
	
		esp_http_client_handle_t client = esp_http_client_init(&config);
		// downloading the json file
        esp_err_t err = esp_http_client_open(client,0);
        int len_response = esp_http_client_fetch_headers(client);

//		esp_err_t err = esp_http_client_perform(client);
        //int len_response=esp_http_client_get_content_length(client);
        char response[500]; 
        if(len_response> 0){

            //char charbuffer[get_content];
            int data_len = esp_http_client_read_response(client,response, len_response);
			response[len_response]=0;
    		ESP_LOGI(TAG_MYOTA,"Looking for Version Json %s...\n Response Size %d \n Response %s",rec_parameters_ota_task->url_update_ota_json,len_response,response);
        }

		if(err == ESP_OK) {
			

			// parse the json file	
			cJSON *json = cJSON_Parse(response);

			if(json == NULL) ESP_LOGI(TAG_MYOTA,"downloaded file is not a valid json, aborting...\n");
			else {	
				cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version");
				cJSON *file = cJSON_GetObjectItemCaseSensitive(json, "file");
				
				// check the version
				if(!cJSON_IsNumber(version)) printf("unable to read new version, aborting...\n");
				else {
					
					double new_version = version->valuedouble;
					if(new_version > rec_parameters_ota_task->firmware_version) {
						
						ESP_LOGI(TAG_MYOTA,"current firmware version (%.1f) is lower than the available one (%.1f), upgrading...\n", rec_parameters_ota_task->firmware_version, new_version);
						if(cJSON_IsString(file) && (file->valuestring != NULL)) {
							ESP_LOGI(TAG_MYOTA,"downloading and installing new firmware (%s)...\n", file->valuestring);
							
							esp_http_client_config_t ota_client_config = {
								.url = file->valuestring,
						        //.transport_type=HTTP_TRANSPORT_OVER_SSL,
								//.skip_cert_common_name_check=true,
								//.cert_pem = (char *)server_cert_pem_start

							};
							esp_err_t ret = esp_https_ota(&ota_client_config);
							if (ret == ESP_OK) {
								ESP_LOGI(TAG_MYOTA,"OTA OK, restarting...\n");
								esp_restart();
							} else {
								ESP_LOGE(TAG_MYOTA,"OTA failed...\n");
							}
						}
						else ESP_LOGE(TAG_MYOTA,"unable to read the new file name, aborting...\n");
					}
					else ESP_LOGI(TAG_MYOTA,"current firmware version (%.1f) is greater or equal to the available one (%.1f), nothing to do...\n", rec_parameters_ota_task->firmware_version, new_version);
				}
			}
		}
		else ESP_LOGE(TAG_MYOTA,"unable to download the json file, aborting...\n");
		
		// cleanup
		esp_http_client_cleanup(client);
	    xEventGroupSetBits(http_ota_event_group, BIT0);
        vTaskDelay(30000 / portTICK_PERIOD_MS);
    
};

void wait_ota_request(void ){
    if(http_ota_event_group!=NULL){
        ESP_LOGI(TAG_MYOTA, "Wait  http_task ");
        xEventGroupWaitBits(http_ota_event_group, BIT0, false, true,portMAX_DELAY);
        ESP_LOGD(TAG_MYOTA,"*****************  MEM  after xEventGroupWaitBits %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
    }
};
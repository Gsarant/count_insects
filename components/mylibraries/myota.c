#include "myota.h"


void check_update_task(void *pvParameter) {
		ESP_LOGI(TAG_MYOTA,"Looking for a new firmware...\n");
		Rec_parameters_ota_task *rec_parameters_ota_task;
    	rec_parameters_ota_task=(Rec_parameters_ota_task*)pvParameter;
		char response[500]; 
        
		if(rec_parameters_ota_task->new_firmware_version > rec_parameters_ota_task->firmware_version) {
						
						ESP_LOGI(TAG_MYOTA,"current firmware version (%.4f) is lower than the available one (%.4f), upgrading...\n", rec_parameters_ota_task->firmware_version, rec_parameters_ota_task->new_firmware_version);
						if(rec_parameters_ota_task->url_update_ota_firmware != NULL) {
							ESP_LOGI(TAG_MYOTA,"downloading and installing new firmware (%s)...\n", rec_parameters_ota_task->url_update_ota_firmware);
							
							esp_http_client_config_t ota_client_config = {
								.url = rec_parameters_ota_task->url_update_ota_firmware,
								//.transport_type=HTTP_TRANSPORT_OVER_TCP,
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
						}else {
							ESP_LOGE(TAG_MYOTA,"unable to read the new file name, aborting...\n");
						}
							
					
		}
		// cleanup
		//esp_http_client_cleanup(client);
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
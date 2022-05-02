#ifndef HTTP_OTA_REQ_H_
#define HTTP_OTA_REQ_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>


#include <esp_log.h>
#include <cJSON.h>
#include <esp_https_ota.h>
#include <esp_http_client.h>


static const char *TAG_MYOTA = "MYOTA";
// server certificates
extern const char server_cert_pem_start[] asm("_binary_cert_pem_start");
extern const char server_cert_pem_end[] asm("_binary_cert_pem_end");


#ifdef __cplusplus
extern "C" {
#endif
typedef struct{
    char url_update_ota_json[150];
    float firmware_version;
    int response_len;
    char *response;
}Rec_parameters_ota_task;

EventGroupHandle_t http_ota_event_group;

void check_update_task(void *pvParameter) ;
void wait_ota_request(void);



#ifdef __cplusplus
}
#endif

#endif
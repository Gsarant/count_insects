#ifndef HTTP_REQ_H_
#define HTTP_REQ_H_

#include "esp_http_client.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <string.h>
#include "esp_system.h"
#include <esp_log.h>
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
//#define LOG_LOCAL_LEVEL ESP_LOG_INFO



static const int CONNECTED_BIT0 = BIT0;
static const int CONNECTED_BIT1 = BIT1;

#define FILENAMESIZE 90
#define MAX_HTTP_RECV_BUFFER 700
#define MAX_HTTP_OUTPUT_BUFFER 2048
static const char *TAG_HTTP_REQ = "HTTP_CLIENT";

typedef struct{
    uint8_t *data;
    size_t lendata;
    char filename[80];
    char jsondata[150];
    char url_send_data[250];
    int CONNECTED_BIT;
    esp_http_client_config_t esp_http_client_config;
    int response_len;
    char *response;
}Rec_parameters_http_task;

typedef struct{
    char jsondata[150];
    int CONNECTED_BIT;
    char url_send_data[250];
//    esp_http_client_config_t esp_http_client_config;
    int response_len;
    char *response;
}Rec_parameters_http_task_post_json;

xTaskHandle xHandle;

EventGroupHandle_t http_event_group;

#ifdef __cplusplus
extern "C" {
#endif


esp_err_t _http_event_handle(esp_http_client_event_t *evt);

void http_task_send_image(void *pvParameters);
//void http_task(Rec_parameters_http_task task_rec_parameters_http_task);
void http_task_post_json(void *pvParameters);
void wait_http_request(int connectedbit);
void stop_wait_request(int connectedbit);
#ifdef __cplusplus
}
#endif

#endif
#ifndef CONFIG_FILE_H_
#define CONFIG_FILE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"
#include <esp_log.h>
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

//#include "esp_vfs_fat.h"
//#include "driver/sdspi_host.h"
//#include "driver/spi_common.h"
//#include "sdmmc_cmd.h"
//#include "sdkconfig.h"
#include "sdcard.h"
#include "cJSON.h"
#include "esp_http_client.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/event_groups.h"




#define CONFIG_FILE_NAME "conf.json"
#define STORAGE_NAMESPACE_CONFIG "config"


#ifdef __cplusplus
extern "C" {
#endif
static  char *TAGCONFIG_FILE = "Config File ";

static const char *key_con = "nvs_config";

EventGroupHandle_t http_config_event_group;

typedef struct {
    int index;
    double version_config;
    char DeviceID[20];
    char last_image_file_name[80];
    char url_send_data[250];
    char url_update_ota_json[250];
    char url_update_config[250];
    int time_sleep;
    int flash_power;
    int flash_time;
    int detect;
    int sendphotos;
    char wifi_ssid[30];
    char wifi_password[60];

}Rec_config_file_t;

typedef struct{
    char url_update_config_json[150];
    int response_len;
    char *response;
    Rec_config_file_t *rec_config_file_t;
}Rec_parameters_config_task;

void write_default_values_to_rec_config_file(Rec_config_file_t *rec_config_file);
esp_err_t read_config_file_json(const char *filename, Rec_config_file_t *rec_config_file);
esp_err_t write_config_file_json(const char *filename, Rec_config_file_t *rec_config_file);
void read_config_string(const char *config_json_string,Rec_config_file_t *rec_config_file);
void write_config_string(char *config_json_string,Rec_config_file_t *rec_config_file);
esp_err_t save_config_nvs( Rec_config_file_t *rec_config_file);
esp_err_t read_config_nvs( Rec_config_file_t *rec_config_file);
void check_update_config_task(void *pvParameter);


#ifdef __cplusplus
}
#endif

#endif
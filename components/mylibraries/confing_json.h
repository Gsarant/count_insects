#ifndef CONFIG_FILE_H_
#define CONFIG_FILE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"
#include <esp_log.h>
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "sdcard.h"
#include "cJSON.h"
#include "esp_http_client.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/event_groups.h"

#define CONFIG_FILE_NAME "conf.json"
#define STORAGE_NAMESPACE_CONFIG "config_json"


#ifdef __cplusplus
extern "C" {
#endif
static  char *TAGCONFIG_FILE = "Config File ";

static const char *key_con = "nvs_config";

EventGroupHandle_t http_config_event_group;

static char config_json_string[1000];
static cJSON *root;

typedef struct{
    char url_update_config_json[150];
    int response_len;
    char *response;
    double version;
}Rec_parameters_config_task;

void write_default_values_to_rec_config_file();
esp_err_t read_config_file_json(const char *filename);
esp_err_t write_config_file_json(const char *filename);
esp_err_t save_config_nvs();
esp_err_t read_config_nvs();
void check_update_config_task(void *pvParameter);

int get_int_value(const char *name);
double get_double_value(const char *name);
char *get_string_value(const char *name);

esp_err_t set_int_value(const char *name,int newvalue);
esp_err_t set_double_value(const char *name,double newvalue);
esp_err_t set_string_value(const char *name,char *newvalue);

#ifdef __cplusplus
}
#endif

#endif
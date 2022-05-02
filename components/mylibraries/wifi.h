#ifndef WIFI_H_
#define WIFI_H_

#include <stdio.h>
#include <math.h>
#include <esp_log.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "sdcard.h"
//WIFI Defines

//#define MAX_CONN_FAIL 50
#define MAX_CONN_FAIL 50
#define MAX_LENGTH_WIFI_SSID 31
#define MAX_LENGTH_WIFI_PASS 63
#define WIFI_CONFIG_NAME "wifi_config.txt"

#ifdef __cplusplus
extern "C" {
#endif
//WIFI Variables

EventGroupHandle_t wifi_event_group;

static const int WIFI_CONNECTED_BIT = BIT0;
static const char *TAGWIFI = "Wifi Module";
volatile bool wifi_connected ;
volatile bool wifi_has_ip ;
esp_err_t network_event_handler(void *ctx, system_event_t *e);


void wait_wifi_connect(void);
esp_err_t wifi_init_stat(char *ssid,char *password);
int get_wifi_rssi();
int get_csq();
int get_csq_precent();


#ifdef __cplusplus
}
#endif

#endif
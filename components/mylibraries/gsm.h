#ifndef GSM_H_
#define GSM_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"

#include "esp_netif.h"
#include "esp_netif_ppp.h"
#include "esp_modem.h"
#include "esp_modem_netif.h"
#include "sim800.h"
#include "bg96.h"
#include "sim7600.h"
#include "driver/gpio.h"
#include <esp_log.h>

#define MODEM_POWER_ON_OFF       33

EventGroupHandle_t event__gsm_group;
static const int CONN_BIT = BIT0;
static const int STOP_BIT = BIT1;
static const char *TAG_GSM = "GSM";
int count;


#ifdef __cplusplus
extern "C" {
#endif

volatile bool gsm_connected ; 

void int_gsm_net();

void deint_gsm_net();
int get_GSM_Volt();
int get_GSM_rssi();
static modem_dce_t *dce = NULL;
static modem_dte_t *dte = NULL;
static esp_netif_t *esp_netif =NULL;
static gpio_config_t io_conf;
static uint32_t rssi , ber ;
static uint32_t volTAG_GSMe , bcs , bcl ;


static void *modem_netif_adapter = NULL;



#ifdef __cplusplus
}
#endif

#endif
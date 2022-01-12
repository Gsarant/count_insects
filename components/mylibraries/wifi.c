#include "wifi.h"

/* WiFi Change Event Handler */
esp_err_t network_event_handler(void *ctx, system_event_t *e){
    static uint8_t conn_fail_retries = 0;

    switch(e->event_id)
    {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAGWIFI,"WiFi Station interface Up.\n");
            ESP_LOGI(TAGWIFI,"Connecting...\n");
            esp_wifi_connect();
            break;

        case SYSTEM_EVENT_STA_CONNECTED:
            ESP_LOGI(TAGWIFI,"WiFi connected.\n");
            ESP_LOGI(TAGWIFI,"Waiting for IP...\n");
            wifi_connected = true;
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAGWIFI,"WiFi IPv4 received: %s\n", ip4addr_ntoa(&e->event_info.got_ip.ip_info.ip));
            wifi_has_ip = true;
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            break;

        case SYSTEM_EVENT_STA_LOST_IP:
            ESP_LOGI(TAGWIFI,"WiFi IP lost.\n");
            wifi_has_ip = false;
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            if(wifi_connected)
            {
                ESP_LOGI(TAGWIFI,"WiFi disconnected\n");
                conn_fail_retries = 0;
            }
            else
            {
                ESP_LOGI(TAGWIFI,"Can't connect to AP, trying again...\n");
                conn_fail_retries = conn_fail_retries + 1;
            }
            wifi_has_ip = false;
            wifi_connected = false;
            if(conn_fail_retries < MAX_CONN_FAIL)
                esp_wifi_connect();
            else
            {
                ESP_LOGI(TAGWIFI,"WiFi connection fail %d times.\n", MAX_CONN_FAIL);
                ESP_LOGI(TAGWIFI,"Rebooting the system...\n\n");
                esp_restart();
            }
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;

        case SYSTEM_EVENT_STA_STOP:
            ESP_LOGI(TAGWIFI,"WiFi interface stopped\n");
            conn_fail_retries = 0;
            wifi_has_ip = false;
            wifi_connected = false;
            break;

        default:
            break;
    }
    return ESP_OK;
};

// Init WiFi interface
esp_err_t wifi_init_stat(char *ssid,char *password){

    esp_err_t ret = ESP_OK;
    wifi_connected = false;
    wifi_has_ip = false;
    if(ssid!=NULL && password!=NULL){
        tcpip_adapter_init();
        wifi_event_group = xEventGroupCreate (); // Create an event group
        ESP_ERROR_CHECK(esp_event_loop_init(network_event_handler, NULL));
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_LOGI(TAGWIFI,"Conf...\n\n");
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

        wifi_config_t wifi_config={};
        strcpy((char *)wifi_config.sta.ssid , ssid);
        strcpy((char *)wifi_config.sta.password , password);

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        
        ESP_LOGI(TAGWIFI, "start the WIFI SSID:[%s] password:[%s]", ssid, password);
        ret=esp_wifi_start();
        ESP_ERROR_CHECK(ret);
        ESP_LOGI(TAGWIFI, "Waiting for wifi");
    }else{
        ESP_LOGE(TAGWIFI,"No SSID and  PASSWORD in Config file");
        ret=ESP_FAIL;
    }
    return ret;
};

void wait_wifi_connect(void){
    if(wifi_event_group!=NULL)
        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true,portMAX_DELAY);
};

int get_wifi_rssi(){
    wifi_ap_record_t ap;
    esp_wifi_sta_get_ap_info(&ap);
    return ap.rssi;
}

int get_csq(){
    int rssi=get_wifi_rssi();
    if (rssi<= -109)
        return 2;
    else if (rssi>= -53)
        return 30;
    else 
        return ceil(abs(rssi-(-113))*0.5);
}

int get_csq_precent(){
   int rssi=get_wifi_rssi();
   return abs(rssi);
}


/**************************************************************************************************/
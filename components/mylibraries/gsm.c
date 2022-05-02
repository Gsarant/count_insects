#include "gsm.h"

static void modem_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        switch (event_id) {
        case ESP_MODEM_EVENT_PPP_START:
            gsm_connected=true;
            ESP_LOGI(TAG_GSM, "Modem PPP Started");
            break;
        case ESP_MODEM_EVENT_PPP_STOP:
            gsm_connected=false;
            ESP_LOGI(TAG_GSM, "Modem PPP Stopped");
            xEventGroupSetBits(event__gsm_group, STOP_BIT);
            break;
        case ESP_MODEM_EVENT_UNKNOWN:
            ESP_LOGW(TAG_GSM, "Unknow line received: %s", (char *)event_data);
            break;
        default:
            break;
        }
    }


static void on_ppp_changed(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG_GSM, "PPP state changed event %d", event_id);
    if (event_id == NETIF_PPP_ERRORUSER) {
        /* User interrupted event from esp-netif */
        esp_netif_t *netif = *(esp_netif_t**)event_data;
        ESP_LOGI(TAG_GSM, "User interrupted event from netif:%p", netif);
    }
}


static void on_ip_event(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG_GSM, "IP event! %d", event_id);
    if (event_id == IP_EVENT_PPP_GOT_IP) {
        esp_netif_dns_info_t dns_info;

        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        esp_netif_t *netif = event->esp_netif;

        ESP_LOGI(TAG_GSM, "Modem Connect to PPP Server");
        ESP_LOGI(TAG_GSM, "~~~~~~~~~~~~~~");
        ESP_LOGI(TAG_GSM, "IP          : " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG_GSM, "Netmask     : " IPSTR, IP2STR(&event->ip_info.netmask));
        ESP_LOGI(TAG_GSM, "Gateway     : " IPSTR, IP2STR(&event->ip_info.gw));
        esp_netif_get_dns_info(netif, 0, &dns_info);
        ESP_LOGI(TAG_GSM, "Name Server1: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
        esp_netif_get_dns_info(netif, 1, &dns_info);
        ESP_LOGI(TAG_GSM, "Name Server2: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
        ESP_LOGI(TAG_GSM, "~~~~~~~~~~~~~~");
        gsm_connected=true;
        xEventGroupSetBits(event__gsm_group, CONN_BIT);
        ESP_LOGI(TAG_GSM, "GOT ip event!!!");
    } else if (event_id == IP_EVENT_PPP_LOST_IP) {
        ESP_LOGI(TAG_GSM, "Modem Disconnect from PPP Server");
        gsm_connected=false;
        xEventGroupSetBits(event__gsm_group, CONN_BIT);
       
    } else if (event_id == IP_EVENT_GOT_IP6) {
        ESP_LOGI(TAG_GSM, "GOT IPv6 event!");
        ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
        ESP_LOGI(TAG_GSM, "Got IPv6 address " IPV6STR, IPV62STR(event->ip6_info.ip));

    }
}
void power_modem_init(){
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        //io_conf.pin_bit_mask = (1<<MODEM_RST) | (1<<MODEM_PWKEY) | (1<<MODEM_POWER_ON);
       // io_conf.pin_bit_mask =  (1<<MODEM_PWKEY) | (1<<MODEM_POWER_ON);
        io_conf.pin_bit_mask =  (1ULL<<MODEM_POWER_ON_OFF) ;
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 0;
        gpio_config(&io_conf);
        gpio_set_level(MODEM_POWER_ON_OFF, 0);
        ESP_LOGI(TAG_GSM, "MODEM_PWKEY OFF ");
        vTaskDelay(2000 / portTICK_RATE_MS);
}

void power_on_modem(){
        gpio_set_level(MODEM_POWER_ON_OFF, 1);
        ESP_LOGI(TAG_GSM, "MODEM_PWKEY ON ");
        vTaskDelay(20000 / portTICK_RATE_MS);
    }

void power_off_modem(){
        gpio_set_level(MODEM_POWER_ON_OFF, 0);
        ESP_LOGI(TAG_GSM, "MODEM_PWKEY OFF ");
    }

void int_gsm_net(char *apn){
power_modem_init();
#if CONFIG_LWIP_PPP_PAP_SUPPORT
    esp_netif_auth_type_t auth_type = NETIF_PPP_AUTHTYPE_PAP;
#elif CONFIG_LWIP_PPP_CHAP_SUPPORT
    esp_netif_auth_type_t auth_type = NETIF_PPP_AUTHTYPE_CHAP;
#elif !defined(CONFIG_EXAMPLE_MODEM_PPP_AUTH_NONE)
///#error "Unsupported AUTH Negotiation"
#endif
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &on_ip_event, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(NETIF_PPP_STATUS, ESP_EVENT_ANY_ID, &on_ppp_changed, NULL));

        event__gsm_group = xEventGroupCreate();

        power_on_modem();

        /* create dte object */
        esp_modem_dte_config_t config = ESP_MODEM_DTE_DEFAULT_CONFIG();
        /* setup UART specific configuration based on kconfig options */
        config.tx_io_num = 12;
        config.rx_io_num = 13;
        config.rts_io_num = 0;
        config.cts_io_num = 0;
        
        //config.rx_buffer_size = CONFIG_EXAMPLE_MODEM_UART_RX_BUFFER_SIZE;
        //config.tx_buffer_size = CONFIG_EXAMPLE_MODEM_UART_TX_BUFFER_SIZE;
        //config.pattern_queue_size = CONFIG_EXAMPLE_MODEM_UART_PATTERN_QUEUE_SIZE;
        //config.event_queue_size = CONFIG_EXAMPLE_MODEM_UART_EVENT_QUEUE_SIZE;
        //config.event_task_stack_size = CONFIG_EXAMPLE_MODEM_UART_EVENT_TASK_STACK_SIZE;
        //config.event_task_priority = CONFIG_EXAMPLE_MODEM_UART_EVENT_TASK_PRIORITY;
        //config.line_buffer_size = CONFIG_EXAMPLE_MODEM_UART_RX_BUFFER_SIZE / 2;

        dte = esp_modem_dte_init(&config);
         
        assert(dte != NULL);
        
        if(dte != NULL){
            
            /* Register event handler */
            ESP_ERROR_CHECK(esp_modem_set_event_handler(dte, modem_event_handler, ESP_EVENT_ANY_ID, NULL));

            // Init netif object
            esp_netif_config_t cfg = ESP_NETIF_DEFAULT_PPP();
        
            //esp_netif_t *esp_netif = esp_netif_new(&cfg);
            esp_netif = esp_netif_new(&cfg);

            assert(esp_netif);
            //void *modem_netif_adapter = esp_modem_netif_setup(dte);
            modem_netif_adapter = esp_modem_netif_setup(dte);
            esp_modem_netif_set_default_handlers(modem_netif_adapter, esp_netif);

            
                /* create dce object */
                //#if CONFIG_EXAMPLE_MODEM_DEVICE_SIM800
                    ESP_LOGI(TAG_GSM, "try init sim800l");
                    dce = sim800_init(dte);
               // #elif CONFIG_EXAMPLE_MODEM_DEVICE_BG96
               //         dce = bg96_init(dte);
               // #elif CONFIG_EXAMPLE_MODEM_DEVICE_SIM7600
               //         dce = sim7600_init(dte);
                //#else
                //#error "Unsupported DCE"
                //#endif

                assert(dce != NULL);

                ESP_LOGI(TAG_GSM, "dce intited");

                if (dce != NULL){
                    ESP_LOGI(TAG_GSM, " inited sim800l");
                    ESP_ERROR_CHECK(dce->set_flow_ctrl(dce, MODEM_FLOW_CONTROL_NONE));
                    ESP_ERROR_CHECK(dce->store_profile(dce));
                    /* Print Module ID, Operator, IMEI, IMSI */
                    ESP_LOGI(TAG_GSM, "Module: %s", dce->name);
                    ESP_LOGI(TAG_GSM, "Operator: %s", dce->oper);
                    ESP_LOGI(TAG_GSM, "IMEI: %s", dce->imei);
                    ESP_LOGI(TAG_GSM, "IMSI: %s", dce->imsi);
                    /* Get signal quality */
                   // uint32_t rssi = 0, ber = 0;
                   rssi = 0; ber = 0;

                    ESP_ERROR_CHECK(dce->get_signal_quality(dce, &rssi, &ber));
                    ESP_LOGI(TAG_GSM, "rssi: %d, ber: %d", rssi, ber);
                    /* Get battery volTAG_GSMe */
                    //uint32_t volTAG_GSMe = 0, bcs = 0, bcl = 0;
                    volTAG_GSMe = 0; bcs = 0; bcl = 0;
                    ESP_ERROR_CHECK(dce->get_battery_status(dce, &bcs, &bcl, &volTAG_GSMe));
                    ESP_LOGI(TAG_GSM, "Battery volTAG_GSMe: %d mV", volTAG_GSMe);
                
                    /* setup PPPoS network parameters */
                    #if !defined(CONFIG_EXAMPLE_MODEM_PPP_AUTH_NONE) && (defined(CONFIG_LWIP_PPP_PAP_SUPPORT) || defined(CONFIG_LWIP_PPP_CHAP_SUPPORT))
                        esp_netif_ppp_set_auth(esp_netif, auth_type, CONFIG_EXAMPLE_MODEM_PPP_AUTH_USERNAME, CONFIG_EXAMPLE_MODEM_PPP_AUTH_PASSWORD);
                    #endif
                    /* attach the modem to the network interface */
                    esp_netif_attach(esp_netif, modem_netif_adapter);
                    /* Wait for IP address */
                    xEventGroupWaitBits(event__gsm_group, CONN_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
                    ESP_LOGI(TAG_GSM, "gsm_modem connected");
                    //gsm_connected=true;
                }
            
            }else{
                ESP_LOGI(TAG_GSM, "dte Not init");
            }   
        
            
    }

    void deint_gsm_net(){

            /* Exit PPP mode */
            ESP_ERROR_CHECK(esp_modem_stop_ppp(dte));

            //xEventGroupWaitBits(event__gsm_group, STOP_BIT, pdTRUE, pdTRUE, portMAX_DELAY);

            /* Power down module */
            ESP_ERROR_CHECK(dce->power_down(dce));
            ESP_LOGI(TAG_GSM, "Power down");
            ESP_ERROR_CHECK(dce->deinit(dce));

            

        /* Unregister events, destroy the netif adapter and destroy its esp-netif instance */
        esp_modem_netif_clear_default_handlers(modem_netif_adapter);
        esp_modem_netif_teardown(modem_netif_adapter);
        esp_netif_destroy(esp_netif);

        ESP_ERROR_CHECK(dte->deinit(dte));
        power_off_modem();
        gsm_connected=false;
    }
    



#include <stdio.h>
#include <esp_event_loop.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_task_wdt.h"

#include "esp_vfs_fat.h"



#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>

#include "confing_file.h"

#define Version 1.00
#define SDCARD
//#define NEWCONFIG
#define NETWORK
#define REALTIMECLOCK
#define CAMERA
#define HTTP
//#define OTA
#define SLEEP
#define TFMICRO
#define SAVE_SEND_ORIGINAL_IMAGE


#ifdef NETWORK
  
  #include "esp_wifi.h"
  #include "wifi.h"
  #ifdef HTTP
    #include "http_req.h"
  #endif  
  #ifdef OTA
      #include "myota.h"
  #endif    
#endif

#ifdef REALTIMECLOCK
    #include "esp_sntp.h"
    #include "soc/soc.h"
    #include "soc/rtc_cntl_reg.h"
#endif

#ifdef SLEEP
    #include "esp_sleep.h"
    #include "esp_task_wdt.h"
#endif

#ifdef SDCARD
    #include "sdcard.h"
#endif

#ifdef CAMERA
    #include "mygpio.h"
    #include "mycamera.h"
    #include "fotolib.h"
#endif

#ifdef TFMICRO
    #include "tfmicro.h"
#endif    

//MAIN Defines
#define LOGNAME_FORMAT "f_%Y%m%d%H%M%S"
#define LOGNAME_SIZE 20
#define STORAGE_NAMESPACE "last_image"
#define FILENAMESIZE 90
#define QUALITY_OF_JPG_IMAGE 70

//General Variables
static const char *TAG = "main";
Rec_config_file_t rec_config_file;
bool is_init_camera=false;
const char *key = "nvs_image";

//functions declare 
esp_err_t nvs_init(void);

#ifdef SLEEP
    #define uS_TO_S_FACTOR 1000000  
    RTC_DATA_ATTR int bootCount = 0;
#endif//SLEEP

#ifdef NETWORK
    void int_net(void);
#endif//NETWORK

#ifdef HTTP
    void send_image(Rec_config_file_t *rec_config_file ,char *jpg_file ,uint8_t *_jpg_buf,size_t _jpg_buf_len);
    void send_json_data(Rec_config_file_t *rec_config_file);
#endif//HTTP

#ifdef REALTIMECLOCK
    EventGroupHandle_t sntp_event_group;
    //functions declare 
    void  init_sntp(void *pvParameters);
#endif//REALTIMECLOCK


void app_main(void)
{
    ESP_LOGD(TAG,"*****************  MEM Start %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
    nvs_init();
    #ifdef CAMERA
        //*************************************************
        //Camera init must be before other initialization
        //*************************************************
        esp_err_t ret_wifi =init_camera();
        //ESP_ERROR_CHECK_WITHOUT_ABORT(ret_wifi);
        //ESP_ERROR_CHECK(ret_wifi);
        if(ret_wifi==ESP_OK){
            is_init_camera=true;
            ESP_LOGI(TAG,"Camera Init");
            ESP_LOGD(TAG,"*****************  MEM free camera init %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
        }
        else{
            ESP_LOGE(TAG,"Camera Fail Init");
        }
    #endif//CAMERA

    #ifdef NEWCONFIG
            write_default_values_to_rec_config_file(&rec_config_file);
            ESP_LOGI(TAG, "Init Config From code");
    #else
            #ifdef SDCARD
                char config_filename[60];
                if(sd_enable==false){
                    ESP_ERROR_CHECK(init_sd());
                    //Initialization Config File
                    ESP_LOGI(TAG, "Initialization Config File ");
                    sprintf(config_filename,"%s/%s",MOUNT_POINT,CONFIG_FILE_NAME);
                    esp_err_t err=read_config_file_json(config_filename, &rec_config_file);
                // ESP_ERROR_CHECK(err);
                    if(err==ESP_FAIL){
                        ESP_LOGE(TAG, "\n There is not Config file to SD card \n");
                        ESP_LOGI(TAG, "Try to Load Config from nvs" );
                        if(read_config_nvs(&rec_config_file)!=ESP_OK){
                            ESP_LOGE(TAG, "\n There is not Config file to nvs \n");
                            ESP_LOGI(TAG, "Try to Load embeded Config" );
                            write_default_values_to_rec_config_file(&rec_config_file);
                            ESP_LOGI(TAG, "Init Config From code");
                        }
                
                    }else {
                        ESP_LOGI(TAG, "index:%d  filename:%s \n",rec_config_file.index, rec_config_file.last_image_file_name);
                    }
                    ESP_LOGD(TAG,"*****************  MEM after sd init  %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
                }
            #else
                ESP_LOGI(TAG, "Try to Load Config from nvs");
                if(read_config_nvs(&rec_config_file)!=ESP_OK){
                    ESP_LOGE(TAG, "\n There is not Config file to nvs \n");
                    ESP_LOGI(TAG, "Try to Load embeded Config" );

                    write_default_values_to_rec_config_file(&rec_config_file);
                    ESP_LOGI(TAG, "Init Config From code");
                }
            #endif //SDCARD   
    #endif //NEWCONFIG   
    ESP_LOGI(TAG, "Current  Config Version %lf",rec_config_file.version_config);

    
    #ifdef REALTIMECLOCK
        char tmp_timename1[18];
        time_t now1 = time(0);

        strftime(tmp_timename1, sizeof(tmp_timename1), "%d-%m-%Y  %H:%M:%S", localtime(&now1));
        ESP_LOGI(TAG,"%ld  ---   %s",now1,tmp_timename1);
        if(now1<100){
            #ifdef NETWORK
                int_net();
            #endif//NETWORK
            sntp_event_group = xEventGroupCreate (); // Create an event group
            xTaskCreate(&init_sntp, "init_sntp", 2000, NULL, tskIDLE_PRIORITY, NULL);
            //Wait untill finished sntp
            xEventGroupWaitBits(sntp_event_group, CONNECTED_BIT, false, true,portMAX_DELAY);
            ESP_LOGI(TAG,"*****************  MEM  after sntp init %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
        }
    #endif//REALTIMECLOCK

    #ifdef NETWORK
    //Update Config File
        ESP_LOGI(TAG, "UpDate Config");
        ESP_LOGI(TAG, "Current  Config Version %lf",rec_config_file.version_config);
        if(!wifi_connected)
            int_net();
        
        Rec_parameters_config_task rec_parameters_config_task;
        strcpy(rec_parameters_config_task.url_update_config_json,rec_config_file.url_update_config);
        rec_parameters_config_task.rec_config_file_t=&rec_config_file;
        // memcpy(rec_parameters_config_task.rec_config_file_t, &rec_config_file, sizeof(rec_config_file));
        http_config_event_group = xEventGroupCreate (); // Create an event group
	    xTaskCreate(&check_update_config_task, "check_update_config_task", 10000, (void*)&rec_parameters_config_task, tskIDLE_PRIORITY, NULL);  
       
        if(http_config_event_group!=NULL){
            ESP_LOGI(TAG, "Wait  check_update_config_task ");
            xEventGroupWaitBits(http_config_event_group, BIT0, false, true,portMAX_DELAY);
            ESP_LOGD(TAG,"*****************  MEM  after xEventGroupWaitBits %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
        }
        
        if(rec_config_file.version_config < rec_parameters_config_task.rec_config_file_t->version_config){
            #ifdef SDCARD
                write_config_file_json(config_filename,rec_parameters_config_task.rec_config_file_t);
                save_config_nvs(rec_parameters_config_task.rec_config_file_t);
            #else
                save_config_nvs(rec_parameters_config_task.rec_config_file_t);
            #endif//SDCARD
        }

    #endif//NETWORK
    
    #ifdef TFMICRO
       // is_init_tf=init_tf();
    #endif //TFMICRO
    //prepeare string with time for filename
    char timename[LOGNAME_SIZE];
    time_t now = time(0);
    strftime(timename, sizeof(timename), LOGNAME_FORMAT, localtime(&now));

    #ifdef CAMERA
        if(is_init_camera==true){
            //Capture Foto  
            //ESP_LOGI(TAG,"Flash Enable");
            flash_on(rec_config_file.flash_power);
            vTaskDelay(rec_config_file.flash_time / portTICK_PERIOD_MS);
            camera_fb_t * fb = NULL;    
            fb = esp_camera_fb_get();
            if (!fb) {
                ESP_LOGE(TAGCAMERA,"Camera capture failed");
            } else {
               // ESP_LOGI(TAG, "Camera capture");
            }   
            vTaskDelay(rec_config_file.flash_time / portTICK_PERIOD_MS);
            flash_off();     
            ESP_LOGI(TAG,"Flash Disable");
            ESP_LOGD(TAG,"*****************  MEM Camera Capture %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));

            if(!fb){
                ESP_LOGE(TAG,"*****************  MEM fb NULL %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
            }else{
                int pred=-1;
                #ifdef TFMICRO
                    if(rec_config_file.detect==1){
                        time_t now_1 = time(0);
                        pred=getPredict(fb->buf,IMAGE_SIZE_ROW,IMAGE_SIZE_COL,1);
                        ESP_LOGI(TAG,"Predict %d",pred);
                        time_t now_2 =time(0) ;
                        time_t differ =now_2-now_1;
                        ESP_LOGI(TAG,"** total predict time  %ld **",time(0)-now_1);
                    }
                    else{
                        ESP_LOGI(TAG,"No Predict from config");
                    }
                    
                #endif 
                #ifdef SAVE_SEND_ORIGINAL_IMAGE
                    //Save last changed jpg file
                    uint8_t * _jpg_buf=NULL;
                    size_t _jpg_buf_len=0;
                    char jpg_file[FILENAMESIZE];
                    rec_config_file.index=rec_config_file.index+1;
                    sprintf(jpg_file,"%s/%s_%d_%d.jpg",MOUNT_POINT,timename, rec_config_file.index,pred);
                    fmt2jpg(fb->buf,IMAGE_SIZE_ROW*IMAGE_SIZE_COL, IMAGE_SIZE_ROW, IMAGE_SIZE_COL, PIXFORMAT_GRAYSCALE, QUALITY_OF_JPG_IMAGE, &_jpg_buf, &_jpg_buf_len);
                    ESP_LOGD(TAG," Image  %s convert to jpg old Size %d New Size %d",jpg_file ,IMAGE_SIZE_ROW*IMAGE_SIZE_COL,_jpg_buf_len );

                    #ifdef SDCARD
                        if(_jpg_buf_len>0){
                            save_file(jpg_file,_jpg_buf,_jpg_buf_len);
                            ESP_LOGD(TAG,"*****************  MEM after save %s   %d   *******************",jpg_file,heap_caps_get_free_size(MALLOC_CAP_8BIT));
                        }
                    
                    #endif//SDCARD

                    sprintf(rec_config_file.last_image_file_name,"%s/%s_%d",MOUNT_POINT,timename,rec_config_file.index);
                
                    #ifdef HTTP
                        if(rec_config_file.sendphotos==1){
                            if(_jpg_buf_len>0)
                                send_image(&rec_config_file ,jpg_file,_jpg_buf,_jpg_buf_len);
                            }
                            else{
                                ESP_LOGI(TAG,"No Send Photo from config");
                            }    

                    #endif//HTTP    
                #endif //SAVE_SEND_ORIGINAL_IMAGE  
                 
            }
        }

    #endif CAMERA
    
    #ifdef SDCARD
        //write_config_file_json(config_filename,rec_parameters_config_task.rec_config_file_t);
        save_config_nvs(rec_parameters_config_task.rec_config_file_t);
    #else
        save_config_nvs(rec_parameters_config_task.rec_config_file_t);
    #endif//SDCARD
    
    #ifdef OTA
        
        Rec_parameters_ota_task rec_parameters_ota_task;
        strcpy(rec_parameters_ota_task.url_update_ota_json,rec_config_file.url_update_ota_json);
        rec_parameters_ota_task.firmware_version=Version;
        
        http_ota_event_group = xEventGroupCreate (); // Create an event group

	    xTaskCreate(&check_update_task, "check_update_task", 8192, (void*)&rec_parameters_ota_task, 5, NULL);  
        wait_ota_request();
        

    #endif//OTA
    
    #ifdef SLEEP
        ESP_LOGD(TAG,"Sleep for %d Seconds ",rec_config_file.time_sleep);
        uint64_t sleeptime = UINT64_C(rec_config_file.time_sleep * uS_TO_S_FACTOR);
        esp_sleep_enable_timer_wakeup(sleeptime);
        esp_deep_sleep_start();
    #endif//SLEEP
}


#ifdef REALTIMECLOCK

    void init_sntp(void *pvParameters){
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        ESP_LOGI(TAG, "Initializing SNTP");
        sntp_init(); 
        int retry = 0;
        const int retry_count = 10;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
            ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }  
        char tmp_timename[18];
        time_t now = time(0);
        strftime(tmp_timename, sizeof(tmp_timename), "%d-%m-%Y  %H:%M:%S", localtime(&now));
        ESP_LOGD(TAG,"%s",tmp_timename);
        sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
        xEventGroupSetBits(sntp_event_group, CONNECTED_BIT);
        vTaskDelete(NULL);
    }
#endif//REALTIMECLOCK

#ifdef NETWORK
    void int_net(void){
        ESP_LOGD(TAG,"*****************  MEM after nvs_init %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
        wifi_init_stat(rec_config_file.wifi_ssid,rec_config_file.wifi_password);
        ESP_LOGD(TAG,"*****************  MEM  after wifi init %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
        wait_wifi_connect();
    }
#endif//NETWORK    

esp_err_t nvs_init(void){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
        ESP_LOGI(TAG, "************************Not Init************************");
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "************************Init App************************");
    return ret;
};

#ifdef HTTP
void send_json_data(Rec_config_file_t *rec_config_file){
    #ifdef NETWORK
        if(!wifi_connected)
            int_net();
    #endif//NETWORK
    Rec_parameters_http_task_post_json *rec_parameters_http_task_post_json;
    rec_parameters_http_task_post_json=(Rec_parameters_http_task_post_json *)malloc(sizeof(Rec_parameters_http_task_post_json));
    strcpy(rec_parameters_http_task_post_json->url_send_data,rec_config_file->url_send_data);
    srand(time(NULL));
    int r = (rand() % (99 - 80)) + 80;
    ESP_LOGV(TAG,"\n************ send_json_data WIFI RSSI %d **************\n",get_wifi_rssi());
    time_t temp_now = time(0)+rec_config_file->time_sleep;
    sprintf(rec_parameters_http_task_post_json->jsondata,"{\"System\":{\"DevId\":\"%s\",\"DevType\":8888,\"Batt\":%d,\"NxtUpdTm\":%ld,\"TotCount\":%d,\"CSQ\":%d}}\r\n",rec_config_file->DeviceID,r,temp_now,1,get_csq_precent());
    
    rec_parameters_http_task_post_json->CONNECTED_BIT=CONNECTED_BIT0;
    
         if(wifi_connected==false){
            wifi_init_stat(rec_config_file->wifi_ssid,rec_config_file->wifi_password);
            ESP_LOGI(TAG,"*****************  MEM  after wifi init %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
            wait_wifi_connect();
        }   
        http_event_group = xEventGroupCreate ();
        xHandle=NULL;
        BaseType_t mytask=xTaskCreate(&http_task_post_json, "http_task_post_json", 3000,rec_parameters_http_task_post_json, tskIDLE_PRIORITY, &xHandle);
        ESP_LOGD(TAG,"*****************  MEM  after xTaskCreate http_task_post_json %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
        switch(mytask){
            case pdPASS:
                ESP_LOGI(TAG,"xTaskCreate  ok http_task_post_json");
                break;
            case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
                ESP_LOGE(TAG,"xTaskCreate  fail http_task_post_json");
                stop_wait_request(rec_parameters_http_task_post_json->CONNECTED_BIT);
                break;    
        } 
        wait_http_request(rec_parameters_http_task_post_json->CONNECTED_BIT);
        
        if(rec_parameters_http_task_post_json!=NULL){
            //free(rec_parameters_http_task_post_json);
        }
}

void send_image(Rec_config_file_t *rec_config_file ,char *jpg_file ,uint8_t* _jpg_buf,size_t _jpg_buf_len){
    #ifdef NETWORK
        if(!wifi_connected)
            int_net();
    #endif//NETWORK
    if (_jpg_buf_len>0) {
        Rec_parameters_http_task rec_parameters_http_task;
        //rec_parameters_http_task=(Rec_parameters_http_task *)malloc(sizeof(Rec_parameters_http_task));
        memcpy(rec_parameters_http_task.filename ,jpg_file,strlen(jpg_file)+1);
        strcpy(rec_parameters_http_task.url_send_data,rec_config_file->url_send_data);
        rec_parameters_http_task.lendata=_jpg_buf_len;
        ESP_LOGV(TAG,"\n************ WIFI RSSI %d **************\n",get_wifi_rssi());

        time_t temp_now = time(0)+rec_config_file->time_sleep;
        sprintf(rec_parameters_http_task.jsondata,"{\"System\":{\"DevId\":\"%s\",\"DevType\":8888,\"Batt\":94,\"NxtUpdTm\":%ld,\"TotCount\":%d,\"CSQ\":%d}}\r\n",rec_config_file->DeviceID,temp_now,1,get_csq_precent());
        rec_parameters_http_task.data=(uint8_t *)malloc(sizeof(uint8_t)*(rec_parameters_http_task.lendata));
        rec_parameters_http_task.response=(Rec_parameters_http_task *)malloc(2000);
        memcpy(rec_parameters_http_task.data,_jpg_buf,_jpg_buf_len);
        rec_parameters_http_task.CONNECTED_BIT=CONNECTED_BIT0;
        
        esp_http_client_config_t tmp_client_config = {
            .url = rec_config_file->url_send_data,
            .timeout_ms = 5000						                    
        };
        rec_parameters_http_task.esp_http_client_config= tmp_client_config;

        //strcpy(rec_parameters_http_task.esp_http_client_config.url ,rec_config_file.url_send_data);
        rec_parameters_http_task.esp_http_client_config.timeout_ms=5000;
    
        if(wifi_connected==false){
            wifi_init_stat(rec_config_file->wifi_ssid,rec_config_file->wifi_password);
            ESP_LOGI(TAG,"*****************  MEM  after wifi init %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
            wait_wifi_connect();
        }   

        http_event_group = xEventGroupCreate ();
        xHandle=NULL;
        ESP_LOGI(TAG,"Memory size %d",sizeof(rec_parameters_http_task));
        BaseType_t mytask=xTaskCreate(&http_task_send_image, "http_task", 18000,  & rec_parameters_http_task, tskIDLE_PRIORITY, &xHandle);
        ESP_LOGD(TAG,"*****************  MEM  after xTaskCreate %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
        switch(mytask){
            case pdPASS:
                ESP_LOGI(TAG,"xTaskCreate  ok http_task");
                break;
            case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
                ESP_LOGE(TAG,"xTaskCreate  Fail http_task NOT_ALLOCATE_REQUIRED_MEMORY");
                stop_wait_request(rec_parameters_http_task.CONNECTED_BIT);
                break;    
        } 
        wait_http_request(rec_parameters_http_task.CONNECTED_BIT);
       // vTaskDelay(5000 / portTICK_PERIOD_MS);
        if(rec_parameters_http_task.data!=NULL)
            free(rec_parameters_http_task.data);
        if(rec_parameters_http_task.response!=NULL)
            free(rec_parameters_http_task.response);    
    }
}
#endif//HTTP  
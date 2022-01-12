#include "http_req.h"


#define MAX_HTTP_RECV_BUFFER 512

esp_err_t _http_event_handle(esp_http_client_event_t *evt){
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG_HTTP_REQ, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG_HTTP_REQ, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG_HTTP_REQ, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG_HTTP_REQ, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG_HTTP_REQ, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                printf("%.*s", evt->data_len, (char*)evt->data);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG_HTTP_REQ, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG_HTTP_REQ, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}


void http_task_send_image(void *pvParameters){
    
    Rec_parameters_http_task *task_rec_parameters_http_task;
    task_rec_parameters_http_task=(Rec_parameters_http_task*)pvParameters;
    ESP_LOGV(TAG_HTTP_REQ, "task_rec_parameters_http_task->lendata %d   %s",task_rec_parameters_http_task->lendata,task_rec_parameters_http_task->filename);

    if(task_rec_parameters_http_task->lendata>0){
        esp_http_client_handle_t client = esp_http_client_init(&task_rec_parameters_http_task->esp_http_client_config);
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_header(client, "Content-Type", "multipart/form-data; boundary=WebKitFormBoundary7MA4YWxkTrZu0gW");
        esp_http_client_set_header(client, "Connection", "keep-alive");
        esp_http_client_set_header(client, "Cache-Control","no-cache");
        char dest[7000];
        const char *boundary1="--WebKitFormBoundary7MA4YWxkTrZu0gW\r\n";
        const char *boundary2="\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

        const char *post_parameters[200];
        sprintf(post_parameters,"Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\nContent-Type: image/jpeg\r\n\r\n",task_rec_parameters_http_task->filename); 
        
            
        const char *post_parameters2[200];
        sprintf(post_parameters2,"Content-Disposition: form-data; name=\"message\"\r\n\r\n"); 

        
        
        size_t offset=0; 
        memcpy(dest,boundary1,strlen(boundary1));
        offset=offset+strlen(boundary1);

        memcpy(dest+offset,post_parameters2,strlen(post_parameters2));
        offset=offset+strlen(post_parameters2);

        memcpy(dest+offset,task_rec_parameters_http_task->jsondata,strlen(task_rec_parameters_http_task->jsondata));
        offset=offset+strlen(task_rec_parameters_http_task->jsondata);
        
        

        memcpy(dest+offset,boundary1,strlen(boundary1));
        offset=offset+strlen(boundary1);

        memcpy(dest+offset,post_parameters,strlen(post_parameters));
        offset=offset+strlen(post_parameters);
        
        memcpy(dest+offset,task_rec_parameters_http_task->data,task_rec_parameters_http_task->lendata);
        offset=offset+task_rec_parameters_http_task->lendata;
        

        memcpy(dest+offset,boundary2,strlen(boundary2));
        offset=offset+strlen(boundary2);

        esp_http_client_set_post_field(client, (const char*)dest, offset);
        
        
        esp_err_t err = esp_http_client_perform(client);
             
        if (err == ESP_OK) {
            ESP_LOGI(TAG_HTTP_REQ, "HTTPS Response Status = %d, content_length = %d",
                    esp_http_client_get_status_code(client),
                    esp_http_client_get_content_length(client));
        } else {
            ESP_LOGE(TAG_HTTP_REQ, "Error HTTPS Response Status %s", esp_err_to_name(err));
        }
        task_rec_parameters_http_task->response_len=esp_http_client_get_content_length(client);
        ESP_LOGD(TAG_HTTP_REQ,"http len %d", task_rec_parameters_http_task->response_len);
        if(task_rec_parameters_http_task->response_len> 0){
            //int data_len = esp_http_client_read(client, task_rec_parameters_http_task->response, task_rec_parameters_http_task->response_len);
            task_rec_parameters_http_task->response_len = esp_http_client_read_response(client, task_rec_parameters_http_task->response, task_rec_parameters_http_task->response_len);

            ESP_LOGV(TAG_HTTP_REQ,"\n\n Response Image Data length %i \n Response Data %s \n\n", task_rec_parameters_http_task->response_len,task_rec_parameters_http_task->response);
        }
        // esp_http_client_close(client);
        esp_http_client_cleanup(client);
    }   
    vTaskDelay(1000 / portTICK_PERIOD_MS);
   // ESP_LOGD(TAG_HTTP_REQ, "http_event_group  %s  set bit %d",task_rec_parameters_http_task->filename,task_rec_parameters_http_task->CONNECTED_BIT);
    stop_wait_request(task_rec_parameters_http_task->CONNECTED_BIT);
    vTaskDelete(xHandle);
}

void http_task_post_json2(void *pvParameters){
    Rec_parameters_http_task_post_json *rec_parameters_http_task_post_json;
    rec_parameters_http_task_post_json=(Rec_parameters_http_task_post_json*)pvParameters;
    if(strlen(rec_parameters_http_task_post_json->jsondata)>0){
        esp_http_client_config_t config = {
            .url = rec_parameters_http_task_post_json->url_send_data,
            .timeout_ms = 5000,
            .transport_type=HTTP_TRANSPORT_OVER_TCP,
            .method=HTTP_METHOD_POST,
            .keep_alive_enable=true
        };
        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_header(client, "Content-Type", "multipart/form-data; boundary=WebKitFormBoundary7MA4YWxkTrZu0gW");
        esp_http_client_set_header(client, "Connection", "keep-alive");
        esp_http_client_set_header(client, "Cache-Control","no-cache");
        char dest[500];
        const char *boundary1="--WebKitFormBoundary7MA4YWxkTrZu0gW\r\n";
        const char *boundary2="\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";
        
        const char *post_parameters2[36];
        sprintf(post_parameters2,"Content-Disposition: form-data; name=\"message\"\r\n\r\n"); 
        
        size_t offset=0; 
        memcpy(dest,boundary1,strlen(boundary1));
        offset=offset+strlen(boundary1);

        memcpy(dest+offset,post_parameters2,strlen(post_parameters2));
        offset=offset+strlen(post_parameters2);

        memcpy(dest+offset,rec_parameters_http_task_post_json->jsondata,strlen(rec_parameters_http_task_post_json->jsondata));
        offset=offset+strlen(rec_parameters_http_task_post_json->jsondata);

        memcpy(dest+offset,boundary2,strlen(boundary2));
        offset=offset+strlen(boundary2);
        dest[offset+1]=0;
       
        esp_err_t err = esp_http_client_open(client,0);

        esp_http_client_set_post_field(client, (const char*)dest, offset);
       
        ESP_LOGV(TAG_HTTP_REQ, "URL request  %s \n Size of post data %d \n Post Data %s\n",rec_parameters_http_task_post_json->url_send_data,offset,dest);

        //esp_err_t err = esp_http_client_perform(client);
        
        
        if (err == ESP_OK) {
            ESP_LOGI(TAG_HTTP_REQ, "HTTPS Response Status = %d, content_length = %d",
                    esp_http_client_get_status_code(client),
                    esp_http_client_get_content_length(client));
        } else {
            ESP_LOGE(TAG_HTTP_REQ, "Error HTTPS Response Status %s", esp_err_to_name(err));
        }
        rec_parameters_http_task_post_json->response_len = esp_http_client_fetch_headers(client);
        //rec_parameters_http_task_post_json->response_len=esp_http_client_get_content_length(client);

        if(rec_parameters_http_task_post_json->response_len> 0){
            //char charbuffer[get_content];
           // int data_len = esp_http_client_read(client, rec_parameters_http_task_post_json->response, rec_parameters_http_task_post_json->response_len);
            int data_read = esp_http_client_read_response(client, rec_parameters_http_task_post_json->response, rec_parameters_http_task_post_json->response_len);
            ESP_LOGV(TAG_HTTP_REQ,"Response data length %i \n Response Data %s", rec_parameters_http_task_post_json->response_len,rec_parameters_http_task_post_json->response);
        }
        esp_http_client_cleanup(client);
    }   
    stop_wait_request(rec_parameters_http_task_post_json->CONNECTED_BIT);
    vTaskDelete(xHandle);
}


void http_task_post_json(void *pvParameters){
    Rec_parameters_http_task_post_json *rec_parameters_http_task_post_json;
    rec_parameters_http_task_post_json=(Rec_parameters_http_task_post_json*)pvParameters;
    if(strlen(rec_parameters_http_task_post_json->jsondata)>0){
        esp_http_client_config_t config = {
            .url = rec_parameters_http_task_post_json->url_send_data,
            .timeout_ms = 5000,
            .transport_type=HTTP_TRANSPORT_OVER_TCP,
            .method=HTTP_METHOD_POST,
            .keep_alive_enable=true
        };
        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, rec_parameters_http_task_post_json->jsondata, strlen(rec_parameters_http_task_post_json->jsondata));
        
        ESP_LOGV(TAG_HTTP_REQ, "URL request  %s \n Size of post data %d \n Post Data %s\n",rec_parameters_http_task_post_json->url_send_data,strlen(rec_parameters_http_task_post_json->jsondata),rec_parameters_http_task_post_json->jsondata);

        esp_err_t err = esp_http_client_open(client,0);
        rec_parameters_http_task_post_json->response_len = esp_http_client_fetch_headers(client);
        
        if(rec_parameters_http_task_post_json->response_len> 0){
            //char charbuffer[get_content];
            //int data_read = esp_http_client_read_response(client, rec_parameters_http_task_post_json->response, rec_parameters_http_task_post_json->response_len);
            int data_len = esp_http_client_read(client, rec_parameters_http_task_post_json->response, rec_parameters_http_task_post_json->response_len);
            ESP_LOGV(TAG_HTTP_REQ,"Response data length %i \n Response Data %s", rec_parameters_http_task_post_json->response_len,rec_parameters_http_task_post_json->response);
        }
       // esp_http_client_close(client);
        esp_http_client_cleanup(client);
    }   
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    stop_wait_request(rec_parameters_http_task_post_json->CONNECTED_BIT);
    vTaskDelete(xHandle);
}


void stop_wait_request(int connectedbit){
    xEventGroupSetBits(http_event_group,connectedbit);
}

void wait_http_request(int connectedbit){
    if(http_event_group!=NULL){
        ESP_LOGI(TAG_HTTP_REQ, "\n Wait  http_task \n");
        xEventGroupWaitBits(http_event_group, connectedbit, false, true,portMAX_DELAY);
        ESP_LOGD(TAG_HTTP_REQ,"*****************  MEM  after xEventGroupWaitBits %d   *******************",heap_caps_get_free_size(MALLOC_CAP_8BIT));
    }
}
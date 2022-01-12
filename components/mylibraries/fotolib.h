#ifndef FOTOLIB_H_
#define FOTOLIB_H_


#include <stdio.h>
#include <math.h>
#include <esp_err.h>

#include <esp_log.h>
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <string.h>

#include "sdcard.h"
#include "confing_file.h"
#include "http_req.h"
#include "mycamera.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"



typedef struct {
    int top;
    int bottom;
    int left;                
    int right;
    int change;
}Rec_change_img_t;

#ifdef __cplusplus
extern "C" {
#endif
Rec_config_file_t rec_config_file;


static const char *TAGFOTOLIB = "Foto library";


float get_percentage_change(const uint8_t * previous_img,const uint8_t * current_img,const int imagerow,const int imagecol,int updown);
int save_file_as_jpg(const char * filename, camera_fb_t * fb);

void create_last_object(const uint8_t *previous_img,const uint8_t *current_img, uint8_t *object_img,const int imagerow,const int imagecol, const int updown, const int threshold,const int half_pad_size);
void create_black_white_image(const uint8_t *current_img, uint8_t *black_white_img,const int imagerow,const int imagecol, const int threshold);

void find_changes( const uint8_t *black_white_img, Rec_change_img_t *rec_change_img ,const int imagerow,const int imagecol, const int threshold);


error_t create_black_white_last_object_from_files(const char *previous_img,const char *current_img, uint8_t *black_white_img, const int updown, const int threshold);
error_t crop_image(const uint8_t *src_image,uint8_t *dest_image,  Rec_change_img_t *rec_change_img, const size_t src_image_w,const size_t src_image_h, size_t *len_dest_image);

void clear_image(uint8_t *black_white_img,const int imagerow,const int imagecol,const int check_area_size);

#ifdef __cplusplus
}
#endif

#endif
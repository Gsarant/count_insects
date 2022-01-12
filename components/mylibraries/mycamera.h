#ifndef MYCAMERA_H_
#define MYCAMERA_H_


#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include "driver/gpio.h"
#include <esp_camera.h>


#include <nvs_flash.h>
#include "sys/param.h"
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BOARD_ESP32CAM_AITHINKER
#ifdef BOARD_ESP32CAM_AITHINKER
    #define CAM_PIN_PWDN 32
    #define CAM_PIN_RESET -1 //software reset will be performed
    #define CAM_PIN_XCLK 0
    #define CAM_PIN_SIOD 26
    #define CAM_PIN_SIOC 27

    #define CAM_PIN_D7 35
    #define CAM_PIN_D6 34
    #define CAM_PIN_D5 39
    #define CAM_PIN_D4 36
    #define CAM_PIN_D3 21
    #define CAM_PIN_D2 19
    #define CAM_PIN_D1 18
    #define CAM_PIN_D0 5
    #define CAM_PIN_VSYNC 25
    #define CAM_PIN_HREF 23
    #define CAM_PIN_PCLK 22
#endif//BOARD_ESP32CAM_AITHINKER

//#define IMAGE_SIZE 240*240
#define IMAGE_SIZE_ROW 240
#define IMAGE_SIZE_COL 240

#ifdef __cplusplus
extern "C" {
#endif

static  char *TAGCAMERA = "Camera ";


esp_err_t init_camera();



#ifdef __cplusplus
}
#endif

#endif
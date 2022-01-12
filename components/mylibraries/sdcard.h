#ifndef SDCARD_H_
#define SDCARD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_err.h>
#include <esp_log.h>
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "freertos/FreeRTOS.h"

#include <nvs_flash.h>
#include <esp_vfs_fat.h>


#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include <sdmmc_cmd.h>
#include "sdkconfig.h"

#define MOUNT_POINT "/sdcard"


#ifdef __cplusplus
extern "C" {
#endif
sdmmc_card_t* card;
static bool sd_enable=false;

esp_err_t init_sd(void);

esp_err_t save_file(const char *filename,const uint8_t * filedata, const size_t len_data);

esp_err_t read_file(const char *filename, uint8_t *file_data,  size_t *len_data );

#ifdef __cplusplus
}
#endif

#endif
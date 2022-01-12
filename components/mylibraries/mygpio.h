#ifndef MY_GPIO_H
#define MY_GPIO_H

#include <stdio.h>
#include <esp_log.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include <math.h>

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (4) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (2) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz



#ifdef __cplusplus
extern "C" {
#endif
static const char *TAGMYGPIO = "MYGPIO Module";
void flash_on(int x);
void flash_off(void);


#ifdef __cplusplus
}
#endif

#endif
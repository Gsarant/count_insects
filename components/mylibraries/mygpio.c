#include "mygpio.h"

    
static void ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void flash_on(int x){
    ledc_init();
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, (uint32_t)(pow(2,13)-1)*((float)x/100) ));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

void flash_off(void){
    uint32_t idle_level=0;
    ESP_ERROR_CHECK(ledc_stop(LEDC_MODE, LEDC_CHANNEL,idle_level));
    
}
    
    // gpio_config_t led_conf;
    // led_conf.intr_type = GPIO_INTR_DISABLE;
    // led_conf.mode = GPIO_MODE_OUTPUT;
    // //led_conf.pin_bit_mask = GPIO_NUM_33;
    // led_conf.pin_bit_mask = GPIO_NUM_4;
    // led_conf.pull_down_en = 0;
    // led_conf.pull_up_en = 0;    
    // gpio_config(&led_conf);
    // gpio_set_level(GPIO_NUM_4, 0);
    // ESP_LOGI(TAG, "GPIO :%d   is ON\n",GPIO_NUM_4);
    //  vTaskDelay(5000 / portTICK_RATE_MS);
    //  gpio_set_level(GPIO_NUM_4, 1);
    // ESP_LOGI(TAG, "GPIO :%d   is OFF\n",GPIO_NUM_4);
    // //INIT QUICKLY
    // gpio_pad_select_gpio(GPIO_NUM_33);
    // gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);
    // gpio_set_level(GPIO_NUM_33, 0);
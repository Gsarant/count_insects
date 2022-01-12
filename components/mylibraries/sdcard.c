#include "sdcard.h"

static const char *TAGSD = "SD Module";


#ifdef CONFIG_IDF_TARGET_ESP32
#include "driver/sdmmc_host.h"
#endif

#define USE_SPI_MODE

// ESP32-S2 doesn't have an SD Host peripheral, always use SPI:
#ifdef CONFIG_IDF_TARGET_ESP32S2
#ifndef USE_SPI_MODE
#define USE_SPI_MODE
#endif // USE_SPI_MODE
// on ESP32-S2, DMA channel must be the same as host id
#define SPI_DMA_CHAN    host.slot
#endif //CONFIG_IDF_TARGET_ESP32S2

// DMA channel to be used by the SPI peripheral
#ifndef SPI_DMA_CHAN
#define SPI_DMA_CHAN    1
#endif //SPI_DMA_CHAN


#ifdef USE_SPI_MODE
#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 15
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   13
#endif //USE_SPI_MODE

esp_err_t init_sd(void){
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        #ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
            .format_if_mount_failed = true,
        #else
            .format_if_mount_failed = false,
        #endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
            .max_files = 5,
            .allocation_unit_size = 16 * 1024
        };
        
        const char mount_point[] = MOUNT_POINT;
        ESP_LOGI(TAGSD, "Initializing SD card");

        
    #ifndef USE_SPI_MODE
        ESP_LOGI(TAGSD, "Using SDMMC peripheral");
        sdmmc_host_t host = SDMMC_HOST_DEFAULT();
        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
        gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
        gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
        gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
        gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
        gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes
        ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    #else
        ESP_LOGI(TAGSD, "Using SPI peripheral");
        sdmmc_host_t host = SDSPI_HOST_DEFAULT();
        spi_bus_config_t bus_cfg = {
            .mosi_io_num = PIN_NUM_MOSI,
            .miso_io_num = PIN_NUM_MISO,
            .sclk_io_num = PIN_NUM_CLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 4000,
        };
        ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
        if (ret != ESP_OK) {
            ESP_LOGE(TAGSD, "Failed to initialize bus.");
            if(ret==ESP_OK)
                sd_enable=true;
            else
                sd_enable=false;
            return ret;
        }

        sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
        slot_config.gpio_cs = PIN_NUM_CS;
        slot_config.host_id = host.slot;

        ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    #endif //USE_SPI_MODE

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAGSD, "Failed to mount filesystem. "
                "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAGSD, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        
    }
    if(ret==ESP_OK)
        sd_enable=true;
    else
        sd_enable=false;
    return ret;
};

esp_err_t read_dir(){
    FRESULT res;
    ESP_LOGI(TAGSD,"try to open dir");
    FF_DIR* dir=malloc(sizeof(dir)); 
    res= f_opendir(dir, "/sdcard");
    if (res != FR_OK) {
        ESP_LOGE(TAGSD, "Failed to open dir");
        return ESP_FAIL;
    }
    else{
        FILINFO* fileinfo=malloc(sizeof(fileinfo));
        ESP_LOGI(TAGSD, "read dir ");
        res = f_readdir(dir, fileinfo);
        while (res == FR_OK && fileinfo->fname[0]) {
            printf("%s\n", fileinfo->fname);
            ESP_LOGI(TAGSD,"%s",fileinfo->fname);
            res = f_readdir(dir, fileinfo);
        }
        free(fileinfo);
        f_closedir(dir); 
        free(dir);
        ESP_LOGI(TAGSD, "Closeing dir");
        return ESP_OK;
    }
};

esp_err_t read_file(const char *filename,  uint8_t *file_data, size_t *len_data ){
    struct stat st;
    if (stat(filename, &st) == 0) {
        if(st.st_size>0){
            FILE* f = fopen(filename, "r");
            if (f == NULL) {
                ESP_LOGE(TAGSD, "Failed to open file for reading");
                return ESP_FAIL;
            }
            else{
                ESP_LOGI(TAGSD, "Read file len_data ");
                *len_data=fread( file_data,1,*len_data,f);
                if((int)*len_data<=0){
                    fclose(f); 
                    ESP_LOGE(TAGSD, "Failed to Read data from  file %s  size data %d", filename,(int)*len_data);
                    return ESP_FAIL;
                }
                fclose(f); 
                ESP_LOGI(TAGSD, " Read file %s  size data %d", filename,(int)*len_data);
                return ESP_OK;
            }
        }else{
            ESP_LOGE(TAGSD, "Size of file %s is zero",filename);
            return ESP_FAIL;
        }

    }else{
        ESP_LOGE(TAGSD, "There is not file %s to read",filename);
        return ESP_FAIL;
    }
};    

esp_err_t save_file(const char *filename,const uint8_t * file_data, const size_t len_data){
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        ESP_LOGE(TAGSD, "Failed to open file for writing");
        return ESP_FAIL;
    }
    else{
        size_t write_size=fwrite(file_data,1,len_data,f);
        if(write_size<=0){
            ESP_LOGE(TAGSD, "Failed to write in file %s size %d", filename,write_size);
            fclose(f); 
            return ESP_FAIL;
        }
        fclose(f); 
        ESP_LOGI(TAGSD, "Write in file %s size %d", filename,write_size);
        return ESP_OK;
    }
};


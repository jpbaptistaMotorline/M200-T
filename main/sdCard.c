/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#include "sdCard.h"
#include "esp_log.h"
#include "fcntl.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

// #include "gpio.h"



const char *file_hello = "/sdcard/MOTORLINE.txt";
const char *file_foo = MOUNT_POINT "/m4g.txt";

esp_vfs_fat_sdmmc_mount_config_t mount_config =
    {

        .format_if_mount_failed = false,
        .max_files = 1,
        .allocation_unit_size = 8 * 1024};

const char mount_point[] = MOUNT_POINT;
// //ESP_LOGI("TAG", "Initializing SD card");

// Use settings defined above to initialize SD card and mount FAT filesystem.
// Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
// Please check its source code and implement error recovery when developing
// production applications.
sdmmc_card_t *card;
sdmmc_host_t host = SDSPI_HOST_DEFAULT();
sdspi_device_config_t slot_config;
spi_host_device_t spi_host_device;

spi_bus_config_t bus_cfg = {
    .mosi_io_num = PIN_NUM_MOSI,
    .miso_io_num = PIN_NUM_MISO,
    .sclk_io_num = PIN_NUM_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 4000,
};

void free_SPI_Host()
{
    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    // ////printf("\n\n unmount 222\n\n");
    //ESP_LOGI("TAG", "Card unmounted");

    // deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
}

void mount_SD_CARD()
{
    esp_err_t ret;

    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    //ESP_LOGI("TAG", "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            //ESP_LOGE("TAG", "Failed to mount filesystem. ""If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else
        {
            //ESP_LOGE("TAG", "Failed to initialize the card (%s). ""Make sure SD card lines have pull-up resistors in place.",esp_err_to_name(ret));
        }
        return;
    }
    //ESP_LOGI("TAG", "Files System mounted12");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    //ESP_LOGI("TAG", "Opening file %s", "/sdcard/m4g.csv");
    char *fileName;

    uint8_t lastMonth = get_NVS_Last_Month();
    //////printf("\nLAST MONTH %d\n", lastMonth);
    FILE *f;

    if (get_RTC_System_Time())
    {

        // lastMonth = 5;
        asprintf(&fileName, "/sdcard/%02d.csv", nowTime.month);
        //////printf("\nfileName sdcard %s\n", fileName);

        if (lastMonth == nowTime.month)
        {

            f = fopen(fileName, "a");
            long int file_Len = 0;

            fseek(f, 0, SEEK_END);

            file_Len = ftell(f);

            //////printf("\nFILE LEN = %s - %ld\n", fileName, file_Len);

            if (file_Len < 10)
            {
                fclose(f);
                f = fopen(fileName, "w");
                fprintf(f, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s\n", return_Json_SMS_Data("NAME"), ';', return_Json_SMS_Data("PHONE_NUMBER"), ';', return_Json_SMS_Data("RELAY"), ';', return_Json_SMS_Data("RELAY_STATE"), ';', return_Json_SMS_Data("DATE"), ';', return_Json_SMS_Data("TIME"), ';', return_Json_SMS_Data("TYPE"), ';', return_Json_SMS_Data("ERROR"));
            }

            //////printf("\nsdcard21 \n");

            //////printf("\nsdcard22 \n");
            fclose(f);
        }
        else
        {
            lastMonth = nowTime.month;
            save_NVS_Last_Month(lastMonth);
            f = fopen(fileName, "w");
            fprintf(f, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s\n", return_Json_SMS_Data("NAME"), ';', return_Json_SMS_Data("PHONE_NUMBER"), ';', return_Json_SMS_Data("RELAY"), ';', return_Json_SMS_Data("RELAY_STATE"), ';', return_Json_SMS_Data("DATE"), ';', return_Json_SMS_Data("TIME"), ';', return_Json_SMS_Data("TYPE"), ';', return_Json_SMS_Data("ERROR"));

            fclose(f);
            //////printf("\nsdcard32 \n");
        }
    }

    free(fileName);

    //ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d", xPortGetFreeHeapSize());
    //ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
    //ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    //ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

void init_SDCard()
{

    //ESP_LOGI("TAG", "xPortGetFreeHeapSize1212            : %d", xPortGetFreeHeapSize());
    //ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
    //ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    //ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    // ////printf("\n\ntask_Send_LOG_File 11\n\n");
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);

    // spi_bus_free()
    if (ret != ESP_OK)
    {
        //ESP_LOGE("TAG", "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    //ESP_LOGI("TAG", "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    // ////printf("\n\ntask_Send_LOG_File 12\n\n");
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            //ESP_LOGE("TAG", "Failed to mount filesystem. ""If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else
        {
            //ESP_LOGE("TAG", "Failed to initialize the card (%s).""Make sure SD card lines have pull-up resistors in place.",esp_err_to_name(ret));
        }
        return;
    }
    //ESP_LOGI("TAG", "Files System mounted12");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    //ESP_LOGI("TAG", "Opening file %s", "/sdcard/m4g.csv");
    char *fileName;

    uint8_t lastMonth = get_NVS_Last_Month();
    //////printf("\nLAST MONTH %d\n", lastMonth);
    FILE *f;

    if (get_RTC_System_Time())
    {

        // lastMonth = 5;
        asprintf(&fileName, "/sdcard/%02d.csv", nowTime.month);
        //////printf("\nfileName sdcard %s\n", fileName);

        if (lastMonth == nowTime.month)
        {

            f = fopen(fileName, "a");
            long int file_Len = 0;

            fseek(f, 0, SEEK_END);

            file_Len = ftell(f);

            //////printf("\nFILE LEN = %s - %ld\n", fileName, file_Len);

            if (file_Len < 10)
            {
                fclose(f);
                f = fopen(fileName, "w");
                fprintf(f, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s\n", return_Json_SMS_Data("NAME"), ';', return_Json_SMS_Data("PHONE_NUMBER"), ';', return_Json_SMS_Data("RELAY"), ';', return_Json_SMS_Data("RELAY_STATE"), ';', return_Json_SMS_Data("DATE"), ';', return_Json_SMS_Data("TIME"), ';', return_Json_SMS_Data("TYPE"), ';', return_Json_SMS_Data("ERROR"));
            }

            //////printf("\nsdcard21 \n");

            //////printf("\nsdcard22 \n");
            fclose(f);
        }
        else
        {
            lastMonth = nowTime.month;
            save_NVS_Last_Month(lastMonth);
            f = fopen(fileName, "w");
            fprintf(f, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s\n", return_Json_SMS_Data("NAME"), ';', return_Json_SMS_Data("PHONE_NUMBER"), ';', return_Json_SMS_Data("RELAY"), ';', return_Json_SMS_Data("RELAY_STATE"), ';', return_Json_SMS_Data("DATE"), ';', return_Json_SMS_Data("TIME"), ';', return_Json_SMS_Data("TYPE"), ';', return_Json_SMS_Data("ERROR"));

            fclose(f);
            //////printf("\nsdcard32 \n");
        }
    }

    free(fileName);

    //ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d", xPortGetFreeHeapSize());
    //ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
    //ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    //ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));

    /* //////printf("\n\n unmount 111\n\n");
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    //////printf("\n\n unmount 222\n\n");
    //ESP_LOGI("TAG", "Card unmounted");

    // deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
    //////printf("\n\n unmount 333\n\n"); */
}

void init_rdySem_Control_SD_Card_Write()
{
    rdySem_Control_SD_Card_Write = xSemaphoreCreateBinary();
    xSemaphoreGive(rdySem_Control_SD_Card_Write);
}

void sdCard_Write_LOGS(sdCard_Logs_struct *logs_Struct)
{
    //////printf("\n\n sdcard 11\n\n");

    //////printf("\n\n sdcard 22\n\n");
    xSemaphoreTake(rdySem_Control_SD_Card_Write, pdMS_TO_TICKS(10000));

    char *data_to_save;

    ////printf("\n\n data to save111 %s\n\n", logs_Struct->name);
    ////printf("\n\n data to save222 %s\n\n", logs_Struct->phone);

    if (!strcmp(logs_Struct->name, "S/N") || strlen(logs_Struct->name) == 0)
    {
        memset(logs_Struct->name, 0, sizeof(logs_Struct->name));
        sprintf(logs_Struct->name, "%s", return_Json_SMS_Data("NO_NAME"));
    }

    if (!strcmp(logs_Struct->type, "WEB"))
    {
        asprintf(&data_to_save, "%s;%s;%s;%s;%s;%s;%s", "", logs_Struct->phone, logs_Struct->relay, logs_Struct->relay_state, logs_Struct->date, logs_Struct->type, logs_Struct->error);
    }
    else
    {
        asprintf(&data_to_save, "%s;%s;%s;%s;%s;%s;%s", logs_Struct->name, logs_Struct->phone, logs_Struct->relay, logs_Struct->relay_state, logs_Struct->date, logs_Struct->type, logs_Struct->error);
    }

    printf("\n\n data to save %s\n\n", data_to_save);

    UDP_logs_label = get_INT8_Data_From_Storage(NVS_NETWORK_LABEL_SEND_LOGS, nvs_System_handle);
    ////printf("\n\n data to save %s\n\n", data_to_save);

    if (UDP_logs_label == 1)
    {
        //char *UDP_log;
        ////printf("\n\n data to save5555 %s\n\n", logs_Struct->phone);
        memset(&logs_Struct->mqttLogs_info,0,sizeof(logs_Struct->mqttLogs_info));
        sprintf(logs_Struct->mqttLogs_info.data, "# %s", data_to_save);
        ////printf("\n\n data to save 44 %d\n\n", UDP_logs_label);
        vTaskDelay(pdMS_TO_TICKS(100));
        //send_UDP_Send(UDP_log);
        send_UDP_queue(&logs_Struct->mqttLogs_info);
        //free(UDP_log);
    }
    else
    {
        if (UDP_logs_label == 255)
        {
            save_INT8_Data_In_Storage(NVS_NETWORK_LABEL_SEND_LOGS, 0, nvs_System_handle);
            UDP_logs_label = 0;
        }
    }
    ////printf("\n\n data to save 7567 %s\n\n", data_to_save);
    if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD))
    {

        init_SDCard();
        ////printf("\n\n sdcard 22\n\n");

        //ESP_LOGI("TAG", "xPortGetFreeHeapSize sd write1            : %d", xPortGetFreeHeapSize());
        //ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
        //ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
        //ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));

        FILE *f = NULL;

        ////printf("\ndata to save sdcard %s\n", data_to_save);

        uint8_t lastMonth = get_NVS_Last_Month();
        //////printf("\nLAST MONTH %d\n", lastMonth);

        if (lastMonth >= 0 && lastMonth <= 12)
        {
            /* code */

            if (get_RTC_System_Time())
            {
                char *fileName;
                // lastMonth = 5;
                asprintf(&fileName, "/sdcard/%02d.csv", nowTime.month);
                //////printf("\nfileName sdcard %s\n", fileName);
                if (lastMonth == nowTime.month)
                {
                    f = fopen(fileName, "a");
                    //////printf("\nsdcard21 \n");

                    //////printf("\nsdcard22 \n");
                }
                else
                {
                    lastMonth = nowTime.month;
                    save_NVS_Last_Month(lastMonth);
                    f = fopen(fileName, "w");
                    fprintf(f, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s\n", return_Json_SMS_Data("NAME"), ';', return_Json_SMS_Data("PHONE_NUMBER"), ';', return_Json_SMS_Data("RELAY"), ';', return_Json_SMS_Data("RELAY_STATE"), ';', return_Json_SMS_Data("DATE"), ';', return_Json_SMS_Data("TIME"), ';', return_Json_SMS_Data("TYPE"), ';', return_Json_SMS_Data("ERROR"));
                    //////printf("\nsdcard31 \n");

                    //////printf("\nsdcard32 \n");
                }
                free(fileName);
            }
            else
            {
                // asprintf(&sdCard_Date, "%s", "ERRO GET TIME");
            }
        }

        if (f == NULL)
        {

            //ESP_LOGE("TAG", "Failed to open file for writing");
        }
        else
        {
            //ESP_LOGI("TAG", "Opening file %s", "/sdcard/m4g.txt");

            fprintf(f, "%s\n", data_to_save);

            //ESP_LOGI("TAG", "File written");

            fclose(f);
            //////printf("\nsdcard1 \n");

            //////printf("\nsdcard3 \n");

            // !!!!!!!!  APAGAR ISTO  !!!!!!!!
            // nowTime.month = lastMonth;

            //////printf("\n\n unmount 111\n\n");
            esp_vfs_fat_sdcard_unmount(mount_point, card);
            //////printf("\n\n unmount 222\n\n");
        }

        //ESP_LOGI("TAG", "Card unmounted");

        // deinitialize the bus after all devices are removed
        spi_bus_free(host.slot);

        //////printf("\n\n unmount 333\n\n");
    }
    
    free(data_to_save);
    xSemaphoreGive(rdySem_Control_SD_Card_Write);
    //ESP_LOGI("TAG", "xPortGetFreeHeapSize sd write2           : %d", xPortGetFreeHeapSize());
    //ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
    //ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    //ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    //////printf("\n\n sdcard 33\n\n");
}

esp_err_t format_sdcard()
{
    esp_err_t ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);

    // spi_bus_free()
    if (ret != ESP_OK)
    {
        //ESP_LOGE("TAG", "Failed to initialize bus.");
        return ESP_FAIL;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    //ESP_LOGI("TAG", "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            //ESP_LOGE("TAG", "Failed to mount filesystem. ""If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else
        {
            //ESP_LOGE("TAG", "Failed to initialize the card (%s). ""Make sure SD card lines have pull-up resistors in place.",esp_err_to_name(ret));
        }
        return 0;
    }

    char drv[3] = {'0', ':', 0};
    const size_t workbuf_size = 4096;
    void *workbuf = NULL;
    esp_err_t err = ESP_OK;
    ESP_LOGW("sdcard", "Formatting the SD card");

    size_t allocation_unit_size = 16 * 1024;

    workbuf = ff_memalloc(workbuf_size);
    //////printf("\n\nSDCARD 1\n\n");
    if (workbuf == NULL)
    {
        //////printf("\n\nSDCARD 1.1\n\n");
        return ESP_ERR_NO_MEM;
    }
    //////printf("\n\nSDCARD 2\n\n");
    size_t alloc_unit_size = esp_vfs_fat_get_allocation_unit_size(
        card->csd.sector_size,
        allocation_unit_size);

    //////printf("\n\nSDCARD 3\n\n");

    FRESULT res = f_mkfs(drv, FM_ANY, alloc_unit_size, workbuf, workbuf_size);
    if (res != FR_OK)
    {
        //////printf("\n\nSDCARD 3.1\n\n");
        err = ESP_FAIL;
        //ESP_LOGE("sdcard", "f_mkfs failed (%d)", res);
    }
    //////printf("\n\nSDCARD 4\n\n");
    free(workbuf);
    //////printf("\n\nSDCARD 5\n\n");

    //ESP_LOGI("sdcard", "Successfully formatted the SD card");

    //////printf("\n\n unmount 111\n\n");
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    //////printf("\n\n unmount 222\n\n");
    //ESP_LOGI("TAG", "Card unmounted");

    // deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
    //////printf("\n\n unmount 333\n\n");

    return err;
}

void read_BackupFile()
{
    //ESP_LOGI("TAG", "Opening file %s", "/sdcard/backup.txt");
    FILE *f = fopen("/sdcard/backup.txt", "r");
    FILE *f_contacts = fopen("/spiffs/hello.txt", "w");

    if (f == NULL)
    {
        //ESP_LOGE("TAG", "Failed to open file for writing");
        return;
    }
    else
    {
        //////printf("\nbackup file open sucess\n");
    }

    char buffer[256];
    char sdCard_DecrypData[256];

    memset(buffer, 0, sizeof(buffer));

    char ch = 0;
    int SDCARD_charCounter = 0;

    do
    {
        ch = fgetc(f);

        if (SDCARD_charCounter < 255)
        {
            buffer[SDCARD_charCounter] = ch;
            SDCARD_charCounter++;
        }
        else
        {
            buffer[SDCARD_charCounter] = ch;
            SDCARD_charCounter++;
            // decrypt1(&buffer, 256);

            for (int i = 0; i < 256; i++)
            {
                if (buffer[i] == '$')
                {
                    buffer[i] = 0;
                }
            }

            fprintf(f_contacts, "%s\n", buffer);

            for (int i = 0; i < 256; i++)
            {
                //////printf("%02x ", buffer[i]);
            }

            SDCARD_charCounter = 0;
        }

        if (feof(f))
        {
            //////printf("\nsd counter %d\n", SDCARD_charCounter);
            break;
        }

    } while (1);

    fclose(f_contacts);
    fclose(f);

    // remove("/sdcard/backup.txt");
}

void hex_to_string(char *msg, size_t msg_sz, char *hex, size_t hex_sz)
{
    memset(msg, '\0', msg_sz);
    if (hex_sz % 2 != 0 || hex_sz / 2 >= msg_sz)
        return;

    for (int i = 0; i < hex_sz; i += 2)
    {
        uint8_t msb = (hex[i + 0] <= '9' ? hex[i + 0] - '0' : (hex[i + 0] & 0x5F) - 'A' + 10);
        uint8_t lsb = (hex[i + 1] <= '9' ? hex[i + 1] - '0' : (hex[i + 1] & 0x5F) - 'A' + 10);
        msg[i / 2] = (msb << 4) | lsb;
    }
}

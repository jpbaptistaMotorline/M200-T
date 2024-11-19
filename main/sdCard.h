/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#ifndef _SD_CARD_H_
#define _SD_CARD_H_

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "ff.h"
#include "vfs_fat_internal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/sdspi_host.h"
#include "driver/sdmmc_types.h"

#include "core.h"

#define SPI_DMA_CHAN SPI_DMA_CH_AUTO

#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO 37
#define PIN_NUM_MOSI 35
#define PIN_NUM_CLK 36
#define PIN_NUM_CS 34

#define FILE_SDCARD_LOGS "/sdcard/M200.csv"
#define FILE_SDCARD_BACKUP "/sdcard/backup.txt"

#define SDCARD_WRITE_LOGS_OPERATION 0
#define SDCARD_READ_LOGS_OPERATION 1
#define SDCARD_WRITE_BACKUP_OPERATION 2
#define SDCARD_READ_BACKUP_OPERATION 3

//sdmmc_card_t *card;
//sdmmc_host_t host;
//sdspi_device_config_t slot_config;
//spi_host_device_t spi_host_device;

void init_rdySem_Control_SD_Card_Write();

static SemaphoreHandle_t rdySem_Control_SD_Card_Write;


typedef struct SD_CARD_Struct
{
  uint8_t SDCard_operation;
  char filename[30];
  char writePayload[185];
  char readPayload[185];

} sdCard_struct;

typedef struct SD_CARD_LOGS_STRUCT
{
  char name[40];
  char phone[50];
  char relay[6];
  char relay_state[30];
  char date[40];
  char type[250];
  char error[50];
  mqtt_information mqttLogs_info;

} sdCard_Logs_struct;

char *hexToAscii(char hex[]);
void init_SDCard();
void sdCard_Write_LOGS(sdCard_Logs_struct *logs_Struct);
void read_BackupFile();
void hex_to_string(char *msg, size_t msg_sz, char *hex, size_t hex_sz);
void mount_SD_CARD();
esp_err_t format_sdcard();

void free_SPI_Host();



#endif
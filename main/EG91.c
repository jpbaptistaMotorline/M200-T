#include "EG91.h"
#include "string.h"
#include "stdio.h"
// #include "uart.h"
#include "AT_CMD_List.h"
// #include "gpio.h"
#include "esp_task_wdt.h"
#include "core.h"
#include "users.h"
#include "rele.h"
#include "cmd_list.h"
#include "math.h"
#include "timer.h"
#include "sdCard.h"
#include "erro_list.h"
#include "routines.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "pcf85063.h"
#include "crc32.h"
#include "ble_spp_server_demo.h"
#include "UDP_Codes.h"
#include "core.h"

#include "esp_ota_ops.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "errno.h"
#include <regex.h>

// #include "semphr.h"
int aux_label_inCall = 0;

TaskHandle_t handle_SMS_TASK;
TaskHandle_t handle_INCOMING_CALL_TASK;
TaskHandle_t handle_SEND_SMS_TASK;
TaskHandle_t handle_UDP_TASK;

uint8_t mqtt_openLabel = 0;
uint8_t mqtt_connectLabel = 0;

uint8_t call_Type;

uint8_t send_ATCommand_Label;
uint8_t incomingCall_Label;
uint8_t call_label;
uint8_t label_Network_Activate;
int RSSI_VALUE;
// char SIM_Balance[500];
char EG91_ICCID_Number[25];
char EG91_IMEI[20];
uint8_t SIM_CARD_PIN_status;

void give_rdySem_Control_SMS_UDP()
{
	xSemaphoreGive(rdySem_Control_SMS_UDP);
}
void take_rdySem_Control_SMS_UDP()
{
	xSemaphoreTake(rdySem_Control_SMS_UDP, pdMS_TO_TICKS(4000));
}

uint8_t EG91_PowerOn()
{
	uint8_t EG91_PowerON_TimeOut = 0;

	// ////printf("\ngpio_get_level(GPIO_INPUT_IO_EG91_STATUS) %d\n", gpio_get_level(GPIO_INPUT_IO_EG91_STATUS));

	if (!gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
	{
		gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
		vTaskDelay(pdMS_TO_TICKS(200));
		// ////printf("\n\nINIT EG915 4\n\n");

		gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
		vTaskDelay(pdMS_TO_TICKS(2000));
		// ////printf("\n\nINIT EG915 5\n\n");
		gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
		// ////printf("\ngpio_get_level(GPIO_INPUT_IO_EG91_STATUS) %d\n", gpio_get_level(GPIO_INPUT_IO_EG91_STATUS));
		vTaskDelay(pdMS_TO_TICKS(8000));
		// ////printf("\n\nINIT EG915 6\n\n");
		while (!gpio_get_level(GPIO_INPUT_IO_EG91_STATUS) && EG91_PowerON_TimeOut < 3)
		{
			// ////printf("\nerror power on\n");
			vTaskDelay(pdMS_TO_TICKS(1000));
			EG91_PowerON_TimeOut++;
		}

		if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
		{
			vTaskDelay(pdMS_TO_TICKS(10000));
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		/* if (check_NetworkState() != 1)
		{ */
		// ////printf("\n\n\n before power on 11\n\n\n");
		return EG91_Power_Reset();
		//}

		// xSemaphoreGive(rdySem_Control_Send_AT_Command);
	}

	return 0;
}

uint8_t EG91_Power_OFF()
{
	uint8_t EG91_PowerON_TimeOut = 0;
	/*  gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
	vTaskDelay(pdMS_TO_TICKS(5000));

	gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
	vTaskDelay(pdMS_TO_TICKS(40000)); */
	// system_stack_high_water_mark("AT+QPOWD");
	if (EG91_send_AT_Command("AT+QPOWD", "OK", 1000))
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
		if (!gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
		{
			return 1;
		}
		else
		{
			/*  gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
		vTaskDelay(pdMS_TO_TICKS(2000)); */
			gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
			vTaskDelay(pdMS_TO_TICKS(5000));
			// ////printf("\n\nbefore reset 11\n\n");
			gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
			// vTaskDelay(pdMS_TO_TICKS(60000));
			// ////printf("\n\nbefore reset 22\n\n");

			while (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS) && EG91_PowerON_TimeOut < 40)
			{
				vTaskDelay(pdMS_TO_TICKS(2000));
				// ////printf("\n\nbefore reset - %d\n\n", EG91_PowerON_TimeOut);
				if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
				{
					EG91_PowerON_TimeOut++;
				}
				else
				{
					break;
				}
			}
		}
	}
	else
	{
		/*  gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
		 vTaskDelay(pdMS_TO_TICKS(2000)); */
		gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
		vTaskDelay(pdMS_TO_TICKS(5000));
		// ////printf("\n\nbefore reset 11\n\n");
		gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
		// vTaskDelay(pdMS_TO_TICKS(60000));
		// ////printf("\n\nbefore reset 22\n\n");

		while (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS) && EG91_PowerON_TimeOut < 40)
		{
			vTaskDelay(pdMS_TO_TICKS(2000));
			// ////printf("\n\nbefore reset - %d\n\n", EG91_PowerON_TimeOut);
			if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
			{
				EG91_PowerON_TimeOut++;
			}
			else
			{
				break;
			}
		}
	}

	if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t EG91_Power_Reset()
{

	uint8_t EG91_PowerON_TimeOut = 0;
	// ////printf("\n ENTER IN RESET EG\n");

	if (EG91_send_AT_Command("AT+QPOWD=1", "OK", 1500))
	{

		// ////printf("\n ENTER IN RESET EG 11\n");
		vTaskDelay(pdMS_TO_TICKS(1000));

		if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
		{
			// ////printf("\n RESET EG22\n");
			return 0;
		}
		else
		{
			gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
			vTaskDelay(pdMS_TO_TICKS(200));

			gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
			vTaskDelay(pdMS_TO_TICKS(2000));
			gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
			vTaskDelay(pdMS_TO_TICKS(8000));
		}
	}
	else
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
		if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
		{
			EG91_Power_OFF();
			while (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS) && EG91_PowerON_TimeOut < 10)
			{
				vTaskDelay(pdMS_TO_TICKS(1000));
				EG91_PowerON_TimeOut++;
			}

			if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
			{
				// xTaskResumeAll();
				// xTaskResumeAll();
				return 0;
			}
			else
			{
				gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
				vTaskDelay(pdMS_TO_TICKS(200));

				gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
				vTaskDelay(pdMS_TO_TICKS(2000));
				gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
				vTaskDelay(pdMS_TO_TICKS(8000));
			}
		}
		else
		{
			gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
			vTaskDelay(pdMS_TO_TICKS(200));
			gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
			vTaskDelay(pdMS_TO_TICKS(2000));
			gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
			vTaskDelay(pdMS_TO_TICKS(8000));
		}
	}
	// }

	if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
	{
		// ////printf("\n RESET EG555\n");
		return 1;
	}
	else
	{
		// ////printf("\n RESET EG666\n");
		return 0;
	}

	return 0;
}

uint8_t check_NetworkState()
{
	// EG91_send_AT_Command("AT+COPS?", "OK", 5000);

	if (!EG91_send_AT_Command("AT+CREG?", "AT+CREG", 1000))
	{
		return 2;
	}

	if (!EG91_send_AT_Command("AT+CGREG?", "AT+CGREG", 1000))
	{
		return 3;
	}

	return EG91_send_AT_Command("AT+CSQ", "CSQ", 1000);
}

uint8_t parse_NetworkStatus(char *payload)
{

	char *str = strtok(payload, (const char *)",");
	str = strtok((char *)NULL, (const char *)"\r");

	// ////printf("check %d", atoi(str));

	if ((atoi(str) == 1) || (atoi(str) == 5))
	{
		return 1;
	}
	else
	{
		RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

		gpio_set_level(GPIO_OUTPUT_ACT, 1);
		return 0;
	}

	return 0;
}

void send_Type_Call_queue(uint8_t state)
{
	xQueueSend(Type_Call_queue, (void *)&state, pdMS_TO_TICKS(500));
}

uint8_t send_UDP_Send(char *data, char *topic)
{
	char UDP_OK_Data[10];
	uint8_t count_Send_UDP = 0;
	////printf("\n\n udp send 00\n\n");
	int64_t time1 = esp_timer_get_time();
	label_network_portalRegister = get_INT8_Data_From_Storage(NVS_NETWORK_PORTAL_REGISTER, nvs_System_handle);

	if (label_network_portalRegister == 255)
	{
		label_network_portalRegister = 1;
		save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER, label_network_portalRegister, nvs_System_handle);
	}

	printf("\n\n label_network_portalRegister 000 %d - %s\n\n", label_network_portalRegister, data);

	if (gpio_get_level(GPIO_INPUT_IO_SIMPRE))
	{
printf("\n\n label_network_portalRegister 111 %d - %s\n\n", label_network_portalRegister, data);
		if (label_network_portalRegister == 1)
		{

			if (!gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
			{
				printf("\n\n label_network_portalRegister 222 %d - %s\n\n", label_network_portalRegister, data);
				gpio_set_level(GPIO_OUTPUT_ACT, 1);
				uint8_t pwrFD = 0;
				uint8_t InitNetworkCount = 0;
				uint8_t pwrFD_count = 0;
				// system_stack_high_water_mark("SEND UDP1");
				while (pwrFD != 1)
				{
					if (pwrFD_count == 3 || pwrFD == 1)
					{
						break;
					}
					pwrFD_count++;

					pwrFD = EG91_PowerOn();
				}
				// system_stack_high_water_mark("SEND UDP2");
				if (pwrFD == 1)
				{

					while (InitNetworkCount < 3)
					{
						// ////printf("INIT EG915 311");
						if (!EG91_initNetwork())
						{
							RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

							gpio_set_level(GPIO_OUTPUT_ACT, 1);
							update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
							InitNetworkCount++;
						}
						else
						{
							// ////printf("\n\n activate network 1\n\n");
							break;
						}
					}

					if (InitNetworkCount == 3)
					{
						nvs_erase_key(nvs_System_handle, NVS_NETWORK_LOCAL_CHANGED);
						return 1;
					}
				}
			}

			// TODO:DESCOMENTAR CASO NAO FUNCIONE
			// xSemaphoreGive(rdySem_Control_Send_AT_Command);

			// TODO: RETIRAR CASO FUNCIONE MAL
			if (strlen(topic) < 2)
			{
				/* code */

				xSemaphoreGive(rdySem_Control_Send_AT_Command);

				while (count_Send_UDP < 3)
				{
					memset(UDP_OK_Data, 0, sizeof(UDP_OK_Data));
					// ////printf("\nsms ghghkk444433 %s\n",data);
					// ////printf("\nsms ghghkk555\n");
					////printf("\n\n task_EG91_Send_UDP 12345 %s\n\n", data);
					if (send_UDP_Package(data, strlen(data), ""))
					{
						return 1;
					}

					count_Send_UDP++;
				}
			}
			else	
			{
				xSemaphoreGive(rdySem_Control_Send_AT_Command);

				while (count_Send_UDP < 3)
				{
					memset(UDP_OK_Data, 0, sizeof(UDP_OK_Data));
					////printf("\nsms ghghkk444433 %s\n", data);
					// ////printf("\nsms ghghkk555\n");
					// ////printf("\n\n task_EG91_Send_UDP 12345 %s\n\n", data);
					if (send_UDP_Package(data, strlen(data), topic))
					{
						return 1;
					}

					count_Send_UDP++;
				}
			}

			count_Send_UDP = 0;
			checkIf_mqttSend_fail(data);
			// ////printf("\n\nlost pack %d\n\n", label_UDP_fail_and_changed);

			/* if (label_Reset_Password_OR_System == 2)
			{
				vTaskResume(handle_SEND_SMS_TASK);
				// vTaskResume(handle_SMS_TASK);
				timer_start(TIMER_GROUP_1, TIMER_0);
			} */
		}
	}
	else
	{
		if (label_network_portalRegister == 1)
		{
			checkIf_mqttSend_fail(data);
		}
printf("\n\nlost pack %d\n\n", 5555);
		return 0;
	}
	return 0;
	// vTaskResume(xHandle_Timer_VerSystem);
	//  xSemaphoreGive(rdySem_Control_Send_AT_Command);
	//   vTaskDelay(pdMS_TO_TICKS(3000));
}

void checkIf_mqttSend_fail(char *data)
{
	uint8_t label_UDP_fail_and_changed = 0;
	char auxData[200] = {};
	sprintf(auxData, "%s", data);
	char delim[] = " ";
	// ////printf("\n\ncount_Send_UDP++ 11;\n\n");
	char *command = strtok(auxData, delim);
	// ////printf("\n\ncount_Send_UDP++ 22;\n\n");
	char *parameter = strtok(NULL, delim);
	// ////printf("\n\ncount_Send_UDP++ 33;\n\n");
	char *element = strtok(NULL, delim);
	// ////printf("\n\ncount_Send_UDP++ 44;\n\n");
	//  char *element = strtok(NULL, delim);

	printf("\n\ncount_Send_UDP++ 55;\n\n");
	//  ////printf("\n\n cmd - %s, parameter - %s, elemente - %s\n\n",command,parameter,element);
	// ////printf("\n\ncount_Send_UDP++ 66;\n\n");
	label_UDP_fail_and_changed = get_INT8_Data_From_Storage(NVS_NETWORK_LOCAL_CHANGED, nvs_System_handle);
	// ////printf("\n\ncount_Send_UDP++ 77;\n\n");
	// ////printf("\n\ncount_Send_UDP++ 66;\n\n");

	if (label_UDP_fail_and_changed == 255)
	{
		label_UDP_fail_and_changed = 0;
		label_UDP_fail_and_changed |= 3;
		save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED, label_UDP_fail_and_changed, nvs_System_handle);
	}

	if (!strcmp(command, "RT"))
	{
		label_UDP_fail_and_changed |= 1;
	}

	if (!strcmp(command, "UR") || (!strcmp(command, "ME") && (parameter[0] == 'S' || parameter[0] == 'R') && (element[0] == 'A' || element[0] == 'U')))
	{
		label_UDP_fail_and_changed |= 2;
		// ////printf("\n\nlost pack UR %d\n\n", label_UDP_fail_and_changed);
	}

	if (!strcmp(command, "ME") && parameter[0] == 'R' && element[0] == 'K')
	{
		label_UDP_fail_and_changed |= 4;
	}

	/* if ((!strcmp(command, "ME") && (parameter[0] == 'S' || parameter[0] == 'R') && element[0] == 'F'))
	{
		label_UDP_fail_and_changed |= 4;
	} */

	save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED, label_UDP_fail_and_changed, nvs_System_handle);
}
void task_EG91_Send_UDP(void *pvParameter)
{

	mqtt_information mqttInfo;

	for (;;) // ajudado por deus deus@paraiso.org!!
	{

		xQueueReceive(UDP_Send_queue, &mqttInfo, portMAX_DELAY);

		// //printf("\nsms mqtt00\n");
		/* if (label_ResetSystem == 1)
		{
			timer_pause(TIMER_GROUP_1, TIMER_0);
			//////printf("\nsms mqtt\n");
			disableAlarm();
			vTaskSuspend(xHandle_Timer_VerSystem);
			//////printf("\nsms mqtt1\n");
			vTaskSuspend(handle_SEND_SMS_TASK);
			//////printf("\nsms mqtt2\n");
			xSemaphoreGive(rdySem_Control_Send_AT_Command);
		} */

		// save_INT8_Data_In_Storage(NVS_QMT_LARGE_DATA_TIMER_LABEL, 1, nvs_System_handle);
		// xSemaphoreTake(rdySem_Control_SMS_UDP, pdMS_TO_TICKS(3000));
		////printf("\n\n receive UDP queue %s - %s\n\n", mqttInfo.data, mqttInfo.topic);
		send_UDP_Send(mqttInfo.data, mqttInfo.topic);

		/* if (label_ResetSystem == 1)
		{
			vTaskResume(handle_SEND_SMS_TASK);
			vTaskResume(xHandle_Timer_VerSystem);
			timer_start(TIMER_GROUP_1, TIMER_0);
			enableAlarm();
		} */
		// save_INT8_Data_In_Storage(NVS_QMT_LARGE_DATA_TIMER_LABEL, 0, nvs_System_handle);
		//  xSemaphoreGive(rdySem_Control_SMS_UDP);
	}
}

void send_UDP_queue(mqtt_information *mqttInfo)
{
	////printf("\n\ntask_refresh_SystemTime 000111 - %s - %s\n\n", mqttInfo->data, mqttInfo->topic);

	xQueueSendToBack(UDP_Send_queue, mqttInfo, pdMS_TO_TICKS(3000));
}

esp_ota_handle_t updateHandle = 0;
const esp_partition_t *updatePartition = NULL;

void EG915_fota(mqtt_information *mqttInfo)
{
	char atCommand[200] = {};
	uint32_t CRC32_FOTA = 0;

	timer_pause(TIMER_GROUP_1, TIMER_0);
	// // ////printf("\nsms\n");
	// disableAlarm();
	// vTaskSuspend(xHandle_Timer_VerSystem);
	// vTaskSuspend(handle_SEND_SMS_TASK);

	EG915_readDataFile_struct.mode = EG91_FILE_FOTA_MODE;
	disableBLE();
	

	EG91_send_AT_Command("AT+QFDEL=\"sound_8.wav\"", "OK", 1000);
	EG91_send_AT_Command("AT+QFDEL=\"sound_7.wav\"", "OK", 1000);
	EG91_send_AT_Command("AT+QFDEL=\"sound_6.wav\"", "OK", 1000);
	EG91_send_AT_Command("AT+QFDEL=\"sound_5.wav\"", "OK", 1000);
	EG91_send_AT_Command("AT+QFDEL=\"sound_4.wav\"", "OK", 1000);
	EG91_send_AT_Command("AT+QFDEL=\"sound_3.wav\"", "OK", 1000);
	EG91_send_AT_Command("AT+QFDEL=\"sound_2.wav\"", "OK", 1000);


	EG91_send_AT_Command("AT+QINDCFG=\"ring\", 0", "OK", 1000);

	EG91_send_AT_Command("AT+CNMI=0,0,0,0,0", "OK", 1000);

	EG91_send_AT_Command("AT+QIACT?", "OK", 1000);
	EG91_send_AT_Command("AT+QHTTPCFG=\"contextid\",1", "OK", 1000);
	EG91_send_AT_Command("AT+QHTTPCFG=\"responseheader\",1 ", "OK", 1000);
	// snprintf(atCommand, sizeof(atCommand), "AT+QHTTPURL=%d,%d\r\n%s%c", strlen("https://www.alipay.com"), 60, "https://www.alipay.com",13);

	EG91_send_AT_Command("AT+QHTTPCFG=\"sslctxid\",1", "OK", 1000);
	EG91_send_AT_Command("AT+QSSLCFG=\"sslversion\",1,3", "OK", 1000);
	EG91_send_AT_Command("AT+QSSLCFG=\"ciphersuite\",1,0XC02F", "OK", 1000);
	EG91_send_AT_Command("AT+QSSLCFG=\"seclevel\",1,0 ", "OK", 1000);
	EG91_send_AT_Command("AT+QSSLCFG=\"ignoreinvalidcertsign\",1,1 ", "OK", 1000);
	EG91_send_AT_Command("AT+QSSLCFG=\"sni\",1,1", "OK", 1000);
	EG91_send_AT_Command("AT+QHTTPCFG=\"responseheader\",0", "OK", 1000);

	// EG91_send_AT_Command("AT+QSSLCFG=\"renegotiation\",1,1","OK",1000);
	// EG91_send_AT_Command("AT+QSSLOPEN=1,1,4,\"https://api.mconnect.motorline.pt/health\",443,0","OK",1000);
	// vTaskDelay(pdMS_TO_TICKS(5000));
	sprintf(atCommand, "%s%d%s%c", "AT+QHTTPURL=", strlen("https://static.portal.motorline.pt/firmwares/m200.bin"), ",80", 13);
	EG91_send_AT_Command(atCommand, "CONNECT", 60000);
	// uart_write_bytes(UART_NUM_1, atCommand, strlen(atCommand));
	vTaskDelay(pdMS_TO_TICKS(520));
	// EG91_send_AT_Command(atCommand,"OK",60000);
	EG91_send_AT_Command("https://static.portal.motorline.pt/firmwares/m200.bin", "OK", 60000);

	// EG91_send_AT_Command("AT","OK",10000);

	EG91_send_AT_Command("AT+QHTTPGET=80", "QHTTPGET", 10000);
	// vTaskDelay(pdMS_TO_TICKS(10000));
	// EG91_send_AT_Command("AT+QHTTPREAD=80","OK",10000);

	char AT_Command[100];
	// EG91_send_AT_Command("AT+QHTTPREAD=80", "OK", 10000);
	EG91_send_AT_Command("AT+QHTTPREADFILE=\"UFS:3.txt\",80", "OK", 10000);
	EG91_send_AT_Command("ATE1", "OK", 1000);
	uint8_t ACK = 0;
	ACK = EG91_send_AT_Command("AT+QFOPEN=\"UFS:3.txt\"", "+QFOPEN:", 20000);
	// vTaskDelay(pdMS_TO_TICKS(5000));788206364

	esp_err_t err;
	int idFile = atoi(fileID);
	int nowFileSize = 0;
	//printf("\nfile id %d - %s", idFile, fileID);
	sprintf((char *)iv, "%s", "cqfDXcNe167GMAT2");
	if (ACK)
	{

		updatePartition = esp_ota_get_next_update_partition(NULL);
		// ESP_LOGI(GATTS_TABLE_TAG, "Writing to partition subtype %d at offset 0x%x",                        update_partition->subtype, update_partition->address);
		assert(updatePartition != NULL);
		err = esp_ota_begin(updatePartition, OTA_WITH_SEQUENTIAL_WRITES, &updateHandle);

		sprintf(AT_Command, "AT+QFREAD=%d,1024", idFile);
		int count = 0;
		while (nowFileSize < EG915_readDataFile_struct.fileSize)
		{
			EG91_send_AT_Command(/* "AT+QFREAD=1027,1024" */ AT_Command, "+QFREAD", 3000);

			nowFileSize += EG915_readDataFile_struct.nowFileSize;
			err = esp_ota_write(updateHandle, (const void *)EG915_readDataFile_struct.receiveData, EG915_readDataFile_struct.decryptSize);

			if (err != ESP_OK)
			{
				err = ESP_FAIL;
				// 	esp_ota_abort(updateHandle);
				// 	EG915_readDataFile_struct.mode = 0;
				// 	sprintf(AT_Command, "AT+QFCLOSE=%d", idFile);
				// EG91_send_AT_Command(/* "AT+QFREAD=1027,1024" */ AT_Command, "OK", 3000);

				ESP_LOGI("GATTS_TABLE_TAG", "esp_ota_write error!");
				break;
			}

			//printf("\n\nnowFileSize 11 - %d - %d \n\n", nowFileSize, EG915_readDataFile_struct.fileSize);

			CRC32_FOTA = esp_rom_crc32_le(CRC32_FOTA, (uint8_t *)EG915_readDataFile_struct.receiveData, EG915_readDataFile_struct.packetFile_size);
			//printf("CRC32: 0x%08X - 0x%08X\n", CRC32_FOTA, EG915_readDataFile_struct.ckm);

			count++;
		}

		if (err != ESP_OK || CRC32_FOTA != EG915_readDataFile_struct.ckm)
		{
			// esp_ota_end()
			esp_ota_abort(updateHandle);
			send_UDP_Send("«\0", "");
			sprintf(AT_Command, "AT+QFCLOSE=%d", idFile);
			EG91_send_AT_Command(/* "AT+QFREAD=1027,1024" */ AT_Command, "OK", 3000);

			memset(&EG915_readDataFile_struct, 0, sizeof(EG915_readDataFile_struct));
			if (err == ESP_ERR_OTA_VALIDATE_FAILED)
			{
				// ESP_LOGE(GATTS_TABLE_TAG, "Image validation failed, image is corrupted");
			}
		}
		else
		{
			err = esp_ota_end(updateHandle);
			err = esp_ota_set_boot_partition(updatePartition);

			sprintf(AT_Command, "AT+QFCLOSE=%d", idFile);
			EG91_send_AT_Command(AT_Command, "OK", 1000);
			EG91_send_AT_Command("AT+QFDEL=\"UFS:3.txt\"", "OK", 1000);

			vTaskDelay(pdMS_TO_TICKS(1500));
			esp_restart();
		}
	}
	else
	{
		//esp_ota_abort(updateHandle);
		send_UDP_Send("«\0", "");
	}

	restartBLE();
	EG91_send_AT_Command("AT+QFDEL=\"UFS:3.txt\"", "OK", 1000);
	////printf("CRC32: 0x%08X - 0x%08X\n", CRC32_FOTA, EG915_readDataFile_struct.ckm);
	EG91_send_AT_Command("AT+QINDCFG=\"ring\", 1", "OK", 1000);
	EG915_readDataFile_struct.ckm = 0;
	EG91_send_AT_Command("AT+CNMI=2,1,0,0,0", "OK", 1000);

	// vTaskResume(xHandle_Timer_VerSystem);
	// vTaskResume(handle_SEND_SMS_TASK);
	timer_start(TIMER_GROUP_1, TIMER_0);
	// enableAlarm();
	EG915_readDataFile_struct.mode = EG91_FILE_NORMAL_MODE;
}

void init_EG91(void)
{
	// xTaskCreate(uart_event_task, "uart_event_task", 8000, NULL, 4, NULL);
	// ////printf("INIT EG915 1");
	uartInit();
	uint8_t pwrFD = 0;

	mqtt_openLabel = 0;
	mqtt_connectLabel = 0;
	// ////printf("INIT EG915 2");

	rdySem_Control_Send_AT_Command = xSemaphoreCreateBinary();
	rdySem_Control_IncomingCALL = xSemaphoreCreateMutex();
	rdySem_No_Carrie_Call = xSemaphoreCreateBinary();
	rdySem_QPSND = xSemaphoreCreateBinary();
	rdySem_UART_CTR = xSemaphoreCreateBinary();
	rdySem_Lost_SMS = xSemaphoreCreateBinary();
	rdySem_Control_SMS_Task = xSemaphoreCreateBinary();
	rdySem_Control_SMS_UDP = xSemaphoreCreateBinary();
	rdySem_Control_pubx = xSemaphoreCreateBinary();

	xSemaphoreGive(rdySem_Control_Send_AT_Command);
	xSemaphoreTake(rdySem_No_Carrie_Call, 0);
	xSemaphoreTake(rdySem_QPSND, 0);
	xSemaphoreTake(rdySem_Lost_SMS, 0);

	// xSemaphoreTake(rdySem_Control_IncomingCALL, (portTickType) portMAX_DELAY);

	// ESP_LOGI("TAG", "xPortGetFreeHeapSize EG91 QUEUE: %d", xPortGetFreeHeapSize());
	// ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
	// ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
	// ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));

	AT_Command_Feedback_queue = xQueueCreate(AT_QUEUE_SIZE, sizeof(char) * BUF_SIZE);
	EG91_CALL_SMS_UART_queue = xQueueCreate(3, 180);
	// EG91_CALL_CALL_UART_queue = xQueueCreate(1, sizeof(data_ReceiveAT_Serial));
	EG91_CALL_CLCC_UART_queue = xQueueCreate(3, sizeof(char) * 50);
	EG91_CALL_CHUP_UART_queue = xQueueCreate(1, 50);
	// Type_Call_queue = xQueueCreate(2, sizeof(call_Type));
	receive_mqtt_queue = xQueueCreate(2, sizeof(char) * 250);
	NO_CARRIER_Call_queue = xQueueCreate(1, 50);
	UDP_Send_queue = xQueueCreate(2, sizeof(mqtt_information));
	HTTPS_data_queue = xQueueCreate(2, sizeof(char) * 30);

	// Receive_UDP_OK_queue = xQueueCreate(5, sizeof(char) * 10);
	// Lost_SMS_queue = xQueueCreate(1, sizeof(data_ReceiveAT_Serial));
	// EG91_GET_SIM_BALANCE_queue = xQueueCreate(1, 200);

	// EG91_WRITE_FILE_queue = xQueueCreate(5, 100);

	xTaskCreate(task_EG91_Run_SMS, "task_EG91_Run_SMS", /* 22 */ 21 * 1024, NULL, 29, &handle_SMS_TASK);
	// ////printf("\n pwrfg = finish create run sms\n");

	xTaskCreate(task_EG91_SendSMS, "task_EG91_SendSMS", 6 * 2048 + 1024, NULL, 31, &handle_SEND_SMS_TASK);
	// ////printf("\n pwrfg = finish create run send sms\n");

	// xTaskCreate(task_EG91_Feedback_Call, "task_EG91_Feedback_Call", 6000, NULL, 26, NULL);

	xTaskCreate(task_EG91_Receive_UDP, "task_EG91_Receive_UDP", 15000 + 2046 + 1024, NULL, 20, NULL);

	xTaskCreate(task_EG91_Send_UDP, "task_EG91_Send_UDP", 9 * 1024, NULL, 26, &handle_UDP_TASK);

	// xTaskCreate(task_EG91_Verify_Unread_SMS, "task_EG91_Verify_Unread_SMS", 7000, NULL, 20, NULL);

	// ////printf("\n\n\n before power on\n\n\n");

	//  initTimers();

	if (gpio_get_level(GPIO_INPUT_IO_SIMPRE))
	{

		uint8_t pwrFD_count = 0;
		pwrFD = 0;
		uint8_t InitNetworkCount = 0;
		printf("INIT EG915 3");

		 while (pwrFD != 1)
		{
			if (pwrFD_count == 3 || pwrFD == 1)
			{
				break;
			}
			pwrFD_count++; 

			pwrFD = EG91_PowerOn();
		}
		// esp_timer_stop(periodic_timer);
		send_ATCommand_Label = 1;

		// feedback_Audio_timeout = 0;

		// Type_Call_queue = xQueueCreate(1, sizeof(uint8_t));

		// ESP_LOGI("TAG", "xPortGetFreeHeapSize EG91 AFTER POWER ON: %d", xPortGetFreeHeapSize());
		// ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
		// ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
		// ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));

		xSemaphoreGive(rdySem_Control_SMS_Task);
		
		//   EG91_writeFile(f1, t1);
		//   EG91_send_AT_Command("AT+QFLST=\"*\"", "OK", 1000);
		//   gpio_set_level(GPIO_OUTPUT_ACT, 1);
		// ////printf("\n pwrfg = %d\n", pwrFD);

		if (pwrFD == 1)
		{
			while (InitNetworkCount < 3)
			{
				// ////printf("INIT EG915 311");
				if (!EG91_initNetwork())
				{
					RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

					gpio_set_level(GPIO_OUTPUT_ACT, 1);
					update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
					InitNetworkCount++;
				}
				else
				{
					// ////printf("\n\n init OK label \n\n");
					break;
				}
			}
		}
		else
		{
			InitNetworkCount = 3;
		}

		if (InitNetworkCount == 3)
		{
			update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
		}
	}
	else
	{
		// ////printf("\n\n init NOT OK label \n\n");
	}

	// EG915_fota();
}

/* uint8_t som[] asm("_binary_som_beep_wav_start");
uint8_t som_end[] asm("_binary_som_beep_wav_end"); */

// uint8_t som[] asm("_binary_sound_8_wav_start");
// uint8_t som_end[] asm("_binary_sound_8_wav_end");

// /* uint8_t som_final[] asm("_binary_som_beep_final_wav_start");
// uint8_t som_final_end[] asm("_binary_som_beep_final_wav_end"); */

// uint8_t som_final[] asm("_binary_alertMotorline_wav_start");
// uint8_t som_final_end[] asm("_binary_alertMotorline_wav_end");

void EG91_writeFile(char *fileName, char *file, int filesize)
{

	//     char atCommand[100];
	//     uint8_t counterACK = 5;
	//     char dtmp1[BUFF_SIZE];
	//     unsigned int som_len = sizeof(uint8_t) * (som_end - som);
	//     unsigned int somFinal_len = sizeof(uint8_t) * (som_final_end - som_final);
	//     /* FILE *exemple;
	//     fopen() */
	//     ////printf("\nWRITE atCommand1 11\n");
	//     // EG91_send_AT_Command("AT+QFLST=\"*\"", "OK", 1000);
	//     ////printf("\nWRITE atCommand1 22\n");
	//     sprintf(atCommand, " AT+QFOPEN=\"%s\"", "sound_8.wav" /* fileName */);

	//     EG91_send_AT_Command(atCommand, "QFOPEN", 1000);
	//     ////printf("\nWRITE atCommand1 33\n");
	//     /* sprintf(atCommand, "AT+QFREAD=%d,%d", atoi(fileID), 30);
	//     ////printf("\nWRITE atCommand1 %s - atoi %d - %d\n", atCommand, atoi(fileID), strlen(atCommand));
	//     EG91_send_AT_Command(atCommand, "OK", 3000); */

	//     memset(atCommand, 0, sizeof(atCommand));
	//     sprintf(atCommand, "AT+QFWRITE=%d,%d,80", atoi(fileID), som_len /* filesize */);
	//     ////printf("\nWRITE atCommand1 %s - atoi %d - %d\n", atCommand, atoi(fileID), strlen(atCommand));
	//     EG91_send_AT_Command(atCommand, "CONNECT", 5000);
	//     // vTaskDelay(pdMS_TO_TICKS(250));

	//     ////printf("\nWRITE FILESIZE %d\n", filesize);

	//     uart_write_bytes(UART_NUM_1, som /* file */, som_len /* filesize */);
	//     ////printf("\nWRITE FILE 1 %s - %d\n", som_final, somFinal_len);

	//     while (counterACK > 0)
	//     {
	//         memset(dtmp1, 0, sizeof(dtmp1));
	//         ////printf("\nWRITE FILE 2\n");
	//         xQueueReceive(EG91_WRITE_FILE_queue, &dtmp1, pdMS_TO_TICKS(5000 * 2));
	//         ////printf("\nWRITE FILE 3\n");
	//         ////printf("\nAT_Command_Feedback_queue = %s\n", dtmp1);
	//         ////printf("\nWRITE FILE 4\n");
	//         if (strstr(dtmp1, "OK") != NULL)
	//         {
	//             /* char *fileName;
	//             FILE *f = NULL;
	//             asprintf(&fileName, "/sdcard/%s", fileName);
	//             ////printf("\nfileName sdcard %s\n", fileName);
	//             f = fopen(fileName, "a");

	//             for (int i = 0; i < filesize; i++)
	//             {
	//                 fprintf(f, "%c", file[i]);
	//                 // fwrite(file,1,sizeof(file),f);
	//             }

	//             // fprintf(f, "%d", file);

	//             fclose(f);
	//  */
	//             memset(atCommand, 0, sizeof(atCommand));
	//             // sprintf(atCommand, "AT+QFCLOSE=%d", atoi(fileID));
	//             // EG91_send_AT_Command(atCommand, "OK", 1000);
	//             ////printf("\nWRITE FILE 5\n"); // file_CRC32
	//             // esp_rom_crc32_le(crc32_val, data1, strlen(file));
	//             break;
	//         }
	//         else
	//         {
	//             ////printf("\nWRITE FILE 6\n");
	//             counterACK--;
	//         }

	//         vTaskDelay(pdMS_TO_TICKS((1000)));
	//         break;
	//     }

	//     // EG91_send_AT_Command("AT+QFLST=\"*\"", "OK", 1000);
	//     ////printf("\nWRITE FILE 7\n");
	//     ////printf("\nWRITE FILE 8\n");
	//     send_ATCommand_Label = 0;
	// return 0;
}

uint8_t parse_OpenFile(char *payload)
{
	regex_t regex;
	const char s[2] = ":";
	const char sp[2] = " ";
	char *token;
	int strIndex = 0;
	int auxCount = 0;
	char dtmp1[100];
	int reti;

	// xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(time));

	reti = regcomp(&regex, "^[0-9]+$", REG_EXTENDED);
	memset(fileID, 0, sizeof(fileID));

	// char response[] = "+QFOPEN: 1027";

	char codigo[10]; // Ajuste o tamanho conforme necessário

	// Encontrar a posição do primeiro espaço após ":"
	token = strstr(payload, ": ") + 2;

	// Copiar o código para a variável 'codigo'
	sscanf(token, "%s", fileID);

	// Imprimir o código
	////printf("Código do arquivo: %s\n", fileID);

	/* for (int i = 0; i < strlen(payload); i++)
	{
		if (payload[i] == ':')
		{
			auxCount++;
			strIndex = 0;
		}
		else
		{
			if (auxCount == 1)
			{
				if (payload[i] >= '0' && payload[i] <= '9')
				{
					fileID[strIndex++] = payload[i];

					if (strIndex > 5)
					{
						return 0;
					}
				}
			}
		}
	} */

	////printf("\ntoken3 =%s\n", fileID);

	if (strlen(fileID) == 0 || (regexec(&regex, fileID, 0, NULL, 0)))
	{
		return 0;
	}

	/*

		token = strtok(payload, s);
		//////printf("\ntxt1 = %s\n", payload);
		token = strtok(NULL, s);
		////printf("\ntoken1 =%s\n", token);
		token = strtok(token, ":");
		////printf("\ntoken2 =%s\n", token);
		token = strtok(NULL, sp);
		////printf("\ntoken3 =%s\n", token);

		sprintf(fileID, "%s", token);
		////printf("\nfileID =%s\n", fileID); */

	return 1;
}

uint8_t verify_QFWRITE(char *payload)
{
	if ((strstr(payload, "CONNECT")) != NULL)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t EG91_parse_CPAS(char *receiveData)
{
	const char s1[2] = ": ";
	const char s2[2] = " ";

	char *token;
	token = strtok(receiveData, s1);

	/* walk through other tokens */

	token = strtok(NULL, s2);
	// ////printf("token cpas=%s\n", token);

	if (token[0] == '0')
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t parse_IMEI(char *payload)
{
	uint8_t enterCount = 0;
	uint8_t strIndex = 0;
	memset(EG91_IMEI, 0, 20);

	for (size_t i = 0; i < strlen(payload); i++)
	{
		if (payload[i] == '\n')
		{
			enterCount++;
			strIndex = 0;
		}
		else
		{
			if (enterCount == 1)
			{
				EG91_IMEI[strIndex++] = payload[i];
			}

			if (enterCount > 1)
			{
				break;
			}
		}
	}

	EG91_IMEI[strlen(EG91_IMEI) - 1] = 0;

	for (size_t i = 0; i < strlen(EG91_IMEI); i++)
	{
		if (EG91_IMEI[i] < '0' || EG91_IMEI[i] > '9')
		{
			return 0;
		}
	}

	// ////printf("\n\n\n parse imei %s\n\n\n", EG91_IMEI);
	return 1;
}

uint8_t EG91_parse_CPIN(char *payload)
{

	// ////printf("\n\n parse CPIN \n\n");

	if (strstr(payload, "READY") != NULL)
	{
		SIM_CARD_PIN_status = 1;

		return 1;
	}
	else
	{
		SIM_CARD_PIN_status = 0;

		RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

		gpio_set_level(GPIO_OUTPUT_ACT, 1);

		return 0;
	}

	return 0;
}

uint8_t parse_QMTOPEN(char *receiveData)
{
	uint8_t auxcounter = 0;
	char qmtopenResult = 0;

	// ////printf("\n\n QMTOPEN data %s\n\n", receiveData);

	for (size_t i = 0; i < strlen(receiveData); i++)
	{
		if (receiveData[i] == ',')
		{
			qmtopenResult = receiveData[i + 1];
			break;
		}
	}

	// ////printf("\n\n QMTOPEN data %c\n\n", qmtopenResult);

	if (qmtopenResult == '0')
	{
		return 1;
	}
	else
	{
		return 0;
	}

	return 0;
}

uint8_t parse_at_qmtopen_(char *receiveData)
{
	uint8_t counter = 0;

	for (size_t i = 0; i < strlen(receiveData); i++)
	{
		if (receiveData[i] == ',')
		{
			counter++;
		}
	}

	if (counter == 0)
	{
		mqtt_openLabel = 0;
	}
	else
	{
		mqtt_openLabel = 1;
	}

	// ////printf("\n\n mqtt_openLabel - %d - %s\n\n", mqtt_openLabel, receiveData);

	return 1;
}

uint8_t EG91_parse_QFREAD(char *payload)
{
	if (payload == NULL)
	{
		////printf("Payload inválido.\n");
		return 0; // Se o argumento for inválido, retorna falso
	}

	/* code */

	EG915_readDataFile_struct.readOK = 0; // Por padrão, readOK é definido como 0 (falso)

	char *connectPtr = strstr(payload, "CONNECT");
	if (connectPtr != NULL)
	{
		sscanf(connectPtr, "CONNECT %d", &EG915_readDataFile_struct.nowFileSize); // Lê o nowFileSize após "CONNECT"
		// //printf("nowFileSize: %d\n", EG915_readDataFile_struct.nowFileSize);
		char *dataStart = strchr(connectPtr, '\n'); // Início da sequência de dados
		if (dataStart != NULL)
		{
			char *dataEnd = strstr(dataStart, "\r\nOK"); // Fim da sequência de dados
			if (dataEnd != NULL)
			{
				size_t dataSize = dataEnd - dataStart - 1; // Calcula o tamanho da sequência de dados
														   // //printf("Data size: %zu\n", dataSize);
				// if (dataSize < sizeof(EG915_readDataFile_struct.receiveData)) {
				unsigned char decrypted[2000] = {};

				strncpy(EG915_readDataFile_struct.receiveData, dataStart + 1, dataSize); // Copia a sequência de dados
				EG915_readDataFile_struct.receiveData[dataSize] = '\0';
				/* //printf("\n\n aaa 2222\n\n");
				free(connectPtr);				 // Adiciona o terminador de string
				////printf("\n\n aaa 3333\n\n"); */
				// //printf("\nData received1111: %s\n", EG915_readDataFile_struct.receiveData);
				if (EG915_readDataFile_struct.mode == EG91_FILE_FOTA_MODE)
				{
					decrypt_aes_cfb_padding((unsigned char *)EG915_readDataFile_struct.receiveData, strlen((char *)EG915_readDataFile_struct.receiveData), &decrypted);
					return 1;
				}
				else if (EG915_readDataFile_struct.mode == EG91_FILE_USERS_MODE)
				{

					return 1;
				}
				else
				{
					return 0;
				}

				EG915_readDataFile_struct.readOK = 1; // Define readOK como verdadeiro
			}
			else
			{
				////printf("OK not found.\n");
				return 0; // Retorna falso se "\nOK" não for encontrado
			}
		}
		else
		{
			////printf("Data start not found.\n");
			return 0; // Retorna falso se o início dos dados não for encontrado
		}
	}
	else
	{
		////printf("CONNECT not found.\n");
		return 0; // Retorna falso se "CONNECT" não for encontrado
	}
	// //printf("\n\nnowlifesize %d\n\n", EG915_readDataFile_struct.nowFileSize);
	return EG915_readDataFile_struct.readOK; // Retorna 1 se OK foi encontrado, senão retorna 0

	return 0;
}

int8_t EG91_Parse_ReceiveData(char *receiveData, char *rsp)
{

	char dtmp[100];

	if ((strstr(receiveData, rsp)) != NULL)
	{
		// ////printf("\n(strstr(receiveData, rsp)) != NULL\n");
		// system_stack_high_water_mark("receiveData1");
		if (!strcmp(rsp, "OK"))
		{
			if ((strstr(receiveData, "OK")) != NULL)
			{
				if (strstr(receiveData, "+CNUM"))
				{
					EG91_parse_CNUM(receiveData);
				}

				return 1;
			}
			else
			{
				// vTaskDelay(pdMS_TO_TICKS((100)));
				return 0;
			}
		}
		else if (!strcmp(rsp, "AT+CREG") || !strcmp(rsp, "AT+CGREG"))
		{
			return parse_NetworkStatus(receiveData);
		}
		else if (!strcmp(rsp, "+CMGR"))
		{
			////printf("\n\nparse_SMS_Payload %s\n\n", receiveData);
			parse_SMS_Payload(receiveData);
			return 1;
		}
		else if (!strcmp(rsp, "AT+QMTCONN?"))
		{
			// ////printf("\nstrcmp(rsp, +CMGR)\n");
			//   xSemaphoreGive(rdySem_Control_Send_AT_Command);

			return parse_qmtconn(receiveData);
		}
		else if (!strcmp(rsp, "+QMTOPEN:"))
		{
			return parse_verify_mqtt_open(receiveData);
		}
		else if (!strcmp(rsp, "AT+QMTOPEN?"))
		{

			return parse_at_qmtopen_(receiveData);
		}
		else if (!strcmp(rsp, "+QMTCONN:"))
		{
			return parse_verify_mqtt_conection(receiveData);
		}
		else if (!strcmp(rsp, "QHTTPGET"))
		{
			return parse_https_get(receiveData);
		}
		else if (!strcmp(rsp, "GSN"))
		{

			parse_IMEI(receiveData);
			return 1;
		}
		else if (!strcmp(rsp, "+QFREAD"))
		{
			return EG91_parse_QFREAD(receiveData);
		}
		else if (!strcmp(rsp, "+QMTSUB:"))
		{
			return parse_qmtsub(receiveData);
		}
		else if (!strcmp(rsp, "CPIN"))
		{
			return EG91_parse_CPIN(receiveData);
		}
		else if (!strcmp(rsp, "CPAS"))
		{
			return EG91_parse_CPAS(receiveData);
		}
		else if (!strcmp(rsp, "CLCC"))
		{
			return parse_IncomingCall_Payload(receiveData);
		}
		else if (!strcmp(rsp, "QLTS"))
		{
			return parse_NowTime(receiveData);
		}
		else if (!strcmp(rsp, "CSQ"))
		{
			// ////printf("\nparse RSSI\n");
			parse_RSSI(receiveData);
			return 1;
		}
		else if (!strcmp(rsp, "CMGL"))
		{
			parse_SMS_List(receiveData);

			return 1;
		}
		else if (!strcmp(rsp, "CMGS:"))
		{
			// ////printf("\nparse RSSI\n");
			return parse_Confirmation_Send_SMS(receiveData);
		}
		else if (!strcmp(rsp, "+QFOPEN:"))
		{
			// ////printf("\nparse RSSI\n");
			return parse_OpenFile(receiveData);
		}
		else if (!strcmp(rsp, "CONNECT"))
		{
			// ////printf("\nparse verify_QFWRITE\n");
			return verify_QFWRITE(receiveData);
		}
		/* else if (!strcmp(rsp, "CUSD"))
		{
			// ////printf("\nparse CUSD\n");
			return EG91_parse_CUSD(receiveData);
		} */
		else if (!strcmp(rsp, "+QCCID"))
		{
			// ////printf("\nparse CUSD\n");
			return EG91_parse_ICCID(receiveData);
		}
		else if (!strcmp(rsp, "+CIMI"))
		{
			return EG91_parseGet_imsi(receiveData);
		}
		else if (!strcmp(rsp, "ERROR"))
		{
			if ((strstr(receiveData, "ERROR")) != NULL)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}
	else
	{
		return 0;
	}

	return 0;
}

uint8_t EG91_parse_ICCID(char *receiveData)
{

	char *number_start;
	char *number_end;
	char *number;
	char *prefix = "+QCCID: ";

	// ////printf("\n\niccid iccid\n\n");

	// Procurar a sequência "+QCCID: " na resposta
	number_start = strstr(receiveData, prefix);

	if (number_start != NULL)
	{
		// Avançar o ponteiro para o início do número
		number_start += strlen(prefix);

		// Procurar o delimitador de nova linha para encontrar o final do número
		number_end = strchr(number_start, '\r');

		if (number_end != NULL)
		{
			// Copiar o número para uma nova string
			int length = number_end - number_start;
			number = malloc(length + 1);
			strncpy(number, number_start, length);
			number[length] = '\0';

			// ////printf("Número:%s\n", number);
			memset(EG91_ICCID_Number, 0, sizeof(EG91_ICCID_Number));

			sprintf(EG91_ICCID_Number, "%s", number);
			save_STR_Data_In_Storage(NVS_EG91_ICCID_VALUE, EG91_ICCID_Number, nvs_System_handle);
			// ////printf("EG91_ICCID_Number:%s\n", EG91_ICCID_Number);
			//  Liberar a memória alocada
			free(number);
		}
	}

	return 1;
}

/* uint8_t EG91_parse_CUSD(char *receiveData)
{
	uint8_t auxCount = 0;
	uint8_t strIndex = 0;

	memset(SIM_Balance, 0, sizeof(SIM_Balance));

	for (int i = 0; i < strlen(receiveData); i++)
	{
		if (receiveData[i] == ':')
		{
			for (i = i; i < strlen(receiveData); i++)
			{
				if (receiveData[i] == '"')
				{
					auxCount++;
					strIndex = 0;
				}
				else
				{
					if (auxCount == 1)
					{
						SIM_Balance[strIndex++] = receiveData[i];
					}
					else if (auxCount == 2)
					{
						break;
					}
				}
			}

			if (auxCount == 2)
			{
				break;
			}
		}
	}

	// ////printf("\n\n USDD output = %s \nfiledata - %s\n\n", receiveData, SIM_Balance);
	// ////printf("\n\n receiveData[strlen(receiveData) - 1] = %c\n\n", receiveData[strlen(receiveData) - 1]);
	return 1;
} */

// char *EG91_Send_Parse_CUSD(char *payload, char *output)
// {
//     char receiveData[200] = {};
//     char AT_CUSD_Command[100] = {};
//     char aux_OutputData[200] = {};
//     uint8_t auxCount = 0;
//     int strIndex = 0;

//     // timer_pause(TIMER_GROUP_1, TIMER_0);

//     memset(SIM_Balance, 0, 500);

//     xSemaphoreGive(rdySem_Control_Send_AT_Command);

//     // EG91_send_AT_Command("AT+CHLD=0", "OK", 3000);

//     EG91_send_AT_Command("ATE0", "OK", 3000);

//     if (EG91_send_AT_Command("AT+QCFG=\"nwscanmode\",1", "OK", 10000))
//     {

//         sprintf(AT_CUSD_Command, "AT+CUSD=1,\"%s\",15", payload);

//         xSemaphoreTake(rdySem_Control_Send_AT_Command, pdMS_TO_TICKS(15000));

//         if (EG91_send_AT_Command(AT_CUSD_Command, "CUSD", 25000))
//         {
//             // ////printf("\n\n\n pass after balance\n\n\n");
//             //  xSemaphoreTake(rdySem_Control_Send_AT_Command, pdMS_TO_TICKS(5000));
//             sprintf(AT_CUSD_Command, "%c%c", 1, 26);
//             EG91_send_AT_Command(AT_CUSD_Command, "ERROR", 1000);

//             // EG91_send_AT_Command("ATH", "OK", 1000);
//             // EG91_send_AT_Command("AT+CUSD=2", "OK", 1000);
//             EG91_send_AT_Command("ATE1", "OK", 1000);
//             EG91_send_AT_Command("AT+QCFG=\"nwscanmode\",0", "OK", 8000);
//             vTaskDelay(pdMS_TO_TICKS(1000));
//             // ////printf("\n\nexit get balance\n\n");
//             check_NetworkState();
//             // xSemaphoreGive(rdySem_Control_Send_AT_Command);
//             /* vTaskResume(xHandle_Timer_VerSystem);
//             timer_start(TIMER_GROUP_1, TIMER_0); */

//             // ////printf("\n\n strlen SIM BALANCE 11 - %d\n\n", strlen(SIM_Balance));
//             return SIM_Balance;
//             /*   }
//           } */
//         }
//         else
//         {
//             sprintf(AT_CUSD_Command, "%c%c", 1, 26);
//             EG91_send_AT_Command(AT_CUSD_Command, "ERROR", 1000);
//             EG91_send_AT_Command("AT+CUSD=2", "OK", 1000);
//             EG91_send_AT_Command("AT+QCFG=\"nwscanmode\",0", "OK", 3000);
//             EG91_send_AT_Command("ATE1", "OK", 1000);
//             vTaskDelay(pdMS_TO_TICKS(1000));
//             check_NetworkState();
//             // xSemaphoreGive(rdySem_Control_Send_AT_Command);
//             /*  vTaskResume(xHandle_Timer_VerSystem);
//              timer_start(TIMER_GROUP_1, TIMER_0); */

//             return return_Json_SMS_Data("ERROR_GET");
//         }

//         /* sprintf(AT_CUSD_Command, "%c%c", 1, 26);
//         EG91_send_AT_Command(AT_CUSD_Command, "ERROR", 1000);
//         EG91_send_AT_Command("AT+CUSD=2", "OK", 1000);
//         EG91_send_AT_Command("ATE1", "OK", 1000);
//         EG91_send_AT_Command("AT+QCFG=\"nwscanmode\",0", "OK", 3000); */
//         check_NetworkState();
//         // xSemaphoreGive(rdySem_Control_Send_AT_Command);
//         /* vTaskResume(xHandle_Timer_VerSystem);
//         timer_start(TIMER_GROUP_1, TIMER_0); */

//         return return_Json_SMS_Data("ERROR_GET");
//     }
//     sprintf(AT_CUSD_Command, "%c%c", 1, 26);
//     EG91_send_AT_Command(AT_CUSD_Command, "ERROR", 1000);
//     EG91_send_AT_Command("AT+CUSD=2", "OK", 1000);
//     EG91_send_AT_Command("ATE1", "OK", 1000);
//     EG91_send_AT_Command("AT+QCFG=\"nwscanmode\",0", "OK", 3000);
//     check_NetworkState();
//     // xSemaphoreGive(rdySem_Control_Send_AT_Command);
//     /*   vTaskResume(xHandle_Timer_VerSystem);
//       timer_start(TIMER_GROUP_1, TIMER_0); */

//     return return_Json_SMS_Data("ERROR_GET");
// }

uint8_t parse_RSSI(char *payload)
{

	char *RSSI_ValueSTR = (strtok((char *)payload, (const char *)":") + 2);
	// ////printf("\nRSSI value11 = %s\n", RSSI_ValueSTR);
	RSSI_ValueSTR = strtok(((char *)NULL), (const char *)",");

	// ////printf("\nint rssi %d\n", atoi(RSSI_ValueSTR));
	RSSI_VALUE = atoi(RSSI_ValueSTR);

	if ((RSSI_VALUE >= 20 && RSSI_VALUE <= 31) || (RSSI_VALUE >= 120 && RSSI_VALUE <= 131))
	{
		RSSI_LED_TOOGLE = RSSI_EXCELLENT_LED_TIME;
		// update_ACT_TimerVAlue((double)RSSI_EXCELLENT_LED_TIME);
		updateSystemTimer(SYSTEM_TIMER_NORMAL_STATE);
	}
	else if ((RSSI_VALUE >= 15 && RSSI_VALUE <= 19) || (RSSI_VALUE >= 115 && RSSI_VALUE <= 119))
	{
		RSSI_LED_TOOGLE = RSSI_GOOD_LED_TIME;
		// update_ACT_TimerVAlue((double)RSSI_GOOD_LED_TIME);
		updateSystemTimer(SYSTEM_TIMER_NORMAL_STATE);
	}
	else if ((RSSI_VALUE >= 10 && RSSI_VALUE <= 14) || (RSSI_VALUE >= 110 && RSSI_VALUE <= 114))
	{
		RSSI_LED_TOOGLE = RSSI_WEAK_LED_TIME;
		// update_ACT_TimerVAlue(RSSI_WEAK_LED_TIME);
		updateSystemTimer(SYSTEM_TIMER_ALARM_STATE);
	}
	else if ((RSSI_VALUE >= 0 && RSSI_VALUE <= 9) || (RSSI_VALUE >= 100 && RSSI_VALUE <= 109))
	{
		RSSI_LED_TOOGLE = RSSI_VERY_WEAK_LED_TIME;
		// update_ACT_TimerVAlue((double)RSSI_VERY_WEAK_LED_TIME);
		updateSystemTimer(SYSTEM_TIMER_URGENT_STATE);
	}
	else if (RSSI_VALUE == 99 || RSSI_VALUE == 199)
	{
		RSSI_LED_TOOGLE = RSSI_NOT_DETECT;
		// update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
		updateSystemTimer(SYSTEM_TIMER_URGENT_STATE);
	}
	else
	{
		RSSI_LED_TOOGLE = RSSI_NOT_DETECT;
		// update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
		updateSystemTimer(SYSTEM_TIMER_URGENT_STATE);
	}

	// ////printf("\nRSSI value22 = %s\n", RSSI_ValueSTR);

	return 1;
}

void verify_SMS_List()
{
	char AT_Command[20];
	// xSemaphoreTake(rdySem_Lost_SMS, pdMS_TO_TICKS(30000));
	// xSemaphoreTake(rdySem_Lost_SMS, pdMS_TO_TICKS(2000));CMGL
	EG91_send_AT_Command("AT+CMGL=\"ALL\"", "CMGL", 1500);
	/* sprintf(AT_Command, "%s%c", "AT+CMGL=\"ALL\"", 13);
	uart_write_bytes(UART_NUM_1, AT_Command, strlen (AT_Command));*/
}

uint8_t parse_SMS_List(char *payload)
{

	data_EG91_Receive_SMS receive_SMS_data;
	char smsID[10];
	char *token;
	char auxID[3];
	char phoneNumber[100];
	char auxPayload[200];
	uint8_t strIndex = 0;
	uint8_t plusCounter = 0;
	uint8_t auxCount = 0;
	uint8_t dotCounter = 0;

	size_t required_size;
	char aux_own_Number[30] = {};
	memset(aux_own_Number, 0, sizeof(aux_own_Number));

	if (nvs_get_str(nvs_System_handle, NVS_KEY_OWN_NUMBER, NULL, &required_size) == ESP_OK)
	{

		nvs_get_str(nvs_System_handle, NVS_KEY_OWN_NUMBER, aux_own_Number, &required_size);
	}

	// ////printf("\nENTER PARSE SMS LIST 1\n");
	//   xSemaphoreGive(rdySem_Control_Send_AT_Command);
	for (int i = 0; i < strlen(payload); i++)
	{
		if (payload[i] == '+')
		{
			plusCounter++;
		}
	}

	// ////printf("\nENTER PARSE SMS LIST 2\n");
	if (plusCounter > 1)
	{
		// ////printf("\nENTER PARSE SMS LIST 3\n");
		plusCounter = 0;
		for (int i = 0; i < strlen(payload); i++)
		{
			if (payload[i] == '+')
			{
				plusCounter++;
				auxCount = 0;
				strIndex = 0;
				dotCounter = 0;
				memset(&receive_SMS_data, 0, sizeof(receive_SMS_data));
				memset(smsID, 0, sizeof(smsID));
				int y = 0;
				if (plusCounter > 1)
				{
					dotCounter = 0;
					for (int j = i; payload[j] != '\n'; j++)
					{
						if (payload[j] == ',')
						{
							dotCounter++;
						}
					}
					// ////printf("\nDOT COUNTER PARSE SMS LIST = %d\n", dotCounter);
					if (dotCounter == 7)
					{
						/* code */

						/* for (y = i; y < strlen(payload); y++)
						{

							if (payload[y] == ',' || payload[y] == '\n')
							{
								auxCount++;
							}

							if (auxCount == 9)
							{
								////printf("\nauxCount == 9\n");
								break;
							}
						} */

						/* if (auxCount == 9)
						{
							auxCount = 0;
						}
						else
						{
							i = y - 1;
							break;
						} */

						for (int j = i; j < strlen(payload); j++)
						{

							if (payload[j] == ',' || payload[j] == '\n')
							{
								auxCount++;
								strIndex = 0;
							}
							else
							{

								if (auxCount == 0)
								{
									if (payload[j] != '"')
									{
										smsID[strIndex] = payload[j];
										strIndex++;
									}
								}

								if (auxCount == 1)
								{
									if (payload[j] != '"')
									{
										receive_SMS_data.SMS_Status[strIndex] = payload[j];
										strIndex++;
									}
								}

								if (auxCount == 2)
								{
									if (payload[j] != '"')
									{
										receive_SMS_data.phNumber[strIndex] = payload[j];
										strIndex++;
									}
								}

								if (auxCount == 4)
								{
									if (payload[j] != '"')
									{
										// if (payload[j] != '/')
										//{
										receive_SMS_data.strDate[strIndex] = payload[j];
										strIndex++;
										// }
									}
								}

								if (auxCount == 5)
								{
									/* if (payload[j] == ':')
									{
										dotCounter++;
									}*/

									if (payload[j] != '"')
										/*if (payload[j] != ':')
											if (dotCounter < 2)
											{ */
										receive_SMS_data.strHour[strIndex] = payload[j];
									strIndex++;
									// }
								}

								if (auxCount == 8)
								{
									if (payload[j] != 13)
									{
										receive_SMS_data.receiveData[strIndex] = payload[j];
										strIndex++;
									}
								}
								if (auxCount == 9)
								{
									i = j - 1;

									break;
								}
							}
						}

						char *token;

						token = strtok(smsID, ":");
						token = strtok(NULL, ":");

						// ////printf("token = %s\n", token);
						receive_SMS_data.SMS_ID = 0;
						receive_SMS_data.SMS_ID = atoi(token);

						// ////printf("\nid = %d\n", receive_SMS_data.SMS_ID);

						char dst1[strlen(receive_SMS_data.phNumber)];
						char dst2[5];
						char dst3[strlen(receive_SMS_data.receiveData)];

						for (int i = 0; i < strlen(receive_SMS_data.phNumber); i = i + 4)
						{
							char aux_UTF8[5] = {};
							memset(aux_UTF8, 0, 4);

							aux_UTF8[0] = receive_SMS_data.phNumber[i];
							aux_UTF8[1] = receive_SMS_data.phNumber[i + 1];
							aux_UTF8[2] = receive_SMS_data.phNumber[i + 2];
							aux_UTF8[3] = receive_SMS_data.phNumber[i + 3];
							aux_UTF8[4] = 0;
							utf8decode((utf8chr_t *)aux_UTF8, &dst2);

							if (i == 0)
							{
								strcpy(dst1, dst2);
							}
							else
							{
								strcat(dst1, dst2);
							}
						}

						// ////printf("\n\n dst1 after hex  %s \n\n", dst1);
						memset(receive_SMS_data.phNumber, 0, strlen(receive_SMS_data.phNumber));
						sprintf(receive_SMS_data.phNumber, "%s", dst1);
						// ////printf("\n\n GMGL dst1\n %s \n\n", receive_SMS_data.phNumber);

						for (int i = 0; i < strlen(receive_SMS_data.receiveData); i = i + 4)
						{
							char aux_UTF8[5] = {};
							memset(aux_UTF8, 0, 4);

							aux_UTF8[0] = receive_SMS_data.receiveData[i];
							aux_UTF8[1] = receive_SMS_data.receiveData[i + 1];
							aux_UTF8[2] = receive_SMS_data.receiveData[i + 2];
							aux_UTF8[3] = receive_SMS_data.receiveData[i + 3];
							aux_UTF8[4] = 0;
							utf8decode((utf8chr_t *)aux_UTF8, &dst2);

							if (i == 0)
							{
								strcpy(dst3, dst2);
							}
							else
							{
								strcat(dst3, dst2);
							}
						}

						// ////printf("\n\n dst1 after receiveData  %s \n\n", dst1);

						memset(receive_SMS_data.receiveData, 0, strlen(receive_SMS_data.receiveData));
						sprintf(receive_SMS_data.receiveData, "%s", dst3);

						char atCommand[50] = {};
						// ////printf("\n\n GMGL aaaaa2\n\n");
						if (/* abs(atoi(receive_SMS_data.strHour) - nowTime.time) < 2 && (atoi(receive_SMS_data.strDate) == nowTime.date) &&  */ !strcmp(SMS_UNREAD, receive_SMS_data.SMS_Status))
						{
							// ////printf("\nENTER PARSE LOST SMS\n");

							// TODO:DESCOMENTAR SE NAO FUNCIONAR
							xSemaphoreGive(rdySem_Control_Send_AT_Command);

							if (!strcmp(check_IF_haveCountryCode(aux_own_Number, 0), check_IF_haveCountryCode(receive_SMS_data.phNumber, 0)))
							{
								sprintf(atCommand, "%s%d,0", "AT+CMGD=", receive_SMS_data.SMS_ID);
								EG91_send_AT_Command(atCommand, "OK", 1000);
							}
							else
							{
								// ////printf("\n\n GMGL aaaaa3\n\n");
								parse_SMS_data(&receive_SMS_data);
								// xSemaphoreGive(rdySem_Control_Send_AT_Command);

								sprintf(atCommand, "%s%d,0", "AT+CMGD=", receive_SMS_data.SMS_ID);
								EG91_send_AT_Command(atCommand, "OK", 1000);
							}

							// xSemaphoreGive(rdySem_Control_Send_AT_Command);
							// ////printf("\nEND PARSE LOST SMS\n");
							// xSemaphoreGive(rdySem_Control_Send_AT_Command);
						}
						else
						{
							// TODO:DESCOMENTAR SE NAO FUNCIONAR
							xSemaphoreGive(rdySem_Control_Send_AT_Command);
							sprintf(atCommand, "%s%d,0", "AT+CMGD=", receive_SMS_data.SMS_ID);
							// ////printf("\n\n GMGL aaaaa4 - %s\n\n", atCommand);
							EG91_send_AT_Command(atCommand, "OK", 1000);
							// ////printf("\n\n GMGL aaaaa41\n\n");
						}
						// xSemaphoreGive(rdySem_Control_Send_AT_Command);
					}
					// ////printf("\nEND PARSE SMS LIST\n");
				}
			}
		}
		xSemaphoreGive(rdySem_Control_Send_AT_Command);
	}
	else
	{
		xSemaphoreGive(rdySem_Control_Send_AT_Command);
		// ////printf("\n\n\nEND PARSE SMS LIST GIVE\n\n\n");
	}
	// ////printf("\n\n GMGL aaaaa5\n\n");
	// ////printf("\nEND PARSE SMS LIST\n");
	return 1;
}

uint8_t parse_Confirmation_Send_SMS(char *payload)
{

	if ((strstr(payload, "CMGS:")) != NULL)
	{

		if ((strstr(payload, "OK")) != NULL)
		{
			// ////printf("\nconfirmation_Send_SMS 2 - %s", payload);
			return 1;
		}
		else
		{
			// ////printf("\nconfirmation_Send_SMS 3 - %s", payload);
			return 0;
		}
	}
	else
	{

		return 0;
	}
	return 0;
}

static unsigned int
is_leapyear_aux(unsigned int y)
{
	return (y % 4) == 0 && ((y % 100) != 0 || (y % 400) == 0);
}

time_t epoch_Calculator(struct tm *tm)
{
	static const unsigned int ndays[2][12] = {
		{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
		{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
	time_t r = 0;
	int i;
	unsigned int *nday = (unsigned int *)ndays[is_leapyear_aux(tm->tm_year + 1900)];

	static const int epoch_year = 70;
	if (tm->tm_year >= epoch_year)
	{
		for (i = epoch_year; i < tm->tm_year; ++i)
			r += is_leapyear_aux(i + 1900) ? 366 * 24 * 60 * 60 : 365 * 24 * 60 * 60;
	}
	else
	{
		for (i = tm->tm_year; i < epoch_year; ++i)
			r -= is_leapyear_aux(i + 1900) ? 366 * 24 * 60 * 60 : 365 * 24 * 60 * 60;
	}
	for (i = 0; i < tm->tm_mon; ++i)
		r += nday[i] * 24 * 60 * 60;
	r += (tm->tm_mday - 1) * 24 * 60 * 60;
	r += tm->tm_hour * 60 * 60;
	r += tm->tm_min * 60;
	r += tm->tm_sec;
	return r;
}

uint8_t parse_NowTime(char *payload)
{
	// ////printf("\n parse_NowTime\n");

	// ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d", xPortGetFreeHeapSize());
	// ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
	// ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
	// ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	int auxCount = 0;
	int strIndex = 0;
	char aux_year[5] = {};
	char aux_month[3] = {};
	char aux_day[3] = {};
	char aux_hour[3] = {};
	char aux_minute[3] = {};
	char aux_second[3] = {};
	char timezone_network[5] = {};
	char timeZone_Signal = 0;

	uint8_t slashCounter = 0;
	uint8_t colonCounter = 0;
	uint8_t commaCounter = 0;

	memset(nowTime.strTime, 0, sizeof(nowTime.strTime));
	memset(aux_year, 0, sizeof(aux_year));
	memset(aux_month, 0, sizeof(aux_month));
	memset(aux_day, 0, sizeof(aux_day));
	memset(aux_hour, 0, sizeof(aux_hour));
	memset(aux_minute, 0, sizeof(aux_minute));

	// ////printf("\ntime payload %s\n", payload);

	for (int i = 0; i < strlen(payload); i++)
	{
		if (payload[i] == '"')
		{
			auxCount++;
		}
		else
		{
			if (auxCount == 1)
			{

				if (payload[i] == '+' || payload[i] == '-')
				{
					strIndex = 0;
					timeZone_Signal = payload[i];
					// timezone[strIndex++] = payload[i];
					auxCount++;
				}
				else
				{
					nowTime.strTime[strIndex++] = payload[i];
				}
			}
			else if (auxCount == 2)
			{
				if (payload[i] == ',')
				{
					break;
				}
				else
				{
					timezone_network[strIndex++] = payload[i];
				}
			}
		}
	}

	// ////printf("\n\nTIMEZONE QLTS - %c\n\n", timeZone_Signal);

	for (int i = 0; i < strlen(nowTime.strTime); i++)
	{
		if (nowTime.strTime[i] == '/')
		{
			slashCounter++;
		}
		else if (nowTime.strTime[i] == ':')
		{
			colonCounter++;
		}
		else if (nowTime.strTime[i] == ',')
		{
			commaCounter++;
		}
	}

	auxCount = 0;
	strIndex = 0;
	// ////printf("\ntime strtime %s\n", nowTime.strTime);

	if (slashCounter == 2 && colonCounter == 2 && commaCounter == 1)
	{
		// ////printf("\ntime strtime 11%s\n", nowTime.strTime);

		for (int i = 0; i < strlen(nowTime.strTime); i++)
		{
			if (nowTime.strTime[i] == '/' || nowTime.strTime[i] == ',' || nowTime.strTime[i] == ':')
			{
				auxCount++;
				strIndex = 0;
			}
			else
			{
				if (auxCount == 0)
				{
					aux_year[strIndex] = nowTime.strTime[i];
					strIndex++;
				}
				else if (auxCount == 1)
				{
					aux_month[strIndex] = nowTime.strTime[i];
					strIndex++;
				}
				else if (auxCount == 2)
				{
					aux_day[strIndex] = nowTime.strTime[i];
					strIndex++;
				}
				else if (auxCount == 3)
				{
					aux_hour[strIndex] = nowTime.strTime[i];
					strIndex++;
				}
				else if (auxCount == 4)
				{
					aux_minute[strIndex] = nowTime.strTime[i];
					strIndex++;
				}
				else if (auxCount == 5)
				{
					aux_second[strIndex] = nowTime.strTime[i];
					strIndex++;
				}
			}
		}

		// ////printf("\nyear year %s\n", aux_year);
		nowTime.date = ((atoi(aux_year) - 2000) * 10000) + (atoi(aux_month) * 100) + atoi(aux_day);
		nowTime.time = (atoi(aux_hour) * 100) + atoi(aux_minute);
		nowTime.year = (atoi(aux_year) - 2000);
		// ////printf("\nyear year111 %d\n", nowTime.year);
		nowTime.month = atoi(aux_month);
		nowTime.day = atoi(aux_day);
		int weekDay_now = calculate_weekDay(nowTime.year, nowTime.month, nowTime.day);

		nowTime.weekDay = pow(2, weekDay_now);
		// ////printf("\nyear year111 %s\n", aux_year);
		/*  uint8_t lastMonth = nowTime.month;
		 save_NVS_Last_Month(lastMonth); */

		// struct timeval epoch = {atoi(str_Clock), 0};
		struct tm timeinfo;

		int int_epoch = 0; // mktime(&timeinfo);

		// ////printf("\n\n\n int epoch %d\n\n\n", int_epoch);
		char timeZone[50] = {};

		esp_err_t err = 0;
		size_t required_size;

		// ////printf("\nENTER get_Data_STR_LastCALL_From_Storage\n");
		//  ////printf("\nerror 1 %d\n", err);

		// ////printf("\n\n\n getenv - %s",getenv("TZ"));
		//  settimeofday(&epoch, NULL);

		timeinfo.tm_year = atoi(aux_year) - 1900;
		timeinfo.tm_mon = atoi(aux_month) - 1;
		timeinfo.tm_mday = atoi(aux_day);
		timeinfo.tm_hour = atoi(aux_hour);
		timeinfo.tm_min = atoi(aux_minute);
		timeinfo.tm_sec = atoi(aux_second);
		timeinfo.tm_wday = weekDay_now;

		time_t now;
		struct tm timeinfo_Now;

		/* localtime_r(&now, &timeinfo_Now);*/

		// ////printf("\n\n year - %d, month - %d, day - %d, dayweek - %d, hour - %d, minute - %d, sec - %d\n\n", (timeinfo.tm_year + 1900), timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_wday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

		// int_epoch = mktime(&timeinfo);

		/* memset(timezone_network, 0, 5);
		sprintf(timezone_network, "%s", "32");
		timeZone_Signal = '-'; */
		// ////printf("\n\n timezone network %d\n\n", atoi(timezone_network));

		if (nvs_get_str(nvs_System_handle, NVS_TIMEZONE, NULL, &required_size) == ESP_OK)
		{
			// ////printf("\nrequire size %d\n", required_size);

			if (nvs_get_str(nvs_System_handle, NVS_TIMEZONE, timeZone, &required_size) == ESP_OK)
			{
				// ////printf("\n\n enter qlts memory\n\n");
				if (strcmp(timeZone, "GMT0"))
				{
					PCF85_SetTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
					PCF85_SetDate(timeinfo.tm_wday, timeinfo.tm_mday, (timeinfo.tm_mon + 1), (timeinfo.tm_year + 1900));

					// asprintf(timeinfo.tm_zone,"%s",timeZone);
					int_epoch = epoch_Calculator(&timeinfo);
					// int_epoch = mktime(&timeinfo);
					// ////printf("\n\n epoch11 - %d\n\n", int_epoch);
				}
				else
				{
					if (timeZone_Signal == '+')
					{
						int_epoch = epoch_Calculator(&timeinfo) + atoi(timezone_network) * 15 * 60;
						// ////printf("\n\n int_epoch plus 55\n\n");
					}
					else if (timeZone_Signal == '-')
					{
						int_epoch = epoch_Calculator(&timeinfo) - atoi(timezone_network) * 15 * 60;
						// ////printf("\n\n int_epoch minus 66\n\n");
					}

					// ////printf("\n\n int_epoch %d\n\n", int_epoch);

					memset(timeZone, 0, 50);

					gmtime_r(&int_epoch, &timeinfo_Now);
					PCF85_SetTime(timeinfo_Now.tm_hour, timeinfo_Now.tm_min, timeinfo_Now.tm_sec);
					PCF85_SetDate(timeinfo_Now.tm_wday, timeinfo_Now.tm_mday, (timeinfo_Now.tm_mon + 1), (timeinfo_Now.tm_year + 1900));

					sprintf(timeZone, "%s", "GMT0");
				}

				// int_epoch = mktime(&timeinfo);
			}
			else
			{

				if (timeZone_Signal == '+')
				{
					int_epoch = epoch_Calculator(&timeinfo) + atoi(timezone_network) * 15 * 60;
					// ////printf("\n\n int_epoch plus\n\n");
				}
				else if (timeZone_Signal == '-')
				{
					int_epoch = epoch_Calculator(&timeinfo) - atoi(timezone_network) * 15 * 60;
					// ////printf("\n\n int_epoch minus\n\n");
				}

				// ////printf("\n\n int_epoch %d\n\n", int_epoch);

				memset(timeZone, 0, 50);

				gmtime_r(&int_epoch, &timeinfo_Now);
				PCF85_SetTime(timeinfo_Now.tm_hour, timeinfo_Now.tm_min, timeinfo_Now.tm_sec);
				PCF85_SetDate(timeinfo_Now.tm_wday, timeinfo_Now.tm_mday, (timeinfo_Now.tm_mon + 1), (timeinfo_Now.tm_year + 1900));

				sprintf(timeZone, "%s", "GMT0");
			}
		}
		else
		{
			if (timeZone_Signal == '+')
			{

				int_epoch = epoch_Calculator(&timeinfo) + atoi(timezone_network) * 15 * 60;

				// ////printf("\n\n plus clock epoch %d, %d\n\n", int_epoch, atoi(timezone_network));
			}
			else if (timeZone_Signal == '-')
			{

				int_epoch = epoch_Calculator(&timeinfo) - atoi(timezone_network) * 15 * 60;
			}

			memset(timeZone, 0, 50);

			gmtime_r(&int_epoch, &timeinfo_Now);
			PCF85_SetTime(timeinfo_Now.tm_hour, timeinfo_Now.tm_min, timeinfo_Now.tm_sec);
			PCF85_SetDate(timeinfo_Now.tm_wday, timeinfo_Now.tm_mday, (timeinfo_Now.tm_mon + 1), (timeinfo_Now.tm_year + 1900));

			sprintf(timeZone, "%s", "GMT0");
		}

		struct timeval epoch = {.tv_sec = int_epoch};

		//     ////printf("\n\n hour hour 1221\n year - %d, month - %d, day - %d, dayweek - %d, hour - %d, minute - %d, sec - %d\n\n", (timeinfo_Now.tm_year + 1900), timeinfo_Now.tm_mon, timeinfo_Now.tm_mday, timeinfo_Now.tm_wday, timeinfo_Now.tm_hour, timeinfo_Now.tm_min, timeinfo_Now.tm_sec);

		if (!settimeofday(&epoch, NULL))
		{

			// ////printf("\n\n epoch33 - %d\n\n", int_epoch);
			time(&now);

			// ////printf("\n\n timezone qlts %s", timeZone);
			if (!setenv("TZ", timeZone, 1))
			{

				// ////printf("\n\n\n getenv22 - %s", getenv("TZ"));
				save_STR_Data_In_Storage(NVS_TIMEZONE, timeZone, nvs_System_handle);
				tzset();

				localtime_r(&now, &timeinfo_Now);

				// ////printf("\n\n hour hour 1221\n year - %d, month - %d, day - %d, dayweek - %d, hour - %d, minute - %d, sec - %d\n\n", (timeinfo_Now.tm_year + 1900), timeinfo_Now.tm_mon, timeinfo_Now.tm_mday, timeinfo_Now.tm_wday, timeinfo_Now.tm_hour, timeinfo_Now.tm_min, timeinfo_Now.tm_sec);
			}
			else
			{
				// enableAlarm();
				return 0;
			}
		}
		else
		{
			// enableAlarm();
			return 0;
		}

		label_To_Send_Periodic_SMS = get_INT8_Data_From_Storage(NVS_KEY_LABEL_PERIODIC_SMS, nvs_System_handle);

		if (label_To_Send_Periodic_SMS == 255)
		{
			label_To_Send_Periodic_SMS = 0;
		}

		// label_To_Send_Periodic_SMS = 0;
		if (label_To_Send_Periodic_SMS == 0)
		{

			label_To_Send_Periodic_SMS = 1;
			save_INT8_Data_In_Storage(NVS_KEY_LABEL_PERIODIC_SMS, label_To_Send_Periodic_SMS, nvs_System_handle);

			date_To_Send_Periodic_SMS = parseDatetoInt(nowTime.date, 60);
			nvs_set_u32(nvs_System_handle, NVS_KEY_DATE_PERIODIC_SMS, date_To_Send_Periodic_SMS);
			// ////printf("\n\n enter label inicial sms periodic %d\n\n", date_To_Send_Periodic_SMS);
		}
		// getLocalTime(&timeinfo_Now, 0);

		// timeinfo_Now = localtime(&now);
		// localtime_r(&now, &timeinfo_Now);
		char str_Time[256] = {};

		strftime(str_Time, 256, "%c", &timeinfo_Now);

		// ////printf("\n\n str_Time - %s\n\n", str_Time);

		// printftf("\n\n hour hour 1221\n year - %d, month - %d, day - %d, dayweek - %d, hour - %d, minute - %d, sec - %d\n\n", (timeinfo_Now.tm_year + 1900), timeinfo_Now.tm_mon, timeinfo_Now.tm_mday, timeinfo_Now.tm_wday, timeinfo_Now.tm_hour, timeinfo_Now.tm_min, timeinfo_Now.tm_sec);

		get_RTC_System_Time();

		// ////printf("\nafter verify_WeekAcess\n");
		// ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d", xPortGetFreeHeapSize());
		// ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
		// ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
		// ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
		// enableAlarm();
		return 1;
	}

	return 0;
}

char *activateUDP_network()
{
	char rsp[50] = {};
	uint8_t InitNetworkCount = 0;
	label_network_portalRegister = 1;
	save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER, label_network_portalRegister, nvs_System_handle);
	timer_pause(TIMER_GROUP_1, TIMER_0);

	if (gpio_get_level(GPIO_INPUT_IO_SIMPRE))
	{

		if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
		{
			RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

			gpio_set_level(GPIO_OUTPUT_ACT, 1);

			while (InitNetworkCount < 3)
			{

				// ////printf("INIT EG915 311");
				if (!EG91_initNetwork())
				{
					RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

					gpio_set_level(GPIO_OUTPUT_ACT, 1);
					update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
					InitNetworkCount++;
				}
				else
				{
					// ////printf("\n\n activate network 1\n\n");
					break;
				}
			}

			if (InitNetworkCount == 3)
			{
				label_network_portalRegister = 0;
				save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER, label_network_portalRegister, nvs_System_handle);
				save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 1, nvs_System_handle);
				RSSI_LED_TOOGLE = MQTT_NOT_CONECT_LED_TIME;
				timer_start(TIMER_GROUP_1, TIMER_0);
				return "ERROR";
			}
		}
		else
		{
			gpio_set_level(GPIO_OUTPUT_ACT, 1);
			uint8_t pwrFD = 0;
			uint8_t pwrFD_count = 0;

			while (pwrFD != 1)
			{
				if (pwrFD_count == 3 || pwrFD == 1)
				{
					break;
				}
				pwrFD_count++;

				pwrFD = EG91_PowerOn();
			}

			if (pwrFD == 1)
			{

				while (InitNetworkCount < 3)
				{
					// ////printf("INIT EG915 311");
					if (!EG91_initNetwork())
					{
						RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

						gpio_set_level(GPIO_OUTPUT_ACT, 1);
						update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
						InitNetworkCount++;
					}
					else
					{
						// ////printf("\n\n activate network 1\n\n");
						break;
					}
				}

				if (InitNetworkCount == 3)
				{
					label_network_portalRegister = 0;
					save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER, label_network_portalRegister, nvs_System_handle);
					timer_start(TIMER_GROUP_1, TIMER_0);
					return "ERROR";
				}
			}
		}
	}

	// ////printf("\n\n activate network 22\n\n");
	//  sprintf(rsp, "ME S W %s", "OK");
	timer_start(TIMER_GROUP_1, TIMER_0);
	return "OK";
}

uint8_t desactivateUDP_network()
{
	char rsp[50];
	label_network_portalRegister = 0;
	EG91_send_AT_Command("AT+QMTCONN?", "AT+QMTCONN?", 1000);

	if (mqtt_connectLabel)
	{
		if (EG91_send_AT_Command("AT+QMTDISC=0", "OK", 30000))
		{
			save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER, label_network_portalRegister, nvs_System_handle);
			return 1;
		}
	}
	else
	{
		save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER, label_network_portalRegister, nvs_System_handle);
		return 1;
	}

	return 0;
}

int8_t parse_IncomingCall_Payload(char *payload)
{
	char phNumber[18];
	int strIndex = 0;
	char callState = 0;
	int dotCounter = 0;
	char id[2] = {};
	char ATCommand[100] = {};
	char sdCard_log[100] = {};
	char incomingCALL_IDX = 0;
	uint8_t plusCounter = 0;
	// ////printf("\n\nparse_IncomingCall_Payload: %s\n\n", payload);
	memset(phNumber, 0, sizeof(phNumber));

	for (int i = 0; i < strlen(payload); i++)
	{
		if (payload[i] == ':')
		{
			plusCounter++;
		}
	}

	// EG91_send_AT_Command("AT+CHLD=3", "OK", 2000);

	// plusCounter = plusCounter - 1;
	// ////printf("\nplusCounter = %d\n", plusCounter);

	if (plusCounter == 0 || plusCounter == 255)
	{
		if (strstr(payload, "OK") != NULL)
		{
			return -1;
		}

		return -1;
	}
	else if (plusCounter > 1)
	{
		for (int i = plusCounter; i > 1; i--)
		{
			// ////printf("\nplusCounter in for = %d\n", plusCounter);
			if (plusCounter <= 1)
			{
				plusCounter = 1;
				break;
			}

			// sprintf(ATCommand, "%s%d", "AT+CHLD=1" /* "AT+QHUP=17" */, plusCounter);
			// EG91_send_AT_Command(ATC                                                            ommand, "OK", 2000);
		}
		// return 255;
	}
	plusCounter = 0;
	if ((strstr(payload, "CLCC")))
	{

		// ////printf("\n\nCLCC payload %s \n\n", payload);
		for (int i = 0; i < strlen(payload); i++)
		{
			if (payload[i] == ':' && payload[i + 1] == ' ')
			{
				for (i = i + 2; i < strlen(payload); i++)
				{

					if (payload[i] == ',')
					{
						dotCounter++;
						strIndex = 0;
					}
					else
					{
						if (dotCounter == 0)
						{
							id[strIndex] = payload[i];
							strIndex++;

							if (strIndex == 2)
							{
								return 0;
								// parseCHUP("AT+QHUP=27");
								break;
							}
						}

						if (dotCounter == 2)
						{
							callState = payload[i];
						}

						if (dotCounter == 5)
						{
							if (payload[i] != '"')
							{
								phNumber[strIndex] = payload[i];
								strIndex++;
							}
						}
					}
				}
			}
		}
		// system_stack_high_water_mark("call state");
		//  ////printf("\ncallState - %c\n", callState);
		if (callState == '4')
		{
			/* code */
			char awnser_QHUP[30] = {};
			char cpy_phNumber[18] = {};
			memset(cpy_phNumber, 0, sizeof(cpy_phNumber));
			strcpy(cpy_phNumber, phNumber);

			save_STR_Data_In_Storage(NVS_LAST_CALL, phNumber, nvs_System_handle);

			 /* sprintf(phNumber, "%s", "34637239294");
			printf("\n incoming call phnumber: %s\n", phNumber); */
			int line = 0;
			char aabff[200];
			MyUser user_validateData;
			memset(&user_validateData, 0, sizeof(user_validateData));
			memset(sdCard_log, 0, sizeof(sdCard_log));
			data_EG91_Send_SMS IncomingCALL_data;
			IncomingCALL_data.labelIncomingCall = 1;

			get_RTC_System_Time();

			vTaskDelay(pdMS_TO_TICKS((2000)));
			// vTaskDelay(200);
			esp_err_t search_User_err = ESP_FAIL;
			char *phNumber1 = remove_non_printable(phNumber, strlen(phNumber));
			;
			// //printf("\n\n ph number aux 0000 %s - %d\n\n", phNumber, search_User_err);
			// sprintf(phNumber, "%s", remove_non_printable(phNumber, strlen(phNumber)));
			// //printf("\n\n ph number aux 1111 %s - %d\n\n", phNumber, search_User_err);
			search_User_err = MyUser_Search_User(phNumber1, aabff);

			
			// //printf("\n\n ph number aux 2222 %s - %d\n\n", phNumber, search_User_err);
			if (search_User_err != ESP_OK)
			{
				memset(aabff, 0, sizeof(aabff));
				search_User_err = MyUser_Search_User_AUX_Call(phNumber1, aabff);
			}

			free(phNumber1);
			uint8_t sms_call_verifications = get_INT8_Data_From_Storage(NVS_SMS_CALL_VERIFICATION, nvs_System_handle);

			if (sms_call_verifications == 255)
			{
				save_INT8_Data_In_Storage(NVS_SMS_CALL_VERIFICATION, 0, nvs_System_handle);
				sms_call_verifications = 0;
			}

			// //printf("\n\n ph number aux 6666 %s - %d\n\n", phNumber, search_User_err);
			if (search_User_err == ESP_OK)
			{
				parse_ValidateData_User(aabff, &user_validateData);
				// ////printf("\n\n ph number aux 6666 %s - %d\n\n",phNumber,search_User_err);
				// ////printf("\n\n ph number aux 7777 %d - %d - %c - %c\n\n",label_Routine1_ON,fd_configurations.alarmMode.A,user_validateData.relayPermition, user_validateData.permition);
				if (user_validateData.permition == '0')
				{
					if ( user_validateData.relayPermition == '1')
					{

						sprintf(awnser_QHUP, "%s,%s", "AT+QHUP=17", id);
						EG91_send_AT_Command(awnser_QHUP, "OK", 500);

						// parseCHUP(awnser_QHUP);

						/* if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD))
						{ */
						sdCard_Logs_struct logs_struct;
						memset(&logs_struct, 0, sizeof(logs_struct));

						sprintf(logs_struct.type, "%s", return_Json_SMS_Data("CALL"));

						sprintf(logs_struct.name, "%s", user_validateData.firstName);

						sprintf(logs_struct.phone, "%s", user_validateData.phone);
						sprintf(logs_struct.relay, "%s", "R1");
						sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("NOT_CHANGE"));

						if (get_RTC_System_Time())
						{
							sprintf(logs_struct.date, "%s", replace_Char_in_String(nowTime.strTime, ',', ';'));

							if (user_validateData.relayPermition == '1')
							{
								sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERROR_LOGS_NOT_HAVE_PERMITION_THIS_RELAY"));
							}
							else if (label_Routine1_ON == 1)
							{
								sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
							}
							else
							{
								sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERROR_LOGS_USER_NOT_PERMITION"));
							}
						}
						else
						{
							sprintf(logs_struct.date, "%s", "0;0");
							sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));

							if (user_validateData.relayPermition == '1')
							{
								strcat(logs_struct.error, "/\0");
								strcat(logs_struct.error, return_Json_SMS_Data("ERROR_LOGS_NOT_HAVE_PERMITION_THIS_RELAY"));
							}
							else if (label_Routine1_ON == 1)
							{
								sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
							}
							else
							{
								strcat(logs_struct.error, "/\0");
								strcat(logs_struct.error, return_Json_SMS_Data("ERROR_LOGS_USER_NOT_PERMITION"));
							}
						}

						sdCard_Write_LOGS(&logs_struct);
						//}
						// ////printf("\nuser not acess call\n");
						return 0;
					}

					if (verify_TimeAcess(&user_validateData) || sms_call_verifications == 2 || sms_call_verifications == 4)
					{

						// EG91_send_AT_Command("AT+QHUP=88,1", "OK", 1000);
						IncomingCALL_data.labelIncomingCall = 1;
						sprintf(awnser_QHUP, "%s,%s", "AT+QHUP=17", id);
						EG91_send_AT_Command(awnser_QHUP, "OK", 1000);
						// EG91_send_AT_Command(awnser_QHUP, "OK", 1000);
						// EG91_send_AT_Command("AT+QHUP=88,1", "OK", 500);
						// parseCHUP("AT+QHUP=88,1");
						setReles(RELE1_NUMBER, NULL, NULL, NULL, NULL, &IncomingCALL_data, NULL);

						/* if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD))
						{ */
						sdCard_Logs_struct logs_struct;
						memset(&logs_struct, 0, sizeof(logs_struct));

						sprintf(logs_struct.type, "%s", return_Json_SMS_Data("CALL"));

						sprintf(logs_struct.name, "%s", user_validateData.firstName);

						sprintf(logs_struct.phone, "%s", user_validateData.phone);
						sprintf(logs_struct.relay, "%s", "R1");

						rele1_Mode_Label = get_INT8_Data_From_Storage(NVS_RELAY1_MODE, nvs_System_handle);

						if (rele1_Mode_Label == BIESTABLE_MODE_INDEX)
						{
							if (getRele1())
							{
								sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("ON"));
							}
							else
							{
								sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("OFF"));
							}
						}
						else
						{
							sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("PULSE"));
						}

						sprintf(logs_struct.date, "%s", replace_Char_in_String(nowTime.strTime, ',', ';'));

						sdCard_Write_LOGS(&logs_struct);
						//}

						char rsp_notify[100] = {};
						sprintf(rsp_notify, "%s %c %c %d", RELE1_ELEMENT, 'S', 'R', getRele1());

						BLE_Broadcast_Notify(rsp_notify);
						// vTaskDelay(300);

						// vTaskDelay(pdMS_TO_TICKS((1000)));

						return 1;
					}
					else
					{
						// vTaskDelay(pdMS_TO_TICKS((300)));
						sprintf(awnser_QHUP, "%s,%s", "AT+QHUP=17", id);
						EG91_send_AT_Command(awnser_QHUP, "OK", 1000);
						// parseCHUP("AT+QHUP=27");

						/* if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD))
						{ */
						sdCard_Logs_struct logs_struct;
						memset(&logs_struct, 0, sizeof(logs_struct));

						sprintf(logs_struct.type, "%s", return_Json_SMS_Data("CALL"));

						sprintf(logs_struct.name, "%s", user_validateData.firstName);

						sprintf(logs_struct.phone, "%s", user_validateData.phone);
						sprintf(logs_struct.relay, "%s", "R1");
						sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("NOT_CHANGE"));

						sprintf(logs_struct.date, "%s", replace_Char_in_String(nowTime.strTime, ',', ';'));

						sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERROR_LOGS_USER_NOT_PERMITION"));
						sdCard_Write_LOGS(&logs_struct);
						//}
						// ////printf("\nuser not acess call\n");
						return 0;
					}
				}
				else if (user_validateData.permition == '1' || user_validateData.permition == '2')
				{

					if (label_Routine1_ON == 1)
					{
						if (user_validateData.permition == '2')
						{
							sprintf(awnser_QHUP, "%s,%s", "AT+QHUP=17", id);
							EG91_send_AT_Command(awnser_QHUP, "OK", 1000);
							// parseCHUP(awnser_QHUP);
							resetRele1();
							label_Routine1_ON = 0;
							save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, 0, nvs_System_handle);
							save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_TIME_ON, 0, nvs_System_handle);

							/* if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD))
							{ */
							sdCard_Logs_struct logs_struct;
							memset(&logs_struct, 0, sizeof(logs_struct));

							sprintf(logs_struct.type, "%s", "CALL");

							sprintf(logs_struct.name, "%s", return_Json_SMS_Data("LOGS_DISABLE_ROUTINE"));
							sprintf(logs_struct.phone, "%s", user_validateData.phone);
							sprintf(logs_struct.relay, "%s", "R1");

							sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("OFF"));

							if (get_RTC_System_Time())
							{
								sprintf(logs_struct.date, "%s", replace_Char_in_String(nowTime.strTime, ',', ';'));
							}
							else
							{
								sprintf(logs_struct.date, "%s", "0,0");
								sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
							}

							sdCard_Write_LOGS(&logs_struct);
							//}

							char rsp_notify[100] = {};
							sprintf(rsp_notify, "%s %c %c %d", RELE1_ELEMENT, 'S', 'R', getRele1());

							BLE_Broadcast_Notify(rsp_notify);
							// ////printf("\nuser not acess call\n");
							return 1;
						}
						else
						{
							sprintf(awnser_QHUP, "%s,%s", "AT+QHUP=17", id);
							EG91_send_AT_Command(awnser_QHUP, "OK", 1000);
							// parseCHUP("AT+QHUP=27");

							/* if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD))
							{ */
							sdCard_Logs_struct logs_struct;
							memset(&logs_struct, 0, sizeof(logs_struct));

							sprintf(logs_struct.type, "%s", return_Json_SMS_Data("CALL"));

							sprintf(logs_struct.name, "%s", user_validateData.firstName);

							sprintf(logs_struct.phone, "%s", user_validateData.phone);
							sprintf(logs_struct.relay, "%s", "R1");
							sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("NOT_CHANGE"));

							if (get_RTC_System_Time())
							{
								sprintf(logs_struct.date, "%s", replace_Char_in_String(nowTime.strTime, ',', ';'));
								sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
							}
							else
							{
								sprintf(logs_struct.date, "%s", "0;0");
								sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
								strcat(logs_struct.error, "/\0");
								strcat(logs_struct.error, return_Json_SMS_Data("ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
							}

							sdCard_Write_LOGS(&logs_struct);
							// }
							// ////printf("\nuser not acess call\n");
							return 0;
						}
					}

					IncomingCALL_data.labelIncomingCall = 1;
					// vTaskDelay(pdMS_TO_TICKS((300)));
					//
					sprintf(awnser_QHUP, "%s,%s", "AT+QHUP=17", id);
					EG91_send_AT_Command(awnser_QHUP, "OK", 500);
					// parseCHUP(awnser_QHUP);
					setReles(RELE1_NUMBER, NULL, NULL, NULL, NULL, &IncomingCALL_data, NULL);

					
					// alarm_I1_Check_And_Save_Data(user_validateData.phone);

					/* if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD))
					{ */
					sdCard_Logs_struct logs_struct;
					memset(&logs_struct, 0, sizeof(logs_struct));

					sprintf(logs_struct.type, "%s", return_Json_SMS_Data("CALL"));

					sprintf(logs_struct.name, "%s", user_validateData.firstName);

					sprintf(logs_struct.phone, "%s", user_validateData.phone);
					sprintf(logs_struct.relay, "%s", "R1");

					// ////printf("\nCALL label_MonoStableRelay1 %d\n", label_MonoStableRelay1);

					rele1_Mode_Label = get_INT8_Data_From_Storage(NVS_RELAY1_MODE, nvs_System_handle);

					if (rele1_Mode_Label == BIESTABLE_MODE_INDEX)
					{
						if (getRele1())
						{
							sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("ON"));
						}
						else
						{
							sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("OFF"));
						}
					}
					else
					{
						sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("PULSE"));
					}

					if (get_RTC_System_Time())
					{
						sprintf(logs_struct.date, "%s", replace_Char_in_String(nowTime.strTime, ',', ';'));
					}
					else
					{
						sprintf(logs_struct.date, "%s", "0;0");
						sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
					}

					sdCard_Write_LOGS(&logs_struct);
					// }

					char rsp_notify[100] = {};
					sprintf(rsp_notify, "%s %c %c %d", RELE1_ELEMENT, 'S', 'R', getRele1());

					BLE_Broadcast_Notify(rsp_notify);

					return 1;
				}
				else
				{
					// vTaskDelay(pdMS_TO_TICKS((300)));
					sprintf(awnser_QHUP, "%s,%s", "AT+QHUP=17", id);
					EG91_send_AT_Command(awnser_QHUP, "OK", 1000);
					// parseCHUP("AT+QHUP=27");
					//  EG91_send_AT_Command("AT+QHUP=27", "OK", 500);
					// ////printf("\nuser not acess call 777722\n");
					return 0;
				}

				// EG91_send_AT_Command("AT+QLTS=2", "QLTS", 1000);
				//  EG91_send_AT_Command("AT+CCLK?", "OK", 500);
			}
			else
			{

				if (label_Routine1_ON == 0 )
				{
					if (sms_call_verifications == 2 || sms_call_verifications == 4)
					{

						setReles(RELE1_NUMBER, NULL, NULL, NULL, NULL, &IncomingCALL_data, NULL);

						sdCard_Logs_struct logs_struct;
						memset(&logs_struct, 0, sizeof(logs_struct));

						sprintf(logs_struct.type, "%s", return_Json_SMS_Data("CALL"));

						sprintf(logs_struct.name, "%s", return_Json_SMS_Data("NO_NAME"));

						sprintf(logs_struct.phone, "%s", cpy_phNumber);
						sprintf(logs_struct.relay, "%s", "R1");

						if (rele1_Mode_Label == BIESTABLE_MODE_INDEX)
						{
							if (getRele1())
							{
								sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("ON"));
							}
							else
							{
								sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("OFF"));
							}
						}
						else
						{
							sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("PULSE"));
						}

						if (get_RTC_System_Time())
						{
							sprintf(logs_struct.date, "%s", replace_Char_in_String(nowTime.strTime, ',', ';'));
						}
						else
						{
							sprintf(logs_struct.date, "%s", "0;0");
							sprintf(logs_struct.error, "%s ", return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
						}

						sdCard_Write_LOGS(&logs_struct);
					}
					else
					{
						/* vTaskDelay(pdMS_TO_TICKS((300)));
						sprintf(awnser_QHUP, "%s,%s", "AT+QHUP=17", id);
						EG91_send_AT_Command(awnser_QHUP, "OK", 1000); */
						// parseCHUP("AT+QHUP=27");

						sdCard_Logs_struct logs_struct;
						memset(&logs_struct, 0, sizeof(logs_struct));

						sprintf(logs_struct.type, "%s", return_Json_SMS_Data("CALL"));

						sprintf(logs_struct.name, "%s", return_Json_SMS_Data("NO_NAME"));

						sprintf(logs_struct.phone, "%s", cpy_phNumber);
						sprintf(logs_struct.relay, "%s", "R1");
						sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("NOT_CHANGE"));

						if (get_RTC_System_Time())
						{
							sprintf(logs_struct.date, "%s", replace_Char_in_String(nowTime.strTime, ',', ';'));
						}
						else
						{
							sprintf(logs_struct.date, "%s", "0;0");
							sprintf(logs_struct.error, "%s ", return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));
						}

						sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERROR_LOGS_USER_NOT_FOUND"));
						sdCard_Write_LOGS(&logs_struct);

						// EG91_send_AT_Command("AT+QHUP=27", "OK", 500);
						// ////printf("\nuser not acess call\n");

						/* return 0; */
					}
				}
				else
				{
					sdCard_Logs_struct logs_struct;
					memset(&logs_struct, 0, sizeof(logs_struct));

					sprintf(logs_struct.type, "%s", return_Json_SMS_Data("CALL"));

					sprintf(logs_struct.name, "%s", return_Json_SMS_Data("NO_NAME"));

					sprintf(logs_struct.phone, "%s", cpy_phNumber);
					sprintf(logs_struct.relay, "%s", "R1");
					sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("NOT_CHANGE"));

					if (get_RTC_System_Time())
					{
						sprintf(logs_struct.date, "%s", replace_Char_in_String(nowTime.strTime, ',', ';'));

						if (label_Routine1_ON == 1)
						{
							sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
						}
						else
						{
							sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERROR_LOGS_USER_NOT_PERMITION"));
						}
					}
					else
					{
						sprintf(logs_struct.date, "%s", "0;0");
						sprintf(logs_struct.error, "%s ", return_Json_SMS_Data("ERRO_LOGS_GET_TIME"));

						if (label_Routine1_ON == 1)
						{
							sprintf(logs_struct.error, "%s", return_Json_SMS_Data("ERROR_LOGS_IS_RUNNING_ROUTINE_ON_RELAY"));
						}
						else
						{
							strcat(logs_struct.error, "/\0");
							strcat(logs_struct.error, return_Json_SMS_Data("ERROR_LOGS_USER_NOT_PERMITION"));
						}
					}

					sdCard_Write_LOGS(&logs_struct);
				}
			}
			vTaskDelay(pdMS_TO_TICKS((300)));

			sprintf(awnser_QHUP, "%s,%s", "AT+QHUP=17", id);
			EG91_send_AT_Command(awnser_QHUP, "OK", 1000);
			// parseCHUP("AT+QHUP=27");
			//  EG91_send_AT_Command("AT+QHUP=27", "OK", 500);
			// ////printf("\nincoming call ph %s\n", phNumber);
			return 0;
		}
		else if (callState == '0')
		{
			// ////printf("\n return call state = 0\n");
			return 0;
		}
		else if (callState < '0' || callState > '5')
		{
			// ////printf("\n return call state = -1\n");
			return -1;
		}
		else
		{
			////printf("\n return call state = %d\n", (callState - 48));
			// system_stack_high_water_mark("parse call2");
			return callState - 48;
		}
	}
	else
	{
		return 0;
	}
	return 9;
}

uint8_t getFile_importUsers(char *DNS)
{
	/* char domain[300] = {};
	sprintf(domain,"AT+QHTTPURL=%d,80",strlen("https://static.portal.home.motorline.pt/docs/teste.txt"));
	char ct[2] = {};
		ct[0] = 13;
		ct[1] = 0;

	 EG91_send_AT_Command("AT+QHTTPCFG=\"contextid\",1","OK", 1000);
	 EG91_send_AT_Command("AT+QHTTPCFG=\"responseheader\",1","OK", 1000);
	 EG91_send_AT_Command("AT+QHTTPCFG=\"sslctxid\",1","OK", 1000);
	 EG91_send_AT_Command("AT+QSSLCFG=\"sslversion\",1,1","OK", 1000);
	 EG91_send_AT_Command("AT+QSSLCFG=\"ciphersuite\",1,0x0005","OK", 1000);
	 EG91_send_AT_Command("AT+QSSLCFG=\"seclevel\",1,0","OK", 1000);
	 strcat(domain,ct);
	 uart_write_bytes(UART_NUM_1, domain, strlen(domain));
	 vTaskDelay( pdMS_TO_TICKS(1000));
	 uart_write_bytes(UART_NUM_1, "https://static.portal.home.motorline.pt/docs/teste.txt", 85);
	 vTaskDelay( pdMS_TO_TICKS(1000));
	 EG91_send_AT_Command("AT+QHTTPREAD=80","OK", 1000);  */
	/*EG91_send_AT_Command("https://www.google.com/webhp?hl=pt-PT&sa=X&ved=0ahUKEwj2wuSZs9qEAxWpX0EAHSWmCzkQPAgJ","OK", 1000);  */

	return 1;
}

void task_EG91_SendSMS(void *pvParameter)
{

	data_EG91_Send_SMS cpy_message;
	queue_EG91_SendSMS = xQueueCreate(2, sizeof(data_EG91_Send_SMS));
	rdySem_Control_SEND_SMS_Task = xSemaphoreCreateBinary();
	xSemaphoreTake(rdySem_Control_SEND_SMS_Task, 0);
	// xSemaphoreGive(rdySem_Control_SMS_Task);
	for (;;)
	{

		if (xQueueReceive(queue_EG91_SendSMS, &(cpy_message), portMAX_DELAY))
		{
			xSemaphoreTake(rdySem_Control_SMS_UDP, pdMS_TO_TICKS(7000));
			disableAlarm();
			// vTaskSuspend(xHandle_Timer_VerSystem);
			//  ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));

			// TODO: COMENTAR SE NAO FUNCIONAR
			// vTaskSuspend(handle_UDP_TASK);

			//  xSemaphoreGive(rdySem_Control_Send_AT_Command)

			// TODO: VERIFICAR SE O TIMER E A TASK ESTAO A CORRER OU PARADA
			/* if (label_Reset_Password_OR_System != 1)
			{
				////printf("\nsend sms %s\n", cpy_message.payload);
				timer_pause(TIMER_GROUP_1, TIMER_0);
				////printf("\nsend sms11 %s\n", cpy_message.payload);
				////printf("\nsms\n");
				//  disableAlarm();
				vTaskSuspend(xHandle_Timer_VerSystem);
				////printf("\nsend sms22 %s\n", cpy_message.payload);
			} */
			timer_pause(TIMER_GROUP_1, TIMER_0);
			// vTaskSuspend(handle_SEND_SMS_TASK);

			// TODO: UNNCOMMENT IF NOT WORK
			// xSemaphoreGive(rdySem_Control_Send_AT_Command);

			// ////printf("\nsend sms33 %s\n", cpy_message.payload);

			EG91_send_AT_Command("AT+QINDCFG=\"ring\",0,0", "OK", 1500);
			// xSemaphoreGive(rdySem_Control_Send_AT_Command);

			// ////printf("\nsend sms phNumber %s\n", cpy_message.phoneNumber);
			/*
						////printf("\n\nsend sms after11\n\n");

						//ESP_LOGI("TAG", "xPortGetFreeHeapSize  parseInputData1111: %d", xPortGetFreeHeapSize());
						//ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
						//ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
						//ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));

						////printf("\n\nsend sms after22\n\n"); */

			// ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
			// ////printf("\n\n strlen SIM BALANCE 22 - %d\n\n", strlen(cpy_message.payload));

			EG91_Send_SMS(cpy_message.phoneNumber, cpy_message.payload);

			/* ESP_ERROR_CHECK(heap_trace_stop());
			heap_trace_dump(); */

			/*   ////printf("\n\nsend sms after 33\n\n");

			  //ESP_LOGI("TAG", "xPortGetFreeHeapSize  parseInputData1111: %d", xPortGetFreeHeapSize());
			  //ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
			  //ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
			  //ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));

			  ////printf("\n\nsend sms after44\n\n"); */

			// TODO: UNNCOMMENT IF NOT WORK
			// xSemaphoreGive(rdySem_Control_Send_AT_Command);

			EG91_send_AT_Command("AT+QINDCFG=\"ring\",1,0", "OK", 1500);
			// xSemaphoreGive(rdySem_Control_Send_AT_Command);

			// vTaskResume(xHandle_Timer_VerSystem);
			timer_start(TIMER_GROUP_1, TIMER_0);
			enableAlarm();

			if (label_Semaphore_Reset_System == 1)
			{
				label_Semaphore_Reset_System = 0;
				give_rdySem_Reset_System();
			}

			// xSemaphoreGive(rdySem_Control_Send_AT_Command);

			if (lost_SMS_addUser.flag_Lost_SMS_Add_User != 1)
			{
				// ////printf("\n\nxSemaphoreGive(rdySem_Control_SMS_UDP);\n\n");
				xSemaphoreGive(rdySem_Control_SMS_UDP);
			}
			else
			{
				// ////printf("\n\n NOT NOT xSemaphoreGive(rdySem_Control_SMS_UDP);\n\n");
			}

			// xSemaphoreGive(rdySem_Lost_SMS);

			// ESP_ERROR_CHECK(heap_trace_stop());
			// heap_trace_dump();
			//  vTaskResume(xHandle_Timer_VerSystem);
			// xSemaphoreGive(rdySem_Control_SEND_SMS_Task);
		}
	}
}

uint8_t has_letters(const char *str)
{

	while (*str)
	{
		if (isalpha(*str))
		{
			return 1; // A string contém pelo menos uma letra
		}
		str++;
	}
	return 0; // A string não contém letras
}

uint8_t checkIF_operator_sms(data_EG91_Receive_SMS receive_SMS_data)
{

	data_EG91_Send_SMS sms_Data;
	memset(&sms_Data, 0, sizeof(sms_Data));

	if (has_letters(receive_SMS_data.phNumber))
	{
		uint8_t redirectSMS_value = get_INT8_Data_From_Storage(NVS_REDIRECT_SMS, nvs_System_handle);

		if (redirectSMS_value == 255)
		{
			save_INT8_Data_In_Storage(NVS_REDIRECT_SMS, 0, nvs_System_handle);
		}

		if (redirectSMS_value == 1)
		{

			MyUser owner_Struct;
			memset(&owner_Struct, 0, sizeof(owner_Struct));
			char aux_Get_Data_User_str[200] = {};
			size_t required_size = 0;

			if (nvs_get_str(nvs_System_handle, NVS_KEY_OWNER_INFORMATION, NULL, &required_size) == ESP_OK)
			{
				// //printf("\nrequire size %d\n", required_size);
				// //printf("\nGET USERS NAMESPACE\n");
				if (nvs_get_str(nvs_System_handle, NVS_KEY_OWNER_INFORMATION, aux_Get_Data_User_str, &required_size) == ESP_OK)
				{
					// ////printf("\naux get str %s\n", aux_Get_Data_User_str);
					parse_ValidateData_User(aux_Get_Data_User_str, &owner_Struct);
					memset(sms_Data.phoneNumber, 0, sizeof(sms_Data.phoneNumber));
					sprintf(sms_Data.phoneNumber, "%s", owner_Struct.phone);

					sprintf(sms_Data.payload, "M200: %s\n%s", receive_SMS_data.phNumber, receive_SMS_data.receiveData);
					// //printf("\n\nowner phone %s\n - %s\n - %s\n - %s \n\n", aux_Get_Data_User_str, owner_Struct.phone, sms_Data.phoneNumber, sms_Data.payload);
					//  EG91_Send_SMS(sms_Data.phoneNumber, sms_Data.payload);
					xQueueSendToBack(queue_EG91_SendSMS, (void *)&sms_Data, pdMS_TO_TICKS(100));
					// vTaskDelay(pdMS_TO_TICKS(1000));
					xSemaphoreGive(rdySem_Control_SMS_Task);
					// free(rsp);
					//  ////printf("\n\n SMS RSP444\n\n");
					// free(SMS_InputData);
					// ESP_LOGI("TAG", "xPortGetFreeHeapSize  parseInputData2222: %d", xPortGetFreeHeapSize());
					// ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
					// ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
					// ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
					// verify_SMS_List();
					//  }
					return 1;
				}
			}
		}
	}

	return 0;
}

uint8_t parse_SMS_Payload(char *payload)
{
	data_EG91_Send_SMS sms_Data;
	data_EG91_Receive_SMS receive_SMS_data;

	memset(&receive_SMS_data, 0, sizeof(receive_SMS_data));
	memset(&sms_Data, 0, sizeof(sms_Data));

	int strIndex = 0;
	int count = 0;
	char payload_phNumber[21];
	char rsp[200];

	char *cpy_payload;
	char phNumber[21];
	char strDate[50];
	char strHour[50];
	char year[20];
	char month[3];
	char day[3];
	char hour[3];
	char minute[3];
	char payload1[200];
	char SMS_Status[100];
	// char receiveData[200];

	memset(phNumber, 0, sizeof(phNumber));
	memset(receive_SMS_data.receiveData, 0, strlen(receive_SMS_data.receiveData));
	memset(receive_SMS_data.strDate, 0, sizeof(strDate));
	memset(receive_SMS_data.strHour, 0, sizeof(strHour));
	memset(year, 0, sizeof(year));
	memset(month, 0, sizeof(month));
	memset(day, 0, sizeof(day));
	memset(hour, 0, sizeof(hour));

	////printf("\n\nsms strlen(payload) %s - %d\n\n", payload, strlen(payload));

	for (int i = 0; i < strlen(payload); i++)
	{
		if (payload[i] == '\n' || payload[i] == ',' || payload[i] == '\"')
		{
			count++;
			strIndex = 0;
		}
		else
		{
			if (count == 2)
			{
				receive_SMS_data.SMS_Status[strIndex] = payload[i];
				// ////printf("\nstrindex %d, %c , %c", strIndex, receive_SMS_data.SMS_Status[strIndex], payload[i]);
				strIndex++;
				receive_SMS_data.SMS_Status[strIndex] = 0;
			}

			if (count == 5)
			{
				if (strIndex < 20)
				{
				}
				receive_SMS_data.phNumber[strIndex] = payload[i];
				strIndex++;
				receive_SMS_data.phNumber[strIndex] = 0;
			}

			if (count == 9)
			{
				receive_SMS_data.strDate[strIndex] = payload[i];
				strIndex++;
			}
			if (count == 10)
			{
				receive_SMS_data.strHour[strIndex] = payload[i];
				strIndex++;
				receive_SMS_data.strHour[strIndex] = 0;
			}
			if (count == 21)
			{
				if (payload[i] == 13)
				{
					receive_SMS_data.receiveData[strIndex] = 0;
					break;
				}

				receive_SMS_data.receiveData[strIndex] = payload[i];
				// ////printf("\nstrindex %d, %c", strIndex, receiveData[strIndex]);
				strIndex++;

				receive_SMS_data.receiveData[strIndex] = 0;
			}
		}
	}

	// ////printf("\n\n phone number hex  %s \n\n", receive_SMS_data.phNumber);

	char dst1[strlen(receive_SMS_data.phNumber)];
	char dst2[5];
	char dst3[strlen(receive_SMS_data.receiveData)];

	if (strlen(receive_SMS_data.phNumber) > 6 && strlen(receive_SMS_data.receiveData) > 4)
	{

		for (int i = 0; i < strlen(receive_SMS_data.phNumber); i = i + 4)
		{
			char aux_UTF8[5] = {};
			memset(aux_UTF8, 0, 4);

			aux_UTF8[0] = receive_SMS_data.phNumber[i];
			aux_UTF8[1] = receive_SMS_data.phNumber[i + 1];
			aux_UTF8[2] = receive_SMS_data.phNumber[i + 2];
			aux_UTF8[3] = receive_SMS_data.phNumber[i + 3];
			aux_UTF8[4] = 0;
			utf8decode((utf8chr_t *)aux_UTF8, &dst2);

			if (i == 0)
			{
				strcpy(dst1, dst2);
			}
			else
			{
				strcat(dst1, dst2);
			}
		}

		// ////printf("\n\n dst1 after hex  %s \n\n", dst1);
		memset(receive_SMS_data.phNumber, 0, strlen(receive_SMS_data.phNumber));
		sprintf(receive_SMS_data.phNumber, "%s", dst1);
		// ////printf("\n\ndst1\n %s \n\n", receive_SMS_data.phNumber);

		for (int i = 0; i < strlen(receive_SMS_data.receiveData); i = i + 4)
		{
			char aux_UTF8[5] = {};
			memset(aux_UTF8, 0, 4);

			aux_UTF8[0] = receive_SMS_data.receiveData[i];
			aux_UTF8[1] = receive_SMS_data.receiveData[i + 1];
			aux_UTF8[2] = receive_SMS_data.receiveData[i + 2];
			aux_UTF8[3] = receive_SMS_data.receiveData[i + 3];
			aux_UTF8[4] = 0;
			utf8decode((utf8chr_t *)aux_UTF8, &dst2);

			if (i == 0)
			{
				strcpy(dst3, dst2);
			}
			else
			{
				strcat(dst3, dst2);
			}
		}
		////printf("\n\n dst3 after receiveData  %s \n\n", dst3);
		// ////printf("\n\n dst1 after receiveData  %s \n\n", dst1);

		memset(receive_SMS_data.receiveData, 0, strlen(receive_SMS_data.receiveData));
		sprintf(receive_SMS_data.receiveData, "%s", dst3);
		char input_First_ReceiveData[200] = {};
		// memset(input_First_ReceiveData, 0, strlen(receive_SMS_data.receiveData));

		if (checkIF_operator_sms(receive_SMS_data))
		{
			return 1;
		}

		for (size_t i = 0; i < strlen(receive_SMS_data.receiveData); i++)
		{
			if (receive_SMS_data.receiveData[i] == ' ')
			{
				break;
			}

			input_First_ReceiveData[i] = receive_SMS_data.receiveData[i];
		}

		// ////printf("\n\ndst3\n%s \n\n", receive_SMS_data.receiveData);
		// ////printf("\n\nFIRST \n%s \n\n", input_First_ReceiveData);
		// ////printf("\n\nabel_Reset_Password_OR_System - %d \n\n", label_Reset_Password_OR_System);

		count = 0;
		uint8_t stringIndex = 0;
		char input_Payload[100] = {};

		for (size_t i = 0; i < strlen(input_First_ReceiveData); i++)
		{
			input_First_ReceiveData[i] = toupper(input_First_ReceiveData[i]);
		}

		if (!strcmp("ME.S.O", input_First_ReceiveData))
		{
			// ////printf("\n\nadd owner sms\n\n");
			char aux_phoneNumber[20] = {};
			sprintf(sms_Data.phoneNumber, "%s", receive_SMS_data.phNumber);
			sprintf(aux_phoneNumber, "%s", check_IF_haveCountryCode(receive_SMS_data.phNumber, 1));

			sprintf(sms_Data.payload, "%s", MyUser_add_Owner(receive_SMS_data.receiveData, aux_phoneNumber, SMS_INDICATION));
			xSemaphoreGive(rdySem_Control_Send_AT_Command);
			xQueueSendToBack(queue_EG91_SendSMS, (void *)&sms_Data, pdMS_TO_TICKS(100));
			xSemaphoreGive(rdySem_Control_SMS_Task);

			return "NTRSP";
		}
		else if (!strcmp("ME.R.K", input_First_ReceiveData))
		{
			// ////printf("\n\nENTER CHANGE OWNER PASSWORD\n\n");

			for (size_t i = 0; i < strlen(receive_SMS_data.receiveData); i++)
			{
				if (receive_SMS_data.receiveData[i] == ' ')
				{
					count++;
					stringIndex = 0;
				}
				else
				{

					if (count == 1)
					{
						memcpy(input_Payload + stringIndex, receive_SMS_data.receiveData + i, 1);
						stringIndex++;
					}
				}
			}
			// system_stack_high_water_mark("parse sms 12");
			//  ////printf("\n\n\n sms input payload - %s\n\n", input_Payload);
			sprintf(sms_Data.phoneNumber, "%s", receive_SMS_data.phNumber);
			xSemaphoreGive(rdySem_Control_Send_AT_Command);
			reset_Owner_password(input_Payload, &sms_Data);
			xSemaphoreGive(rdySem_Control_SMS_Task);

			return "NTRSP";
		}

		int codepoint;

		if (lost_SMS_addUser.flag_Lost_SMS_Add_User == 1)
		{
			int dotCounter = 0;
			// lost_SMS_addUser.flag_Lost_SMS_Add_User = 0;

			char *pch;
			char auxPH1[100] = {};
			char auxPH2[100] = {};
			// ////printf("\n\nsms HELLOO 2\n\n");
			sprintf(auxPH1, "%s", check_IF_haveCountryCode(lost_SMS_addUser.phone, 0));
			// ////printf("\n\nsms HELLOO 3 - %s\n\n", auxPH1);
			sprintf(auxPH2, "%s", check_IF_haveCountryCode(receive_SMS_data.phNumber, 0));
			// ////printf("\n\nsms HELLOO 4 - %s\n\n", auxPH2);
			pch = strstr(auxPH1, auxPH2);

			uint8_t confSMS = get_INT8_Data_From_Storage(NVS_SMS_CALL_VERIFICATION, nvs_System_handle);

			if (confSMS == 255)
			{
				save_INT8_Data_In_Storage(NVS_SMS_CALL_VERIFICATION, 0, nvs_System_handle);
				confSMS = 0;
			};

			// ////printf("\n\nsms HELLOO 5\n\n");
			if (pch != NULL || confSMS == 1 || confSMS == 4)
			{
				strcat(lost_SMS_addUser.payload, dst3);
				int dotCount = 0;
				// ////printf("\n\nsms HELLOO 5\n\n");
				// ////printf("\n\nsms HELLOO 6 - %s\n\n", lost_SMS_addUser.payload);
				for (size_t i = 0; i < strlen(lost_SMS_addUser.payload); i++)
				{
					if (lost_SMS_addUser.payload[i] == '.')
					{
						dotCount++;
					}

					if (dotCount == 9)
					{
						lost_SMS_addUser.payload[i + 2] = 0;
						break;
					}
				}

				memset(dst2, 0, 5);
				/* memset(receive_SMS_data.phNumber, 0, strlen(receive_SMS_data.phNumber));
				sprintf(receive_SMS_data.phNumber, "%s", dst1); */
				// ////printf("\n\ndst1111\n %s \n\n", receive_SMS_data.phNumber);

				/* memset(receive_SMS_data.receiveData, 0, strlen(receive_SMS_data.receiveData));
				sprintf(receive_SMS_data.receiveData, "%s", lost_SMS_addUser.payload); */
				// ////printf("\n\ndst33333\n %s \n\n", receive_SMS_data.receiveData);
				// ////printf("\n\n\nlost_SMS_addUser.payload %s \n\n", lost_SMS_addUser.payload);
			}
			// ////printf("\n\nsms HELLOO 8\n\n");
		}

		parse_SMS_data(&receive_SMS_data);
		return 1;
	}

	xSemaphoreGive(rdySem_Control_SMS_Task);
	// }

	// free(payload_phNumber);
	// free(receiveData);
	return 1;
}

uint8_t parseQMTPUBEX(char *receiveData)
{
	// ////printf("\n\n PUBX12345 string %d\n\n", strlen(receiveData));
	//  uint8_t index_Found = strpos(receiveData, "+QMTPUBEX:");
	//  ////printf("\n\n PUBX12345 indexfound - %d\n\n",index_Found);
	uint8_t counter = 0;
	char result = 0;
	////printf("\n\npubx receive data\n");
	for (size_t i = 0; i < strlen(receiveData); i++)
	{
		// //printf("\n\nPUBXID - %d\n\n", i);
		// ////printf("%c", receiveData[i]);
		if (receiveData[i] == ',')
		{
			counter++;
		}

		if (counter == 2)
		{
			result = receiveData[i + 1];
			break;
		}
	}

	// ////printf("\n\nPUBX54321\n\n");free heap memory
	////printf("\n\n finish pubx receive data - %c\n", result);
	ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d", xPortGetFreeHeapSize());
	ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
	ESP_LOGI("TAG", "heap_caps_get_free_size         : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	if (result == '0' || result == '1')
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t EG91_parseGet_imsi(char *data)
{
	/* char imsi[30] = {};
	uint8_t strIndex = 0;

	for (size_t i = 0; i < strlen(data); i++)
	{
		if (isdigit(data[i]))
		{
			imsi[strIndex++] = data[i];
		}
	}

	////printf("\n\n imsi - %s\n\n", imsi);
	if (strIndex == 14 || strIndex == 15)
	{
		save_STR_Data_In_Storage(NVS_EG91_ICCID_VALUE, imsi, nvs_System_handle);
		return 1;
	} */

	return 0;
}

uint8_t parse_https_get(char *payload)
{
	char dtmp1[50] = {};

	////printf("\n\n parse http get222\n\n");
	/* if (strstr(payload,"OK") != NULL)
	{ */
	xQueueReceive(HTTPS_data_queue, &dtmp1, pdMS_TO_TICKS(10000));
	////printf("\n\n parse http get\n\n");
	if (strstr(dtmp1, "+QHTTPGET:") != NULL)
	{
		char *token;

		// Ignora o prefixo "+QHTTPGET: "
		token = strtok(dtmp1, ": ");
		token = strtok(NULL, ","); // status
		int x = atoi(token);

		token = strtok(NULL, ","); // length
		EG915_readDataFile_struct.status = atoi(token);

		token = strtok(NULL, ","); // timestamp
		EG915_readDataFile_struct.fileSize = atoi(token);

		////printf("\n\n get https %d - %d - %d\n\n", x, EG915_readDataFile_struct.status, EG915_readDataFile_struct.fileSize);

		return 1;
	}
	else
	{
		////printf("\n\n parse http get22\n\n");
	}

	return 0;
}

uint8_t EG91_send_AT_Command(char *data, char *rsp, int time)
{

	char AT_Command[300] = {};
	uint8_t counterACK = 5;
	char dtmp1[BUF_SIZE] = {};
	char phone[250] = {};
	char text[1024] = {};

	// ////printf("\nAT COMMAND 000  %s\n", data);

	printf("\nAT COMMAND = %s\n", data);

	////printf("\nAT COMMAND\n");

	xSemaphoreTake(rdySem_Control_Send_AT_Command, pdMS_TO_TICKS(15000));
	////printf("\nAFTER TAKE SEND AT COMAND\n");
	/* ////printf("\nAFTER TAKE SEND AT COMAND11\n");
	//////printf("\nAFTER TAKE SEND AT COMAND22\n"); */
	// //printf("\n\n at sender1 - %s\n\n", data);
	send_ATCommand_Label = 1;
	// system_stack_high_water_mark("atsend1");
	//  //printf("\nAFTER TAKE SEND AT COMAND111 %d\n", strlen(data));
	if (!strcmp(rsp, "QMTPUBEX"))
	{

		char UDP_send_command[1024] = {};
		char ct[2] = {};
		ct[0] = 13;
		ct[1] = 0;

		char mqtt_Data[1024] = {};
		char topic_mqtt[70] = {};
		uint8_t count = 0;
		int strIndex = 0;
		////printf("\nAT COMMAND 445655\n");

		// //printf("\nAFTER TAKE SEND AT COMAND222 %d\n", (data != NULL) ? strlen(data) : 0);
		// //printf("\nAFTER TAKE SEND AT COMAND222 %d\n", strlen(data));
		for (size_t i = 0; i < strlen(data); i++)
		{
			if (data[i] == '$')
			{
				count++;
				strIndex = 0;
			}
			else if (count == 0)
			{
				mqtt_Data[strIndex++] = data[i];
			}
			else if (count == 1)
			{
				topic_mqtt[strIndex++] = data[i];
			}
		}
		////printf("\nAFTER TAKE SEND AT COMAND333\n");
		// //printf("\n\n at sender2 - %s - %s\n\n", mqtt_Data, topic_mqtt);
		/* if (strlen(topic_mqtt) > 0)
		{ */
		// printf("\nAFTER TAKE SEND AT COMAND444\n");
		sprintf(UDP_send_command, "%s%s%s%d", "AT+QMTPUBEX=0,0,1,0,\"m200", topic_mqtt, "\",", strlen(mqtt_Data));

		// printf("\nAFTER TAKE SEND AT COMAND5555\n");
		/*  }
		 else
		 {
			 sprintf(UDP_send_command, "%s%d", "AT+QMTPUBEX=0,0,1,0,\"m200/\",", strlen(mqtt_Data));
		 } */

		// printf("\n\n PUBX0101010 - %s\n\n", UDP_send_command);
		//  xSemaphoreGive(rdySem_Control_Send_AT_Command);

		/*  */
		strcat(UDP_send_command, ct);

		uart_write_bytes(UART_NUM_1, UDP_send_command, strlen(UDP_send_command));
		vTaskDelay(pdMS_TO_TICKS(50));
		// system_stack_high_water_mark("SEND UDP3");
		// printf("\n\n at sender3 - %s\n\n", mqtt_Data);
		uart_write_bytes(UART_NUM_1, mqtt_Data, strlen(mqtt_Data));
		// sprintf(AT_Command, "%s%c", "AT", 13);
		// uart_write_bytes(UART_NUM_1,AT_Command , strlen(AT_Command));
		//  vTaskDelay(pdMS_TO_TICKS(500));
		// ////printf("\n\n PUBX12345 - %s\n\n", data);
		xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(time));
		// printf("\n\n at sender4 - %s\n\n", data);

		// ////printf("\n\n PUBX0000 - %s\n\n", dtmp1);
		// ////printf("\n\n PUBX12345\n\n");
		//  ////printf("\n\n PUBX: %s \n\n", dtmp1);
		send_ATCommand_Label = 0;
		// uint8_t ack_pubex =
		if (parseQMTPUBEX(dtmp1))
		{
			// xSemaphoreGive(rdySem_Control_pubx);
			xSemaphoreGive(rdySem_Control_Send_AT_Command);
			return 1;
		}
		else
		{
			// xSemaphoreGive(rdySem_Control_pubx);
			xSemaphoreGive(rdySem_Control_Send_AT_Command);
			return 0;
		}

		return 1;
	}

	if (!strcmp(rsp, "CMGS:"))
	{
		// ////printf("\n SMS SEND START1233 - %s\n", data);
		int strIndex = 0;
		int count = 0;

		for (int i = 0; i < strlen(data); i++)
		{

			if (data[i] == '$')
			{
				count++;
				strIndex = 0;
			}
			else
			{
				if (count == 0)
				{
					text[strIndex++] = data[i];
				}
				else if (count == 1)
				{
					// sprintf((char*)phone + strIndex, "%02X", data[i]);
					phone[strIndex++] = data[i];
					// strIndex++;
				}
				/* else
				{
					return 0;
				} */
			}
		}
		char ct[2] = {};
		ct[0] = 26;
		ct[1] = 0;
		strcat(text, ct);
		// ////printf("\n rsp rsp11 - %s - %s\n\n", phone, text);
	}

	// ////printf("\n SMS SEND START2112 -\n");
	// system_stack_high_water_mark("atsend11");
	while (counterACK > 0)
	{
		memset(dtmp1, 0, sizeof(dtmp1));
		// ////printf("\n SMS SEND START12 -\n");
		if (!strcmp(rsp, "CMGS:"))
		{

			// ////printf("\n SMS - %s\n - %s\n", phone, text);
			//   memset(rsp,0,sizeof(rsp));
			//   strcpy(rsp,"OK");
			//   uart_write_bytes(UART_NUM_1, "AT0", strlen(phone));

			uart_write_bytes(UART_NUM_1, phone, strlen(phone));
			xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(1500));

			// ////printf("\ndtmp1 11 %s\n", dtmp1);
			//   xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(time * 5));
			memset(dtmp1, 0, sizeof(dtmp1));
			uart_write_bytes(UART_NUM_1, text, sizeof(text) /* strlen(text) */);

			xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(3000));
			// ////printf("\ndtmp1 11 %s\n", dtmp1);
			//   xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(time * 5));
			memset(dtmp1, 0, sizeof(dtmp1));

			/* uart_wait_tx_done(UART_NUM_1, pdMS_TO_TICKS(1000));
			uart_write_bytes_with_break(UART_NUM_1, text, strlen(text), pdMS_TO_TICKS(1000)); */
			// xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(time * 2));

			// vTaskDelay(pdMS_TO_TICKS((500)));
			// ////printf("\n SMS 1 - %s \n - %s\n", phone, text);
			xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(3000));
			// ////printf("\ndtmp1 22 %s\n", dtmp1);
		}
		else
		{
			sprintf(AT_Command, "%s%c", data, 13);
			// //printf("\nSEND AT COMMAND 11 = %s\n", AT_Command);

			/* uart_wait_tx_done(UART_NUM_1, pdMS_TO_TICKS(1000));
			uart_write_bytes_with_break(UART_NUM_1, AT_Command, strlen(AT_Command), pdMS_TO_TICKS(1000)); */
			// system_stack_high_water_mark("atsend12");
			uart_write_bytes(UART_NUM_1, AT_Command, strlen(AT_Command));

			/* if (!strcmp("AT+CNUM",data))
			{
				////printf("\n parse data %d\n", 3);
				vTaskDelay(pdMS_TO_TICKS(300));
			} */

			xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(time));

			// //printf("\nSEND AT COMMAND 22 = %s\n", dtmp1);
		}
		// ////printf("\n SMS SEND START123 -\n");
		//(TickType_t)(time / portTICK_PERIOD_MS)
		//  ////printf("\nAT_Command_Feedback_queue = %s\n", dtmp1);
		// system_stack_high_water_mark("atsend22");
		if (strlen(dtmp1) < 1 || strstr(dtmp1, "CME ERROR") != NULL || !strcmp(rsp, "SMS"))
		{
			/* uart_event_t uartEvent;
			uartEvent.type = UART_DATA;
			uartEvent.size=200; */

			memset(dtmp1, 0, sizeof(dtmp1));
			// ////printf("\n parse data error %d\n", 3);
			memset(AT_Command, 0, sizeof(AT_Command));

			xSemaphoreGive(rdySem_Control_Send_AT_Command);
			// ////printf("\n parse data %d\n", 1);
			send_ATCommand_Label = 0;
			return 0;
		}

		// ////printf("\n SMS SEND START124 -\n");
		if (EG91_Parse_ReceiveData(dtmp1, rsp))
		{
			// free(rsp);
			//  free(&dtmp1);
			// ////printf("\n parse data %d\n", 1);

			// ////printf("\n parse data %d\n", 2);
			memset(dtmp1, 0, sizeof(dtmp1));
			// ////printf("\n parse data %d\n", 3);
			memset(AT_Command, 0, sizeof(AT_Command));

			// free(dtmp1);
			//  xQueueReset(AT_Command_Feedback_queue);
			xSemaphoreGive(rdySem_Control_Send_AT_Command);
			// ////printf("\n parse data %d\n", 4);
			send_ATCommand_Label = 0;
			return 1;
		}
		else
		{
			// xSemaphoreGive(rdySem_Control_Send_AT_Command);

			memset(dtmp1, 0, sizeof(dtmp1));
			counterACK--;
			// ////printf("\n parse data %d\n", 5);
			if (!strcmp(rsp, "CPIN"))
			{
				break;
			}
		}

		// xSemaphoreGive(rdySem_Control_Send_AT_Command);
		memset(dtmp1, 0, sizeof(dtmp1));
		vTaskDelay(pdMS_TO_TICKS((1000)));
	}

	// free(data);
	// free(rsp);
	send_ATCommand_Label = 0;

	// ////printf("\n parse data %d\n", 5);
	//   memset(dtmp1, 0, sizeof(dtmp1));
	xSemaphoreGive(rdySem_Control_Send_AT_Command);
	return 0;
}

uint8_t EG91_parse_CNUM(char *receiveData)
{
	uint8_t strIndex = 0;
	uint8_t count = 0;
	char *ownNumber = (char *)malloc(strlen(receiveData) * sizeof(char));

	memset(ownNumber, 0, strlen(receiveData) * sizeof(char));

	for (size_t i = 0; i < strlen(receiveData); i++)
	{
		if (receiveData[i] == '"')
		{
			count++;
			strIndex = 0;
		}
		else
		{
			if (count == 3)
			{
				ownNumber[strIndex++] = receiveData[i];
			}
		}
	}

	save_STR_Data_In_Storage(NVS_KEY_OWN_NUMBER, ownNumber, nvs_System_handle);
	free(ownNumber);
	// ////printf("\n\n own number %s\n\n", ownNumber);

	return 1;
}

uint8_t parse_qmtsub(char *receiveData)
{
	uint8_t counter = 0;
	char result = 0;

	for (size_t i = 0; i < strlen(receiveData); i++)
	{
		if (receiveData[i] == ',')
		{
			counter++;
			if (counter == 2)
			{
				result = receiveData[i + 1];
				break;
			}
		}
	}

	// ////printf("\n\n subResult %c\n\n", result);

	if (result == '0')
	{
		return 1;
	}
	else
	{
		return 0;
	}

	return 0;
}

uint8_t init_UDP_socket() /* 83.240.211.213 */
{

	char imei[100] = {};
	char *connectData;
	uint8_t ack = 0;
	// ////printf("\n\ntask_refresh_SystemTime 02\n\n");
	EG91_get_IMEI();

	get_STR_Data_In_Storage(NVS_KEY_EG91_IMEI, nvs_System_handle, &imei);

	size_t plaintext_len = strlen(imei);

	char *encrypImei = encrypt_and_base64_encode((unsigned char *)AES_KEY1, (unsigned char *)imei, plaintext_len);
	char *willMsg;

	asprintf(&willMsg, "%s !", imei);

	char *willMsg_encrypt;
	asprintf(&willMsg_encrypt, "%s", encrypt_and_base64_encode((unsigned char *)AES_KEY1, (unsigned char *)willMsg, strlen(willMsg)));

	char *willStr;

	asprintf(&willStr, "AT+QMTCFG=\"will\",0,1,1,0,\"m200/\",\"%s\"", willMsg_encrypt);

	EG91_send_AT_Command(willStr, "OK", 1000);

	free(willMsg_encrypt);
	free(willMsg);
	free(willStr);

	EG91_send_AT_Command("AT+QMTCFG=\"keepalive\",0,1800", "OK", 3000);
	EG91_send_AT_Command("AT+QMTCFG=\"recv/mode\",0,0,1", "OK", 3000);

	// EG91_send_AT_Command(" AT+QMTCFG=\"will\",0,1,1,0,m200/,!", "OK", 1000);

	EG91_send_AT_Command("AT+QMTOPEN?", "AT+QMTOPEN?", 5000);
	// ////printf("\n\n OPEN VERIFY24352! - %d \n\n", mqtt_openLabel);

	if (!mqtt_openLabel)
	{
		// ////printf("\n\n OPEN VERIFY555! \n\n");

#ifndef CONFIG_SECURE_FLASH_ENCRYPTION_MODE_DEVELOPMENT
		if (EG91_send_AT_Command("AT+QMTOPEN=0,\"api.portal.motorline.pt\",8883", "OK", 10000))
		{

#else

		/* if (EG91_send_AT_Command("AT+QMTOPEN=0,\"test.mosquitto.org\",8883", "OK", 10000))
		{ */
		if (EG91_send_AT_Command("AT+QMTOPEN=0,\"api-portal.home.motorline.pt\",8883", "OK", 10000))
		{

#endif // Seleção de DNS

			uint8_t ACK_connect = 0;
			// ////printf("\n\n OPEN VERIFY! \n\n");
			while (ACK_connect < 10)
			{
				EG91_send_AT_Command("AT+QMTOPEN?", "AT+QMTOPEN?", 2000);
				if (mqtt_openLabel)
				{
					ack++;
					break;
				}
				ACK_connect++;
				vTaskDelay(pdMS_TO_TICKS(1000));
			}

			if (mqtt_openLabel)
			{
				ACK_connect = 0;
				EG91_send_AT_Command("AT+QMTCONN?", "AT+QMTCONN?", 1000);

				if (!mqtt_connectLabel)
				{

					asprintf(&connectData, "AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"", imei, imei, encrypImei);

					if (EG91_send_AT_Command(connectData, "OK", 20000))
					{
						uint8_t ACK_connect = 0;
						// ////printf("\n\n CONNECT VERIFY! \n\n");
						while (ACK_connect < 10)
						{
							EG91_send_AT_Command("AT+QMTCONN?", "AT+QMTCONN?", 1000);

							if (mqtt_connectLabel)
							{
								ack++;
								break;
							}
							ACK_connect++;
							vTaskDelay(pdMS_TO_TICKS(1000));
						}
					}
					else
					{
						// TODO: IMPLENTAR MANTER O STAT
						EG91_send_AT_Command("AT+QMTDISC=0", "OK", 30000);
					}
				}
				else
				{
					ack++;
				}
			}
			else
			{
				// TODO: IMPLENTAR MANTER O STAT
			}
		}
		else
		{
			// TODO: IMPLENTAR MANTER O STAT
		}
	}
	else
	{
		// ////printf("\n\n OPEN VERIFy888888! \n\n");
		ack++;

		EG91_send_AT_Command("AT+QMTCONN?", "AT+QMTCONN?", 1000);

		if (!mqtt_connectLabel)
		{

			asprintf(&connectData, "AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"", imei, imei, encrypImei);

			if (EG91_send_AT_Command(connectData, "OK", 20000))
			{
				uint8_t ACK_connect = 0;
				// ////printf("\n\n CONNECT VERIFY! \n\n");
				while (ACK_connect < 10)
				{
					EG91_send_AT_Command("AT+QMTCONN?", "AT+QMTCONN?", 1000);

					if (mqtt_connectLabel)
					{
						ack++;
						break;
					}
					ACK_connect++;
					vTaskDelay(pdMS_TO_TICKS(1000));
				}
			}
			else
			{
				// TODO: IMPLENTAR MANTER O STAT
				EG91_send_AT_Command("AT+QMTDISC=0", "OK", 20000);
			}
		}
		else
		{
			ack++;
		}
	}

	memset(connectData, 0, strlen(connectData));
	asprintf(&connectData, "AT+QMTSUB=0,1,\"set/m200/%s/+\",1", imei);
	EG91_send_AT_Command(connectData, "+QMTSUB:", 10000);
	// printf("\n\n finish sub1\n\n");
	//  EG91_send_AT_Command("AT+QICFG=\"send/auto\",2,5,\"Dados automáticos\"", "OK", 1000);

	free(connectData);
	// printf("\n\n finish sub2\n\n");
	free(encrypImei);
	// printf("\n\n finish sub3\n\n");
	return ack;
}

uint8_t parse_qmtconn(char *receiveData)
{
	char conn_verify = 0;

	for (size_t i = 0; i < strlen(receiveData); i++)
	{
		if (receiveData[i] == ',')
		{
			conn_verify = receiveData[i + 1];
			break;
		}
	}

	// ////printf("\n\nconn_verify %c \n\n", conn_verify);

	if (conn_verify == '3')
	{
		mqtt_connectLabel = 1;
		save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 0, nvs_System_handle);
	}
	else
	{
		mqtt_connectLabel = 0;
		save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 1, nvs_System_handle);
	}

	// ////printf("\n\nconn_verify2 %c \n\n", conn_verify);
	return 1;
}
char output_mqtt_data[300];
uint8_t send_UDP_Package(char *data, int size, char *topic)
{
	char UDP_send_command[1024] = {};
	char imei[20] = {}; // NULL;
	size_t required_size;
	// printf("\nsms ghghkk 532523\n");

	if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, NULL, &required_size) == ESP_OK)
	{
		// printf("\n\n send udp 01 - %d \n\n", required_size);

		if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, imei, &required_size) == ESP_OK)
		{
			sprintf(UDP_send_command, "%s ", imei);
			// printf("\n\n send udp 2 \n\n");
			strcat(UDP_send_command, data);
			// printf("\n\n send udp 3 - %s\n\n", UDP_send_command);
			//  ////printf("\nUDP_send_command:454546 %s\n", "base64_str");
		}
		else
		{
			EG91_get_IMEI();

			if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, NULL, &required_size) == ESP_OK)
			{
				// ////printf("\n\n send udp 01 - %d \n\n", required_size);

				if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, imei, &required_size) == ESP_OK)
				{
					sprintf(UDP_send_command, "%s ", imei);
					// ////printf("\n\n send udp 2 \n\n");
					strcat(UDP_send_command, data);
				}
			}
			else
			{
				// ////printf("\n\nERROR 1\n\n");
				sprintf(UDP_send_command, "%s ", "ERROR");
				strcat(UDP_send_command, data);
			}
		}
	}
	else
	{
		EG91_get_IMEI();

		if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, NULL, &required_size) == ESP_OK)
		{
			// ////printf("\n\n send udp 01 - %d \n\n", required_size);

			if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, imei, &required_size) == ESP_OK)
			{
				sprintf(UDP_send_command, "%s ", imei);
				// ////printf("\n\n send udp 2 \n\n");
				strcat(UDP_send_command, data);
				// ////printf("\n\n send udp 3 \n\n");
			}
		}
		else
		{
			// ////printf("\n\nERROR 1\n\n");
			sprintf(UDP_send_command, "%s ", "ERROR");
			strcat(UDP_send_command, data);
		}
	}
	// system_stack_high_water_mark("SEND UDP4");
	vTaskDelay(10);
	// printf("\n\nUDP_send_commandçç - %s\n\n", UDP_send_command);
	size_t plaintext_len = strlen(UDP_send_command);

	char *base64_str = encrypt_and_base64_encode((unsigned char *)AES_KEY1, (unsigned char *)UDP_send_command, plaintext_len);

	// printf("\n\nbase64: %s\n\n", base64_str);

	/*   if (base64_str != NULL)
	  {
		  // ////printf("\n\nbase64: %s\n\n", base64_str);
		  //   Liberar a memória alocada para a string retornada
	  }
	  else
	  {
		  // ////printf("\n\nErro ao criptografar e codificar em base64.\n\n");
	  } */

	// char *output_mqtt_data;

	if (strlen(topic) < 2)
	{
		sprintf(output_mqtt_data, "%s$/%s%c", base64_str, imei, 0);
	}
	else
	{
		sprintf(output_mqtt_data, "%s$%s%c", base64_str, topic, 0);
	}

	// asprintf(&output_mqtt_data, "%s$%s", base64_str, topic);

	// printf("\n\n send udp 66 %s - %d\n\n", output_mqtt_data, strlen(output_mqtt_data));
	char fff[1000] = {}; //
	// vTaskList(fff); vTaskGetRunTimeStats(fff); //printf("\n%s\n",fff);
	timer_pause(TIMER_GROUP_1, TIMER_0);
	// printf("\n\n send udp 0101010 \n\n");
	if (EG91_send_AT_Command(output_mqtt_data, "QMTPUBEX", 2000))
	{
		// free(output_mqtt_data);
		free(base64_str);
		timer_start(TIMER_GROUP_1, TIMER_0);
		return 1;
	}
	else
	{
		// free(output_mqtt_data);
		free(base64_str);
		timer_start(TIMER_GROUP_1, TIMER_0);
		return 0;
	}
}

void register_UDP_Device()
{

	char registerData[30] = {};
	char ownerNumber[20] = {};
	char ownerPassword[7] = {};
	size_t required_size = 0;
	char owner_Data[200] = {};
	MyUser user_validateData;

	memset(&user_validateData, 0, sizeof(user_validateData));

	if (nvs_get_str(nvs_System_handle, NVS_KEY_OWNER_INFORMATION, NULL, &required_size) == ESP_OK)
	{
		// printf("\nrequire size %d\n", required_size);
		//  ////printf("\nGET OWNER NAMESPACE\n");
		if (nvs_get_str(nvs_System_handle, NVS_KEY_OWNER_INFORMATION, owner_Data, &required_size) == ESP_OK)
		{

			char *data_register;
			char imsi[16] = {};
			char input_data[100];

			get_STR_Data_In_Storage(NVS_EG91_ICCID_VALUE, nvs_System_handle, &EG91_ICCID_Number);
			sprintf(input_data, "%c %s", '[', EG91_ICCID_Number);
			send_UDP_Send(input_data, "");
			memset(input_data, 0, 100);

			parse_ValidateData_User(owner_Data, &user_validateData);
			sprintf(input_data, "%s %s ME.G.I", user_validateData.phone, user_validateData.key);
			// printf("\nrequire size33 %d\n", required_size);
			data_register = parseInputData(&input_data, BLE_INDICATION, NULL, NULL, NULL, NULL, NULL);
			// printf("\nrequire size44 %d\n", required_size);
			send_UDP_Send(data_register, "");
			// printf("\nrequire size55 %d\n", required_size);
			//  send_UDP_queue(data_register);
			memset(input_data, 0, 100);

			

			sprintf(input_data, "%s %s ME.G.M", user_validateData.phone, user_validateData.key);
			data_register = parseInputData(&input_data, UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);
			send_UDP_Send(data_register, "");
			// printf("\nrequire size44 bbb\n");
			//  send_UDP_queue(data_register);
			memset(input_data, 0, 100);

			// printf("\nrequire size44 aaa\n");

			uint8_t label_UDP_fail_and_changed = get_INT8_Data_From_Storage(NVS_NETWORK_LOCAL_CHANGED, nvs_System_handle);
			// ////printf("\n\nlabel_UDP_fail_and_changed trete %d\n\n", label_UDP_fail_and_changed);
			if ((label_UDP_fail_and_changed & 1) || label_UDP_fail_and_changed == -1)
			{
				// printf("\nrequire size44 vvv\n");
				sprintf(input_data, "%s %s RT.G.T", user_validateData.phone, user_validateData.key);
				data_register = parseInputData(&input_data, UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);
				send_UDP_Send(data_register, "");
				// send_UDP_queue(data_register);
				memset(input_data, 0, 100);
				// ////printf("\n\nlabel_UDP_fail_and_changed trete 12122 %d\n\n", label_UDP_fail_and_changed);

				send_UDP_Send("RT R R", "");
				send_UDP_Send("RT R D", "");

				sprintf(input_data, "%s %s RT.G.D", user_validateData.phone, user_validateData.key);
				data_register = parseInputData(&input_data, UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);
				send_UDP_Send(data_register, "");
				// send_UDP_queue(data_register);
				memset(input_data, 0, 100);

				sprintf(input_data, "%s %s RT.G.H", user_validateData.phone, user_validateData.key);
				data_register = parseInputData(&input_data, UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);

				send_UDP_Send(data_register, "");
				// send_UDP_queue(data_register);
				memset(input_data, 0, 100);

				sprintf(input_data, "%s %s RT.G.F", user_validateData.phone, user_validateData.key);
				data_register = parseInputData(&input_data, UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);
				send_UDP_Send(data_register, "");
				// send_UDP_queue(data_register);
				memset(input_data, 0, 100);
				// printf("\n\nlabel_UDP_fail_and_changed trete 12122 \n\n");
				//  send_UDP_queue(data_register);
				memset(input_data, 0, 100);

				if (send_udp_routines_funtion())
				{
					label_UDP_fail_and_changed = label_UDP_fail_and_changed & 254;
					save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED, label_UDP_fail_and_changed, nvs_System_handle);
				}
				else
				{
					label_UDP_fail_and_changed |= 1;
					save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED, label_UDP_fail_and_changed, nvs_System_handle);
				}
			}

			// printf("\n\nlabel_UDP_fail_and_changed tqwe %d\n\n", label_UDP_fail_and_changed);
			//  TODO: DESCOMENTAR
			if (label_UDP_fail_and_changed & 2 || label_UDP_fail_and_changed == -1)
			{
				// printf("\nrequire size44 rrrr\n");

				if (!send_UDP_Send("*\0", ""))
				{
					// printf("\n\n\n send udp ççççç \n\n\n");

					return 0;
				}

				if (sendUDP_all_User_funtion())
				{
					label_UDP_fail_and_changed = label_UDP_fail_and_changed & 253;
					save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED, label_UDP_fail_and_changed, nvs_System_handle);
					free(data_register);
				}
				else
				{
					// printf("\nrequire size44 121212\n");
					label_UDP_fail_and_changed |= 2;
					save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED, label_UDP_fail_and_changed, nvs_System_handle);
					free(data_register);
				}
			}
			// printf("\nrequire size44 iiii\n");
			if (label_UDP_fail_and_changed & 4 || label_UDP_fail_and_changed == -1)
			{
				memset(input_data, 0, 100);
				sprintf(input_data, "ME R K %s;%s", user_validateData.phone, user_validateData.key);
				// printf("\nrequire size44 767676\n");
				if (send_UDP_Send(input_data, ""))
				{
					label_UDP_fail_and_changed = label_UDP_fail_and_changed & 251;
					save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED, label_UDP_fail_and_changed, nvs_System_handle);
				}
				else
				{
					label_UDP_fail_and_changed |= 4;
					save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED, label_UDP_fail_and_changed, nvs_System_handle);
				}
			}
			// printf("\nrequire size44 65456\n");
		}
	}
}

uint8_t reopen_and_connection()
{
	if (EG91_send_AT_Command("AT+QMTCLOSE=0", "OK", 1000))
	{
		if (init_UDP_socket() == 2)
		{

			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	return 0;
}

uint8_t EG91_initNetwork()
{
	uint8_t ACK = 0;

	if (EG91_send_AT_Command("AT", "OK", 1000))
	{
		ACK++;
	}

	if (EG91_send_AT_Command("ATV1", "OK", 1000))
	{
		ACK++;
	}

	if (EG91_send_AT_Command("ATE1", "OK", 1000))
	{
		ACK++;
	}

	if (EG91_send_AT_Command("AT+GMR", "OK", 1000))
	{
		ACK++;
	}

	// EG91_send_AT_Command("AT+QCFG=0", "OK", 1000);

	// EG91_get_IMEI("asas");

	if (EG91_Check_IF_Have_PIN())
	{

		EG91_send_AT_Command("AT+QCCID", "+QCCID", 1000);
		// EG91_send_AT_Command("AT+CLCK=\"SC\",0,\"1111\"", "OK", 1000);tnf.m2m

		EG91_send_AT_Command(" AT+QCFG=\"nwscanmode\",0", "OK", 1000);

		if (EG91_send_AT_Command("AT+CREG?", "AT+CREG", 1000))
		{
			ACK++;
		}

		if (EG91_send_AT_Command("AT+QLTS=1", "QLTS", 3000))
		{
			ACK++;
		}

		if (EG91_send_AT_Command("AT+QCFG=\"urc/ri/ring\",\"off\"", "OK", 1000))
		{
			ACK++;
		}

		/* if (EG91_send_AT_Command("AT+COPS=3,0", "OK", 5000))
		{
			ACK++;
		} */

		if (EG91_send_AT_Command("AT+COPS?", "OK", 5000))
		{
			ACK++;
		}

		if (EG91_send_AT_Command("AT+QURCCFG=\"urcport\",\"uart1\"", "OK", 1000))
		{
			ACK++;
		}

		if (EG91_send_AT_Command("AT+QINDCFG=\"ring\", 1", "OK", 1000))
		{
			ACK++;
		}

		if (EG91_send_AT_Command("AT+CNMI=2,1,0,0,0", "OK", 1000))
		{
			ACK++;
		}

		// EG91_send_AT_Command("AT+QCFG=\"urc/ri/ring\"", "OK", 1000);
		EG91_send_AT_Command("AT+CNUM", "OK", 10000);

		vTaskDelay(pdMS_TO_TICKS(500));

		EG91_send_AT_Command("AT+CGATT=1", "OK", 1000);

		EG91_send_AT_Command("AT+CMEE=1", "OK", 1000);
		EG91_send_AT_Command("ATX", "OK", 1000);
		EG91_send_AT_Command("ATX4", "OK", 1000);

		EG91_send_AT_Command("ATS7=5", "OK", 1000);

		EG91_send_AT_Command("ATS0?", "OK", 1000);

		/* EG91_send_AT_Command("AT+CGSMS?", "OK", 1000);
		EG91_send_AT_Command("AT+CGSMS=1", "OK", 1000);
		EG91_send_AT_Command("AT+CGSMS?", "OK", 1000); */

		EG91_send_AT_Command("AT+QCFG=\"urc/ri/other\",\"off\"", "OK", 1000);

		// EG91_send_AT_Command("AT+QCFG=\"risignaltype\",\"respective\"", "OK", 1000);
		EG91_send_AT_Command("AT+QCFG=\"urc/ri/ring\"", "OK", 1000);

		// EG91_send_AT_Command("AT+QCFG=\"urc/ri/ring\",\"pulse\",1,1", "OK", 1000);

		/* EG91_send_AT_Command("AT+QCFG=\"urc/ri/smsincoming\"", "OK", 1000);
		EG91_send_AT_Command("AT+QCFG=\"urc/ri/smsincoming\",\"off\"", "OK", 1000); */

		// EG91_send_AT_Command("ATX0", "OK", 1000);
		//  EG91_send_AT_Command("ATO", "OK", 1000);

		EG91_send_AT_Command("AT+CMGD=1,4", "OK", 1000);
		EG91_send_AT_Command("AT+CPMS?", "OK", 1000);

		EG91_send_AT_Command("AT+CGDCONT= 1,\"IP\",\"\",\"0.0.0.0\",0,0", "OK", 1000);

		/* EG91_send_AT_Command("AT+QCFG=\"ims\",1", "OK", 1000);
		EG91_send_AT_Command("AT+QCFG=\"ims\"", "OK", 1000); */
		// EG91_send_AT_Command("AT+QCFG=\"ims\",1", "OK", 1000);

		EG91_send_AT_Command("AT+CGACT=1,1", "OK", 1000);
		// EG91_send_AT_Command("AT+QIACT=1", "OK", 1000);
		EG91_send_AT_Command("AT+QIACT?", "OK", 1000);
		EG91_send_AT_Command("AT+CGCONTRDP=1", "OK", 1000);
		/* for (size_t i = 0; i < 15; i++)
		{
			//////printf("%X\n", registerDevice[i]);
			// returnData[i]
		} */

		// ////printf("\n\nIMEI\n%s\n", auxIMEI);

		if (EG91_send_AT_Command("AT+CSCS=\"UCS2\"", "OK", 1000))
		{
			ACK++;
		}

		if (EG91_send_AT_Command("AT+CREG?", "AT+CREG", 1000))
		{
			ACK++;
		}

		if (EG91_send_AT_Command("AT+CGREG?", "AT+CGREG", 1000))
		{
			ACK++;
		}

		// EG91_send_AT_Command("AT+QMBNCFG=\"List\"", "OK", 1000);
		EG91_send_AT_Command("AT+CGDCONT?", "OK", 1000);

		/* for (size_t i = 0; i < 10; i++)
		{
			EG91_send_AT_Command("AT+CIREG?", "OK", 1000);
			vTaskDelay(pdMS_TO_TICKS(1000));
		} */

		EG91_send_AT_Command("AT+CSDH=1", "OK", 1000);

		EG91_send_AT_Command("AT+QCFG=\"urc/ri/other\",\"off\"", "OK", 1000);

		EG91_get_IMEI();

		// getFile_importUsers("char *DNS");
		/*  EG91_send_AT_Command("AT+QFOTADL=\"http://static.home.motorline.pt/homes/Update_EG915UE.pack\"", "OK", 100000);  */

		uint8_t label_network_portalRegister = get_INT8_Data_From_Storage(NVS_NETWORK_PORTAL_REGISTER, nvs_System_handle);

		if (label_network_portalRegister == 255)
		{
			label_network_portalRegister = 1;
			save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER, label_network_portalRegister, nvs_System_handle);
		}

		if (label_network_portalRegister == 1)
		{
			if (init_UDP_socket() == 2)
			{
				ACK++;
				// printf("\n\n finish sub99\n\n");
				save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 0, nvs_System_handle);
				// printf("\n\n finish sub443\n\n");
				register_UDP_Device();
				// printf("\n\n finish sub77\n\n");
				if (EG91_send_AT_Command(AT_CSQ, "CSQ", 1000))
				{
					ACK++;
				}
				// printf("\n\n init_UDP_socket OK \n\n");
			}
			else
			{
				mqtt_openLabel = 0;
				mqtt_connectLabel = 0;
				save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 1, nvs_System_handle);
			}
		}
		else
		{
			if (EG91_send_AT_Command(AT_CSQ, "CSQ", 1000))
			{
				ACK++;
			}
		}
		// printf("\n\n finish sub88\n\n");

		/*for (size_t i = 0; i < 5; i++)
		 {
			 EG91_UDP_Ping();
		 } */

		char registerDevice[30] = {};

		// ////printf("\n registerDevice\n");

		// sprintf(registerDevice, "%s %d", "081321069139715", RSSI_VALUE);

		// xQueueSendToBack(UDP_Send_queue, (void *)&registerDevice, pdMS_TO_TICKS(1000));
		// send_UDP_Package(registerDevice, 16);

		// EG91_send_AT_Command("AT&F0", "OK", 1000);

		if (((label_network_portalRegister == 0) && ACK == 15) || ((label_network_portalRegister == 1) && (ACK == 16)))
		{
			// ////printf("\n registerDevice 22\n");
			return 1;
		}
	}

	// timer_start(0, 0);
	return 0;
}

uint8_t disconect_mqtt()
{
	uint8_t ACK = 0;

	if (EG91_send_AT_Command("AT+QMTDISC=0", "OK", 30000))
	{
		/* code */
	}
	return 0;
}

void give_rdySem_Control_Send_AT_Command()
{
	if (rdySem_Control_Send_AT_Command != NULL)
	{
		// ////printf("\rdySem_Control_Send_AT_Command != NULL\n");
		xSemaphoreGive(rdySem_Control_Send_AT_Command);
	}
	else
	{
		// ////printf("\rdySem_Control_Send_AT_Command = NULL\n");
	}
}

void give_rdySem_Feedback_Call()
{
	if (rdySem_Feedback_Call != NULL)
	{
		// ////printf("\nrdySem_Feedback_Call != NULL\n");
		xSemaphoreGive(rdySem_Feedback_Call);
	}
	else
	{
		// ////printf("\nrdySem_Feedback_Call = NULL\n");
	}
}

void task_EG91_Record_Feedback_Call(void *pvParameter)
{
	char AT_Command[300];
	uint8_t counterACK = 25;
	char dtmp1[BUFF_SIZE];
	uint8_t ACK = 9;
	rdySem_Feedback_Call = xSemaphoreCreateBinary();

	for (;;)
	{
		xSemaphoreTake(rdySem_Feedback_Call, portMAX_DELAY);
		// EG91_writeFile("Y.txt", "motorline");
		EG91_send_AT_Command("AT+QFDEL=\"UFS:RECORD_SOUND.wav\"", "OK", 1000);
		EG91_send_AT_Command("ATD917269448", "OK", 1000); /* 918712872 */ /* HELDER 918712872*/

		sprintf(AT_Command, "%s%c", "AT+CLCC", 13);

		// ////printf("\nAT COMMAND parse_IncomingCall = %s\n", AT_Command);
		//   esp_task_wdt_init(10, true);
		while (counterACK > 0)
		{
			memset(dtmp1, 0, sizeof(dtmp1));
			uart_write_bytes(UART_NUM_1, AT_Command, strlen(AT_Command));
			xQueueReceive(EG91_CALL_CLCC_UART_queue, dtmp1, pdMS_TO_TICKS(1500)); //(TickType_t)(time / portTICK_PERIOD_MS)
			// ////printf("\nAT_Command_Feedback_queue_CLCC = %s\n", dtmp1);
			ACK = parse_IncomingCall_Payload(&dtmp1);
			if (ACK == 0)
			{
				// free(rsp);
				//  free(&dtmp1);
				// ////printf("\n call accept %d\n", 1);

				memset(dtmp1, 0, sizeof(dtmp1));
				vTaskDelay(pdMS_TO_TICKS((750)));
				EG91_send_AT_Command("AT+QPSND=1,\"UFS:beepInicial.wav\",0,1,0", "OK", 3000);
				EG91_send_AT_Command("AT+QAUDRD=1,\"UFS:RECORD_SOUND.wav\",13,1", "OK", 1000);

				// xQueueReset(AT_Command_Feedback_queue);
				// xSemaphoreGive(rdySem_Control_Send_AT_Command);
				break;
			}
			else
			{
				memset(dtmp1, 0, sizeof(dtmp1));
			}
			counterACK--;
			memset(dtmp1, 0, sizeof(dtmp1));
			vTaskDelay(pdMS_TO_TICKS((1000)));
		}
	}
}

void receive_NoCarrie_queue()
{
	char dtmp1[600];
	xSemaphoreTake(rdySem_QPSND, pdMS_TO_TICKS(15000));
	// xQueueReceive(NO_CARRIER_Call_queue, &dtmp1, pdMS_TO_TICKS(15000));
}

void record_Feedback_Sound()
{
	// ////printf("\n ENTER RECORD SOUND %d\n", 1);
	uint8_t counterACK = 18;
	char dtmp1[BUFF_SIZE];
	xQueueReset(NO_CARRIER_Call_queue);
	call_label = 1;
	uint8_t ACK = 9;

	if (incomingCall_Label == 1)
	{
		EG91_Send_SMS("917269448", "THE SYSTEM IS RECEIVING ANOTHER CALL AT THIS TIME. PLEASE TRY AGAIN");
	}
	else
	{

		// timer_pause(1, 0);
		//  timer_pause(0, 0);

		// vTaskSuspend(xHandle_Timer_VerSystem);
		//  EG91_writeFile("Y.txt", "motorline");
		//  EG91_send_AT_Command("AT+QDAI=1,1,1,4,0,0,1,1", "OK", 1000);AT+QTTS=1,"6B228FCE4F7F752879FB8FDC6A215757"

		EG91_send_AT_Command("AT+CLVL?", "OK", 1000);
		// EG91_send_AT_Command("AT+QFLST=\"*\"", "OK", 1000);
		EG91_send_AT_Command("ATD917269448", "OK", 1000); /* 918712872 936356241 */
		//
		counterACK = 33;

		while (counterACK > 0)
		{

			ACK = parse_Call(CALL_STATE);

			if (ACK == 0)
			{
				EG91_send_AT_Command("AT+QFDEL=\"UFS:RECORD_SOUND.wav\"", "OK", 1000);
				// EG91_send_AT_Command("AT+QFLST=\"*\"", "OK", 1000);
				// ////printf("\n call accept %d\n", 1);

				memset(dtmp1, 0, sizeof(dtmp1));
				// vTaskDelay(pdMS_TO_TICKS(500));

				EG91_send_AT_Command("AT+QPSND=1,\"UFS:beepInicial.wav\",0,1,0", "OK", 3000);
				// EG91_send_AT_Command("AT+QPSND=1,\"UFS:RECORD_SOUND.wav\",0,1,0", "OK", 3000);
				xSemaphoreTake(rdySem_QPSND, pdMS_TO_TICKS(3000));
				EG91_send_AT_Command("AT+QAUDRD=1,\"UFS:RECORD_SOUND.wav\",13,1", "OK", 1000);
				// ////printf("\n call accept before queue\n");
				vTaskDelay(pdMS_TO_TICKS((2500)));
				xQueueReceive(NO_CARRIER_Call_queue, dtmp1, pdMS_TO_TICKS(10000));
				// xQueueReset(NO_CARRIER_Call_queue);
				// ////printf("\n call accept after queue\n");
				if (strstr(dtmp1, "NO CARRIER") != NULL)
				{
					// ////printf("\n call accept NO CARRIER\n");
					EG91_send_AT_Command("AT+QAUDRD=0", "OK", 1000);
					vTaskDelay(pdMS_TO_TICKS(300));
					EG91_send_AT_Command("ATH", "OK", 1000);
				}
				else
				{
					// ////printf("\n call accept NOT NOT CARRIER\n");
					EG91_send_AT_Command("AT+QAUDRD=0", "OK", 1000);
					vTaskDelay(pdMS_TO_TICKS((100)));
					EG91_send_AT_Command("AT+QPSND=1,\"UFS:beepFinal.wav\",0,1,0", "OK", 3000);
					xSemaphoreTake(rdySem_QPSND, pdMS_TO_TICKS(4000));

					EG91_send_AT_Command("ATH", "OK", 1000);
				}

				break;
			}
			else if (ACK == 255)
			{
				memset(dtmp1, 0, sizeof(dtmp1));
				break;
			}
			else
			{
				memset(dtmp1, 0, sizeof(dtmp1));
			}

			counterACK--;

			if (counterACK == 0)
			{
				EG91_send_AT_Command("ATH", "OK", 1000);
				memset(dtmp1, 0, sizeof(dtmp1));
				break;
			}

			memset(dtmp1, 0, sizeof(dtmp1));
			vTaskDelay(pdMS_TO_TICKS((500)));
		}
		// EG91_send_AT_Command("ATH", "OK", 1000);
		call_label = 0;
		// vTaskResume(xHandle_Timer_VerSystem);
		// timer_start(1, 0);
	}
}

void play_Feedback_Sound()
{

	// ////printf("\n ENTER RECORD SOUND %d\n", 1);
	uint8_t counterACK = 18;
	MyUser MyUser_Feedback_Call;

	char dtmp1[600];
	char AT_Command[100];
	char owner_info[200] = {};
	char ownerNumber[30] = {};
	size_t required_size = 0;
	xQueueReset(NO_CARRIER_Call_queue);
	call_label = 1;
	int8_t ACK = 9;

	memset(&MyUser_Feedback_Call, 0, sizeof(MyUser_Feedback_Call));

	if (nvs_get_str(nvs_System_handle, NVS_KEY_OWNER_INFORMATION, NULL, &required_size) == ESP_OK)
	{
		// ////printf("\nrequire size %d\n", required_size);
		// ////printf("\nGET USERS NAMESPACE\n");
		if (nvs_get_str(nvs_System_handle, NVS_KEY_OWNER_INFORMATION, owner_info, &required_size) == ESP_OK)
		{

			parse_ValidateData_User(owner_info, &MyUser_Feedback_Call);

			if (incomingCall_Label == 1)
			{
				EG91_Send_SMS(MyUser_Feedback_Call.phone, return_Json_SMS_Data("THE_SYSTEM_IS_RECEIVING_ANOTHER_CALL"));
			}
			else
			{

				// timer_pause(1, 0);
				//  timer_pause(0, 0);

				// vTaskSuspend(xHandle_Timer_VerSystem);
				//  EG91_writeFile("Y.txt", "motorline");
				//  EG91_send_AT_Command("AT+QDAI=1,1,1,4,0,0,1,1", "OK", 1000);
				// EG91_send_AT_Command("AT+CLVL?", "OK", 1000);
				// EG91_send_AT_Command("AT+QFLST=\"*\"", "OK", 1000);
				ACK = EG91_send_AT_Command("AT+QFOPEN=\"RECORD_SOUND.wav\"", "QFOPEN", 5000);

				int idFile = atoi(fileID);
				//////printf("\nfile id %d - %s", idFile, fileID);
				if (ACK)
				{

					sprintf(AT_Command, "UFS:AT+QFCLOSE=%d", idFile);
					EG91_send_AT_Command(AT_Command, "OK", 1000);
				}
				else
				{
					EG91_Send_SMS("917269448", "THE SYSTEM DOES NOT CONTAIN A RECORDED AUDIO FILE. PLEASE RECORD A NEW AUDIO");
					return;
				}

				// EG91_send_AT_Command("AT+QFLST=\"*\"", "OK", 1000);
				EG91_send_AT_Command("ATS7=0", "OK", 1500);
				sprintf(AT_Command, "ATD%s", MyUser_Feedback_Call.phone);
				EG91_send_AT_Command(AT_Command, "OK", 1000); /* 918712872 936356241 935531736 */
				//
				counterACK = 18;

				while (counterACK > 0)
				{
					timer_pause(TIMER_GROUP_1, TIMER_0);
					// ////printf("\nsms\n");
					disableAlarm();
					vTaskSuspend(xHandle_Timer_VerSystem);
					vTaskSuspend(handle_SEND_SMS_TASK);

					ACK = parse_Call(CALL_STATE);
					// ////printf("\n after parse_Call \n");
					if (ACK == 0)
					{

						// ////printf("\n call accept %d\n", 1);

						memset(dtmp1, 0, sizeof(dtmp1));
						vTaskDelay(pdMS_TO_TICKS(750));
						char *sound_AT_Command_file_Name;
						uint8_t soundValue = get_INT8_Data_From_Storage(NVS_KEY_FEEDBACK_SOUND_INDEX, nvs_System_handle);

						if (soundValue >= 1 && soundValue <= 8)
						{
							asprintf(&sound_AT_Command_file_Name, "AT+QPSND=1,\"UFS:sound_%d.wav\",0,1,0", soundValue);
						}
						else
						{
							asprintf(&sound_AT_Command_file_Name, "%s", "AT+QPSND=1,\"UFS:sound_1.wav\",0,1,0");
						}

						// ////printf("\n\nsound_AT_Command_file_Name\n%s\n\n", sound_AT_Command_file_Name);
						EG91_send_AT_Command(sound_AT_Command_file_Name, "OK", 3000);
						/* EG91_send_AT_Command("AT+QPSND=1,\"UFS:RECORD_SOUND.wav\",0,1,0", "OK", 3000); */
						xSemaphoreTake(rdySem_QPSND, pdMS_TO_TICKS(15000));
						free(sound_AT_Command_file_Name);
						break;
					}
					else if (ACK == 255)
					{
						memset(dtmp1, 0, sizeof(dtmp1));

						break;
					}
					else
					{
						memset(dtmp1, 0, sizeof(dtmp1));
					}

					counterACK--;
					memset(dtmp1, 0, sizeof(dtmp1));
					vTaskDelay(pdMS_TO_TICKS((1000)));
				}
				// ////printf("\nsms receive 123\n");
				vTaskResume(handle_SEND_SMS_TASK);
				vTaskResume(xHandle_Timer_VerSystem);
				timer_start(TIMER_GROUP_1, TIMER_0);
				EG91_send_AT_Command("ATS7=3", "OK", 1500);
				EG91_send_AT_Command("ATH", "OK", 1000);
				enableAlarm();
			}
		}
		else
		{
			return;
		}
	}
	else
	{
		return;
	}
}

void task_EG91_Feedback_Call(void *pvParameter)
{
	char AT_Command[300];

	char dtmp1[BUFF_SIZE];
	uint8_t ACK = 9;
	uint8_t typeCall_index = 0;

	rdySem_Feedback_Call = xSemaphoreCreateBinary();
	xSemaphoreGive(rdySem_Feedback_Call);

	for (;;)
	{
		xQueueReceive(Type_Call_queue, &typeCall_index, portMAX_DELAY);
		xSemaphoreTake(rdySem_Feedback_Call, portMAX_DELAY);

		// EG91_send_AT_Command("AT+QCFG=\"nwscanmode\",1", "OK", 3000);

		if (typeCall_index == RECORD_SOUND_STATE)
		{
			record_Feedback_Sound();
		}
		else if (typeCall_index == PLAY_RECORD_SOUND_STATE)
		{
			play_Feedback_Sound();
		}

		// EG91_send_AT_Command("AT+QCFG=\"nwscanmode\",0", "OK", 3000);
		xSemaphoreGive(rdySem_Feedback_Call);
	}
}

void task_EG91_Run_IncomingCall(void *pvParameter)
{

	incomingCall_Label = 1;
	uint8_t ACK = 0;

	// ////printf("\nincoming call before suspend! 1\n");

	if (label_initSystem_CALL != 0)
	{

		timer_pause(TIMER_GROUP_1, TIMER_0);
		// ////printf("\nsms or call %s\n", atcmd);
		vTaskSuspend(xHandle_Timer_VerSystem);
		// ////printf("\nincoming call before suspend! 2\n");
		vTaskSuspend(handle_SEND_SMS_TASK);
		// ////printf("\nincoming call before suspend! 3\n");
	}

	// TODO: ALTERAÇÃO NESTE GIVE

	// xSemaphoreGive(rdySem_Control_Send_AT_Command);

	// EG91_send_AT_Command("AT+QHUP=88,1", "OK", 1000);

	// if (rele1_Mode_Label == BIESTABLE_MODE_INDEX)
	// {
	/* if (label_MonoStableRelay1 != 1)
	{ */
	// ////printf("\ncall receive\n");
	// ////printf("\nincoming call before suspend! 4\n");
	// ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
	parse_Call(INCOMING_CALL_STATE);
	/*   }
	  else
	  {
		  // vTaskDelay(pdMS_TO_TICKS((300)));
		  // sprintf(awnser_QHUP, "%s,%s", "AT+QHUP=17", id);
		  EG91_send_AT_Command("AT+QHUP=27", "OK", 1000);
		  // EG91_send_AT_Command("AT+QHUP=27", "OK", 500);
	  } */
	/*   }
	  else
	  {
		  //////printf("\ncall receive\n");
		  parse_Call(INCOMING_CALL_STATE);
	  } */

	aux_label_inCall = 0;
	incomingCall_Label = 0;

	if (label_initSystem_CALL != 0)
	{
		vTaskResume(xHandle_Timer_VerSystem);
		vTaskResume(handle_SEND_SMS_TASK);
		timer_start(TIMER_GROUP_1, TIMER_0);
	}

	xSemaphoreGive(rdySem_Control_Send_AT_Command);

	vTaskDelete(NULL);
}

void task_EG91_Run_SMS(void *pvParameter)
{

	char atcmd[BUFF_SIZE] = {}; //= (char *)pvParameter; //[BUFF_SIZE];
	// ////printf("\nsms param receive %s\n", atcmd);
	char rsp[30];

	// incomingCall_Label = 0;
	uint8_t ACK = 0;

	xSemaphoreGive(rdySem_Control_SMS_Task);

	for (;;)
	{
		xQueueReceive(EG91_CALL_SMS_UART_queue, &atcmd, portMAX_DELAY);
		// ////printf("\nsms kakakak\n");

		if ((strstr(atcmd, "CMTI")) != NULL)
		{
			xSemaphoreTake(rdySem_Control_SMS_Task, pdMS_TO_TICKS(2000));
			// ////printf("\n\nsms HELLOO 1\n\n");
			/* timer_pause(TIMER_GROUP_1, TIMER_0);
			 //////printf("\n\nsms HELLOO 1\n\n");
			disableAlarm();
			vTaskSuspend(xHandle_Timer_VerSystem);
			vTaskSuspend(handle_SEND_SMS_TASK);
			xSemaphoreGive(rdySem_Control_Send_AT_Command); */
			// ////printf("\nsms receive\n %s\n", atcmd);
			//    vTaskDelay(pdMS_TO_TICKS(2000));
			//    xSemaphoreTake(rdySem_Control_SEND_SMS_Task, pdMS_TO_TICKS(10000));
			// system_stack_high_water_mark("parse sms1");
			parse_SMS(atcmd);
			// verify_SMS_List();
			// ////printf("\nsms receive 123\n");
			// ////printf("\n\nsms HELLOO 2\n\n");
			/* vTaskResume(handle_SEND_SMS_TASK);
			vTaskResume(xHandle_Timer_VerSystem);
			timer_start(TIMER_GROUP_1, TIMER_0);
			enableAlarm();*/
			// xSemaphoreGive(rdySem_Control_Send_AT_Command);
			memset(atcmd, 0, BUFF_SIZE);
			// xSemaphoreGive(rdySem_Control_SMS_Task);
		}

		// ////printf("\nsms receive 124\n");
	}
	handle_SMS_TASK = NULL;
	vTaskDelete(NULL);
	// return 0;
}

uint8_t parse_SMS_data(data_EG91_Receive_SMS *receive_SMS_data)
{

	data_EG91_Send_SMS sms_Data;

	// if (!strcmp(receive_SMS_data->SMS_Status, "REC UNREAD"))
	// {
	/* code */

	int strIndex = 0;
	int count = 0;
	char payload_phNumber[21];

	char strDate[50];
	char strHour[20];
	char year[20];
	char month[3];
	char day[3];
	char hour[3];
	char minute[3];
	char payload1[200];
	char SMS_Status[100];

	char aux_STR_Date[30] = {};
	char aux_STR_Time[5] = {};
	memset(aux_STR_Date, 0, sizeof(aux_STR_Date));
	memset(aux_STR_Time, 0, sizeof(aux_STR_Time));
	memset(strDate, 0, sizeof(strDate));
	memset(strHour, 0, sizeof(strHour));
	memset(year, 0, sizeof(year));
	memset(month, 0, sizeof(month));
	memset(day, 0, sizeof(day));
	memset(hour, 0, sizeof(hour));
	memset(minute, 0, sizeof(minute));
	memset(payload1, 0, sizeof(payload1));
	memset(SMS_Status, 0, sizeof(SMS_Status));

	int aux_year;

	// ////printf("\nSMS_Status1 - %s - %d\n", receive_SMS_data->SMS_Status, strlen(receive_SMS_data->SMS_Status));
	strIndex = 0;

	for (int i = strlen(receive_SMS_data->receiveData) - 1; i >= 0; i--)
	{
		if (receive_SMS_data->receiveData[i] == ' ')
		{
			receive_SMS_data->receiveData[i] = 0;
		}
		else
		{
			break;
		}
	}
	// printf("\n aux_STR_Data ww %s\n", receive_SMS_data->receiveData);
	//  ////printf("\nstrlen receivedata %d\n", strlen(receive_SMS_data->receiveData));
	for (int i = 0; i < strlen(receive_SMS_data->receiveData); i++)
	{
		if (receive_SMS_data->receiveData[i] == ' ' || i > 20)
		{
			payload_phNumber[i] = 0;
			break;
		}

		payload_phNumber[i] = receive_SMS_data->receiveData[i];
		// payload_phNumber[i] = 0;
	}
	// ////printf("\nSMS_Status2 - %s - %d\n", receive_SMS_data->SMS_Status, strlen(receive_SMS_data->SMS_Status));
	count = 0;
	strIndex = 0;

	for (int i = 0; i < strlen(receive_SMS_data->strDate); i++)
	{
		if (receive_SMS_data->strDate[i] == '/')
		{
			count++;
			strIndex = 0;
		}
		else
		{
			if (count == 0)
			{

				year[strIndex] = receive_SMS_data->strDate[i];
				strIndex++;
				year[strIndex] = 0;
			}
			if (count == 1)
			{
				if (strIndex < 2)
				{
					month[strIndex] = receive_SMS_data->strDate[i];
					strIndex++;
					month[strIndex] = 0;
				}
			}
			if (count == 2)
			{
				if (strIndex < 2)
				{
					day[strIndex] = receive_SMS_data->strDate[i];
					strIndex++;
					day[strIndex] = 0;
				}
			}
		}
		// payload_phNumber[i] = payload1[i];
	}

	// SMS_Status[strlen(SMS_Status)]=0;
	// ////printf("\nSMS_Status3 - %s - %d\n", receive_SMS_data->SMS_Status, strlen(receive_SMS_data->SMS_Status));

	year[5] = 0;
	month[2] = 0;
	day[2] = 0;

	count = 0;
	strIndex = 0;

	for (int i = 0; i < strlen(receive_SMS_data->strHour); i++)
	{
		if (receive_SMS_data->strHour[i] == ':')
		{
			count++;
			strIndex = 0;
		}
		else
		{
			if (count == 0)
			{
				hour[strIndex] = receive_SMS_data->strHour[i];
				strIndex++;
			}
			if (count == 1)
			{
				minute[strIndex] = receive_SMS_data->strHour[i];
				strIndex++;
			}
			if (count == 2)
			{
				break;
			}
		}
	}

	if (atoi(year) > 99)
	{
		aux_year = atoi(year) - 2000;
		sprintf(aux_STR_Date, "%d%s%s", aux_year, month, day);
	}
	else
	{
		sprintf(aux_STR_Date, "%s%s%s", year, month, day);
	}

	sprintf(aux_STR_Time, "%s%s", hour, minute);
	// printf("\n aux_STR_Data %s\n", receive_SMS_data->receiveData);
	//  //printf("\n aux_STR_Time %s\n", receive_SMS_data->SMS_Status);

	hour[2] = 0;
	minute[2] = 0;

	uint8_t zeroNumber_label = 0;

	// TODO: METER VERIFICAÇÃO DA HORA COM A INTEGRAÇÃO DO RTC

	char *rsp = NULL;
	char *SMS_InputData = NULL;

	if (/* abs(atoi(aux_STR_Time) - nowTime.time) < 2 && (atoi(aux_STR_Date) == nowTime.date) */ /*  &&*/ (!strcmp(SMS_UNREAD, receive_SMS_data->SMS_Status) || lost_SMS_addUser.flag_Lost_SMS_Add_User == 1))
	{
		char SMS_number[100] = {};
		sprintf(SMS_number, "%s", receive_SMS_data->phNumber);

		// printf("\n\nowner phone22 %s\n - %s\n - \n\n", receive_SMS_data->phNumber, receive_SMS_data->receiveData);

		sprintf(receive_SMS_data->phNumber, "%s", check_IF_haveCountryCode(receive_SMS_data->phNumber, 0));

		// //printf("phNumber %s\n", receive_SMS_data->phNumber);
		sprintf(payload_phNumber, "%s", check_IF_haveCountryCode(payload_phNumber, 0));

		uint8_t verification_SMS_CALL = get_INT8_Data_From_Storage(NVS_SMS_CALL_VERIFICATION, nvs_System_handle);

		if (verification_SMS_CALL == 255)
		{
			save_INT8_Data_In_Storage(NVS_SMS_CALL_VERIFICATION, 0, nvs_System_handle);
			verification_SMS_CALL = 0;
		}

		if (strlen(receive_SMS_data->receiveData) > 1)
		{

			if (!strcmp(receive_SMS_data->phNumber, payload_phNumber) || lost_SMS_addUser.flag_Lost_SMS_Add_User == 1 || verification_SMS_CALL == 1 || verification_SMS_CALL == 4)
			{
				// //printf("\n  payload phnumber sms 999 %s \n", lost_SMS_addUser.phone);
				// //printf("\nreceiveData[strlen = %c\n", receive_SMS_data->receiveData[strlen(receive_SMS_data->receiveData) - 2]);
				sms_Data.labelRsp = 0;

				for (size_t i = strlen(receive_SMS_data->receiveData) - 1; i > 0; i--)
				{
					if (receive_SMS_data->receiveData[i] == '.')
					{
						break;
					}

					if (receive_SMS_data->receiveData[i] == '?')
					{
						receive_SMS_data->receiveData[i] = 0;
						sms_Data.labelRsp = 1;

						for (size_t j = strlen(receive_SMS_data->receiveData) - 1; j > 0; j--)
						{
							if (receive_SMS_data->receiveData[j] == ' ')
							{
								receive_SMS_data->receiveData[j] = 0;
							}
							else
							{
								break;
							}
						}

						break;
					}
				}

				// TODO: DESCOMENTAR SE NAO FUNCIONAR
				sprintf(sms_Data.phoneNumber, "%s", SMS_number);

				if (lost_SMS_addUser.flag_Lost_SMS_Add_User == 1)
				{
					if ((lost_SMS_addUser.phone[0] == '0') && lost_SMS_addUser.phone[1] != '0')
					{

						// //printf("\n  payload phnumber sms 33 %s \n", lost_SMS_addUser.phone);
						for (size_t i = 1; i < strlen(lost_SMS_addUser.phone); i++)
						{
							lost_SMS_addUser.phone[i - 1] = lost_SMS_addUser.phone[i];
						}

						lost_SMS_addUser.phone[strlen(lost_SMS_addUser.phone) - 1] = 0;
					}

					sprintf(lost_SMS_addUser.phone, "%s", check_IF_haveCountryCode(lost_SMS_addUser.phone, 0));
					// //printf("receive data 12 lost %s - %s\n", lost_SMS_addUser.phone, receive_SMS_data->phNumber);
					//  ////printf("\n323232 phnumber sms %s \n", receive_SMS_data->phNumber);
					//  TODO: ALTERAR NO CODIGO DO 4G
					if (!strcmp(receive_SMS_data->phNumber /* sms_Data.phoneNumber */, lost_SMS_addUser.phone) || verification_SMS_CALL == 1 || verification_SMS_CALL == 4)
					{
						// //printf("receive data 155 LOSTlost %s\n", lost_SMS_addUser.phone);
						asprintf(&SMS_InputData, "%s", lost_SMS_addUser.payload);
						// ////printf("\n\n lost_SMS_addUser.phone %s\n\n", lost_SMS_addUser.phone);
						sprintf(sms_Data.phoneNumber, "%s", SMS_number /* lost_SMS_addUser.phone */);
					}

					lost_SMS_addUser.flag_Lost_SMS_Add_User = 0;

					// //printf("\n\n lost_SMS_addUser.phone sms_Data %s\n\n", SMS_InputData);
				}
				else
				{

					asprintf(&SMS_InputData, "%s", receive_SMS_data->receiveData);
					sprintf(sms_Data.phoneNumber, "%s", SMS_number /* receive_SMS_data->phNumber */);
				}

				/*memset(&rsp, 0, sizeof(rsp));*/
				// xSemaphoreGive(rdySem_Control_Send_AT_Command);
				// //printf("receive data 155 LOSTlost 4132 %s\n", SMS_InputData);
				mqtt_information mqttInfo;
				rsp = parseInputData((uint8_t *)SMS_InputData, SMS_INDICATION, NULL, NULL, NULL, &sms_Data, &mqttInfo);

				// ////printf("\n\n SMS SMS_InputData %s\n\n", SMS_InputData);
				// ////printf("\n\n rsp rsp %s\n\n", rsp);
				//   //////printf("\n\n SMS RSP %s\n\n",rsp);

				// //printf("\n\n SMS RSP11 %s\n\n", rsp);

				if (sms_Data.labelRsp == 1 && strcmp(rsp, "NTRSP") /*&&  !strcmp(receive_SMS_data->SMS_Status, "REC UNREAD") */)
				{
					sprintf(sms_Data.payload, "%s", rsp);
					xQueueSendToBack(queue_EG91_SendSMS, (void *)&sms_Data, pdMS_TO_TICKS(100));
				}
				else
				{
					// ////printf("\n\n SMS RSP777\n\n");
					if (strcmp(rsp, "NTRSP"))
					{
						// ////printf("\n\n rsp verify_SMS_List 11\n\n");
						xSemaphoreGive(rdySem_Control_Send_AT_Command);
						xSemaphoreGive(rdySem_Control_SMS_Task);
						// verify_SMS_List();
					}
					else
					{
						xSemaphoreGive(rdySem_Control_SMS_Task);
					}

					// xSemaphoreGive(rdySem_Lost_SMS);
					// xSemaphoreGive(rdySem_Control_SMS_Task);
				}
			}
			else
			{
				xSemaphoreGive(rdySem_Control_SMS_Task);
			}
		}
	}

	// //printf("\n\n SMS RSP333\n\n");
	/*



	 //////printf("\n\n SMS RSP222\n\n");
	 free(rsp);

	 free(SMS_InputData);*/
	// //printf("\n\n SMS RSP444\n\n");
	xSemaphoreGive(rdySem_Control_SMS_Task);
	free(rsp);
	// //printf("\n\n SMS RSP444\n\n");
	free(SMS_InputData);
	// ESP_LOGI("TAG", "xPortGetFreeHeapSize  parseInputData2222: %d", xPortGetFreeHeapSize());
	// ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
	// ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
	// ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	//  }
	return 1;
}

void give_rdySem_Control_SMS_Task()
{
	xSemaphoreGive(rdySem_Control_SMS_Task);
}

uint8_t parse_SMS(char *payload)
{
	char atCommand[50];

	char *str = strtok(payload, (const char *)",");
	str = strtok((char *)NULL, (const char *)"\r");
	uint8_t msgCounter = atoi(str);

	/*     ////printf("\n\nstrtok sms after11\n\n");

		//ESP_LOGI("TAG", "xPortGetFreeHeapSize  parseInputData1111: %d", xPortGetFreeHeapSize());
		//ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
		//ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
		//ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));

		//////printf("\n\nstrtok sms after22\n\n"); */

	// while (msgCounter >= 0)
	// {
	// xSemaphoreTake(rdySem_Control_SMS_Task, pdMS_TO_TICKS(60000));
	// xSemaphoreGive(rdySem_Control_Send_AT_Command);
	// EG91_send_AT_Command("ATE1", "OK", 1000);

	get_RTC_System_Time();
	// ////printf("parse_SMS number %d", atoi(str));

	// EG91_send_AT_Command("ATE1", "OK", 1500);
	EG91_send_AT_Command("AT+CSCS=\"UCS2\"", "OK", 1000);

	EG91_send_AT_Command("AT+CMGF=1", "OK", 1000);
	// EG91_send_AT_Command("AT+CSDH=1", "OK", 1000);

	// vTaskSuspend(handle_SEND_SMS_TASK);

	sprintf(atCommand, "%s%d", "AT+CMGR=", msgCounter);
	EG91_send_AT_Command(atCommand, "+CMGR", 1000);
	// ////printf("\nafter CMGR\n");
	memset(atCommand, 0, sizeof(atCommand));
	// ////printf("\n\nCMGD == 22\n\n");
	sprintf(atCommand, "%s%d,0", "AT+CMGD=", msgCounter);

	EG91_send_AT_Command(atCommand, "OK", 1000);

	// TODO: UNCOMMENT IF NOT WORK
	xSemaphoreGive(rdySem_Control_Send_AT_Command);

	if (msgCounter > 25)
	{
		EG91_send_AT_Command("AT+CMGD=1,4", "OK", 1000);
		EG91_send_AT_Command("AT+CMGD=4", "OK", 1000);
	}
	// ////printf("\nparse_SMS number 13\n");
	//   verify_SMS_List();
	//  free(str);
	//  xSemaphoreGive(rdySem_Control_SMS_Task);
	return 1;
}

int8_t parse_Call(uint8_t state)
{

	char AT_Command[100];
	uint8_t counterACK = 3;
	char dtmp1[BUFF_SIZE];
	int8_t ACK = 0;
	sprintf(AT_Command, "%s%c", "AT+CLCC", 13);

	// ////printf("\nAT COMMAND parse_Call = %s\n", AT_Command);
	//   esp_task_wdt_init(10, true);

	if (state == INCOMING_CALL_STATE)
	{
		// ////printf("\nAT COMMAND parse_Call = %s\n", AT_Command);
		while (counterACK > 0)
		{

			memset(dtmp1, 0, sizeof(dtmp1));
			// EG91_send_AT_Command(AT_Command, "CLCC", 1500);
			uart_write_bytes(UART_NUM_1, AT_Command, strlen(AT_Command));
			// EG91_send_AT_Command(AT_Command, "CLCC", 1000);
			xQueueReceive(EG91_CALL_CLCC_UART_queue, dtmp1, pdMS_TO_TICKS(1500)); //(TickType_t)(time /  portTICK_PERIOD_MS)
			// xSemaphoreGive(rdySem_Control_Send_AT_Command);
			// ////printf("\nAT_Command_Feedback_queue_CLCC = %s\n", dtmp1);
			// system_stack_high_water_mark("parse call1");
			ACK = parse_IncomingCall_Payload(&dtmp1);
			if (ACK == 1)
			{
				// free(rsp);
				//  free(&dtmp1);
				// ////printf("\n parse data %d\n", 1);

				// ////printf("\n parse data %d\n", 2);
				memset(dtmp1, 0, sizeof(dtmp1));

				// xQueueReset(AT_Command_Feedback_queue);
				// xSemaphoreGive(rdySem_Control_Send_AT_Command);
				// vTaskResume(handle_SEND_SMS_TASK);
				return 1;
			}
			else if (ACK == 255)
			{
				// vTaskResume(handle_SMS_TASK);
				break;
			}

			else
			{
				memset(dtmp1, 0, sizeof(dtmp1));
				counterACK--;
			}
			memset(dtmp1, 0, sizeof(dtmp1));
			vTaskDelay(pdMS_TO_TICKS((1000)));
		}

		// EG91_send_AT_Command("AT+CLCC", "CLCC", 5000);

		// EG91_send_AT_Command("AT+QHUP=16", "OK", 500);
		// incomingCall_Label = 0;
		// vTaskResume(handle_SEND_SMS_TASK);
		return 0;
	}

	if (state == CALL_STATE)
	{
		memset(dtmp1, 0, sizeof(dtmp1));
		uart_write_bytes(UART_NUM_1, AT_Command, strlen(AT_Command));
		xQueueReceive(EG91_CALL_CLCC_UART_queue, dtmp1, pdMS_TO_TICKS(1500)); //(TickType_t)(time / portTICK_PERIOD_MS)
		// ////printf("\nAT_Command_Feedback_queue_CLCC = %s\n", dtmp1);
		return parse_IncomingCall_Payload(&dtmp1);
	}

	return -1;
}

void giveSem_CtrIncomingCall()
{
	xSemaphoreGive(rdySem_Control_IncomingCALL);
}

uint8_t parseCHUP(char *payload)
{
	char AT_Command[300];
	uint8_t counterACK = 3;
	char dtmp1[BUFF_SIZE];
	sprintf(AT_Command, "%s%c", payload, 13);

	// ////printf("\nAT COMMAND parse_IncomingCall = %s\n", AT_Command);
	//   vTaskDelay(pdMS_TO_TICKS(1000));
	while (counterACK > 0)
	{
		memset(dtmp1, 0, sizeof(dtmp1));

		// if (rele1_Mode_Label == 0)
		// {

		//     xSemaphoreTake(rdySem_Control_IncomingCALL, (portTickType)portMAX_DELAY);
		// }

		uart_write_bytes(UART_NUM_1, AT_Command, strlen(AT_Command));
		xQueueReceive(EG91_CALL_CHUP_UART_queue, dtmp1, pdMS_TO_TICKS(1500)); //(TickType_t)(time / portTICK_PERIOD_MS)
		// ////printf("\nAT_Command_Feedback_queue_CLCC = %s\n", dtmp1);

		if (EG91_Parse_ReceiveData(dtmp1, "OK"))
		{
			// free(rsp);
			//  free(&dtmp1);
			// ////printf("\n parse data %d\n", 1);

			// ////printf("\n parse data %d\n", 2);
			memset(dtmp1, 0, sizeof(dtmp1));
			// xQueueReset(AT_Command_Feedback_queue);
			// xSemaphoreGive(rdySem_Control_Send_AT_Command);
			return 1;
		}
		else
		{
			memset(dtmp1, 0, sizeof(dtmp1));
			counterACK--;
		}
		memset(dtmp1, 0, sizeof(dtmp1));
		vTaskDelay(pdMS_TO_TICKS((1000)));
	}
	return 0;
}

uint8_t EG91_Call_PHONE(char *phNumber)
{

	char AT_CMD[BUFF_SIZE];

	sprintf(AT_CMD, "%s%s", ATD, phNumber);

	EG91_send_AT_Command(AT_CMD, "OK", 500);
	vTaskDelay(pdMS_TO_TICKS((20000)));
	EG91_send_AT_Command("AT+CLCC", "CLCC", 500);
	vTaskDelay(pdMS_TO_TICKS((6000)));
	EG91_send_AT_Command("AT+QHUP=21", "OK", 500);
	vTaskDelay(pdMS_TO_TICKS((2000)));

	return 1;
}

int u8_wc_toutf8(char *dest, u_int32_t ch)
{

	// ////printf("\n\n ch - %d - %x", ch, ch);
	if (ch < 0x80)
	{
		dest[0] = (char)ch;
		dest[1] = 0;
		return 1;
	}
	if (ch < 0x800)
	{
		dest[0] = (ch >> 6) | 0xC0;
		dest[1] = (ch & 0x3F) | 0x80;
		dest[2] = 0;
		return 2;
	}
	if (ch < 0x10000)
	{
		dest[0] = (ch >> 12) | 0xE0;
		dest[1] = ((ch >> 6) & 0x3F) | 0x80;
		dest[2] = (ch & 0x3F) | 0x80;
		dest[3] = 0;
		return 3;
	}
	if (ch < 0x110000)
	{
		dest[0] = (ch >> 18) | 0xF0;
		dest[1] = ((ch >> 12) & 0x3F) | 0x80;
		dest[2] = ((ch >> 6) & 0x3F) | 0x80;
		dest[3] = (ch & 0x3F) | 0x80;
		dest[4] = 0;
		return 4;
	}
	return 0;
}

int conv_utf8_to_ucs2(const char *src, size_t len)
{
	iconv_t cb = iconv_open("UTF-16", "UTF-8");
	// ////printf("inRemains outRemains\n");
	if (cb == (iconv_t)(-1))
		return 0;

	uint16_t *outBuff[len + 1];
	char *pout = (char *)outBuff;
	size_t inRemains = len;
	size_t outRemains = len * sizeof(uint16_t);

	// ////printf("inRemains:%d outRemains:%d\n", (int)inRemains, (int)outRemains);

	size_t cvtlen = iconv(cb, (char **)&src, (size_t *)&inRemains, (char **)&pout, (size_t *)&outRemains);
	if (cvtlen == (size_t)-1)
	{
		// ////printf("error:%s, %d\n", strerror(errno), errno);
		goto out;
	}
	*pout = 0;

	// ////printf("inRemains:%d outRemains:%d cvtlen:%d\n", (int)inRemains, (int)outRemains, (int)cvtlen);

	/*   for (int i = 0; (i < len) && outBuff[i]; i++)
		  ////printf("0x%04x\n", (int)outBuff[i]); */

out:
	// if(outBuff)
	free(outBuff);
	iconv_close(cb);

	return 0;
}

uint8_t EG91_Send_SMS(char *phNumber, char *text)
{
	char SMS_text[BUFF_SIZE] = {};
	char SMS_phoneNumber[100] = {};
	uint8_t TimeOut = 5;

	char buffer[4];

	// ////printf("\n\n , hex = %0x\n\n", ',');

	utf8_encode(&buffer, (rune_t)'‚');

	char *line, *u8s;
	unsigned int *wcs;
	int len;
	char x[strlen(phNumber) * 4];
	char y[strlen(text) * 4];

	memset(x, 0, strlen(phNumber) * 4);

	int length = 0;
	int length_aux = utf8_string_length((utf8_string)phNumber);
	utf16_string utf16_phNumber = utf8_to_utf16((utf16_string)phNumber);

	length = 0;

	int count = 0;
	while (1)
	{

		if (utf16_phNumber[count] == '\0')
		{
			break;
		}

		length += sprintf(x + length, "%04X", (utf16_phNumber[count++]));
	}

	for (size_t i = 0; i < length; ++i)
	{
		// ////printf("%04x", (int)x[i]);
	}

	sprintf(SMS_phoneNumber, "%s%s%c%c", "AT+CMGS=\"", x, '\"', 13);

	length = 0;

	len = strlen(text);

	length_aux = utf8_string_length((utf8_string)text);
	utf16_string utf16_text = utf8_to_utf16((utf16_string)text);
	// ////printf("encode utf8 string codes: \n");
	// ////printf("\n\nX send sms number:\n%s\n\n", x);
	utf16_t x1[300];

	length = 0;
	// length_aux = utf8_string_length((utf8_string)utf16_text);

	/* if ((u_int32_t)(text[i]) < 128)
	{ */
	count = 0;
	while (1)
	{

		if (utf16_text[count] == '\0')
		{
			break;
		}

		length += sprintf(y + length, "%04X", utf16_text[count++]);
	}

	// utf8_to_utf16((utf8_t)y, strlen(y), x1, 300);

	sprintf(SMS_text, "%s", y);

	// ////printf("\n\n strlen SIM BALANCE 33 - %d\n\n", (strlen(y) - (strlen("AT+CMGS=\"") * 4) - 4));
	strcat(SMS_text, "$\0");

	strcat(SMS_text, SMS_phoneNumber);

	EG91_send_AT_Command("AT+CMGF=1", "OK", 1500);
	// vTaskDelay(pdMS_TO_TICKS((1000)));

	// vTaskDelay(pdMS_TO_TICKS((1000)));
	// EG91_send_AT_Command("AT+CSCS=\"UCS2\"", "OK", 1000);

	EG91_send_AT_Command("AT+CSMP=49,167,0,25", "OK", 1000);

	// timer_pause(TIMER_GROUP_1, TIMER_0);
	// vTaskSuspend(xHandle_Timer_VerSystem);

	// TODO: COMENTAR O GIVE EM BAIXO SE NAO FUNCIONAR!!

	xSemaphoreGive(rdySem_Control_Send_AT_Command);
	if (EG91_send_AT_Command(SMS_text, "CMGS:" /* "+CMGS:" */, 10000))
	{

		// ////printf("\n\n rsp verify_SMS_List 22\n\n");
		EG91_send_AT_Command("ATE1", "OK", 1000);
		// ////printf("\n\n rsp verify_SMS_List 22\n\n");
		//  xSemaphoreGive(rdySem_Control_Send_AT_Command);
		// verify_SMS_List();
		free(utf16_text);
		free(utf16_phNumber);
		/* vTaskResume(xHandle_Timer_VerSystem);
		timer_start(TIMER_GROUP_1, TIMER_0); */
		// EG91_send_AT_Command("ATS7=0", "OK", 1500);
		//  EG91_send_AT_Command("ATS7=0", "OK", 10000);
		//  xSemaphoreGive(rdySem_Lost_SMS);
		return 1;
	}
	else
	{
		// ////printf("\n SMS SEND FAIL\n");
		TimeOut--;
		// vTaskDelay(pdMS_TO_TICKS(2000));
	}
	// xSemaphoreGive(rdySem_Control_Send_AT_Command);
	free(utf16_phNumber);
	free(utf16_text);
	// verify_SMS_List();
	//  EG91_send_AT_Command("ATS7=0", "OK", 1500);
	// vTaskResume(xHandle_Timer_VerSystem);
	// timer_start(TIMER_GROUP_1, TIMER_0);
	/* }
	else
	{
		TimeOut--;
		vTaskDelay(pdMS_TO_TICKS(500));
	} */
	//}
	//  vTaskDelay(pdMS_TO_TICKS((700)));

	// uart_write_bytes(UART_NUM_1, SMS_text, strlen(SMS_text));
	// EG91_send_AT_Command("ATE1", "OK", 1000);
	// EG91_send_AT_Command("ATS7=0", "OK", 1000);
	return 0;
}

void task_EG91_Verify_Unread_SMS(void *pvParameter)
{
	char dtmp[BUF_SIZE];
	for (;;)
	{
		if (xQueueReceive(Lost_SMS_queue, &dtmp, portMAX_DELAY))
		{

			// ////printf("\ntask_Verify_Unread_SMS - %s\n", dtmp);
			//  xSemaphoreTake(rdySem_Lost_SMS, pdMS_TO_TICKS(40000));
			//  xSemaphoreGive(rdySem_Control_Send_AT_Command);
			parse_SMS_List(dtmp);
			// xSemaphoreGive(rdySem_Control_Send_AT_Command);
		}
	}
}

static const char *TAG = "uart_events";

uint8_t parse_Incoming_UDP_data(char *mqtt_data)
{

	unsigned char receive_UDP_data[1024] = {};
	char mqtt_topic[70] = {};
	uint8_t count = 0;
	uint8_t aux_count = 0;
	int strIndex = 0;

	// ////printf("\n\n sizeof(mqtt_data) -%s -> %d\n\n", mqtt_data, strlen(mqtt_data));
	for (size_t i = 0; i < strlen(mqtt_data); i++)
	{
		if (mqtt_data[i] == '"')
		{
			count++;
			strIndex = 0;
			// ////printf("\n\n incoming data count - %d\n\n", count);
		}
		else if (count == 1)
		{
			if (mqtt_data[i] == '/')
			{
				aux_count++;
			}

			if (aux_count >= 2)
			{

				mqtt_topic[strIndex++] = mqtt_data[i];
			}
		}
		else if (count == 3)
		{
			receive_UDP_data[strIndex++] = mqtt_data[i];
		}
	}
	/* size_t length = 0;
	while (receive_UDP_data[length] != '\0')
	{
		length++;
	} */

	printf("\n\n receive_UDP_data 1234 - %s -> %s\n\n", mqtt_topic, receive_UDP_data);

	// receive_UDP_data[strlen(receive_UDP_data) - 1] = 0;

	if (strlen(mqtt_data) > 0)
	{
		/* code */

		char *output_Data;
		unsigned char decrypted[1024] = {};

		char AES_IV[] = "1BzUeGptJ3SnvnIv";

		decrypt_aes_cbc_padding((unsigned char *)receive_UDP_data, strlen((char *)receive_UDP_data), &decrypted, (unsigned char *)AES_KEY1, (unsigned char *)AES_IV);

		remove_padding(decrypted, strlen((char *)decrypted));

		// ////printf("Dados descriptografados: %s - %d\n", decrypted, strlen((char *)decrypted));
		mqtt_information mqttInfo;
		memset(&mqttInfo, 0, sizeof(mqttInfo));
		sprintf(mqttInfo.topic, "%s", mqtt_topic);


		

		// printf("\n\n receive_UDP_data 9876 - %s -> %s\n\n", mqtt_topic, decrypted);
		output_Data = parseInputData(decrypted, UDP_INDICATION, NULL, NULL, NULL, NULL, &mqttInfo);

		// printf("\n\noutput_Data UDP - %s\n\n", output_Data);

		if (strstr("NTRSP", output_Data) == NULL)
		{
			// printf("\n\noutput_Data topic - %s\n\n", mqtt_topic);
			send_UDP_Package(output_Data, strlen(output_Data), mqtt_topic);
		}
		// }

		// free(decrypted);
		free(output_Data);
	}

	// sprintf(receive_UDP_AT, "%s%d", "AT+QIRD=", connectionID);

	// ////printf("\n\n AT+QIRD - %s\n\n", receive_UDP_data);
	//  EG91_send_AT_Command(receive_UDP_AT, "+QIRD", 3000);

	return 1;
}

uint8_t parse_qmtstat(char *stat_receiveData)
{
	uint8_t counter = 0;
	char erro_qmtstat = 0;

	for (size_t i = 0; i < strlen(stat_receiveData); i++)
	{
		if (stat_receiveData[i] == ',')
		{
			erro_qmtstat = stat_receiveData[i + 1];
			break;
		}
	}
	erro_qmtstat = erro_qmtstat - 48;

	if (erro_qmtstat != 0)
	{
		RSSI_LED_TOOGLE = MQTT_NOT_CONECT_LED_TIME;
		// update_ACT_TimerVAlue((double)RSSI_EXCELLENT_LED_TIME);
		updateSystemTimer(SYSTEM_TIMER_ALARM_STATE);
	}

	// ////printf("\n\n erro_qmtstat = %d\n\n", erro_qmtstat);

	return erro_qmtstat;
}

int encontrarSubstring(const char *str, const char *subStr)
{
	int i, j;

	for (i = 0; str[i] != '\0'; i++)
	{
		// Verificar se há uma correspondência no início da substring
		j = 0;
		while (subStr[j] != '\0' && str[i + j] == subStr[j])
		{
			j++;
		}

		// Se a substring foi encontrada, retorne a posição inicial
		if (subStr[j] == '\0')
		{
			return i;
		}
	}

	// Caso contrário, retorne -1 para indicar que a substring não foi encontrada
	return -1;
}

void task_EG91_Receive_UDP(void *pvParameter)
{
	char mqtt_receiveData[1024];
	char connectID[2] = {};
	char mqttData[200] = {};
	int count = 0;
	int strIndex = 0;
	uint8_t result = 0;

	for (;;)
	{
		if (xQueueReceive(receive_mqtt_queue, &mqtt_receiveData, portMAX_DELAY))
		{
			/* timer_pause(TIMER_GROUP_1, TIMER_0);
			 vTaskSuspend(xHandle_Timer_VerSystem);
	xSemaphoreGive(rdySem_Control_Send_AT_Command); */
			save_INT8_Data_In_Storage(NVS_QMT_LARGE_DATA_TIMER_LABEL, 1, nvs_System_handle);
			memset(connectID, 0, sizeof(connectID));
			memset(mqttData, 0, sizeof(mqttData));

			if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
			{
				if (encontrarSubstring(mqtt_receiveData, "+QMTSTAT:") != -1 /* && !strstr("QIURC", mqtt_receiveData) */)
				{
					// ////printf("\n\n QMTSTAT:\n\n");
					/* if (label_Reset_Password_OR_System == 2)
					{ */
					// ////printf("\n\n qmtstat -> %d\n\n", get_INT8_Data_From_Storage(NVS_QMTSTAT_LABEL, nvs_System_handle));
					save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, parse_qmtstat(mqtt_receiveData), nvs_System_handle);
					/* } */
				}
			}
			else
			{
			}

			if (encontrarSubstring(mqtt_receiveData, "RECV") != -1)
			{
				// ////printf("\n\nreceive UDP 2222 - %s\n\n", mqttData);
				xSemaphoreGive(rdySem_Control_Send_AT_Command);
				parse_Incoming_UDP_data(mqtt_receiveData);
			}
			else if (encontrarSubstring(mqtt_receiveData, "QIURC") != -1 || encontrarSubstring(mqtt_receiveData, "closed:") != -1)
			{
				if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS))
				{
					if (label_Reset_Password_OR_System == 2)
					{
						activateUDP_network();
					}
				}
			}
			save_INT8_Data_In_Storage(NVS_QMT_LARGE_DATA_TIMER_LABEL, 0, nvs_System_handle);
			count = 0;
			/* vTaskResume(xHandle_Timer_VerSystem);
						timer_start(TIMER_GROUP_1, TIMER_0); */
		}
	}
}

uint8_t parse_verify_mqtt_conection(char *receiveData)
{
	uint8_t auxCounter = 0;
	char result = 0;
	char ret_code = 0;

	for (size_t i = 0; i < strlen(receiveData); i++)
	{
		if (receiveData[i] == ',')
		{
			auxCounter++;
		}
		else
		{
			if (auxCounter == 1)
			{
				result = receiveData[i];
			}
			else if (auxCounter == 2)
			{
				ret_code = receiveData[i];
				break;
			}
		}
	}

	// ////printf("\n\n ret_code = %c / result = %c / data - %s\n\n", ret_code, result, receiveData);

	if (ret_code == '0' && result == '0')
	{
		return 1;
	}
	else
	{
		return 0;
	}

	return 0;
}

uint8_t parse_verify_mqtt_open(char *receiveData)
{
	char open_verify = 0;
	uint8_t counter = 0;

	for (size_t i = 0; i < strlen(receiveData); i++)
	{
		if (receiveData[i] == ',')
		{
			counter++;
			open_verify = receiveData[i - 1];
			break;
		}
	}

	// ////printf("\n\nopen_verify %c \n\n", open_verify);

	if ((open_verify == '0' || open_verify == '2') && counter != 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}

	return 0;
}

static void uart_event_task(void *pvParameters)
{
	uart_event_t event;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = false;
	int counter;
	int len;
	size_t buffered_size;
	char dtmp[BUF_SIZE] = {};
	char mqtt_data[508] = {};
	char SMS_Data[50] = {};
	char CLCC_Data[100] = {};
	int ring_buff_len;
	uint8_t data_len = 0;

	for (;;)
	{
		xQueueReset(uart0_queue);
		//   Waiting for UART event.
		if (xQueueReceive(uart0_queue, (void *)&event, portMAX_DELAY))
		{
			// xSemaphoreTake(rdySem_UART_CTR, 500 / portTICK_RATE_MS);
			// ////printf("\n!!!!!! read rsp bb !!!!!\n");

			bzero(dtmp, 600);
			//  //ESP_LOGI(TAG, "uart[%d] event:", UART_NUM_1);
			switch (event.type)
			{

			case UART_DATA:

				// vTaskSuspend(&FileBackup_task_handle);

				/* We have not woken a task at the start of the ISR. */

				vTaskDelay(100 / portTICK_PERIOD_MS); // 10 msec sleep
				memset(dtmp, 0, sizeof(dtmp));
				uart_get_buffered_data_len(UART_NUM_1, (size_t *)&ring_buff_len);
				// ////printf("\nread rsp aa %d\n", ring_buff_len);
				len = uart_read_bytes(UART_NUM_1, dtmp, /* 1024 */ ring_buff_len, 0);

				// xQueueReset(uart0_queue);
				//    ////printf("\nring_buff_len %d - len %d\n", ring_buff_len,len);
				printf("\nBUFFER UART : %s\n", dtmp);
				//  xQueueOverwrite(AT_Command_Feedback_queue,&dtmp);
				// //printf("\n%s\n", dtmp);
				if (/* ring_buff_len > */ 1)
				{
					if ((strstr(dtmp, "RING")) != NULL)
					{
						// ////printf("\nBUFFER UART RING 11: %s\n", dtmp);
						//  eTaskState incomingCall__task_status;

						if (aux_label_inCall == 0)
						{
							aux_label_inCall = 1;
							//printf("\nBUFFER UART RING 22: %s\n", dtmp);
							//   vTaskSuspend(handle_SMS_TASK);
							//   vTaskSuspend(handle_SEND_SMS_TASK);
							//  ////printf("\ndtmp ring\n%s\n", dtmp);
							xTaskCreate(task_EG91_Run_IncomingCall, "task_EG91_Run_IncomingCall", 10 * 1024, NULL, 25, &handle_INCOMING_CALL_TASK);
							// incomingCall_Label++;
						}

						break;
						//}AT_CUSD_Command
					}

					else if (strstr(dtmp, "CLCC") || (strstr(dtmp, "CMTI")) || /* strstr(dtmp, "AT") || */ /* strstr(dtmp, "+CMGS:") */ /* || */ strstr(dtmp, "CUSD:") || strstr(dtmp, "CME ERROR") || strstr(dtmp, "+QMTRECV:") || send_ATCommand_Label == 1 || strstr(dtmp, "+QHTTPGET:"))
					{

						if (AT_Command_Feedback_queue != NULL)
						{
							if ((strstr(dtmp, "CLCC")) != NULL)
							{

								int index_Found = -1;
								uint8_t strIndex = 0;

								index_Found = strpos(dtmp, "+CLCC:");

								memset(CLCC_Data, 0, sizeof(CLCC_Data));

								for (size_t i = index_Found; i < strlen(dtmp); i++)
								{

									if (dtmp[i] == '\r' || dtmp[i] == '\n')
									{
										break;
									}

									CLCC_Data[strIndex++] = dtmp[i];
								}

								// ////printf("\ndtmp CLCC\n%s\n", CLCC_Data);

								if (xQueueSendFromISR(EG91_CALL_CLCC_UART_queue,
													  (void *)&CLCC_Data,
													  xHigherPriorityTaskWoken) != pdPASS)
								{
									// ////printf("\n after LOG UART11 FAIL \n");
								}

								if (xHigherPriorityTaskWoken)
								{

									portYIELD_FROM_ISR();
								}
								// break;
								// }
							}
							else if (strstr(dtmp, "+QHTTPGET:") != NULL)
							{
								if (xQueueSendFromISR(HTTPS_data_queue,
													  (void *)&dtmp,
													  xHigherPriorityTaskWoken) != pdPASS)
								{
									// xSemaphoreGive(rdySem_Control_Send_AT_Command);
									// ////printf("\n after LOG UART11 sms FAIL \n");
								}

								// ////printf("\n after LOG UART11 sms FAIL 11 \n");
								if (xHigherPriorityTaskWoken)
								{
									/* Actual macro used here is port specific. */
									portYIELD_FROM_ISR();
								}
							}
							else if (strstr(dtmp, "AT+QMTOPEN?") != NULL || strstr(dtmp, "AT+QMTCONN?") != NULL)
							{

								// ////printf("\n qmtlog uart 2222 \n");

								if (xQueueSendFromISR(AT_Command_Feedback_queue,
													  (void *)&dtmp,
													  xHigherPriorityTaskWoken) != pdPASS)
								{
									// xSemaphoreGive(rdySem_Control_Send_AT_Command);
									// ////printf("\n after LOG UART11 sms FAIL \n");
								}

								// ////printf("\n after LOG UART11 sms FAIL 11 \n");
								if (xHigherPriorityTaskWoken)
								{
									/* Actual macro used here is port specific. */
									portYIELD_FROM_ISR();
								}
							}
							else if (/* (strstr(dtmp, "AT+QMTOPEN")) || strstr(dtmp, "AT+QMTCONN") ||  */ strstr(dtmp, "AT+QMTPUBEX") || strstr(dtmp, "AT+QMTSUB"))
							{
								// ////printf("\n\n +++QMT+++\n\n");

								if (strstr(dtmp, "+QMTPUBEX:"))
								{
									uint8_t UDP_data_count = 0;
									int index_Found = -1;
									uint8_t strIndex = 0;
									// ////printf("\n UDP_data_count - %d \n", UDP_data_count);
									index_Found = strpos(dtmp, "+QMTPUBEX:");
									memset(mqtt_data, 0, sizeof(mqtt_data));
									// ////printf("\n\n +QMT5555+++\n\n");
									for (size_t i = index_Found; i < strlen(dtmp); i++)
									{

										if (dtmp[i] == '\r' || dtmp[i] == '\n')
										{
											// ////printf("\n +QMTPUBEX: %d \n", UDP_data_count);
											UDP_data_count++;
										}

										if (UDP_data_count == 3)
										{
											break;
										}

										mqtt_data[strIndex++] = dtmp[i];
									}

									// ////printf("\n\nUDP DATA qmtcon - %s\n\n", mqtt_data);
									if (xQueueSendFromISR(AT_Command_Feedback_queue,
														  (void *)&mqtt_data,
														  xHigherPriorityTaskWoken) != pdPASS)
									{
										// xSemaphoreGive(rdySem_Control_Send_AT_Command);
										// ////printf("\n after LOG UART11 sms FAIL \n");
									}

									// ////printf("\n after LOG UART11 sms FAIL 11 \n");
									if (xHigherPriorityTaskWoken)
									{
										/* Actual macro used here is port specific. */
										portYIELD_FROM_ISR();
									}
									// ////printf("\n after LOG UART11 sms FAIL 22 \n");
								}

								// continue;
							}
							else if ((strstr(dtmp, "CMGL")))
							{

								if (xQueueSendFromISR(AT_Command_Feedback_queue,
													  (void *)&dtmp,
													  xHigherPriorityTaskWoken) != pdPASS)
								{
									// ////printf("\n after LOG UART11 FAIL \n");
								}

								if (xHigherPriorityTaskWoken)
								{

									portYIELD_FROM_ISR();
								}
								// break;
							}
							else if ((strstr(dtmp, "AT+QFOPEN=") != NULL) && (strstr(dtmp, "+QFOPEN:") == NULL))
							{
							}
							else if (send_ATCommand_Label == 1)
							{

								// ////printf("\ndtmp AT\n%s\n", dtmp);
								if (xQueueSendFromISR(AT_Command_Feedback_queue,
													  (void *)&dtmp,
													  xHigherPriorityTaskWoken) != pdPASS)
								{
									// ////printf("\n after LOG UART11 FAIL \n");
								}
								// xSemaphoreGiveFromISR(rdySem_Control_Send_AT_Command, xHigherPriorityTaskWoken);
								if (xHigherPriorityTaskWoken)
								{
									/* Actual macro used here is port specific. */
									portYIELD_FROM_ISR();
								}
								// memset(dtmp, 0, BUFF_SIZE);
								// break;
							}

							if (strstr(dtmp, "+QMTRECV:") != NULL)
							{
								int position[10] = {};
								uint8_t UDP_data_count = 0;
								int index_Found = -1;
								uint8_t strIndex = 0;
								char substring[] = "+QMTRECV:";

								int tamanhoString = strlen(dtmp);
								int tamanhoSubstring = strlen(substring);

								if (tamanhoString < tamanhoSubstring)
								{
									// ////printf("Erro: A substring é maior que a string principal.\n");
									return;
								}

								uint8_t subStr_counter = 0;
								for (int i = 0; i <= tamanhoString - tamanhoSubstring; ++i)
								{
									int j;
									for (j = 0; j < tamanhoSubstring; ++j)
									{
										if (dtmp[i + j] != substring[j])
										{
											break;
										}
									}

									if (j == tamanhoSubstring)
									{
										position[subStr_counter++] = i;
										// ////printf("\n\nposition %d\n\n ", i);
									}
								}

								/* index_Found = strpos(dtmp, "+QMTRECV:");
								memset(mqtt_data, 0, sizeof(mqtt_data)); */

								for (size_t j = 0; j < subStr_counter; j++)
								{
									memset(mqtt_data, 0, sizeof(mqtt_data));
									strIndex = 0;

									for (size_t i = position[j]; i < tamanhoString; i++)
									{
										// ////printf("\n\nposition1 %d\n\n ", position[j]);
										if (dtmp[i] == '\r' || dtmp[i] == '\n')
										{
											// ////printf("\n UDP_data_count - %c \n", dtmp[i]);
											//  UDP_data_count++;
											break;
										}

										/*  if (UDP_data_count == 1)
										 {
											 break;
										 } */

										mqtt_data[strIndex++] = dtmp[i];
									}

									mqtt_data[strIndex + 1] = 0;
									// ////printf("\n UDP_data_count - %s - %d \n", mqtt_data, strIndex);

									// ////printf("\n\nUDP DATA - %s\n\n", mqtt_data);
									if (xQueueSendFromISR(receive_mqtt_queue,
														  (void *)&mqtt_data,
														  xHigherPriorityTaskWoken) != pdPASS)
									{
										// xSemaphoreGive(rdySem_Control_Send_AT_Command);
										// ////printf("\n after LOG UART11 sms FAIL \n");
									}
								}

								// ////printf("\n after LOG UART11 sms FAIL 11 \n");
								if (xHigherPriorityTaskWoken)
								{
									/* Actual macro used here is port specific. */
									portYIELD_FROM_ISR();
								}
								// ////printf("\n after LOG UART11 sms FAIL 22 \n");
							}

							// ////printf("\n\n ***QMT*** - %s \n\n", dtmp);

							if (strstr(dtmp, "+QMTSUB:") != NULL)
							{

								uint8_t UDP_data_count = 0;
								int index_Found = -1;
								uint8_t strIndex = 0;
								// ////printf("\n UDP_data_count - %d \n", UDP_data_count);
								index_Found = strpos(dtmp, "+QMTSUB:");
								memset(mqtt_data, 0, sizeof(mqtt_data));

								for (size_t i = index_Found; i < strlen(dtmp); i++)
								{

									if (dtmp[i] == '\r' || dtmp[i] == '\n')
									{
										// ////printf("\n QMTSUB data - %d \n", UDP_data_count);
										UDP_data_count++;
									}

									if (UDP_data_count == 3)
									{
										break;
									}

									mqtt_data[strIndex++] = dtmp[i];
								}

								// ////printf("\n\nUDP DATA qmtcon - %s\n\n", mqtt_data);
								if (xQueueSendFromISR(AT_Command_Feedback_queue,
													  (void *)&mqtt_data,
													  xHigherPriorityTaskWoken) != pdPASS)
								{
									// xSemaphoreGive(rdySem_Control_Send_AT_Command);
									// ////printf("\n after LOG UART11 sms FAIL \n");
								}

								// ////printf("\n after LOG UART11 sms FAIL 11 \n");
								if (xHigherPriorityTaskWoken)
								{
									/* Actual macro used here is port specific. */
									portYIELD_FROM_ISR();
								}
								// ////printf("\n after LOG UART11 sms FAIL 22 \n");
							}
							if (strstr(dtmp, "+QMTSTAT:") != NULL)
							{

								uint8_t UDP_data_count = 0;
								int index_Found = -1;
								uint8_t strIndex = 0;

								index_Found = strpos(dtmp, "+QMTSTAT:");
								memset(mqtt_data, 0, sizeof(mqtt_data));

								for (size_t i = index_Found; i < strlen(dtmp); i++)
								{

									if (dtmp[i] == '\r' || dtmp[i] == '\n')
									{
										// ////printf("\n UDP_data_count - %d \n", UDP_data_count);
										UDP_data_count++;
									}

									if (UDP_data_count == 3)
									{
										mqtt_data[strIndex] = '\0';
										break;
									}

									mqtt_data[strIndex++] = dtmp[i];
								}

								// ////printf("\n\nUDP DATA - %s\n\n", mqtt_data);
								if (xQueueSendFromISR(receive_mqtt_queue,
													  (void *)&mqtt_data,
													  xHigherPriorityTaskWoken) != pdPASS)
								{
									// xSemaphoreGive(rdySem_Control_Send_AT_Command);
									// ////printf("\n after LOG UART11 sms FAIL \n");
								}

								// ////printf("\n after LOG UART11 sms FAIL 11 \n");
								if (xHigherPriorityTaskWoken)
								{
									/* Actual macro used here is port specific. */
									portYIELD_FROM_ISR();
								}
								// ////printf("\n after LOG UART11 sms FAIL 22 \n");
							}
							// ////printf("\n\n ***QMT4*** - %s \n\n", dtmp);
							if ((strstr(dtmp, "CMTI")))
							{
								// xSemaphoreGive(rdySem_Control_Send_AT_Command);
								// ////printf("\n receive sms11111\n");
								if (EG91_CALL_SMS_UART_queue != NULL)
								{
									// ////printf("\ndtmp sms\n%s\n", dtmp);
									int index_Found = -1;
									uint8_t strIndex = 0;

									index_Found = strpos(dtmp, "+CMTI:");
									memset(SMS_Data, 0, sizeof(SMS_Data));

									for (size_t i = index_Found; i < strlen(dtmp); i++)
									{

										if (dtmp[i] == '\r' || dtmp[i] == '\n')
										{
											break;
										}

										SMS_Data[strIndex++] = dtmp[i];
									}

									// ////printf("\n\n receive sms - %s\n\n", SMS_Data);
									//   xTaskCreate(task_EG91_Run_SMS, "task_EG91_Run_SMS", 20 * 1024, (void *)dtmp, 20, &handle_SMS_TASK);
									unsigned int q_size = uxQueueMessagesWaiting(EG91_CALL_SMS_UART_queue);
									// print_shit();
									unsigned int q_size1 = uxQueueMessagesWaiting(AT_Command_Feedback_queue);

									// ////printf("\n\n AT_Command_Feedback_queue size - %d\n\n", q_size1);

									// ////printf("\n\n q_size sms - %d\n\n", q_size);

									// if (q_size1 == 3/* AT_QUEUE_SIZE - 1 */)
									// {
									//     xQueueReset(AT_Command_Feedback_queue);
									// }

									// ////printf("\nsms receive\n");
									if (xQueueSendFromISR(EG91_CALL_SMS_UART_queue,
														  (void *)&SMS_Data,
														  (TickType_t)0) != pdPASS)
									{
										// ////printf("\nsms receive rreeww\n");
										//  xSemaphoreGive(rdySem_Control_Send_AT_Command);
										//  ////printf("\n after LOG UART11 sms FAIL \n");
									}
									// xSemaphoreGive(rdySem_Control_Send_AT_Command);
								}
								// break;
							}
							// ////printf("\n\n ***QMT5*** - %s \n\n", dtmp);
						}
					}
					// ////printf("\n\n ***QMT6*** - %s \n\n", dtmp);
					/* else if (strstr(dtmp, "+CUSD:"))
					{
						//////printf("\n CUSD ISR\n");
						if (xQueueSendFromISR(EG91_GET_SIM_BALANCE_queue,
											  (void *)&dtmp,
											  xHigherPriorityTaskWoken) != pdPASS)
						{
							//////printf("\n after LOG UART11 FAIL \n");
						}
						else
						{
							//////printf("\n UART CUSD ISR SEND\n");
						}

						if (xHigherPriorityTaskWoken)
						{

							portYIELD_FROM_ISR();
						}
						break;
					} */
					else if (strstr(dtmp, "NO CARRIER"))
					{
						// ////printf("\n UART NO CARRIER\n");
						if (xQueueSendFromISR(NO_CARRIER_Call_queue,
											  (void *)&dtmp,
											  xHigherPriorityTaskWoken) != pdPASS)
						{
							// ////printf("\n after LOG UART11 FAIL \n");
						}
						else
						{
							// ////printf("\n UART NO CARRIER SEND\n");
						}

						if (xHigherPriorityTaskWoken)
						{

							portYIELD_FROM_ISR();
						}
						break;
					}
					else if (strstr(dtmp, "+QPSND: 0"))
					{
						// ////printf("\n rdySem_QPSND give\n");
						xSemaphoreGive(rdySem_QPSND);
						break;
					}
					else if ((strstr(dtmp, "QFWRITE")) != NULL)
					{
						if (EG91_CALL_SMS_UART_queue != NULL)
						{
							// ////printf("\ndtmp sms\n%s\n", dtmp);

							if (EG91_WRITE_FILE_queue != NULL)
							{
								// ////printf("\nEG91_WRITE_FILE_queue != NULL\n");
								if (xQueueSendFromISR(EG91_WRITE_FILE_queue, (void *)&dtmp, xHigherPriorityTaskWoken) != pdPASS)
								{
									// ////printf("\n EG91_WRITE_FILE_queue \n");
								}
							}
							else
							{
								// ////printf("\nEG91_WRITE_FILE_queue == NULL\n");
							}
						}
						break;
					}
				}

				if (strstr(dtmp, "+QMTSTAT:") != NULL)
				{

					uint8_t UDP_data_count = 0;
					int index_Found = -1;
					uint8_t strIndex = 0;

					index_Found = strpos(dtmp, "+QMTSTAT:");
					memset(mqtt_data, 0, sizeof(mqtt_data));

					for (size_t i = index_Found; i < strlen(dtmp); i++)
					{

						if (dtmp[i] == '\r' || dtmp[i] == '\n')
						{
							// ////printf("\n UDP_data_count - %d \n", UDP_data_count);
							UDP_data_count++;
						}

						if (UDP_data_count == 3)
						{
							break;
						}

						mqtt_data[strIndex++] = dtmp[i];
					}

					// ////printf("\n\nUDP DATA - %s\n\n", mqtt_data);
					if (xQueueSendFromISR(receive_mqtt_queue,
										  (void *)&mqtt_data,
										  xHigherPriorityTaskWoken) != pdPASS)
					{
						// xSemaphoreGive(rdySem_Control_Send_AT_Command);
						// ////printf("\n after LOG UART11 sms FAIL \n");
					}

					// ////printf("\n after LOG UART11 sms FAIL 11 \n");
					if (xHigherPriorityTaskWoken)
					{
						/* Actual macro used here is port specific. */
						portYIELD_FROM_ISR();
					}
					// ////printf("\n after LOG UART11 sms FAIL 22 \n");
				}

				if (strstr(dtmp, "+QMTPING:") != NULL)
				{
					save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 1, nvs_System_handle);
				}

				// ////printf("\n\n ***QMT6*** - %s \n\n", dtmp);
				break;
			// Event of HW FIFO overflow detected
			case UART_FIFO_OVF:
				// ESP_LOGI(TAG, "hw fifo overflow");
				//  If fifo overflow happened, you should consider adding flow control for your application.
				//  The ISR has already reset the rx FIFO,
				//  As an example, we directly flush the rx buffer here in order to read more data.
				uart_flush_input(UART_NUM_1);
				xQueueReset(uart0_queue);
				break;
			// Event of UART ring buffer full
			case UART_BUFFER_FULL:
				// ESP_LOGI(TAG, "ring buffer full");
				//  If buffer full happened, you should consider encreasing your buffer size
				//  As an example, we directly flush the rx buffer here in order to read more data.
				uart_flush_input(UART_NUM_1);
				xQueueReset(uart0_queue);
				break;
			// Event of UART RX break detected
			case UART_BREAK:
				// ESP_LOGI(TAG, "uart rx break");
				break;
			// Event of UART parity check error
			case UART_PARITY_ERR:
				// ESP_LOGI(TAG, "uart parity error");
				break;
			// Event of UART frame error
			case UART_FRAME_ERR:
				// ESP_LOGI(TAG, "uart frame error");
				break;
			// UART_PATTERN_DET
			case UART_PATTERN_DET:
				uart_get_buffered_data_len(UART_NUM_1, &buffered_size);
				int pos = uart_pattern_pop_pos(UART_NUM_1);
				// ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
				if (pos == -1)
				{
					// There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
					// record the position. We should set a larger queue size.
					// As an example, we directly flush the rx buffer here.
					uart_flush_input(UART_NUM_1);
				}
				else
				{
					uart_read_bytes(UART_NUM_1, dtmp, pos, 100 / portTICK_PERIOD_MS);
					uint8_t pat[PATTERN_CHR_NUM + 1];
					memset(pat, 0, sizeof(pat));
					uart_read_bytes(UART_NUM_1, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
					// ESP_LOGI(TAG, "read data: %s", dtmp);
					// ESP_LOGI(TAG, "read pat : %s", pat);
				}
				break;
			// Others
			default:
				// ESP_LOGI(TAG, "uart event type: %d", event.type);
				break;
			}
			// xSemaphoreGive(rdySem_UART_CTR);
		}
	}
	// free(dtmp);
	//  dtmp = NULL;
	vTaskDelete(NULL);
}

void uartInit(void)
{
	// esp_log_level_set(TAG, "ESP_LOG_INFO");

	/* Configure parameters of an UART driver,
	 * communication pins and install the driver */
	uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB,
	};

	// Install UART driver, and get the queue.
	uart_driver_install(UART_NUM_1, BUF_SIZE, BUF_SIZE, 3, &uart0_queue, 0);
	uart_param_config(UART_NUM_1, &uart_config);

	// Set UART log level
	esp_log_level_set(TAG, ESP_LOG_INFO);
	// Set UART pins (using UART0 default pins ie no changes.)
	uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

	//gpio_set_pull_mode(RXD_PIN, GPIO_PULLUP_ONLY);
	// Set uart pattern detect function.
	// uart_enable_pattern_det_baud_intr(UART_NUM_1, 10, PATTERN_CHR_NUM, 9, 0, 0);
	// Reset the pattern queue length to record at most 20 pattern positions.
	// uart_pattern_queue_reset(UART_NUM_1, 20);

	// Create a task to handler UART event from ISR
	xTaskCreate(uart_event_task, "uart_event_task", 5 * 1024, NULL, 32, NULL);
}

uint8_t EG91_get_IMEI(/* char *IMEI */)
{
	EG91_send_AT_Command("AT+GSN", "GSN", 3000);

	if (strstr(EG91_IMEI, "ERROR") != NULL)
	{
		return 0;
	}

	// ////printf("\n\n\nEG91_get_IMEI - %s\n\n\n", EG91_IMEI);
	//   sprintf(IMEI, "%s", EG91_IMEI);
	save_STR_Data_In_Storage(NVS_KEY_EG91_IMEI, EG91_IMEI, nvs_System_handle);

	return 1;
}

uint8_t EG91_UDP_Ping()
{
	// char imei[30] = {};
	// char CMD = '$';
	// char rssi[3] = {};

	// /* if (xHandle_Timer_VerSystem != NULL)
	// {
	//     timer_pause(TIMER_GROUP_1, TIMER_0);
	//     vTaskSuspend(xHandle_Timer_VerSystem);
	// } */

	// char data[100] = {};
	// // ////printf("\n\ntask_refresh_SystemTime 02\n\n");
	// get_STR_Data_In_Storage(NVS_KEY_EG91_IMEI, nvs_System_handle, &data);
	// // ////printf("\n\ntask_refresh_SystemTime 03\n\n");

	// sprintf(data, "%s", "$ ");
	// // strcat(data," $ ");
	// // ////printf("\n\ntask_refresh_SystemTime 04\n\n");
	// sprintf(rssi, "%d", RSSI_VALUE);
	// strcat(data, rssi);
	// // ////printf("\n\ntask_refresh_SystemTime 05\n\n");

	// label_network_portalRegister = get_INT8_Data_From_Storage(NVS_NETWORK_PORTAL_REGISTER, nvs_System_handle);

	// if (label_network_portalRegister == 255)
	// {
	//     label_network_portalRegister = 1;
	//     save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER, label_network_portalRegister, nvs_System_handle);
	// }

	// if (label_network_portalRegister == 1)
	// {
	//     send_UDP_Package(data, strlen(data));
	//     // send_UDP_queue(data);
	// }

	// send_UDP_queue(data);

	/* if (xHandle_Timer_VerSystem != NULL)
	{
		//////printf("\n\ntask_refresh_SystemTime 06\n\n");
		vTaskResume(xHandle_Timer_VerSystem);
		timer_start(TIMER_GROUP_1, TIMER_0);
	} */

	return 1;
}

uint8_t EG91_Check_IF_Have_PIN()
{
	SIM_CARD_PIN_status = 0;
	EG91_send_AT_Command("AT+CPIN?", "CPIN", 5000);

	// ////printf("\n\n SIM CARD PIN = %d\n\n", SIM_CARD_PIN_status);
	return SIM_CARD_PIN_status;
}

uint8_t EG91_Unlock_SIM_PIN(char *lastPIN)
{
	char atCommand[100] = {};

	sprintf(atCommand, "%s%s%c", "AT+CLCK=\"SC\",0,\"", lastPIN, '\"');

	if (EG91_send_AT_Command(atCommand, "OK", 1000))
	{
		EG91_initNetwork();
		return 1;
	}
	else
	{
		return 0;
	}

	return 0;
}

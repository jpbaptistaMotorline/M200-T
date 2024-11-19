/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#include "routines.h"
#include "nvs.h"
#include "unity.h"
#include "cron.h"
#include "jobs.h"
#include <sys/time.h>
#include <time.h>
#include "erro_list.h"
#include "rele.h"
#include "cmd_list.h"
#include "ble_spp_server_demo.h"
#include "esp_gatts_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "users.h"

uint8_t label_Cron_Init = 0;
TaskHandle_t task_Routine_BiState_RelayState1;
TaskHandle_t task_Routine_BiState_RelayState2;
allUsers_parameters_Message Routines_message;
SemaphoreHandle_t rdySem_Send_Routines;
SemaphoreHandle_t rdySem_Routine_BiState_Send_RelayState1;
SemaphoreHandle_t rdySem_Routine_BiState_Send_RelayState2;

uint8_t label_Routine1_ON;
uint8_t label_Routine2_ON;
uint8_t routines_ID;

struct list_node *routine_list;

char file_contents_routines[200];
char file_contents[200];

char *parse_RoutineData(uint8_t BLE_SMS_Indication, uint8_t gattsIF, uint16_t connID, uint16_t handle_table, char cmd, char param, char *payload)
{

    char *rsp = NULL;
    /*memset(&rsp, 0, sizeof(rsp));*/

    if (cmd == SET_CMD)
    {
        if (param == ROUTINE_PARAMETER)
        {
            label_BLE_UDP_send = 0;
            asprintf(&rsp, "%s", setRoutine(payload));
            return rsp;
        }
        else if (param == ROUTINE_RANGE_PARAMETER)
        {
            /*memset(&rsp, 0, sizeof(rsp));*/
            label_BLE_UDP_send = 0;
            asprintf(&rsp, "%s %c %c %s", ROUTINE_ELEMENT, cmd, param, set_Routine_Range(payload));
            return rsp;
        }
        else if (param == ROUTINE_DAY_EXCEPTION_PARAMETER)
        {
            /*memset(&rsp, 0, sizeof(rsp));*/
            label_BLE_UDP_send = 0;
            asprintf(&rsp, "%s %c %c %s", ROUTINE_ELEMENT, cmd, param, set_ExeptionDay(payload));
            return rsp;
        }
        else if (param == ROUTINE_ACTIVATE_PARAMETER)
        {

            asprintf(&rsp, "%s", activate_Routines(BLE_SMS_Indication, gattsIF, connID, handle_table));
            return rsp;
        }
        else if (param == ROUTINE_HOLIDAYS_MOBILE_DAYS)
        {
            label_BLE_UDP_send = 0;
            asprintf(&rsp, "%s", set_holidaysDays(atoi(payload)));
            return rsp;
        }
        else if (param == ROUTINES_HOLIDAYS_RANGE_PARAMETER)
        {
            label_BLE_UDP_send = 0;
            asprintf(&rsp, "%s %c %c %s", ROUTINE_ELEMENT, cmd, param, set_rangeHolidaysDays(payload));
            return rsp;
        }
        else
        {
            return return_ERROR_Codes(&rsp, ERROR_PARAMETER);
        }
    }
    else if (cmd == GET_CMD)
    {
        if (param == ROUTINE_PARAMETER)
        {
            // ////printf("\n enter get routines 1\n");
            asprintf(&rsp, "%s", getRoutines(gattsIF, connID, handle_table, BLE_SMS_Indication));
            return rsp;
        }
        else if (param == ROUTINE_RANGE_PARAMETER)
        {
            char range_InicialTime[7] = {};
            char range_FinalTime[7] = {};

            if (get_Routine_Range(&range_InicialTime, &range_FinalTime))
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                asprintf(&rsp, "%s %c %c %s;%s", ROUTINE_ELEMENT, cmd, param, range_InicialTime, range_FinalTime);

                // ////printf("\n\n get_Routine_Range - %s\n\n", rsp);
                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, "RT G T NO RANGE");
            }
        }
        else if (param == ROUTINES_HOLIDAYS_RANGE_PARAMETER)
        {
            char range_InicialTime[7] = {};
            char range_FinalTime[7] = {};

            if (get_rangeHolidaysDays(&range_InicialTime, &range_FinalTime))
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                asprintf(&rsp, "%s %c %c %s;%s", ROUTINE_ELEMENT, cmd, param, range_InicialTime, range_FinalTime);

                // ////printf("\n\n get_Routine_Range - %s\n\n", rsp);
                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, "RT G F NO RANGE");
            }
        }
        else if (param == ROUTINE_DAY_EXCEPTION_PARAMETER)
        {
            /*memset(&rsp, 0, sizeof(rsp));*/
            asprintf(&rsp, "%s %c %c %s", ROUTINE_ELEMENT, cmd, param, get_ExeptionDay());
            return rsp;
        }
        else if (param == ROUTINE_HOLIDAYS_MOBILE_DAYS)
        {
            /*memset(&rsp, 0, sizeof(rsp));*/
            asprintf(&rsp, "%s %c %c %s", ROUTINE_ELEMENT, cmd, param, get_holidaysDays());
            return rsp;
        }
        else
        {
            return return_ERROR_Codes(&rsp, ERROR_PARAMETER);
        }
    }
    else if (cmd == RESET_CMD)
    {
        if (param == ROUTINE_PARAMETER)
        {
            label_BLE_UDP_send = 0;
            asprintf(&rsp, "%s", eraseRoutines());
            return rsp;
        }
        else if (param == ROUTINE_RANGE_PARAMETER)
        {
            label_BLE_UDP_send = 0;
            asprintf(&rsp, "%s %c %c %s", ROUTINE_ELEMENT, cmd, param, erase_Routine_Range());
            return rsp;
        }
        else if (param == ROUTINES_HOLIDAYS_RANGE_PARAMETER)
        {
            label_BLE_UDP_send = 0;
            asprintf(&rsp, "%s %c %c %s", ROUTINE_ELEMENT, cmd, param, reset_rangeHolidaysDays());
            return rsp;
        }
        else if (param == ROUTINE_DAY_EXCEPTION_PARAMETER)
        {
            label_BLE_UDP_send = 0;
            asprintf(&rsp, "%s %c %c %s", ROUTINE_ELEMENT, cmd, param, erase_ExeptionDay());
            return rsp;
        }
        else if (param == ROUTINE_HOLIDAYS_MOBILE_DAYS)
        {
            label_BLE_UDP_send = 0;
            asprintf(&rsp, "%s %c %c %s", ROUTINE_ELEMENT, cmd, param, reset_holidaysDays());
            return rsp;
        }
        else
        {
            return return_ERROR_Codes(&rsp, ERROR_PARAMETER);
        }
    }

    return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
}

char *set_holidaysDays(uint8_t holidaysDays)
{
    // ////printf("\n\n holidaysDays . %d\n\n", holidaysDays);
    if (holidaysDays & CARNIVAL_HOLYDAY_DAY)
    {
        // ////printf("\n\n set carnaval \n\n");
        set_carnival();
    }

    if (holidaysDays & EASTER_FRIDAY_HOLYDAY_DAY)
    {
        // ////printf("\n\n set sexta santa \n\n");
        set_easter_friday();
    }

    if (holidaysDays & EASTER_HOLYDAY_DAY)
    {
        // ////printf("\n\n set pascoa \n\n");
        set_easter_day();
    }

    if (holidaysDays & CORPUS_CHRISTI_HOLYDAY_DAY)
    {
        // ////printf("\n\n set corpo de cristo \n\n");
        set_corpusChristi();
    }

    if (holidaysDays & EASTER_MONDAY_HOLYDAY_DAY)
    {
        // ////printf("\n\n set corpo de cristo \n\n");
        set_easter_monday();
    }

    return "RT S H OK";
}

char *reset_holidaysDays()
{
    if (nvs_erase_all(nvs_Mobile_Holydays_handle) == ESP_OK)
    {
        nvs_erase_key(nvs_System_handle, NVS_KEY_LABEL_CARNIVAL);
        nvs_erase_key(nvs_System_handle, NVS_KEY_LABEL_EASTER_FRIDAY);
        nvs_erase_key(nvs_System_handle, NVS_KEY_LABEL_EASTER);
        nvs_erase_key(nvs_System_handle, NVS_KEY_LABEL_CORPUS_CHRISTI);
        nvs_erase_key(nvs_System_handle, NVS_KEY_LABEL_EASTER_MONDAY);
        // ////printf("\nreset reset_holidaysDays\n");
        return "OK";
    }
    else
    {
        // ////printf("\nxep days ERROR\n");
        return "ERROR";
    }
}

char *get_holidaysDays()
{

    uint8_t get_holidays = 0;

    if (get_INT8_Data_From_Storage(NVS_KEY_LABEL_CARNIVAL, nvs_System_handle) == 1)
    {
        get_holidays = get_holidays + pow(2, 0);
    }

    if (get_INT8_Data_From_Storage(NVS_KEY_LABEL_EASTER_FRIDAY, nvs_System_handle) == 1)
    {
        get_holidays = get_holidays + pow(2, 1);
    }

    if (get_INT8_Data_From_Storage(NVS_KEY_LABEL_EASTER, nvs_System_handle) == 1)
    {
        get_holidays = get_holidays + pow(2, 2);
    }

    if (get_INT8_Data_From_Storage(NVS_KEY_LABEL_CORPUS_CHRISTI, nvs_System_handle) == 1)
    {
        get_holidays = get_holidays + pow(2, 3);
    }

    if (get_INT8_Data_From_Storage(NVS_KEY_LABEL_EASTER_MONDAY, nvs_System_handle) == 1)
    {
        get_holidays = get_holidays + pow(2, 4);
    }

    sprintf(file_contents, "%d", get_holidays);
    return file_contents;
}

char *setRoutine(char *payload)
{
    // ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
    char *cron_Expression = (char *)malloc(strlen(payload));
    char *cron_data = (char *)malloc(3);
    memset(cron_Expression, 0, strlen(payload));
    memset(cron_data, 0, 3);
    uint8_t dot_Counter = 0;
    uint8_t strIndex = 0;
    routines_ID = 0;
    char *nvs_key;

    for (int i = 0; i < strlen(payload); i++)
    {
        if (payload[i] == ';')
        {
            dot_Counter++;
            strIndex = 0;
        }
        else
        {
            if (dot_Counter == 0)
            {
                memcpy(cron_Expression + strIndex, payload + i, 1);
                // cron_Expression[strIndex] = payload[i];
                strIndex++;
            }
            else if (dot_Counter == 1)
            {
                memcpy(cron_data + strIndex, payload + i, 1);

                // cron_data+strIndex = payload[i];
                strIndex++;
            }
            else
            {
                return ERROR_INPUT_DATA;
            }
        }
    }

    // ////printf("\nset routine payload - %s\n", payload);
    // ////printf("\nset routine cron_Expression - %s, %d\n", cron_Expression, strlen(cron_Expression));
    // ////printf("\nset routine cron_data - %s\n", cron_data);

    routines_ID = get_INT8_Data_From_Storage(NVS_ROUTINES_ID, nvs_System_handle);
    // ////printf("\nset routine routines_ID11 - %d\n", routines_ID);

    if (routines_ID == 255)
    {
        routines_ID = 0;
    }
    else
    {
        routines_ID++;
    }

    // ////printf("\nset routine routines_ID11 - %d\n", routines_ID);
    //   routine_list->data = &cron_data;
    // ////printf("\nset routine routines_ID1222 - %d\n", routines_ID);

    // routine_list = list_insert_beginning(routine_list, cron_Expression, cron_data);
    //   memcpy(routine_list->data,&cron_data,sizeof(cron_data));
    /* //////printf("\nroutine_list->data - %s\n", routine_list->data);*/
    // ////printf("\n cron_Expression %s - %s\n", cron_Expression, cron_data);

    cron_job_create(cron_Expression, test_cron_job_sample_callback, (void *)cron_data);

    // ////printf("\nset routine 22 \n");
    asprintf(&nvs_key, "%d", routines_ID);
    save_STR_Data_In_Storage(nvs_key, payload, nvs_Routines_handle);
    // ////printf("\nset routine 33 \n");
    save_INT8_Data_In_Storage(NVS_ROUTINES_ID, routines_ID, nvs_System_handle);
    // ////printf("\nset routine 44 \n");
    free(nvs_key);
    free(cron_Expression);
    // ////printf("\nset routine 55 \n");
    free(cron_data);
    // ////printf("\nset routine 66 \n");
    /*  ESP_ERROR_CHECK(heap_trace_stop());
     heap_trace_dump(); */
    return "RT S R OK";
}

char *getRoutines(uint8_t gattsIF, uint16_t connID, uint16_t handle_table, uint8_t BLE_UDP_Indication)
{
    memset(&Routines_message, 0, sizeof(Routines_message));

    Routines_message.gattsIF = gattsIF;
    Routines_message.connID = connID;
    Routines_message.handle_table = handle_table;
    Routines_message.BLE_UDP_Indication = BLE_UDP_Indication;

    // ////printf("\n enter get routines 3 - %d ; %d ; %d\n", Routines_message.gattsIF, Routines_message.connID, Routines_message.handle_table);
    xQueueSend(send_Routines_queue, (void *)&Routines_message, pdMS_TO_TICKS(1000));
    // xSemaphoreTake(rdySem_Send_Routines,pdMS_TO_TICKS(3000));

    return "NTRSP";
}

char *eraseRoutines()
{

    // ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
    // ////printf("\nERASE 1\n");

    if (task_Routine_BiState_RelayState1 != NULL)
    {
        // ////printf("\nERASE 31\n");

        // xTaskAbortDelay(task_Routine_BiState_RelayState1);
        // gpio_set_level(GPIO_OUTPUT_IO_0, 0);
    }

    if (task_Routine_BiState_RelayState2 != NULL)
    {
        // ////printf("\nERASE 32\n");

        // xTaskAbortDelay(task_Routine_BiState_RelayState2);
        // gpio_set_level(GPIO_OUTPUT_IO_1, 0);
    }

    // label_MonoStableRelay1 = 0;
    // label_MonoStableRelay2 = 0;

    cron_stop();
    cron_job_clear_all();

    /* //////printf("\nERASE 2\n");
    list_destroy(&routine_list);
    //////printf("\nERASE 3\n");
    list_print(routine_list);
    routine_list = (list_node *)malloc(sizeof(list_node));
    routine_list->next = NULL; */
    /* routine_list->data = NULL;
   routine_list->cron_expression = NULL; */

    routines_ID = 0;
    save_INT8_Data_In_Storage(NVS_ROUTINES_ID, routines_ID, nvs_System_handle);
    // ////printf("\nERASE 4\n");
    if (nvs_erase_all(nvs_Routines_handle) == ESP_OK)
    {
        // ////printf("\nERASE 5\n");
        label_Routine2_ON = 0;
        label_Routine1_ON = 0;
        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, 0, nvs_System_handle);
        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_TIME_ON, 0, nvs_System_handle);
        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_LABEL, 0, nvs_System_handle);
        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_TIME_ON, 0, nvs_System_handle);
        return "RT R R OK";
    }
    else
    {
        // ////printf("\nERASE 6\n");
        return "RT R R ERROR";
    }

    /*  ESP_ERROR_CHECK(heap_trace_stop());
     heap_trace_dump(); */

    return "RT R R ERROR";
}

char *set_ExeptionDay(char *payload)
{
    char exeption_day[7] = {};
    uint8_t strIndex = 0;
    for (int i = 0; i < strlen(payload); i++)
    {
        if (payload[i] == ';')
        {
            // ////printf("\nset_ExeptionDay1 - %s\n", exeption_day);
            if (save_INT8_Data_In_Storage(exeption_day, 1, nvs_Exeption_Days_handle) != ESP_OK)
            {
                return "ERROR";
            }
            strIndex = 0;
            memset(exeption_day, 0, 7);
        }
        else
        {
            exeption_day[strIndex] = payload[i];
            strIndex++;
        }
    }
    // ////printf("\nset_ExeptionDay2 - %s\n", exeption_day);
    if (save_INT8_Data_In_Storage(exeption_day, 1, nvs_Exeption_Days_handle) != ESP_OK)
    {
        return "ERROR";
    }
    strIndex = 0;
    memset(exeption_day, 0, 7);

    return "OK";
}

char *erase_ExeptionDay()
{

    if (nvs_erase_all(nvs_Exeption_Days_handle) == ESP_OK)
    {
        // ////printf("\nexep days\n");
        return "OK";
    }
    else
    {
        // ////printf("\nxep days ERROR\n");
        return "ERROR";
    }
}

char *set_Routine_Range(char *payload)
{
    char inicialTime[7] = {};
    char finalTime[7] = {};
    uint8_t dot_label = 0;
    uint8_t strIndex = 0;

    for (int i = 0; i < strlen(payload); i++)
    {
        if (payload[i] == ';')
        {
            dot_label = 1;
            strIndex = 0;
        }
        else
        {
            if (dot_label == 0)
            {
                inicialTime[strIndex] = payload[i];
                strIndex++;
            }
            else if (dot_label == 1)
            {
                finalTime[strIndex] = payload[i];
                strIndex++;
            }
            else
            {
                return "ERROR";
            }
        }
    }

    if (atoi(inicialTime) > 0 && atoi(inicialTime) <= 991131 && (strlen(inicialTime) == 6))
    {
        save_STR_Data_In_Storage(NVS_ROUTINES_RANGE_T0, inicialTime, nvs_System_handle);
    }
    else
    {
        return "ERROR INICIAL TIME";
    }

    if (atoi(finalTime) > 0 && atoi(finalTime) <= 991131 && (strlen(finalTime) == 6))
    {
        save_STR_Data_In_Storage(NVS_ROUTINES_RANGE_T1, finalTime, nvs_System_handle);
    }
    else
    {
        return "ERROR FINAL TIME";
    }

    return "OK";
}

char *erase_Routine_Range()
{
    esp_err_t err = nvs_erase_key(nvs_System_handle, NVS_ROUTINES_RANGE_T0);

    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        return "ERROR";
    }

    err = nvs_erase_key(nvs_System_handle, NVS_ROUTINES_RANGE_T1);

    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        return "ERROR";
    }

    return "OK";
}
#include "cron.h"
#include "jobs.h"
void test_cron_job_sample_callback(cron_job *job)
{
    //ESP_LOGI("TAG", "xPortGetFreeHeapSize  parseInputData1111: %d", xPortGetFreeHeapSize());
    //ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
    //ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    //ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));

    time_t now;
    struct tm timeinfo;
    char buffer[256], buffer2[256];
    int buffer_len = 256;
    char *cron_data;
    char ex_Day[20] = {};
    char date_Day[20] = {};
    asprintf(&cron_data, "%s", (char *)job->data);
    // cron_data = (char *)job->data;

    time(&now);
    /* setenv("TZ", "UTC1", 1);
    tzset(); */
    localtime_r(&now, &timeinfo);
    strftime(buffer, buffer_len, "%c", &timeinfo);
    /* setenv("TZ", "UTC1", 1);
    tzset(); */

    sprintf(ex_Day, "%02d%02d", timeinfo.tm_mday, timeinfo.tm_mon);
    sprintf(date_Day, "%02d%02d%02d", (timeinfo.tm_year - 100), timeinfo.tm_mon, timeinfo.tm_mday);

    localtime_r(&(job->next_execution), &timeinfo);
    strftime(buffer2, buffer_len, "%c", &timeinfo);

    // ////printf("\n (char *)job->data - %s\n", (char *)job->data);

    // ////printf("\n\n ex_Day - %s\n\n", ex_Day);
    // ////printf("\n\n date_Day - %s\n\n", date_Day);

    uint8_t value = 0;
    uint8_t range_Label = 0;
    uint8_t range_holidaysLabel = 0;
    char range_InicialTime[7] = {};
    char range_FinalTime[7] = {};
    char range_holidaysInicialTime[7] = {};
    char range_holydaysFinalTime[7] = {};

    if (get_Routine_Range(&range_InicialTime, &range_FinalTime))
    {

        // ////printf("\n inicial range - %s / final range - %s\n", range_InicialTime, range_FinalTime);
        if (atoi(date_Day) >= atoi(range_InicialTime) && atoi(date_Day) <= atoi(range_FinalTime))
        {
            range_Label = 1;
            // ////printf("\n\n range_Label = 1\n\n");
        }
        else
        {
            // ////printf("\n\n range_Label = 0\n\n");
            range_Label = 0;
        }
    }
    else
    {
        range_Label = 1;
    }

    // verifica se é dia de exceção
    if (get_rangeHolidaysDays(&range_holidaysInicialTime, &range_holydaysFinalTime))
    {
        if (atoi(date_Day) >= atoi(range_holidaysInicialTime) && atoi(date_Day) <= atoi(range_holydaysFinalTime))
        {
            range_holidaysLabel = 0;
            // ////printf("\n\n range_Label = 1\n\n");
        }
        else
        {
            // ////printf("\n\n range_Label = 0\n\n");
            range_holidaysLabel = 1;
        }
    }
    else
    {
        range_holidaysLabel = 1;
    }

    // verifica se é dia de exceção

    esp_err_t err = nvs_get_u8(nvs_Exeption_Days_handle, ex_Day, &value);
    esp_err_t err_holydays = nvs_get_u8(nvs_Mobile_Holydays_handle, ex_Day, &value);

    if (err != ESP_OK)
    {
        if (err_holydays != ESP_OK)
        {

            if (range_Label == 1 && range_holidaysLabel == 1)
            {

                sdCard_Logs_struct logs_struct;
                memset(&logs_struct, 0, sizeof(logs_struct));
                if (job->data[0] == '1')
                {
                    if (job->data[1] == '0')
                    {
                        label_Routine1_ON = 0;

                        gpio_set_level(GPIO_OUTPUT_IO_0, 0);
                        // TODO: INSERIR NO CODIGO DO M200
                        save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, 0, nvs_System_handle);
                        label_Routine1_ON = 0;
                        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, 0, nvs_System_handle);
                        nvs_set_u64(nvs_System_handle, NVS_KEY_ROUTINE1_TIME_ON, 0);

                        BLE_Broadcast_Notify("R1 S R 0");
                    }
                    else if (job->data[1] == '1')
                    {
                        label_Routine1_ON = 1;
                        gpio_set_level(GPIO_OUTPUT_IO_0, 1);

                        save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, 1, nvs_System_handle);
                        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, 1, nvs_System_handle);

                        /**********************************************************/

                        // TODO: INSERIR NO CODIGO DO M200

                        struct cron_job_node *jobNode = cron_job_list_first();

                        uint64_t auxTime_routine = -1;

                        ////printf("\n\nROUTINE TIME 12345: %ld - %d\n\n", jobNode->job->next_execution, jobNode->job->id);

                        while (jobNode != NULL)
                        {
                            if (!strcmp(jobNode->job->data, "10") || !strcmp(jobNode->job->data, "12"))
                            {
                                if (jobNode->job->next_execution < auxTime_routine)
                                {
                                    auxTime_routine = jobNode->job->next_execution;
                                }
                            }

                            ////printf("\n\nROUTINE TIME AFTER122: %ld - %d\n\n", jobNode->job->next_execution, jobNode->job->id);

                            ////printf("\n\nROUTINE EXPRESSION: %hhn,%hhn,%hhn\n\n", jobNode->job->expression.seconds, jobNode->job->expression.minutes, jobNode->job->expression.hours);

                            jobNode = jobNode->next;
                        }

                        ////printf("\n\nROUTINE TIME final: %lld\n\n", auxTime_routine);
                        nvs_set_u64(nvs_System_handle, NVS_KEY_ROUTINE1_TIME_ON, auxTime_routine);

                        free(jobNode);

                        /************************************************************/
                        BLE_Broadcast_Notify("R1 S R 1");
                        // ////printf("\n\nGPIO_OUTPUT_IO_0 job->data[1] == '1'\n\n");
                    }
                    else if (job->data[1] == '2')
                    {
                        if (label_MonoStableRelay1 != 1)
                        {
                            // TODO: INSERIR NO CODIGO DO M200
                            xSemaphoreGive(rdySem_Routine_BiState_Send_RelayState1);
                            save_INT8_Data_In_Storage(NVS_KEY_RELAY1_LAST_VALUE, 0, nvs_System_handle);
                            label_Routine1_ON = 0;
                            save_INT8_Data_In_Storage(NVS_KEY_ROUTINE1_LABEL, 0, nvs_System_handle);
                            nvs_set_u64(nvs_System_handle, NVS_KEY_ROUTINE1_TIME_ON, 0);
                        }
                    }

                    if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) || network_Activate_Flag == 1)
                    {

                        memset(&logs_struct, 0, sizeof(logs_struct));

                        sprintf(logs_struct.type, "%s", "");
                        sprintf(logs_struct.name, "%s", return_Json_SMS_Data("ROUTINE"));
                        sprintf(logs_struct.phone, "%s", "");
                        sprintf(logs_struct.relay, "%s", "R1");

                        if (job->data[1] == '0')
                        {
                            sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("OFF"));
                            // ////printf("\n\n ROUTINES logs_struct.relay_state %s\n\n", logs_struct.relay_state);
                        }
                        else if (job->data[1] == '1')
                        {
                            sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("ON"));
                            // ////printf("\n\n ROUTINES logs_struct.relay_state %s\n\n", logs_struct.relay_state);
                        }
                        else if (job->data[1] == '2')
                        {
                            sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("PULSE"));
                        }

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
                    }
                }
                else if (job->data[0] == '2')
                {
                    if (job->data[1] == '0')
                    {
                        label_Routine2_ON = 0;
                        gpio_set_level(GPIO_OUTPUT_IO_1, 0);
                        // TODO: INSERIR NO CODIGO DO M200
                        save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, 0, nvs_System_handle);
                        label_Routine2_ON = 0;
                        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_LABEL, 0, nvs_System_handle);
                        nvs_set_u64(nvs_System_handle, NVS_KEY_ROUTINE2_TIME_ON, 0);

                        BLE_Broadcast_Notify("R2 S R 0");
                    }
                    else if (job->data[1] == '1')
                    {
                        label_Routine2_ON = 1;
                        gpio_set_level(GPIO_OUTPUT_IO_1, 1);

                        save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, 1, nvs_System_handle);
                        save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_LABEL, 1, nvs_System_handle);

                        /* ////printf("\n\nROUTINE TIME AFTER1: %d - %hhn\n\n", job->id, job->expression.minutes); */
                        ////printf("\n\nROUTINE TIME AFTER2:\n\n");

                        /**********************************************************/

                        // TODO: INSERIR NO CODIGO DO M200

                        // STATIC FUNCTION DECLARATIONS
                        struct cron_job_node *jobNode = cron_job_list_first();

                        uint64_t auxTime_routine = -1;

                        ////printf("\n\nROUTINE TIME 12345: %ld - %d\n\n", jobNode->job->next_execution, jobNode->job->id);

                        while (jobNode != NULL)
                        {
                            if (!strcmp(jobNode->job->data, "20") || !strcmp(jobNode->job->data, "22"))
                            {
                                if (jobNode->job->next_execution < auxTime_routine)
                                {
                                    auxTime_routine = jobNode->job->next_execution;
                                }
                            }

                            ////printf("\n\nROUTINE TIME AFTER122: %ld - %d\n\n", jobNode->job->next_execution, jobNode->job->id);

                            ////printf("\n\nROUTINE EXPRESSION: %hhn,%hhn,%hhn\n\n", jobNode->job->expression.seconds, jobNode->job->expression.minutes, jobNode->job->expression.hours);

                            jobNode = jobNode->next;
                        }

                        nvs_set_u64(nvs_System_handle, NVS_KEY_ROUTINE2_TIME_ON, auxTime_routine);
                        ////printf("\n\nROUTINE TIME final: %lld\n\n", auxTime_routine);

                        free(jobNode);
                        /************************************************************/

                        BLE_Broadcast_Notify("R2 S R 1");
                        // ////printf("\n\nGPIO_OUTPUT_IO_1 job->data[1] == '1'\n\n");
                    }
                    else if (job->data[1] == '2')
                    {
                        if (label_MonoStableRelay2 != 1)
                        { // TODO: INSERIR NO CODIGO DO M200
                            xSemaphoreGive(rdySem_Routine_BiState_Send_RelayState2);
                            save_INT8_Data_In_Storage(NVS_KEY_RELAY2_LAST_VALUE, 0, nvs_System_handle);
                            label_Routine2_ON = 0;
                            save_INT8_Data_In_Storage(NVS_KEY_ROUTINE2_LABEL, 0, nvs_System_handle);
                            nvs_set_u64(nvs_System_handle, NVS_KEY_ROUTINE2_TIME_ON, 0);
                        }
                    }

                    if (!gpio_get_level(GPIO_INPUT_IO_CD_SDCARD) || network_Activate_Flag == 1)
                    {

                        memset(&logs_struct, 0, sizeof(logs_struct));

                        sprintf(logs_struct.type, "%s", "");
                        sprintf(logs_struct.name, "%s", return_Json_SMS_Data("ROUTINE"));
                        sprintf(logs_struct.phone, "%s", "");
                        sprintf(logs_struct.relay, "%s", "R2");

                        if (job->data[1] == '0')
                        {
                            sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("OFF"));
                            // ////printf("\n\n ROUTINES logs_struct.relay_state %s\n\n", logs_struct.relay_state);
                        }
                        else if (job->data[1] == '1')
                        {
                            sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("ON"));
                            // ////printf("\n\n ROUTINES logs_struct.relay_state %s\n\n", logs_struct.relay_state);
                        }
                        else if (job->data[1] == '2')
                        {
                            sprintf(logs_struct.relay_state, "%s", return_Json_SMS_Data("PULSE"));
                        }

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
                    }
                }
            }
        }
        else
        {
            /* gpio_set_level(GPIO_OUTPUT_IO_0, 0);
            gpio_set_level(GPIO_OUTPUT_IO_1, 0); */
            // ////printf("\n OUT OF RANGE\n");
        }
    }
    else
    {
        /* gpio_set_level(GPIO_OUTPUT_IO_0, 0);
        gpio_set_level(GPIO_OUTPUT_IO_1, 0); */
        // ////printf("\n\n IS EXEPTION DAY\n\n");
    }

    // ////printf("\n\njob->data - %s\n\n", cron_data);

    // ////printf("\n CALLBACK RUNNED AT TIME: %s SHOULD RUN AT: %s WITH DATA: %s \n", buffer, buffer2, (char *)job->data);

    free(cron_data);

    //ESP_LOGI("TAG", "xPortGetFreeHeapSize  parseInputData1111: %d", xPortGetFreeHeapSize());
    //ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  : %d", esp_get_minimum_free_heap_size());
    //ESP_LOGI("TAG", "heap_caps_get_largest_free_block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    //ESP_LOGI("TAG", "free heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));

    return;
}

char *get_ExeptionDay()
{

    nvs_iterator_t it;
    it = nvs_entry_find("keys", NVS_EXEPTION_DAYS_NAMESPACE, NVS_TYPE_U8);
    char value[20] = {};
    size_t required_size = 0;

    memset(file_contents_routines, 0, 200);

    if (it == NULL)
    {
        return "NO EXCEPTION DAYS";
    }

    while (it != NULL)
    {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        it = nvs_entry_next(it);
        // ////printf("key '%s', type '%d' \n", info.key, info.type);
        // ////printf("\naux get str %s\n", value);
        strcat(file_contents_routines, info.key);
        strcat(file_contents_routines, ";\0");
    }

    file_contents_routines[strlen(file_contents_routines) - 1] = '\0';
    // ////printf("\n\nget_ExeptionDay - %s \n\n", file_contents_routines);
    free(it);
    return file_contents_routines;
}

uint8_t send_udp_routines_funtion()
{
    allUsers_parameters_Message message;

    char value[200] = {};
    char buffer_BLE[600] = {};
    uint8_t label_sendUDP_routines = 0;
    uint8_t routines_Count = 0;
    label_Cron_Init = 0;
    size_t required_size = 0;
    // send_Routines_queue = xQueueCreate(2, sizeof(message));
    nvs_entry_info_t info;

    // ////printf("\n task_Send_Routines 001 - %d ; %d ; %d \n", message.gattsIF, message.connID, message.handle_table);

    routines_Count = 0;
    label_Cron_Init = 0;
    required_size = 0;
    memset(value, 0, 200);
    memset(buffer_BLE, 0, 200);
    uint8_t routines_Number_exp = 0;

    nvs_iterator_t it;
    it = nvs_entry_find("keys", NVS_ROUTINES_NAMESPACE, NVS_TYPE_STR);

    sprintf(buffer_BLE, "%s", "RT R R");

    

    if (it == NULL)
    {
        // ////printf("\n GET ROTINES !! \n");
        sprintf(buffer_BLE, "%s", "RT G R NO ROUTINES");
    }

    routines_Number_exp = 9;

    // ////printf("\n GET ROTINES !! \n");
    while (it != NULL)
    {

        nvs_entry_info(it, &info);
        it = nvs_entry_next(it);
        // ////printf("key '%s', type '%d' \n", info.key, info.type);

        if (nvs_get_str(nvs_Routines_handle, info.key, NULL, &required_size) == ESP_OK)
        {
            // ////printf("\nrequire size %d\n", required_size);
            // ////printf("\nGET USERS NAMESPACE\n");
            if (nvs_get_str(nvs_Routines_handle, info.key, value, &required_size) == ESP_OK)
            {
                // ////printf("\naux get str %s\n", value);
            }
        }

        if (routines_Count == 0)
        {
            sprintf(buffer_BLE, "%s", "RT G R:");
            strcat(buffer_BLE, value);
            routines_Count++;
            required_size = 0;
            memset(value, 0, 200);
        }
        else if (routines_Count > 0 && routines_Count < routines_Number_exp)
        {
            strcat(buffer_BLE, ":\0");
            strcat(buffer_BLE, value);
            required_size = 0;
            routines_Count++;
            memset(value, 0, 200);
        }
        else if (routines_Count == routines_Number_exp)
        {
            strcat(buffer_BLE, ":\0");
            strcat(buffer_BLE, value);

            if (!send_UDP_Send(buffer_BLE,""))
            {
                free(it);
                return 0;
            }

            routines_Count = 0;
            required_size = 0;
            memset(value, 0, 200);
        }
    }

    uint8_t dotCounter = 0;

    for (int i = 0; i < strlen(buffer_BLE); i++)
    {
        if (buffer_BLE[i] == ':')
        {
            dotCounter++;
        }
    }

    if (dotCounter < (routines_Number_exp + 1))
    {

        if (!send_UDP_Send(buffer_BLE,""))
        {
            free(it);
            return 0;
        }

        // ////printf("\nROUTINES SEND2 - %s\n", buffer_BLE);
    }

    free(it);

    return 1;

    if (label_sendUDP_routines == 1)
    {
        uint8_t label_UDP_fail_and_changed = get_INT8_Data_From_Storage(NVS_NETWORK_LOCAL_CHANGED, nvs_System_handle);
        label_UDP_fail_and_changed |= 1;
        save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED, label_UDP_fail_and_changed, nvs_System_handle);
    }
 
    return 1;
}

void task_Send_Routines(void *pvParameter)
{
    // ////printf("\n task_Send_Routines 0\n");

    allUsers_parameters_Message message;

    char value[200] = {};
    char buffer_BLE[600] = {};
    uint8_t label_sendUDP_routines = 0;
    uint8_t routines_Count = 0;
    label_Cron_Init = 0;
    size_t required_size = 0;
    send_Routines_queue = xQueueCreate(1, sizeof(message));
    nvs_entry_info_t info;

    for (;;)
    {

        if (xQueueReceive(send_Routines_queue, &message, portMAX_DELAY))
        {

            // ////printf("\n task_Send_Routines 001 - %d ; %d ; %d \n", message.gattsIF, message.connID, message.handle_table);

            routines_Count = 0;
            label_Cron_Init = 0;
            required_size = 0;
            memset(value, 0, 200);
            memset(buffer_BLE, 0, 200);
            uint8_t routines_Number_exp = 0;

            nvs_iterator_t it;
            it = nvs_entry_find("keys", NVS_ROUTINES_NAMESPACE, NVS_TYPE_STR);

           /*  if (!send_UDP_Send("RT R R"))
            {
                label_sendUDP_routines = 1;
            } */

            if (it == NULL)
            {
                // ////printf("\n GET ROTINES !! \n");
                sprintf(buffer_BLE, "%s", "RT G R NO ROUTINES");
               
            }

            if (message.BLE_UDP_Indication == UDP_INDICATION)
            {
                routines_Number_exp = 9;
            }
            else
            {
                routines_Number_exp = 3;
            }

            // ////printf("\n GET ROTINES !! \n");
            while (it != NULL)
            {

                nvs_entry_info(it, &info);
                it = nvs_entry_next(it);
                // ////printf("key '%s', type '%d' \n", info.key, info.type);

                if (nvs_get_str(nvs_Routines_handle, info.key, NULL, &required_size) == ESP_OK)
                {
                    // ////printf("\nrequire size %d\n", required_size);
                    // ////printf("\nGET USERS NAMESPACE\n");
                    if (nvs_get_str(nvs_Routines_handle, info.key, value, &required_size) == ESP_OK)
                    {
                        // ////printf("\naux get str %s\n", value);
                    }
                }

                if (routines_Count == 0)
                {
                    sprintf(buffer_BLE, "%s", "RT G R:");
                    strcat(buffer_BLE, value);
                    routines_Count++;
                    required_size = 0;
                    memset(value, 0, 200);
                }
                else if (routines_Count > 0 && routines_Count < routines_Number_exp)
                {
                    strcat(buffer_BLE, ":\0");
                    strcat(buffer_BLE, value);
                    required_size = 0;
                    routines_Count++;
                    memset(value, 0, 200);
                }
                else if (routines_Count == routines_Number_exp)
                {
                    strcat(buffer_BLE, ":\0");
                    strcat(buffer_BLE, value);
                    // ////printf("\nROUTINES SEND1 - %s\n", buffer_BLE);
                    //printf("\n task_Send_Routines 1 - %d ; %d ; %d \n", message.gattsIF, message.connID, message.handle_table);
                    if (message.BLE_UDP_Indication == UDP_INDICATION)
                    {
                        /* if (!send_UDP_Send(buffer_BLE,""))
                        {
                            label_sendUDP_routines = 1;
                            break;
                        } */
                    }
                    else
                    {
                        //printf("\n task_Send_Routines 2 - %d ; %d ; %d \n", message.gattsIF, message.connID, message.handle_table);
                        if (esp_ble_gatts_send_indicate(message.gattsIF, message.connID, message.handle_table, strlen(buffer_BLE), (uint8_t *)buffer_BLE, false) != ESP_OK)
                        {
                            //printf("\n task_Send_Routines 3 - %d ; %d ; %d \n", message.gattsIF, message.connID, message.handle_table);
                            break;
                        }
                        //printf("\n task_Send_Routines 4 - %d ; %d ; %d \n", message.gattsIF, message.connID, message.handle_table);
                        xSemaphoreTake(rdySem_Send_Routines, pdMS_TO_TICKS(3000));
                    }

                    routines_Count = 0;
                    required_size = 0;
                    memset(value, 0, 200);
                }
            }

            uint8_t dotCounter = 0;

            for (int i = 0; i < strlen(buffer_BLE); i++)
            {
                if (buffer_BLE[i] == ':')
                {
                    dotCounter++;
                }
            }

            if (dotCounter < (routines_Number_exp + 1))
            {

                if (message.BLE_UDP_Indication == UDP_INDICATION)
                {
                    /* if (!send_UDP_Send(buffer_BLE,""))
                    {
                        label_sendUDP_routines = 1;
                    } */
                }
                else
                {
                    esp_ble_gatts_send_indicate(message.gattsIF, message.connID, message.handle_table, strlen(buffer_BLE), (uint8_t *)buffer_BLE, false);
                    xSemaphoreTake(rdySem_Send_Routines, pdMS_TO_TICKS(3000));
                }
                // ////printf("\nROUTINES SEND2 - %s\n", buffer_BLE);
            }

            free(it);

            if (label_sendUDP_routines == 1)
            {
                uint8_t label_UDP_fail_and_changed = get_INT8_Data_From_Storage(NVS_NETWORK_LOCAL_CHANGED, nvs_System_handle);
                label_UDP_fail_and_changed |= 1;
                save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED, label_UDP_fail_and_changed, nvs_System_handle);
            }
        }
    }

    vTaskDelete(NULL);
}

uint8_t get_Routine_Range(char *inicalTime, char *finalTime)
{

    size_t required_size;
    char aux_Get_Routine_Range_Data[7];
    memset(aux_Get_Routine_Range_Data, 0, sizeof(aux_Get_Routine_Range_Data));
    // ////printf("\nget_Routine_Range\n");
    if (nvs_get_str(nvs_System_handle, NVS_ROUTINES_RANGE_T0, NULL, &required_size) == ESP_OK)
    {
        if (nvs_get_str(nvs_System_handle, NVS_ROUTINES_RANGE_T0, aux_Get_Routine_Range_Data, &required_size) == ESP_OK)
        {
            // ////printf("\naux get str %s\n", aux_Get_Routine_Range_Data);
            sprintf(inicalTime, "%s", aux_Get_Routine_Range_Data);
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

    memset(aux_Get_Routine_Range_Data, 0, sizeof(aux_Get_Routine_Range_Data));
    required_size = 0;

    if (nvs_get_str(nvs_System_handle, NVS_ROUTINES_RANGE_T1, NULL, &required_size) == ESP_OK)
    {
        if (nvs_get_str(nvs_System_handle, NVS_ROUTINES_RANGE_T1, aux_Get_Routine_Range_Data, &required_size) == ESP_OK)
        {
            // ////printf("\naux get str %s\n", aux_Get_Routine_Range_Data);
            sprintf(finalTime, "%s", aux_Get_Routine_Range_Data);
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

    return 1;
}

void task_Routine_BiState_Send_RelayState1(void *pvParameter)
{

    while (1)
    {
        xSemaphoreTake(rdySem_Routine_BiState_Send_RelayState1, portMAX_DELAY);
        // ////printf("\n\n biestavel routine %d\n\n",rele1_Bistate_Time);
        if (label_MonoStableRelay1 != 1)
        {
            label_MonoStableRelay1 = 1;
            if (label_MonoStableRelay1 == 1)
            {
                gpio_set_level(GPIO_OUTPUT_IO_0, 1);
            }

            vTaskDelay(pdMS_TO_TICKS((rele1_Bistate_Time * 1000)));

            if (label_MonoStableRelay1 == 1)
            {
                gpio_set_level(GPIO_OUTPUT_IO_0, 0);
            }
            label_MonoStableRelay1 = 0;
        }
    }
    task_Routine_BiState_RelayState1 = NULL;
    vTaskDelete(task_Routine_BiState_RelayState1);
}

void task_Routine_BiState_Send_RelayState2(void *pvParameter)
{
    while (1)
    {
        xSemaphoreTake(rdySem_Routine_BiState_Send_RelayState2, portMAX_DELAY);
        if (label_MonoStableRelay2 != 1)
        {
            label_MonoStableRelay2 = 1;
            if (label_MonoStableRelay2 == 1)
            {
                gpio_set_level(GPIO_OUTPUT_IO_1, 1);
            }

            vTaskDelay(pdMS_TO_TICKS((rele2_Bistate_Time * 1000)));

            if (label_MonoStableRelay2 == 1)
            {
                gpio_set_level(GPIO_OUTPUT_IO_1, 0);
            }
        }
        label_MonoStableRelay2 = 0;
    }
    task_Routine_BiState_RelayState2 = NULL;
    vTaskDelete(task_Routine_BiState_RelayState2);
}

void initRoutines()
{
    // heap_trace_start(HEAP_TRACE_LEAKS);
    // label_Routine1_ON = 0;
    // label_Routine2_ON = 0;
    nvs_iterator_t it;
    it = nvs_entry_find("keys", NVS_ROUTINES_NAMESPACE, NVS_TYPE_STR);
    char value[200];
    label_Cron_Init = 1;
    size_t required_size = 0;
    routines_ID = 0;
    save_INT8_Data_In_Storage(NVS_ROUTINES_ID, routines_ID, nvs_System_handle);
    // ////printf("\n\n akakak 1000\n\n");
    while (it != NULL)
    {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        it = nvs_entry_next(it);
        // ////printf("ROUTINES - key '%s', type '%d' \n", info.key, info.type);

        if (nvs_get_str(nvs_Routines_handle, info.key, NULL, &required_size) == ESP_OK)
        {
            // ////printf("\nROUTINES require size %d\n", required_size);
            // ////printf("\n ROUTINES NAMESPACE\n");
            if (nvs_get_str(nvs_Routines_handle, info.key, value, &required_size) == ESP_OK)
            {
                // ////printf("\naux get str %s\n", value);
                setRoutine(value);
            }
        }
        required_size = 0;
        memset(value, 0, 200);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    label_Cron_Init = 0;

    it = nvs_entry_find("keys", NVS_ROUTINES_NAMESPACE, NVS_TYPE_STR);

    if (it != NULL)
    {
        cron_start();
    }
    // ////printf("\n\n akakak 1001\n\n");
    free(it);
    // ////printf("\n\n akakak 1002\n\n");
    // heap_trace_stop();
    // heap_trace_dump();
}

char *activate_Routines(uint8_t BLE_indication, uint8_t gattsIF, uint16_t connID, uint16_t handle_table)
{
    size_t required_size = 0;
    char timezone[75];
    time_t now;
    time(&now);

    // ////printf("\n activate now time %ld\n", now);

    struct timeval epoch = {now, 0};
    struct timeval *tv = &epoch;

    nvs_iterator_t it;
    it = nvs_entry_find("keys", NVS_ROUTINES_NAMESPACE, NVS_TYPE_STR);

    if (it != NULL)
    {

        if (!settimeofday(tv, NULL))
        {
            if (nvs_get_str(nvs_System_handle, NVS_TIMEZONE, NULL, &required_size) == ESP_OK)
            {
                // ////printf("\nrequire size %d\n", required_size);
                if (nvs_get_str(nvs_System_handle, NVS_TIMEZONE, timezone, &required_size) == ESP_OK)
                {
                    if (!setenv("TZ", timezone, 1))
                    {
                        // ////printf("\n activate now time \n");
                        tzset();
                    }
                    else
                    {
                        free(it);
                        return return_Json_SMS_Data("ERROR_ACTIVATE_ROUTINES");
                    }
                }
            }

            cron_start();

            if (BLE_indication == BLE_INDICATION)
            {
                esp_ble_gatts_send_indicate(gattsIF, connID, handle_table, strlen("RT S A OK"), (uint8_t *)("RT S A OK"), false);
                vTaskDelay(pdMS_TO_TICKS(500));
            }

            if (label_network_portalRegister == 1)
            {

                label_network_portalRegister = get_INT8_Data_From_Storage(NVS_NETWORK_PORTAL_REGISTER, nvs_System_handle);

                send_UDP_Send("RT S A OK","");
                //printf("\n\n\ send routines UDP2\n\n");

                vTaskDelay(pdMS_TO_TICKS(500));

                MyUser user_validateData;
                memset(&user_validateData, 0, sizeof(user_validateData));
                char owner_Data[200] = {};
                uint8_t label_UDP_fail_and_changed = 0;
                if (nvs_get_str(nvs_System_handle, NVS_KEY_OWNER_INFORMATION, NULL, &required_size) == ESP_OK)
                {
                    // ////printf("\nrequire size %d\n", required_size);
                    //  ////printf("\nGET OWNER NAMESPACE\n");
                    if (nvs_get_str(nvs_System_handle, NVS_KEY_OWNER_INFORMATION, owner_Data, &required_size) == ESP_OK)
                    {

                        char *data_register;

                        char input_data[100];
                        // ////printf("\n\n\ send routines UDP3\n\n");
                        parse_ValidateData_User(owner_Data, &user_validateData);

                        sprintf(input_data, "%s %s RT.G.T", user_validateData.phone, user_validateData.key);
                        data_register = parseInputData(&input_data, UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);
                        send_UDP_Send(data_register,"");
                        
                        memset(input_data, 0, 100);
                        // ////printf("\n\nlabel_UDP_fail_and_changed trete 12122 %d\n\n", label_UDP_fail_and_changed);
                        sprintf(input_data, "%s %s RT.G.D", user_validateData.phone, user_validateData.key);
                        data_register = parseInputData(&input_data, UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);
                        send_UDP_Send(data_register,"");
                      
                        memset(input_data, 0, 100);

                        sprintf(input_data, "%s %s RT.G.H", user_validateData.phone, user_validateData.key);
                        data_register = parseInputData(&input_data, UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);
                        send_UDP_Send(data_register,"");
                        // send_UDP_queue(data_register);
                        memset(input_data, 0, 100);

                        sprintf(input_data, "%s %s RT.G.F", user_validateData.phone, user_validateData.key);
                        data_register = parseInputData(&input_data, UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);
                        send_UDP_Send(data_register,"");
                        // send_UDP_queue(data_register);
                        memset(input_data, 0, 100);

                        // send_UDP_queue(data_register);
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

                        free(data_register);
                    }
                }
            }
        }
        else
        {
            free(it);
            return return_Json_SMS_Data("ERROR_ACTIVATE_ROUTINES");
        }
    }

    send_UDP_Send("+","");

    free(it);
    label_BLE_UDP_send = 0;
    return "NTRSP";
}

void calcularPascoa(int ano, int *dia, int *mes)
{
    int a = ano % 19;
    int b = ano / 100;
    int c = ano % 100;
    int d = b / 4;
    int e = b % 4;
    int f = (b + 8) / 25;
    int g = (b - f + 1) / 3;
    int h = (19 * a + b - d - g + 15) % 30;
    int i = c / 4;
    int k = c % 4;
    int l = (32 + 2 * e + 2 * i - h - k) % 7;
    int m = (a + 11 * h + 22 * l) / 451;
    int n = (h + l - 7 * m + 114) / 31;
    int p = (h + l - 7 * m + 114) % 31;

    *dia = p + 1;
    *mes = n;
}

// Função para subtrair dias de uma data
int subtrairDias(int date, int days)
{
    int diasPorMes[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int year = date / 10000;
    int month = (date / 100) - (year * 100);
    int day = date - ((year * 10000) + (month * 100));

    // Subtrair os dias da data
    day -= days;

    while (day <= 0)
    {
        month--;

        if (month <= 0)
        {
            year--;
            month = 12;
        }

        if (month == 2 && isLeapYear(year))
        {
            day += 29;
        }
        else
        {
            day += diasPorMes[month - 1];
        }
    }

    // Imprimir a nova data
    // ////printf("\n\nData atual: %02d/%02d/%d\n", day, month, year);

    int finalDate = year * 10000 + month * 100 + day;
    // ////printf("new year %d\n", finalDate);

    return finalDate;
}

uint8_t set_easter_day()
{
    int day = 0;
    int month = 0;
    char date[20] = {};

    get_RTC_System_Time();

    calcularPascoa(nowTime.year + 2000, &day, &month);

    // date = day * 100 + month;
    sprintf(date, "%02d%02d", day, month - 1);

    // ////printf("\n\n pascoa - %s", date);

    if (save_INT8_Data_In_Storage(date, 1, nvs_Mobile_Holydays_handle) != ESP_OK)
    {
        return 0;
    }

    if (save_INT8_Data_In_Storage(NVS_KEY_LABEL_EASTER, 1, nvs_System_handle) != ESP_OK)
    {
        return 0;
    }

    return 1;
}

uint8_t set_easter_monday()
{
    get_RTC_System_Time();
    int day = 0;
    int month = 0;
    int date = 0;
    int carnaval_Date = 0;

    // ////printf("\n\n SEGUNDA DE PASCOA nowtime.year %d\n\n", nowTime.year);
    calcularPascoa(nowTime.year + 2000, &day, &month);

    date = nowTime.year * 10000 + month * 100 + day;
    int finalDate = parseDatetoInt(date, 1);

    int easter_monday_year = finalDate / 10000;
    int easter_monday_month = (finalDate / 100) - (easter_monday_year * 100);
    int easter_monday_day = finalDate - ((easter_monday_year * 10000) + (easter_monday_month * 100));

    char easterMondayDate[20] = {};

    sprintf(easterMondayDate, "%02d%02d", easter_monday_day, easter_monday_month - 1);

    // ////printf("\n\n SEGUNDA DE PASCOA date - %s\n\n", easterMondayDate);

    if (save_INT8_Data_In_Storage(easterMondayDate, 1, nvs_Mobile_Holydays_handle) != ESP_OK)
    {
        return 0;
    }

    if (save_INT8_Data_In_Storage(NVS_KEY_LABEL_EASTER_MONDAY, 1, nvs_System_handle) != ESP_OK)
    {
        return 0;
    }

    return 1;
}

uint8_t set_easter_friday()
{
    get_RTC_System_Time();
    int day = 0;
    int month = 0;
    int date = 0;
    int easter_friday_Date = 0;

    // ////printf("\n\n carnaval nowtime.year %d\n\n", nowTime.year);
    calcularPascoa(nowTime.year + 2000, &day, &month);

    date = nowTime.year * 10000 + month * 100 + day;
    int finalDate = subtrairDias(date, 2);

    int easter_friday_year = finalDate / 10000;
    int easter_friday_month = (finalDate / 100) - (easter_friday_year * 100);
    int easter_friday_day = finalDate - ((easter_friday_year * 10000) + (easter_friday_month * 100));

    char easter_fridayDate[20] = {};

    sprintf(easter_fridayDate, "%02d%02d", easter_friday_day, easter_friday_month - 1);

    // ////printf("\n\n easter_friday date - %s\n\n", easter_fridayDate);

    if (save_INT8_Data_In_Storage(easter_fridayDate, 1, nvs_Mobile_Holydays_handle) != ESP_OK)
    {
        return 0;
    }

    if (save_INT8_Data_In_Storage(NVS_KEY_LABEL_EASTER_FRIDAY, 1, nvs_System_handle) != ESP_OK)
    {
        return 0;
    }

    return 1;
}

char *set_rangeHolidaysDays(char *payload)
{
    char inicialTime[7] = {};
    char finalTime[7] = {};
    uint8_t dot_label = 0;
    uint8_t strIndex = 0;

    for (int i = 0; i < strlen(payload); i++)
    {
        if (payload[i] == ';')
        {
            dot_label = 1;
            strIndex = 0;
        }
        else
        {
            if (dot_label == 0)
            {
                inicialTime[strIndex] = payload[i];
                strIndex++;
            }
            else if (dot_label == 1)
            {
                finalTime[strIndex] = payload[i];
                strIndex++;
            }
            else
            {
                return "ERROR";
            }
        }
    }

    if (atoi(inicialTime) > 0 && atoi(inicialTime) <= 991131 && (strlen(inicialTime) == 6))
    {
        save_STR_Data_In_Storage(NVS_ROUTINES_HOLIDAYS_RANGE_T1, inicialTime, nvs_System_handle);
    }
    else
    {
        return "ERROR INICIAL TIME";
    }

    if (atoi(finalTime) > 0 && atoi(finalTime) <= 991131 && (strlen(finalTime) == 6))
    {
        save_STR_Data_In_Storage(NVS_ROUTINES_HOLIDAYS_RANGE_T2, finalTime, nvs_System_handle);
    }
    else
    {
        return "ERROR FINAL TIME";
    }

    return "OK";
}

uint8_t get_rangeHolidaysDays(char *inicalTime, char *finalTime)
{
    size_t required_size;
    char aux_Get_Routine_Range_Data[7];
    memset(aux_Get_Routine_Range_Data, 0, sizeof(aux_Get_Routine_Range_Data));
    // ////printf("\nget_Routine_Range\n");
    if (nvs_get_str(nvs_System_handle, NVS_ROUTINES_HOLIDAYS_RANGE_T1, NULL, &required_size) == ESP_OK)
    {
        if (nvs_get_str(nvs_System_handle, NVS_ROUTINES_HOLIDAYS_RANGE_T1, aux_Get_Routine_Range_Data, &required_size) == ESP_OK)
        {
            // ////printf("\naux get str %s\n", aux_Get_Routine_Range_Data);
            sprintf(inicalTime, "%s", aux_Get_Routine_Range_Data);
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

    memset(aux_Get_Routine_Range_Data, 0, sizeof(aux_Get_Routine_Range_Data));
    required_size = 0;

    if (nvs_get_str(nvs_System_handle, NVS_ROUTINES_HOLIDAYS_RANGE_T2, NULL, &required_size) == ESP_OK)
    {
        if (nvs_get_str(nvs_System_handle, NVS_ROUTINES_HOLIDAYS_RANGE_T2, aux_Get_Routine_Range_Data, &required_size) == ESP_OK)
        {
            // ////printf("\naux get str %s\n", aux_Get_Routine_Range_Data);
            sprintf(finalTime, "%s", aux_Get_Routine_Range_Data);
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

    return 1;
}

char *reset_rangeHolidaysDays()
{
    esp_err_t err = nvs_erase_key(nvs_System_handle, NVS_ROUTINES_HOLIDAYS_RANGE_T1);

    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        return "ERROR";
    }

    err = nvs_erase_key(nvs_System_handle, NVS_ROUTINES_HOLIDAYS_RANGE_T2);

    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        return "ERROR";
    }

    return "OK";
}

void refresh_holidaysDays()
{
    nvs_erase_all(nvs_Mobile_Holydays_handle);
    if (get_INT8_Data_From_Storage(NVS_KEY_LABEL_CARNIVAL, nvs_System_handle) == 1)
    {
        set_carnival();
    }

    if (get_INT8_Data_From_Storage(NVS_KEY_LABEL_EASTER_FRIDAY, nvs_System_handle) == 1)
    {
        set_easter_friday();
    }

    if (get_INT8_Data_From_Storage(NVS_KEY_LABEL_EASTER, nvs_System_handle) == 1)
    {
        set_easter_day();
    }

    if (get_INT8_Data_From_Storage(NVS_KEY_LABEL_CORPUS_CHRISTI, nvs_System_handle) == 1)
    {
        set_corpusChristi();
    }

    if (get_INT8_Data_From_Storage(NVS_KEY_LABEL_EASTER_MONDAY, nvs_System_handle) == 1)
    {
        set_easter_monday();
    }
}

uint8_t set_corpusChristi()
{

    get_RTC_System_Time();
    int day = 0;
    int month = 0;
    int date = 0;
    int carnaval_Date = 0;

    // ////printf("\n\n corpo cristo nowtime.year %d\n\n", nowTime.year);
    calcularPascoa(nowTime.year + 2000, &day, &month);

    date = nowTime.year * 10000 + month * 100 + day;
    int finalDate = parseDatetoInt(date, 60);

    int corpusChristi_year = finalDate / 10000;
    int corpusChristi_month = (finalDate / 100) - (corpusChristi_year * 100);
    int corpusChristi_day = finalDate - ((corpusChristi_year * 10000) + (corpusChristi_month * 100));

    char corpusChristiDate[20] = {};

    sprintf(corpusChristiDate, "%02d%02d", corpusChristi_day, corpusChristi_month - 1);

    // ////printf("\n\n corpo de cristo date - %s\n\n", corpusChristiDate);

    if (save_INT8_Data_In_Storage(corpusChristiDate, 1, nvs_Mobile_Holydays_handle) != ESP_OK)
    {
        return 0;
    }

    if (save_INT8_Data_In_Storage(NVS_KEY_LABEL_CORPUS_CHRISTI, 1, nvs_System_handle) != ESP_OK)
    {
        return 0;
    }

    return 1;
}

uint8_t set_carnival()
{
    get_RTC_System_Time();
    int day = 0;
    int month = 0;
    int date = 0;
    int carnaval_Date = 0;

    // ////printf("\n\n carnaval nowtime.year %d\n\n", nowTime.year);
    calcularPascoa(nowTime.year + 2000, &day, &month);

    date = nowTime.year * 10000 + month * 100 + day;
    int finalDate = subtrairDias(date, 47);

    int carnaval_year = finalDate / 10000;
    int carnaval_month = (finalDate / 100) - (carnaval_year * 100);
    int carnaval_day = finalDate - ((carnaval_year * 10000) + (carnaval_month * 100));

    char carnavalDate[20] = {};

    sprintf(carnavalDate, "%02d%02d", carnaval_day, carnaval_month - 1);

    // ////printf("\n\n carnaval date - %s\n\n", carnavalDate);

    if (save_INT8_Data_In_Storage(carnavalDate, 1, nvs_Mobile_Holydays_handle) != ESP_OK)
    {
        return 0;
    }

    if (save_INT8_Data_In_Storage(NVS_KEY_LABEL_CARNIVAL, 1, nvs_System_handle) != ESP_OK)
    {
        return 0;
    }

    return 1;
}

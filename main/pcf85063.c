/*
 * Copyright (c) 2022 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "pcf85063.h"

// LOG_MODULE_REGISTER(pcf85063a);

#define DT_DRV_COMPAT nxp_pcf85063a

uint8_t I2C_ReadRegister(uint8_t reg_addr, uint8_t *data, size_t len)
{
    esp_err_t err = ESP_OK;

    uint8_t rxBuf[4]; //[len];

    i2c_master_write_read_device(I2C_MASTER_NUM, 0x51, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

     //////printf("\n value - %d ", data[0]);

    return 1;
}

uint8_t I2C_Master_write(uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, I2C_ADDR | WRITE_BIT, ACK_CHECK_EN);

    i2c_master_write_byte(cmd, reg_addr | WRITE_BIT, ACK_CHECK_DIS);

    i2c_master_write(cmd, data /* write_buf */, len, ACK_CHECK_EN);

    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);

    return ret;
}

/*
 * Function from https://stackoverflow.com/questions/19377396/c-get-day-of-year-from-date
 */
static int yisleap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/*
 * Function from https://stackoverflow.com/questions/19377396/c-get-day-of-year-from-date
 */
static int get_yday(int mon, int day, int year)
{
    static const int days[2][12] = {
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
        {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};
    int leap = yisleap(year);

    /* Minus 1 since gmtime starts from 0 */
    return days[leap][mon] + day - 1;
}

static uint8_t decToBcd(int val);
static int bcdToDec(uint8_t val);

void PCF85_Init()
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);

    uint8_t PCFINIT_ = 0x49;
    I2C_Master_write(REG_CTRL1_ADDR, &PCFINIT_, 1);

    enableAlarm();
    // HAL_I2C_Mem_Write(_pcf850_i2c, I2C_ADDR, REG_CTRL1_ADDR ,1, &PCFINIT_, 1, 1000 );
}

void PCF85_Reset()
{
    uint8_t PCFIRSTS_ = 0x59;
    I2C_Master_write(REG_CTRL1_ADDR, &PCFIRSTS_, 1);
    // HAL_I2C_Mem_Write(_pcf850_i2c, I2C_ADDR, REG_CTRL1_ADDR ,1, &PCFIRSTS_, 1, 1000 )
}

void PCF85_SetTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    uint8_t buf[3] = {decToBcd(second),
                      decToBcd(minute),
                      decToBcd(hour)};

    I2C_Master_write(REG_TIME_ADDR, buf, 3);
    // HAL_I2C_Mem_Write(_pcf850_i2c, I2C_ADDR, REG_TIME_ADDR, 1, buf, 3, 1000);
}

void PCF85_SetDate(uint8_t weekday, uint8_t day, uint8_t month, uint16_t yr)
{
    uint8_t buf[4] = {decToBcd(day),
                      decToBcd(weekday),
                      decToBcd(month),
                      decToBcd(yr - YEAR_OFFSET)};

    I2C_Master_write(REG_DATE_ADDR, buf, 4);
    // HAL_I2C_Mem_Write(_pcf850_i2c, I2C_ADDR, REG_DATE_ADDR, 1, buf, 4, 1000);
}

uint8_t PCF85_Readnow(uint8_t *buf)
{
    uint8_t bufss[7] = {0};
    // HAL_I2C_Mem_Read(_pcf850_i2c, I2C_ADDR, REG_TIME_ADDR, 1, bufss, 7, 1000)
    if (I2C_ReadRegister(REG_TIME_ADDR, &bufss, 7))
    {

        buf[0] = bcdToDec(bufss[0] & 0x7F);
        buf[1] = bcdToDec(bufss[1] & 0x7F);
        buf[2] = bcdToDec(bufss[2] & 0x3F);
        buf[3] = bcdToDec(bufss[3] & 0x3F);
        buf[4] = bcdToDec(bufss[4] & 0x07);
        buf[5] = bcdToDec(bufss[5] & 0x1F);
        buf[6] = bcdToDec(bufss[6]);

        ////printf("\n\n read rtc %s\n\n",buf);

        return 1;
    }

    return 0;
}

void enableAlarm() // datasheet 8.5.6.
{
    // check Table 2. Control_2
    uint8_t control_2 = 0;
    uint8_t aux_ctr = 0;
    control_2 = 0x00 | PCF85063A_CTRL2_MI | PCF85063A_CTRL2_COF_LOW; // PCF85063A_CTRL2_AIE; // enable interrupt
    // control_2 &= ~PCF85063A_CTRL2_AF;       // clear alarm flag

    I2C_Master_write(PCF85063A_CTRL2, &control_2, 1);
    I2C_ReadRegister(PCF85063A_CTRL2, &aux_ctr, 1);
}

void disableAlarm() // datasheet 8.5.6.
{
    // check Table 2. Control_2
    uint8_t control_2 = 0;
    uint8_t aux_ctr = 0;
    control_2 = 0x00;// | PCF85063A_CTRL2_MI; // PCF85063A_CTRL2_AIE; // enable interrupt
    // control_2 &= ~PCF85063A_CTRL2_AF;       // clear alarm flag

    I2C_Master_write(PCF85063A_CTRL2, &control_2, 1);
    I2C_ReadRegister(PCF85063A_CTRL2, &aux_ctr, 1);
}

void setAlarm(uint8_t alarm_second, uint8_t alarm_minute, uint8_t alarm_hour, uint8_t alarm_day, uint8_t alarm_weekday)
{
    uint8_t alarm_Time[5];
    if (alarm_second < 99)
    { // second
        // alarm_second = constrain(alarm_second, 0, 59);
        alarm_second = decToBcd(alarm_second);
        alarm_second &= ~RTC_ALARM;
    }
    else
    {
        alarm_second = 0x0;
        alarm_second |= RTC_ALARM;
    }

    if (alarm_minute < 99)
    { // minute
        // alarm_minute = constrain(alarm_minute, 0, 59);
        alarm_minute = decToBcd(alarm_minute);
        alarm_minute &= ~RTC_ALARM;
    }
    else
    {
        alarm_minute = 0x0;
        alarm_minute |= RTC_ALARM;
    }

    if (alarm_hour < 99)
    { // hour
        // alarm_hour = constrain(alarm_hour, 0, 23);
        alarm_hour = decToBcd(alarm_hour);
        alarm_hour &= ~RTC_ALARM;
    }
    else
    {
        alarm_hour = 0x0;
        alarm_hour |= RTC_ALARM;
    }

    if (alarm_day < 99)
    { // day
        // alarm_day = constrain(alarm_day, 1, 31);
        alarm_day = decToBcd(alarm_day);
        alarm_day &= ~RTC_ALARM;
    }
    else
    {
        alarm_day = 0x0;
        alarm_day |= RTC_ALARM;
    }

    if (alarm_weekday < 99)
    { // weekday
        // alarm_weekday = constrain(alarm_weekday, 0, 6);
        alarm_weekday = decToBcd(alarm_weekday);
        alarm_weekday &= ~RTC_ALARM;
    }
    else
    {
        alarm_weekday = 0x0;
        alarm_weekday |= RTC_ALARM;
    }

    //enableAlarm();
    I2C_Master_write(PCF85063A_SECOND_ALARM, &alarm_second, 1);
    I2C_Master_write(PCF85063A_MINUTE_ALARM, &alarm_minute, 1);
    I2C_Master_write(PCF85063A_HOUR_ALARM, &alarm_hour, 1);
    I2C_Master_write(PCF85063A_DAY_ALARM, &alarm_day, 1);
    I2C_Master_write(PCF85063A_WEEKDAY_ALARM, &alarm_weekday, 1);

    /*   Wire.beginTransmission(I2C_ADDR);
      Wire.write(RTC_SECOND_ALARM);
      Wire.write(alarm_second);
      Wire.write(alarm_minute);
      Wire.write(alarm_hour);
      Wire.write(alarm_day);
      Wire.write(alarm_weekday);
      Wire.endTransmission(); */
}

// Convert normal decimal numbers to binary coded decimal
uint8_t decToBcd(int val)
{
    return (uint8_t)((val / 10 * 16) + (val % 10));
}
// Convert binary coded decimal to normal decimal numbers
static int bcdToDec(uint8_t val)
{
    return (int)((val / 16 * 10) + (val % 16));
}
#include <time.h>
#include "stdio.h"
#include "driver/i2c.h"

#ifndef ZEPHYR_DRIVERS_RTC_PCF85063A_PCF85063A_H_
#define ZEPHYR_DRIVERS_RTC_PCF85063A_PCF85063A_H_

#define I2C_MASTER_SCL_IO 33        // CONFIG_I2C_MASTER_SCL      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 26        // CONFIG_I2C_MASTER_SDA      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0            /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ   /*!< I2C master read */
#define ACK_CHECK_EN 0x1           /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0          /*!< I2C master will not check ack from slave */

#define PCF85063A_BCD_UPPER_SHIFT 4
#define PCF85063A_BCD_LOWER_MASK 0x0f
#define PCF85063A_BCD_UPPER_MASK 0xf0
#define PCF85063A_BCD_UPPER_MASK_SEC 0x70

#define PCF85063A_CTRL1 0x00
#define PCF85063A_CTRL1_EXT_TEST BIT(7)
#define PCF85063A_CTRL1_STOP BIT(5)
#define PCF85063A_CTRL1_SR BIT(4)
#define PCF85063A_CTRL1_CIE BIT(2)
#define PCF85063A_CTRL1_12_24 BIT(1)
#define PCF85063A_CTRL1_CAP_SEL BIT(0)

#define PCF85063A_CTRL2 0x01
#define PCF85063A_CTRL2_AIE BIT(7)
#define PCF85063A_CTRL2_AF BIT(6)
#define PCF85063A_CTRL2_MI BIT(5)
#define PCF85063A_CTRL2_HMI BIT(4)
#define PCF85063A_CTRL2_TF BIT(3)

/* CLKOUT frequency selection */
#define PCF85063A_CTRL2_COF_32K 0x0
#define PCF85063A_CTRL2_COF_16K 0x1
#define PCF85063A_CTRL2_COF_8K 0x2
#define PCF85063A_CTRL2_COF_4K 0x3
#define PCF85063A_CTRL2_COF_2K 0x4
#define PCF85063A_CTRL2_COF_1K 0x5
#define PCF85063A_CTRL2_COF_1 0x6
#define PCF85063A_CTRL2_COF_LOW 0x7

#define PCF85063A_OFFSET 0x02
#define PCF85063A_RAM_BYTE 0x03
#define PCF85063A_TENS_SHIFT 4

/* Time related */
#define PCF85063A_SECONDS 0x04
#define PCF85063A_SECONDS_OS BIT(7)
#define PCF85063A_SECONDS_MASK 0x7f

#define PCF85063A_MINUTES 0x05
#define PCF85063A_MINUTES_MASK 0x7f

#define PCF85063A_HOURS 0x06
#define PCF85063A_HOURS_AM_PM BIT(5)
#define PCF85063A_HOURS_MASK 0x3f

#define PCF85063A_DAYS 0x07
#define PCF85063A_DAYS_MASK 0x3f

#define PCF85063A_WEEKDAYS 0x08
#define PCF85063A_WEEKDAYS_MASK 0x7
#define PCF85063A_WEEKDAYS_SUN 0x0
#define PCF85063A_WEEKDAYS_MON 0x1
#define PCF85063A_WEEKDAYS_TUE 0x2
#define PCF85063A_WEEKDAYS_WED 0x3
#define PCF85063A_WEEKDAYS_THU 0x4
#define PCF85063A_WEEKDAYS_FRI 0x5
#define PCF85063A_WEEKDAYS_SAT 0x6

#define PCF85063A_MONTHS 0x09
#define PCF85063A_MONTHS_MASK 0x1f
#define PCF85063A_MONTHS_JAN 0x1
#define PCF85063A_MONTHS_FEB 0x2
#define PCF85063A_MONTHS_MAR 0x3
#define PCF85063A_MONTHS_APR 0x4
#define PCF85063A_MONTHS_MAY 0x5
#define PCF85063A_MONTHS_JUN 0x6
#define PCF85063A_MONTHS_JUL 0x7
#define PCF85063A_MONTHS_AUG 0x8
#define PCF85063A_MONTHS_SEP 0x9
#define PCF85063A_MONTHS_OCT 0x10
#define PCF85063A_MONTHS_NOV 0x11
#define PCF85063A_MONTHS_DEC 0x12

#define PCF85063A_YEARS 0x0a

/* Alarm related */
#define PCF85063A_SECOND_ALARM 0x0b
#define PCF85063A_SECOND_ALARM_EN BIT(7)

#define PCF85063A_MINUTE_ALARM 0x0c
#define PCF85063A_MINUTE_ALARM_EN BIT(7)

#define PCF85063A_HOUR_ALARM 0x0d
#define PCF85063A_HOUR_ALARM_EN BIT(7)
#define PCF85063A_HOUR_ALARM_AM_PM BIT(5)

#define PCF85063A_DAY_ALARM 0x0e
#define PCF85063A_DAY_ALARM_EN BIT_MASK(7)

#define PCF85063A_WEEKDAY_ALARM 0x0f
#define PCF85063A_WEEKDAY_ALARM_EN BIT(7)

/* Timer registers */
#define PCF85063A_TIMER_VALUE 0x10
#define PCF85063A_TIMER_MODE 0x11
#define PCF85063A_TIMER_MODE_FREQ_MASK BIT(4) | BIT(3)
#define PCF85063A_TIMER_MODE_FREQ_SHIFT 3
#define PCF85063A_TIMER_MODE_FREQ_4K 0x0
#define PCF85063A_TIMER_MODE_FREQ_64 0x1
#define PCF85063A_TIMER_MODE_FREQ_1 0x2
#define PCF85063A_TIMER_MODE_FREQ_1_60 0x3
#define PCF85063A_TIMER_MODE_EN BIT(2)
#define PCF85063A_TIMER_MODE_INT_EN BIT(1)
#define PCF85063A_TIMER_MODE_INT_TI_TP BIT(0)

#define REG_CTRL1_ADDR 0x00
#define REG_CTRL2_ADDR 0x01
#define REG_TIME_ADDR 0x04
#define REG_DATE_ADDR 0x07

#define I2C_ADDR 0x51 << 1

#define YEAR_OFFSET 1970

#define RTC_ALARM 0x80 // set AEN_x registers

uint8_t I2C_ReadRegister(uint8_t reg_addr, uint8_t *data, size_t len);
uint8_t I2C_Master_write(uint8_t reg_addr, uint8_t *data, uint8_t len);

void PCF85_Init();

void PCF85_Reset();

void PCF85_SetTime(uint8_t hour, uint8_t minute, uint8_t second);

void PCF85_SetDate(uint8_t weekday, uint8_t day, uint8_t month, uint16_t yr);

uint8_t PCF85_Readnow(uint8_t *buf);

void enableAlarm();
void disableAlarm();

void setAlarm(uint8_t alarm_second, uint8_t alarm_minute, uint8_t alarm_hour, uint8_t alarm_day, uint8_t alarm_weekday);

#endif /* ZEPHYR_DRIVERS_RTC_PCF85063A_PCF85063A_H_ */

// weekday format
// 0 - sunday
// 1 - monday
// 2 - tuesday
// 3 - wednesday
// 4 - thursday
// 5 - friday
// 6 - saturday
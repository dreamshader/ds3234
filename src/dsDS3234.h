/*
 ***********************************************************************
 *
 *  dsDS3234.h - definitions for ds3234 SPI RTC class
 *
 *  Copyright (C) 2018 Dreamshader (aka Dirk Schanz)
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ***********************************************************************
 */

#ifndef _DSDS3234_H_
#define _DSDS3234_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/spi/spidev.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#ifdef RASPBERRY
#include <pigpio.h>
#endif // RASPBERRY

#include "dsConnection.h"

#ifdef __cplusplus
extern "C" {
#endif
 

#define DS324_SPI_BUS                          0
#define DS324_SPI_DEVICE                       0
#define DS324_SPI_MODE             (SPI_MODE_1 | SPI_NO_CS)
#define DS324_SPI_BITS_PER_WORD                8
#define DS324_SPI_SPEED                   500000 
#define DS324_SPI_DELAY                        0

#define DS324_PIN_SQW                         24
#define DS324_PIN_MOSI                        10
#define DS324_PIN_MISO                         9
#define DS324_PIN_SCK                         11
#define DS324_PIN_CS0                          8
#define DS324_PIN_CS1                          7

#define DS3234_DATA_BUF_SIZE                  33

#define DS3234_ERR_SETTINGS_NULL             -33
#define DS3234_ERR_ALARM_NUMBER              -34
#define DS3234_ERR_ALARM_SETTINGS_NULL       -35


#define DS3234_REG_00_SECONDS_MASK            0b01111111
#define DS3234_REG_01_MINUTES_MASK            0b01111111
#define DS3234_REG_02_HOURS_MASK              0b00011111
#define DS3234_REG_02_AM_PM_10HOURS_MASK      0b00100000
#define DS3234_REG_02_12_24_FLAG_MASK         0b01000000
#define DS3234_REG_03_DAY_MASK                0b00000111
#define DS3234_REG_04_DATE_MASK               0b00011111
#define DS3234_REG_05_MONTH_MASK              0b00011111
#define DS3234_REG_05_CENTURY_MASK            0b10000000
#define DS3234_REG_06_YEAR_MASK               0b11111111

#define DS3234_ADDR_READ_ALL                  0x00
#define DS3234_NUM_ACCESSIBLE_REGS            0x15

#define DS3234_ADDR_READ_DATE                 0x00
#define DS3234_ADDR_WRITE_DATE                0x80
#define DS3234_NUM_DATE_REGS                   8

#define DS3234_NUM_ALARM_1_REGS                5
#define DS3234_NUM_ALARM_2_REGS                4


#define DS3234_ALARM_ONCE_PER_SECOND          0b00001111
#define DS3234_ALARM_SEC_MATCH                0b00001110
#define DS3234_ALARM_MIN_SEC_MATCH            0b00001100
#define DS3234_ALARM_HOUR_MIN_SEC_MATCH       0b00001000
#define DS3234_ALARM_DATE_HOUR_MIN_SEC_MATCH  0b00000000
#define DS3234_ALARM_DAY_HOUR_MIN_SEC_MATCH   0b00010000

#define DS3234_ALARM_ONCE_PER_MINUTE          0b00001110
#define DS3234_ALARM_MIN_MATCH                0b00001100
#define DS3234_ALARM_HOUR_MIN_MATCH           0b00001000
#define DS3234_ALARM_DATE_HOUR_MIN_MATCH      0b00000000
#define DS3234_ALARM_DAY_HOUR_MIN_MATCH       0b00010000


struct _ds3234_timestruct_ {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t wday;
    uint8_t date;
    uint8_t month;
    uint8_t year;
    uint8_t hour24;
    uint8_t century;

};


struct _ds3234_alarm_settings_ {
    uint8_t alrmNo;
    uint8_t alrmMode;
    bool    enable;
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t date;

};



class dsDS3234 {

    protected:
        dsConnection connection;

        uint8_t dataIn[DS3234_DATA_BUF_SIZE];
        uint8_t dataOut[DS3234_DATA_BUF_SIZE];


    protected:
        void buffer2Timestruct( uint8_t buffer[], 
                             struct _ds3234_timestruct_ *pTime, int numRegs );
        void timestruct2Buffer( uint8_t regs[], 
                             struct _ds3234_timestruct_ *pTime, int numRegs );
        void maskRegisterContent( uint8_t regs[], int numRegs );

    public:
        dsDS3234( void );
        ~dsDS3234( void );

        int connectSPI( int bus, int device, int csPin, uint8_t  spiMode, 
                        uint8_t  spiBits, uint32_t spiBaud, uint16_t spiDelay,
                        uint32_t maxDataSize );

        void printAlarmSettings(struct _ds3234_alarm_settings_ *pAlarmSettings);
        void printTimestruct( struct _ds3234_timestruct_ *pTime );

        int readDate( struct _ds3234_timestruct_ *pTime );
        int writeDate( struct _ds3234_timestruct_ *pTime );

        int setAlarm( struct _ds3234_alarm_settings_ *pAlarmSettings );
        int readAlarm( struct _ds3234_alarm_settings_ *pAlarmSettings );
        int clearAlarm( struct _ds3234_alarm_settings_ *pAlarmSettings );
        int enableAlarm( int alarmNo );
        int disableAlarm( int alarmNo );

        int readAll( void );



};


#ifdef __cplusplus
}
#endif

#endif // _DSDS3234_H_



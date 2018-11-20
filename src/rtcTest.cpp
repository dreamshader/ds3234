/*
 ***********************************************************************
 *
 *  dsDS3234.cpp - class file for ds3234 SPI RTC
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

#include "dsUtil.h"
#include "dsSPI.h"

#include "dsDS3234.h"

int main(int argc, char *argv[])
{
    int retVal;

    int bus              = DS324_SPI_BUS;
    int device           = DS324_SPI_DEVICE;
    int csPin            = DS324_PIN_CS0;
    uint8_t  spiMode     = DS324_SPI_MODE;
    uint8_t  spiBits     = DS324_SPI_BITS_PER_WORD;
    uint32_t spiBaud     = DS324_SPI_SPEED;
    uint16_t spiDelay    = DS324_SPI_DELAY;
    uint32_t maxDataSize = DS3234_DATA_BUF_SIZE;

    time_t currSecs;
    struct tm *pNow;

    dsDS3234 clock1;

    struct _ds3234_timestruct_     currTime;
    struct _ds3234_alarm_settings_ alarmSettings;

#ifdef _USE_PIGPIO_
    if( (retVal = gpioInitialise()) >= 0)
    {
#else // NOT _USE_PIGPIO_
    if( (retVal = pinLock( csPin, DSGPIO_PIN_MODE_OUTPUT )) >= 0 )
    {
        if( retVal = pinState( csPin, DSGPIO_ACTION_SET_STATE, 
                                   DSGPIO_PIN_STATE_HIGH ) >= 0 )
        {
#endif // _USE_PIGPIO_

            retVal = clock1.connectSPI(bus, device, csPin, spiMode, spiBits, 
                                    spiBaud, spiDelay, maxDataSize );

            printf("connectSPI: retVal = %d\n", retVal );

            if( retVal >= 0 )
            {
                retVal = clock1.readDate( &currTime );
                printf("readDate: retVal = %d\n", retVal );

                time(&currSecs);
                pNow = localtime(&currSecs);

                currTime.second  = pNow->tm_sec;         /* seconds */
                currTime.minute  = pNow->tm_min;         /* minutes */
                currTime.hour    = pNow->tm_hour;        /* hours */

                currTime.hour24  =  0;

                currTime.wday    = pNow->tm_wday + 1;    /* day of the week */
                currTime.date    = pNow->tm_mday;        /* day of the month */
                currTime.month   = pNow->tm_mon + 1;     /* month */
                currTime.year    = pNow->tm_year;        /* year */

                currTime.century =  0;

                clock1.printTimestruct( &currTime );


                retVal = clock1.writeDate( &currTime );
                printf("writeDate: retVal = %d\n", retVal );

                alarmSettings.alrmNo   = 1;
                alarmSettings.alrmMode = 0;
                alarmSettings.enable   = false;
                alarmSettings.second   = 23;
                alarmSettings.minute   = 10;
                alarmSettings.hour     = 22;
                alarmSettings.date     = 17;

                clock1.printAlarmSettings( &alarmSettings );
                retVal = clock1.setAlarm( &alarmSettings );
                printf("setAlarm: retVal = %d\n", retVal );

                retVal = clock1.readAlarm( &alarmSettings );
                printf("writeDate: retVal = %d\n", retVal );
                clock1.printAlarmSettings( &alarmSettings );

                retVal = clock1.enableAlarm( 1 );
                printf("enableAlarm: retVal = %d\n", retVal );

                retVal = clock1.disableAlarm( 1 );
                printf("disableAlarm: retVal = %d\n", retVal );

                alarmSettings.alrmNo   = 1;
                alarmSettings.alrmMode = 0;
                alarmSettings.enable   = false;
                alarmSettings.second   = 23;
                alarmSettings.minute   = 10;
                alarmSettings.hour     = 22;
                alarmSettings.date     = 17;
    
                retVal = clock1.clearAlarm( &alarmSettings );
                printf("clearAlarm: retVal = %d\n", retVal );

            } 

#ifdef _USE_PIGPIO_
        gpioTerminate();
    }
#else // NOT _USE_PIGPIO_
        }
        pinRelease( csPin );
    }
#endif // _USE_PIGPIO_


    return( retVal );

}




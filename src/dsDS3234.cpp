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





dsDS3234::dsDS3234()
{

}

dsDS3234::~dsDS3234()
{

}


int dsDS3234::connectSPI( int bus, int device, int csPin, uint8_t  spiMode, uint8_t  spiBits, uint32_t spiBaud, uint16_t spiDelay, uint32_t maxDataSize )
{
    return( connection.connectSPI( bus, device, csPin, spiMode, spiBits, 
                        spiBaud, spiDelay, maxDataSize) );
}







void dsDS3234::buffer2Timestruct( uint8_t buffer[], struct _ds3234_timestruct_ *pTime, int numRegs )
{
    pTime->second = bcd2bin( buffer[0] & DS3234_REG_00_SECONDS_MASK );
    pTime->minute = bcd2bin( buffer[1] & DS3234_REG_01_MINUTES_MASK );

    pTime->hour   = bcd2bin( buffer[2] & (DS3234_REG_02_HOURS_MASK |
                                           DS3234_REG_02_AM_PM_10HOURS_MASK) );

    pTime->hour24 = (buffer[2] & DS3234_REG_02_12_24_FLAG_MASK) >> 6;

    pTime->wday = bcd2bin( buffer[3] & DS3234_REG_03_DAY_MASK );
    pTime->date = bcd2bin( buffer[4] & DS3234_REG_04_DATE_MASK );
    pTime->month = bcd2bin( buffer[5] & DS3234_REG_05_MONTH_MASK );
    pTime->year = bcd2bin( buffer[6] & DS3234_REG_06_YEAR_MASK );

    pTime->century = (buffer[5] & DS3234_REG_05_CENTURY_MASK) >> 7;
}


void dsDS3234::maskRegisterContent( uint8_t regs[], int numRegs )
{
    regs[0] &= DS3234_REG_00_SECONDS_MASK;
    regs[1] &= DS3234_REG_01_MINUTES_MASK;
    regs[2] &= (DS3234_REG_02_HOURS_MASK |
                DS3234_REG_02_AM_PM_10HOURS_MASK |
                DS3234_REG_02_12_24_FLAG_MASK );

    regs[3] &= DS3234_REG_03_DAY_MASK;
    regs[4] &= DS3234_REG_04_DATE_MASK;
    regs[5] &= ( DS3234_REG_05_MONTH_MASK |
                 DS3234_REG_05_CENTURY_MASK );
    regs[6] &= DS3234_REG_06_YEAR_MASK;
}

void dsDS3234::timestruct2Buffer( uint8_t regs[], struct _ds3234_timestruct_ *pTime, int numRegs )
{
    regs[0] = bin2bcd( pTime->second );
    regs[1] = bin2bcd( pTime->minute );
    regs[2] = bin2bcd( pTime->hour );
    regs[3] = bin2bcd( pTime->wday );
    regs[4] = bin2bcd( pTime->date );
    regs[5] = bin2bcd( pTime->month );
    regs[6] = bin2bcd( pTime->year );

    maskRegisterContent( regs, numRegs );
}


void dsDS3234::printTimestruct( struct _ds3234_timestruct_ *pTime )
{

    printf("%02d:%02d.%02d\n", pTime->hour, pTime->minute, pTime->second );
    printf("%02d.%02d.%d\n", pTime->date, pTime->month, pTime->year+1900 );

    printf("hour24 is %s\n", (pTime->hour24 > 0 ? "am/pm" : "24 hours") );
    printf("century is %d\n", pTime->century );
    printf("wday is %d\n", pTime->wday );
}


void dsDS3234::printAlarmSettings( struct _ds3234_alarm_settings_ *pAlarmSettings )
{
    if( pAlarmSettings != NULL )
    {
        fprintf(stdout, "alrmNo ..: %d\n", pAlarmSettings->alrmNo );
        fprintf(stdout, "alrmMode : %d\n", pAlarmSettings->alrmMode );
        fprintf(stdout, "enable ..: %s\n", 
                        pAlarmSettings->enable == true ? "true" : "false" );
        fprintf(stdout, "second ..: %d\n", pAlarmSettings->second );
        fprintf(stdout, "minute ..: %d\n", pAlarmSettings->minute );
        fprintf(stdout, "hour ....: %d\n", pAlarmSettings->hour );
        fprintf(stdout, "date ....: %d\n", pAlarmSettings->date );
    }
}


int dsDS3234::readDate( struct _ds3234_timestruct_ *pTime )
{
    int retVal;

    memset(dataOut, '\0', DS3234_DATA_BUF_SIZE );
    memset(dataIn, '\0', DS3234_DATA_BUF_SIZE );

    dataOut[0] = DS3234_ADDR_READ_DATE;

    retVal = connection.xfer( DS_CONNECTION_MODE_READ, (void*) dataOut, 
                              (void*) dataIn, DS3234_NUM_DATE_REGS + 1 );

#ifdef _DEBUG_
    printf("retVal = %d\n", retVal);
    dumpBuffer( dataIn, retVal );
#endif // _DEBUG_

    buffer2Timestruct( &dataIn[1], pTime, retVal );

#ifdef _DEBUG_
    printTimestruct( pTime );
#endif // _DEBUG_

    return( retVal );
}

int dsDS3234::writeDate( struct _ds3234_timestruct_ *pTime )
{
    int retVal;

#ifdef _DEBUG_
    fprintf(stderr, "Set date/time to:\n");
    printTimestruct( pTime );
#endif // _DEBUG_

    memset(dataOut, '\0', DS3234_DATA_BUF_SIZE );
    memset(dataIn, '\0', DS3234_DATA_BUF_SIZE );

    dataOut[0] = DS3234_ADDR_WRITE_DATE;

    timestruct2Buffer( &dataOut[1], pTime, DS3234_NUM_DATE_REGS );

#ifdef _DEBUG_
    dumpBuffer( dataOut, DS3234_NUM_DATE_REGS );
#endif // _DEBUG_

    retVal = connection.xfer( DS_CONNECTION_MODE_WRITE, (void*) dataOut, 
                              (void*) dataIn, DS3234_NUM_DATE_REGS );

#ifdef _DEBUG_
    printf("retVal = %d\n", retVal);
    dumpBuffer( dataIn, retVal );
#endif // _DEBUG_

    return( retVal );
}


int dsDS3234::setAlarm( struct _ds3234_alarm_settings_ *pAlarmSettings )
{
    int retVal;

    if( pAlarmSettings!= NULL )
    {

        memset(dataOut, '\0', DS3234_DATA_BUF_SIZE );
        memset(dataIn, '\0', DS3234_DATA_BUF_SIZE );

        if( pAlarmSettings->alrmNo == 1 )
        {
            dataOut[0] = 0x87;          // write to 0x87
            dataOut[1] = bin2bcd( pAlarmSettings->second );
            dataOut[2] = bin2bcd( pAlarmSettings->minute );
            dataOut[3] = bin2bcd( pAlarmSettings->hour );
            dataOut[4] = bin2bcd( pAlarmSettings->date );

            dataOut[1] |= ((pAlarmSettings->alrmMode << 7) & 0b10000000);
            dataOut[2] |= ((pAlarmSettings->alrmMode << 6) & 0b10000000);
            dataOut[3] |= ((pAlarmSettings->alrmMode << 5) & 0b10000000);
            dataOut[4] |= ((pAlarmSettings->alrmMode << 4) & 0b10000000);
            dataOut[3] |= ((pAlarmSettings->alrmMode << 2) & 0b01000000);

#ifdef _DEBUG_
    dumpBuffer( dataOut, DS3234_NUM_ALARM_1_REGS );
#endif // _DEBUG_

    retVal = connection.xfer( DS_CONNECTION_MODE_WRITE, (void*) dataOut, 
                              (void*) dataIn, DS3234_NUM_ALARM_1_REGS );

        }
        else
        {
            if( pAlarmSettings->alrmNo == 2 )
            {
                dataOut[0] = 0x8B;          // write to 0x8B
                dataOut[1] = bin2bcd( pAlarmSettings->minute );
                dataOut[2] = bin2bcd( pAlarmSettings->hour );
                dataOut[3] = bin2bcd( pAlarmSettings->date );

                dataOut[1] |= ((pAlarmSettings->alrmMode << 6) & 0b10000000);
                dataOut[2] |= ((pAlarmSettings->alrmMode << 5) & 0b10000000);
                dataOut[3] |= ((pAlarmSettings->alrmMode << 4) & 0b10000000);
                dataOut[3] |= ((pAlarmSettings->alrmMode << 2) & 0b01000000);

#ifdef _DEBUG_
    dumpBuffer( dataOut, DS3234_NUM_ALARM_2_REGS );
#endif // _DEBUG_

    retVal = connection.xfer( DS_CONNECTION_MODE_WRITE, (void*) dataOut, 
                              (void*) dataIn, DS3234_NUM_ALARM_2_REGS );

            }
            else
            {
                retVal = DS3234_ERR_ALARM_NUMBER;
            }
        }
    }
    else
    {
        retVal = DS3234_ERR_ALARM_SETTINGS_NULL;
    }

    return( retVal );
}

int dsDS3234::readAlarm( struct _ds3234_alarm_settings_ *pAlarmSettings )
{
    int retVal;

        if( pAlarmSettings!= NULL )
        {

            memset(dataOut, '\0', DS3234_DATA_BUF_SIZE );
            memset(dataIn, '\0', DS3234_DATA_BUF_SIZE );

            if( pAlarmSettings->alrmNo == 1 )
            {
                dataOut[0] = 0x07;          // read from 0x07

    retVal = connection.xfer( DS_CONNECTION_MODE_WRITE, (void*) dataOut, 
                              (void*) dataIn, DS3234_NUM_ALARM_1_REGS );

#ifdef _DEBUG_
    dumpBuffer( dataOut, DS3234_NUM_ALARM_1_REGS );
#endif // _DEBUG_

                pAlarmSettings->second = bcd2bin( dataIn[1] );
                pAlarmSettings->minute = bcd2bin( dataIn[2] );
                pAlarmSettings->hour   = bcd2bin( dataIn[3] );
                pAlarmSettings->date   = bcd2bin( dataIn[4] );

            }
            else
            {
                if( pAlarmSettings->alrmNo == 2 )
                {
                    dataOut[0] = 0x0B;          // read from 0x0B

    retVal = connection.xfer( DS_CONNECTION_MODE_WRITE, (void*) dataOut, 
                              (void*) dataIn, DS3234_NUM_ALARM_2_REGS );

#ifdef _DEBUG_
    dumpBuffer( dataOut, DS3234_NUM_ALARM_2_REGS );
#endif // _DEBUG_

                    pAlarmSettings->second = 0;
                    pAlarmSettings->minute = bcd2bin( dataIn[1] );
                    pAlarmSettings->hour   = bcd2bin( dataIn[2] );
                    pAlarmSettings->date   = bcd2bin( dataIn[3] );

                }
                else
                {
                    retVal = DS3234_ERR_ALARM_NUMBER;
                }
            }
        }
        else
        {
            retVal = DS3234_ERR_ALARM_SETTINGS_NULL;
        }

    return( retVal );
}



int dsDS3234::clearAlarm( struct _ds3234_alarm_settings_ *pAlarmSettings )
{
    int retVal;

        if( pAlarmSettings!= NULL )
        {

            memset(dataOut, '\0', DS3234_DATA_BUF_SIZE );
            memset(dataIn, '\0', DS3234_DATA_BUF_SIZE );

            if( pAlarmSettings->alrmNo == 1 )
            {
                dataOut[0] = 0x87;          // write to 0x87
                dataOut[1] = 0;
                dataOut[2] = 0;
                dataOut[3] = 0;
                dataOut[4] = 0;

    retVal = connection.xfer( DS_CONNECTION_MODE_WRITE, (void*) dataOut, 
                              (void*) dataIn, DS3234_NUM_ALARM_1_REGS );

            }
            else
            {
                if( pAlarmSettings->alrmNo == 2 )
                {
                    dataOut[0] = 0x8B;          // write to 0x8B
                    dataOut[1] = 0;
                    dataOut[2] = 0;
                    dataOut[3] = 0;

    retVal = connection.xfer( DS_CONNECTION_MODE_WRITE, (void*) dataOut, 
                              (void*) dataIn, DS3234_NUM_ALARM_2_REGS );

                }
                else
                {
                    retVal = DS3234_ERR_ALARM_NUMBER;
                }
            }
        }
        else
        {
            retVal = DS3234_ERR_ALARM_SETTINGS_NULL;
        }

    return( retVal );
}


int dsDS3234::enableAlarm( int alarmNo )
{
    int retVal = 0;

    memset(dataOut, '\0', DS3234_DATA_BUF_SIZE );
    memset(dataIn, '\0', DS3234_DATA_BUF_SIZE );

    return( retVal );
}

int dsDS3234::disableAlarm( int alarmNo )
{
    int retVal = 0;

    memset(dataOut, '\0', DS3234_DATA_BUF_SIZE );
    memset(dataIn, '\0', DS3234_DATA_BUF_SIZE );

    return( retVal );
}

int dsDS3234::readAll( void )
{
    int retVal;

    memset(dataOut, '\0', DS3234_DATA_BUF_SIZE );
    memset(dataIn, '\0', DS3234_DATA_BUF_SIZE );

    dataOut[0] = DS3234_ADDR_READ_ALL;

    retVal = connection.xfer( DS_CONNECTION_MODE_WRITE, (void*) dataOut, 
                              (void*) dataIn, DS3234_NUM_ACCESSIBLE_REGS );

#ifdef _DEBUG_
    printf("retVal = %d\n", retVal);
    dumpBuffer( dataIn, retVal );
#endif // _DEBUG_
 
    return( retVal );

}




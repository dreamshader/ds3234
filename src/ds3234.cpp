#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
// #include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pigpio.h>

#define DS324_SPI_BUS                       0
#define DS324_SPI_DEVICE                    0
#define DS324_SPI_MODE          (SPI_MODE_1 | SPI_NO_CS)
#define DS324_SPI_BITS_PER_WORD             8
#define DS324_SPI_SPEED                500000 
#define DS324_SPI_DELAY                     0

#define DS324_PIN_SQW                      24
#define DS324_PIN_MOSI                     10
#define DS324_PIN_MISO                      9
#define DS324_PIN_SCK                      11
#define DS324_PIN_CS0                       8
#define DS324_PIN_CS1                       7

#define DS3234_SPI_R_BUF_SIZE              33  // 32 byte max
#define DS3234_SPI_W_BUF_SIZE              33  // 32 byte max

#define ERROR_PARAM_SETTINGS_NULL         -33
#define ERROR_ALARM_NO                    -34
#define ERROR_PARAM_ALARM_SETTINGS_NULL   -35


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

#define DS3234_ADDR_READ_ALL               0x00
#define DS3234_NUM_ACCESSIBLE_REGS         0x15

#define DS3234_ADDR_READ_DATE              0x00
#define DS3234_ADDR_WRITE_DATE             0x80
#define DS3234_NUM_DATE_REGS                8

#define DS3234_NUM_ALARM_1_REGS             5
#define DS3234_NUM_ALARM_2_REGS             4


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


struct timeStruc {
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

struct _spi_device_settings_ {
    uint8_t mode;
    uint8_t bits;
    uint32_t baud;
    uint16_t delay;
};

struct _alarm_settings_ {
    uint8_t alrmNo;
    uint8_t alrmMode;
    bool    enable;
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t date;

};

struct _spi_device_ {

    const char *pDevice;
    int         spiHandle;
    uint8_t     csPin;

    struct _spi_device_settings_ spiSettingsOld;
    struct _spi_device_settings_ spiSettingsNew;

    uint8_t spiReadBuf[DS3234_SPI_R_BUF_SIZE];
    uint8_t spiWriteBuf[DS3234_SPI_W_BUF_SIZE];
};


void printBits( uint32_t value, int bits, uint8_t flags )
{
    for( int i = bits-1; i >= 0; i-- )
    {
        printf("%c", '0' + ((value >> i) & 1) );
    }
}

char *bits2str( uint32_t value, int bits, uint8_t flags )
{
    static char bitBuffer[ sizeof(uint32_t) * 8 + 1];

    memset( bitBuffer, '\0', (sizeof(uint32_t) * 8) + 1 );
    int x = 0;
    for( int i = bits-1; i >= 0; i-- )
    {
        bitBuffer[x++] = '0' + ((value >> i) & 1);
    }
    return( &bitBuffer[0] );
}


void dumpBuffer( uint8_t responseBuffer[], int len )
{
    for( int i = 0; i < len; i++ )
    {
        printf("buffer[%02X] -> %3d -> 0x%02X -> %s\n", i, responseBuffer[i], 
            responseBuffer[i], bits2str( responseBuffer[i], 8, 0 ) );
    }
}

uint8_t bin2bcd(uint8_t val) 
{
  return((val/10*16) + (val%10));
}

uint8_t bcd2bin(uint8_t val) 
{
  return((val/16*10) + (val%16));
}


void regs2Date( uint8_t regs[], struct timeStruc *pTime, int numRegs )
{
    pTime->second = bcd2bin( regs[0] & DS3234_REG_00_SECONDS_MASK );
    pTime->minute = bcd2bin( regs[1] & DS3234_REG_01_MINUTES_MASK );
    pTime->hour   = bcd2bin( regs[2] & DS3234_REG_02_HOURS_MASK );

    pTime->hour += ((regs[2] & DS3234_REG_02_AM_PM_10HOURS_MASK) >> 5) * 10;
    pTime->hour24 = (regs[2] & DS3234_REG_02_12_24_FLAG_MASK) >> 6;

    pTime->wday = bcd2bin( regs[3] & DS3234_REG_03_DAY_MASK );
    pTime->date = bcd2bin( regs[4] & DS3234_REG_04_DATE_MASK );
    pTime->month = bcd2bin( regs[5] & DS3234_REG_05_MONTH_MASK );
    pTime->year = bcd2bin( regs[6] & DS3234_REG_06_YEAR_MASK );

    pTime->century = (regs[5] & DS3234_REG_05_CENTURY_MASK) >> 7;
}


void mapRegs( uint8_t regs[], int numRegs )
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

void date2Regs( uint8_t regs[], struct timeStruc *pTime, int numRegs )
{
    regs[0] = bin2bcd( pTime->second );
    regs[1] = bin2bcd( pTime->minute );
    regs[2] = bin2bcd( pTime->hour );
    regs[3] = bin2bcd( pTime->wday );
    regs[4] = bin2bcd( pTime->date );
    regs[5] = bin2bcd( pTime->month );
    regs[6] = bin2bcd( pTime->year );

    mapRegs( regs, numRegs );
}


void printDate( struct timeStruc *pTime )
{

    printf("%02d:%02d.%02d\n", pTime->hour, pTime->minute, pTime->second );
    printf("%02d.%02d.%d\n", pTime->date, pTime->month, pTime->year+1900 );

    printf("hour24 is %s\n", (pTime->hour24 > 0 ? "am/pm" : "24 hours") );
    printf("century is %d\n", pTime->century );
    printf("wday is %d\n", pTime->wday );
}


int spiXferMulti8(int spiHandle, const uint8_t csPin, struct _spi_device_settings_ *pSettings, uint8_t *pDataOut, uint8_t *pDataIn, int dataLength)
{
    struct spi_ioc_transfer spiData; 
    int retVal;

    if( pSettings != NULL )
    {
        memset( &spiData, '\0', sizeof(spiData) );

        spiData.tx_buf        = (unsigned long) pDataOut;
        spiData.rx_buf        = (unsigned long) pDataIn;
        spiData.len           = dataLength;

        spiData.delay_usecs   = pSettings->delay;
        spiData.speed_hz      = pSettings->baud;
        spiData.bits_per_word = pSettings->bits;
        spiData.cs_change     = 0;

        gpioWrite(DS324_PIN_CS0, PI_LOW);
        if( (retVal = ioctl(spiHandle, SPI_IOC_MESSAGE(1), &spiData)) < 0 )
        {
            perror("SPI Message failed.");
        }
        gpioWrite(DS324_PIN_CS0, PI_HIGH);

    }
    else
    {
        retVal = ERROR_PARAM_SETTINGS_NULL;
    }
    return retVal;
}


int spiXferSingle8(int spiHandle, const uint8_t csPin, struct _spi_device_settings_ *pSettings, uint8_t *pDataOut, uint8_t *pDataIn, int dataLength)
{
    struct spi_ioc_transfer spiData[dataLength]; 
    int retVal;

    if( pSettings != NULL )
    {
        gpioWrite(DS324_PIN_CS0, PI_LOW);
        for(int i = 0; i < dataLength; i++)
        {
            memset( &spiData[i], '\0', sizeof(spiData[i]) );
            spiData[i].tx_buf        = (unsigned long)(pDataOut + i);
            spiData[i].rx_buf        = (unsigned long)(pDataIn + i);
            spiData[i].len           = 1;

            spiData[i].delay_usecs   = pSettings->delay;
            spiData[i].speed_hz      = pSettings->baud;
            spiData[i].bits_per_word = pSettings->bits;

            spiData[i].cs_change     = 0;
        }

        if( (retVal = ioctl(spiHandle, SPI_IOC_MESSAGE(dataLength), spiData)) < 0 )
        {
            perror("SPI Message failed.");
        }
        gpioWrite(DS324_PIN_CS0, PI_HIGH);
    }
    else
    {
        retVal = ERROR_PARAM_SETTINGS_NULL;
    }

    return retVal;
}


int spiReadSettings( int spiHandle, struct _spi_device_settings_ *pSettings )
{
    int retVal;

    if( pSettings != NULL )
    {
        if( (retVal = ioctl(spiHandle, SPI_IOC_RD_MODE, 
                            &pSettings->mode )) >= 0 )
        {
            if( (retVal = ioctl(spiHandle, SPI_IOC_RD_BITS_PER_WORD, 
                                &pSettings->bits )) >= 0 )
            {
                if( (retVal = ioctl(spiHandle, SPI_IOC_RD_MAX_SPEED_HZ, 
                                    &pSettings->baud )) < 0 )
                {
                    perror("read spi settings >speed<");
                }
            }
            else
            {
                perror("read spi settings >wordsize<");
            }
        }
        else
        {
            perror("read spi settings >mode<");
        }
    }
    else
    {
        retVal = ERROR_PARAM_SETTINGS_NULL;
    }
   
    return( retVal );
}


int spiWriteSettings( int spiHandle, struct _spi_device_settings_ *pSettings )
{
    int retVal;

    if( pSettings != NULL )
    {
        if( (retVal = ioctl(spiHandle, SPI_IOC_WR_MODE, 
                            &pSettings->mode )) >= 0 )
        {
            if( (retVal = ioctl(spiHandle, SPI_IOC_WR_BITS_PER_WORD, 
                                &pSettings->bits )) >= 0 )
            {
                if( (retVal = ioctl(spiHandle, SPI_IOC_WR_MAX_SPEED_HZ, 
                                    &pSettings->baud )) < 0 )
                {
                    perror("read spi settings >mode<");
                }
            }
            else
            {
                perror("read spi settings >wordsize<");
            }
        }
        else
        {
            perror("read spi settings >speed<");
        }
    }
    else
    {
        retVal = ERROR_PARAM_SETTINGS_NULL;
    }
   
    return( retVal );
}


int spiSetup( const char *pDevice, 
              struct _spi_device_settings_ *pNewSettings, 
              struct _spi_device_settings_ *pOldSettings )
{
    int retVal;
    int spiHandle;

    if( pDevice != NULL )
    {
        if( pNewSettings != NULL )
        {
            if( pOldSettings != NULL )
            {
                if( (spiHandle = open(pDevice, O_RDWR)) >= 0)
                {
                    if( (retVal = spiReadSettings( spiHandle, 
                                                    pOldSettings  )) >= 0 )
                    {
                        if( (retVal = spiWriteSettings( spiHandle, 
                                                        pNewSettings  )) >= 0 )
                        {
                            retVal = spiHandle;

                            printf("SPI dev ......: %s\n", pDevice);
                            printf("SPI mode .....: %d\n", pNewSettings->mode);
                            printf("bits per word : %d\n", pNewSettings->bits);
                            printf("delay ........: %d\n", pNewSettings->delay);
                            printf("baud .........: %d Hz (%d kHz)\n", 
                                                pNewSettings->baud, 
                                                pNewSettings->baud/1000);
                        }
                    }
                }
            }
        }
    }
    return( retVal );
}


void readDate( int handle, int cs, struct _spi_device_settings_ *pSettings, 
               uint8_t wBuf[], uint8_t rBuf[], struct timeStruc *pTime )
{
    int retVal;

    wBuf[0] = DS3234_ADDR_READ_DATE;

    retVal = spiXferMulti8(handle, cs, pSettings, wBuf, rBuf, 
                            DS3234_NUM_DATE_REGS);

    printf("retVal = %d\n", retVal);
    dumpBuffer( rBuf, retVal );

    regs2Date( &rBuf[1], pTime, retVal );

    printDate( pTime );

}

void writeDate( int handle, int cs, struct _spi_device_settings_ *pSettings, 
                uint8_t wBuf[], uint8_t rBuf[], struct timeStruc *pTime )
{
    int retVal;

//    printDate( pTime );

    wBuf[0] = DS3234_ADDR_WRITE_DATE;

    date2Regs( &wBuf[1], pTime, DS3234_NUM_DATE_REGS );

    dumpBuffer( wBuf, DS3234_NUM_DATE_REGS );

    retVal = spiXferMulti8(handle, cs, pSettings, wBuf, rBuf, 
                            DS3234_NUM_DATE_REGS);

    printf("retVal = %d\n", retVal);
    dumpBuffer( rBuf, retVal );
}

void readAllRegs( int handle, int cs, struct _spi_device_settings_ *pSettings, 
               uint8_t wBuf[], uint8_t rBuf[] )
{
    int retVal;

    wBuf[0] = DS3234_ADDR_READ_ALL;

    retVal = spiXferMulti8( handle, cs, pSettings, wBuf, rBuf, 
                            DS3234_NUM_ACCESSIBLE_REGS );

    printf("retVal = %d\n", retVal);
    dumpBuffer( rBuf, retVal );
}


int setAlarm( int handle, int cs, struct _spi_device_settings_ *pSettings, 
                uint8_t wBuf[], uint8_t rBuf[], 
                struct _alarm_settings_ *pAlarmSettings )
{
    int retVal;

    if( pSettings!= NULL )
    {
        if( pAlarmSettings!= NULL )
        {

            if( pAlarmSettings->alrmNo == 1 )
            {
                wBuf[0] = 0x87;          // write to 0x87
                wBuf[1] = bin2bcd( pAlarmSettings->second );
                wBuf[2] = bin2bcd( pAlarmSettings->minute );
                wBuf[3] = bin2bcd( pAlarmSettings->hour );
                wBuf[4] = bin2bcd( pAlarmSettings->date );

                wBuf[1] |= ((pAlarmSettings->alrmMode << 7) & 0b10000000);
                wBuf[2] |= ((pAlarmSettings->alrmMode << 6) & 0b10000000);
                wBuf[3] |= ((pAlarmSettings->alrmMode << 5) & 0b10000000);
                wBuf[4] |= ((pAlarmSettings->alrmMode << 4) & 0b10000000);
                wBuf[3] |= ((pAlarmSettings->alrmMode << 2) & 0b01000000);

    dumpBuffer( wBuf, DS3234_NUM_ALARM_1_REGS );

                retVal = spiXferMulti8(handle, cs, pSettings, wBuf, rBuf, 
                                        DS3234_NUM_ALARM_1_REGS);

            }
            else
            {
                if( pAlarmSettings->alrmNo == 2 )
                {
                    wBuf[0] = 0x8B;          // write to 0x8B
                    wBuf[1] = bin2bcd( pAlarmSettings->minute );
                    wBuf[2] = bin2bcd( pAlarmSettings->hour );
                    wBuf[3] = bin2bcd( pAlarmSettings->date );

                    wBuf[1] |= ((pAlarmSettings->alrmMode << 6) & 0b10000000);
                    wBuf[2] |= ((pAlarmSettings->alrmMode << 5) & 0b10000000);
                    wBuf[3] |= ((pAlarmSettings->alrmMode << 4) & 0b10000000);
                    wBuf[3] |= ((pAlarmSettings->alrmMode << 2) & 0b01000000);

    dumpBuffer( wBuf, DS3234_NUM_ALARM_2_REGS );

                    retVal = spiXferMulti8(handle, cs, pSettings, wBuf, rBuf, 
                                            DS3234_NUM_ALARM_2_REGS);

                }
                else
                {
                    retVal = ERROR_ALARM_NO;
                }
            }
        }
        else
        {
            retVal = ERROR_PARAM_ALARM_SETTINGS_NULL;
        }
    }
    else
    {
        retVal = ERROR_PARAM_SETTINGS_NULL;
    }

    return( retVal );
}

int readAlarm( int handle, int cs, struct _spi_device_settings_ *pSettings, 
                uint8_t wBuf[], uint8_t rBuf[], 
                struct _alarm_settings_ *pAlarmSettings )
{
    int retVal;

    if( pSettings!= NULL )
    {
        if( pAlarmSettings!= NULL )
        {

            if( pAlarmSettings->alrmNo == 1 )
            {
                wBuf[0] = 0x07;          // read from 0x07

                retVal = spiXferMulti8(handle, cs, pSettings, wBuf, rBuf, 
                                        DS3234_NUM_ALARM_1_REGS);

    dumpBuffer( wBuf, DS3234_NUM_ALARM_1_REGS );

                pAlarmSettings->second = bcd2bin( rBuf[1] );
                pAlarmSettings->minute = bcd2bin( rBuf[2] );
                pAlarmSettings->hour   = bcd2bin( rBuf[3] );
                pAlarmSettings->date   = bcd2bin( rBuf[4] );

            }
            else
            {
                if( pAlarmSettings->alrmNo == 2 )
                {
                    wBuf[0] = 0x0B;          // read from 0x0B

                    retVal = spiXferMulti8(handle, cs, pSettings, wBuf, rBuf, 
                                            DS3234_NUM_ALARM_2_REGS);

    dumpBuffer( wBuf, DS3234_NUM_ALARM_2_REGS );

                    pAlarmSettings->second = 0;
                    pAlarmSettings->minute = bcd2bin( rBuf[1] );
                    pAlarmSettings->hour   = bcd2bin( rBuf[2] );
                    pAlarmSettings->date   = bcd2bin( rBuf[3] );

                }
                else
                {
                    retVal = ERROR_ALARM_NO;
                }
            }
        }
        else
        {
            retVal = ERROR_PARAM_ALARM_SETTINGS_NULL;
        }
    }
    else
    {
        retVal = ERROR_PARAM_SETTINGS_NULL;
    }

    return( retVal );
}


int clearAlarm( int handle, int cs, struct _spi_device_settings_ *pSettings, 
                uint8_t wBuf[], uint8_t rBuf[], 
                struct _alarm_settings_ *pAlarmSettings )
{
    int retVal;

    if( pSettings!= NULL )
    {
        if( pAlarmSettings!= NULL )
        {

            if( pAlarmSettings->alrmNo == 1 )
            {
                wBuf[0] = 0x87;          // write to 0x87
                wBuf[1] = 0;
                wBuf[2] = 0;
                wBuf[3] = 0;
                wBuf[4] = 0;

                retVal = spiXferMulti8(handle, cs, pSettings, wBuf, rBuf, 
                                        DS3234_NUM_ALARM_1_REGS);

            }
            else
            {
                if( pAlarmSettings->alrmNo == 2 )
                {
                    wBuf[0] = 0x8B;          // write to 0x8B
                    wBuf[1] = 0;
                    wBuf[2] = 0;
                    wBuf[3] = 0;

                    retVal = spiXferMulti8(handle, cs, pSettings, wBuf, rBuf, 
                                            DS3234_NUM_ALARM_2_REGS);

                }
                else
                {
                    retVal = ERROR_ALARM_NO;
                }
            }
        }
        else
        {
            retVal = ERROR_PARAM_ALARM_SETTINGS_NULL;
        }
    }
    else
    {
        retVal = ERROR_PARAM_SETTINGS_NULL;
    }

    return( retVal );
}

// #define DS3234_ALARM_ONCE_PER_SECOND          0b00001111
// #define DS3234_ALARM_SEC_MATCH                0b00001110
// #define DS3234_ALARM_MIN_SEC_MATCH            0b00001100
// #define DS3234_ALARM_HOUR_MIN_SEC_MATCH       0b00001000
// #define DS3234_ALARM_DATE_HOUR_MIN_SEC_MATCH  0b00000000
// #define DS3234_ALARM_DAY_HOUR_MIN_SEC_MATCH   0b00010000
// 
// #define DS3234_ALARM_ONCE_PER_MINUTE          0b00001110
// #define DS3234_ALARM_MIN_MATCH                0b00001100
// #define DS3234_ALARM_HOUR_MIN_MATCH           0b00001000
// #define DS3234_ALARM_DATE_HOUR_MIN_MATCH      0b00000000
// #define DS3234_ALARM_DAY_HOUR_MIN_MATCH       0b00010000


void printAlarm( struct _alarm_settings_ *pAlarmSettings )
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



int main(int argc, char *argv[])
{
    int retVal;

    int spiHandle;
    int pinSQW, pinMOSI, pinMISO, pinSCK, pinCS0, pinCS1;

    struct timeStruc currTime;
    struct tm *now;
    time_t currSecs;

    int xferLength;

    static const char *pDevice;
    struct _spi_device_settings_ spiSettingsOld;
    struct _spi_device_settings_ spiSettingsNew;

    uint8_t spiReadBuf[DS3234_SPI_R_BUF_SIZE];
    uint8_t spiWriteBuf[DS3234_SPI_W_BUF_SIZE];

    struct _alarm_settings_ alarmSettings;

//    const char *pDevice;
//    int         spiHandle;
//    uint8_t     csPin;
//
//    struct _spi_device_settings_ spiSettingsOld;
//    struct _spi_device_settings_ spiSettingsNew;
//
//    uint8_t spiReadBuf[DS3234_SPI_R_BUF_SIZE];
//    uint8_t spiWriteBuf[DS3234_SPI_W_BUF_SIZE];
//

    pinSQW   = DS324_PIN_SQW;
    pinMOSI  = DS324_PIN_MOSI;
    pinMISO  = DS324_PIN_MISO;
    pinSCK   = DS324_PIN_SCK;
    pinCS0   = DS324_PIN_CS0;
    pinCS1   = DS324_PIN_CS1;

    if(gpioInitialise() < 0)
    {
        return(1);
    }

    gpioSetMode( pinCS0, PI_OUTPUT );
    gpioWrite(   pinCS0, PI_HIGH );

    pDevice = "/dev/spidev0.0";
    spiSettingsNew.mode = DS324_SPI_MODE;
    spiSettingsNew.bits = DS324_SPI_BITS_PER_WORD;
    spiSettingsNew.baud = DS324_SPI_SPEED;
    spiSettingsNew.delay = DS324_SPI_DELAY;

    if( (spiHandle = spiSetup( pDevice, &spiSettingsNew, 
                                         &spiSettingsOld )) >= 0 )
    {

#if 0
        time(&currSecs);
        now = localtime(&currSecs);

        currTime.second  = now->tm_sec;         /* seconds */
        currTime.minute  = now->tm_min;         /* minutes */
        currTime.hour    = now->tm_hour;        /* hours */

        currTime.hour24  =  0;

        currTime.wday    = now->tm_wday + 1;    /* day of the week */
        currTime.date    = now->tm_mday;        /* day of the month */
        currTime.month   = now->tm_mon + 1;     /* month */
        currTime.year    = now->tm_year;        /* year */

        currTime.century =  0;

        memset( spiReadBuf,  '\0', DS3234_SPI_R_BUF_SIZE );
        memset( spiWriteBuf, '\0', DS3234_SPI_W_BUF_SIZE );
        writeDate( spiHandle, pinCS0, &spiSettingsNew, spiWriteBuf, 
                   spiReadBuf, &currTime );
#endif // 0

        memset( spiReadBuf,  '\0', DS3234_SPI_R_BUF_SIZE );
        memset( spiWriteBuf, '\0', DS3234_SPI_W_BUF_SIZE );
        readDate( spiHandle, pinCS0, &spiSettingsNew, spiWriteBuf, 
                  spiReadBuf, &currTime );

        alarmSettings.alrmNo   = 1;
        alarmSettings.alrmMode = 0;
        alarmSettings.enable   = false;
        alarmSettings.second   = 23;
        alarmSettings.minute   = 10;
        alarmSettings.hour     = 22;
        alarmSettings.date     = 17;

        printAlarm( &alarmSettings );

        setAlarm( spiHandle, pinCS0, &spiSettingsNew, spiWriteBuf, 
                  spiReadBuf, &alarmSettings );

        readAllRegs( spiHandle, pinCS0, &spiSettingsNew, spiWriteBuf, 
                  spiReadBuf );

        alarmSettings.alrmNo   = 1;
        alarmSettings.alrmMode = 0;
        alarmSettings.enable   = false;
        alarmSettings.second   = 0;
        alarmSettings.minute   = 0;
        alarmSettings.hour     = 0;
        alarmSettings.date     = 0;

        readAlarm( spiHandle, pinCS0, &spiSettingsNew, spiWriteBuf, 
                  spiReadBuf, &alarmSettings );

        printAlarm( &alarmSettings );

        alarmSettings.alrmNo   = 1;
        alarmSettings.alrmMode = 0;
        alarmSettings.enable   = false;
        alarmSettings.second   = 0;
        alarmSettings.minute   = 0;
        alarmSettings.hour     = 0;
        alarmSettings.date     = 0;

        clearAlarm( spiHandle, pinCS0, &spiSettingsNew, spiWriteBuf, 
                  spiReadBuf, &alarmSettings );

        alarmSettings.alrmNo   = 1;
        alarmSettings.alrmMode = 0;
        alarmSettings.enable   = false;
        alarmSettings.second   = 0;
        alarmSettings.minute   = 0;
        alarmSettings.hour     = 0;
        alarmSettings.date     = 0;

        readAlarm( spiHandle, pinCS0, &spiSettingsNew, spiWriteBuf, 
                  spiReadBuf, &alarmSettings );

        printAlarm( &alarmSettings );

        close( spiHandle );
    }

    gpioTerminate();

}



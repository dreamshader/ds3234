/*
 ***********************************************************************
 *
 *  dsSPI.cpp - low level SPI access
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

#include "dsSPI.h"


int spiCheckParam( int csPin, uint8_t  spiMode, uint8_t  spiBits, uint32_t spiBaud, uint16_t spiDelay )
{
    int retVal = SPI_ERROR_NO_ERROR;

    return( retVal );
}


int spiXferMulti8(int spiHandle, const uint8_t csPin, struct _spiTransferSettings_ *pSettings, uint8_t *pDataOut, uint8_t *pDataIn, int dataLength)
{
    struct spi_ioc_transfer spiData; 
    int retVal;

    if( pSettings != NULL )
    {
        memset( &spiData, '\0', sizeof(spiData) );

        spiData.tx_buf        = (unsigned long) pDataOut;
        spiData.rx_buf        = (unsigned long) pDataIn;
        spiData.len           = dataLength;

        spiData.delay_usecs   = pSettings->spiDelay;
        spiData.speed_hz      = pSettings->spiBaud;
        spiData.bits_per_word = pSettings->spiBits;
        spiData.cs_change     = 0;

        gpioWrite(csPin, PI_LOW);
        if( (retVal = ioctl(spiHandle, SPI_IOC_MESSAGE(1), &spiData)) < 0 )
        {
            perror("SPI Message failed.");
        }
        gpioWrite(csPin, PI_HIGH);

    }
    else
    {
        retVal = SPI_ERROR_SETTINGS_NULL;
    }
    return retVal;
}


int spiXferSingle8(int spiHandle, const uint8_t csPin, struct _spiTransferSettings_ *pSettings, uint8_t *pDataOut, uint8_t *pDataIn, int dataLength)
{
    struct spi_ioc_transfer spiData[dataLength]; 
    int retVal;

    if( pSettings != NULL )
    {
        gpioWrite(csPin, PI_LOW);
        for(int i = 0; i < dataLength; i++)
        {
            memset( &spiData[i], '\0', sizeof(spiData[i]) );
            spiData[i].tx_buf        = (unsigned long)(pDataOut + i);
            spiData[i].rx_buf        = (unsigned long)(pDataIn + i);
            spiData[i].len           = 1;

            spiData[i].delay_usecs   = pSettings->spiDelay;
            spiData[i].speed_hz      = pSettings->spiBaud;
            spiData[i].bits_per_word = pSettings->spiBits;

            spiData[i].cs_change     = 0;
        }

        if( (retVal = ioctl(spiHandle, SPI_IOC_MESSAGE(dataLength), spiData)) < 0 )
        {
            perror("SPI Message failed.");
        }
        gpioWrite(csPin, PI_HIGH);
    }
    else
    {
        retVal = SPI_ERROR_SETTINGS_NULL;
    }

    return retVal;
}


int spiSaveSettings( int spiHandle, uint8_t  *pMode, uint8_t  *pBits, uint32_t *pBaud, uint16_t *pDelay )
{
    int retVal;

    if( pMode != NULL && pBits != NULL && pBaud != NULL && pDelay != NULL )
    {
        if( (retVal = ioctl(spiHandle, SPI_IOC_RD_MODE, pMode )) >= 0 )
        {
            if( (retVal = ioctl(spiHandle, SPI_IOC_RD_BITS_PER_WORD, 
                                                        pBits )) >= 0 )
            {
                if( (retVal = ioctl(spiHandle, SPI_IOC_RD_MAX_SPEED_HZ, 
                                                        pBaud )) < 0 )
                {
                    perror("read spi settings >speed<");
                }
                else
                {
                    retVal = SPI_ERROR_NO_ERROR;
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
        retVal = SPI_ERROR_SETTINGS_NULL;
    }
   
    return( retVal );
}


int spiWriteNewSettings( int spiHandle, uint8_t  *pMode, uint8_t  *pBits, uint32_t *pBaud, uint16_t *pDelay )
{
    int retVal;

    if( pMode != NULL && pBits != NULL && pBaud != NULL && pDelay != NULL )
    {
        if( (retVal = ioctl(spiHandle, SPI_IOC_WR_MODE, pMode )) >= 0 )
        {
            if( (retVal = ioctl(spiHandle, SPI_IOC_WR_BITS_PER_WORD, 
                                   pBits )) >= 0 )
            {
                if( (retVal = ioctl(spiHandle, SPI_IOC_WR_MAX_SPEED_HZ, 
                                       pBaud )) < 0 )
                {
                    perror("read spi settings >mode<");
                }
                else
                {
                    retVal = SPI_ERROR_NO_ERROR;
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
        retVal = SPI_ERROR_SETTINGS_NULL;
    }
   
    return( retVal );
}




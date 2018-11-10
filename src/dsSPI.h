/*
 ***********************************************************************
 *
 *  dsSPI.h - declarations for low level SPI access
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

#ifndef _DSSPI_H_
#define _DSSPI_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/spi/spidev.h>
#include <sys/stat.h>
#include <time.h>

#include <pigpio.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
 

struct _spiTransferSettings_ {
    uint8_t  spiMode;
    uint8_t  spiBits;
    uint32_t spiBaud;
    uint16_t spiDelay;
};

 
#define SPI_ERROR_NO_ERROR                  0
#define SPI_ERROR_BASE                    -49
#define SPI_ERROR_INVALID_MODE              ( SPI_ERROR_BASE -  1 )
#define SPI_ERROR_INVALID_BPW               ( SPI_ERROR_BASE -  2 )          
#define SPI_ERROR_INVALID_BAUD              ( SPI_ERROR_BASE -  3 )    
#define SPI_ERROR_INVALID_DELAY             ( SPI_ERROR_BASE -  4 )
#define SPI_ERROR_SETTINGS_NULL             ( SPI_ERROR_BASE -  5 )


#define MIN_SPI_BUS_NUMBER                  0
#define MAX_SPI_BUS_NUMBER                  1

#define DEVICES_PER_SPI_BUS                 2
#define MAX_SPI_DEVICENAME_LENGTH          20
// "/dev/spidev0.0"

#define DEFAULT_SPI_BUS                     0
#define DEFAULT_SPI_DEVICE                  0
#define DEFAULT_SPI_MODE        (SPI_MODE_1 | SPI_NO_CS)
#define DEFAULT_SPI_BITS_PER_WORD           8
#define DEFAULT_SPI_SPEED              500000 
#define DEFAULT_SPI_DELAY                   0

#define DEFAULT_SPI_BUF_SIZE               33


struct _spiDevice_ {
    uint8_t pinMOSI;
    uint8_t pinMISO;
    uint8_t pinSCK;

    uint8_t *pDeviceName;
    uint8_t bus;
    uint8_t device;
    uint8_t pinCS;

    uint8_t  spiModeSave;
    uint8_t  spiBitsSave;
    uint32_t spiBaudSave;
    uint16_t spiDelaySave;

    uint8_t  spiModeNew;
    uint8_t  spiBitsNew;
    uint32_t spiBaudNew;
    uint16_t spiDelayNew;

    uint8_t  *pTxBuf;
    uint8_t  *pRxBuf;
    uint32_t  bufSize;

    int spiHandle;
};

int spiCheckParam( int csPin, uint8_t  spiMode, uint8_t  spiBits, uint32_t spiBaud, uint16_t spiDelay );

int spiXferMulti8(int spiHandle, const uint8_t csPin, struct _spiTransferSettings_ *pSettings, uint8_t *pDataOut, uint8_t *pDataIn, int dataLength);

int spiXferSingle8(int spiHandle, const uint8_t csPin, struct _spiTransferSettings_ *pSettings, uint8_t *pDataOut, uint8_t *pDataIn, int dataLength);

int spiSaveSettings( int spiHandle, uint8_t  *pMode, uint8_t  *pBits, uint32_t *pBaud, uint16_t *pDelay );

int spiWriteNewSettings( int spiHandle, uint8_t  *pMode, uint8_t  *pBits, uint32_t *pBaud, uint16_t *pDelay );

int spiSetup( const char *pDevice, 
              struct _spiTransferSettings_ *pNewSettings, 
              struct _spiTransferSettings_ *pOldSettings );







#ifdef __cplusplus
}
#endif

#endif // _DSSPI_H_




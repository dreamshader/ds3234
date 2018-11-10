/*
 ***********************************************************************
 *
 *  dsConnection.h - defintions and declarations for connection class
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


#ifndef _DSCONNECTION_H_
#define _DSCONNECTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/spi/spidev.h>
#include <sys/stat.h>
#include <time.h>

#include "dsSPI.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#define DS_CONNECTION_MODE_READ          0x01
#define DS_CONNECTION_MODE_WRITE         0x02

#define DS_CONNECTION_TYPE_NONE          0b00000000
#define DS_CONNECTION_TYPE_SPI           0b00000001

#define DS_CONNECTION_STATE_NONE         0b00000000
#define DS_CONNECTION_STATE_INITIALIZED  0b00000001
#define DS_CONNECTION_STATE_CONNECTED    0b00000010
#define DS_CONNECTION_STATE_BUSY         0b00000100

#define DS_CONNECTION_ERR_BASE           0
#define DS_CONNECTION_ERR_NOERROR        0
#define DS_CONNECTION_ERR_NOT_CONNECTED  ( DS_CONNECTION_ERR_BASE -  1 )
#define DS_CONNECTION_ERR_CONNECTED      ( DS_CONNECTION_ERR_BASE -  2 )
#define DS_CONNECTION_ERR_BUSNO          ( DS_CONNECTION_ERR_BASE -  3 )
#define DS_CONNECTION_ERR_DEVICE         ( DS_CONNECTION_ERR_BASE -  4 )
#define DS_CONNECTION_ERR_MALLOC         ( DS_CONNECTION_ERR_BASE -  5 )

class dsConnection {

    protected:
        uint8_t connectionType;
        uint8_t connectionState;
        struct _spiDevice_ _spi;

    public:
        int lastErrno;

    protected:
        int allocBuffers( uint32_t maxDataSize );
        int checkSPIConnectionParam( int bus, int device, int csPin, uint8_t  spiMode, uint8_t  spiBits, uint32_t spiBaud, uint16_t spiDelay );

    public:
        dsConnection();
        ~dsConnection();
        int connectSPI( int bus, int device, int csPin, uint8_t  spiMode, 
                        uint8_t  spiBits, uint32_t spiBaud, uint16_t spiDelay,
                        uint32_t maxDataSize );

        int connect( void );
        int disconnect( void );
        int xfer( int mode, void* pDataOut, void* pDataIn, uint32_t dataSize );

};


#ifdef __cplusplus
}
#endif

#endif // _DSCONNECTION_H_





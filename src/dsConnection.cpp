/*
 ***********************************************************************
 *
 *  dsConnection.cpp - class file for connection class
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


#include "dsConnection.h"
#include "dsUtil.h"

dsConnection::dsConnection()
{
    connectionType = DS_CONNECTION_TYPE_NONE;
    connectionState = DS_CONNECTION_STATE_NONE;
}

dsConnection::~dsConnection()
{
}

int dsConnection::connect()
{
    return(0);
}


int dsConnection::checkSPIConnectionParam( int bus, int device, int csPin, uint8_t  spiMode, uint8_t  spiBits, uint32_t spiBaud, uint16_t spiDelay )
{

    int retVal = 0;

    if( connectionState & DS_CONNECTION_STATE_CONNECTED )
    {
        retVal = DS_CONNECTION_ERR_CONNECTED;
    }
    else
    { 
        if( bus < MIN_SPI_BUS_NUMBER || bus > MAX_SPI_BUS_NUMBER )
        {
            retVal = DS_CONNECTION_ERR_BUSNO;
        }
        else
        {
            if( device < 0 || device >= DEVICES_PER_SPI_BUS )
            {
                retVal = DS_CONNECTION_ERR_DEVICE;
            }
            else
            {
                retVal = spiCheckParam( csPin, spiMode, spiBits, 
                                 spiBaud, spiDelay );
            }
        }
    }

    return( retVal );
}



int dsConnection::allocBuffers( uint32_t maxDataSize )
{
    int retVal;

    if( (_spi.bufSize = maxDataSize) == 0 )
    {
        _spi.bufSize = DEFAULT_SPI_BUF_SIZE;
    }

    if( (_spi.pTxBuf = (uint8_t*) malloc( _spi.bufSize )) != NULL )
    {
        if( (_spi.pRxBuf = (uint8_t*) malloc( _spi.bufSize )) != NULL )
        {
            if( (_spi.pDeviceName = 
                    (uint8_t*) malloc( MAX_SPI_DEVICENAME_LENGTH )) != NULL )
            {
                retVal = DS_CONNECTION_ERR_NOERROR;
            }
            else
            {
                retVal = DS_CONNECTION_ERR_MALLOC;
            }
        }
        else
        {
            retVal = DS_CONNECTION_ERR_MALLOC;
        }
    }
    else
    {
        retVal = DS_CONNECTION_ERR_MALLOC;
    }

    return( retVal );
}




int dsConnection::connectSPI( int bus, int device, int csPin, uint8_t  spiMode, uint8_t  spiBits, uint32_t spiBaud, uint16_t spiDelay, uint32_t maxDataSize )
{

    int retVal;

    if( (retVal = checkSPIConnectionParam( bus, device, csPin, spiMode, 
                        spiBits, spiBaud, spiDelay )) == SPI_ERROR_NO_ERROR )
    {
        if( allocBuffers( maxDataSize ) == SPI_ERROR_NO_ERROR )
        {
            sprintf((char*) _spi.pDeviceName, "/dev/spidev%d.%d", bus, device);


            _spi.spiModeNew  = spiMode;
            _spi.spiBitsNew  = spiBits;
            _spi.spiBaudNew  = spiBaud;
            _spi.spiDelayNew = spiDelay;

            _spi.bus         = bus;
            _spi.device      = device;
            _spi.pinCS       = csPin;

            if( (_spi.spiHandle = open((char*) _spi.pDeviceName, O_RDWR)) >= 0)
            {
                if( (retVal = spiSaveSettings( _spi.spiHandle, 
                                               &_spi.spiModeSave, 
                                               &_spi.spiBitsSave, 
                                               &_spi.spiBaudSave, &
                                               _spi.spiDelaySave )) >= 0 )
                {

                    if( (retVal = spiWriteNewSettings( _spi.spiHandle, 
                                                     &_spi.spiModeNew, 
                                                     &_spi.spiBitsNew, 
                                                     &_spi.spiBaudNew, 
                                                     &_spi.spiDelayNew )) >= 0 )
                    {
                        connectionType = DS_CONNECTION_TYPE_SPI;
                        connectionState = DS_CONNECTION_STATE_CONNECTED;
                        connectionState = DS_CONNECTION_STATE_INITIALIZED;
                        retVal = DS_CONNECTION_ERR_NOERROR;
                    }
                }
            }
#ifdef _DEBUG_
fprintf(stdout, "Open device [%s] returns %d as handle\n", _spi.pDeviceName, _spi.spiHandle );
#endif // _DEBUG_
        }
    }

    return( retVal );
}


int dsConnection::disconnect( void )
{
    int retVal = 0;

    return( retVal );
}

int dsConnection::xfer( int mode, void* pDataOut, void* pDataIn, uint32_t dataSize )
{
    int retVal = 0;
    struct _spiTransferSettings_ transfer;

    transfer.spiMode = _spi.spiModeNew;
    transfer.spiBits = _spi.spiBitsNew;
    transfer.spiBaud = _spi.spiBaudNew;
    transfer.spiDelay = _spi.spiDelayNew;

#ifdef _DEBUG_
fprintf(stdout, "DataOut before XferMulti8:\n");

    dumpBuffer( (uint8_t*) pDataOut, dataSize );
#endif // _DEBUG_

    retVal = spiXferMulti8(_spi.spiHandle, _spi.pinCS, &transfer, 
                  (uint8_t*) pDataOut, (uint8_t*) pDataIn, (int) dataSize);

#ifdef _DEBUG_
fprintf(stdout, "DataIn after XferMulti8:\n");

    dumpBuffer( (uint8_t*) pDataIn, retVal );
#endif // _DEBUG_

    return( retVal );
}


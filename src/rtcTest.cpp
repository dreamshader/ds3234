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

    dsDS3234 clock1;
    struct _ds3234_timestruct_ now;

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
                retVal = clock1.readDate( &now );

                printf("readDate: retVal = %d\n", retVal );
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




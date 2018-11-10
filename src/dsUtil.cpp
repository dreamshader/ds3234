/*
 ***********************************************************************
 *
 *  dsUtil.cpp - useful helpers ...
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


void dumpBuffer( uint8_t buffer[], int len )
{
    if( buffer != NULL )
    {
        for( int i = 0; i < len; i++ )
        {
            printf("buffer[%02X] -> %3d -> 0x%02X -> %s\n", i, buffer[i], 
                buffer[i], bits2str( buffer[i], 8, 0 ) );
        }
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




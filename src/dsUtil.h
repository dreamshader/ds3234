/*
 ***********************************************************************
 *
 *  dsUtil.h - header file for useful helpers ...
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

#ifndef _DSUTIL_H_
#define _DSUTIL_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
 

void printBits( uint32_t value, int bits, uint8_t flags );
char *bits2str( uint32_t value, int bits, uint8_t flags );
void dumpBuffer( uint8_t buffer[], int len );
uint8_t bin2bcd(uint8_t val) ;
uint8_t bcd2bin(uint8_t val) ;


#ifdef __cplusplus
}
#endif

#endif // _DSUTIL_H_





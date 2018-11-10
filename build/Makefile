SHELL := /bin/bash
#################################################################################
# Makefile for DS3234 class library and example
#
#################################################################################
# This Makefile is part of my DS3234 project
#// // // // #	git://github.com/dreamshader/DS3234.git
#
#  Copyright (C) 2018 Dreamshader (aka Dirk Schanz)
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#################################################################################
CXX = g++
CC  = gcc
AR  = ar rc
#
CCDEBUG  = -g
CXXDEBUG = -g
#
CXXFLAGS = -Wall
#
# CXXEXTRAFLAGS = -DLINUX
# CXXEXTRAFLAGS = -DLINUX -DDEBUG -DDEBUG_STATUS_BITS
# CXXEXTRAFLAGS = -DRASPBERRY
# CXXEXTRAFLAGS = -DRASPBERRY -DDEBUG
#
SOURCEDIR = ../src
#
SOLIBNAME = libdsRTC.so
STATLIBNAME = libdsRTC.a
#
LIB_SRC = $(SOURCEDIR)/dsConnection.cpp  $(SOURCEDIR)/dsDS3234.cpp  $(SOURCEDIR)/dsSPI.cpp  $(SOURCEDIR)/dsUtil.cpp  

SRC_INC = $(SOURCEDIR)/dsConnection.h  $(SOURCEDIR)/dsDS3234.h  $(SOURCEDIR)/dsSPI.h  $(SOURCEDIR)/dsUtil.h  

LIB_OBJ = dsConnection.o  dsDS3234.o  dsSPI.o  dsUtil.o  

EXAMPLE_SRC = $(SOURCEDIR)/rtcTest.cpp

EXAMPLE_NAME = rtcTest

BUILD_FLAGS = -I. -L ../build
#
PIGPIO = -lpigpio
#
#
EXTRALIBS = -lrt -lpthread ${PIGPIO}
CXXEXTRAFLAGS = -DLINUX -DDEBUG -DDEBUG_STATUS_BITS -DRASPBERRY
#

#
#all: lib-static lib-shared example
all: $(STATLIBNAME) $(SOLIBNAME) $(EXAMPLE_NAME)

old:
	$(CXX) -o ds3234 $(CXXDEBUG) $(CXXEXTRAFLAGS) $(SOURCEDIR)/ds3234.cpp $(BUILD_FLAGS) ${EXTRALIBS}


$(STATLIBNAME): $(LIB_SRC) $(SRC_INC)
	$(CXX) $(CXXFLAGS) $(CXXEXTRAFLAGS) -c $(LIB_SRC)
	 $(AR) $(STATLIBNAME)  $(LIB_OBJ)

$(SOLIBNAME): $(LIB_SRC) $(SRC_INC)
	$(CXX) $(CXXFLAGS) $(CXXEXTRAFLAGS) $(CXXDEBUG) $(CXXLIBSOFLAGS) -c $(LIB_SRC)
	$(CXX) -shared  -Wl,-soname,$(SOLIBNAME) -o $(SOLIBNAME) $(LIB_OBJ)


$(EXAMPLE_NAME): $(EXAMPLE_SRC) $(LIB_SRC) $(SRC_INC)
	$(CXX) -o $(EXAMPLE_NAME) $(CXXDEBUG) $(CXXEXTRAFLAGS) $(EXAMPLE_SRC) $(LIB_SRC) $(BUILD_FLAGS) ${EXTRALIBS}

clean:
	$rm *.o *.so *.a






# LIBPROJ = libmip$(MACHINE).a


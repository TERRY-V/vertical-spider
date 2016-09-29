#****************************************************************************
#
# Copyright (C) 2010-2014 Terry Niu (Beijing, China)
#
# Author:	TERRY-V
# Email:	cnbj8607@163.com
# Support:	http://blog.sina.com.cn/terrynotes
#
# This is a GNU make (gmake) makefile
#
#****************************************************************************

CC		:= gcc
CXX		:= g++
LD		:= g++
AR		:= ar rc
RANLIB		:= ranlib

VPATH		:= ./:../include/common:../src:../lib

DEBUG_CFLAGS	:= -Wall -Wno-unused-function -g -DDEBUG -D__multi_thread -D__DENOISE_MODE -D__URL_UNIQ
RELEASE_CFLAGS	:= -Wall -Wno-strict-aliasing -Wno-unused-function -O3 -D__multi_thread -D__DENOISE_MODE -D__URL_UNIQ

DEBUG_CXXFLAGS	:= ${DEBUG_CFLAGS} -fexceptions -fPIC
RELEASE_CXXFLAGS:= ${RELEASE_CFLAGS} -fexceptions -fPIC

DEBUG_LDFLAGS	:= -g -lcurl -lpcre -lpthread
RELEASE_LDFLAGS	:= -lcurl -lpcre -lpthread

SHARE_FLAGS 	:= -shared

ifeq (YES, ${DEBUG})
	CFLAGS	:= ${DEBUG_CFLAGS}
	CXXFLAGS:= ${DEBUG_CXXFLAGS}
	LDFLAGS	:= ${DEBUG_LDFLAGS}
else
	CFLAGS	:= ${RELEASE_CFLAGS}
	CXXFLAGS:= ${RELEASE_CXXFLAGS}
	LDFLAGS	:= ${RELEASE_LDFLAGS}
endif

#****************************************************************************
# Include paths
#****************************************************************************

INCS		:= -I../include/common
INCS		+= -I../src

#****************************************************************************
# Include libs
#****************************************************************************

LIBS		:= 

#****************************************************************************
# Targets of the build
#****************************************************************************

OUTPUT_1	:= ../bin/spider-server
OUTPUT_2	:= ../bin/testRegex

all: ${OUTPUT_1} ${OUTPUT_2}

#****************************************************************************
# Source files
#****************************************************************************

SRCS		:= qallocator.cc
SRCS		+= qbitmap.cc
SRCS		+= qdir.cc
SRCS		+= qfile.cc
SRCS		+= qlogger.cc
SRCS		+= qnetworkaccessmanager.cc
SRCS		+= qservice.cc
SRCS		+= templatemanager.cc
SRCS		+= qremotemonitor.cc
SRCS		+= qtcpsocket.cc
SRCS		+= tinyxml2.cpp
SRCS		+= htmlparser.cc
SRCS		+= urlprocessor.cc
SRCS		+= spiderserver.cc
SRCS_1		:= main.cc
SRCS_2		:= testRegex.cc

OBJS_1		:= $(addsuffix .o, $(basename ${SRCS} ${SRCS_1}))
OBJS_2		:= $(addsuffix .o, $(basename ${SRCS} ${SRCS_2}))

#****************************************************************************
# Output
#****************************************************************************

${OUTPUT_1}: $(OBJS_1)
	@echo Linking ...
	${LD} -o $@ ${OBJS_1} ${LIBS} ${LDFLAGS}
	@echo ^_^ Congratulations!

${OUTPUT_2}: $(OBJS_2)
	@echo Linking ...
	${LD} -o $@ ${OBJS_2} ${LIBS} ${LDFLAGS}
	@echo ^_^ Congratulations!

#****************************************************************************
# Common rules
#****************************************************************************

# Rules for compiling source files to object files
%.o: %.c
	@echo Compiling $< ...
	${CC} ${CFLAGS} ${INCS} -o $@ -c $<

%.o: %.cc
	@echo Compiling $< ...
	${CXX} ${CXXFLAGS} ${INCS} -o $@ -c $<

%.o: %.cpp
	@echo Compiling $< ...
	${CXX} ${CXXFLAGS} ${INCS} -o $@ -c $<

clean:
	-rm -f core *.o ${OUTPUT}


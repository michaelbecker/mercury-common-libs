#
#	What we are building.
#
TARGET = libTaGpio.so

default: all

all: $(TARGET) 


#
#	Source Files
#
SRC =	ParseConfig.cpp \
		TaGpio.cpp	\
		Gpio.cpp	\
		GpioInput.cpp \
		GpioOutput.cpp \



#
#	Libraries we depend on.
#
LIBS= -lLogLib \
	  -lTimers	\
	  ${CPPLIBS} \



#
#	Relative directory to where "rules.make" lives. 
#
COMMON_DIR=..

#
#	Generic makefile 
#
include ${COMMON_DIR}/rules.make


#
#	Our target rule
#
$(TARGET):	$(SRC)
		${CXX} ${INCLUDE_DIRS} ${CPPFLAGS} ${CPP_LIB_FLAGS} -c $? 
		${CXX} ${LDFLAGS} ${LD_LIB_FLAGS} -o $@ ${OBJ} ${LIBS} 


install: script_install

script_install:
	ls ${INSTALL_DIR} > /dev/null 2>&1 || mkdir -p ${INSTALL_DIR}
	cp gpio-bd.conf ${INSTALL_DIR}


uninstall: script_uninstall

script_uninstall:
	- rm ${INSTALL_DIR}/gpio-bd.conf




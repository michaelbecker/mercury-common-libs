#
#	What we are building.
#
TARGET = GpioTest

default: all

all: $(TARGET) 


#
#	Source Files
#
SRC =	main.cpp



#
#	Libraries we depend on.
#
LIBS= -lTaGpio \
	  	-lTimers \
		-lLogLib \
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
		${CXX} ${INCLUDE_DIRS} ${CPPFLAGS} ${CPP_EXE_FLAGS} -c $? 
		${CXX} ${LDFLAGS} ${LD_EXE_FLAGS} -o $@ ${OBJ} ${LIBS} 




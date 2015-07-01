#
#	What we are building.
#
TARGET = libSignalCalculator.so

default: all

all: $(TARGET) 


#
#	Source Files
#
SRC =	Calculator.cpp \
		CalculatorDbg.cpp \
		DataReadyCallback.cpp \
		DataSyncedCallback.cpp \
		RunCalculationsAsync.cpp \



#
#	Libraries we depend on.
#
LIBS= -lLogLib \
	  -lWorkQueue \
	  ${CPPLIBS}


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




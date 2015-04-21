#
#	What we are building.
#
TARGET = DataStoreUnitTest

default: all

all: $(TARGET) 


#
#	Source Files
#
SRC =	\
	DataStoreTest.cpp



#
#	Libraries we depend on.
#
LIBS= -lDataStore \
	  -lSyslog \
	  -lTimers \
	  -lCommonInstrumentInterface \
	  -lTaCanProtocol \
	  -lWorkQueue \
	  -lLogLib \
	  ${CPPLIBS} \



INCLUDE_DIRS+=-I../DataStore


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




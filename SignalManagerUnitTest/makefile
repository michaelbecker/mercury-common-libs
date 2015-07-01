#
#	What we are building.
#
TARGET = SignalManagerUnitTest

default: all

all: $(TARGET) 


#
#	Source Files
#
SRC =	\
	SignalManagerUnitTest.cpp \
	UnitTestRaw.cpp \
	UnitTestBackward.cpp \
	UnitTestRawRealTime.cpp \
	UnitTestEarlyReset.cpp \
	UnitTestSelectDataOut1.cpp \
	UnitTestForwardBackwardAvg.cpp \
 	UnitTestSelectDataOut2.cpp \
	UnitTestForwardBackwardIndependent.cpp \
	UnitTestForward.cpp \
	UnitTestSort1.cpp \
	UnitTestMdscIndicies.cpp \
	UnitTestSort2.cpp



#
#	Libraries we depend on.
#
LIBS= -lSignalCalculator \
	  -lConsoleMemoryDump \
	  -lWorkQueue \
	  -lLogLib \
	  ${CPPLIBS} \


INCLUDE_DIRS+=-I../SignalManagerAlgorithms


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




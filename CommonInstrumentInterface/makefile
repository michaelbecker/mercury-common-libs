#
#	What we are building.
#
TARGET = libCommonInstrumentInterface.so

default: all

all: $(TARGET) 


#
#	Source Files
#
SRC =	CiiAlloc.cpp \
		CiiLogging.cpp \
		CiiStatus.cpp \
		CommFaultInjection.cpp \
		CommonInstrumentInterface.cpp \
		TriosSocketAccept.cpp \
		TriosSocketManager.cpp \



#
#	Libraries we depend on.
#
LIBS= -lLogLib \
	  -lWorkQueue \
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




#
#	What we are building.
#
TARGET = libDataStore.a

default: all

all: $(TARGET) 


#
#	Source Files
#
SRC =	DataStore.cpp \
		RunTimeErrorsStub.cpp \


#
#	Libraries we depend on.
#
LIBS= -lLogLib \
	  -lSyslog \
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
		${CXX} ${INCLUDE_DIRS} ${CPPFLAGS} ${CPP_STATIC_FLAGS} -c $? 
		${AR} ${AR_FLAGS} $@ ${OBJ}




#
#	What we are building.
#
TARGET = TimersTest

default: all

all: $(TARGET) 


#
#	Source Files
#

SRC =	main.cpp


#
#	Libraries we depend on.
#
LIBS= -lTimers \
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
		${CXX} ${INCLUDE_DIRS} ${CPPFLAGS} -Wno-int-to-pointer-cast ${CPP_EXE_FLAGS} -c $? 
		${CXX} ${LDFLAGS} ${LD_EXE_FLAGS} -o $@ ${OBJ} ${LIBS} 




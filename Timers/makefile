#
#	What we are building.
#
TARGET = libTimers.so

default: all

all: $(TARGET) 


#
#	List of our sources
#
SRC =	Timers.cpp


#
#	Libraries we depend on.
#
LIBS= ${CPPLIBS} \



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



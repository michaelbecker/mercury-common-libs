#
#	What we are building.
#
TARGET = libSegmentExecutionEngine.so

default: all

all: $(TARGET) 


#
#	Source Files
#
SRC =	ControlCommands.cpp  \
		FindSegments.cpp  \
		ParseSegmentList.cpp  \
		RegisterCommands.cpp  \
		RepeatSegment.cpp  \
		SegmentExecutionEngine.cpp \
		LogVersion.cpp



#
#	Libraries we depend on.
#
LIBS= -lLogLib \
	  -lMessageQueue \
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




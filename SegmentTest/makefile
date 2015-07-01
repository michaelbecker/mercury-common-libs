#
#	What we are building.
#
TARGET = SegmentTest

default: all

all: $(TARGET) 


#
#	Source Files
#
SRC = 	SegmentTest.cpp \
	  	UnitTestAbort2FalseTrue.cpp \
	  	UnitTestSimpleAbortFalse.cpp \
		StaticCommands.cpp \
		UnitTestAbort2TrueFalse.cpp \
		UnitTestIsoNestedRepeats.cpp \
 		UnitTestSimpleAbortTrue.cpp \
		TestSegmentAbort2.cpp \
		UnitTestAbortAtEndFalse.cpp \
   		UnitTestIsoRepeatUntil.cpp \
		UnitTestSkipSegment.cpp \
		TestSegmentAbort.cpp \
		UnitTestAbortAtEndTrue.cpp \
  		UnitTestModifyAddDeleteRepeat.cpp \
	  	UnitTestStartNextSegmentListAtEnd.cpp \
  		UnitTestAbort.cpp \
		UnitTestModifyComplex.cpp \
		UnitTestStop.cpp \
		TestSegmentIso.cpp \
		UnitTestAbortRepeatFalse.cpp \
		UnitTestModifyDeleteCurSegment.cpp \
	   	UnitTestTerminate.cpp \
		TestSegmentRamp.cpp \
		UnitTestAbortRepeatTrue.cpp \
   		UnitTestModifyRepeat.cpp \
		TestSegmentRepeatUntil.cpp \
	 	UnitTestInfiniteRepeat.cpp \
  		UnitTestModifySimple.cpp \
		UnitTestAbort2FalseFalse.cpp \
	   	UnitTestInfiniteRepeatUntil.cpp \
	   	UnitTestRampIsoRepeat.cpp \
		TestSegmentIsoClass.cpp \
		UnitTestIso2ClassNestedRepeats.cpp  \


#
#	Libraries we depend on.
#
LIBS= -lSegmentHelpers \
	  -lSegmentExecutionEngine \
	  -lMessageQueue \
	  -lSegmentHelpers \
	  -lLogLib \
	  -lWorkQueue \
	  -lTimers \
	  ${CPPLIBS} \



INCLUDE_DIRS+=-I../SegmentHelpers


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




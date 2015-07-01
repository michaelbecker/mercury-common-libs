#
#   You may need to adjust these if you did not check out to 
#   the default relative locations.
#
#TA_MERCURY_INCLUDE = ${COMMON_DIR}/../../../include
#TA_MERCURY_COMMON_INCLUDE = ${COMMON_DIR}/../../../common/include


VPATH +=    ${COMMON_DIR}/src \



INCLUDE_DIRS +=	-I. \
				-I../inc \
                -I${COMMON_DIR}/src \
				-I${TA_MERCURY_INCLUDE} \
				-I${TA_MERCURY_COMMON_INCLUDE} \
                -I${TA_THERMOCOUPLE_DIR} \



LIB_DIRS += -L${COMMON_DIR}/LogLib \
			-L${COMMON_DIR}/Timers \
			-L${COMMON_DIR}/Gpio \
			-L${COMMON_DIR}/ConsoleMemoryDump \
			-L${COMMON_DIR}/TaCanProtocol \
			-L${COMMON_DIR}/WorkQueue \
			-L${COMMON_DIR}/MessageQueue \
			-L${COMMON_DIR}/CommonInstrumentInterface \
            -L${COMMON_DIR}/SystemEvents \
            -L${COMMON_DIR}/DataStore \
            -L${COMMON_DIR}/SegmentExecutionEngine \
            -L${COMMON_DIR}/SegmentHelpers \
            -L${COMMON_DIR}/SignalCalculator \



CXX				= ${CROSS_COMPILE}gcc
AR              = ${CROSS_COMPILE}ar

OPTIMIZE_FLAGS 	+= -O2 -fno-strict-aliasing
DEBUG_FLAGS		+= -g -fno-omit-frame-pointer
ERROR_FLAGS 	+= -Werror -Wall -Wno-multichar -Wno-write-strings 

CFLAGS		    += ${OPTIMIZE_FLAGS} ${DEBUG_FLAGS} ${ERROR_FLAGS} -pthread 
CPPFLAGS		+= ${OPTIMIZE_FLAGS} ${DEBUG_FLAGS} ${ERROR_FLAGS} -pthread 
AR_FLAGS		+= rcs

CPP_LIB_FLAGS	+= -fpic -fvisibility=hidden
CPP_STATIC_FLAGS += -fpic
CPP_EXE_FLAGS	+= 	

LD_LIB_FLAGS	+= -shared
LD_EXE_FLAGS	+= 
LDFLAGS			+= ${LIB_DIRS} -pthread -Wl,-rpath=.,-rpath=../,-rpath=../bin 


CPPLIBS         += -lstdc++ -lrt -lm
CLIBS           += -lc -lrt



#
#	Need a list of object files for the link phase.
#
OBJ = ${SRC:.cpp=.o}


#
#	Generic Rule(s) - there is no standard rule...
#
#	Executable
#$(TARGET):	$(SRC)
#		${CXX} ${INCLUDE_DIRS} ${CPPFLAGS} ${CPP_EXE_FLAGS} -c $? 
#		${CXX} ${LDFLAGS} ${LD_EXE_FLAGS} -o $@ ${OBJ} ${LIBS} 
#
#	Library
#$(TARGET):	$(SRC)
#		${CXX} ${INCLUDE_DIRS} ${CPPFLAGS} ${CPP_LIB_FLAGS} -c $? 
#		${CXX} ${LDFLAGS} ${LD_LIB_FLAGS} -o $@ ${OBJ} ${LIBS} 
#



#
#   Default Install Dir - for common testing
#
ifndef INSTALL_DIR
INSTALL_DIR=${COMMON_DIR}/install
endif



#
#	Utility targets
#

.PHONY: install
install:all
	ls ${INSTALL_DIR} > /dev/null 2>&1 || mkdir -p ${INSTALL_DIR}
	cp ${TARGET} ${INSTALL_DIR}



.PHONY: uninstall
uninstall:
	- rm ${INSTALL_DIR}/${TARGET} 



.PHONY : clean
clean:
	-rm ${OBJ}
	-rm *~
	-rm ${TARGET}


.PHONY : distclean
distclean: clean uninstall 

.PHONY: inc_version
inc_version:
	- inc_build_number.sh 



# vim: set syntax=make:

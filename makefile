###############################################################################
#
#	Makefile
#
###############################################################################


#
#	Subdirectories needed for the project.
#
NODE_DIRS =	\
	LogLib	\
	LogLibTest \
	Timers	\
	TimersTest \
	Gpio \
	GpioTest \
	TaCanProtocol \
	ConsoleMemoryDump \
	WorkQueue \
	WorkQueueTest \
	MessageQueue \
	MessageQueueTest \
	CommonInstrumentInterface \
	SystemEvents \
	SystemEventsTest \
	DataStore \
	DataStoreTest \
	SegmentExecutionEngine \
	SegmentHelpers \
	SegmentTest \
	SignalCalculator \
	SignalManagerUnitTest \



#
#	Where the final images are sent for this project.
#
ifndef INSTALL_DIR
INSTALL_DIR=./install
endif

#
#	Where all of the generic project build rules live
#
include project.make


install: install_script_dir

install_script_dir:
	cd scripts ; ./install.sh ; cd ..


uninstall: uninstall_script_dir

uninstall_script_dir:
	cd scripts ; ./uninstall.sh ; cd ..




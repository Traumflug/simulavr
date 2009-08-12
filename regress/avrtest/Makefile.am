#
#  $Id$
#

AVR_GCC=@AVR_GCC@

if USE_AVR_CROSS
AVR_TESTS     = maxruntime test_abort test_exit
AVR_MODEL     = atmega128
AVR_MCU_CFLAG =-mmcu=$(AVR_MODEL)

check_target=runsim
build_targets=$(AVR_TESTS)
endif

noinst_DATA  = $(build_targets)

avr_objects=test_maxruntime.o test_abort.o test_exit.o avrtest_help.o
CLEANFILES = $(AVR_TESTS) $(avr_objects) trace

%.o: $(srcdir)/%.c
	$(AVR_GCC) -g -Os $(AVR_MCU_CFLAG) -c -o $@ $< 

maxruntime:  test_maxruntime.o avrtest_help.o
	$(AVR_GCC) $(AVR_MCU_CFLAG) -o $@ $^

test_abort: test_abort.o avrtest_help.o
	$(AVR_GCC) $(AVR_MCU_CFLAG) -o $@ $^

test_exit: test_exit.o avrtest_help.o
	$(AVR_GCC) $(AVR_MCU_CFLAG) -o $@ $^

run_sim: run_maxruntime run_abort run_exit

run_maxruntime:  maxruntime 
	@echo "*** Running until timeout ***"
	../../src/simulavr --device $(AVR_MODEL) -f maxruntime \
	    -W 0x52,- -m 1000000
	@echo "*** Did it timeout? ***"

run_abort: test_abort
	@echo "*** Running until voluntary abort ***"
	-../../src/simulavr --device $(AVR_MODEL) -f test_abort \
	    -W 0x52,- -a 0x49
	@echo "*** Did it abort? ***"

run_exit: test_exit
	@echo "*** Running until voluntary exit ***"
	../../src/simulavr --device $(AVR_MODEL) -f test_exit \
	    -W 0x52,- -e 0x4F
	@echo "*** Did it voluntarily exit? ***"
  
# run target run_sim together with other tests on "make check"
check-local: $(check_target)


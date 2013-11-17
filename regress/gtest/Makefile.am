#
#  $Id$
#

# google test library
GTEST_OBJS = $(GTEST_LIB_PATH)/src/gtest-port.cc \
             $(GTEST_LIB_PATH)/src/gtest-test-part.cc \
             $(GTEST_LIB_PATH)/src/gtest-filepath.cc \
             $(GTEST_LIB_PATH)/src/gtest-typed-test.cc \
             $(GTEST_LIB_PATH)/src/gtest-death-test.cc \
             $(GTEST_LIB_PATH)/src/gtest-printers.cc \
             $(GTEST_LIB_PATH)/src/gtest.cc 

GTEST_INCLUDE = -I$(GTEST_LIB_PATH)/include/gtest -I$(GTEST_LIB_PATH)/include -I$(GTEST_LIB_PATH)

GTEST_CXXFLAGS = -Dprivate=public -Dprotected=public 

# simulavr bindings
SIMULAVR_PATH = ../..
SIMULAVR_INCLUDE = -I$(SIMULAVR_PATH)/src
SIMULAVR_LIB = $(SIMULAVR_PATH)/src/.libs/libsimulavr.a

# design under test settings
OBJS_UNITTEST = session_001/unittest001.cpp \
                session_irq_check/unittest_irq.cpp \
                session_io_pin/unittest_io_pin.cpp \
                gtest_main.cpp

# target sources (needed for make dist), if you change this list, you have to change OBJS_TARGET too!
OBJS_SRC = session_001/avr_code.s \
           session_irq_check/check.s \
           session_irq_check/tc1.s \
           session_irq_check/tc2.s \
           session_irq_check/tc3.s \
           session_io_pin/tc1.s

# target objects (needed for test), if you change this list, you have to change OBJS_SRC too!
OBJS_TARGET = session_001/avr_code.atmega32.o \
              session_irq_check/check.atmega32.o \
              session_irq_check/tc1.atmega32.o \
              session_irq_check/tc2.atmega32.o \
              session_irq_check/tc3.atmega32.o \
              session_io_pin/tc1.atmega128.o

AM_CXXFLAGS = $(GTEST_CXXFLAGS) $(GTEST_INCLUDE) $(SIMULAVR_INCLUDE) -g

EXTRA_DIST = $(OBJS_SRC)

SUFFIXES = .c .s

CLEANFILES = */*.o

# design under test rules
if GTEST_USE
noinst_PROGRAMS = dut
dut_SOURCES = $(OBJS_UNITTEST)
nodist_dut_SOURCES = $(GTEST_OBJS)
dut_LDADD = -lpthread $(SIMULAVR_LIB) $(AVR_LIBIBERTY_LIB) $(AVR_LIBBFD_LIB) -lz
dut_DEPENDENCIES = $(SIMULAVR_LIB)
endif

define build-asm-m32
avr-gcc -Wa,--gstabs,-D -xassembler-with-cpp -mmcu=atmega32 $< -o $@
endef

define build-c-m32
avr-gcc -mmcu=atmega32 -O2 $< -o $@
endef

define build-asm-m128
avr-gcc -Wa,--gstabs,-D -xassembler-with-cpp -mmcu=atmega128 $< -o $@
endef

session_001/avr_code.atmega32.o: session_001/avr_code.s
	$(build-asm-m32)

session_irq_check/check.atmega32.o: session_irq_check/check.s
	$(build-asm-m32)

session_irq_check/tc1.atmega32.o: session_irq_check/tc1.s
	$(build-asm-m32)

session_irq_check/tc2.atmega32.o: session_irq_check/tc2.s
	$(build-asm-m32)

session_irq_check/tc3.atmega32.o: session_irq_check/tc3.s
	$(build-asm-m32)

session_io_pin/tc1.atmega128.o: session_io_pin/tc1.s
	$(build-asm-m128)

check-local: run-gtest

if GTEST_USE
run-gtest: dut $(OBJS_TARGET)
	./dut
else
run-gtest:
	@echo "  Google test library not configured. Test dosn't run!."
endif

.PHONY: run-gtest


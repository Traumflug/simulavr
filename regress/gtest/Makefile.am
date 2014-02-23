#
#  $Id$
#

# google test library
GTEST_OBJS = gtest-1.6.0/src/gtest-all.cc

GTEST_INCLUDE = -Igtest-1.6.0/include/gtest -Igtest-1.6.0/include -Igtest-1.6.0

GTEST_CXXFLAGS = -Dprivate=public -Dprotected=public 

GTEST_EXTRA_FILES = gtest-1.6.0/src/gtest-internal-inl.h \
                    gtest-1.6.0/src/gtest-filepath.cc \
                    gtest-1.6.0/src/gtest.cc \
                    gtest-1.6.0/src/gtest-port.cc \
                    gtest-1.6.0/src/gtest-printers.cc \
                    gtest-1.6.0/src/gtest-test-part.cc \
                    gtest-1.6.0/src/gtest-death-test.cc \
                    gtest-1.6.0/src/gtest_main.cc \
                    gtest-1.6.0/src/gtest-typed-test.cc \
                    gtest-1.6.0/CONTRIBUTORS \
                    gtest-1.6.0/CHANGES \
                    gtest-1.6.0/README \
                    gtest-1.6.0/COPYING \
                    gtest-1.6.0/include/gtest/gtest-printers.h \
                    gtest-1.6.0/include/gtest/gtest-param-test.h \
                    gtest-1.6.0/include/gtest/gtest_pred_impl.h \
                    gtest-1.6.0/include/gtest/gtest-death-test.h \
                    gtest-1.6.0/include/gtest/gtest_prod.h \
                    gtest-1.6.0/include/gtest/gtest.h \
                    gtest-1.6.0/include/gtest/gtest-spi.h \
                    gtest-1.6.0/include/gtest/gtest-typed-test.h \
                    gtest-1.6.0/include/gtest/gtest-test-part.h \
                    gtest-1.6.0/include/gtest/gtest-message.h \
                    gtest-1.6.0/include/gtest/internal/gtest-filepath.h \
                    gtest-1.6.0/include/gtest/internal/gtest-string.h \
                    gtest-1.6.0/include/gtest/internal/gtest-linked_ptr.h \
                    gtest-1.6.0/include/gtest/internal/gtest-port.h \
                    gtest-1.6.0/include/gtest/internal/gtest-type-util.h \
                    gtest-1.6.0/include/gtest/internal/gtest-death-test-internal.h \
                    gtest-1.6.0/include/gtest/internal/gtest-param-util-generated.h \
                    gtest-1.6.0/include/gtest/internal/gtest-tuple.h \
                    gtest-1.6.0/include/gtest/internal/gtest-internal.h \
                    gtest-1.6.0/include/gtest/internal/gtest-param-util.h

# simulavr bindings
SIMULAVR_PATH = ../..
SIMULAVR_INCLUDE = -I$(SIMULAVR_PATH)/src
SIMULAVR_LIB = $(SIMULAVR_PATH)/src/.libs/libsim.a

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

EXTRA_DIST = $(OBJS_SRC) $(GTEST_EXTRA_FILES)

SUFFIXES = .c .s

CLEANFILES = */*.o

# design under test rules
noinst_PROGRAMS = dut
dut_SOURCES = $(OBJS_UNITTEST) $(GTEST_OBJS)
dut_LDADD = -lpthread $(SIMULAVR_LIB) $(LIBZ_FLAGS) $(EXTRA_LIBS)
dut_DEPENDENCIES = $(SIMULAVR_LIB)

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
	@DOLLAR_SIGN@(build-asm-m32)

session_irq_check/check.atmega32.o: session_irq_check/check.s
	@DOLLAR_SIGN@(build-asm-m32)

session_irq_check/tc1.atmega32.o: session_irq_check/tc1.s
	@DOLLAR_SIGN@(build-asm-m32)

session_irq_check/tc2.atmega32.o: session_irq_check/tc2.s
	@DOLLAR_SIGN@(build-asm-m32)

session_irq_check/tc3.atmega32.o: session_irq_check/tc3.s
	@DOLLAR_SIGN@(build-asm-m32)

session_io_pin/tc1.atmega128.o: session_io_pin/tc1.s
	@DOLLAR_SIGN@(build-asm-m128)

if USE_AVR_CROSS
check-local: dut $(OBJS_TARGET)
	./dut
else
check-local:
	@echo "  Configure could not find AVR cross compiling environment so gtest"
	@echo "  can not be run."
endif


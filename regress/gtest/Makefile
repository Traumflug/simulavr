all: ut


GTEST_PATH=../../../../gtest-1.6.0/

GTEST_OBJ= \
			  $(GTEST_PATH)/src/gtest-port.o \
			  $(GTEST_PATH)/src/gtest-test-part.o \
			  $(GTEST_PATH)/src/gtest-filepath.o \
			  $(GTEST_PATH)/src/gtest-typed-test.o \
			  $(GTEST_PATH)/src/gtest-death-test.o \
			  $(GTEST_PATH)/src/gtest-printers.o \
			  $(GTEST_PATH)/src/gtest.o 

GTEST_INCLUDE=\
				  -I$(GTEST_PATH)/include/gtest \
				  -I$(GTEST_PATH)/include \
				  -I$(GTEST_PATH) 

GTEST_CXXFLAGS= \
					 -Dprivate=public \
					 -Dprotected=public 



OBJ_UNITTEST= \
				  session_001/unittest001.o \
				  session_irq_check/unittest_irq.o \
				  gtest_main.o

SIMULAVR_PATH=../..
#SIMULAVR_PATH=/home/zfrdh/avrsim/simulavr-1.0.0_patched_004pr/


SIMULAVR_INCLUDE= -I$(SIMULAVR_PATH)/src
SIMULAVR_LIB= $(SIMULAVR_PATH)/src/.libs/libsimulavr.a

LIBBFD= /home/schon_im_backup/work/binutils-2.19/build_avr/bfd/libbfd.a
LIBIBERTY=/home/schon_im_backup/work/binutils-2.19/build_avr/libiberty/libiberty.a


ut: CXXFLAGS+=$(GTEST_CXXFLAGS) $(GTEST_INCLUDE) $(SIMULAVR_INCLUDE) -g

ALL_OBJ= $(OBJ_UNITTEST) $(GTEST_OBJ)

ut: $(ALL_OBJ)
	g++ $(ALL_OBJ) -O2 -lpthread $(SIMULAVR_LIB) $(LIBBFD) $(LIBIBERTY) -lz -o ut

%.atmega32.o: %.s
	avr-gcc -Wa,--gstabs,-D -xassembler-with-cpp -mmcu=atmega32 $< -o $@

%.atmega32.o: %.c
	avr-gcc -mmcu=atmega32 -O2 $< -o $@



AVR_OBJ= \
			session_001/avr_code.atmega32.o \
			session_irq_check/check.atmega32.o \
			session_irq_check/tc1.atmega32.o \
			session_irq_check/tc2.atmega32.o \
			session_irq_check/tc3.atmega32.o \
			


go: ut $(AVR_OBJ)
	./ut

clean:
	rm -f ut
	rm -f $(ALL_OBJ)
	rm -f $(AVR_OBJ)



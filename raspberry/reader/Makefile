RPI?=24
SRC=src
APP=nrf_reader
DST=faure-mendy

CROSSDIR        = /users/enseig/franck/peri
CROSS_COMPILE   = $(CROSSDIR)/arm-bcm2708hardfp-linux-gnueabi/bin/bcm2708hardfp-

INC=$(HOME)/rf24/include
LIB=$(HOME)/rf24/lib
CFLAGS=-Wall -Wfatal-errors -O2 -I$(INC)
LDFLAGS=-L$(LIB) -lrf24

all:  $(APP).x

$(APP).x: $(APP).cpp
	$(CROSS_COMPILE)g++ -o $@ -I$(INC) $<  -O2 $(LDFLAGS)

upload:
	scp -P50$(RPI) $(APP).x pi@peri:$(DST)

clean:
	rm -f *.o *.x *~i

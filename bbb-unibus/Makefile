#CROSS=arm-linux-gnueabi-
CC=$(CROSS)gcc

CFLAGS	= \
	-std=c99 \
	-O3 \
	-W \
	-Wall \
	-Werror \
	-Wp,-MMD,.$@.d \
	-Wp,-MT,$@ \
	-I. \

bbb-unibus: unibus-monitor.o util.o gpio.o
	$(CC) -o $@ $^ -lm -lpthread

clean:
	$(RM) *.o

-include .*.d

firmware: firmware-install
	echo CAPE-BONE-UNIBUS > /sys/devices/bone_capemgr.8/slots

firmware-install: /lib/firmware/CAPE-BONE-UNIBUS-00A0.dtbo
/lib/firmware/CAPE-BONE-UNIBUS-00A0.dtbo: cape-bone-unibus.dts
	dtc -O dtb -o $@ -b 0 -@ $<

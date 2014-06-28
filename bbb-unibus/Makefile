CROSS=arm-linux-gnueabi-
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

/** \file
 * Memory mapped access to the GPIO registers on the BeagleBone Black.
 *
 * Based on the code in http://stackoverflow.com/a/20874882
 * by madscientist159
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include "gpio.h"
#include "util.h"

static const uintptr_t gpio_base[GPIO_COUNT] = {
	0x44E07000,
	0x4804C000,
	0x481AC000,
	0x481AE000,
};

static const uintptr_t pinmux_base = 0x44e10000;
static const unsigned pinmux_offsets[] = {
	[  2] = 0x150, // gpio0.2
	[  3] = 0x154, // gpio0.3
	[  4] = 0x158, // gpio0.4
	[  5] = 0x15C, // gpio0.5
	[  7] = 0x164, // gpio0.7
	[  8] = 0x0D0, // gpio0.8
	[  9] = 0x0D4, // gpio0.9
	[ 10] = 0x0D8, // gpio0.10
	[ 11] = 0x0DC, // gpio0.11
	[ 12] = 0x178, // gpio0.12
	[ 13] = 0x17C, // gpio0.13
	[ 14] = 0x180, // gpio0.14
	[ 15] = 0x184, // gpio0.15
	[ 20] = 0x1B4, // gpio0.20
	[ 22] = 0x020, // gpio0.22
	[ 23] = 0x024, // gpio0.23
	[ 26] = 0x028, // gpio0.26
	[ 27] = 0x02C, // gpio0.27
	[ 30] = 0x070, // gpio0.30
	[ 31] = 0x074, // gpio0.31
	[ 32] = 0x000, // gpio1.0
	[ 33] = 0x004, // gpio1.1
	[ 34] = 0x008, // gpio1.2
	[ 35] = 0x00C, // gpio1.3
	[ 36] = 0x010, // gpio1.4
	[ 37] = 0x014, // gpio1.5
	[ 38] = 0x018, // gpio1.6
	[ 39] = 0x01C, // gpio1.7
	[ 44] = 0x030, // gpio1.12
	[ 45] = 0x034, // gpio1.13
	[ 46] = 0x038, // gpio1.14
	[ 47] = 0x03C, // gpio1.15
	[ 48] = 0x040, // gpio1.16
	[ 49] = 0x044, // gpio1.17
	[ 50] = 0x048, // gpio1.18
	[ 51] = 0x04C, // gpio1.19
	[ 60] = 0x078, // gpio1.28
	[ 61] = 0x07C, // gpio1.29
	[ 62] = 0x080, // gpio1.30
	[ 63] = 0x084, // gpio1.31
	[ 65] = 0x08C, // gpio2.1
	[ 66] = 0x090, // gpio2.2
	[ 67] = 0x094, // gpio2.3
	[ 68] = 0x098, // gpio2.4
	[ 69] = 0x09C, // gpio2.5
	[ 70] = 0x0A0, // gpio2.6
	[ 71] = 0x0A4, // gpio2.7
	[ 72] = 0x0A8, // gpio2.8
	[ 73] = 0x0AC, // gpio2.9
	[ 74] = 0x0B0, // gpio2.10
	[ 75] = 0x0B4, // gpio2.11
	[ 76] = 0x0B8, // gpio2.12
	[ 77] = 0x0BC, // gpio2.13
	[ 78] = 0x0C0, // gpio2.14
	[ 79] = 0x0C4, // gpio2.15
	[ 80] = 0x0C8, // gpio2.16
	[ 81] = 0x0CC, // gpio2.17
	[ 86] = 0x0E0, // gpio2.22
	[ 87] = 0x0E4, // gpio2.23
	[ 88] = 0x0E8, // gpio2.24
	[ 89] = 0x0EC, // gpio2.25
	[110] = 0x190, // gpio3.14
	[111] = 0x194, // gpio3.15
	[112] = 0x198, // gpio3.16
	[113] = 0x19C, // gpio3.17
	[114] = 0x1A0, // gpio3.18
	[115] = 0x1A4, // gpio3.19
	[116] = 0x1A8, // gpio3.20
	[117] = 0x1AC, // gpio3.21
};


#define GPIO_SIZE  0xFFF
#define PINMUX_SIZE 0x1FFF

volatile uint32_t *
gpio_pin(
	gpio_t * const gpio,
	unsigned pin
)
{
	return (volatile uint32_t*)(gpio->pinmux + 0x800 + pinmux_offsets[pin]);
}


gpio_t *
gpio_init(void)
{
	gpio_t * const gpio = calloc(1, sizeof(*gpio));
	if (!gpio)
	{
		perror("calloc");
		return NULL;
	}

	const int fd = open("/dev/mem", O_RDWR|O_SYNC);
	if (fd < 0) {
		perror("/dev/mem");
		goto fail;
	}

	gpio->pinmux = mmap(
	    0,
	    PINMUX_SIZE,
	    PROT_READ | PROT_WRITE,
	    MAP_SHARED,
	    fd,
	    pinmux_base
	);

	if (gpio->pinmux == MAP_FAILED) {
		perror("mmap pinmux");
		goto fail;
	}

	/*
	for (int i = 0 ; i < 4*32 ; i++)
	{
		printf("%3d %d.%2d: %08x\n",
			i,
			i / 32,
			i % 32,
			*gpio_pin(gpio, i)
		);
	}
	*/

	for (int i = 0 ; i < GPIO_COUNT ; i++)
	{
		void * m = mmap(
		    0,
		    GPIO_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd,
		    gpio_base[i]
		);

		if (m == MAP_FAILED) {
			perror("mmap");
			goto fail;
		}

		gpio->gpio[i] = m;
	}

	close(fd);

	return gpio;

fail:
	if (fd >= 0)
		close(fd);
	free(gpio);
	return NULL;
}


int
gpio_config(
	gpio_t * const gpio,
	const unsigned bank_num,
	const unsigned bank_pin,
	const unsigned direction,
	const unsigned pullup,
	const unsigned initial_value
)
{
	// even though we have the pinmux device mapped,
	// we will still use the /sys files to change the gpio status
	// so that the Linux kernel knows how things are mapped.
	const unsigned pin_num = bank_num * 32 + bank_pin;
	volatile uint32_t * const pinptr = gpio_pin(gpio, pin_num);

	if (1)
	printf("%08"PRIxPTR" %3d: %2d %2d %08x dir=%d pull=%d val=%d\n",
		((uintptr_t) pinptr - (uintptr_t) gpio->pinmux) + pinmux_base,
		pin_num,
		bank_num,
		bank_pin,
		*pinptr,
		direction,
		pullup,
		initial_value
	);

	const char * export_name = "/sys/class/gpio/export";
	FILE * const export = fopen(export_name, "w");
	if (!export)
		die("%s: Unable to open? %s\n",
			export_name,
			strerror(errno)
		);

	fprintf(export, "%d\n", pin_num);
	fclose(export);

	if (direction)
	{
		char value_name[64];
		snprintf(value_name, sizeof(value_name),
			"/sys/class/gpio/gpio%u/value",
			pin_num
		);

		FILE * const value = fopen(value_name, "w");
		if (!value)
			die("%s: Unable to open? %s\n",
				value_name,
				strerror(errno)
			);

		fprintf(value, "%d\n", initial_value);
		fclose(value);
	}

	char dir_name[64];
	snprintf(dir_name, sizeof(dir_name),
		"/sys/class/gpio/gpio%u/direction",
		pin_num
	);

	FILE * const dir = fopen(dir_name, "w");
	if (!dir)
		die("%s: Unable to open? %s\n",
			dir_name,
			strerror(errno)
		);

	fprintf(dir, "%s\n", direction ? "out" : "in");
	fclose(dir);

	uint32_t pin_value = *pinptr;
	if (pullup)
	{
		*pinptr = (pin_value & ~0x18) | 0x10;
	} else {
		*pinptr = (pin_value & ~0x18) | 0x08;
	}

	if (0)
	printf("%08"PRIxPTR" %3d: %2d %2d %08x\n",
		((uintptr_t) pinptr - (uintptr_t) gpio->pinmux) + pinmux_base,
		pin_num,
		bank_num,
		bank_pin,
		*pinptr
	);

	return 0;
}


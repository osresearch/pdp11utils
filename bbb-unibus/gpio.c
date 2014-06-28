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

#define GPIO_SIZE  0xFFF
#define PINMUX_SIZE 0x1FFF

volatile uint32_t *
gpio_pin(
	gpio_t * const gpio,
	unsigned pin
)
{
	return (volatile uint32_t*)(gpio->pinmux + 0x800 + 4*pin);
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
	// this could be re-written to use the gpio interface
	(void) pullup; // FIX ME

	const unsigned pin_num = bank_num * 32 + bank_pin;
	volatile uint32_t * const pinptr = gpio_pin(gpio, pin_num);
	printf("%08"PRIxPTR" %3d: %2d %2d %08x\n", ((uintptr_t) pinptr - (uintptr_t) gpio->pinmux) + pinmux_base, pin_num, bank_num, bank_pin, *pinptr);
	const char * export_name = "/sys/class/gpio/export";
	FILE * const export = fopen(export_name, "w");
	if (!export)
		die("%s: Unable to open? %s\n",
			export_name,
			strerror(errno)
		);

	fprintf(export, "%d\n", pin_num);
	fclose(export);

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

	return 0;
}


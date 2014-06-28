/** \file
 * Memory mapped access to the GPIO registers on the BeagleBone Black.
 *
 * Based on the code in http://stackoverflow.com/a/20874882
 * by madscientist159
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "gpio.h"
#include "util.h"

static const uintptr_t gpio_base[GPIO_COUNT] = {
	0x44E07000,
	0x4804C000,
	0x481AC000,
	0x481AE000,
};

#define GPIO_SIZE  0x00000FFF

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
	free(gpios);
	return NULL;
}


int
gpio_config(
	gpio_t * const gpio_unused,
	const unsigned gpio,
	const unsigned pin,
	const unsigned direction,
	const unsigned pullup,
	const unsigned initial_value
)
{
	// this could be re-written to use the gpio interface
	(void) gpio_unused;
	(void) pullup; // FIX ME

	const unsigned pin_num = gpio * 32 + pin;
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


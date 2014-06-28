/** \file
 * Print the state of the Unibus lines.
 */
#include <stdio.h>
#include <unistd.h>
#include "util.h"
#include "bitshuffle.h"


/** GPIO pins used by the Unibus port.
 *
 * The device tree should handles part of this configuration for us.
 * So instead we have to repeat them here as well.
 */
static const uint8_t gpios0[] = {
	23, 27, 22, 10, 9, 8, 26, 11, 30, 31, 5, 3, 20, 4, 2, 14, 7, 15
};

static const uint8_t gpios1[] = {
	13, 15, 12, 14, 29, 16, 17, 28, 18, 19,
};

static const uint8_t gpios2[] = {
	2, 5, 22, 23, 14, 12, 10, 8, 6, 3, 4, 1, 24, 25, 17, 16, 15, 13, 11, 9, 7,
};

static const uint8_t gpios3[] = {
	21, 19, 15, 14, 17, 16
};

#define ARRAY_COUNT(a) ((sizeof(a) / sizeof(*a)))


int
gpio_config(
	const unsigned gpio,
	const unsigned pin,
	const unsigned direction,
	const unsigned initial_value
)
{
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


/*
 * Configure all of our output pins.
 * These must have also been set by the device tree overlay.
 * If they are not, some things will appear to work, but not
 * all the output pins will be correctly configured as outputs.
 */
static void
unibus_gpio_init(void)
{
	for (unsigned i = 0 ; i < ARRAY_COUNT(gpios0) ; i++)
		gpio_config(0, gpios0[i], 1, 0);
	for (unsigned i = 0 ; i < ARRAY_COUNT(gpios1) ; i++)
		gpio_config(1, gpios1[i], 1, 0);
	for (unsigned i = 0 ; i < ARRAY_COUNT(gpios2) ; i++)
		gpio_config(2, gpios2[i], 1, 0);
	for (unsigned i = 0 ; i < ARRAY_COUNT(gpios3) ; i++)
		gpio_config(3, gpios3[i], 1, 0);
}


typedef struct
{
	uint32_t	addr:18;
	uint32_t	data:16;
	uint8_t		c:2;
	uint8_t		dcl0:1;
	uint8_t		init:1;
	uint8_t		pa:1;
	uint8_t		pb:1;
	uint8_t		bbsy:1;
	uint8_t		intr:1;
	uint8_t		ssyn:1;
	uint8_t		npr:1;
	uint8_t		npg:1;
	uint8_t		br:4;
	uint8_t		bg:4;
} unibus_t;


int
unibus_read(
	unibus_t * const u
)
{
	const uint32_t g0 = gpio_read(0);
	const uint32_t g1 = gpio_read(1);
	const uint32_t g2 = gpio_read(2);
	const uint32_t g3 = gpio_read(3);

	// shuffle the bits in gpio0 to make the address
asm("nop");
	u->addr = 0
		| bit_range( 0, g0,  2, 7)
		| bit_range( 4, g0,  8, 12)
		| bit_range( 9, g0, 14, 15)
		| bit_range(11, g0, 20, 20)
		| bit_range(12, g0, 22, 23)
		| bit_range(14, g0, 26, 27)
		| bit_range(16, g0, 30, 31)
		;

asm("nop");
	u->data = 0
		| bit_range( 0, g2,  1, 16)
		;

}


int
main(
	int argc,
	char ** argv
)
{
	
}

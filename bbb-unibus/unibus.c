/** \file
 * Print the state of the Unibus lines.
 */
#include <stdio.h>
#include <unistd.h>
#include "gpio.h"
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

typedef struct
{
	gpio_t *	gpio;
	uint32_t	addr:18;
	uint32_t	data:16;
	uint8_t		code:2;
	uint8_t		aclo:1;
	uint8_t		init:1;
	uint8_t		parity:2;
	uint8_t		bus_busy:1;
	uint8_t		intr:1;
	uint8_t		slave_ack:1;
	uint8_t		slave_sync:1;
	uint8_t		master_sync:1;
	uint8_t		npr:1;
	uint8_t		npg:1;
	uint8_t		bus_req:4;
	uint8_t		bus_grant:4;
} unibus_t;



/*
 * Configure all of our output pins.
 * These must have also been set by the device tree overlay.
 * If they are not, some things will appear to work, but not
 * all the output pins will be correctly configured as outputs.
 */
static void
unibus_gpio_init(
	unibus_t * const u
)
{
	for (unsigned i = 0 ; i < ARRAY_COUNT(gpios0) ; i++)
		gpio_config(u->gpio, 0, gpios0[i], 0, 1, 0);
	for (unsigned i = 0 ; i < ARRAY_COUNT(gpios1) ; i++)
		gpio_config(u->gpio, 1, gpios1[i], 0, 1, 0);
	for (unsigned i = 0 ; i < ARRAY_COUNT(gpios2) ; i++)
		gpio_config(u->gpio, 2, gpios2[i], 0, 1, 0);
	for (unsigned i = 0 ; i < ARRAY_COUNT(gpios3) ; i++)
		gpio_config(u->gpio, 3, gpios3[i], 0, 1, 0);
}



unibus_t *
unibus_init(void)
{
	unibus_t * const u = calloc(1, sizeof(*u));

	u->gpio = gpio_init();

	for(int i = 0 ; i < 4 ; i++)
		printf("%08x\n", u->gpio->gpio[i][GPIO_OE]);

	unibus_gpio_init(u);

	return u;
}


void
unibus_read(
	unibus_t * const u
)
{
	const uint32_t g0 = gpio_read(u->gpio, 0);
	const uint32_t g1 = gpio_read(u->gpio, 1);
	const uint32_t g2 = gpio_read(u->gpio, 2);
	const uint32_t g3 = gpio_read(u->gpio, 3);

	// shuffle the bits in gpio0 to make the address
	u->addr = 0
		| bit_range( 0, g0,  2, 5)
		| bit_range( 4, g0,  7, 11)
		| bit_range( 9, g0, 14, 15)
		| bit_range(11, g0, 20, 20)
		| bit_range(12, g0, 22, 23)
		| bit_range(14, g0, 26, 27)
		| bit_range(16, g0, 30, 31)
		;

	u->data = bit_range( 0, g2,  1, 16);
	u->code = bit_range(0, g1, 28, 29);

	u->npg = bit(g1, 12);
	u->npr = bit(g1, 13);
	u->parity = bit_range(0, g1, 14, 15);
	u->slave_ack = bit(g1, 16);
	u->bus_busy = bit(g1, 17);

	u->intr = bit(g3, 19);
	u->master_sync = bit(g1, 18);
	u->slave_sync = bit(g3, 21);

	u->init = bit(g2, 17);
	u->aclo = bit(g1, 19);

	u->bus_grant = bit_range(0, g3, 14, 17);
	u->bus_req = bit_range(0, g2, 22, 25);
}


void
unibus_print(
	const unibus_t * const u
)
{
	//printf("%08x %08x %08x %08x ", g0, g1, g2, g3);
	printf("A=%06o D=%06o C%1x %s%1x/%1x NP%s%s P%1x%s%s%s%s%s%s\n",
		u->addr,
		u->data,
		u->code,
		u->bus_busy ? "B" : "b",
		u->bus_req,
		u->bus_grant,
		u->npr ? "R" : "r",
		u->npg ? "G" : "g",
		u->parity,
		u->aclo ? " ACLO" : "",
		u->init ? " INIT" : "",
		u->intr ? " INTR" : "",
		u->slave_ack ? " SACK": "",
		u->slave_sync ? " SSYN": "",
		u->master_sync ? " MSYN": ""
	);
}

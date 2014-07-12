/** \file
 * Print the state of the Unibus lines.
 */
#include <stdio.h>
#include <unistd.h>
#include "unibus.h"
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
	if (!u->gpio)
		die("gpio mapping failed\n");

	// bring up the GPIO in slave mode
	unibus_gpio_init(u);
	unibus_master(u, 0);

	for(int i = 0 ; i < 4 ; i++)
		printf("%08x\n", u->gpio->gpio[i][GPIO_OE]);

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


// Address bits mapped in GPIO0
const uint32_t g0_addr_mask = 0
	| bitmask(2, 5)
	| bitmask(7, 11)
	| bitmask(14, 15)
	| bitmask(20, 20)
	| bitmask(22, 23)
	| bitmask(26, 27)
	| bitmask(30, 31)
	;

const uint32_t g1_data_mask = 0
	| bitmask(1, 16)
	;


void
unibus_master(
	unibus_t * const u,
	int take_master
)
{
	if (take_master)
	{
		// force all the output direction bits hi
		u->gpio->gpio[0][GPIO_OE] |= g0_addr_mask;
		u->gpio->gpio[1][GPIO_OE] |= g1_data_mask;
	} else {
		// force all the output direction bits low
		// and enable the pullups
		u->gpio->gpio[0][GPIO_OE] &= ~g0_addr_mask;
		u->gpio->gpio[0][GPIO_OUT] |= g0_addr_mask;

		u->gpio->gpio[1][GPIO_OE] &= ~g1_data_mask;
		u->gpio->gpio[1][GPIO_OUT] |= g1_data_mask;
	}
}


void
unibus_write_addr(
	unibus_t * const u,
	const uint32_t addr
)
{
	// unpack all the bits
	uint32_t g0_bits = 0
		| bit_range( 2, addr,  0,  3)
		| bit_range( 7, addr,  4,  8)
		| bit_range(14, addr,  9, 10)
		| bit_range(20, addr, 11, 11)
		| bit_range(22, addr, 12, 13)
		| bit_range(26, addr, 14, 15)
		| bit_range(30, addr, 16, 17)
		;

	volatile uint32_t * const gpio0 = u->gpio->gpio[0];
	volatile uint32_t * const out_ptr = &gpio0[GPIO_OUT];
	*out_ptr = (*out_ptr & ~g0_addr_mask) | g0_bits;
	
	/*
	// force all the output bits low
	gpio0[GPIO_OE] &= ~g0_addr_mask;
	*/
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

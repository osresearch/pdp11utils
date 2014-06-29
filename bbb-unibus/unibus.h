/** \file
 * Unibus interface
 */
#ifndef _unibus_h_
#define _unibus_h_

#include <stdint.h>
#include "gpio.h"

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


extern unibus_t *
unibus_init(void);


extern void
unibus_read(
	unibus_t * const u
);


extern void
unibus_master(
	unibus_t * const u,
	int take_master
);


extern void
unibus_write_addr(
	unibus_t * const u,
	const uint32_t addr
);


extern void
unibus_print(
	const unibus_t * const u
);

#endif

/** \file
 * Monitor the Unibus.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "util.h"
#include "bitshuffle.h"
#include "unibus.h"


extern void usleep(int);

int
main(
	int argc,
	char ** argv
)
{
	(void) argc;
	(void) argv;

	unibus_t * const u = unibus_init();
	if (!u)
		die("Unable to initialize unibus\n");

	// try writing a zero to the address lines
	uint32_t addr = 0x12345;
	(void) addr;
	unibus_write_addr(u, 0xffffffff);

	while (1)
	{
		unibus_master(u, 1);
		for (int i = 0 ; i < 1000 ; i++)
		{
			unibus_write_addr(u, addr++);
		}
		unibus_master(u, 0);
		usleep(10000);
		unibus_read(u);
		unibus_print(u);
		usleep(10000);
	}

	return 0;
}

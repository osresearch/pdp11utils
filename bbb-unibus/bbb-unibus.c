/** \file
 * Monitor the Unibus.
 */
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "unibus.h"

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

	while (1)
	{
		unibus_read(u);
		unibus_print(u);
		sleep(1);
	}

	return 0;
}

/** \file
 * Memory mapped access to GPIO pins.
 */
#ifndef _gpio_h_
#define _gpio_h_

#include <stdint.h>


// OE: 0 is output, 1 is input
#define GPIO_OE 0x14d
#define GPIO_IN 0x14e
#define GPIO_OUT 0x14f

#define GPIO_COUNT 4

typedef struct
{
	volatile uint8_t * pinmux;
	volatile uint32_t * gpio[GPIO_COUNT];
} gpio_t;


gpio_t *
gpio_init(void);

int
gpio_config(
	gpio_t * const gpio_unused,
	const unsigned gpio,
	const unsigned pin,
	const unsigned direction, // 0 == in, 1 == out
	const unsigned pullup,
	const unsigned initial_value
);


static inline uint32_t
gpio_read(
	gpio_t * const gpio,
	unsigned which
)
{
	if (which >= GPIO_COUNT)
		return 0;

	return gpio->gpio[which][GPIO_IN];
}

#endif

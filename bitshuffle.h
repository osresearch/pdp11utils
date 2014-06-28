#ifndef _bitshuffle_h_
#define _bitshuffle_h_
/** \file
 * Bit shuffling routines.
 */
#include <stdint.h>

#if 1

static inline uint32_t
bitshift(
	const uint32_t word,
	const int delta
)
{
	if (delta > 0)
		return word << delta;
	else
	if (delta < 0)
		return word >> delta;
	else
		return word;
}


static inline uint32_t
bitmask(
	const int lo,
	const int hi
)
{
	return ((1 << (hi - lo + 1)) - 1) << lo;
}


static inline uint32_t
bit_range(
	const int to_bit,
	const uint32_t word,
	const int from_lo,
	const int from_hi
)
{
	return bitshift(word & bitmask(from_lo, from_hi), to_bit - from_lo);
}
#else

#define bitshift(word, delta) \
	((delta) > 0 ? ((word) << (delta)) : ((word) >> (delta)))

#define bitmask(lo, hi) \
	(((1 << ((hi) - (lo) + 1)) - 1) << (lo))

#define bit_range(to_bit, word, from_lo, from_hi) \
	bitshift((word) & bitmask(from_lo, from_hi), (to_bit) - (from_lo));

#endif

#endif

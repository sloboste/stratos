/* spi.c
 *
 */

#include "spi.h"

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

bool spi_write(uint8_t buffer[], size_t length) 
{
	/* STUB!!! */
	/* Do stuff to make the complier stop complaining */
	uint8_t x = buffer[0];
	size_t s = length;
	s = (size_t) x;
	x = (uint8_t) s;
	return true;
}
 
size_t spi_read(uint8_t buffer[], size_t length)
{
	/* STUB!!! */
	/* Do stuff to make the complier stop complaining */
	uint8_t x = buffer[0];
	size_t s = length;
	s = (size_t) x;
	x = (uint8_t) s;
	return 0;
}


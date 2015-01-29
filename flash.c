/* flash.c
 *
 */

#include "flash.h"

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

bool flash_store(int index, uint8_t buffer[], size_t length)
{
	/* STUB!!! */
	/* Do stuff to make the complier stop complaining */
	uint8_t x = buffer[0];
	size_t s = length;
	s = (size_t) x;
	x = (uint8_t) s;
	x = (uint8_t) index;
	return true;
}
 
size_t flash_retrieve(int index, uint8_t buffer[], size_t length)
{
	/* STUB!!! */
	/* Do stuff to make the complier stop complaining */
	uint8_t x = buffer[0];
	size_t s = length;
	s = (size_t) x;
	x = (uint8_t) s;
	x = (uint8_t) index;
	return 0;
}
 
bool flash_erase(int index)
{
	/* STUB!!! */
	/* Do stuff to make the complier stop complaining */
	int x = 0;
	x = index;
	index = x;
	return true;
}


/* swipe.c
 *
 */

#include "swipe.h"

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

bool swipe_enable(uint8_t buffer[], size_t length) 
{
	/* STUB!!! */
	/* Do something with parameters to make complier stop complaining */
	uint8_t x = buffer[0]; 
	size_t s = length;
	s = (size_t) x;
	x = (uint8_t) s;
	return true;
}
 
void swipe_disable()
{
	/* STUB!!! */
	return;
}


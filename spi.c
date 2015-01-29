/* spi.c
 *
 */

#include "spi.h"

#include "assert.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdio.h"
#include "stdint.h"

static uint8_t last_command;
static uint8_t data[10] = {0,1,2,3,4,5,6,7,8,9};
static uint8_t key;

bool spi_write(uint8_t buffer[], size_t length) 
{
	/* STUB!!! */
	assert(length > 0); 
	last_command = buffer[0];
	if (last_command == 0x02) { /* STORE_SUCCESS */
		key = buffer[1];
		return false;
	}
	return true;
}
 
size_t spi_read(uint8_t buffer[], size_t length)
{
	/* STUB!!! */
	static int read_num = 0;
	switch (read_num) {
	case 0: 
		/* verify id */
		assert(length == 2);
		buffer[0] = 0x34;
		buffer[1] = 0x12;
		break;
	case 1:
		/* ask for cmd */
		assert(length == 3);
		buffer[0] = 0x01; /* STORE */
		buffer[1] = 0x0A;
		buffer[2] = 0x00;
		break;
	case 2:
		/* give store data */
		assert(length == 10);
		int i;
		for (i = 0; i < 10; ++i) {
			buffer[i] = data[i];
		}
		break;
	case 4:
		/* ask for cmd */
		assert(length == 3);
		buffer[0] = 0x00; /* SET_KEY_ACTIVE */
		buffer[1] = key;
		break;

	case 5:
		/* ask for cmd */
		assert(length == 3);
		buffer[0] = 0x02; /* ERASE_KEY */
		buffer[1] = key;
		break;

	default:
		/* done */
		assert(0);
		break;
	}
	++read_num;
	return length;
}


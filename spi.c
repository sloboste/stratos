/* spi.c
 *
 */

#include "spi.h"

#include "assert.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdio.h"
#include "stdint.h"

static uint8_t data[10] = {0,1,2,3,4,5,6,7,8,9};
static uint8_t key;

bool spi_write(uint8_t buffer[], size_t length) 
{
	/* STUB!!! */
	static int write_num = 0;
	bool r = true;
	assert(length > 0); 
	switch(write_num) {
	case 2:
		key = buffer[1];
		printf("key = %d\n", key);
		break;
	case 7:
		/* no more */
		r = false;
		break;
	default:
		break;
	}
	++write_num;
	return r;
}
 
size_t spi_read(uint8_t buffer[], size_t length)
{
	/* STUB!!! */
	static int read_num = 0;
	size_t s = 0;
	printf("read_num = %d\n", read_num);
	switch (read_num) {
	case 0: 
		/* verify id */
		printf("give id\n");
		assert(length == 2);
		buffer[0] = 0x34;
		buffer[1] = 0x12;
		s = 2;
		break;
	case 1:
		/* asked for cmd */
		printf("request store\n");
		assert(length == 3);
		buffer[0] = 0x01; /* STORE */
		buffer[1] = 0x0A;
		buffer[2] = 0x00;
		s = 3;
		break;
	case 2:
		/* give store data */
		printf("give store data\n");
		assert(length == 10);
		int i;
		for (i = 0; i < 10; ++i) {
			buffer[i] = data[i];
		}
		s = 10;
		break;
	case 3:
		/* asked for cmd */
		printf("request set key active\n");
		assert(length == 3);
		buffer[0] = 0x00; /* SET_KEY_ACTIVE */
		buffer[1] = key;
		s = 2;
		break;

	case 4:
		/* asked for cmd */
		printf("request erase key\n");
		assert(length == 3);
		buffer[0] = 0x02; /* ERASE_KEY */
		buffer[1] = key;
		s = 2;
		break;

	default:
		/* done */
		printf("will abort here because I didnt spoof any more transactions\n");
		assert(0);
		break;
	}
	++read_num;
	return s;
}


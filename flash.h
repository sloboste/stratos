/* flash.h
 *
 */

#ifndef _FLASH_H
#define _FLASH_H

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

/* Public: Store a buffer in the given index in flash memory.
 *
 * The flash memory is organized into key:value pairs. You can store and
 * retrieve a buffer of infinite length from any given index.
 *
 * index - The index to store the buffer. Must be >= 0.
 * buffer - The data to store in flash.
 * length - The length of the buffer.
 *
 * Returns true if the data was written successfully to flash.
 * Returns false if the index or buffer was invalid.
 */
bool flash_store(int index, uint8_t buffer[], size_t length);
 
/* Public: Retrieve data from an index previously stored in flash memory.
 *
 * index - The stored flash memory index to retrieve.
 * buffer - A buffer to store the retrieved data.
 * length - The length of the buffer.
 *
 * Returns the number of bytes retrieved and written into the buffer.
 */
size_t flash_retrieve(int index, uint8_t buffer[], size_t length);
 
/* Public: Erase any data stored in flash at the index.
 *
 * index - The index of flash memory to erase.
 *
 * Returns true if the memory was erased successfully.
 */
bool flash_erase(int index);

#endif


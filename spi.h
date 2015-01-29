/* spi.h
 *
 */

#ifndef _SPI_H
#define _SPI_H

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

/* Public: Write data to the SPI bus.
 *
 * buffer - A buffer of data to write to SPI.
 * length - The length of the buffer.
 *
 * Returns true if there is data ready to be read from the slave.
 */
bool spi_write(uint8_t buffer[], size_t length);
 
/* Public: Read data from SPI.
 *
 * buffer - The buffer to store data received from SPI.
 * length - The length of the buffer, i.e. the maximum number of bytes to read.
 *
 * Returns the number of bytes written into the buffer.
 */
size_t spi_read(uint8_t buffer[], size_t length);

#endif


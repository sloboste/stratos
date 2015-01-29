/* swipe.h
 *
 */

#ifndef _SWIPE_H
#define _SWIPE_H

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

/* Public: Activate the magstripe with the given stripe data.
 *
 * buffer - A buffer containing the data to radiate from the magstripe.
 * length - The length of the buffer.
 *
 * Returns true if the magstripe was enabled successfully.
 */
bool swipe_enable(uint8_t buffer[], size_t length); 
 
/* Public: Disable the magstripe if activated.
 *
 * If the magstripe is not activate, this has no effect.
 */
void swipe_disable();

#endif


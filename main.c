/* main.c
 * Author: Steven Sloboda : sloboste@umich.edu
 */

#include "flash.h"
#include "spi.h"
#include "swipe.h"

#include "assert.h"
#include "stdbool.h" /* bool   */
#include "stddef.h"  /* size_t */
#include "stdint.h"  
#include "stdio.h"   /* DEBUGGING */
#include "stdlib.h"  /* DEBUGGING  rand */

/* Maximum number of stripe data that we want to store. This puts an upper limit
 * on the number of keys that we have to keep track of.
 * Note: it would be better if this were infinite...
 */
#define MAX_STRIPE_DATA 255 /* uint8_t is the key type right now... */


/* Keybox: responsible for keeping track of keys to identify each magnetic
 * 		   stripe datum.
 */
 /* Private: */
bool Keybox_key_status[MAX_STRIPE_DATA];
bool Keybox_data_active;
uint8_t Keybox_active_key;
uint8_t Keybox_num_in_use;

/* Public */
void Keybox_init()
{
	int i;
	for (i = 0; i < MAX_STRIPE_DATA; ++i) {
		Keybox_key_status[i] = false;
	}
	Keybox_num_in_use = 0;
	Keybox_data_active = false;
}

bool Keybox_is_empty()
{
	return (Keybox_num_in_use == MAX_STRIPE_DATA);
}

uint8_t Keybox_get_key()
{
	/* O(n)... room for improvement here 
	 * I would normally use a C++ std::set or a similar tree structure to hold
	 * the keys so that it would be O(log(n)) 
	 */
	 assert( !Keybox_is_empty() );
	 uint8_t key;
	 int i;
	 for (i = 0; i < MAX_STRIPE_DATA; ++i) {
		if (!Keybox_key_status[i]) {
			key = i;
			Keybox_key_status[i] = true;
			++Keybox_num_in_use;
			break;
		}
	 }
	 return key;
}

void Keybox_giveup_key(uint8_t key)
{
	assert(key < MAX_STRIPE_DATA);
	assert(Keybox_key_status[key]);
	Keybox_key_status[key] = false;
	--Keybox_num_in_use;
	return;
}

void Keybox_activate_key(uint8_t key)
{
	assert(key < MAX_STRIPE_DATA);
	assert(Keybox_key_status[key]);
	Keybox_active_key = key;
	Keybox_data_active = true;
}

uint8_t Keybox_get_active_key()
{
	return Keybox_active_key;
}

bool Keybox_key_in_use(uint8_t key)
{	
	assert(key < MAX_STRIPE_DATA);
	return Keybox_key_status[key];
}

void Keybox_deactivate()
{
	Keybox_data_active = false;
}
/* End Keybox stuff */


/* Commands that the host sends to the client during an spi write */
enum HostSPIcmds {
	VERIFY_CLIENT_ID 	= 0x00,
	ASK_FOR_CMD			= 0x01,
	STORE_SUCCESS		= 0x02,
	STORE_FAIL			= 0x03,
	ACTIVATE_SUCCESS	= 0x04,
	ACTIVATE_FAIL		= 0x05,
	ERASE_SUCCESS		= 0x06,
	ERASE_FAIL			= 0x07
};

/* Commands that the client sends to the host during an spi read
 * Note: the STORE_X commands refer to storing tracks 1, 2, and/or 3 on an 
 * 		 ISO/IEC 7813 magnetic stripe card. I made the asssumption for reading
 *		 data from the client that the size of the stores would correspond to
 *		 the sizes of the tracks on a card (rounded up in bytes).
 */
enum ClientSPIcmds {
	SET_KEY_ACTIVE		= 0x00,
	STORE_1				= 0x01,
	STORE_2				= 0x02,
	STORE_3				= 0x03,
	STORE_12			= 0x04,
	STORE_13			= 0x05,
	STORE_23 			= 0x06,
	STORE_123			= 0x07,
	ERASE_KEY			= 0x08,
	STORE_ARB			= 0x09
};

/* Assume the client has some sort of chip id number that we can use to verify
 * that we are indeed talking to the client we want to receive commands from.
 */
const uint16_t CLIENT_ID = 0x1234;

/* Assume poll_client is set to true by some sort of interrupt that occurs when
 * we want to poll the client (since we are the spi master)
 */
volatile bool poll_client = false;

/* Big old buffer FIXME*/
uint8_t buffer[128]; /* Store command will at most store 128 bytes */


int main(void) 
{
	/* Assume flash, spi, and swipe modules are initialized */
	printf("flash, spi, and swipe modules initialized\n"); /* DEBUGGING */

	/* Verify client id number (assume the client has some sort of chip id) */
	bool client_verified = false;
	uint8_t tx_buff[2];
	uint8_t rx_buff_id[2];
	tx_buff[0] = VERIFY_CLIENT_ID;
	spi_write(tx_buff, 1);
	spi_read(rx_buff_id, 2);
	uint32_t id = (uint16_t) (rx_buff_id[0]) + ((uint16_t) (rx_buff_id[1]) << 8);
	if (id == CLIENT_ID) {
		client_verified = true;	
		printf("Client ID verified\n"); /* DEBUGGING */
	} else {
		/* This would be a good spot to notify the user of an error. Not sure
		 * how to handle the error however...
		 */
		printf("Error: unknown client\n"); /* DEBUGGING */
	}
	
	/* Loop to handle client requests */
	while(1) {
		printf("Begin client request loop iteration\n"); /* DEBUGGING */

		if (poll_client && client_verified) {
			printf("poll_client && client_verified\n"); /* DEBUGGING */
			
			/* Reset poll_client */
			poll_client = false;

			/* Loop to ask client if it has commands to send */
			tx_buff[0] = ASK_FOR_CMD;
			while( spi_write(tx_buff, 1) == true ) {
				
				/* Receive the command */
				uint8_t rx_buff_cmd[1];
				spi_read(rx_buff_cmd, 1);

				/* Interpret the command */
				uint8_t rx_buff_key[1];
				switch (rx_buff_cmd[0]) {
				case SET_KEY_ACTIVE:
					/* Read key from client */
					spi_read(rx_buff_key, 1);
					/* Sanity check */
					assert( Keybox_key_in_use(rx_buff_key[0]) );
					/* Set this key as active */
					uint8_t size = flash_retrieve(rx_buff_key[0], buffer, 128);
					bool activated = swipe_enable(buffer, size); 
					if (activated) {
						Keybox_activate_key(rx_buff_key[0]);
					}
					/* Send response to client */
					tx_buff[0] = (activated) ? (ACTIVATE_SUCCESS) : (ACTIVATE_FAIL);
					spi_write(tx_buff, 1);
					break;
						
				case STORE_1:
					/* Read 128 bytes from client (track 1: 7 bits/char, 79 chars) FIXME: dont have to round to power of 2 */	
					spi_read(buffer, 128);
					/* Generate a key for this data and store data */
					uint8_t key;
					bool stored = false;
					if ( !Keybox_is_empty() ) {
						key = Keybox_get_key();
						stored = flash_store(key, buffer, 128);
					}
					/* Send response to client */
					tx_buff[0] = (stored) ? (STORE_SUCCESS) : (STORE_FAIL);
					if (stored) {
						tx_buff[1] = key;
						spi_write(tx_buff, 2);
					} else {
						spi_write(tx_buff, 1);
					}
					break;
				
				case STORE_2:
					/* Read data from client */

					/* Generate key and store data */

					/* Send response to client */


					break;

				case STORE_3:
					/* Read data from client */

					/* Generate key and store data */

					/* Send response to client */


					break;
				
				case STORE_12:
					/* Read data from client */

					/* Generate key and store data */

					/* Send response to client */


					break;
				
				case STORE_13:
					/* Read data from client */

					/* Generate key and store data */

					/* Send response to client */


					break;

				case STORE_23:
					/* Read data from client */

					/* Generate key and store data */

					/* Send response to client */


					break;
				
				case STORE_123:
					/* Read data from client */

					/* Generate key and store data */

					/* Send response to client */

					break;
				
				case ERASE_KEY:
					/* Read key from client */
					spi_read(rx_buff_key, 1);
					/* Sanity check */
					assert( Keybox_key_in_use(rx_buff_key[0]) );
					/* Deactivate this data if it is active */
					if (Keybox_get_active_key() == rx_buff_key[0]) {
						swipe_disable();
						Keybox_deactivate();
					}
					/* Erase the data associated with this key */
					bool erased = flash_erase(rx_buff_key[0]);
					if (erased) {
						Keybox_giveup_key(rx_buff_key[0]);
					}
					/* Send response to client */
					tx_buff[0] = (erased) ? (ERASE_SUCCESS) : (ERASE_FAIL);
					spi_write(tx_buff, 1);
					break;
				case STORE_ARB:
					/* Read data length from client */

					/* Read actual data from client */

					/* Generate key and store data */

					/* Send response to client */
				
					break;
						
				default:
					/* Undefined command */
					assert(0);
					break;
				} /* switch (rx_buff_cmd) */

			} /* while ( spi_read() == true ) */

		} else if (!client_verified) {	
			printf("NOT (poll_client && client_verified)\n"); /* DEBUGGING */

			/* Try to validate client again */
			uint8_t tx_buff[2];
			uint8_t rx_buff_id[2];
			tx_buff[0] = VERIFY_CLIENT_ID;
			spi_write(tx_buff, 1);
			spi_read(rx_buff_id, 2);
			uint32_t id = (uint16_t) (rx_buff_id[0]) + ((uint16_t) (rx_buff_id[1]) << 8);
			if (id == CLIENT_ID) {
				client_verified = true;	
				printf("Client ID verified\n"); /* DEBUGGING */
			} else {
				/* This would be a good spot to notify the user of an error. Not sure
		 		 * how to handle the error however...
		 		 */
				printf("Error: unknown client\n"); /* DEBUGGING */
			}
		} /* if (poll_client && client_verified) else if (!client_verified)  */
		
		/* Simulate poll_client being set by timer-triggered interrupt FIXME*/
		poll_client = !( (bool) (rand() % 4) );

	} /* while (1) */
	
	return 0;
} /* main */


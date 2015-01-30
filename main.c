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
#include "unistd.h"  /* DEBUGGING sleep */

/* Maximum number of stripe data that we want to store. This puts an upper limit
 * on the number of keys that we have to keep track of.
 * Note: it would be better if this were infinite since a limit was never
 */
#define MAX_STRIPE_DATA 255 /* uint8_t is the key type right now... */


/* Keybox: responsible for keeping track of keys to identify each magnetic
 * 		   stripe datum.
 * Note: I really should have written a wrapper for the flash api and
 *		 integrated this keybox idea into the wrapper so it would be hidden
 * 		 from the programmer.
 */
 
/* Private members */
bool Keybox_key_status[MAX_STRIPE_DATA]; /* Is the key out of the box? */
size_t Keybox_data_size[MAX_STRIPE_DATA]; /* Sizeof data associated with key */
bool Keybox_data_active; /* Is there data active on stripe? */
uint8_t Keybox_active_key; /* Key associated with the data active on stripe */
uint8_t Keybox_num_in_use; /* Number of keys currently out of the box */

/* Public functions */
/* Public: Initialize the keybox with no data active and all keys in the box
 */
void Keybox_init()
{
	int i;
	for (i = 0; i < MAX_STRIPE_DATA; ++i) {
		Keybox_key_status[i] = false;
	}
	Keybox_num_in_use = 0;
	Keybox_data_active = false;
}

/* Public: Check if the keybox has no keys left to take out
 * 
 * Returns true if all keys are in use, i.e., no more room in flash
 * Returns false otherwise
 */
bool Keybox_is_empty()
{
	return (Keybox_num_in_use == MAX_STRIPE_DATA);
}

/* Public: Take a key out of the keybox to use as an index for flash
 * 
 * Requires that the keybox is not empty
 *
 * Returns an available key
 */
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

/* Public: Return a key to the keybox, i.e., free an index in flash
 *
 * Requires that the key is a valid key that has been checked out
 */
void Keybox_giveup_key(uint8_t key)
{
	assert(key < MAX_STRIPE_DATA);
	assert(Keybox_key_status[key]);
	Keybox_key_status[key] = false;
	--Keybox_num_in_use;
	return;
}

/* Public: Mark this key (index in flash) as the active stripe data
 *
 * Requires that the key is a valid key that has been checked out
 */
void Keybox_activate_key(uint8_t key)
{
	assert(key < MAX_STRIPE_DATA);
	assert(Keybox_key_status[key]);
	Keybox_active_key = key;
	Keybox_data_active = true;
}

/* Public: Find out the index to the data active on the stripe
 *
 * Note: You must check if there is data active first before calling this
 *
 * Returns the key to the active stripe data
 */
uint8_t Keybox_get_active_key()
{
	assert(Keybox_data_active);
	return Keybox_active_key;
}

/* Public: Find out if the given key has been checked out from the keybox
 *
 * Requires that key is a valid key
 *
 * Returns true if key is not in the box, returns false otherwise
 */
bool Keybox_key_in_use(uint8_t key)
{	
	assert(key < MAX_STRIPE_DATA);
	return Keybox_key_status[key];
}

/* Public: Tell the keybox that there is no active data stripe
 */
void Keybox_deactivate()
{
	Keybox_data_active = false;
}

/* Public: Get the size of the data stored in flash at the given index
 * 
 * Requires that key is a valid key
 *
 * Returns the size of the data stored at index = key
 */
size_t Keybox_size_at(uint8_t key)
{
	assert(key < MAX_STRIPE_DATA);
	assert(Keybox_key_status[key]);
	return Keybox_data_size[key];
}

/* Public: Find out if there is active stripe data
 *
 * Returns true if there is active stripe data, false if there is not
 */
bool Keybox_active()
{
	return Keybox_data_active;
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
	ERASE_FAIL			= 0x07,
	BAD_CMD				= 0x08
};

/* Commands that the client sends to the host during an spi read */
enum ClientSPIcmds {
	SET_KEY_ACTIVE		= 0x00,
	STORE				= 0x01,
	ERASE_KEY			= 0x02
};

/* Assume the client has some sort of chip id number that we can use to verify
 * that we are indeed talking to the client we want to receive commands from.
 */
const uint16_t CLIENT_ID = 0x1234;

/* Assume poll_client is set to true by some sort of interrupt that occurs when
 * we want to poll the client (since we are the spi master)
 */
volatile bool poll_client = false;



int main(void) 
{
	/* Variables that will be used a lot */	
	bool client_verified = false;
	uint8_t tx_buff[1];
	uint8_t rx_buff_id[2];   /* lower 8, upper 8 */
	uint8_t rx_buff_cmd[3];  /* cmd, number bytes data (lower 8, upper 8) */
	uint8_t * buffer = NULL; /* general purpose buffer */
	size_t s;				 /* size of a buffer */

	/* Assume flash, spi, and swipe modules are initialized */
	printf("flash, spi, and swipe modules initialized\n"); /* DEBUGGING */

	/* Verify client id number (assume the client has some sort of chip id) */
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
			printf("ask for cmd\n");
			while( spi_write(tx_buff, 1) == true ) {
				
				/* Receive the command */
				spi_read(rx_buff_cmd, 3);
				printf("rx_buff_cmd[0] = %x\n", rx_buff_cmd[0]); /* DEBUGGING */
				printf("rx_buff_cmd[1] = %x\n", rx_buff_cmd[1]); /* DEBUGGING */
				printf("rx_buff_cmd[2] = %x\n", rx_buff_cmd[2]); /* DEBUGGING */

				/* Interpret the command */
				switch (rx_buff_cmd[0]) {
				case SET_KEY_ACTIVE:
					printf("set_key_active\n");
					/* rx_buff_cmd[0] = SET_KEY_ACTIVE 
					 * rx_buff_cmd[1] = key
					 * rx_buff_cmd[2] = unused
					 */
					/* Sanity check */
					assert( Keybox_key_in_use(rx_buff_cmd[1]) );
					/* Set this key as active */
					s = Keybox_size_at(rx_buff_cmd[1]); 
					buffer = malloc(s);
					size_t size = flash_retrieve(rx_buff_cmd[1], buffer, s);
					bool activated = swipe_enable(buffer, size);
					free(buffer); buffer = NULL; 
					if (activated) {
						Keybox_activate_key(rx_buff_cmd[1]);
					}
					/* Send response to client */
					tx_buff[0] = (activated) ? (ACTIVATE_SUCCESS) : (ACTIVATE_FAIL);
					spi_write(tx_buff, 1);
					break;
						
				case STORE:
					printf("store\n");
					/* rx_buff_cmd[0] = STORE 
					 * rx_buff_cmd[1] = num data bytes (lower 8)
					 * rx_buff_cmd[2] = num data bytes (upper 8)
					 */
					/* Read data from client */
					s = (size_t) (rx_buff_cmd[1] + (rx_buff_cmd[2] << 8));
					buffer = malloc(s);
					spi_read(buffer, s);
					/* Generate a key for this data and store data */
					uint8_t key;
					bool stored = false;
					if ( !Keybox_is_empty() ) {
						key = Keybox_get_key();
						stored = flash_store(key, buffer, s);
					}
					free(buffer); buffer = NULL;
					/* Send response to client */
					tx_buff[0] = (stored) ? (STORE_SUCCESS) : (STORE_FAIL);
					if (stored) {
						tx_buff[1] = key;
						spi_write(tx_buff, 2);
					} else {
						spi_write(tx_buff, 1);
					}
					break;
				
				case ERASE_KEY:
					printf("erase key\n");
					/* rx_buff_cmd[0] = ERASE_KEY 
					 * rx_buff_cmd[1] = key
					 * rx_buff_cmd[2] = unused
					 */
					/* Sanity check */
					assert( Keybox_key_in_use(rx_buff_cmd[1]) );
					/* Deactivate this data if it is active */
					if ( Keybox_active() &&
						(Keybox_get_active_key() == rx_buff_cmd[1]) ) {
						swipe_disable();
						Keybox_deactivate();
					}
					/* Erase the data associated with this key */
					bool erased = flash_erase(rx_buff_cmd[1]);
					if (erased) {
						Keybox_giveup_key(rx_buff_cmd[1]);
					}
					/* Send response to client */
					tx_buff[0] = (erased) ? (ERASE_SUCCESS) : (ERASE_FAIL);
					spi_write(tx_buff, 1);
					break;
						
				default:
					/* Undefined command */
					tx_buff[1] = BAD_CMD;
					spi_write(tx_buff, 1);
					break;
				} /* switch (rx_buff_cmd) */

				printf("ask for cmd\n");
			
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
		
		/* DEBUGGING */
		/* Simulate poll_client being set by timer-triggered interrupt */
		poll_client = !( (bool) (rand() % 2) );
		printf("set poll_client = %d\n", poll_client);
		/* Wait a bit so the output isn't way too fast */
		sleep(1);

	} /* while (1) */
	
	return 0;
} /* main */


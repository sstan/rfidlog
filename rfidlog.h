// rfid.h

#ifndef _RFIDLOG_h
#define _RFIDLOG_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define RFID_UART_RX_BUFF_SIZE 40
#define BAUD 9600


#define NUM_ACCEPTED_TAGS 2
const char accepted_tags[NUM_ACCEPTED_TAGS][32] =
	{	"0300019501",
		"test2"
	};


typedef enum {	SET_READER_ACTIVE,
				SET_FORMAT_HEXADECIMAL,
				SET_FORMAT_DECIMAL,
				SET_OUTPUT_MODE_0,
				SET_OUTPUT_MODE_1,
				READ_FIRMWARE_VERSION_CODE,
				READ_LOG_FILE} rfidlog_cmd;

extern volatile int  rfid_ok;	/* received OK flag */
extern volatile int  rcv;		/* received return or <CR> character flag. */
extern volatile int  rfid_tag;  /* Scanned tag is valid */
extern volatile char rfid_uart_rx_buff[RFID_UART_RX_BUFF_SIZE]; /* rx buffer */
extern volatile char rfid_tag_str[RFID_UART_RX_BUFF_SIZE]; /* The rx buffer is copied here */


void rfidlog_uart_init();
void rfidlog_send_command(rfidlog_cmd cmd);
void rfidlog_send_string(char * str);


#endif


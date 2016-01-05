// 
// 
// 

#include "rfidlog.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#include <util/setbaud.h>
#include <util/delay.h>
#include <string.h>

#define BV(x)			(1 << x)
#define setBit(P,B) 	(P |= BV(B))
#define clearBit(P,B)	(P &= ~BV(B))
#define toggleBit(P,B)	(P ^= BV(B))

const char SET_READER_ACTIVE_str[]			= "SRA\r";
const char SET_FORMAT_HEXADECIMAL_str[]		= "SFH\r";
const char SET_FORMAT_DECIMAL_str[]			= "SFD\r";
const char SET_OUTPUT_MODE_0_str[]			= "SO0\r";
const char SET_OUTPUT_MODE_1_str[]			= "SO1\r";
const char READ_FIRMWARE_VERSION_CODE_str[]	= "VER\r";
const char READ_LOG_FILE_str[]				= "RLF\r";

int next_ch_idx = 0;
volatile char * rfid_string_to_send_p = NULL;

volatile char rfid_uart_rx_buff[RFID_UART_RX_BUFF_SIZE];
volatile int  rfid_uart_rx_buff_idx;
volatile int  rfid_ok = 0;
volatile int  rfid_tag = 0;
volatile int  rcv = 0;
volatile char rfid_tag_str[RFID_UART_RX_BUFF_SIZE];


void clear_rx_buffer()
{
	for (rfid_uart_rx_buff_idx=0;
	rfid_uart_rx_buff_idx < RFID_UART_RX_BUFF_SIZE-1;
	rfid_uart_rx_buff_idx++)
	{
		rfid_uart_rx_buff[rfid_uart_rx_buff_idx] = '\0';
	}
	rfid_uart_rx_buff_idx = 0;
}

void rfidlog_uart_init()
{
	cli();
	clearBit(PRR1, PRUSART1);
	UBRR1H = (unsigned char) (UBRR_VALUE >> 8);
	UBRR1L = (unsigned char) (UBRR_VALUE);
	
	setBit(UCSR1B, RXCIE1); // RX Complete Interrupt Enable
	setBit(UCSR1B, RXEN1); // Receiver Enable
	setBit(UCSR1B, TXEN1); // Transmitter Enable
	
	clear_rx_buffer();
	rfid_ok = 0;
	rfid_tag = 0;
	
	sei();
	return;
}

void rfidlog_send_string(char * str)
{
	next_ch_idx = 0;
	if (str != NULL)
	{
		rfid_string_to_send_p = str;
		// UART Data Register Ready Interrupt Enable
		setBit(UCSR1B, UDRIE1);
	}
}

void rfidlog_send_command(rfidlog_cmd cmd)
{
	
	char * cmd_str_p = NULL;
	
	switch(cmd)
	{
		case SET_READER_ACTIVE:
			cmd_str_p = SET_READER_ACTIVE_str;
			break;
		case SET_FORMAT_HEXADECIMAL:
			cmd_str_p = SET_FORMAT_HEXADECIMAL_str;
			break;
		case SET_FORMAT_DECIMAL:
			cmd_str_p = SET_FORMAT_DECIMAL_str;
			break;
		case SET_OUTPUT_MODE_0:
			cmd_str_p = SET_OUTPUT_MODE_0_str;
			break;
		case SET_OUTPUT_MODE_1:
			cmd_str_p = SET_OUTPUT_MODE_1_str;
			break;
		case READ_FIRMWARE_VERSION_CODE:
			cmd_str_p = READ_FIRMWARE_VERSION_CODE_str;
			break;
		case READ_LOG_FILE:
			cmd_str_p = READ_LOG_FILE_str;
			break;
		default:
			cmd_str_p = NULL;
	}
	
	if (cmd_str_p != NULL)
	{
		next_ch_idx = 0;
		rfid_string_to_send_p = cmd_str_p;
		// UART Data Register Ready Interrupt Enable
		setBit(UCSR1B, UDRIE1);
	}
}


ISR(USART1_UDRE_vect)
{
	if (rfid_string_to_send_p == NULL)
	{
		return;
	}
	else if (rfid_string_to_send_p[next_ch_idx] == '\0')
	{
		clearBit(UCSR1B, UDRIE1);
		next_ch_idx = 0;
	}
	else
	{
		UDR1 = rfid_string_to_send_p[next_ch_idx++];
	}
}

void rfid_check_received_command()
{
	int cntr = 0;
	char * start = NULL;
	char ok[] = "OK";
	


	rcv = 1; /* received return or <CR> character flag. */
	
	/* Check if the received message is "OK" */
	if(strstr(rfid_uart_rx_buff, ok))
	{
		rfid_ok = 1; /* Set the received OK flag. */
	}
	else /* Else it must be a TAG. */
	{
    strcpy(rfid_tag_str, rfid_uart_rx_buff);
		/* Compare with all tags. */
		for (cntr=0; cntr < NUM_ACCEPTED_TAGS; cntr++)
		{
			if(start = strstr(rfid_uart_rx_buff, accepted_tags[cntr]))
			{
				rfid_tag = 1; /* set the scanned valid tag flag */
				break;
			}
		}
	}
}

ISR(USART1_RX_vect)
{
	char ch = UDR1; /* read the received character */
	int cntr = 0;
	
	
	if (rfid_uart_rx_buff_idx >= RFID_UART_RX_BUFF_SIZE-2)
	{
		/* If the buffer is full (it shouldn't happen), purge it . */
		clear_rx_buffer();
	}
	else if (ch == '\n' || ch == (char) 0x0D)
	{
		/* If we received the return character or <CR>,
		 * process the data from the buffer. 
		 */
		rfid_check_received_command();

		for (cntr=0; cntr < RFID_UART_RX_BUFF_SIZE-1; cntr++)
		{
			rfid_uart_rx_buff[cntr] = '\0';
		}
		rfid_uart_rx_buff_idx = 0;

	}
	else
	{
		/* Store the received character. */
		rfid_uart_rx_buff[rfid_uart_rx_buff_idx++] = ch;
	}
}

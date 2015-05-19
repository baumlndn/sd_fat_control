/*
 * usart.c
 *
 *  Created on: 11.02.2015
 *      Author: baumlndn
 */

#include "usart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

static volatile char rx_buf[255];
static volatile uint8_t rx_count;

void USART_Init( unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
	rx_count=0;
}

void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;

	cli();
	rx_count = 0;
	sei();
}

unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	/* Get and return received data from buffer */
	return UDR0;
}

uint8_t USART_ReadBuffer ( char * destBuffer, uint8_t maxLength )
{
	uint8_t idx;

	cli();

	if (rx_count < maxLength)
	{
		maxLength = rx_count;
	}

	for (idx=0;idx<maxLength;idx++)
	{
		destBuffer[idx] = rx_buf[idx];
	}

	sei();

	return maxLength;
}

uint8_t USART_BufferLength ( void )
{
	uint8_t retVal;

	cli();
	retVal = rx_count;
	sei();

	return retVal;
}

ISR(USART_RX_vect)
{
	/* Read and store received data into buffer */
	rx_buf[rx_count] = USART_Receive();
	rx_count++;
	rx_buf[rx_count] = '\0';
}

void LogLineString( char * param )
{
	uint8_t len = strlen(param);
	uint8_t idx;

	for ( idx=0 ; idx < len ; idx++)
	{
		USART_Transmit(param[idx]);
	}

	USART_Transmit(0x0D);
	USART_Transmit(0x0A);
}

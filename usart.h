/*
 * usart.h
 *
 *  Created on: 11.02.2015
 *      Author: baumlndn
 */

#ifndef USART_H_
#define USART_H_

#include <avr/io.h>

#define FOSC 8000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

void USART_Init( unsigned int ubrr);
void USART_Transmit( unsigned char data );
unsigned char USART_Receive( void );
uint8_t USART_ReadBuffer ( char * destBuffer, uint8_t maxLength );
uint8_t USART_BufferLength ( void );
void LogLineString( char * param );

#endif /* USART_H_ */

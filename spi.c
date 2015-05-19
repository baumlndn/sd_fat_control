/*
 * spi.c
 *
 *  Created on: 16.05.2015
 *      Author: baumlndn
 */

#include "spi.h"
#include "config.h"
#include "usart.h"
#include <avr/io.h>
#include <util/delay.h>

void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK)|(1<<2);
	/* Enable SPI, Master, set clock rate fck/64 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);

	/* Set SPI CS to output */
	DDR_SPI_CS |= (1<<DD_CS);
	/* Switch on CS */
	PORT_SPI_CS &= ~(1<<DD_CS);
}

void SPI_MasterTransmit(char cData)
{
	/* Activate CS */
//	PORT_SPI_CS &= ~(1<<DD_CS);
//	_delay_us(50);

	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	{
		/* Do nothing */
	}

	/* Deactivate CS */
//	PORT_SPI_CS |= (1<<DD_CS);
//	_delay_us(50);
}

char SPI_MasterRecheive()
{
	return SPDR;
}

char SPI_sendchar(char cData)
{
	SPI_MasterTransmit(cData);
	USART_Transmit(SPDR);

	return SPDR;
}

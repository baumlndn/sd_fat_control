/*
 * main.c
 *
 *  Created on: 11.02.2015
 *      Author: baumlndn
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "config.h"
#include "usart.h"
#include "spi.h"
#include "sd.h"

int main()
{
	USART_Init(MYUBRR);
	LogLineString("SD-Karten Test");
	SPI_MasterInit();

	if (ini_SD() == 0)
	{
		if (ini_FAT() == 0)
		{
			/* Write data to SD card */
			char tmpChar[] = "das ist ein Testinhalt";
//			writeFileToSD("FINAL   ",tmpChar,strlen(tmpChar));
		}
	}


	while (1)
	{
		/* Do nothing */
	}

	return 0;
}

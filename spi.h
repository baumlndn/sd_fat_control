/*
 * spi.h
 *
 *  Created on: 16.05.2015
 *      Author: baumlndn
 */

#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>

void SPI_MasterInit(void);
void SPI_MasterTransmit(char cData);
char SPI_MasterRecheive();
char SPI_sendchar(char cData);


#endif /* SPI_H_ */

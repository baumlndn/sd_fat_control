/*
 * config.h
 *
 *  Created on: 12.02.2015
 *      Author: baumlndn
 */

#ifndef CONFIG_H_
#define CONFIG_H_


#define SIM900_POWER_USAGE	1

#define SIM900_POWER_DDR	DDRD
#define SIM900_POWER_PORT	PORTD
#define SIM900_POWER_PIN	PD3

#define SIM900_RST_USAGE	1

#define SIM900_RST_DDR		DDRD
#define SIM900_RST_PORT		PORTD
#define SIM900_RST_PIN		PD2

#define WIFI_POWER_USAGE	1

#define WIFI_POWER_DDR		DDRD
#define WIFI_POWER_PORT		PORTD
#define WIFI_POWER_PIN		PD4

#define DELAY_MS_DEFAULT	500
#define DEBUG				1

#define DS1820_DDR			DDRD
#define DS1820_PORT			PORTD
#define DS1820_PIN			PIND
#define DS1820_BIT			PD7

enum ds1820_t
{
	DS18S20,
	DS18B20
};

//#define DS18S20

#define DDR_SPI 			DDRB
#define DD_MOSI				3
#define DD_MISO				4
#define DD_SCK				5
#define PIN_SPI				PINB

#define DDR_SPI_CS			DDRD
#define DD_CS				4
#define PORT_SPI_CS			PORTD

#endif /* CONFIG_H_ */

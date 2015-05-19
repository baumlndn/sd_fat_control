/*
 * sd.h
 *
 *  Created on: 17.05.2015
 *      Author: baumlndn
 */

#ifndef SD_H_
#define SD_H_

#include <avr/io.h>

typedef struct
{
	char     id_ca[8];
	char     label_ca[11];
	uint32_t startSector_u32;
	uint32_t startFAT1_u32;
	uint32_t startFAT2_u32;
	uint32_t sectorsPerFAT_u32;
	uint32_t startRoot_u32;
	uint8_t  sectorsPerCluster_u8;

} partitionData_t;

uint8_t ini_SD(void);
uint8_t ini_FAT(void);

void writeFileToSD(char * name, char * data, uint16_t length);
void testWriteSD();

#endif /* SD_H_ */

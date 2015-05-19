/*
 * sd.c
 *
 *  Created on: 17.05.2015
 *      Author: baumlndn
 */

#include "sd.h"
#include "config.h"
#include "usart.h"
#include "spi.h"
#include <avr/io.h>
#include <util/delay.h>


/* Locally used variables */
char sectorBuffer[514];
partitionData_t partitionOne_s;


/*Locally used functions */
unsigned char CRC7( char * chr, int cnt)
{
	int i,a;
	unsigned char crc,Data;

	crc=0;
	for (a=0;a<cnt;a++)
	{
		Data=chr[a];
		for (i=0;i<8;i++)
		{
			crc <<= 1;

			if ((Data & 0x80)^(crc & 0x80))
			crc ^=0x09;
			Data <<= 1;
		}
	}
	crc=(crc<<1)|1;
	return(crc);
}

char Command(char cmd, uint16_t ArgH, uint16_t ArgL, char crc )
{
	char input[5];

	input[0] = cmd;
	input[1] = (char) (ArgH>>8);
	input[2] = (char) (ArgH);
	input[3] = (char) (ArgL>>8);
	input[4] = (char) (ArgL);

     SPI_sendchar(0xFF);
     SPI_sendchar(cmd);
     SPI_sendchar((uint8_t)(ArgH >> 8));
     SPI_sendchar((uint8_t)ArgH);
     SPI_sendchar((uint8_t)(ArgL >> 8));
     SPI_sendchar((uint8_t)ArgL);
     SPI_sendchar(CRC7(input,5));
     SPI_sendchar(0xFF);
     return SPI_sendchar(0xFF);                // Returns the last byte received
}

void readSector(uint32_t param)
{
	/* Read block */
	Command(0x51,(uint16_t) (param>>16),(uint16_t) param,0xFF);

	char sectorString[] = "Read Sector 0x00000000";
	for (uint8_t i=0;i<8;i++)
	{
		uint8_t tmp = (uint8_t) ((param & (0xF << (i*4) )) >> (i*4));

		if (tmp < 10)
		{
			sectorString[21-i] = tmp + 0x30;
		}
		else
		{
			sectorString[21-i] = tmp + 0x37;
		}
	}

	LogLineString(sectorString);

	while (SPI_sendchar(0xFF) != 0xFE)
	{
		// wait
	}

	for (uint16_t idx=0;idx<514;idx++)
	{
		sectorBuffer[idx] = SPI_sendchar(0xFF);
	}
}

void writeSector(uint32_t sector, char * buffer)
{
	/* Send CMD24 */
	if (Command(0x58,((sector & 0xFFFF0000)>>16),(sector & 0x0000FFFF),0xFF) == 0x00)
	{
		SPI_sendchar(0xFE);
		for (uint16_t idx=0;idx<512;idx++)
		{
			SPI_sendchar(buffer[idx]);
		}
		SPI_sendchar(0xFF);
		SPI_sendchar(0xFF);
		SPI_sendchar(0xFF);
		while (SPI_sendchar(0xFF) == 0)
		{
			/* wait until SD Card is finished writing data */
		}
	}
	readSector(sector);
}

uint16_t findFreeCluster(void)
{
	uint16_t tmpCluster = 0;
	uint8_t tmpIdx = 2;

	readSector(partitionOne_s.startFAT1_u32);

	while ( (tmpIdx<127) && (tmpCluster == 0) )
	{
		if (
				(sectorBuffer[2*tmpIdx  ] == 0) &&
				(sectorBuffer[2*tmpIdx+1] == 0)
		   )
		{
			tmpCluster = tmpIdx;
		}
		tmpIdx++;
	}
	return tmpCluster;
}

void writeNewDirectoryEntry(char * title, uint32_t length, uint16_t cluster)
{
	uint32_t tmpEntry = 0;
	uint8_t tmpIdx = 0;
	uint8_t tmpSector = 0;

	while ( (tmpSector < partitionOne_s.sectorsPerCluster_u8) && (tmpEntry == 0) )
	{
		readSector(partitionOne_s.startRoot_u32+tmpSector);
		tmpIdx = 0;
		while ( (tmpIdx<16) && (tmpEntry == 0) )
		{
			if ( (sectorBuffer[32*tmpIdx] == 0xE5) ||
				 (sectorBuffer[32*tmpIdx] == 0x00)
			   )
			{
				tmpEntry = tmpIdx;
			}
			tmpIdx++;
		}
		tmpSector++;
	}

	if (tmpEntry != 0)
	{
		uint16_t tmpStart = 32*tmpEntry;

		for (uint8_t idx=0;idx<8;idx++)
		{
			sectorBuffer[tmpStart+idx] = title[idx];
		}

		sectorBuffer[tmpStart+ 8] = 'T';
		sectorBuffer[tmpStart+ 9] = 'X';
		sectorBuffer[tmpStart+10] = 'T';

		sectorBuffer[tmpStart + 0x0B    ] = 0x20;

		sectorBuffer[tmpStart + 0x1A + 0] = (char) ( (cluster & 0x00FF)       );
		sectorBuffer[tmpStart + 0x1A + 1] = (char) ( (cluster & 0xFF00) >>  8 );

		sectorBuffer[tmpStart + 0x1C + 0] = (char) ( (length & 0x000000FF)       );
		sectorBuffer[tmpStart + 0x1C + 1] = (char) ( (length & 0x0000FF00) >>  8 );
		sectorBuffer[tmpStart + 0x1C + 2] = (char) ( (length & 0x00FF0000) >> 16 );
		sectorBuffer[tmpStart + 0x1C + 3] = (char) ( (length & 0xFF000000) >> 24 );

		writeSector(partitionOne_s.startRoot_u32+tmpSector-1,sectorBuffer);
	}
}

/* return 1 = error */
uint8_t ini_SD(void)
{
     char i;

     PORT_SPI_CS |= _BV(DD_CS);                    //disable CS

     for(i=0; i < 10; i++)
     {
    	 SPI_sendchar(0xFF);                // Send 10 * 8 = 80 clock pulses 400 kHz
     }

     PORT_SPI_CS &= ~(_BV(DD_CS));                 //enable CS

     for(i=0; i < 2; i++)
     {
    	 SPI_sendchar(0xFF);                // Send 2 * 8 = 16 clock pulses 400 kHz
     }

     /* Reset SD Card controller */
     LogLineString ("CMD0 Reset");
     if ( Command(0x40,0,0,0x95) != 0x01 )
     {
    	 LogLineString("Init fail - Unknown card");
    	 return 1;
     }
     else
     {
    	 LogLineString("CMD8");
    	 Command(0x48,0,0x01AA,0xFF);
    	 SPI_sendchar(0xFF);
    	 SPI_sendchar(0xFF);
    	 if (
    			 (SPI_sendchar(0xFF) != 0x01) ||
				 (SPI_sendchar(0xFF) != 0xAA)
    	    )
    	 {
    		 LogLineString("Init fail - Unknown card");
    		 return 1;
    	 }
    	 else
    	 {
    		 LogLineString("ACMD41");
    	     Command(0x77,0,0,0xFF);

    	     while (Command(0x69,0x4000,0,0xFF) != 0)
    	     {
    	    	 Command(0x77,0,0,0xFF);
    	    	 // wait
    	     }

    	     LogLineString("CMD58");
    	     Command(0x7A,0,0,0xFF);

    	     if (SPI_sendchar(0xFF) & 0x40)
    	     {
    	    	 LogLineString("Init successful - SD Vers.2 Block address");
    	    	 SPI_sendchar(0xFF);
   	     	     SPI_sendchar(0xFF);
  	     	     SPI_sendchar(0xFF);
    	    	 return 0;
    	     }
    	     else
    	     {
    	    	 LogLineString("Init fail - Unknown card");
    	    	 return 1;
    	     }

    	 }

     }
}

uint8_t ini_FAT(void)
{
	uint32_t tmp_u32;

	/* Read MBR */
	readSector(0x00000000);

	if (
			(sectorBuffer[510] = 0x55) &&
			(sectorBuffer[510] = 0xAA)
	   )
	{

		/* Find 1st partition */
		tmp_u32  = ( (uint32_t) (sectorBuffer[0x01BE + 0x0C])      );
		tmp_u32 |= ( (uint32_t) (sectorBuffer[0x01BE + 0x0D]) <<  8);
		tmp_u32 |= ( (uint32_t) (sectorBuffer[0x01BE + 0x0E]) << 16);
		tmp_u32 |= ( (uint32_t) (sectorBuffer[0x01BE + 0x0F]) << 24);

		if (tmp_u32 > 0)
		{
			partitionOne_s.startSector_u32  = ( (uint32_t) (sectorBuffer[0x01BE + 0x08])      );
			partitionOne_s.startSector_u32 |= ( (uint32_t) (sectorBuffer[0x01BE + 0x09]) <<  8);
			partitionOne_s.startSector_u32 |= ( (uint32_t) (sectorBuffer[0x01BE + 0x0A]) << 16);
			partitionOne_s.startSector_u32 |= ( (uint32_t) (sectorBuffer[0x01BE + 0x0B]) << 24);

			readSector(partitionOne_s.startSector_u32);

			if (
					(sectorBuffer[510] = 0x55) &&
					(sectorBuffer[510] = 0xAA)
			   )
			{
				partitionOne_s.sectorsPerCluster_u8 = sectorBuffer[0x0D];

				partitionOne_s.startFAT1_u32  = partitionOne_s.startSector_u32;
				partitionOne_s.startFAT1_u32 |= ( (uint32_t) (sectorBuffer[0x0E])      );
				partitionOne_s.startFAT1_u32 |= ( (uint32_t) (sectorBuffer[0x0F]) <<  8);

				partitionOne_s.sectorsPerFAT_u32  = ( (uint32_t) (sectorBuffer[0x24])      );
				partitionOne_s.sectorsPerFAT_u32 |= ( (uint32_t) (sectorBuffer[0x25]) <<  8);
				partitionOne_s.sectorsPerFAT_u32 |= ( (uint32_t) (sectorBuffer[0x26]) << 16);
				partitionOne_s.sectorsPerFAT_u32 |= ( (uint32_t) (sectorBuffer[0x27]) << 24);

				partitionOne_s.startFAT2_u32 = partitionOne_s.startFAT1_u32 +     partitionOne_s.sectorsPerFAT_u32;
				partitionOne_s.startRoot_u32 = partitionOne_s.startFAT1_u32 + 2 * partitionOne_s.sectorsPerFAT_u32;

				readSector(partitionOne_s.startFAT1_u32);
				readSector(partitionOne_s.startFAT2_u32);
				readSector(partitionOne_s.startRoot_u32);

				for (uint8_t k=0;k<16;k++)
				{
					char tmp[] = "Name:xxxxxxxx.xxx Cluster:0xxxxxxxxx";
					tmp[ 5] = sectorBuffer[32*k+0];
					tmp[ 6] = sectorBuffer[32*k+1];
					tmp[ 7] = sectorBuffer[32*k+2];
					tmp[ 8] = sectorBuffer[32*k+3];
					tmp[ 9] = sectorBuffer[32*k+4];
					tmp[10] = sectorBuffer[32*k+5];
					tmp[11] = sectorBuffer[32*k+6];
					tmp[12] = sectorBuffer[32*k+7];

					tmp[14] = sectorBuffer[32*k+8];
					tmp[15] = sectorBuffer[32*k+9];
					tmp[16] = sectorBuffer[32*k+10];


					tmp[28] = sectorBuffer[32*k+0x1A+0];
					tmp[29] = sectorBuffer[32*k+0x1A+1];

					if (
							(sectorBuffer[32*k+0x0B] != 0xFF) && // Longname
							((sectorBuffer[32*k+0x0B] & 0x02) == 0x00) && // Hidden
							((sectorBuffer[32*k+0x0B] & 0x04) == 0x00) && // System file
							(sectorBuffer[32*k] != 0xE5) &&      // Deleted
							(sectorBuffer[32*k] != 0x00)         //Unused
					   )
					{
						LogLineString(tmp);
					}

				}
//				readSector(partitionOne_s.startRoot_u32+0x1A*partitionOne_s.sectorsPerCluster_u8);
				LogLineString("MBR and Partition1 read successfully");
				return 0;

			}
			else
			{
				LogLineString("No Partition Boot sector found");
				return 1;
			}
		}
		else
		{
			LogLineString("Partition 1 not found in MBR");
			return 1;
		}
	}
	else
	{
		LogLineString("No MBR found");
		return 1;
	}

}

void writeFileToSD(char * name, char * data, uint16_t length)
{
	uint16_t tmpCluster = findFreeCluster();
	uint32_t tmpSector  = partitionOne_s.startRoot_u32 + ( (tmpCluster-2) * partitionOne_s.sectorsPerCluster_u8);
	uint8_t tmpIdx = 0;

	/* First write Data into data section */
	if ( (length/512) > partitionOne_s.sectorsPerCluster_u8)
	{
		length = 512*partitionOne_s.sectorsPerCluster_u8;
	}

	for (uint16_t idxString=0;idxString<length;idxString++)
	{
		sectorBuffer[tmpIdx] = data[idxString];
		if (tmpIdx == 255)
		{
			writeSector(tmpSector,sectorBuffer);
			tmpSector++;
			tmpIdx = 0;
		}
		else
		{
			tmpIdx++;
		}
	}
	writeSector(tmpSector,sectorBuffer);

	/* Create Fill cluster in FAT1 and FAT 2 */
	readSector(partitionOne_s.startFAT1_u32);
	sectorBuffer[2*tmpCluster  ] = 0xFF;
	sectorBuffer[2*tmpCluster+1] = 0xFF;
	writeSector(partitionOne_s.startFAT1_u32,sectorBuffer);
	writeSector(partitionOne_s.startFAT2_u32,sectorBuffer);

	/* Finally update Root directory */
	writeNewDirectoryEntry(name,length,tmpCluster);
}

void testWriteSD()
{
	char tmpBuffer[512];
	for (uint16_t i=0;i<512;i++)
	{
		tmpBuffer[i] = (char) (i & 0x00FF);
	}

	readSector(0x00002002);
	writeSector(0x00002002,tmpBuffer);
	readSector(0x00002002);
}


/**
 * \file  ramdisk.c
 *
 * \brief Part of MSC device example application
 */

/*
* Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/ 
*
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <string.h>
#include "ramdisk.h"

#define RAM_DISK_SIZE (1024 * 1024 * 16)
#define LBA_SIZE 512
#define TRANSFER_SIZE 512

#if defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=(32)
unsigned char ram_disk[RAM_DISK_SIZE];
#elif defined __TMS470__ || defined _TMS320C6X
#pragma DATA_ALIGN(ram_disk, 32);
unsigned char ram_disk[RAM_DISK_SIZE];
#else
unsigned char ram_disk[RAM_DISK_SIZE]__attribute__((aligned(32)));
#endif



void ramdisk_disk_initialize(void)
{
	memset(ram_disk, 0,RAM_DISK_SIZE);
}

void ramdisk_disk_read(unsigned int lba, unsigned char *buf,
		unsigned int len)
{
	int start;
	
	start = lba * TRANSFER_SIZE;
	len = len * TRANSFER_SIZE;
	
	if (start + len <= RAM_DISK_SIZE )
	{
		memcpy(buf, &ram_disk[start], len);
	}

}
void ramdisk_disk_write(unsigned int lba, unsigned char *buf,
		unsigned int len)
{
	int start;
	
	start = lba * TRANSFER_SIZE;
	len = len * TRANSFER_SIZE;

	if (start + len <= RAM_DISK_SIZE )
	{
		memcpy(&ram_disk[start], buf, len);
	}
}

void ramdisk_disk_ioctl (unsigned int drive, unsigned int  command,  unsigned int *buffer)
{

	switch(command)
	{
        
        case GET_SECTOR_COUNT:
        {
           *buffer = (RAM_DISK_SIZE /LBA_SIZE);

            break;
        }
		case GET_SECTOR_SIZE:
        {
           *buffer = LBA_SIZE;

            break;
        }
		default:
		{
			buffer = 0;
			break;
		}
	
	}



}



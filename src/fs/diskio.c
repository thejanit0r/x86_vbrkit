/*---------------------------------------------------------------------------/
/  Petit FatFs - FAT file system module include file  R0.03   (C)ChaN, 2014
/----------------------------------------------------------------------------/
/ Petit FatFs module is an open source software to implement FAT file system to
/ small embedded systems. This is a free software and is opened for education,
/ research and commercial developments under license policy of following trems.
/
/  Copyright (C) 2014, ChaN, all right reserved.
/
/ * The Petit FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial use UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2014      */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "disk.h"
#include "mem.h"
#include "libc.h"


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
    return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
    BYTE* buff,     /* Pointer to the destination object */
    DWORD sector,   /* Sector number (LBA) */
    UINT offset,    /* Offset in the sector */
    UINT count      /* Byte count (bit15:destination) */
)
{
    uint8_t* buffer = (uint8_t *)malloc(512);

    if(buffer == NULL)
    {
        return RES_ERROR;
    }

    if(!disk_io(READ, disk_get_booting_drive(), sector, 1, buffer))
    {
        return RES_ERROR;
    }

    memcpy(buff, buffer + offset, count);
    free(buffer);

    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/

DRESULT disk_writep (
    BYTE* buff,     /* Pointer to the data to be written, NULL:Initiate/Finalize write operation */
    DWORD sc        /* Sector number (LBA) or Number of bytes to send */
)
{
    static int sector_number;
    static uint8_t* buffer;

    if(!buff) 
    {
        if(sc) 
        {
            buffer = (uint8_t *)malloc(512);

            sector_number = sc;

            if(buffer == NULL)
            {
                return RES_ERROR;
            }

            return RES_OK;
        } 
        else 
        {
            free(buffer);
        }
    } 
    else 
    {
        memset(buffer, 0, 512);
        memcpy(buffer, buff, sc);

        if(!disk_io(WRITE, disk_get_booting_drive(), sector_number, 1, buffer))
        {
            return RES_ERROR;
        }
    }

    return RES_OK;
}


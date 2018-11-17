/*
==============================================================================

Project:        MOST NetServices 
Module:         MOST Low Level Driver, Macro Definitions, Header Inclusion
File: 			most_drv.h
Version:        1.10.03 
Language:       C
Author(s):		S.Kerber
Date:			25.Feb.2005

FileGroup:      Layer I
Customer ID:    20D0FF0B011003.N.SIERE
FeatureCode:	FCR1
------------------------------------------------------------------------------

                (c) Copyright 1998-2005
                Oasis SiliconSystems AG
                All Rights Reserved

------------------------------------------------------------------------------



Modifications
~~~~~~~~~~~~~
Date    By      Description

==============================================================================
*/

#ifndef _MOSTDRV_H
#define _MOSTDRV_H




//------------------------------------------
// Serial Control Port Driver: I2C 
//------------------------------------------
#ifdef CP_I2C 
	#include "sw_i2c.h"							// Include your own respective headerfile
#endif



//------------------------------------------
// Serial Control Port Driver: SPI 
//------------------------------------------
#ifdef CP_SPI
	#include "spi.h"							// Include your own respective headerfile
#endif



//------------------------------------------
// Parallel Control Port Driver
//------------------------------------------
#ifdef CP_PAR
	#include "par_cp.h"							// Include your own respective headerfile
#endif



//------------------------------------------
// Com Port Driver 
// (only if using internal MOST Transceiver)
//------------------------------------------
#ifdef CP_COM 
	#include "com_port.h"						// Include your own respective headerfile
#endif



//------------------------------------------
// Parallel Asynchronous Source Port 
// Interface
//------------------------------------------
#ifdef PADT
	#include "par_sp.h"							// Include your own respective headerfile
#endif										




//-------------------------------------------------------
// Select MOST I2C Address if using I2C Interface
//-------------------------------------------------------
#define I2C_ADDR_MOST	0x40





//-------------------------------------------------------
// Access on Controll Port
//-------------------------------------------------------

#ifdef CP_I2C
	#define	MOST_WRITE(map,value)			I2cWrite((tMostMap)(map), (byte)(value), (byte)I2C_ADDR_MOST)

	#define	MOST_WRITEBLOCK(map,num,ptr)	I2cWriteBlock((tMostMap)(map), (byte)(num), (byte *)(ptr), (byte)I2C_ADDR_MOST)

	#define	MOST_READ(map)					I2cRead((tMostMap)(map), (byte)I2C_ADDR_MOST)

	#define	MOST_READBLOCK(map,num,ptr)		I2cReadBlock((tMostMap)(map), (byte)(num), (byte *)(ptr), (byte)I2C_ADDR_MOST)
#endif

#ifdef CP_SPI
	#define	MOST_WRITE(map,value)			SPIWrite((tMostMap)(map), (byte)(value))
						
	#define	MOST_WRITEBLOCK(map,num,ptr)	SPIWriteBlock((tMostMap)(map), (byte)(num), (byte *)(ptr))
								
	#define	MOST_READ(map)					SPIRead((tMostMap)(map))

	#define	MOST_READBLOCK(map,num,ptr)		SPIReadBlock((tMostMap)(map), (byte)(num), (byte *)(ptr))
#endif

#ifdef CP_PAR
	#define	MOST_WRITE(map,value)			ParWrite((tMostMap)(map),(byte)(value))

	#define	MOST_WRITEBLOCK(map,num,ptr)	ParWriteBlock((tMostMap)(map),(byte)(num), (byte *)(ptr))

	#define MOST_READ(map)					ParRead((tMostMap)(map))

	#define MOST_READBLOCK(map,num,ptr)		ParReadBlock((tMostMap)(map),(byte)(num), (byte *)(ptr))
#endif


#ifdef CP_COM
	#define	MOST_WRITE(map,value)			CheWrite((tMostMap)(map),(byte)(value))

	#define	MOST_WRITEBLOCK(map,num,ptr)	CheWriteBlock((tMostMap)(map),(byte)(num), (byte *)(ptr))

	#define MOST_READ(map)					CheRead((tMostMap)(map))

	#define MOST_READBLOCK(map,num,ptr)		CheReadBlock((tMostMap)(map),(byte)(num), (byte *)(ptr))
#endif



//-------------------------------------------------------
// Access on Parallel Asynchronous Source Port:
//-------------------------------------------------------
#ifdef PADT

#ifndef MOST_MAP_16BIT	// 8Bit map
	#define PAR_A_WRITE_MAP(page, map)					ParAWriteMap((byte)(page), (byte)(map))

	#define PAR_A_WRITE_SINGLE_BYTE(page, map, value)	ParAWriteSingleByte((byte)(page), (byte)(map), (byte)(value))

	#define PAR_A_WRITE_DATA(num, ptr)					ParAWriteData((byte)(num), (byte*)(ptr))

	#define PAR_A_READ_DATA(num, ptr)					ParAReadData((byte)(num), (byte*)(ptr))
#endif

#ifdef MOST_MAP_16BIT	// 16Bit map (page+map)
	#define PAR_A_WRITE_MAP(page, map)					ParAWriteMap((word)(map))						// parameter page is not used

	#define PAR_A_WRITE_SINGLE_BYTE(page, map, value)	ParAWriteSingleByte((word)(map), (byte)(value))	// parameter page is not used

	#define PAR_A_WRITE_DATA(num, ptr)					ParAWriteData((byte)(num), (byte*)(ptr))

	#define PAR_A_READ_DATA(num, ptr)					ParAReadData((byte)(num), (byte*)(ptr))
#endif

#endif




/*------------------------------------------------------------------------------------------------------------
	Secondary Node Solution
 	
 	The following macros are only required, if SECONDARY NODE is defined in adjust.h

	Please note: The following macros are just an example (control port in I2C mode).
------------------------------------------------------------------------------------------------------------*/
#ifdef SECONDARY_NODE

	#define I2C_ADDR_MOST_SEC	0x42

	#define MOST_WRITE_SEC(map, value)					I2cWrite((tMostMap)(map), (byte)(value), (byte)I2C_ADDR_MOST_SEC)

	#define MOST_WRITEBLOCK_SEC(map, num, ptr)			I2cWriteBlock((tMostMap)(map), (byte)(num), (byte *)(ptr), (byte)I2C_ADDR_MOST_SEC)

	#define MOST_READ_SEC(map)							I2cRead((tMostMap)(map), (byte)I2C_ADDR_MOST_SEC)

	#define MOST_READBLOCK_SEC(map, num, ptr)			I2cReadBlock((tMostMap)(map), (byte)(num), (byte *)(ptr), (byte)I2C_ADDR_MOST_SEC)

#endif



/*------------------------------------------------------------------------------------------------------------
	Extended Hardware Layer Interface
 	
 	The following macros are only required, if MNS_MSG_INTF	is defined in adjust.h
------------------------------------------------------------------------------------------------------------*/

//-------------------------------------------------------
// Access to the HW Layer CTRL-IN-FIFO
//-------------------------------------------------------
//	#define MOST_WRITE_CMD(pcmd)							MostWriteCmd((pTMnsCtrl)(pcmd))

//-------------------------------------------------------
// Access to the HW Layer CTRL-OUT-FIFO
//-------------------------------------------------------
//	#define MOST_READ_INFO(pinfo)							MostReadInfo((pTMnsCtrl)(pinfo))
//	#define MOST_INFO_PENDING								MostInfoPending()


//-------------------------------------------------------
// Access to the HW Layer MSG-IN-FIFO
//-------------------------------------------------------
//	#define	MOST_MSG_TRANSMIT(handle, pmsg, num_data)		MostMsgTransmit((word)(handle), (byte*)(pmsg), (byte)(num_data))

//-------------------------------------------------------
// Access to the HW Layer MSG-OUT-FIFO
//-------------------------------------------------------
//	#define MOST_MSG_RECEIVE(pmsg)							MostMsgReceive((byte*)(pmsg))
//	#define MOST_MSG_RXPENDING								MostMsgRxPending()



#endif // _MOSTDRV_H

//-------------------------------------------------------------------
// END OF most_drv.h
//-------------------------------------------------------------------

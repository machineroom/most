/*
==============================================================================

Project:        MOST NetServices 
Module:         Register Definitions of MOST Transceiver OS8104
File:           MostReg.h
Version:        1.10.03 
Language:       C
Author(s):      S.Kerber
Date:           25.Feb.2005

FileGroup:      Layer I
Customer ID:    20D0FF0B011003.N.SIERE
FeatureCode:    FCR1
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

#ifndef _MOSTREG_H
#define _MOSTREG_H


#ifndef _ADJUST_H
    // Due to downward capatibility (no macro _ADJUST_H in older ADJUST.H file):
    // When MOSTREG.H is used without ADJUST.H, the macro MOST_MAP_16BIT must be
    // defined, before MOSTREG.H is included, but only in case of 16Bit MAP mode
    // is demanded. When 8Bit MAP mode is desired, no macro has to be defined.
    // When you use MOSTREG.H in the ordinary way (with ADJUST.H) you can
    // ignore this hint.
#endif

#ifndef _MOSTDEF1_H
#ifndef USE_OWN_TYPE_DEFINITION 
typedef unsigned char byte;
typedef unsigned short int word;
#endif
#endif


#ifdef MOST_MAP_16BIT
    #define tMostMap        word    
    #define MOST_MAP_MASK   0xFFFF
#else
    #define tMostMap        byte
    #define MOST_MAP_MASK   0xFF
#endif      





//-------------------------------------------------------------------
// Registers of Page0
//-------------------------------------------------------------------
#define     RIT             (tMostMap)0x0000                        // Routing information table
#define     RIT_LENGTH      (tMostMap)0x0080                        // Length of Routing Section
#define     XCR             (tMostMap)0x0080                        // Transceiver-Control
#define     XSR             (tMostMap)0x0081                        // Transceiver-Status
#define     SDC1            (tMostMap)0x0082                        // Source Data Control 1
#define     CM1             (tMostMap)0x0083                        // Clock-Manager 1
#define     NC              (tMostMap)0x0084                        // Network Control
#define     MSGC            (tMostMap)0x0085                        // Message Control
#define     MSGS            (tMostMap)0x0086                        // Message Status
#define     NPR             (tMostMap)0x0087                        // Node position
#define     IE              (tMostMap)0x0088                        // Interrupt enable register
#define     GA              (tMostMap)0x0089                        // Group Address
#define     NAH             (tMostMap)0x008A                        // Node Address High
#define     NAL             (tMostMap)0x008B                        // Node Address Low
#define     SDC2            (tMostMap)0x008C                        // Source Data Control 2
#define     SDC3            (tMostMap)0x008D                        // Source Data Control 3
#define     CM2             (tMostMap)0x008E                        // Clock-Manager 2
#define     NDR             (tMostMap)0x008F                        // Node Delay
#define     MPR             (tMostMap)0x0090                        // Maximum Position
#define     MDR             (tMostMap)0x0091                        // Maximum Delay
#define     CM3             (tMostMap)0x0092                        // Clock-Manager 3 
#define     CM4             (tMostMap)0x0093                        // Clock-Manager 4 
#define     FRHI            (tMostMap)0x0094                        // Frequency Regulator High Control
#define     FRLO            (tMostMap)0x0095                        // Frequency Regulator Low Control
#define     SBC             (tMostMap)0x0096                        // Synchronous Bandwidth Control
#define     XSR2            (tMostMap)0x0097                        // Transceiver-Status 2
#define     RCMB            (tMostMap)0x00A0                        // Receive Ctrl Message Buffer
#define     RTYP            (tMostMap)0x00A0                        // Receive Message Type
#define     RSAH            (tMostMap)0x00A1                        // Source Address High
#define     RSAL            (tMostMap)0x00A2                        // Source Address Low
#define     RCD0            (tMostMap)0x00A3                        // Receive Control Data 0
#define     RCD1            (tMostMap)0x00A4                        // Receive Control Data 1           
#define     RCD2            (tMostMap)0x00A5                        // Receive Control Data 2
#define     RCD3            (tMostMap)0x00A6                        // Receive Control Data 3
#define     RCD4            (tMostMap)0x00A7                        // Receive Control Data 4
#define     RCD5            (tMostMap)0x00A8                        // Receive Control Data 5
#define     RCD6            (tMostMap)0x00A9                        // Receive Control Data 6
#define     RCD7            (tMostMap)0x00AA                        // Receive Control Data 7
#define     XTIM            (tMostMap)0x00BE                        // Xmit Retry Time
#define     XRTY            (tMostMap)0x00BF                        // Xmit Retry
#define     XCMB            (tMostMap)0x00C0                        // Xmit Control Message Buffer
#define     XPRI            (tMostMap)0x00C0                        // Xmit Priority
#define     XTYP            (tMostMap)0x00C1                        // Xmit Message Type
#define     XTAH            (tMostMap)0x00C2                        // Target Address High
#define     XTAL            (tMostMap)0x00C3                        // Target Address Low
#define     XCD0            (tMostMap)0x00C4                        // Xmit Control Data 0         
#define     XCD1            (tMostMap)0x00C5                        // Xmit Control Data 1
#define     XCD2            (tMostMap)0x00C6                        // Xmit Control Data 2
#define     XCD3            (tMostMap)0x00C7                        // Xmit Control Data 3
#define     XCD4            (tMostMap)0x00C8                        // Xmit Control Data 4
#define     XCD5            (tMostMap)0x00C9                        // Xmit Control Data 5
#define     XCD6            (tMostMap)0x00CA                        // Xmit Control Data 6
#define     XCD7            (tMostMap)0x00CB                        // Xmit Control Data 7
#define     XTS             (tMostMap)0x00D5                        // Xmit Transfer Status
#define     PCTC            (tMostMap)0x00E2                        // Packet Control
#define     PCTS            (tMostMap)0x00E3                        // Packet Status
#define     PCMA            (tMostMap)0x00E6                        // Parallel Combined Mode Register
#define     APAH            (tMostMap)0x00E8                        // Alternat. Packet Addr. High
#define     APAL            (tMostMap)0x00E9                        // Alternat. Packet Addr. Low
#define     PSTX            (tMostMap)0x00EA                        // Packet Start Tx Register
#define     PLDT            (tMostMap)0x00EC                        // Packet Length for Data Xmit
#define     PPI             (tMostMap)0x00F2                        // Packet Priority
#define     PAGE            (tMostMap)0x00FF                        // Swap RAM page 
                                                                        // (available in all pages)
//-------------------------------------------------------------------



//-------------------------------------------------------------------
// Registers of Page1
//-------------------------------------------------------------------
#define     ARP             (tMostMap)(0x0180 & MOST_MAP_MASK)      // Async Rcv. Packet Buffer
#define     ARTH            (tMostMap)(0x0180 & MOST_MAP_MASK)      // Received Tgt.Addr. High
#define     ARTL            (tMostMap)(0x0181 & MOST_MAP_MASK)      // Received Tgt.Addr. Low
#define     ASAH            (tMostMap)(0x0182 & MOST_MAP_MASK)      // Source Address High
#define     ASAL            (tMostMap)(0x0183 & MOST_MAP_MASK)      // Source Address Low
#define     ARD0            (tMostMap)(0x0184 & MOST_MAP_MASK)      // Async Receive Data 0
#define     AXP             (tMostMap)(0x01C0 & MOST_MAP_MASK)      // Async. Xmit Packet Buffer
#define     ATAH            (tMostMap)(0x01C0 & MOST_MAP_MASK)      // Received Tgt.Addr. High
#define     ATAL            (tMostMap)(0x01C1 & MOST_MAP_MASK)      // Received Tgt.Addr. Low
#define     AXD0            (tMostMap)(0x01C2 & MOST_MAP_MASK)      // Async Receive Data 0
//-------------------------------------------------------------------



//-------------------------------------------------------------------
// Registers of Page3
//-------------------------------------------------------------------
#define     CRA             (tMostMap)(0x0380 & MOST_MAP_MASK)      // Allocation Table
#define 	XLRTY           (tMostMap)(0x03C0 & MOST_MAP_MASK)      // Transmit Long Retry Register
#define 	XSTIM           (tMostMap)(0x03C1 & MOST_MAP_MASK)      // Transmit Short Retry Time Register
//-------------------------------------------------------------------



//-------------------------------------------------------------------
// Registers available in Standalone Mode only
//-------------------------------------------------------------------
#define     SIMB            (tMostMap)0x00EB                        // Standalone I2C Msg Buffer
#define     SIMC            (tMostMap)0x00EB                        // Count of bytes to send
#define     SITA            (tMostMap)0x00EC                        // I2C target address
#define     SIMA            (tMostMap)0x00ED                        // I2C MAP
#define     SITD0           (tMostMap)0x00EE                        // I2C transfer Data 0
#define     SITD1           (tMostMap)0x00EF                        // I2C transfer Data 1
#define     SITD2           (tMostMap)0x00F0                        // I2C transfer Data 2
#define     SITD3           (tMostMap)0x00F1                        // I2C transfer Data 3
#define     SITC            (tMostMap)0x00F2                        // I2C transfer control
//-------------------------------------------------------------------











//-------------------------------------------------------------------
// Bits of XCR
//-------------------------------------------------------------------
#define     MTR             (byte)0x80      // Master/Slave
#define     OE              (byte)0x40      // Output enable
#define     LBO             (byte)0x20      // Legacy Bypass Operation (since OS8104A)  
#define     LPW             (byte)0x10      // Low power wake-up
#define     SAN             (byte)0x08      // Stand-Alone mode
#define     SBY             (byte)0x04      // Source data bypass
#define     ABY             (byte)0x02      // All bypass
#define     REN             (byte)0x01      // RMCK enable
#define     O_EN            (byte)0x40      // Mask: Output enable  
#define     O_DIS           (byte)0xBF      // Mask: Output disable     

//-------------------------------------------------------------------
// Bits of XSR
//-------------------------------------------------------------------
#define 	FRA				(byte)0x80		// Frequency Regulator Active (OS8104A only)
#define     MSL             (byte)0x40      // Mask S/PDIF lock error
#define     MXL             (byte)0x20      // Mask transceiver lock error
#define     ME              (byte)0x10      // Mask coding error
#define     ERR             (byte)0x08      // All Error capture
#define  	FRLR			(byte)0x04		// Frequency Regulator locked to reference clock (OS8104A only)
#define     ESL             (byte)0x02      // Error capture S/PDIF
#define     EXL             (byte)0x01      // Error capture transceiver

//-------------------------------------------------------------------
// Bits of SDC1
//-------------------------------------------------------------------
#define     EDG             (byte)0x80      // active edge of SCK
#define     DEL             (byte)0x40      // Delay first bit against FSY
#define     POL             (byte)0x20      // Polarity of FSY
#define     IO              (byte)0x10      // I/O Select of FSY and SCK
#define     NBR             (byte)0x08      // Number of SCK cycles per frame
#define     SPD             (byte)0x04      // S/PDIF port enable
#define     MT              (byte)0x02      // Mute source data outputs
#define     TCE             (byte)0x01      // Transparent channel enable

//-------------------------------------------------------------------
// Bits of CM1
//-------------------------------------------------------------------
#define     PLD             (byte)0x80      // PLL disable

//-------------------------------------------------------------------
// Bits of NC
//-------------------------------------------------------------------
#define     ARE 			(byte)0x08      // Asynchronous Initialization Enable (OS8104A only)
#define     VRE             (byte)0x04      // Variable Retry Enable
#define     CME             (byte)0x02      // Channel Mute Enable

//-------------------------------------------------------------------
// Bits of CM3
//-------------------------------------------------------------------
// OS8104A only 
#define 	ENH				(byte)0x40		// Enhanced Mode Active
#define     FREN            (byte)0x10      // Frequency Regulator Enable
#define     ASR             (byte)0x08      // Automatic Switch to Crystal Enable
#define     ACD             (byte)0x04      // Automatic Crystal Disable
#define		FRR				(byte)0x02		// Frequency Regulator Reset

//-------------------------------------------------------------------
// Bits of MSGC
//-------------------------------------------------------------------
#define     STX             (byte)0x80      // Start transmission
#define     RBE             (byte)0x40      // Receive buffer enable
#define     SAI             (byte)0x10      // Start address initialisation
#define     RALC            (byte)0x08      // Reset Allocation change interrupt
#define     RERRPO          (byte)0x04      // Reset Error or Power-on after start-up interrupt
#define     RMTX            (byte)0x02      // Reset Message transmitted interrupt
#define     RMRX            (byte)0x01      // Reset Message received interrupt

//-------------------------------------------------------------------
// Bits of MSGS
//-------------------------------------------------------------------
#define     RBS             (byte)0x80      // Receive buffer status
#define     TXR             (byte)0x40      // Transmission result
#define     ALC             (byte)0x08      // Allocation change
#define     ERRPO           (byte)0x04      // Error or Power-on after start-up
#define     MTX             (byte)0x02      // Message transmitted
#define     MRX             (byte)0x01      // Message received

//-------------------------------------------------------------------
// Bits of IER
//-------------------------------------------------------------------
#define     IALC            (byte)0x08      // Interrupt on Allocation change
#define     IERR            (byte)0x04      // Interrupt on Error or Power-on after start-up
#define     IMTX            (byte)0x02      // Interrupt on Message transmitted
#define     IMRX            (byte)0x01      // Interrupt on Message received

//-------------------------------------------------------------------
// Bits of SDC3
//-------------------------------------------------------------------
#define     SIO             (byte)0x80      // S/PDIF in 8x mode IO select
#define     SPS             (byte)0x08      // S/PDIF sync source
#define     MTI             (byte)0x02      // Mute Source Port inputs
#define     SPEN            (byte)0x01      // Source Port Enable  

//-------------------------------------------------------------------
// Bits of CM2
//-------------------------------------------------------------------
#define     LOK             (byte)0x80      // PLL lock status
#define     NAC             (byte)0x40      // Network activity status
#define     ZP              (byte)0x20      // Zero power mode enable
#define     LP_MODE         (byte)0x10      // Low power mode enable
#define 	FCA				(byte)0x02		// Force Crystal Active	 (OS8104A only) 

//-------------------------------------------------------------------
// Bits of XSR2
//-------------------------------------------------------------------
#define     DFE             (byte)0x08      // Deep FIFO Enable
#define 	INV				(byte)0x02      // RX Inversion Control

//-------------------------------------------------------------------
// Bits of PCTC
//-------------------------------------------------------------------
#define     RAF             (byte)0x10      // Reset bit AF in PCTS
#define     RAC             (byte)0x08      // Reset bit AC in PCTS
#define     RATX            (byte)0x02      // Clear packet transmitted interrupt
#define     RARX            (byte)0x01      // Unlock the Asynchr. Receive Packet Buffer mARP

//-------------------------------------------------------------------
// Bits of PCTS
//-------------------------------------------------------------------
#define     AF              (byte)0x10      // Packet reject status (mARP full)
#define     AC              (byte)0x08      // Packet reject status (CRC failed)
#define     ARF             (byte)0x04      // Receiption failed (mARP full or CRC failed)
#define     ATX             (byte)0x02      // Packet transmitted event
#define     ARX             (byte)0x01      // Packet received event

//-------------------------------------------------------------------
// Bits of PCMA
//-------------------------------------------------------------------
#define     APCM            (byte)0x01      // Parallel Combined Mode Active

//-------------------------------------------------------------------
// Bits of PSTX
//-------------------------------------------------------------------
#define     ASTX            (byte)0x80      // Start packet transmission



#endif // _MOSTREG_H




                                                                                                                

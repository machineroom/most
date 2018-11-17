/*
==============================================================================

Project:        MOST NetServices
Module:         Adjustment for MOST NetServices
File: 			adjust.h
Version:        01.10.00
Language:       C
Author(s):		S.Kerber, R.Wilhelm
Date:			02.Sept.2002  
Contents:       Layer I+II
FileGroup:      
Customer ID:    <none> - To be used in conjunction with OSS Access DLL
------------------------------------------------------------------------------

                (c) Copyright 1998-2002
                Oasis SiliconSystems AG
                All Rights Reserved

------------------------------------------------------------------------------



Modifications
~~~~~~~~~~~~~
Date    By      Description

==============================================================================
*/

#ifndef _ADJUST_H
#define _ADJUST_H


/* 
 * MOST_LINUX:
 * Always define _OSS_ACCESS_DLL since the Linux NetServices libary uses the direct access to the
 * MOST registers and not message mode
 */
#define _OSS_ACCESS_DLL


//-------------------------------------------------------------------
//	MOST Hardware Definitions
//-------------------------------------------------------------------

//----------------------------------------------//
//	Define Interface Configuration				//
//----------------------------------------------//
#ifdef _OSS_ACCESS_DLL
#define	INTERFACE_CONFIG			7		 	// 		Select one configuration from table
#else
#define	INTERFACE_CONFIG			1		 	// 		Select one configuration from table
#endif
//----------------------------------------------//
//	Interf.Conf.  	 		Interf.Conf. ID	 	//
//----------------------------------------------//
//	SP ser. 	  / CP I2C  		1		 	//		Mode 1..8: If using MOST Transceiver OS8104
//	SP ser. 	  / CP SPI  		2		 	//
//	SP par. sync  / CP I2C  		3		 	//
//  SP par. sync  / CP SPI  		4		 	//
//	SP par. async / CP I2C			5		 	//
//  SP par. async / CP SPI			6		 	//
//	SP par. sync  / CP parallel		7		 	//
// 	SP par. async / CP parallel		8		 	//
//  											//
//  SP ser.       / CP Com Port 	9			//		Mode 9: If using internal MOST Transceiver (OS8401, OS8804, OS8805)
//----------------------------------------------//


//----------------------------------------------//
//	Define type of MOST Transceiver				//
//----------------------------------------------//
#define MOST_TRANSCEIVER_ID			1			//		ID 1: OS8104   			(default, when using INTERFACE_CONFIG 1..8)
												//		ID 2: OS8401 or OS8801  (default, when using INTERFACE_CONFIG 9)
												//		ID 3: OS8804
												//		ID 4: OS8805
//----------------------------------------------//


//----------------------------------------------//		Notice: the choice of serial source data port is fix,
//	Define Source Port mode and speed			//			    but you can dynamically switch between mode 1 and 2
//      (only if in serial mode)	  			//				or between mode 5 and 6 if using sometimes port 1
//----------------------------------------------//			    as transparent data channel.
#define SP_SER_MODE					1			//				If you want to use this feature, you have to choose
//												//				mode 2 or rather mode 6. 
//----------------------------------------------//
//	Mode     SP0   	SP1   	SP2    SP3			//
//----------------------------------------------//
//	 1	  	 Na    	Na    	Na     Na			//==>  	N... represents a serial port format and the
//	 2	  	 Na    	Tb    	Na     Na			// 			 clock/bit rate the port is running on
//   3       N128  	--    	N128   --			// 		
//   4       N256  	--    	--     --			// 	  	a	 represents the clock/bit rate defined 
//   5       S1x   	Na    	Na     Na			// 			 as SP_SER_CLOCKRATE 
//   6       S1x   	Tb    	Na     Na			//
//   8       S2x   	--    	Na     Na			//
//   9       S2x   	--    	N128   --			// 	  	S..x represents S/PDIF format with a certain speed
//  10       S4x   	--    	--     --			//
//  11       S8xIn 	--    	--     --			// 	  	T    represents a transparent format with a sample rate 
//  12       S8xOut	--    	--     --			// 			 defined as TRANSPARENT_SAMPLE_RATE
//												//
//----------------------------------------------//
//												//
#define SP_SER_CLOCKRATE			64			//==>	64,48,32,16 or 8 Fs (only if mode 1,2,5,6 or 8)
//											    // 		only effective on ports which has a variable 
//												//		speed in a certain mode
//												//
#define TRANSPARENT_SAMPLE_RATE		16			//==>	= SP_SER_CLOCKRATE / b  , there b can be 8,4,2 or 1
//												//
//												// 		==> b = SP_SER_CLOCKRATE / TRANSPARENT_SAMPLE_RATE
//												// 
//												// 		Transparent data transport is only available, 
//												//		if SP_SER_MODE = 2 or 6
//----------------------------------------------//



#define	MOST_MAP_16BIT							// If this switch is defined, all register definitions are 16bit
												// values. In that case the RAM page switching must be done
												// by the Hardware Layer (Low-Level-Driver).
												// If this switch is not defined, the RAM page switching is done
												// by the NetServices. In that case, the application have to make sure
												// that page 0 is selected, whenever returning from respective procedure.
												// If using a Multi-Task-OS with the need to access onto Transceiver's
												// registers by application, this switch must be defined and the page supervision
												// must be done by the Hardware Layer.





//-------------------------------------------------------------------
//	Circumference of Net Services 
//-------------------------------------------------------------------


//--------------------------
//  MOST Net Service LayerII
//--------------------------
//	#define SERVICE_LAYER_II						// Enable MOST NetServices Layer II

//	#define NS_AMS_AH								// Enable Interface between AMS and AH
//	#define NS_MNS_MNS2								// Enable Interface between MNS and MNS2
//	#define NS_MSV_NB								// Enable Interface between MSV and NetBlock
//	#define NS_SCS_NB								// Enable Interface between SCS and NetBlock



//--------------------------
//  MOST Net Service Kernel
//--------------------------
	#define MNS_MIN									// minimum requirement of MOST Net Services
	#define MNS_OPT_1								// Option 1: Provide Application with Request Flags for calling MostService()
	#define MNS_OPT_2								// Option 2: Parameter of MostService()	is evaluated
	#define MNS_OPT_3								// Option 3: Provides a function to get lowest timer value
													//			 This option requires the definition of macro TIMER_INT_OPT.


//--------------------------
//  Control Message Service
//--------------------------
	#define CMS_TX_MIN								// Tx Section, minimum requirement
	#define CMS_TX_ADD2								// single call transmission
	#define CMS_TX_ADD3								// check tx buffer
	#define CMS_TX_ADD4								// useful otional functions
	#define	CMS_TX_ADD5								// Provides an additional callback function, which can be
													// used to filter or dispatch a message before transmission
//	#define CMS_TX_ADD6								// Feature that allows to send system messages without the need
													// to implement the respective service (SCS or RCS).
													// The system messages are entered into the CMS TX buffer directly.


	#define CMS_RX_MIN								// Rx Section, minimum requirement
	#define CMS_RX_ADD1								// Msg polling 
	#define CMS_RX_ADD2								// copy message to local memory
	#define CMS_RX_ADD3								// possibility to use addition receive input
	#define CMS_RX_ADD4								// providing Rx filter


//--------------------------
//  Appl.Msg. Service
//--------------------------
	#define AMS_TX_MIN								// Tx Section, minimum requirement 
	#define AMS_TX_ADD1								// check tx buffer
	#define AMS_TX_ADD3								// providing useful functions for encoding data field
	#define AMS_TX_ADD4								// possibility to transmit large messages, without 
													// the need to provide a large AMS Tx Buffer
	#define	AMS_TX_ADD5								// Provides an additional callback function, which can be
													// used to filter or dispatch a message before transmission
	#define AMS_TX_ADD6								// possibility to transmit and receive internal messages
	#define AMS_TX_ADD7								// possibility to postpone a failed transmission
	#define AMS_TX_ADD8								// possibility to store the parameters (Data field) of the
													// AMS message in an own buffer


	#define AMS_RX_MIN								// Rx Section, minimum requirement
	#define AMS_RX_ADD1								// Msg polling
	#define AMS_RX_ADD2								// copy message to local memory
	#define AMS_RX_ADD3								// providing useful functions for decoding data field
	#define AMS_RX_ADD4								// possibility to receive internal messages 
	#define AMS_RX_ADD5								// possibility to receive messages that are longer than
													// the internal AMS RX buffer entries. 
	#define AMS_RX_ADD6								// check rx buffer



//	#define AMS_TX_NOSEG							// there is no need to be able to transmit segmented messages
//	#define AMS_RX_NOSEG							// there is no need to be able to receive segmented messages 


	#define AMS_TX_BYPASS_FILTER					// enables the possibility to bypass a message, which has been entered
													// in the AMS TX FIFO, to the CMS TX service directly.
													// Only possible if it is a single telegram and only if the target
													// address is unequal 0xFFFF.



//--------------------------
//  Remote Control Service
//--------------------------
	#define RCS_WRITE								// Remote Write available
	#define RCS_READ								// Remote Read available



//--------------------------
//  Synchr.Ch.Alloc. Service
//--------------------------
	#define SCS_SOURCE_ALLOC_MIN					// allocate / deallocate minimum requirement (source)
	#define SCS_SOURCE_RE_MIN						// connect / disconnect minimum requirement (source)
	#define SCS_SOURCE_ADD1							// single call functions for allocation and routing	(source)
	#define SCS_SINK_RE_MIN							// connect / disconnect minimum requirement (sink)
	#define SCS_ADD1								// detect channels by label (source & sink)

	//#define SCS_NO_ADDR_CALC						// Calculation of each address reference value for the
													// Routing-Engine is done by application or before compilation




#ifndef _OSS_ACCESS_DLL
//--------------------------
//  Transp.Ch.Alloc. Service
//--------------------------
	#define TCS_SOURCE_ALLOC						// allocate / deallocate transparent data channels
	#define TCS_SOURCE_RE							// connect / disconnect transparent data channels
	#define TCS_SOURCE_ALLOC_RE						// single call functions for transparent data channels
	#define TCS_SINK_RE								// connect / disconnect transparent data channels
#endif


//--------------------------
//  MOST Supervisor
//--------------------------
//	#define MSV_MINIMUM								// Select only one Supervisor (minimum OR extended OR virtual)

#ifdef _OSS_ACCESS_DLL
	#define MSV_EXTENDED
#endif

#ifndef _OSS_ACCESS_DLL
	#define MSV_VIRTUAL								// If this macro is defined, no MOST Supervisor will
#endif												// be implemented. In that case only an interface will be 
													// implemented between the "real" Supervisor in the 
													// Driver Layer and the NetServices Basic Layer.
													// Please make sure that MNS_MSG_INTF is defined,
													// if this item (MSV_VIRTUAL) is selected.


	#define MSV_ADD1								// enable for SET_BYPASS
													// by application. Only available, if MSV_EXTENDED is selected.


	#define MSV_ADD2								// Callback function MostUpdateNpr() is called, whenever
													// the Node Position Register (bNpr) has been updated.
													// Only available, if MSV_EXTENDED or MSV_VIRTUAL is selected.


	#define MSV_ADD3								// Enable this switch to avoid problems in using
													// the parallel asychronous sorce port interface in
													// case of an unlock. If an unlock occured, while having
													// access on this interface, the network is shut down.
													// This workaround is only needed if using the parallel
													// asynchronous source port interface.

	#define MSV_ADD4								// Additional initialisation function MostStartUpExt()

	#define MSV_ADD5								// Additional callback function that is called after the
													// MOST Transceiver has been reset


	#define MOST_CHECK_INT	Most_Por_Int()			// Only needed, if MSV_ADD3 is defined:
													// Define a function, that is called by module MSV to check
													// the state of the MOST Transceiver's interrupt pin (inverted).
													// That means that the function has to return TRUE, if the state of
													// pin is low.
													// If the interrupt pin is not accessible by the software, 
													// the macro has not to be defined.	



//--------------------------
//  MOST Control Service
//--------------------------
	#define MCS_MIN									// minimum requirement of MOST Control Service
	#define MCS_ADD1								// interrupt management
	#define MCS_ADD2								// address setting
	#define MCS_ADD3								// rmck frequency setting
	#define MCS_ADD4								// mute/demute source data outputs
	#define MCS_ADD5								// reading node delay and maximum delay
	#define MCS_ADD6								// provides access to alternative packet address
//	#define MCS_ADD7								// additional functions that are used in secondary node scenario
													// (only possible, if SECONDARY_NODE is defined)

	#define MCS_ADD8								// additional functions for accessing MOST Transceiver's register

	#define MCS_ADD9								// function to request channels by label, similar to SyncFindChannels()

	#define MCS_MASTER_ADD1							// pll inselect
	#define MCS_MASTER_ADD2							// bandwidth setting


#ifndef _OSS_ACCESS_DLL
	#define MOST_SET_NODEADR_EX(nadr)	MostSetNodeAdrEx((nadr))
													// Callback function, that is called on change of NodeAddress

	#define MOST_SET_GRPADR_EX(gadr)	MostSetGroupAdrEx((gadr))	
													// Callback function, that is called on change of GroupAddress
#endif




//--------------------------
//  Async Data Transmission
//        Service
//--------------------------
#ifndef _OSS_ACCESS_DLL
	#define ADS_TX_MIN								// Tx Section, minimum requirement
	#define ADS_TX_ADD1								// Single call transmission
	#define ADS_TX_ADD2								// check tx buffer
	#define ADS_TX_ADD3								// Application is recalled, whenever a packet was transmitted


	#define ADS_RX_MIN								// Rx Section, minimum requirement
	#define ADS_RX_ADD1								// polling rx section
	#define ADS_RX_ADD2								// copy received data packet to local memory
	#define ADS_RX_ADD3								// enable additional callback to filter and dispatch received telegrams
	#define ADS_RX_ADD4								// enable additional callback to notify captured error events
#endif









//-------------------------------------------------------------------
//	Control Message Service
//-------------------------------------------------------------------

//	Define Buffer Size
//--------------------------------

#define MAX_CTRL_TX_MSG			100				// size of Tx-Message-Buffer
#define MAX_TX_HANDLE			1				// number of extended tx data bytes, which will not be send (TxHandle)
#define MAX_CTRL_RX_MSG		   	100				// size of Rx-Message-Buffer
#define	MAX_EXT_DATA		   	8				// number of extended rx data bytes of a receive message (timestamp,...)


//  Define Switches
//--------------------------------
#define CTRL_TX_SUCCESS							// Application will also be recalled, if transmission was succesful
#define CTRL_TX_CRC				0				// number of occured crc errors until application will be informed about

//#define CTRL_RX_TELLEN						// Read rx buffer in respect of length field (depending on protocol).
												// If not defined, the whole rx buffer is read. 

//#define CTRL_FILTER_ID						// If defined: Each CMS and AMS buffer entry provides an additional 
												// filter ID field, which can be used to dispatch the message in the
												// respective filter functions.




//-------------------------------------------------------------------
//	Application Message Service
//-------------------------------------------------------------------

//	Define Buffer Size
//--------------------------------

#define MAX_MSG_TX_MSG			100				// size of Tx-Message-Buffer
#define MAX_MSG_TX_DATA			4096			// number of data bytes per tx application message
												// number of extended tx data bytes (TxHandle) is equal to the number 
												// of extended tx data bytes of the control message buffer

#define MAX_MSG_RX_MSG			100				// size of Rx-Message-Buffer
#define MAX_MSG_RX_DATA			4096			// number of data bytes per rx application message
												// number of extended rx data bytes is equal to the number of
												// extended bytes of the control message buffer
//  Define Switches
//--------------------------------
#define MSG_TX_SUCCESS							// Application will also be recalled, if transmission was succesful



//-------------------------------------------------------------------
//	Asyncronous Data Transmission Service
//-------------------------------------------------------------------

//	Define Buffer Size
//--------------------------------
#define MAX_DATA_TX_MSG			50				// size of Tx-Message-Buffer
#define MAX_DATA_TX_HANDLE		1				// number of extended tx data bytes, which will not be send (TxHandle)
#define MAX_DATA_RX_MSG		   	50				// size of Rx-Message-Buffer
#define	MAX_DATA_EXT_DATA	   	8				// number of extended rx data bytes of a receive packet (e.g. timestamp,...)





//-------------------------------------------------------------------
//	MOST Supervisor
//-------------------------------------------------------------------

#define	OFF_MODE_DEFAULT		1				// preselect the default value of Off_Mode:
												// 		0:	NET_OFF only by application request 
												// 		1:	NET_OFF by request and no light
												// 		2:	NET_OFF by request, no light and no lock
												// Please note: This value is relevant only in a timing
												//              master device. In a slave device this
												//              value will be 2.

//	#define MSV_TIMEOUT_DIAGNOSIS	30000		// Timeout for Ringbreak Diagnosis [msec]
												// Range:  4000..60000 [msec]
												// Please note: This optional macro should only be 
												// defined if desired by the system integrator.
												// If this macro is not defined the default value
												// that has been specified in the MOST Specification
												// is used.

									
//	#define  MSV_REMOTE_STARTUP_ENABLE			// TimingMaster only: 
												// Enable following workaround: All devices with closed bypass are
												// pushed into the network by sending remote write messages.
												// This workaround avoids problems forced by devices, which need a long
												// time ( > 100ms) to open its bypass.
												// This workaround is available only if using MSV_EXTENDED.

//-------------------------------------------------------------------
//	General Options
//-------------------------------------------------------------------

#define TIMER_INT_OPT							// If this macro is defined, the API function 
												// MostTimerIntDiff() must be used instead of MostTimerInt().
												// In that case the interval between two timer interrupts must be 
												// notified by argument. The macro TIMER_INT_DIV has no effect. 


#define TIMER_INT_DIV			25				// Select Frequency of Timer Interrupt 
												//		 1:	You have to call MostTimerInt() every 1ms.
												//		10: You have to call MostTimerInt() every 10ms.
												//		25: You have to call MostTimerInt() every 25ms.


// #define USE_OWN_TYPE_DEFINITION				// If this macro is NOT defined the default basic types
												// are used, which are defined in 'MostDef1.h'.
												// If this macro is defined, you have to define the following
												// basic data types by yourself: bool, byte, word, and dword.
												// In that case the basic type definitions must be included
												// in this 'adjust.h' file.



//-------------------------------------------------------------------
//  Extended Interface between Layer I and Hardware Layer
//-------------------------------------------------------------------

//#define	MNS_MSG_INTF							// The MOST NetServices Kernel provides the message based MNS Control Interface.
												// This interface puts command messages into the CTRL-IN-FIFO in the HW Layer.
												// It also receives info messages from the CTRL-OUT-FIFO in the HW Layer.

//#define CMS_MSG_INTF							// The Control Message Service (CMS) provides a message based interface.
												// It uses the MNS Control Interface to send Command Messages to the HW Layer
												// and to receive info messages from the HW Layer.
												// Furthermore it uses the MOST Message interface to send/ receive MOST messages.
												// Therefore it is a must to define MNS_MSG_INTF, if this feature is selected.

//#define	MCS_MSG_INTF							// The MOST Transceiver Control Service provides an additional interface
												// to write and read registers of the MOST Transceiver using the message based
												// MNS Control Interface. Therefore it is a must to define MNS_MSG_INTF, if
												// this feature is selected.

//#define NO_DIRECT_REG_ACCESS					// If this macro is defined, the standard hardware layer interface
												// (MOST_WRITE, MOST_READ, MOST_WRITEBLOCK, MOST_READBLOCK) is not used
												// by MCS, SCS and TCS.
												// This feature can only be selected, if MNS_MSG_INTF and MCS_MSG_INTF are enabled.
												// This feature is recommend in case of CMS_MSG_INTF and MSV_VIRTUAL is enabled.
												// In that case no direct control port access is performed by the MOST NetServices.



//-------------------------------------------------------------------
//  Secondary Node Solution
//-------------------------------------------------------------------
//	#define	SECONDARY_NODE						// One Transceiver provides access to control channel and asynchronous channel
												// The secondary node provides access to the synchronous channels.

//	#define SECONDARY_NODE_OPT_1				// If this macro is defined, the secondary note is in front of the primary node.
												// (RX of secondary node and TX of primary node are connected to the network).
												// If this macro is NOT defined, the primary node must be in front of the secondary node. 

//	#define	SECONDARY_NODE_CALC_ADDR(a) ((a)+1) // This rule determines the node address of the secondary node. 
												// Argument "a" means the node address of the primary node.

//	#define PADT								// Primary Node: Asynchronous data exchange is done via source port 
												//				 interface (parallel asynchronous mode).
												//               If this macro is not defined, the control port interface
												//				 is used to exchange asynchronous data.

												// Please select only one macro to choose the used source port mode in secondary node:
//	#define SP_SER								// Secondary Node: Source port is used in serial mode. 
//	#define SP_PAR								// Secondary Node: Source port is used in parallel synchronous mode.

												// Please select only one macro to choose the used control port mode in primary node:
//	#define CP_I2C								// Primary Node: Control port in I2C mode
//	#define CP_SPI								// Primary Node: Control port in SPI mode
//	#define CP_PAR								// Primary Node: Control port in PAR mode

	#define ERR_SEC_NODE_SPEC_2V1				// When this switch is defined, the error message "SecondaryNode" is
												// implemented as specified in MOST Spec Rev.2.1:
												// - "NetBlock.Pos.000.F.03.(0A.NodeAddrPrimNode)"
												//
												// If this switch is not defined, the error message is implemented 
												// as follows:
												// - "FBlockID.InstID.FktID.F.03.(0A.NodeAddrPrimNode)"

//	#define PWD_SENSITIVITY_SEC_NODE			//	Optional adjustment of the PWD sensitivity for the secondary node.
												//	When this macro is not defined, the bXSR2 register in the secondary node
												//  will contain the same value as in the primary node (macro PWD_SENSITIVITY).	
										
												





//-------------------------------------------------------------------
//	Predefined Transceiver Settings
//-------------------------------------------------------------------


// The following constants and switches have influence on the MOST registers 
// SDC1, SDC2, SDC3, XCR, XSR, CM1, SBC while initializing the MOST Transceiver.
// These information have to be set only if needed.

//----------------------------------------------//
//	Define Options (Register SDC1)				//
//----------------------------------------------//
#define	EDG_SCK		0							//	Active edge of SCK:
												//		0 = falling edge
												//		1 = rising edge
//----------------------------------------------//
#define DEL_FSY		0							//	Delay first bit against FSY:
												//		0 = no delay between first bit of a sample and FSY change
												//		1 = one SCK cycle delay
//----------------------------------------------//
#define	POL_FSY		0							//	Polarity of FSY:
												//		0 = FSY signal=1 indicates right sample
												//		1 = FSY signal=1 indicates left sample
//----------------------------------------------//
#define	IO_FSY		0							//	Input/Output select of FSY and SCK (only relevant if S/PDIF disabled):
												//		0 = FSY and SCK are both inputs
												//		1 = FSY and SCK are both outputs
//----------------------------------------------//
#define	SOURCE_MUTE	0							//	Mute Source Data Outputs (default after initialisation):
												//		0 = output ports SX0..3 in normal operation
												//		1 = output ports SX0..3 are digitally muted
//----------------------------------------------//		
#define TCH_DISABLE	0							//	Transparent Channel disable after initialisation:
												//	( only relevant if SP_SER_MODE allows usage of transparent channels)
												//		0 = source port 1 works in transparent mode
												//		1 = source port 1 is working as regular serial port until
												//			port 1 is explicitly set to transparent mode													
//----------------------------------------------//
#define SP_DISABLE	0							//	Source Ports disable (OS8805 only):
												//		0 = source ports are enabled 
												//		1 = source ports are disabled (EGPIO mode, only available in OS8805)
												//			Setting IO_FSY is ignored (bits 1..2 in bSDC1 will be set) 													
//----------------------------------------------//




//----------------------------------------------//
//	Define Options (Register SDC2)				//
//----------------------------------------------//
#define MFSY_ENABLE 0							//  Multi speed FSY enable
												//		1 = enable
												//		0 = disable
//----------------------------------------------//



//----------------------------------------------//
//	Define Options (Register SDC3)				//
//----------------------------------------------//
#define	SPDIF_SYNC	0							//	S/PDIF sync source (only relevant if S/PDIF enabled):
												//		0 = synchronize S/PDIF output timing to S/PDIF input
												//		1 = generate independent S/PDIF output timing
//----------------------------------------------//



//----------------------------------------------//
//	Define Options (Register XCR)				//
//----------------------------------------------//
#define	SOURCE_BYPASS		0					//	Source Data Bypass (default after initialisation):
												//		0 = Source data exchange available (sink and source)
												//		1 = Source data directly bypassed (sink only)
//----------------------------------------------//
#define RMCK_ENABLE			1					//	RMCK (System clock) output enable:
												//		1 = output enabled
												//		0 = output disabled (high impedance)
//----------------------------------------------//



//----------------------------------------------//
//	Define Options (Register XSR)				//
//----------------------------------------------//
#define	MASK_ERR_SPDIF		1					//	Mask S/PDIF lock error:
												//		0 = S/PDIF lock error is captured
												//		1 = S/PDIF lock error is ignored
//----------------------------------------------//
#define	MASK_ERR_TRANS		0					//	Mask transceiver lock error:
												//		0 = transceiver lock error is captured
												//		1 = transceiver lock error is ignored
//----------------------------------------------//
#define	MASK_ERR_CODING		0					//	Mask coding error:
												//		0 = coding error is captured
												//		1 = coding error is ignored
//----------------------------------------------//


//----------------------------------------------//
//	Define Options (Register CM1)				//
//----------------------------------------------//
#define PLL_INPUT			2					//	PLL input select:
												//  	This setting will be used only when the device
												//      operates as timing master, or whenever the 
												//		RMCK (additional clock output) is used in a slave 
												//		device while being in state VIRTUAL_ZERO_POWER. 
												//		It can be necessary for a slave device to operate
												//		as timing master while performing diagnosis.
												//		In a timing master device this setting will be used
												//		in normal operation too.
												//		In a slave device this setting will be ignored while
												//		the NetInterface is in normal operation mode.
												//		In that case the pll input will be set to rx network
												//		receive input pin.
												//		Possible values:
												//			1 = clock syncronized to S/PDIF input
												//			2 = clock syncronized to crystal or external clock input
												//			3 = clock syncronized to bit clock input (SCK)
												//
												//
#define PLL_NET_OFF			1					//	PLL operation mode while network is switched off:
												//		In a slave device it can be necessary to change the Pll input
												//		whenever the NetInterface is into state NET_OFF or 
												//		VIRTUAL_ZERO_POWER.
												//		Possible values:
												//			0 = pll input remain to rx network receive input pin (RMCK not available)
												//			1 = pll input is set to the value specified in PLL_INPUT (RMCK available)
												// 
												//		
//----------------------------------------------//
#define	XTL_DIVIDER			384					//	Oscillator divider:
												//		256, 384, or 512 Fs
//----------------------------------------------//
#define RMCK_DIVIDER		1024				//	RMCK divider:
												//		64, 128, 256, 384, 512, 768, 1024 or 1536 Fs
//----------------------------------------------//
												 				

//----------------------------------------------//
//	Define Synchronous Bandwith (Register SBC)	//
//----------------------------------------------//
#define SBC_DEFAULT			0x08				//	Synchronous Bandwith default value after StartUp
												//	(only relevant for timing master devices)
//----------------------------------------------//



//----------------------------------------------//
//	Adjust Optical Interface 					//
//----------------------------------------------//
#define PWD_SENSITIVITY		0x02				//	Adjustment of PWD sensitivity depending on the used optical interface:
												//		
												//	Possible values:   0x02: compensating short high signals (if using HP FOT-AddOn-Module)
												//					   0x00: compensating long high signals  (if using Infineon FOT)
//----------------------------------------------//



//----------------------------------------------//
//	Adjust Interrupt Enable Register			//
//----------------------------------------------//
//	#define INT_ENABLE_AFTER_RESET		0xF		//  This value contains the interrupts, which will be enabled after starting up the 
												//  MOST Transceiver. 
												//  If this macro is not defined by user, the value 0xF (all interrupts) is used by NetServices.
//----------------------------------------------//





//-------------------------------------------------------------------
//	Microcontroller specific Hardware Definitions
//  Only needed by MOST NetService Kernel
//-------------------------------------------------------------------

//	#define DISABLE_TIMER_INT	???		  			  // You can fill out these macros to enable/disable the timer-interrupt
//	#define ENABLE_TIMER_INT	???		  			  // depending on your target hardware


//	#define NS_INC_TGT_SPEC		"my_file.h"			  // This file is included in MNS.C, MNS2.C and MHP.C respectively.
													  // It must contain all the declarations, which are required
													  // by the target hardware specific instructions.
									
										  			  // Please note: These macros are optional and can be used if required
										  	
// Example for Cougar Microcontroller:
#ifdef COUGAR_COMPILER	
	
	#define DISABLE_TIMER_INT	(ier &= ~(int)0x0040) // macro to disable the timer-interrupt of the microcontroller
	#define ENABLE_TIMER_INT	(ier |=  (int)0x0040) // macro to enable the timer-interrupt of the microcontroller 

													  
	#define NS_INC_TGT_SPEC		"cougarns.h"		  // This file is included in MNS.C, MNS2.C and MHP.C respectively.
													  // It contains all Cougar specific definitions.
#endif
//-------------------------------------------------------------------




#define MOST_INT_RESET(a)		MostResetInt(a)








//-------------------------------------------------------------------
//	User specific Include Files 
//-------------------------------------------------------------------

// The following files are included in the respective NetServices modules:

//	#define NS_INC_ADS	"My_File.h"	 	// File is included in module ads.c
//	#define NS_INC_AMS	"My_File.h"	 	// File is included in module ams.c
//	#define NS_INC_CMS	"My_File.h"	 	// File is included in module cms.c	
//	#define NS_INC_MCS	"My_File.h"	 	// File is included in module mcs.c	
//	#define NS_INC_MNS	"My_File.h"	 	// File is included in module mns.c
#define NS_INC_MSV	"msv_linux.h"	 	// File is included in module msv.c
//	#define NS_INC_RCS	"My_File.h"	 	// File is included in module rcs.c
//	#define NS_INC_SCS	"My_File.h"		// File is included in module scs.c
//	#define NS_INC_TCS	"My_File.h"		// File is included in module tcs.c

//-------------------------------------------------------------------









#endif // _ADJUST_H


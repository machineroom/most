/*
==============================================================================

Project:        MOST NetServices 
Module:         Definition File for MOST NetServices Basic Layer
File:           MostDef1.h
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


#ifndef _MOSTDEF1_H
#define _MOSTDEF1_H





//-------------------------------------------------------------------
//  Data Type Definitions
//-------------------------------------------------------------------
#ifndef USE_OWN_TYPE_DEFINITION // Using basic type definition of NetServices ?
#ifndef __cplusplus
typedef unsigned char bool;
#endif
typedef unsigned char byte;
typedef unsigned short int word;
typedef unsigned long int dword;
#endif



//-------------------------------------------------------------------
//  General Definitions
//-------------------------------------------------------------------

#ifndef _CONST                  // not defined in adjust.h ?
 #ifdef _WIN32
    #define _CONST              // define dummy
 #else
    #define _CONST const
 #endif
#endif

#ifndef TRUE
    #define TRUE        0x01
#endif
#ifndef FALSE
    #define FALSE       0x00
#endif
#ifndef ON
    #define ON          0x01
#endif
#ifndef OFF
    #define OFF         0x00
#endif
#ifndef HIGH
    #define HIGH        0x01
#endif
#ifndef LOW
    #define LOW         0x00
#endif
#ifndef NIL
    #define NIL         0x0000
#endif
#ifndef NULL
   #define NULL         ((void *)0)
#endif
#ifndef ENABLE
    #define ENABLE      0x01
#endif
#ifndef DISABLE
    #define DISABLE     0x00
#endif


#ifdef TIMER_INT_OPT            // MostTimerIntDiff() is used instead of MostTimerInt()
    #ifdef TIMER_INT_DIV
    #undef TIMER_INT_DIV
    #define TIMER_INT_DIV   1   // Timer values are divided by 1ms, but there is no need to call
    #endif                      // MostTimerIntDiff() in a strict 1ms interval
#endif

#if (TIMER_INT_DIV != 1) && (TIMER_INT_DIV != 10) && (TIMER_INT_DIV != 25)
	#error Please select TIMER_INT_DIV as 1, 10, or 25
#endif                                        



#define MOST_BROADCAST_ADDRESS  0x03C8      // Broadcast Address on MOST control channel


//-----------------------------------------------------------------//
//  Debug Interface                                                //
//-----------------------------------------------------------------//

// General:
//-----------------
#ifndef MNS_DBG_LEVEL
    #define MNS_DBG_LEVEL   0   // no debug messages
#endif


//-------------------------------------------------------------------
//  Macros for getting Highbyte and Lowbyte of a 16 bit value
//-------------------------------------------------------------------
#ifndef HB
    #define HB(value)   (byte)((value)>>8)
#endif

#ifndef LB
    #define LB(value)   (byte)((value)&0xFF)
#endif


//-------------------------------------------------------------------
//  Macros for differentiate NIC revisions
//-------------------------------------------------------------------

#define NIC_REV_8104A (Most_Revision[0] >= 0x14)

//-------------------------------------------------------------------
//  Interface IDs
//-------------------------------------------------------------------
#define INTF_CTRL   0x01
#define INTF_ASYNC  0x02

//-------------------------------------------------------------------
//  Filter IDs
//-------------------------------------------------------------------
#ifndef CTRL_FILTER_ID_DEFAULT
  #define CTRL_FILTER_ID_DEFAULT    0xFF
#endif


//-------------------------------------------------------------------
//  Errors on function call (Return values)
//-------------------------------------------------------------------
#define ERR_NO          0x00                // no error occurred
#define ERR_PARAM       0x01                // wrong parameter(s)
#define ERR_NOMSG       0x02                // no rx message available
#define ERR_BUFOV       0x04                // tx buffer overflow
#define ERR_NOTAVAIL    0x08                // functionality not available


//-------------------------------------------------------------------
//  Buffer Status Flags
//-------------------------------------------------------------------
#define BUF_FREE            0x00
#define BUF_F_LOCK          0x01
#define BUF_F_READY         0x02
#define BUF_F_ACTIVE        0x04
#define BUF_F_RXTRIGGER     0x08    // RX section only
#define BUF_F_TXRETRY       0x08    // TX section only

#define BUF_F_GBGCOL        0x10    // RX section only
#define BUF_F_TXINTTGT      0x20    // TX section only; transmitted only to internal target



//-------------------------------------------------------------------
//  Macros to set a request for MostService()
//-------------------------------------------------------------------
#ifdef MNS_OPT_1
#define MNS_REQUEST_SET(flags)      MnsPendingSet(flags);
#define MNS_REQUEST_CALL(flags)     MnsPendingSetAndCall(flags);
#else
#define MNS_REQUEST_SET(flags)
#define MNS_REQUEST_CALL(flags)
#endif


//-------------------------------------------------------------------
//  Macros to set a request for MostServiceAsync()
//-------------------------------------------------------------------
#if (defined ADS_TX_MIN) || (defined ADS_RX_MIN)
#ifdef MNS_OPT_1
#define MNS_REQUEST_ASYNC_SET(flags)        MnsPendingAsyncSet(flags);
#define MNS_REQUEST_ASYNC_CALL(flags)       MnsPendingAsyncSetAndCall(flags);
#else
#define MNS_REQUEST_ASYNC_SET(flags)
#define MNS_REQUEST_ASYNC_CALL(flags)
#endif
#endif



//-------------------------------------------------------------------
//  Interrupts, which are enabled after Reset
//
//-------------------------------------------------------------------
#ifndef INT_ENABLE_AFTER_RESET
 #define INT_ENABLE_AFTER_RESET     0xF     // all events are enabled
#endif


//----------------------------------------------------------------------
// MOST Supervisor
//------------------------------------
#if (defined MSV_MINIMUM) && (defined MSV_EXTENDED)
    #undef MSV_MINIMUM
#endif

#if (defined MSV_MINIMUM) && (defined SERVICE_LAYER_II)
	#error Minimum Supervisor does not support LayerII
#endif 

//----------------------------------------------------------------------
//  Secondary Node Solution
//----------------------------------------------------------------------
#ifdef SECONDARY_NODE

    #ifndef SECONDARY_NODE_INT_ENABLE   // define default value of secondary node IE register:
      #ifdef SECONDARY_NODE_OPT_1       // if secondary node is in front of primary node
        #define SECONDARY_NODE_INT_ENABLE   0x05    // Enable Interrupts: IERR + IMRX
      #else
        #define SECONDARY_NODE_INT_ENABLE   0x01    // Enable Interrupt:  IMRX
      #endif
    #endif

    #undef  INTERFACE_CONFIG
    #define INTERFACE_CONFIG 0          // ignore definition of user, if secondary node scenario

    #ifdef MOST_TRANSCEIVER_ID
        #if (MOST_TRANSCEIVER_ID != 1) &&  (MOST_TRANSCEIVER_ID != 5) &&  (MOST_TRANSCEIVER_ID != 7)   // neither OS8104 nor OS8104A ?
            #undef MOST_TRANSCEIVER_ID
            #error Secondary Node Solution is only provided for OS8104 and OS8104A  --> Select correct MOST_TRANSCEIVER_ID
        #endif
    #endif

    #if (defined MSV_MINIMUM) || (defined MSV_VIRTUAL)
        #error Secondary Node Solution is only provided for Extended Supervisor
    #endif

#else
    #undef SECONDRY_NODE_OPT_1
#endif //SECONDARY_NODE



//----------------------------------------------------------------------
//  Prepare Switches for Control Port Communication
//  and Source Port Configuration
//----------------------------------------------------------------------
#if     (INTERFACE_CONFIG == 1)
        #define CP_I2C
        #define SP_SER
#endif
#if     (INTERFACE_CONFIG == 2)
        #define CP_SPI
        #define SP_SER
#endif
#if     (INTERFACE_CONFIG == 3)
        #define CP_I2C
        #define SP_PAR
#endif
#if     (INTERFACE_CONFIG == 4)
        #define CP_SPI
        #define SP_PAR
#endif
#if     (INTERFACE_CONFIG == 5)
        #define CP_I2C
        #define SP_PAR
        #define PADT
#endif
#if     (INTERFACE_CONFIG == 6)
        #define CP_SPI
        #define SP_PAR
        #define PADT
#endif
#if     (INTERFACE_CONFIG == 7)
        #define CP_PAR
        #define SP_PAR
#endif
#if     (INTERFACE_CONFIG == 8)
        #define CP_PAR
        #define SP_PAR
        #define PADT
#endif
#if     (INTERFACE_CONFIG == 9)
        #define CP_COM
        #define SP_SER
#endif

#ifdef RE_MSG_INTF
        #undef  SCS_NO_ADDR_CALC
        #undef  SP_SER
        #undef  SP_PAR
        #define SP_SERPAR
#endif


//----------------------------------------------------------------------
//  Prepare Switches to identify different types of MOST Transceiver
//      - OS8104        (active on OS8104, OS8104A)
//      - OS8104A       (active on OS8104A)
//      - OS8401        (active on OS8401, OS8801, OS8804, OS8805)
//      - OS8805        (active on OS8805)
//----------------------------------------------------------------------

#ifndef MOST_TRANSCEIVER_ID     // Prepare the default settings, if not specified by user:

    #if (INTERFACE_CONFIG == 0)                                 // Secondary Node Scenario
        #define OS8104
    #endif

    #if ((INTERFACE_CONFIG >= 1) && (INTERFACE_CONFIG <= 8))    // Single Chip Solution with OS8104
        #define OS8104
    #endif

    #if (INTERFACE_CONFIG == 9)
        #define OS8401                                          // OS8401, OS8801, OS8804, OS8805
    #endif

#endif

#ifdef MOST_TRANSCEIVER_ID

    #if (MOST_TRANSCEIVER_ID == 1)  // OS8104
        #define OS8104
    #endif

    #if (MOST_TRANSCEIVER_ID == 2)  // OS8401 or OS8801
        #define OS8401
    #endif

    #if (MOST_TRANSCEIVER_ID == 3)  // OS8804
        #define OS8401              //        inherit OS8401
    #endif

    #if (MOST_TRANSCEIVER_ID == 4)  // OS8805
        #define OS8401              //        inherit OS8401


        #ifndef OS8805        
          #define OS8805            //        plus new features of OS8805
        #endif
    #endif

    #if (MOST_TRANSCEIVER_ID == 5)  // OS8104A
        #define OS8104              //        inherit OS8104
        #define OS8104A             //        plus new features of OS8104A
    #endif


    #if (MOST_TRANSCEIVER_ID == 7)  // OS8104A_DYN
        #define OS8104              //        inherit OS8104
        #define OS8104A             //        plus new features of OS8104A
        #define OS8104A_DYN         //        plus dynamic distinction
    #endif


#endif

//----------------------------------------------------------------------
// Plausibility check:  MOST_TRANSCEIVER_ID vs. INTERFACE_CONFIG
//----------------------------------------------------------------------

#ifdef OS8104   // 8104, 8104A:
        #if (INTERFACE_CONFIG == 9)
            #error OS8104 and OS8104A do not support INTERFACE_CONFIG 9
        #endif
#endif

#ifdef OS8401   // OS8401, OS8801, OS8804, OS8805:
        #if (INTERFACE_CONFIG != 9)
            #error OS8401, OS8801, OS8804, and OS8805 do only support INTERFACE_CONFIG 9
        #endif
#endif








//----------------------------------------------------------------------
// RingBreakDiagnosis Timeout Values:
//----------------------------------------------------------------------
// The default diagnosis base timeout value (30sec.) can be modified by
// an optional macro MSV_TIMEOUT_DIAGNOSIS that can be defined in
// adjust.h if desired by the system integrator.
// For the alternative diagnosis mode (MSV_DIAG_EXT) this value is just
// fixed to 3900 ms currently.
// ==> RBD base timeout values:   MSV_TIMEOUT_DIAGNOSIS and MSV_TIMEOUT_DIAGNOSIS_DELTA

#ifndef MSV_DIAG_EXT    // Standard Ringbreak Diagnosis:
    #define MSV_TIMEOUT_DIAGNOSIS_DEF   30000   // Default value: 30 sec.  (=TIME_D_SLAVE)
    #define MSV_TIMEOUT_DIAGNOSIS_MIN   4000    // Min value: 4 sec.
    #define MSV_TIMEOUT_DIAGNOSIS_MAX   60000   // Max value: 60 sec.
    #define MSV_TIMEOUT_DIAGNOSIS_DELTA 2000    // Delta between TIME_D_SLAVE and
#endif

#ifdef MSV_DIAG_EXT     // Alternative Diagnosis Mode:
    #define MSV_TIMEOUT_DIAGNOSIS_DEF   3900    // Defaultvalue = 3900 ms  (=TIME_D_SLAVE
    #define MSV_TIMEOUT_DIAGNOSIS_MIN   3900    // Min value: 3.9 sec.
    #define MSV_TIMEOUT_DIAGNOSIS_MAX   3900    // Max value: 3.9 sec.
    #define MSV_TIMEOUT_DIAGNOSIS_DELTA 1900    // Delta between TIME_D_SLAVE and TIME_D_MASTER
#endif

// Check for correct value of MSV_TIMEOUT_DIAGNOSIS:
#ifndef MSV_TIMEOUT_DIAGNOSIS
    #define MSV_TIMEOUT_DIAGNOSIS  MSV_TIMEOUT_DIAGNOSIS_DEF    // Default value: 30 sec.
#else
  #if (MSV_TIMEOUT_DIAGNOSIS < MSV_TIMEOUT_DIAGNOSIS_MIN)       // Check valid range
    #undef MSV_TIMEOUT_DIAGNOSIS
    #define MSV_TIMEOUT_DIAGNOSIS  MSV_TIMEOUT_DIAGNOSIS_MIN
  #endif
  #if (MSV_TIMEOUT_DIAGNOSIS > MSV_TIMEOUT_DIAGNOSIS_MAX)
    #undef MSV_TIMEOUT_DIAGNOSIS
    #define MSV_TIMEOUT_DIAGNOSIS  MSV_TIMEOUT_DIAGNOSIS_MAX
  #endif
#endif





//----------------------------------------------------------------------
// MOST Supervisor Timeout Values:
//----------------------------------------------------------------------
#define TIME_LOCK           (  100/TIMER_INT_DIV)           // timeout value in milliseconds
#define TIME_D_LOCK         (  250/TIMER_INT_DIV)           //
#define TIME_OFF            (  300/TIMER_INT_DIV)           //
#define TIME_SLAVE          ( 2000/TIMER_INT_DIV)           //
#define TIME_MASTER         ( 2000/TIMER_INT_DIV)           //
#define TIME_NCE_DELAY      (  100/TIMER_INT_DIV)           //
#define TIME_D_SLAVE        ( (MSV_TIMEOUT_DIAGNOSIS) / TIMER_INT_DIV)
#define TIME_D_MASTER       ( ((MSV_TIMEOUT_DIAGNOSIS) - (MSV_TIMEOUT_DIAGNOSIS_DELTA)) / TIMER_INT_DIV)
#define TIME_D_DELTA        ( (MSV_TIMEOUT_DIAGNOSIS_DELTA) / TIMER_INT_DIV)





#ifdef MSV_RBD_OPT1  // switch just used for downward compatibility (no longer needed in adjust.h)
    #ifndef MSV_TIME_D_RESTART
        #define MSV_TIME_D_RESTART  5000    // Defaultvalue: 5 sec.
    #endif
#else
    #ifndef MSV_TIME_D_RESTART
        #define MSV_TIME_D_RESTART  0       // Defaultvalue: 0 sec.
    #endif
#endif  

#define TIME_D_RESTART      (  (MSV_TIME_D_RESTART) /TIMER_INT_DIV)


#define TIME_PASP_ACCESS ( 50/TIMER_INT_DIV)            //

#if (TIMER_INT_DIV == 25)
    #define TIME_UNLOCK 3                               // 75 ms
#else
 #if (TIMER_INT_DIV == 10)
    #define TIME_UNLOCK 7                               // 70 ms
 #else
    #define TIME_UNLOCK 70                              // 70 ms
 #endif
#endif

#if (TIMER_INT_DIV == 25)
    #define TIME_NPR_VALID 3                            // 75 ms
#else
 #if (TIMER_INT_DIV == 10)
    #define TIME_NPR_VALID 7                            // 70 ms
 #else
    #define TIME_NPR_VALID 70                           // 70 ms
 #endif
#endif


//--------------------------------------------
// Zero Power
//--------------------------------------------
#ifdef TIME_ZEROPOWER_DELAY         // Time in ms
    #define TIME_ZP_DEL (TIME_ZEROPOWER_DELAY /TIMER_INT_DIV)
#else
    #define TIME_ZP_DEL  (150/TIMER_INT_DIV)
#endif




//-------------------------------------------------------------------
//  Circumference of Net Services
//  (will be prepared automatically after adjusting the NetServices)
//-------------------------------------------------------------------


// MOST Net Service Kernel
//------------------------------------
#ifdef MNS_MIN                                          // minimum requirement of MOST NetServices
        #define MNS_0
        #define MNS_1
        #define MNS_50

        #ifdef TIMER_INT_OPT
         #define MNS_4          // MnsTimerIntDiff()
        #else
         #define MNS_2          // MnsTimerInt()
        #endif

        #if (defined ADS_TX_MIN) || (defined ADS_RX_MIN)    // only needed if ADS is implemented
         #define MNS_3
        #endif

        #ifdef MSV_EXTENDED                                 // software timer of Extended Supervisor
         #define MNS_21
         #define MNS_22
         #define MNS_23
         #define MNS_27
         #define MNS_28
         #define MNS_30

		#ifdef MSV_ADD10
		  #ifdef OS8805
			#define MNS_32                               	// Rx/Crystal/Rx switch Timer (PLL limiting)
		  #endif
		#endif

        #endif


        #ifdef MSV_MINIMUM
         #define MNS_23
        #endif

        #ifdef AMS_RX_MIN                                   // software timer of AMS
        #ifndef AMS_RX_NOSEG
         #define MNS_24
        #endif
        #endif


        #ifdef AMS_TX_ADD9                                  // mid level retry timer
          #define MNS_29

          #if (TIME_MSG_TX_RETRY > TIMER_INT_DIV)
              #define TIME_MID_LEVEL_RETRY ((word)(TIME_MSG_TX_RETRY/TIMER_INT_DIV))
          #else
              #define TIME_MID_LEVEL_RETRY ((word)1)                        // avoid loading timer with value 0
          #endif

        #endif

        #ifdef CMS_TX_MIN
         #define MNS_25                                     // software timer of CMS Tx Section
        #endif

        #ifdef ADS_TX_MIN
         #define MNS_26                                     // software timer of ADS Tx Section

        #endif


#ifdef MNS_OPT_1                                        // Option 1: Request Flags
        #define MNS_10
        #define MNS_11
        #define MNS_40
        #define MNS_CB1
        #define MNS_CB3

     #if (defined ADS_TX_MIN) || (defined ADS_RX_MIN)   // only needed if ADS is implemented
         #define MNS_12
         #define MNS_13
         #define MNS_CB2
         #define MNS_CB4
     #endif
#endif

#ifdef MNS_OPT_3
    #ifdef TIMER_INT_OPT                                // Only available if using MostTimerIntDiff()
        #define MNS_41                                  // Function to get lowest timer value
    #endif
#endif


#ifdef MNS_MSG_INTF                                     // Message based interface to HW layer
    #define V_MNS_60
    #define V_MNS_61
#endif

#if (MNS_DBG_LEVEL > 0)
    #define MNS_CB10                                    // callback to present the debug messages
#endif

#endif  // MNS_MIN



// Control Message Service Tx Section
//------------------------------------
#ifdef CMS_TX_MIN                                       // minimum requirement
        #define CMS_0
        #define CMS_T1
        #define CMS_T2
        #define CMS_T3
        #define CMS_T4
        #define CMS_T5
        #define CMS_T10
        #define CMS_T13
        #define CMS_T14
        #define CMS_CB1

#ifdef CMS_MSG_INTF         // Message based interface
        #define V_CMS_T6
        #define V_CMS_T7
        #define V_CMS_T20
        #define V_CMS_T21
#else                       // Register based interface
        #define CMS_T6
        #define CMS_T7
        #define CMS_T20
        #define CMS_T21
#endif // CMS_MSG_INTF


#ifdef CMS_TX_ADD2
        #define CMS_T8                                  // single call transmission
#endif

#ifdef CMS_TX_ADD3
        #define CMS_T9                                  // checking tx buffer
#endif

#ifdef CMS_TX_ADD4
        #define CMS_T11                                 // useful optional functions
        #define CMS_T12
#endif

#ifdef CMS_TX_ADD5
        #define CMS_CB5                                 // CMS tx filter
#endif

#ifdef CMS_TX_ADD7
      #ifdef OS8104A    
        #define CMS_T15                                 // set short retry time (OS8104A) 
        #define CMS_T16                                 // set number of long retries (OS8104A)
        #define CMS_T17                                 // get short retry time (OS8104A)
        #define CMS_T18                                 // get number of long retries (OS8104A)
      #endif
#endif

#endif  // CMS_TX_MIN



// Control Message Service Rx Section
//------------------------------------
#ifdef CMS_RX_MIN                                       // minimum requirement
        #define CMS_0
        #define CMS_1
        #define CMS_R3
        #define CMS_R6
        #define CMS_R7
        #if (MAX_EXT_DATA > 0) || (defined CTRL_RX_FAKE)
            #define CMS_CB2
        #endif
        #define CMS_CB3

#ifdef CMS_MSG_INTF         // Message based interface
        #define V_CMS_R5
#else                       // Register based interface
        #define CMS_R5
#endif

#ifdef CMS_RX_ADD1                                      // message polling available
        #ifndef AMS_RX_MIN
            #define CMS_R1
            #define CMS_R2
        #endif
#endif

#ifdef CMS_RX_ADD2                                      // copy message to local memory
        #define CMS_R4
#endif

#ifdef CMS_RX_ADD3                                      // possibility to use addition receive input
        #define CMS_R30
        #define CMS_R32
#endif

#ifdef CMS_RX_ADD4                                      // additional rx filter
        #define CMS_CB4
#endif

#endif // CMS_RX_MIN



// Appl. Message Service Tx Section
//------------------------------------
#ifdef AMS_TX_MIN                                       // minimum requirement
        #define AMS_0
        #define AMS_T1
        #define AMS_T3
        #define AMS_T4
        #define AMS_T5
        #define AMS_T6
        #define AMS_T7
        #define AMS_T8
        #define AMS_T9
        #define AMS_T10
        #define AMS_T11
        #define AMS_T12
        #define AMS_T14
        #define AMS_T51
        #define AMS_CB1

        #ifdef NS_AMS_AH                                // interface to AH of Layer II
            #define AMS_T13
        #endif

#ifdef AMS_TX_ADD1                                      // checking tx buffer
        #define AMS_T2
#endif


#ifdef AMS_TX_ADD3                                      // encoding data field
        #define AMS_T20
        #define AMS_T21
        #define AMS_T22
        #define AMS_T23
#endif

#ifdef AMS_TX_ADD4                                      // sending large messages via handshake
        #define AMS_CB4
#endif

#ifdef AMS_TX_ADD5                                      // possibility to filter tx messages
        #define AMS_CB5
        #define AMS_T16
#endif

#ifdef AMS_TX_ADD6                                      // possibility to transmit and receive internal messages
        #define AMS_T16
        #define AMS_T30
        #define AMS_R30
        #define AMS_R32
        #define MCS_14                                      // shadowing of own address registers
#endif

#ifdef AMS_TX_ADD7                                      // possibility to postpone a failed transmission
        #define AMS_T40
#endif

#ifdef AMS_TX_ADD8                                      // possibility to use an external data field
    #define AMS_T50
#endif

#ifdef AMS_TX_ADD9
    #ifdef DEF_MID_LEVEL_RETRIES
        #if (DEF_MID_LEVEL_RETRIES > 255)
            #undef DEF_MID_LEVEL_RETRIES
            #define DEF_MID_LEVEL_RETRIES 255
        #endif
    #else
        #define DEF_MID_LEVEL_RETRIES 0
    #endif

    #ifdef DEF_MID_LEVEL_RETRIES_INT_PROC
        #if (DEF_MID_LEVEL_RETRIES_INT_PROC > 255)
            #undef DEF_MID_LEVEL_RETRIES_INT_PROC 
            #define DEF_MID_LEVEL_RETRIES_INT_PROC 255
        #endif
    #else
        #define DEF_MID_LEVEL_RETRIES_INT_PROC 0
    #endif
#endif

#ifdef AMS_TX_BYPASS_FILTER                             // Bypass filter AMS TX -> CMS TX
        #define AMS_T15
        #define AMS_CB6
#endif

#endif  // AMS_TX_MIN



// Appl. Message Service Rx Section
//------------------------------------
#ifdef AMS_RX_MIN                                       // minimum requirement
        #define AMS_0
        #define AMS_R1
        #define AMS_R2
        #define AMS_R5
        #define AMS_R7
        #define AMS_R8
        #define AMS_R9
      #ifndef AMS_RX_NOSEG
        #define AMS_R10
      #endif
        #define AMS_R11
        #define AMS_R12
        #define AMS_CB2
        #define AMS_CB3

#ifdef AMS_RX_ADD1                                      // message polling available
        #define AMS_R3
        #define AMS_R4
#endif

#ifdef AMS_RX_ADD2                                      // copy message to local memory
        #define AMS_R6
#endif


#ifdef AMS_RX_ADD3                                      // decoding data field
        #define AMS_R20
        #define AMS_R21
        #define AMS_R22
#endif


#ifdef AMS_RX_ADD4                                      // possibility to receive internal messages
        #define AMS_R30
        #define AMS_R32
#endif

#ifdef AMS_RX_ADD5                                      // Receive Buffer Extension
        #define AMS_R40
        #define AMS_R41
        #define AMS_R42
        #define AMS_CB7
        #define AMS_CB8
#endif

#ifdef AMS_RX_ADD6                                      // request rx buffer status
        #define AMS_R13
#endif

#ifdef AMS_RX_ADD5_OPT1                                 // 
        #define AMS_R43
#endif

#endif  // AMS_RX_MIN



// Remote Control Service
//------------------------------------
#ifdef RCS_WRITE                                        // minimum requirement
        #define RCS_0                                   // for remote write
        #define RCS_1
        #define RCS_CB1
        #define CMS_T8
#endif

#ifdef RCS_READ                                         // minimum requirement
        #define RCS_0                                   // for remote read
        #define RCS_2
        #define RCS_5
        #define RCS_6
        #define RCS_CB2
        #define CMS_T8
#endif


// Synchron Channel Allocation Service
//------------------------------------
#ifdef SCS_SOURCE_ALLOC_MIN                             // allocate / deallocate minimum requirement
        #define SCS_0
        #define SCS_A1
        #define SCS_A2
        #define SCS_A6
        #define SCS_D1
        #define SCS_D2
        #define SCS_D6
        #define SCS_CB1
        #define SCS_CB2
        #define CMS_T8
#endif

#ifdef  SCS_SOURCE_RE_MIN                               // connect / disconnect minimum requirement
        #define SCS_R1
        #define SCS_R2
        #define SCS_R3
        #define SCS_R8
#endif

#ifdef SCS_SOURCE_ADD1                                  // single call functions for allocation and routing
        #define SCS_A5
        #define SCS_D3
#endif

#ifdef SCS_SOURCE_ADD2                                  // Source Port features of OS8104A
    #ifdef OS8104A
        #define SCS_A7
        #define SCS_A8
        #define SCS_R9
        #define SCS_R10
    #endif
#endif

#ifdef SCS_SINK_RE_MIN                                  // connect / disconnect minimum requirement
        #define SCS_R4
        #define SCS_R5
        #define SCS_R6
#endif

#ifdef SCS_ADD1                                         // useful optional functions on source and destination side
        #ifndef NO_DIRECT_REG_ACCESS
        #define SCS_R7  // SyncFindChannel() is not available if NO_DIRECT_REG_ACCESS is used
        #endif
#endif

#ifdef SCS_ADD2
        #define SCS_G1
        #define SCS_G2
        #define SCS_G3
        #define SCS_CB3
#endif

#ifdef SCS_ADD3
		#define SCS_B1
		#define SCS_CB4
#endif


#if ( (defined SCS_SOURCE_ALLOC_MIN) || (defined SCS_SOURCE_RE_MIN) || (defined SCS_SOURCE_ADD1) || (defined SCS_SINK_RE_MIN) )
#if ( (defined OS8401) && (!defined SCS_NO_ADDR_CALC) )
        #error SCS_NO_ADDR_CALC must be defined, when using the SCS on OS8401, OS8801, OS8804, or OS8805
#endif
#endif

#ifdef RE_MSG_INTF
    #undef SCS_R1
    #undef SCS_R4
#endif

// Transparent Channel Allocation Service
//----------------------------------------
#if (defined TCS_SOURCE_ALLOC) || (defined TCS_SOURCE_RE) || (defined TCS_SOURCE_ALLOC_RE) || (defined TCS_SINK_RE)
#ifndef SP_SER
        #error Transparent Channel Allocation Service (TCS) is only available on serial source ports.
#endif
#ifndef OS8104
        #error Transparent Allocation Service (TCS) is only available on OS8104 transceiver.
#endif
#endif

#ifdef TCS_SOURCE_ALLOC                                 // allocate / deallocate transparent data channels
        #define TCS_0
        #define TCS_1
        #define TCS_A1
        #define TCS_D1
#endif


#ifdef TCS_SOURCE_RE                                    // connect / disconnect transparent data channels
        #define TCS_0
        #define TCS_1
        #define TCS_R2
        #define TCS_R3
#endif

#ifdef TCS_SOURCE_ALLOC_RE                              // single call functions for transparent data channels
        #define TCS_0                                   // (TCS_SOURCE_ALLOC and TCS_SOURCE_RE are NOT required !!!)
        #define TCS_1
        #define TCS_A5
        #define TCS_D3
#endif

#ifdef TCS_SINK_RE                                      // connect / disconnect transparent data channels
        #define TCS_0
        #define TCS_1
        #define TCS_R5
        #define TCS_R6
#endif


// MOST Supervisor
//------------------------------------


#if (defined MSV_ADD3) && (defined OS8104A) && (!defined OS8104A_DYN)  
    #undef MSV_ADD3
#endif

#ifdef MSV_MINIMUM                                      // if Minimum Supervisor is selected
        #define MSV_M1
        #define MSV_M3
        #define MSV_M7
        #define MSV_M8
        #define MSV_M9
        #define MSV_M14
        #define MSV_M17
        #define MSV_M26
        #define MSV_CB1
        #define MSV_CB2
        #define MSV_CB5
        #define MSV_CB6
#endif

#ifdef MSV_EXTENDED                                     // if Extended Supervisor is selected
        #define MSV_E1
        #define MSV_E2
        #define MSV_E3
        #define MSV_E5
        #define MSV_E7
        #define MSV_E8
        #define MSV_E9
        #define MSV_E10
        #define MSV_E11
        #define MSV_E13
        #define MSV_E14
        #define MSV_E15
        #define MSV_E16
        #define MSV_E17
        #define MSV_E26
        #define MSV_CB1
        #define MSV_CB2
        #define MSV_CB3
        #define MSV_CB4
        #define MSV_CB5
        #define MSV_CB6
        #define MSV_CB7
        #define MSV_CB8
        #define MSV_CB9
        #define MSV_CB10
        #define MSV_CB11
        #define MSV_CB12
        #define MSV_CB13
        #define MSV_CB14
        #define MSV_CB15
        #define MSV_CB16
        #define MSV_CB17

    #ifdef MSV_ADD2
        #define MSV_CB20
        #define MSV_CB21
    #endif
    #ifdef MSV_ADD3
        #define MSV_E20
    #endif
    #ifdef MSV_ADD4
        #define MSV_E21 // MostStartUpExt()
    #endif
    #ifdef MSV_ADD5
        #define MSV_CB22
    #endif




    #ifdef MSV_ADD9
        #define MSV_E27
        #define MSV_E28
		#define MSV_E29
		#define MSV_CB28
    #endif

	#ifdef MSV_ADD10
	  #ifdef OS8805
    	#define PLL_LIMITING                               // Rx/Crystal/Rx switch Timer (PLL limiting)
	  #endif
	#endif






#endif  // MSV_EXTENDED

#ifdef MSV_VIRTUAL
        #define MSV_V1
        #define MSV_V2
        #define MSV_V3
        #define MSV_V5
        #define MSV_V14
        #define MSV_V17
        #define MSV_V21
        #define MSV_CB3
        #define MSV_CB4
        #define MSV_CB5
        #define MSV_CB6
        #define MSV_CB7
        #define MSV_CB8
        #define MSV_CB9
        #define MSV_CB10
        #define MSV_CB11
        #define MSV_CB12
        #define MSV_CB13
        #define MSV_CB14
        #define MSV_CB15
        #define MSV_CB16
        #define MSV_CB17
        #define MSV_CB22

    #ifdef MSV_ADD2
        #define MSV_CB20
    #endif




    #ifdef MSV_ADD9
        #define MSV_V27
        #define MSV_V28
        #define MSV_V29
		#define MSV_CB28
    #endif


#endif  // MSV_VIRTUAL


// MOST Supervisor Abstraction Layer
//------------------------------------
#if (!defined MSV_EXTENDED) && (!defined MSV_VIRTUAL)
    #undef MSVAL_MIN
#endif

#ifdef MSVAL_MIN
        #define MSVAL_0
        #define MSVAL_1
        #define MSVAL_CB1
        #define MSVAL_CB2
        #define MSVAL_CB3
        #define MSVAL_CB4
        #define MSVAL_CB5
#endif

// MOST Transceiver Control Service
//------------------------------------
#ifdef MCS_MIN                                          // minimum requirement of MOST Control Service
        #define MCS_0
        #define MCS_9
        #define MCS_15

#ifdef MCS_ADD1                                         // interrupt handling
        #define MCS_1
        #define MCS_2
#endif

#ifdef MCS_ADD2                                         // address setting and reading of position registers
        #define MCS_3
        #define MCS_4
        #define MCS_8
        #define MCS_12
#endif

#ifdef MCS_ADD3                                         // rmck frequency setting
        #define MCS_5
#endif

#ifdef MCS_ADD4                                         // mute/demute source data outputs
        #define MCS_6
        #define MCS_7
#endif

#ifdef MCS_ADD5                                         // Reading delay registers
  #ifndef NO_DIRECT_REG_ACCESS                          // These functions are not available
        #define MCS_10                                  // if using only the message based interface !
        #define MCS_11
  #endif
#endif

#ifdef MCS_ADD6                                         // access to alternative packet address
        #define MCS_13
#endif



#ifdef MCS_MASTER_ADD1                                  // pll inselect
        #define MCS_M1
#endif

#ifdef MCS_MASTER_ADD2                                  // bandwidth setting
        #define MCS_M2
#endif

#if (defined MCS_ADD7) && (defined SECONDARY_NODE)      // service receive buffer of secondary node and prepare error message
        #define MCS_20
        #define MCS_21
        #define MCS_22
#endif

#ifdef MCS_ADD9
        #define MCS_50
        #define MCS_CB50
#endif

#if (defined MCS_MSG_INTF) && (defined MNS_MSG_INTF)    // direct register access using the messages based
        #define MCS_30                                  // interface, which is provided by MNS_MSG_INTF.
        #define MCS_31                                  // It is a must to defined MNS_MSG_INTF also !
        #define MCS_32
        #define MCS_33
        #define MCS_CB30
        #define MCS_CB31
        #define MCS_CB32
        #define MCS_CB33
#else                                                   // MCS_ADD8 is ignored, if MCS_MSG_INTF is enabled (same API functions)
    #ifdef MCS_ADD8                                     // additional functions for register access
        #define MCS_40                                  // when using the register based hardware layer interface
        #define MCS_41
        #define MCS_42
        #define MCS_43
        #define MCS_CB40
        #define MCS_CB41
        #define MCS_CB42
        #define MCS_CB43
    #endif
#endif

#endif  // MCS_MIN



// Async Data Transmission Service
// Tx - Section
//------------------------------------
#ifdef ADS_TX_MIN                                       // minimum requirement of Tx section
        #define ADS_0
        #define ADS_1
        #define ADS_T1
        #define ADS_T2
        #define ADS_T5
        #define ADS_T6
        #define ADS_T7
        #define ADS_T10
        #define ADS_CB2

#ifdef ADS_TX_ADD1                                      // single call transmission
        #define ADS_T8
#endif

#ifdef ADS_TX_ADD2                                      // check tx buffer
        #define ADS_T9
#endif


#ifdef ADS_TX_ADD3                                      // tx complete event
        #define ADS_CB3
#endif

#endif  // ADS_TX_MIN



// Async Data Transmission Service
// Rx - Section
//------------------------------------
#ifdef ADS_RX_MIN                                       // minimum requirement of Rx section
        #define ADS_0
        #define ADS_1
        #define ADS_R3
        #define ADS_R5
        #define ADS_R6
        #define ADS_R7
        #define ADS_CB1
        #define ADS_CB2

#ifdef ADS_RX_ADD1                                      // packet polling available
        #define ADS_R1
        #define ADS_R2
#endif

#ifdef ADS_RX_ADD2                                      // copy packet to local memory
        #define ADS_R4
#endif

#ifdef ADS_RX_ADD3
        #define ADS_CB4                                 // external rx filter
#endif

#ifdef ADS_RX_ADD4
        #define ADS_CB5                                 // notify captured error events
#endif

#endif  // ADS_RX_MIN








//-------------------------------------------------------------------
//  Predefine MOST register values
//  depending on your choice in adjust.h
//-------------------------------------------------------------------

//-------------------------
//  Register: SDC1
//-------------------------
#ifdef OS8104
// Prepare Bit EDG
//-----------------
#if (EDG_SCK == 1)
    #define SDC1_EDG    EDG                         // rising edge
#else
    #define SDC1_EDG    0                           // falling edge
#endif

// Prepare Bit DEL
//-----------------
#if (DEL_FSY == 1)
    #define SDC1_DEL    DEL                         // delay enable
#else
    #define SDC1_DEL    0                           // no delay
#endif

// Prepare Bit POL
//-----------------
#if (POL_FSY == 1)
    #define SDC1_POL    POL                         // FSY = high indicates left sample
#else
    #define SDC1_POL    0                           // FSY = high indicates right sample
#endif

// Prepare Bit I/O
//-----------------
#if (IO_FSY == 1)
    #define SDC1_IO     IO                          // FSY / SCK are outputs
#else
    #define SDC1_IO     0                           // FSY / SCK are inputs
#endif

// Prepare Bit NBR
//-----------------
#if (SP_SER_CLOCKRATE == 48)
    #define SDC1_NBR    NBR                         // 48 Fs SCK clock rate
#else
    #define SDC1_NBR    0                           // unequal 48 Fs SCK clock rate
#endif

// Prepare Bit SPD
//-----------------
#if (SP_SER_MODE > 4)
    #define SDC1_SPD    SPD                         // S/PDIF port enable
#else
    #define SDC1_SPD    0                           // S/PDIF port disable
#endif

// Prepare Bit MT
//-----------------
#if (SOURCE_MUTE == 1)
    #define SDC1_MT     0                           // source output ports muted
#else
    #define SDC1_MT     MT                          // source output ports in normal operation
#endif

// Prepare Bit TCE
//-----------------
#ifndef TCH_DISABLE
    #define TCH_DISABLE 0
#endif
#if ( (SP_SER_MODE == 2) || (SP_SER_MODE == 6) ) && (TCH_DISABLE != 1)
    #define SDC1_TCE    0                           // transparent channel enable
#else
    #define SDC1_TCE    TCE                         // transparent channel disable
#endif


// combine all features
//----------------------
#define REG_SDC1        ( SDC1_EDG | SDC1_DEL | SDC1_POL | SDC1_IO | SDC1_NBR | SDC1_SPD | SDC1_MT | SDC1_TCE )

#endif // OS8104



//-------------------------
//  Register: SDC2
//-------------------------
// Prepare SCK rate
//-----------------
#if ( (SP_SER_CLOCKRATE==8) && ((SP_SER_MODE==1)||(SP_SER_MODE==2)||(SP_SER_MODE==5)||(SP_SER_MODE==6)||(SP_SER_MODE==8)) )
    #define SCK_RATE    0x00    // 8 Fs
#endif


#if ( (SP_SER_CLOCKRATE==16) && ((SP_SER_MODE==1)||(SP_SER_MODE==2)||(SP_SER_MODE==5)||(SP_SER_MODE==6)||(SP_SER_MODE==8)) )
    #define SCK_RATE    0x20    // 16 Fs
#endif

#if ( (SP_SER_CLOCKRATE==32) && ((SP_SER_MODE==1)||(SP_SER_MODE==2)||(SP_SER_MODE==5)||(SP_SER_MODE==6)||(SP_SER_MODE==8)) )
    #define SCK_RATE    0x40    // 32 Fs
#endif

#if ( (SP_SER_CLOCKRATE==48) && ((SP_SER_MODE==1)||(SP_SER_MODE==2)||(SP_SER_MODE==5)||(SP_SER_MODE==6)||(SP_SER_MODE==8)) )
    #define SCK_RATE    0x60    // 64 Fs
#endif

#if ( (SP_SER_CLOCKRATE==64) && ((SP_SER_MODE==1)||(SP_SER_MODE==2)||(SP_SER_MODE==5)||(SP_SER_MODE==6)||(SP_SER_MODE==8)) )
    #define SCK_RATE    0x60    // 64 Fs
#endif

#if ( (SP_SER_MODE==3)||(SP_SER_MODE==9) )
    #define SCK_RATE    0x80    // 128 Fs
#endif

#if (SP_SER_MODE==4)
    #define SCK_RATE    0xA0    // 256 Fs
#endif

#ifndef SCK_RATE
    #define SCK_RATE    0x60    // Default: 64 Fs
#endif

#ifdef SP_PAR
    #undef SCK_RATE
    #define SCK_RATE    0xA0    // If SourcePort is in parallel mode
#endif


// Prepare MFSY
//-----------------
#if (MFSY_ENABLE == 1)
    #define MFSY_BIT    0x10    // speed of FSY is increased (2Fs or 4Fs)
#else
    #define MFSY_BIT    0x00    // speed of FSY is not increased (1Fs)
#endif



// Prepare TC clock rate
//----------------------
#if ( (SP_SER_CLOCKRATE / TRANSPARENT_SAMPLE_RATE) == 1 )
    #define TCC_RATE    0x00    // SCK divided by 1
#endif

#if ( (SP_SER_CLOCKRATE / TRANSPARENT_SAMPLE_RATE) == 2 )
    #define TCC_RATE    0x04    // SCK divided by 2
#endif

#if ( (SP_SER_CLOCKRATE / TRANSPARENT_SAMPLE_RATE) == 4 )
    #define TCC_RATE    0x08    // SCK divided by 4
#endif

#if ( (SP_SER_CLOCKRATE / TRANSPARENT_SAMPLE_RATE) == 8 )
    #define TCC_RATE    0x0C    // SCK divided by 8
#endif

#ifndef TCC_RATE
    #define TCC_RATE    0x00    // Default: SCK divided by 1
#endif

// Prepare S/PDIF rate
//----------------------
#if (SP_SER_MODE > 6) && (defined OS8104A) && (!defined OS8104A_DYN)
    #error Source Port Mode not supported by OS8104A 
#endif

#if ( (SP_SER_MODE==5)||(SP_SER_MODE==6) )
    #define SPDIF_RATE  0x00    // 64 Fs
#endif

#if ( (SP_SER_MODE==8)||(SP_SER_MODE==9) )
    #define SPDIF_RATE  0x01    // 128 Fs
#endif

#if (SP_SER_MODE==10)
    #define SPDIF_RATE  0x02    // 256 Fs
#endif

#if ( (SP_SER_MODE==11)||(SP_SER_MODE==12) )
    #define SPDIF_RATE  0x03    // 512 Fs
#endif


#ifndef SPDIF_RATE
    #define SPDIF_RATE  0x00    // Default: 64 Fs
#endif


// combine the three clock rates
//-------------------------------
#define REG_SDC2    (SCK_RATE | MFSY_BIT | TCC_RATE | SPDIF_RATE)

#define SPDIF_RATE_MASK 0x03


//-------------------------
//  Register: SDC3
//-------------------------
// Prepare Bit SIO
//-----------------
#if (SP_SER_MODE==11)
    #define SDC3_SIO    SIO     // S/PDIF 8x input
#else
    #define SDC3_SIO    0       // S/PDIF 8x output
#endif

// Prepare Bit SPS
//-----------------
#if (SPDIF_SYNC == 1)
    #define SDC3_SPS    SPS     // generate independent S/PDIF output timing
#else
    #define SDC3_SPS    0       // synchronize S/PDIF output timing to input
#endif

// Prepare bit MTI
//-----------------
#if (defined OS8104A) && (defined SOURCEPORT_MUTE)
    #define SDC3_MTI    MTI     // all Source Data Input Ports are muted 
#else
    #define SDC3_MTI    0       // Source Data Input Ports are not muted
#endif

// Prepare bit SPEN
//-----------------
#if (defined OS8104A) && (defined SOURCEPORT_DISABLE)
    #define SDC3_SPEN   SPEN    // Source Data Port are disabled
#else
    #define SDC3_SPEN   0       // Source Data Port are enabled
#endif


// combine all features
//----------------------
#define REG_SDC3        ( SDC3_SIO | SDC3_SPS | SDC3_MTI | SDC3_SPEN)


//-------------------------
//  Register: XCR
//-------------------------
// Prepare Bit SBY
//-----------------
#if (SOURCE_BYPASS == 1)
    #define XCR_SBY     SBY     // Source data bypass enable
#else
    #define XCR_SBY     0       // Source data bypass disable
#endif

// Prepare Bit REN
//-----------------
#if (RMCK_ENABLE == 1)
    #define XCR_REN     0       // RMCK output enable
#else
    #define XCR_REN     REN     // RMCK output disable
#endif

// Prepare XCR Basic Value
//------------------------
#define REG_XCR_BASIC   ( LBO | XCR_SBY | XCR_REN ) // Bits MTR, OE, ABY will prepared by Supervisor State Machine



//-------------------------
//  Register: XSR
//-------------------------

// Prepare Bit MSL
//-----------------
#if (MASK_ERR_SPDIF == 1)
    #define XSR_MSL     MSL     // S/PDIF lock error ignored
#else
    #define XSR_MSL     0       // S/PDIF lock error captured
#endif

// Prepare Bit MXL
//-----------------
#if (MASK_ERR_TRANS == 1)
    #define XSR_MXL     MXL     // Transceiver lock error ignored
#else
    #define XSR_MXL     0       // Transceiver lock error captured
#endif

// Prepare Bit ME
//-----------------
#if (MASK_ERR_CODING == 1)
    #define XSR_ME      ME      // Coding error ignored
#else
    #define XSR_ME      0       // Coding error captured
#endif


// Combine all features
//----------------------
    #define REG_XSR         (XSR_MSL | XSR_MXL | XSR_ME)


//-------------------------
//  Register: XSR2
//-------------------------

#if  (defined OS8104A) && (!defined RX_FIFO_DISABLE)
    #define REG_XSR2    (DFE)
#else
    #define REG_XSR2    0x00
#endif
    

//-------------------------
//  Register: CM1
//-------------------------
// Prepare Bits MX1..0          // These bits are ignored by Supervisor, if device is initialized as Slave.
//---------------------         // In a Slave device the pll input is conected with the RX network receive input pin.
#if (PLL_INPUT == 1)
    #define CM1_MX      0x01    // S/PDIF
#elif (PLL_INPUT == 3)
    #define CM1_MX      0x03    // Bit clock (SCK)
#else
    #define CM1_MX      0x02    // default (Master): Chrystal
#endif

// Prepare Bits XTL1..0
//---------------------
#if (XTL_DIVIDER == 512)
    #if (defined OS8104A) && (!defined OS8104A_DYN)
    #error XTL_DIVIDER == 512 is not valid for OS8104A 
    #endif
    #define CM1_XTL     0x08    // 512 Fs
#elif (XTL_DIVIDER == 384)
    #define CM1_XTL     0x04    // 384 Fs
#else
    #define CM1_XTL     0x00    // default: 256 Fs
#endif

// Prepare Bits RD2..0
//---------------------
#if (RMCK_DIVIDER == 256)
    #define CM1_RD      0x10    // 256 Fs
#elif (RMCK_DIVIDER == 128)
    #define CM1_RD      0x20    // 128 Fs
#elif (RMCK_DIVIDER == 64)
    #define CM1_RD      0x30    // 64 Fs
#elif (RMCK_DIVIDER == 1536)
    #define CM1_RD      0x40    // 1536 Fs
#elif (RMCK_DIVIDER == 1024)
    #define CM1_RD      0x50    // 1024 Fs
#elif (RMCK_DIVIDER == 768)
    #define CM1_RD      0x60    // 768 Fs
#elif (RMCK_DIVIDER == 512)
    #define CM1_RD      0x70    // 512 Fs
#else
    #define CM1_RD      0x00    // default: 384 Fs
#endif


// Combine all features
//----------------------
#define REG_CM1         (CM1_MX | CM1_XTL | CM1_RD)




//-------------------------
//  Register: NC
//-------------------------
#ifdef OS8104A
// Prepare Bit EDG
//-----------------
#ifdef CMS_TX_ADD7
    #define NC_VRE      VRE                         // Variable Reries enabled
#else
    #define NC_VRE      0                           // Variable Reries disabled
#endif

#ifdef CHANNEL_MUTE_DISABLE
    #define NC_CME      0                           // Channel Mute disabled
#else
    #define NC_CME      CME                         // Channel Mute enabled
#endif

#ifdef ASYNCH_INIT_ENABLE
    #define NC_ARE      ARE
#else
    #define NC_ARE      0
#endif

// combine all features
//----------------------
#define REG_NC        (NC_ARE | NC_VRE | NC_CME)


#endif



//-------------------------
//  Register: SBC
//-------------------------
#if (SBC_DEFAULT > 0x05) && (SBC_DEFAULT < 0x10)
    #define REG_SBC     SBC_DEFAULT
#else
    #define REG_SBC     0x06
#endif
#define INT_ENABLE_INIT (0xF0|(INT_ENABLE_AFTER_RESET&0xF))



#ifdef OS8104A
//-------------------------
//  Register: CM3
//-------------------------

    #ifdef FREQ_REGULATOR_DISABLE
        #define CM3_FREN    0       // Frequency regulator feature disabled
        #define CM3_ASR     0       // don't care
        #define CM3_ACD     0       // don't care
    #else
        #define CM3_FREN    FREN    // Frequency regulator feature enabled
        #define CM3_ASR     ASR     // automatic switch to reference clock
        #define CM3_ACD     ACD     // crystal is disabled automatically after relock
    #endif

    #define REG_CM3  (CM3_FREN | CM3_ASR | CM3_ACD)

//-------------------------
//  Register: FRLO, FRHI
//-------------------------
    #if (!defined OS8104A_FRLO) || (OS8104A_FRLO > 0x80) 
        #define REG_FRLO 0x7A
    #else 
        #define REG_FRLO OS8104A_FRLO
    #endif


    #if (!defined OS8104A_FRHI) || (OS8104A_FRHI < 0x80) 
        #define REG_FRHI 0x86
    #else 
        #define REG_FRHI OS8104A_FRHI
    #endif

#endif  //OS8104A


//-------------------------------------------------------------------
//  OS8401 and derivatives only:
//  Predefine register values
//  depending on your choice in adjust.h
//-------------------------------------------------------------------

//-------------------------
//  Register: CMCS
//-------------------------

// Prepare Bits MX1..0          // These bits are ignored by Supervisor, if device is initialized as Slave.
//---------------------         // In a Slave device the pll input is conected with the RX network receive input pin.
#if   (PLL_INPUT == 1)
    #define CMCS_MX     0x0003  // S/PDIF
#elif (PLL_INPUT == 3)
    #define CMCS_MX     0x0001  // Bit clock (SCK)
#else
    #define CMCS_MX     0x0000  // default (Master): Chrystal
#endif


// Prepare Bits XTL1..0
//---------------------
#if   (XTL_DIVIDER == 512)
    #define CMCS_XTL    0x0000  // 512 Fs
#elif (XTL_DIVIDER == 384)
    #define CMCS_XTL    0x0004  // 384 Fs
#else
    #define CMCS_XTL    0x0008  // default: 256 Fs
#endif

// Prepare Bits RD2..0
//---------------------
#if   (RMCK_DIVIDER == 256)
    #define CMCS_RD     0x0010  // 256 Fs
#elif (RMCK_DIVIDER == 128)
    #define CMCS_RD     0x0020  // 128 Fs
#elif (RMCK_DIVIDER == 64)
    #define CMCS_RD     0x0030  // 64 Fs
#elif (RMCK_DIVIDER == 1536)
    #define CMCS_RD     0x0040  // 1536 Fs
#elif (RMCK_DIVIDER == 1024)
    #define CMCS_RD     0x0050  // 1024 Fs
#elif (RMCK_DIVIDER == 768)
    #define CMCS_RD     0x0060  // 768 Fs
#elif (RMCK_DIVIDER == 512)
    #define CMCS_RD     0x0070  // 512 Fs
#else
    #define CMCS_RD     0x0000  // default: 384 Fs
#endif

// Prepare Bit /REN
//---------------------
#if (RMCK_ENABLE == 1)
    #define CMCS_RDIS   0x0000  // RMCK enable
#else
    #define CMCS_RDIS   0x0080  // RMCK disable (high impedance)
#endif


// Combine features     (RMCK divider, Crystal divider, PLL mux select, RMCK disable)
//----------------------
#define REG_CMCS_SAH    (CMCS_RD | CMCS_XTL | CMCS_MX | CMCS_RDIS)



//---------------------------
//  Register: SDC1 (OS8401)
//---------------------------
#ifdef OS8401
// Prepare Bit EDG
//-----------------
#if (EDG_SCK == 1)
    #define SDC1_EDG    EDG                         // rising edge
#else
    #define SDC1_EDG    0                           // falling edge
#endif

// Prepare Bit DEL
//-----------------
#if (DEL_FSY == 1)
    #define SDC1_DEL    DEL                         // delay enable
#else
    #define SDC1_DEL    0                           // no delay
#endif

// Prepare Bit POL
//-----------------
#if (POL_FSY == 1)
    #define SDC1_POL    POL                         // FSY = high indicates left sample
#else
    #define SDC1_POL    0                           // FSY = high indicates right sample
#endif

// Prepare Bit NBR
//-----------------
#if (SP_SER_CLOCKRATE == 48)
    #define SDC1_NBR 0x08                           // 48 Fs SCK clock rate
#elif (SP_SER_CLOCKRATE == 32)
    #define SDC1_NBR 0x10                           // 32 Fs SCK clock rate
#else
    #define SDC1_NBR 0                              // 64 Fs SCK clock rate (default)
#endif


// Prepare Bits SPD and I/O in EGPIO mode (OS8805)
//------------------------------------------------
#if (SP_DISABLE == 1) && (defined OS8805)
    #define SDC1_SPD        0x04                    // Enable EGPIO mode in OS8805, source ports are disabled
    #define SDC1_IO         0x02                    // Setting IO_FSY in adjust.h is don't care.
#else
// Prepare Bit SPD
//-----------------
    #if (SP_SER_MODE > 4)
        #define SDC1_SPD    0x04                    // S/PDIF port enable  (Bit2=1)
        #ifdef IO_FSY
         #undef  IO_FSY
         #define IO_FSY 0
        #endif
    #else
        #define SDC1_SPD    0                       // S/PDIF port disable (Bit2=0)
    #endif

// Prepare Bit I/O
//-----------------
    #if (IO_FSY == 1)
        #define SDC1_IO     0x02                    // FSY / SCK are outputs (Bit1=1)
    #else
        #define SDC1_IO     0                       // FSY / SCK are inputs  (Bit1=0)
    #endif
#endif // #if (SP_DISABLE == 1) && (defined OS8805)


// Prepare Bit MT
//-----------------
#if (SOURCE_MUTE == 1)
    #define SDC1_MT     0                           // source output ports muted
#else
    #define SDC1_MT     0x01                        // source output ports in normal operation
#endif



// combine all features
//----------------------
#define REG_SDC1        ( SDC1_EDG | SDC1_DEL | SDC1_POL | SDC1_NBR | SDC1_SPD | SDC1_IO | SDC1_MT )

#endif // OS8401




//-------------------------------------------------------------------
//  Define dummy macros, if not otherwise defined
//-------------------------------------------------------------------
#ifndef RES_MNSP_BEGIN
 #define RES_MNSP_BEGIN                     // Get Ressource:       MnsPending
#endif

#ifndef RES_MNSP_END
 #define RES_MNSP_END                       // Release Ressource:   MnsPending
#endif

#ifndef RES_MNSPA_BEGIN
 #define RES_MNSPA_BEGIN                    // Get Ressource:       MnsPendingAsync  (only needed if ADS implemented)
#endif

#ifndef RES_MNSPA_END
 #define RES_MNSPA_END                      // Release Ressource:   MnsPendingAsync  (only needed if ADS implemented)
#endif



//-------------------------------------------------------------------
//  Default Macros to reset the interrupt request:
//
//  Please note:
//  These default macros can be replaced by the user defined
//  macros, which are defined in adjust.h.
//  The macro has to clear the Interrupt flag in MOST Chip
//  and can be used to clear the request flag of the Operating
//  System if required.
//-------------------------------------------------------------------

#ifndef MOST_INT_RESET      // clear interrupt flags in the Message Control Register:
 #define MOST_INT_RESET(a)      MOST_WRITE(MSGC,(a))
#endif
//-------------------------------------------------------------------

#ifndef MOST_AINT_RESET     // clear interrupt flags in the Packet Control Register:
 #ifdef PADT
  #define MOST_AINT_RESET(a)    PAR_A_WRITE_SINGLE_BYTE(0,(byte)PCTC,(a))   // Using the Parallel Asynchronous Source Port Interface *)
 #else
  #define MOST_AINT_RESET(a)    MOST_WRITE(PCTC,(a))                        // Using the Controll Port Interface *)
 #endif
#endif
                            //
                            //      *) Please note:
                            //
                            //      If you want to define your own MOST_AINT_RESET macro in adjust.h,
                            //      you have to observe that the macro PADT cannot be accessed in adjust.h,
                            //      since it is defined at a later moment. But you simply have to know, what
                            //      interface must be used: the control port interface or the source port interface.

//-------------------------------------------------------------------

#ifndef MOST_INT_RESET_SEC  // clear interrupt flags in the Message Control Register of secondary node:
 #define MOST_INT_RESET_SEC(a)      MOST_WRITE_SEC(MSGC,(a))
#endif

//-------------------------------------------------------------------



#endif  // _MOSTDEF1_H

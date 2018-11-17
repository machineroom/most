/*
==============================================================================

Project:        MOST NetServices 
Module:         Header File of MOST NetServices API (Basic Layer)
File:           MostNS1.h
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
Date        By      Description
==============================================================================
*/


#ifndef _MOSTNS1_H
#define _MOSTNS1_H




#include "mostdef1.h"                           // General Definitions and Macros of MOST NetService API (Basic Layer)
#include "mostreg.h"                            // MOST Transceiver Register Definitions






#ifdef __cplusplus                  // All API functions are C-Functions
extern "C" {
#endif




/********************************************************************************************************************

                            GLOBAL DEFINITIONS OF MOST NETSERVICES (BASIC LAYER)

********************************************************************************************************************/




//-----------------------------------------------------------------//
//  Definitions of Extended MOST Supervisor                        //
//-----------------------------------------------------------------//

#if (defined MSV_EXTENDED) || (defined MSV_VIRTUAL)

/* states of MOST Supervisor state machine */
#define START_UP        0x01
#define ZERO_POWER      0x00
#define CONFIRM_M       0x02
#define CONFIRM_S       0x03
#define NET_ON          0x04
#define NET_OFF         0x05
#define VIRTUAL_ZERO    0x06


/* values for MostStartUp(byte Dev_Mode, byte Options) */

// Dev_Mode

#define MASTER          0x01
#define SLAVE           0x00


// Options
#define STATIC_MASTER   0x80
#define SET_BYPASS      0x40
#define ZERO_WAKEUP     0x20
#define ZERO_REQUEST    0x10
#define FAKE_OFF        0x08
#define SLAVE_WAKEUP    0x04
#define DIAGNOSIS       0x02
#define RESET           0x01


// Options2
// ... reserved


/* values for MostSetOffMode(byte Off_Mode) */
#define OFF_BY_REQUEST  0x00                    // NET_OFF only by application request
#define OFF_BY_LIGHT    0x01                    // NET_OFF by request and no light
#define OFF_BY_LOCK     0x02                    // NET_OFF by request, no light and no lock


/* error ids */
#define MSV_ERROR_1     0x80
#define MSV_ERROR_2     0x81
#define MSV_ERROR_3     0x86
#define MSV_ERROR_4     0x87
#define MSV_ERROR_5     0x83
#define MSV_ERROR_6     0x88
#define MSV_ERROR_7     0x89
#define MSV_ERROR_8     0x82
#define MSV_ERROR_9     0x84
#define MSV_ERROR_10    0x85
#define MSV_ERROR_11    0x8A
#define MSV_ERROR_12    0x8B
#define MSV_ERROR_13    0x8C
#define MSV_ERROR_14    0x8D
#define MSV_ERROR_15    0x8E

/* lock states */
#define LOCK                0x01
#define UN_LOCK             0x00
#define MSV_UNLOCK          0x00                // MostLock = FALSE, SysLock = TRUE
#define MSV_LOCK_SHORT      0x01                // MostLock = TRUE, but not yet stable
#define MSV_LOCK_STABLE     0x02                // MostLock = TRUE, stable
#define MSV_UNLOCK_CRITICAL 0x03                // SysLock  = FALSE (NetOff or critical unlock detected)


/* Diagnosis Info */
#define DIAG_OK             0xFF                // Diagnosis Info: no error detected
#define DIAG_MULTIMASTER    0xFE                // Diagnosis Info: diagnosis ok, but the device, which should be
                                                // configured as timing master is configured as
                                                // slave device, since there is another master in the network

#define DIAG_ALLSLAVE       0xFD                // Diagnosis Info: no timing master existing
#define DIAG_POOR           0xFC                // Diagnosis Info: fatal error, no relative position value available





#endif



//-----------------------------------------------------------------//
//  Definitions of Minimum MOST Supervisor                         //
//-----------------------------------------------------------------//
#ifdef MSV_MINIMUM

/* states of MOST Supervisor state machine */
#define INITIAL_STATE   0x00
#define START_UP        0x01
#define SWITCH_ON       0x10
#define NET_ON          0x04
#define NET_OFF         0x05
#define ZERO_POWER      INITIAL_STATE   // workaround since macro ZERO_POWER is used
                                        // by module MNS.C even if only the Minimum
                                        // Supervisor is implemented

/* values for MostStartUp(byte Dev_Mode, byte Options) */

// Dev_Mode
#define MASTER          0x01
#define SLAVE           0x00

// Options
#define ZERO_WAKEUP     0x20    //  ignored
#define ZERO_REQUEST    0x10    //  ignored
#define FAKE_OFF        0x08    //  ignored
#define SLAVE_WAKEUP    0x04    //  ignored
#define DIAGNOSIS       0x02    //  ignored
#define RESET           0x01    //  available

#endif



//-----------------------------------------------------------------//
//  Definitions of MOST NetService Kernel (MNS)                    //
//-----------------------------------------------------------------//

// Flags of MnsPending:
//----------------------
#define MNS_P_MSV_STATE     (word)0x0001        // MSV: State changed event
#define MNS_P_MSV_TIMEOUT   (word)0x0002        // MSV: Timeout event
#define MNS_P_MSV_VZERO     (word)0x0004        // MSV: Polling Net Activity required (state VIRTUAL_ZERO)
#define MNS_P_CMS_RXBUF_IN  (word)0x0008        // CMS: Rx Buffer overflow (ingoing)
#define MNS_P_CMS_RXBUF_OUT (word)0x0010        // CMS: Rx Buffer request re-trigger (outgoing)
#define MNS_P_AMS_RXBUF_OUT (word)0x0020        // AMS: Rx Buffer request re-trigger    (outgoing)
#define MNS_P_CMS_TX        (word)0x0040        // CMS: Tx Buffer not empty
#define MNS_P_AMS_TX        (word)0x0080        // AMS: Tx Buffer not empty
#define MNS_P_MSV_SBC       (word)0x0100        // MSV: Polling of SBC required or stable lock/light off while initialization
#define MNS_P_MSV_UNLOCK    (word)0x0200        // MSV: Polling required, since error interrupt disabled
#define MNS_P_CMS_TX_FIN    (word)0x0400        // CMS: Request to complete tx process
#define MNS_P_INFO_BUF      (word)0x0800        // MNS: MNS Control Interface (Info Msg) must be re-triggered
#define MNS_P_MSV_LIGHT     (word)0x1000        // MSV: Polling Net Activity required


// Define group of flags:
//------------------------
#if (defined MSV_EXTENDED || defined MSV_MINIMUM)
 #define MNS_P_MASK_MSV     ((MNS_P_MSV_STATE)|(MNS_P_MSV_TIMEOUT)|(MNS_P_MSV_VZERO)|(MNS_P_MSV_UNLOCK)|(MNS_P_MSV_SBC)|(MNS_P_MSV_LIGHT))
#else
 #define MNS_P_MASK_MSV     0
#endif

#ifdef CMS_TX_MIN
 #define MNS_P_MASK_CMS_TX  ((MNS_P_CMS_TX)|(MNS_P_CMS_TX_FIN))
#else
 #define MNS_P_MASK_CMS_TX  0
#endif

#ifdef CMS_RX_MIN
 #define MNS_P_MASK_CMS_RX  ((MNS_P_CMS_RXBUF_IN)|(MNS_P_CMS_RXBUF_OUT))
#else
 #define MNS_P_MASK_CMS_RX  0
#endif

#ifdef AMS_TX_MIN
 #define MNS_P_MASK_AMS_TX  (MNS_P_AMS_TX)
#else
 #define MNS_P_MASK_AMS_TX  0
#endif

#ifdef AMS_RX_MIN
 #define MNS_P_MASK_AMS_RX (MNS_P_AMS_RXBUF_OUT)
#else
 #define MNS_P_MASK_AMS_RX  0
#endif

#ifdef MNS_MSG_INTF
 #define MNS_P_MASK_MNS     (MNS_P_INFO_BUF)
#else
 #define MNS_P_MASK_MNS     0
#endif



// Mask of used flags (control section):
//---------------------------------------------
#define MNS_P_MASK_CS       (MNS_P_MASK_MSV | MNS_P_MASK_CMS_TX | MNS_P_MASK_CMS_RX | MNS_P_MASK_AMS_TX | MNS_P_MASK_AMS_RX | MNS_P_MASK_MNS)

// Mask of all flags, that are serviced only in case of NetOn:
//------------------------------------------------------------
#ifndef MNS_TX_NETOFF
 #define MNS_P_MASK_CS_NETON    (MNS_P_CMS_RXBUF_IN | MNS_P_MASK_CMS_TX | MNS_P_MASK_AMS_TX | MNS_P_CMS_TX_FIN)
#else
 #define MNS_P_MASK_CS_NETON    (MNS_P_CMS_RXBUF_IN)
#endif

// Mask of all flags, that are serviced only in case of state != ZERO_POWER
//-------------------------------------------------------------------------
 #define MNS_P_MASK_CS_NOTZERO  (MNS_P_CMS_RXBUF_IN | MNS_P_MASK_CMS_TX | MNS_P_MASK_AMS_TX | MNS_P_CMS_TX_FIN | MNS_P_CMS_RXBUF_OUT | MNS_P_AMS_RXBUF_OUT )





// Options for MostService:
//--------------------------
#define MNS_O_NO_CMSRX              0x0001      // don't distribute received messages (CMS RX section)
#define MNS_O_NO_AMSRX              0x0002      // don't interprete received AMS messages (AMS RX section)
#define MNS_O_NO_CMSTX              0x0004      // don't send control messages
#define MNS_O_NO_AMSTX              0x0008      // don't service AMS TX section
#define MNS_O_NO_SECRX              0x0010      // don't service RX buffer of secondary node
#define MNS_O_ALL                   0x0000      // service all pending requests
#define MNS_O_INT                   0x001F      // service only the interrupt requests


// Events for MostService:
//--------------------------
#define MNS_E_TIMER                 ((MNS_P_MSV_TIMEOUT)|(MNS_P_CMS_TX_FIN))// Event forced by MnsRequestTimer()
#define MNS_E_PEN                   0x8000                                  // Event forced by return value of MostService()
#define MNS_E_REQ                   0x4000                                  // Event forced by MnsRequest()
#define MNS_E_INT                   0x2000                                  // Event forced by MOST Interrupt (INT)
#define MNS_E_MASK                  MNS_E_TIMER // Mask for events, which must be captured by "MnsPending"




// Flags of MnsPendingAsync:
//---------------------------
#define MNS_P_ADS_RXBUF_IN          (byte)0x01        // ADS Rx Buffer overflow (ingoing)
#define MNS_P_ADS_RXBUF_OUT         (byte)0x02        // ADS Rx Buffer request re-trigger (outgoing)
#define MNS_P_ADS_TX                (byte)0x04        // ADS Tx Buffer not empty or timeout (tx pending)
#define MNS_P_ADS_PCTC              (byte)0x08        // ADS register PCTC pending
#define MNS_P_ADS_UNLOCK            (byte)0x10        // Retrigger required since unlock while being in state NET_ON

// Define group of flags:
//------------------------
#ifdef ADS_RX_MIN
 #define MNS_P_MASK_ADS_RX          ( MNS_P_ADS_RXBUF_IN | MNS_P_ADS_RXBUF_OUT )
#else
 #define MNS_P_MASK_ADS_RX          0
#endif

#ifdef ADS_TX_MIN
 #define MNS_P_MASK_ADS_TX          ( MNS_P_ADS_TX )
#else
 #define MNS_P_MASK_ADS_TX          0
#endif

#if (defined ADS_TX_MIN) || (defined ADS_RX_MIN)
 #define MNS_P_MASK_ADS_GEN         ( MNS_P_ADS_PCTC | MNS_P_ADS_UNLOCK)
#else
 #define MNS_P_MASK_ADS_GEN         0
#endif

// Mask of used flags (async section):
//---------------------------------------------
#define MNS_P_MASK_ADS              (MNS_P_MASK_ADS_RX | MNS_P_MASK_ADS_TX | MNS_P_MASK_ADS_GEN)



// Options for MostServiceAsync:
//-------------------------------
#define MNS_O_NO_ADSRX              0x01            // don't interprete received packets (ADS RX section)
#define MNS_O_NO_ADSTX              0x02            // don't transmit packets
#define MNS_O_ADS_ALL               0x00            // service all pending requests
#define MNS_O_ADS_INT               0x03            // service only the interrupt requests


// Events for MostServiceAsync:
//-------------------------------
#define MNSA_E_TIMER                MNS_P_ADS_TX    // Event forced by MnsRequestAsyncTimer()
#define MNSA_E_PEN                  0x80            // Event forced by return value of MostServiceAsync()
#define MNSA_E_REQ                  0x40            // Event forced by MnsRequestAsync()
#define MNSA_E_INT                  0x20            // Event forced by MOST Async Interrupt (AINT)
#define MNSA_E_MASK                 MNSA_E_TIMER    // Mask for events, which must be captured by "MnsPendingAsync"




//-----------------------------------------------------------------//
//  Command Message Interface between MNS and HW Layer             //
//-----------------------------------------------------------------//
#define MOSTCTRL_MSV_STARTUP                0x0100
#define MOSTCTRL_MSV_OFFMODE                0x0101
#define MOSTCTRL_MSV_SHUTDOWN               0x0102
#define MOSTCTRL_MSV_ZEROPOWER              0x0103
#define MOSTCTRL_MSV_STARTUP_EXT            0x0104
#define MOSTCTRL_MSV_DIAG_GET_RES           0x0105
#define MOSTCTRL_MSV_DIAG_SHUT_DOWN         0x0106
#define MOSTCTRL_MSV_COUNT_CODING_ERRORS    0x0107

#define MOSTCTRL_CMS_SETRBEMODE             0x0200
#define MOSTCTRL_CMS_SETRBENABLE            0x0201

#define MOSTCTRL_READ_CP                    0x0300
#define MOSTCTRL_WRITE_CP                   0x0301
#define MOSTCTRL_WRITE_BIT_CP               0x0302
#define MOSTCTRL_WRITE_BITFIELD_CP          0x0303

#define MOSTCTRL_MCS_FIND_CHANNELS          0x0310

#define MOSTCTRL_NBMIN_MODE                 0x0400

#define MOSTCTRL_SYNC_IN_CONNECT            0x0500
#define MOSTCTRL_SYNC_OUT_CONNECT           0x0501
#define MOSTCTRL_SYNC_OUT_DISCONNECT        0x0502
#define MOSTCTRL_SYNC_IN_DISCONNECT         0x0503
#define MOSTCTRL_SYNC_IN_MUTE               0x0504
#define MOSTCTRL_SYNC_MOVE_BOUNDARY         0x0505

//-----------------------------------------------------------------//
//  Info Message Interface between MNS and HW Layer                //
//-----------------------------------------------------------------//
#define MOSTINFO_MSV_STATE          0x0100
#define MOSTINFO_MSV_MPR            0x0101
#define MOSTINFO_MSV_MPR_DELAYED    0x0102
#define MOSTINFO_MSV_LOCK           0x0103
#define MOSTINFO_MSV_DIAG           0x0104
#define MOSTINFO_MSV_ERROR          0x0105
#define MOSTINFO_MSV_NPR            0x0106
#define MOSTINFO_MSV_MOSTREV        0x0107
#define MOSTINFO_MSV_RESET_INFO     0x0109
#define MOSTINFO_MSV_CODING_ERROR   0x0110

#define MOSTINFO_MSVAL_INFO         0x0180

#define MOSTINFO_MMSG_RESULT        0x0201
#define MOSTINFO_MSG_TX_RESULT      0x0202  // AMS TX Result
#define MOSTINFO_MH_TX_RESULT       0x0203  // MHP TX Result
#define MOSTINFO_MH_RX_CONSTATUS    0x0204  // MHP RX ConStatus
#define MOSTINFO_MH_TX_CONSTATUS    0x0205  // MHP TX ConStatus

#define MOSTINFO_READ_CP            0x0300
#define MOSTINFO_WRITE_CP           0x0301
#define MOSTINFO_WRITE_BIT_CP       0x0302
#define MOSTINFO_WRITE_BITFIELD_CP  0x0303

#define MOSTINFO_MCS_FIND_CHANNELS  0x0310

#define MOSTINFO_SYNC_MOVE_BOUNDARY 0x0505



//-----------------------------------------------------------------//
//  MOST Message Interface between Layer I and HW Layer            //
//-----------------------------------------------------------------//
#define MSG_HANDLE_CMS              0x0001
#define MSG_HANDLE_AMS              0x0004
#define MSG_HANDLE_SCS              0x0007
#define MSG_HANDLE_RCS              0x000A
#define MSG_HANDLE_MHP              0x000D


//-----------------------------------------------------------------//
//  Debug Interface                                                //
//-----------------------------------------------------------------//

#define MNS_DBG_BUF_SIZE    30  // maximum size of debug message

// Debug Level IDs:
//-----------------
#define MNS_DBG_LEVEL_MSV_1         (1<<0)
#define MNS_DBG_LEVEL_MSV_2         (1<<1)
#define MNS_DBG_LEVEL_AMS_TX_1      (1<<2)
#define MNS_DBG_LEVEL_AMS_RX_1      (1<<3)


// Dbg Msg IDs:
//--------------
#define MNS_DBG_MSV                 0x0100
#define MNS_DBG_AMS_RXBUF_STATUS    0x0200
#define MNS_DBG_AMS_RXBUF_NUM       0x0201




//-----------------------------------------------------------------//
//  Definitions of Control Message Service (CMS)                   //
//-----------------------------------------------------------------//

#ifdef CMS_TX_MIN

// Xmit Status
//------------------
#define XMIT_SUCCESS        (byte)0x10
#define XMIT_TYPEFAILED     (byte)0x11
#define XMIT_WRONGTARGET    (byte)0x00
#define XMIT_CRC            (byte)0x20
#define XMIT_BUF            (byte)0x21
#define XMIT_FIFO           (byte)0x40    // no free TX buffer in the driver layer
#define XMIT_TIMEOUT        (byte)0x80
//------------------

#endif




//-----------------------------------------------------------------//
//  Definitions of Application Message Service (AMS)               //
//-----------------------------------------------------------------//

#if (defined AMS_TX_ADD6) || (defined AMS_RX_ADD4)
#define MOST_TGT_INTERN     (word)0x0000                // target address of local FBlock
#define MOST_SRC_INTERN     (word)0x0000                // source address of local FBlock
#endif




#ifdef AMS_RX_MIN

// Possible errors while receiving a telegram
//-------------------------------------------
#define MSG_ERR_1   (byte)0x01    // > CMS            // missing the first telegram of a segmented message
#define MSG_ERR_2   (byte)0x02    // > CMS            // buffer overflow (bufferline not long enough)
#define MSG_ERR_3   (byte)0x03    // > CMS            // invalid telegram (corresponding bufferline not found)
                                                // since wrong blockcounter or wrong source address
#define MSG_ERR_4   (byte)0x04    // > CMS            // buffer overflow (no free bufferline)
#define MSG_ERR_5   (byte)0x85    // > AMS            // missing last telegram(s), garbage collector will clean the bufferline
#define MSG_ERR_6   (byte)0x06    // > CMS            // device is not able to receive a segmented message
#define MSG_ERR_7   (byte)0x87    // > AMS            // missing last telegram(s); message is cleared, since new message from
                                                // same source address has been received
#define MSG_ERR_8   (byte)0x88    // > AMS            // buffer overflow (report first part of message in AMS)

#define MSG_ERR_MASK_AMS    (byte)0x80                // mask that indicates AMS buffer pointer
//-------------------------------------------

#endif




//-----------------------------------------------------------------//
//  Definitions of Synchronous Channel Allocation Service (SCS)    //
//-----------------------------------------------------------------//

#ifdef SCS_SOURCE_ALLOC_MIN

// Answers used for callback function
// (XmitStatus | AllocAnswer)
//-----------------------------------------
#define ALLOC_XMIT_GRANT    0x11                // Xmit Success and Alloc Grant (all requested channels allocated)
#define ALLOC_XMIT_BUSY     0x12                // Xmit Success but master is busy (no channels allocated)
#define ALLOC_XMIT_DENY     0x13                // Xmit Success but there a not enough free channels (no channels allocated)
#define ALLOC_XMIT_CRC      0x20                // Xmit failed since a CRC error (no channels allocated)
#define ALLOC_XMIT_TIMEOUT  0x80                // Timeout error


// Answers used for callback function
// (XmitStatus | DeAllocAnswer)
//-----------------------------------------
#define DEALLOC_XMIT_GRANT   0x11               // Xmit Success and DeAlloc Grant (Deallocation successful)
#define DEALLOC_XMIT_BUSY    0x12               // Xmit Success but master is busy (Deallocation failed)
#define DEALLOC_XMIT_WRONG   0x14               // Xmit Success but the label had a value > 0x3B (Deallocation failed)
#define DEALLOC_XMIT_CRC     0x20               // Xmit failed since a CRC error (Deallocation failed)
#define DEALLOC_XMIT_TIMEOUT 0x80               // Timeout error


// return values for boundary change functions
//-----------------------------------------
#define BD_FAILED  0
#define BD_SUCCESS 1
#define BD_INIT    2

#endif




//-----------------------------------------------------------------//
//  Definitions of MOST Transceiver Control Service                //
//-----------------------------------------------------------------//

// values for: void MostSelectClockInput(byte pll_in)
#define     PLL_SPDIF       0x01                                        // Source: SR0 (S/PDIF)
#define     PLL_CRYSTAL     0x02                                        //         Crystal
#define     PLL_SCK         0x03                                        //         SCK

// values for: void MostSelectClockOutput(byte rmck_divide)

#define     RMCK_1536       0x40                                        // Frmck = 1536 * Fs
#define     RMCK_1024       0x50                                        //         1024 * Fs
#define     RMCK_768        0x60                                        //          768 * Fs
#define     RMCK_512        0x70                                        //          512 * Fs
#define     RMCK_384        0x00                                        //          384 * Fs
#define     RMCK_256        0x10                                        //          256 * Fs
#define     RMCK_128        0x20                                        //          128 * Fs
#define     RMCK_64         0x30                                        //           64 * Fs
#define     RMCK_OFF        0xFF                                        // disable RMCK output

#define     RMCK_MASK       0x70                                        // mask for RMCK bit in bCM2/cmcs register


// values for: void MostEnableInt(byte interrupts)
//             void MostDisableInt(byte interrupts)
#define     INT_ALC         0x08                                        // Interrupt on change of register MPR or MDR
#define     INT_ERR         0x04                                        // Interrupt on Error or Power-on after start-up
#define     INT_MTX         0x02                                        // Interrupt on Message transmitted
#define     INT_MRX         0x01                                        // Interrupt on Message received





//-----------------------------------------------------------------//
//  Definitions of MOST Event Combiner                             //
//-----------------------------------------------------------------//

#ifdef MSVAL_MIN

// State change events
#define MSVAL_S_OFF                 0x00
#define MSVAL_S_INIT                0x01
#define MSVAL_S_RBD                 0x02
#define MSVAL_S_ON                  0x03
#define MSVAL_S_BYPASS              0x04
#define MSVAL_S_ZERO_POWER          0x05


// Lock events
#define MSVAL_LS_UNLOCK             0x00
#define MSVAL_LS_LOCK               0x01
#define MSVAL_LS_LOCKSTABLE         0x02
#define MSVAL_LS_UNLOCKCRITICAL     0x03


// Events
#define MSVAL_E_UNLOCK              0x00
#define MSVAL_E_LOCKSTABLE          0x01
#define MSVAL_E_UNLOCK_CRITICAL     0x02
#define MSVAL_E_MPR                 0x10
#define MSVAL_E_MPRDEL_INC          0x11
#define MSVAL_E_MPRDEL_DEC          0x12
#define MSVAL_E_MPRDEL_EQUAL        0x13
#define MSVAL_E_NETON               0x20
#define MSVAL_E_SHUTDOWN            0x21
#define MSVAL_E_RESET               0x40
#define MSVAL_E_LIGHT_OFF           0x41
#define MSVAL_E_LIGHT_ON            0x42
#define MSVAL_E_BYPASS_CLOSED       0x43
#define MSVAL_E_BYPASS_OPEN         0x44
#define MSVAL_E_AUTOMUTE            0x50
#define MSVAL_E_AUTODEMUTE          0x51

// Errors
#define MSVAL_ERR_UNLOCK_SHORT      0x00
#define MSVAL_ERR_UNLOCK_CRITICAL   0x01
#define MSVAL_ERR_INIT_ERROR        0x04

// Diag Result
#define MSVAL_DIAG_OK               0x00
#define MSVAL_DIAG_POS              0x01
#define MSVAL_DIAG_FAILED           0x02
#define MSVAL_DIAG_LOCKLIGHT        0x03

// Diag parameters
#define MSVAL_D_MULTIMASTER         0x00
#define MSVAL_D_ALLSLAVE            0x01
#define MSVAL_D_POOR                0x02
#define MSVAL_D_NOLOCK              0x00
#define MSVAL_D_NOLIGHT             0x01


#endif




/********************************************************************************************************************

                            GLOBAL TYPE DECLARATIONS OF MOST NETSERVICES (BASIC LAYER)

********************************************************************************************************************/



//-----------------------------------------------------------------//
//  Type Declaration of MOSTNetServices Kernel (MNS)               //
//-----------------------------------------------------------------//
#if (defined MNS_MSG_INTF) || (defined MKC_MIN) || (defined MCS_ADD8)
#define MNS_CTRL_SIZE_DATA 32   // maximum length of a command or info message (length of data field)
typedef struct _TMnsCtrl
{
    word    Command;
    word    Flags;
    byte    NumBytes;
    byte    Data[MNS_CTRL_SIZE_DATA];
}TMnsCtrl, *pTMnsCtrl;
#endif

#ifdef MNS_OPT_3
typedef word TTimerDiff;    // Type of parameter of function MostTimerIntDiff()
#else
typedef byte TTimerDiff;
#endif

#if  (MNS_DBG_LEVEL > 0)
typedef struct _TMnsDbgMsg
{
    word    ID;
    word    Flags;
    word    Length;
    byte    Data[MNS_DBG_BUF_SIZE];
}TMnsDbgMsg, *pTMnsDbgMsg;
#endif



//-----------------------------------------------------------------//
//  Type Declaration of MOST Supervisor (MSV)                      //
//-----------------------------------------------------------------//
typedef struct _TMsvParam
{
    byte    mode;
    byte    opt;
    word    opt2;
    byte    sbc;
    byte    xsr2;
}TMsvParam, *pTMsvParam;


//-----------------------------------------------------------------//
//  Type Declaration of Control Message Service (CMS)              //
//-----------------------------------------------------------------//
#ifdef CMS_TX_MIN
typedef struct Ctrl_Tx_Type
{
    byte Status;
    byte Priority;
    byte MsgType;
    byte Tgt_Adr_H;
    byte Tgt_Adr_L;
    byte Data[17];
    byte Length;
    #if  (MAX_TX_HANDLE > 0)
    byte TxHandle[MAX_TX_HANDLE];
    #endif
    #ifdef CTRL_FILTER_ID
    byte Filter_ID;
    #endif
} TCtrlTx, *pTCtrlTx;
#endif


#ifdef CMS_RX_MIN
typedef struct Ctrl_Rx_Type
{
    byte Status;
    byte Rcv_Type;
    byte Src_Adr_H;
    byte Src_Adr_L;
    byte Data[17];
    #if  (MAX_EXT_DATA > 0)
    byte ExtData[MAX_EXT_DATA];
    #endif
    #ifdef CTRL_FILTER_ID
    byte Filter_ID;
    #endif
} TCtrlRx, *pTCtrlRx;
#endif



//-----------------------------------------------------------------//
//  Type Declaration of Application Message Service (AMS)          //
//-----------------------------------------------------------------//

typedef struct Most_Header_Type
{
    word Device_ID; // can be Scr_Adr or Tgt_Adr
    byte FBlock_ID;
    byte Inst_ID;
    word Func_ID;
    byte Operation;
    word Length;

} TMostHeader, *pTMostHeader;


#ifdef AMS_TX_MIN
typedef struct Msg_Tx_Type
{
    byte Status;
    byte Priority;
    word Tgt_Adr;
    byte FBlock_ID;
    byte Inst_ID;
    word Func_ID;
    byte Operation;
    word Length;
    byte Data[MAX_MSG_TX_DATA];
    #if  (MAX_TX_HANDLE > 0)
    byte TxHandle[MAX_TX_HANDLE];
    #endif
    #ifdef CTRL_FILTER_ID
    byte Filter_ID;
    #endif
    #ifdef AMS_TX_ADD8
    byte *PtrDataExt;
    #endif
    #ifdef AMS_TX_ADD9
    byte MidLevelRetries;
    #endif
} TMsgTx, *pTMsgTx;

typedef struct Msg_TxOpt_Type
{
    byte Priority;
    #if  (MAX_TX_HANDLE > 0)
    byte TxHandle[MAX_TX_HANDLE];
    #endif
    #ifdef CTRL_FILTER_ID
    byte Filter_ID;
    #endif
    #ifdef AMS_TX_ADD9
    byte MidLevelRetries;
    #endif
} TMsgTxOpt, *pTMsgTxOpt;

#endif  // AMS_TX_MIN


#ifdef AMS_RX_MIN
typedef struct Msg_Rx_Type
{
    byte Status;
    byte BlckCnt;
    byte UsageCnt;
    word Src_Adr;
    byte FBlock_ID;
    byte Inst_ID;
    word Func_ID;
    byte Operation;
    word Length;
    byte Data[MAX_MSG_RX_DATA];
    #if  (MAX_EXT_DATA > 0)
    byte ExtData[MAX_EXT_DATA];
    #endif
    #ifdef CTRL_FILTER_ID
    byte Filter_ID;
    #endif
    byte *DPtrIn;
    byte Rcv_Type;

    #ifdef AMS_RX_ADD5
    byte* PtrData;                              // pointer at data field
    word  SizeBuffer;                           // size of data field
    struct Msg_Rx_ExtBuf_Type* PtrNext;         // pointer at next field
    struct Msg_Rx_ExtBuf_Type* PtrCurBufExt;    // pointer at current buffer extension (Last extension)
    word CurSizeMsgBuf;                         // current size of buffer
    #endif

} TMsgRx, *pTMsgRx;

typedef struct Msg_RxOpt_Type
{
    #if  (MAX_EXT_DATA > 0)
    byte ExtData[MAX_EXT_DATA];
    #endif
    #ifdef CTRL_FILTER_ID
    byte Filter_ID;
    #endif
    byte Rcv_Type;

} TMsgRxOpt, *pTMsgRxOpt;

#endif // AMS_RX_MIN


#ifdef AMS_RX_ADD5
typedef struct Msg_Rx_ExtBuf_Type
{
    byte* PtrData;                          // pointer at data field
    word  SizeBuffer;                       // size of data field
    struct Msg_Rx_ExtBuf_Type* PtrNext;     // pointer at next field
    struct Msg_Rx_ExtBuf_Type* PtrPrev;     // pointer at previous field

} TMsgRxExtBuf, *pTMsgRxExtBuf;

#endif



//-----------------------------------------------------------------//
//  Type Declaration of Synchronous Channel Allocation Service     //
//-----------------------------------------------------------------//

#ifndef SCS_NO_ADDR_CALC

    #ifdef SP_SER
    typedef struct SrcData_Type                                            // Source Data ID in serial mode
    {
        byte Port;
        byte Byte;
    } TSrcData, *pTSrcData;
    #endif

    #ifdef SP_PAR
    typedef struct SrcData_Type                                            // Source Data ID in parallel mode
    {
        byte SF;
        byte Byte;
    } TSrcData, *pTSrcData;
    #endif

    #ifdef SP_SERPAR
    typedef struct SrcData_Type                                            // Source Data mode in mini kernel
    {
        byte Port_SF;
        byte Byte;
    } TSrcData, *pTSrcData;
    #endif

#else

    typedef struct SrcData_Type                                            // Address Reference is calculated
    {                                                                      // by application or before compilation
        byte AddrRef;
    } TSrcData, *pTSrcData;

#endif


//-----------------------------------------------------------------//
//  Type Declaration of Async. Data Transmission Service (ADS)     //
//-----------------------------------------------------------------//

#ifdef ADS_TX_MIN
typedef struct Data_Tx_Type
{
    byte Status;
    byte Priority;
    byte Tgt_Adr_H;
    byte Tgt_Adr_L;
    byte Data[48];
    byte Length;
    #if  (MAX_DATA_TX_HANDLE > 0)
    byte TxHandle[MAX_DATA_TX_HANDLE];
    #endif
} TDataTx, *pTDataTx;
#endif


#ifdef ADS_RX_MIN
typedef struct Data_Rx_Type
{
    byte Status;
    byte Tgt_Adr_H;
    byte Tgt_Adr_L;
    byte Src_Adr_H;
    byte Src_Adr_L;
    byte Data[48];
    #if (MAX_DATA_EXT_DATA > 0)
    byte ExtData[MAX_DATA_EXT_DATA];
    #endif
} TDataRx, *pTDataRx;
#endif






/********************************************************************************************************************

                            GLOBAL VARIABLES OF MOST NETSERVICES (BASIC LAYER)

********************************************************************************************************************/



//-----------------------------------------------------------------//
//  Global variables of Extended MOST Supervisor                   //
//-----------------------------------------------------------------//
#ifdef MSV_EXTENDED
extern byte Most_State;                                                 // current state of the state machine

extern byte Sys_Lock;                                                   // provides information about lock
extern byte Most_Lock;                                                  // provides information about lock
extern byte Most_StableLock;                                            // provides information about lock

extern byte Net_Activity;
extern byte Xsr_Reg;
extern byte Cm2_Reg;
extern byte Mpr_Reg;
extern byte Mpr_Reg_Old;
extern byte Npr_Reg;
#endif

//-----------------------------------------------------------------//
//  Global variables of Virtual MOST Supervisor                    //
//-----------------------------------------------------------------//
#ifdef MSV_VIRTUAL
extern byte Most_State;
extern byte Lock_State;
extern byte Mpr_Reg;
extern byte Npr_Reg;
//extern byte Mpr_Reg_Old;
#endif


//-----------------------------------------------------------------//
//  Global variables of Minimum MOST Supervisor                    //
//-----------------------------------------------------------------//
#ifdef MSV_MINIMUM
extern byte Most_State;                                                 // current state of the state machine
#endif


//-----------------------------------------------------------------//
//  Global variables of MOST Transceiver Control Service (MCS)     //
//-----------------------------------------------------------------//
#if (defined MCS_MIN) || (defined MSV_EXTENDED) || (defined MSV_MINIMUM)
extern byte Most_Revision[3];                                           // MOST Transceiver Revision Date: Day, Month, Year
#endif










/********************************************************************************************************************

                            API FUNCTIONS OF MOST NETSERVICES (BASIC LAYER)

********************************************************************************************************************/




//-----------------------------------------------------------------//
//  API Functions of MOST NetService Kernel (MNS)                  //
//-----------------------------------------------------------------//

#ifdef MNS_0                                                                // Init all modules of netservices
 void  InitNetServices(void);
#endif

#ifdef MNS_1                                                                // Trigger function for MOST NetServices
 word  MostService(word opt, word events);
#endif

#ifdef MNS_2                                                                // Has to be called one time in 1, 10 or 25ms
 void  MostTimerInt(void);                               // (adjustable in adjust.h)
#endif

#ifdef MNS_4
 void  MostTimerIntDiff(TTimerDiff diff);                        // Has to be called with time difference as parameter
#endif


#ifdef MNS_3
 byte  MostServiceAsync(byte opt, byte events);          // Trigger function for MOST NetServices Async Section
#endif

#ifdef MNS_41
 word  MostGetMinTimeout(void);
#endif





//-----------------------------------------------------------------//
//  API Functions of Extended MOST Supervisor                      //
//  and Virtual MOST Supervisor                                    //
//-----------------------------------------------------------------//

#if (defined MSV_E1) || (defined MSV_V1)                                    // init state machine
 void  MostStartUp(byte mode, byte options);
#endif

#if (defined MSV_E2) || (defined MSV_V2)                                    // set property off_mode
 void  MostSetOffMode(byte mode);
#endif

#if (defined MSV_E3) || (defined MSV_V3)                                    // switch net off
 void  MostShutDown( void );
#endif

#if (defined MSV_E5) || (defined MSV_V5)                                    // zero power request
 byte  MostZeroPower(byte mode, byte opt);
#endif

#if (defined MSV_E14) || (defined MSV_V14)                                  // get current state of MOST Supervisor
 byte  MostGetState(void);
#endif

#if (defined MSV_E17) || (defined MSV_V17)                                  // get current device mode of MOST Supervisor
 byte  MostGetDevMode(void);
#endif


#if (defined MSV_E21) || (defined MSV_V21)                                  // init state machine
 void  MostStartUpExt (pTMsvParam parms);
#endif

#if (defined MSV_E22) || (defined MSV_V22)                                  // switch net off
 byte  MostDiagnosisGetResult(void);
#endif

#if (defined MSV_E23) || (defined MSV_V23)                                  // get current state of MOST Supervisor
 void  MostDiagnosisShutDown(void);
#endif


#if (defined MSV_E27) || (defined MSV_V27)                                 // enable/disable counting of Coding Errors
 void  MostCountCodingErrors(bool switch_on);
#endif

#if (defined MSV_E28) || (defined MSV_V28)                                                              // get number of Coding Errors
 word  MostGetCodingErrors(void);
#endif


//-----------------------------------------------------------------//
//  Additional API Functions of Extended MOST Supervisor           //
//-----------------------------------------------------------------//

#ifdef MSV_E20
 void  MostPASPAccess(void);                             // enable supervision of unlocks after PASP access
#endif


//-----------------------------------------------------------------//
//  API Functions of Minimum MOST Supervisor                       //
//-----------------------------------------------------------------//

#ifdef MSV_M1                                                               // init state machine
 void  MostStartUp(byte mode, byte options);
#endif

#ifdef MSV_M3                                                               // switch net off
 void  MostShutDown( void );
#endif

#ifdef MSV_M14                                                              // get current state of MOST Supervisor
 byte  MostGetState(void);
#endif

#ifdef MSV_M17                                                              // get current device mode of MOST Supervisor
 byte  MostGetDevMode(void);
#endif






//-----------------------------------------------------------------//
//  API Functions of Control Message Service (CMS)                 //
//-----------------------------------------------------------------//

#ifdef CMS_0                                                                // Init the Control Message Service
 void  CtrlInit(void);
#endif

#ifdef CMS_T1                                                               // Get pointer to free tx buffer line
 struct Ctrl_Tx_Type*  CtrlGetTxPtr(void);
#endif

#ifdef CMS_T2                                                               // Mark buffer line as ready to send
 void  CtrlSend(struct Ctrl_Tx_Type *ptbuf);
#endif

#ifdef CMS_T3                                                               // Retry Tx Message
 void  CtrlTxRetry(void);
#endif

#ifdef CMS_T4                                                               // Cancel current job
 void  CtrlFreeTxMsg(void);
#endif

#ifdef CMS_T8                                                               // Transmit Ctrl Message (Single Call)
 byte  CtrlTransmit(byte priority, byte type, word tgt_adr, byte *ptbuf, byte length);
#endif

#ifdef CMS_T9                                                               // Get the number of free tx buffer lines
 byte  CtrlCheckTxBuffer(void);
#endif

#ifdef CMS_T10                                                              // Set the priority for xmit control message
 byte  CtrlSetTransPriority(byte priority);
#endif

#ifdef CMS_T11                                                              // Set the time between transmisson retries
 byte  CtrlSetTransTime(byte time);
#endif

#ifdef CMS_T12                                                              // Set the total transmission attempts
 byte  CtrlSetTransRetry(byte retry);
#endif

#ifdef CMS_T13                                                              // Get low level retry time
 byte  CtrlGetTransTime(void);
#endif

#ifdef CMS_T14                                                              // Get number of low level retries
 byte  CtrlGetTransRetry(void);
#endif


#ifdef CMS_T15                                                              // Set the time between short transmisson retries (OS8104A)
 byte  CtrlSetTransShortTime(byte short_time);
#endif

#ifdef CMS_T16                                                              // Set the number of long transmission attempts (OS8104A)
 byte  CtrlSetTransLongRetry(byte long_retry);
#endif

#ifdef CMS_T17                                                              // Get low level short retry time (OS8104A)
 byte  CtrlGetTransShortTime(void);
#endif

#ifdef CMS_T18                                                              // Get number of low level long retries (OS8104A)
 byte  CtrlGetTransLongRetry(void);
#endif



#ifdef CMS_R1                                                               // Get number to stored rx messages
 byte  CtrlGetMsgCnt(void);
#endif

#ifdef CMS_R2                                                               // Get pointer to next rx message
 struct Ctrl_Rx_Type*  CtrlGetRxPtr(void);
#endif

#ifdef CMS_R3                                                               // Free rx buffer line
 void  CtrlFreeRxMsg(struct Ctrl_Rx_Type *prbuf);
#endif

#ifdef CMS_R4                                                               // get rx message from rx buffer
 byte  CtrlReceive(struct Ctrl_Rx_Type *prbuf);
#endif

#ifdef CMS_R30
 pTCtrlRx  CtrlGetRxInPtr(void);
#endif

#ifdef CMS_R32
 void  CtrlRxInReady(pTCtrlRx rx_ptr);
#endif




//-----------------------------------------------------------------//
//  API Functions of Application Message Service (AMS)             //
//-----------------------------------------------------------------//

#ifdef AMS_0                                                                // Inits Application Message Service
 void  MsgInit(void);
#endif


#ifdef AMS_T1                                                               // Get ptr at next tx buffer entry
 struct Msg_Tx_Type*  MsgGetTxPtr(void);
#endif

#ifdef AMS_T13                                                              // Get ptr at reserved tx buffer entry
 struct Msg_Tx_Type*  MsgGetTxPtrRes(void);
#endif


#ifdef AMS_T2                                                               // Get number of free tx buffer lines
 byte  MsgCheckTxBuffer(void);
#endif

#ifdef AMS_T3                                                               // Mark buffer line as ready to send
 void  MsgSend(struct Msg_Tx_Type *ptbuf);
#endif
                                                                            // Send a Message after address was verified
#ifdef AMS_T14                                                              // (called by AMS and AH)
 void  MsgSend2(struct Msg_Tx_Type *ptbuf);
#endif


#ifdef AMS_T4                                                               // abort failed transmission
 void  MsgFreeTxMsg(void);
#endif

#ifdef AMS_T9                                                               // retry failed transmission
 void  MsgTxRetry(void);
#endif

#ifdef AMS_T12                                                              // Free unused buffer line
 void  MsgTxUnused(struct Msg_Tx_Type *Tx_Ptr);
#endif

#ifdef AMS_T20                                                              // Encoding type byte
 void  MsgTxDataByte(byte **pptr_tgt, byte *ptr_src);
#endif

#ifdef AMS_T21                                                              // Encoding type word
 void  MsgTxDataWord(byte **pptr_tgt, word *ptr_src);
#endif

#ifdef AMS_T22                                                              // Encoding type dword
 void  MsgTxDataLong(byte **pptr_tgt, dword *ptr_src);
#endif

#ifdef AMS_T23                                                              // Encoding BCD coded number
 void  MsgTxBcdToAscII(byte** pptr_tgt, byte number);
#endif

#ifdef AMS_T40
 void  MsgTxPostpone(void);
#endif

#ifdef AMS_T50
 byte  MsgTransmit(pTMostHeader pHeader, byte* pData , pTMsgTxOpt pOptions);
#endif



#ifdef AMS_R3                                                               // Get number to stored rx messages
 byte  MsgGetMsgCnt(void);
#endif

#ifdef AMS_R4                                                               // Get pointer to next rx message
 struct Msg_Rx_Type*  MsgGetRxPtr(void);
#endif

#ifdef AMS_R5                                                               // Free rx buffer line
 void  MsgFreeRxMsg(struct Msg_Rx_Type *prbuf);
#endif

#ifdef AMS_R6                                                               // get rx message from rx buffer
 byte  MsgReceive(struct Msg_Rx_Type *prbuf);
#endif

#ifdef AMS_R13
 word  MsgCheckRxBuffer(void);                           // get number of free RX buffers
#endif


#ifdef AMS_R20                                                              // Decoding type byte
 void  MsgRxDataByte(byte *ptr_tgt, byte **pptr_src);
#endif

#ifdef AMS_R21                                                              // Decoding type word
 void  MsgRxDataWord(word *ptr_tgt, byte **pptr_src);
#endif

#ifdef AMS_R22                                                              // Decoding type dword
 void  MsgRxDataLong(dword *ptr_tgt, byte **pptr_src);
#endif



#ifdef AMS_R30
 struct Msg_Rx_Type*  MsgGetRxInPtr(void);
#endif

#ifdef AMS_R32
 void  MsgRxInReady(struct Msg_Rx_Type *rx_in_ptr);
#endif


#ifdef AMS_R41
 byte  MsgRxAssemble(pTMsgRx rx_ptr, byte* pdata, word size);
#endif

#ifdef AMS_R42
 byte  MsgRxAssembleExt(pTMsgRx rx_ptr, byte* pData, word size_data,
                                           pTMostHeader pHeader, pTMsgRxOpt pOptions);
#endif

#ifdef AMS_R43                                                               
 void  MsgRxQuitMsg(void);
#endif



//-----------------------------------------------------------------//
//  API Functions of Remote Control Service (RCS)                  //
//-----------------------------------------------------------------//
#ifdef RCS_0                                                                // Inits the Remote Control Service
 void  RemoteInit(void);
#endif

#ifdef RCS_1                                                                // Send a remote write system message
 byte  RemoteWrite(word tgt_adr, byte map, byte *ptbuf, byte num_bytes);
#endif

#ifdef RCS_2                                                                // Send a remote read system message
 byte  RemoteRead(word tgt_adr, byte map, byte num_bytes, byte *prbuf);
#endif




//-----------------------------------------------------------------//
//  API Functions of Synchronous Channel Allocation Service (SCS)  //
//-----------------------------------------------------------------//

#ifdef SCS_0
 void  SyncInit(void);
#endif

#ifdef SCS_A1                                                               // Allocating only
 byte  SyncAllocOnly(byte num_channels, byte *prbuf);
#endif

#ifdef SCS_A5                                                               // Allocating and Connecting
 byte  SyncAlloc(byte num_channels, byte *prbuf, struct SrcData_Type *ptrsrc);
#endif

#ifdef SCS_A7                                                               // Enable Source Data Input Ports (OS8104A)
 byte  SyncSourcePortEnable(void);
#endif

#ifdef SCS_A8                                                               // Disable Source Data Input Ports (OS8104A)
 byte  SyncSourcePortDisable(void);
#endif

#ifdef SCS_D1                                                               // Deallocating only
 byte  SyncDeallocOnly(byte label);
#endif

#ifdef SCS_D3                                                               // Deallocating and Disconnecting
 byte  SyncDealloc(byte num_channels, byte *ptrch);
#endif

#ifdef SCS_G1                                                               // looking for source
 byte  SyncGetSource(byte channel);
#endif

#ifdef SCS_R2                                                               // Connecting (Source)
 void  SyncInConnect(byte num_channels, byte *ptrch, struct SrcData_Type *ptrsrc);
#endif

#ifdef SCS_R3                                                               // Disconnecting (Source)
 void  SyncInDisconnect(byte num_channels, byte *ptrch);
#endif

#ifdef SCS_R5                                                               // Connecting (Sink)
 void  SyncOutConnect(byte num_channels, byte *ptrch, struct SrcData_Type *ptrsrc);
#endif

#ifdef SCS_R6                                                               // Disconnecting (Sink)
 void  SyncOutDisconnect(byte num_channels, struct SrcData_Type *ptrsrc);
#endif

#ifdef SCS_R7                                                               // Detects channels by label
 byte  SyncFindChannels(byte num_channels, byte label, byte *pchrbuf);
#endif

#ifdef SCS_R8                                                               // Mute (Source)
 void  SyncInMute(byte num_channels, byte *ptrch);
#endif

#ifdef SCS_R9                                                               // Mute all Source Data Input Ports
 byte  SyncInMuteAll(void);
#endif

#ifdef SCS_R10                                                               // Demute all Source Data Input Ports
 byte  SyncInDemuteAll(void);
#endif

//-----------------------------------------------------------------//
//  API Functions of Transparent Channel Allocation Service        //
//-----------------------------------------------------------------//

#ifdef TCS_0                                                                // Init the Transp. Channel Allocation Service
 void  TransInit(void);
#endif

#ifdef TCS_1                                                                // Enables/Disables transparent mode
 void  TransEnable(byte mode);
#endif


#ifdef TCS_A1                                                               // Allocating only
 byte  TransAllocOnly(byte num_channels, byte *ptrch);
#endif

#ifdef TCS_A5                                                               // Allocating and Connecting
 byte  TransAlloc(byte num_channels, byte *ptrch, byte *ptrsrc);
#endif


#ifdef TCS_D1                                                               // Deallocating only
 byte  TransDeallocOnly(byte label);
#endif

#ifdef TCS_D3                                                               // Deallocating and Disconnecting
 byte  TransDealloc(byte num_channels, byte *ptrch);
#endif


#ifdef TCS_R2                                                               // Connecting (Source)
 void  TransInConnect(byte num_channels, byte *ptrch, byte *ptrsrc);
#endif

#ifdef TCS_R3                                                               // Disconnecting (Source)
 void  TransInDisconnect(byte num_channels, byte *ptrch);
#endif


#ifdef TCS_R5                                                               // Connecting (Sink)
 void  TransOutConnect(byte num_channels, byte *ptrch, byte *ptrsrc);
#endif

#ifdef TCS_R6                                                               // Disconnecting (Sink)
 void  TransOutDisconnect(byte num_channels, byte *ptrsrc);
#endif


//-----------------------------------------------------------------//
//  API Functions of Async Data Transmission Service               //
//-----------------------------------------------------------------//

#ifdef ADS_0                                                                // Init the Async Data Transmission Service
 void  DataInit(void);
#endif


#ifdef ADS_T1                                                               // Get pointer on free tx buffer line
 struct Data_Tx_Type*  DataGetTxPtr(void);
#endif

#ifdef ADS_T2                                                               // Mark buffer line as ready to send
 void  DataSend(struct Data_Tx_Type *ptbuf);
#endif

#ifdef ADS_T8                                                               // Transmit data packet (Single Call)
 byte  DataTransmit(byte priority, word tgt_adr, byte *ptbuf, byte length);
#endif

#ifdef ADS_T9                                                               // Get the number of free tx buffer lines
 byte  DataCheckTxBuffer(void);
#endif

#ifdef ADS_T10                                                              // Set the default priority
 byte  DataSetTransPriority(byte priority);
#endif


#ifdef ADS_R1                                                               // Get number of stored rx packets
 byte  DataGetMsgCnt(void);
#endif

#ifdef ADS_R2                                                               // Get pointer on next rx packet
 struct Data_Rx_Type*  DataGetRxPtr(void);
#endif

#ifdef ADS_R3                                                               // Free rx buffer line
 void  DataFreeRxMsg(struct Data_Rx_Type *prbuf);
#endif

#ifdef ADS_R4                                                               // get packet from rx buffer
 byte  DataReceive(struct Data_Rx_Type *prbuf);
#endif



//-----------------------------------------------------------------//
//  API Functions of MOST Transceiver Control Service (MCS)        //
//-----------------------------------------------------------------//

#ifdef MCS_0                                                                // Init Most Transceiver Control Service
 void  MostInit(void);
#endif

#ifdef MCS_1                                                                // Enable MOST Interrupts
 void  MostEnableInt(byte interrupts);
#endif

#ifdef MCS_2                                                                // Disable MOST Interrupts
 void  MostDisableInt(byte interrupts);
#endif

#ifdef MCS_3                                                                // Set the node address
 void  MostSetNodeAdr(word adr);
#endif

#ifdef MCS_4                                                                // Set the group address
 void  MostSetGroupAdr(byte group_adr);
#endif

#ifdef MCS_5                                                                // Select RMCK frequency
 void  MostSelectClockOutput(byte rmck_divide);
#endif

#ifdef MCS_6                                                                // Mute source data outputs
 void  MostMute(void);
#endif

#ifdef MCS_7                                                                // Demute source data outputs
 void  MostDemute(void);
#endif

#ifdef MCS_8                                                                // Read the current node position
 byte  MostGetNodePos(void);
#endif

#ifdef MCS_9                                                                // Read MOST Transceiver Revision (Bcd coded)
 void  MostGetRevision(byte* tgt_ptr);
#endif

#ifdef MCS_10                                                               // Read Node Delay Register
 byte  MostGetNodeDelay(void);
#endif

#ifdef MCS_11                                                               // Read Maximum Delay Register
 byte  MostGetMaxDelay(void);
#endif

#ifdef MCS_12                                                               // Read Maximum Position Register
 byte  MostGetMaxPos(void);
#endif

#ifdef MCS_13                                                               // Set alternative packet address
 void  MostSetAltPAdr(word adr);
#endif

#ifdef MCS_21                                                               // Prepare error message in a secondary node
 void  MostSecNodePrepErr(void);
#endif

#if (defined MCS_30) || (defined MCS_40)
 short  MostWriteReg(word handle, byte page, byte map, byte num, byte* pdata);
#endif

#if (defined MCS_31) || (defined MCS_41)
 short  MostReadReg(word handle, byte page, byte map, byte num);
#endif

#if (defined MCS_32) || (defined MCS_42)
 short  MostWriteRegBit(word handle, byte cmd, byte map, byte bitmask);
#endif

#if (defined MCS_33) || (defined MCS_43)
 short  MostWriteRegBitfield(word handle, byte page, byte map, byte mask, byte bitfield);
#endif

#ifdef MCS_50
 short  MostFindChannels(byte num_channels, byte label);
#endif




// functions only available in a Master device
//----------------------------------------------
#ifdef MCS_M1                                                               // Select the PLL input
 void  MostSelectClockInput(byte pll_in);
#endif

#ifdef MCS_M2                                                               // Set the synchronous bandwith
 byte  MostSetBoundary(byte sync_quad);
#endif






/********************************************************************************************************************

                            CALLBACK FUNCTIONS OF MOST NETSERVICES (BASIC LAYER)

********************************************************************************************************************/



//-----------------------------------------------------------------//
// Callback Functions of MOST NetServices Kernel (MNS)             //
// Can be an empty function block if the application has not to    //
// react to this calls.                                            //
//-----------------------------------------------------------------//
#ifdef MNS_CB1
void MnsRequest(word flags);
#endif

#ifdef MNS_CB2
void MnsRequestAsync(byte flags);
#endif

#ifdef MNS_CB3
void MnsRequestTimer(void);
#endif

#ifdef MNS_CB4
void MnsRequestAsyncTimer(void);
#endif

#ifdef MNS_CB10
void MnsPrintDbgMsg(pTMnsDbgMsg pmsg);
#endif







//-----------------------------------------------------------------//
// Callback Functions of Extended MOST Supervisor                  //
// Can be an empty function block if the application has not to    //
// react to this calls.                                            //
//-----------------------------------------------------------------//
#if (defined MSV_EXTENDED) || (defined MSV_VIRTUAL)
void Go_Confirm_S(void);                                                // called by transition to Confirm_Slave state
void Go_Confirm_M(void);                                                // called by transition to Confirm_Master state
void Go_Net_On(void);                                                   // called by transition to On state
void Go_Net_Off(void);                                                  // called by transition to Off state
void Go_Virtual_Zero(void);                                             // called by transition to virtual zero power state
void Go_Zero_Power(void);                                               //
void Go_Start(void);                                                    // called by restart after virtual zero-power
void MostMprChanged(byte new_mpr, byte mpr_old);                        // called if MPR register has changed
void MostMprChangedDelayed(byte new_mpr, byte mpr_old);                 // called if MPR register has changed and MPR is valid


void MostUnlock(void);                                                  // called on temporary unlock
void MostUnlockCritical(void);                                          // called on critical unlock
void MostLock(void);                                                    // called whenever lock gained after unlock
void MostLockStable(void);                                              // called after stable lock gained
void Store_Error_Info(byte);                                            // called to send an error info
void Store_Diag_Info(byte);                                             // called to send the diagnosis info
void MostUpdateNpr(byte npr);                                           // only available, if MSV_ADD2 is defined
void MostUpdateMostRevision(byte* pMostRev);                            // only available, if MSV_ADD2 is defined
void MostInfoReset(void);                                               // only available, if MSV_ADD5 is defined
#ifdef MSV_CB28
void MostCodingError(void);
#endif


#endif  // (defined MSV_EXTENDED) || (defined MSV_VIRTUAL)


//-----------------------------------------------------------------//
// Callback Functions of Extended MOST Supervisor                  //
// These functions MUST work along the discription since needed    //
// by this Service.                                                //
//-----------------------------------------------------------------//
#ifdef MSV_EXTENDED
byte Most_Por_Int(void);                                                // Get POR-Int Pin
void Most_Reset(void);                                                  // Reset MOST Transceiver
#endif


//-----------------------------------------------------------------//
// Callback Functions of Minimum MOST Supervisor                   //
// Can be an empty function block if the application has not to    //
// react to this calls.                                            //
//-----------------------------------------------------------------//
#ifdef MSV_MINIMUM
void Go_Net_On(void);                                                   // called by transition to On state
void Go_Net_Off(void);                                                  // called by transition to Off state
void MostInfoReset(void);                                               // only available, if MSV_ADD5 is defined
#endif


//-----------------------------------------------------------------//
// Callback Functions of Minimum MOST Supervisor                   //
// These functions MUST work along the discription since needed    //
// by this Service.                                                //
//-----------------------------------------------------------------//
#ifdef MSV_MINIMUM
byte Most_Por_Int(void);                                                // Get POR-Int Pin
void Most_Reset(void);                                                  // Reset MOST Transceiver
#endif



//-----------------------------------------------------------------//
// Callback Functions of Event Combiner                            //
// Can be an empty function block if the application has not to    //
// react to this calls.                                            //
//-----------------------------------------------------------------//
#ifdef MSVAL_CB1
void MsvalStateChanged(byte state);
#endif

#ifdef MSVAL_CB2
void MsvalLockChanged(byte lock_state);
#endif

#ifdef MSVAL_CB3
void MsvalEvent(byte event, byte *info);
#endif

#ifdef MSVAL_CB4
void MsvalError(byte error, byte *info);
#endif

#ifdef MSVAL_CB5
void MsvalDiagResult(byte diag_result, byte *info);
#endif




//-----------------------------------------------------------------//
// Callback Functions of Control Message Service (CMS)             //
// Can be an empty function block if the application has not to    //
// react to this calls.                                            //
//-----------------------------------------------------------------//
#ifdef CMS_CB2
void CtrlGetExtData(struct Ctrl_Rx_Type *prbuf);                        // application can attach extended bytes
#endif                                                                  // on every received message (timestamp,...),
                                                                        // but take care that MAX_EXT_DATA is defined
                                                                        // as great as you need




//-----------------------------------------------------------------//
// Callback Functions of Control Message Service (CMS)             //
// These functions MUST work along the discription since needed    //
// by this Service.                                                //
//-----------------------------------------------------------------//
#ifdef CMS_CB1
void CtrlTxStatus(byte Status, struct Ctrl_Tx_Type *ptbuf);             // Function will be called, if a transmission
#endif                                                                  // failed
                                                                        // The application HAS to react on this event
                                                                        // by deleting or retrieing the last transmission

#ifdef CMS_CB3
byte CtrlRxOutMsg(struct Ctrl_Rx_Type *prbuf);                          // function is called whenever a message was received
#endif                                                                  // and CMS output is selected
                                                                        // possible return values:
                                                                        //      0x00 = using polling and/or copy method
                                                                        //      0x01 = acknowledge (pointer grabbed)
                                                                        //      0x02 = force a retrigger

#ifdef CMS_CB4
byte CtrlRxFilter(struct Ctrl_Rx_Type *prbuf);                          // function is called whenever a message was received
#endif                                                                  // and this optional rx filter is enabled
                                                                        // possible return values:
                                                                        //      0x00 = default output (AMS if available / or CMS)
                                                                        //      0x01 = telegram is passed to a further service;
                                                                        //             buffer entry can be cleared
                                                                        //      0x02 = telegram will be passed to a further service;
                                                                        //             buffer entry cannot be cleared, since service busy
                                                                        //      0x03 = CMS output is forced, although AMS is available


#ifdef CMS_CB5                                                          // function is called, before transmission of a CMS telegram.
byte CtrlTxFilter(struct Ctrl_Tx_Type *ptbuf);                          // In this callback function the application can monitor, modify
#endif                                                                  // or dispatch the outgoing telegram.
                                                                        // Possible return values:
                                                                        //      0x00 = Tx message was checked and was modified if required.
                                                                        //             The message is send in an ordinary way
                                                                        //      0x01 = Tx message will be processed by another service.
                                                                        //             The CMS buffer entry is cleared after returning from
                                                                        //             callback function.
                                                                        //      0x02 = Tx message will be processed by another service,
                                                                        //             but the service is busy at that time. The entry
                                                                        //             is not cleared. The callback function will be re-called.





//-----------------------------------------------------------------//
// Callback Functions of Application Message Service (AMS)         //
// Can be an empty function block if the application has not to    //
// react to this calls.                                            //
//-----------------------------------------------------------------//
#ifdef AMS_CB3
void MsgRxError(byte Error_ID, void *prbuf);                            // error appeared while receiving a telegram;
#endif                                                                  // rx buffers are cleared automatically








//-----------------------------------------------------------------//
// Callback Functions of Application Message Service (AMS)         //
// These functions MUST work along the discription since needed    //
// by this Service.                                                //
//-----------------------------------------------------------------//
#ifdef AMS_CB1
void MsgTxStatus(byte Status, struct Msg_Tx_Type *ptbuf);               // Message successfully transmitted
#endif                                                                  // or transmission of last telegram failed.
                                                                        // If any error occurred, there are two
                                                                        // possible ways to react:
                                                                        //      a) deleting the message
                                                                        //      b) retry transmission

#ifdef AMS_CB2
byte MsgRxOutMsg(struct Msg_Rx_Type *prbuf);                            // function is called when a message was received
#endif                                                                  // by Application Message Service
                                                                        // possible return value:
                                                                        //      0x00 = using polling and/or copy method
                                                                        //      0x01 = acknowledge (pointer grabbed)
                                                                        //      0x02 = force a retrigger


#ifdef AMS_CB4                                                          // function is called, whenever the application
void MsgTxRefill(struct Msg_Tx_Type *tx_ptr, word partition_cnt);       // must refill the data fields after last partition
#endif                                                                  // was transmitted


#ifdef AMS_CB5                                                          // function is called, before transmission of a AMS message.
byte MsgTxFilter(struct Msg_Tx_Type *tx_ptr);                           // In this callback function the application can monitor, modify
#endif                                                                  // or dispatch the outgoing message.
                                                                        // Possible return values:
                                                                        //      0x00 = Tx message was checked and was modified if required.
                                                                        //             The message is send in an ordinary way
                                                                        //      0x01 = Tx message will be processed by another service.
                                                                        //             The AMS buffer entry is cleared after returning from
                                                                        //             callback function.
                                                                        //      0x02 = Tx message will be processed by another service,
                                                                        //             but the service is busy at that time. The entry
                                                                        //             is not cleared. The callback function will be re-called.

#ifdef AMS_CB6
bool MsgTxBypassFilter(struct Msg_Tx_Type *ptbuf);                      // function is called, when calling MsgSend().
#endif                                                                  // The application has the possibility to bypass certain messages
                                                                        // directly to the CMS TX service.
                                                                        // Possible return values:
                                                                        //      TRUE  = bypass application message to CMS TX service
                                                                        //      FALSE = message has to be send via AMS TX service


#ifdef AMS_CB7
pTMsgRxExtBuf MsgRxAllocBuf(pTMsgRx rx_ptr);                            // Function to allocate a receive buffer extension
#endif

#ifdef AMS_CB8
void MsgRxDeallocBuf(pTMsgRxExtBuf ptr);                                // Function to deallocate a receive buffer extension and respective data field
#endif




//-----------------------------------------------------------------//
// Callback Functions of Remote Control Service (RCS)              //
// These functions MUST work along the discription since needed    //
// by this Service.                                                //
//-----------------------------------------------------------------//
#ifdef RCS_CB1
void  RemoteWriteComplete(byte XmitStatus,struct Ctrl_Tx_Type *ptrbuf);
#endif

#ifdef RCS_CB2
void RemoteReadComplete(byte XmitStatus, struct Ctrl_Tx_Type *ptrbuf);
#endif




//-----------------------------------------------------------------//
// Callback Functions of Synchronous Channel Alloc. Service (SCS)  //
// These functions MUST work along the discription since needed    //
// by this Service.                                                //
//-----------------------------------------------------------------//
#ifdef SCS_CB1
void SyncAllocComplete(byte Status, byte Num_Channels_Req, byte Num_Channels_Free);     // is called when allocation procedure
#endif                                                                                  // has been finished (bad or ok)

#ifdef SCS_CB2
void SyncDeallocComplete(byte Status, byte Label);                                      // is called when de-allocation procedure
#endif                                                                                  // has been finished (bad or ok)

#ifdef SCS_CB3
void SyncGetSourceComplete(byte status, byte channel, byte node_pos, byte group_adr, word node_adr);    // is called when get source procedure
#endif                                                                                              // has been finished (bad or ok)

#ifdef SCS_CB4
void SyncMoveBoundaryComplete(byte status, byte boundary);    							// is called when boundary change is complete
#endif                                                                                              


//-----------------------------------------------------------------//
// Callback Functions of Async. Data Transmission Service (ADS)    //
// These functions MUST work along the discription since needed    //
// by this Service.                                                //
//-----------------------------------------------------------------//

#ifdef ADS_CB1
byte DataRxOutMsg(struct Data_Rx_Type *prbuf);                          // function is called when a packet was received
#endif                                                                  // possible return value:
                                                                        //      0x00 = using polling and/or copy method
                                                                        //      0x01 = acknowledge (pointer grabbed)
                                                                        //      0x02 = force a retrigger



#ifdef ADS_CB2
bool DataGetInt(void);                                                  // Polling of AsyncInterrupt Pin (/AINT)
#endif                                                                  // or polling of Bit4 in the Control Port Status Register (bCP)
                                                                        // Application has to return TRUE, whenever status of pin /AINT
                                                                        // is low. If the signal is not available, since the pin is not
                                                                        // connected to your uC, Bit4 of bCP can be returned.
                                                                        // Whenever this functions returns TRUE, register bPCTC will be
                                                                        // read to check what kind of interrupt occurred.

#ifdef ADS_CB3
void DataTxComplete(struct Data_Tx_Type *ptbuf);                        // Function will be called, whenever a packet
#endif                                                                  // was transmitted


#ifdef ADS_CB4
byte DataRxFilter(struct Data_Rx_Type *rx_ptr);                         // external rx filter to check kind of protocol
#endif

#ifdef ADS_CB5
void DataRxError(byte error);                                           // notify captured error events
#endif




//-----------------------------------------------------------------//
// Callback Functions of MOST Transceiver Control Service (MCS)    //
// Can be an empty function block if the application has not to    //
// react to this calls.                                            //
//-----------------------------------------------------------------//

#if (defined MCS_CB30) || (defined MCS_CB40)
void MostWriteRegComplete(pTMnsCtrl pinfo);
#endif

#if (defined MCS_CB31) || (defined MCS_CB41)
void MostReadRegComplete(pTMnsCtrl pinfo);
#endif

#if (defined MCS_CB32) || (defined MCS_CB42)
void MostWriteRegBitComplete(pTMnsCtrl pinfo);
#endif

#if (defined MCS_CB33) || (defined MCS_CB43)
void MostWriteRegBitfieldComplete(pTMnsCtrl pinfo);
#endif

#ifdef MCS_CB50
void MostFindChannelsResult(byte num_channels, byte* ptr_result);
#endif













/********************************************************************************************************************

                            THE FOLLOWING FUNCTIONS OF LAYER II ARE CALLED BY LAYER I

********************************************************************************************************************/


//-----------------------------------------------------------------//
//  Functions of Address Handler Module (AH)                       //
//-----------------------------------------------------------------//

#ifdef NS_AMS_AH
void AddrHSearchStart(struct Msg_Tx_Type *ptbuf);                       // Starts searching process, called by AMS
#endif                                                                  //   - Search in decentral registry (if avail.)
                                                                        //   - Search in central registry (if avail.)
                                                                        //   - Search using property NetBlock.FBlockIDs



//-----------------------------------------------------------------//
//  Functions of MOST NetService Kernel Layer II                   //
//-----------------------------------------------------------------//

#ifdef NS_MNS_MNS2
void MostTimerInt2(void);                                               // called by MNS
void MostTimerInt2Diff(TTimerDiff diff);                                        // called by MNS
#endif




//-----------------------------------------------------------------//
//  Functions of NetBlock Module (NB)                              //
//-----------------------------------------------------------------//

#ifdef NS_SCS_NB
void NbSyncFinalAlloc(byte handle);                                     // Called by SCS
#endif

#ifdef NS_SCS_NB
void NbSyncFinalDealloc(byte handle);                                   // Called by SCS
#endif

#ifdef NS_SCS_NB
void NbSyncMoveBoundaryComplete(byte status, byte boundary);	        // Called by SCS
#endif

#ifdef NS_MSV_NB
void NbRefreshNodePos(void);                                            // Called by MSV
#endif

#ifdef NS_MSV_NB
void NbGoNetOff(void);                                                  // Called by MSV
#endif


#ifdef __cplusplus
}
#endif




#endif // _MOSTNS1_H

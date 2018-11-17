/*
 *  Copyright(c) Siemens AG, Muenchen, Germany, 2005, 2006, 2007
 *                           Bernhard Walle <bernhard.walle@gmx.de>
 *                           Gernot Hillier <gernot.hillier@siemens.com>
 *
 * ----------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 */
#ifndef MOST_CONSTANTS_H
#define MOST_CONSTANTS_H

/**
 * @file most-constants.h
 * @ingroup common
 *
 * @brief Various constants for MOST drivers, including register addresses.
 */

/*
 * General constants -----------------------------------------------------------
 */

/**
 * The vendor ID ("OASIS") for the MOST PCI card.
 */
#define PCI_VENDOR_ID_OASIS                         0x121D

/**
 * The device ID for the MOST PCI card from OASIS Silicon Systems. Together with the
 * PCI_VENDOR_ID_OASIS, this must be unique for this card.
 */
#define PCI_DEVICE_ID_OASIS_MOST_PCI_INTERFACE      0x4711

/**
 * Number of devices (PCI cards) that can be in the system at the same time. 
 * It makes sense to define this value static because the number of devices available
 * in /dev/ has to be static, too (and this must be the same value). And the interrupt
 * parameter contains the device number, so a array lookup is fater than a list search.
 */
#define MOST_DEVICE_NUMBER                          8

/**
 * Maximum polling number. Used in control-port operations.
 */
#define MOST_MAX_POLL                               1024

/**
 * Maximum retry count. (use grep to see where the constant is used)
 */
#define MAX_RETRIES                                 10

/**
 * Amount of time in ns that the delay is incremented between pollings.
 * The value starts with 0 and is incremented by this value.
 *
 * If MOST_MAX_POLL is 1024 and MOST_DELAY_INCREMENT is 10, the overall 
 * time for polling takes about 3 ms.
 */
#define MOST_DELAY_INCREMENT                        10

/**
 * How often can a /dev/mostsyncX device be opened?
 */
#define MOST_SYNC_OPENS                             8

/**
 * Number of quadlets in a MOST frame.
 */
#define NUM_OF_QUADLETS                             (60/4)

/**
 * Usual number of MOST frames per second
 */
#define STD_MOST_FRAMES_PER_SEC                     44100

/**
 * Feature mask for the synchronous transfer.
 */
#define MOST_FEATURE_SYNC                           (1 << 0)

/**
 * Feature mask for the asynchronous transfer.
 */
#define MOST_FEATURE_ASYNC                          (1 << 1)

/**
 * Feature mask for the master capability.
 */
#define MOST_FEATURE_MASTER                         (1 << 2)

/**
 * Feature mask for the control transfer.
 */
#define MOST_FEATURE_CTRL                           (1 << 3)

/*
 * Register offset definitions of OS 8604 -------------------------------------
 */

/**
 * 6.2 Control Data Port (CDP) Register And Miscallaneous (MISC) register
 * 6.2.1 Control Data Port (CDP) Register
 * 6.2.1.1 Command (CMD) Register
 * -> p. 76
 */
#define MOST_PCI_CMD_REG                (0x00)

/**
 * MOST Reset bit:
 * If the MOST Reset bit is set (`1'), a reset pulse on the MOST Reset Signal
 * (MOST_RST_N) will be generated.  After execution of reset, the bit will be
 * cleared automatically.
 *
 * It should be noted that if the OS8604 detects a rising edge on the
 * PCI_RST_N signal a low active reset pulse on the MOST_RST_N signal is
 * performed.
 */
#define MRST                            (1 << 11)

/**
 * Execute bit: 
 * The Execute bit starts the execution of the operation that is defined by
 * the CPOP bits. After execution of the specified control port operation, the
 * EXEC bit will be automatically set to `0' and a Control Port FSM Interrupt
 * will be generated.
 * - EXEC = 1 -> Start control port operation
 * - EXEC = 0 -> Control port operation successfully executed
 */
#define EXEC                            (1 << 10)

/**
 * Control Port Operation bits:
 * The Control Port Operation bits determine which operation will be executed
 * on the control port of the MOST Transceiver. The bits are coded as follows.
 * - CPOP1/CPOP0 = 00 -> Write Control Port MAP
 * - CPOP1/CPOP0 = 01 -> Read Control Port Status
 * - CPOP1/CPOP0 = 10 -> Write Control Port Data
 * - CPOP1/CPOP0 = 11 -> Read Control Port Data
 */
#define CPOP1                           (1 << 9)

/**
 * @see CPOP1
 */
#define CPOP0                           (1 << 8)

/**
 * Control Port Data bits:
 * These bits represent the data bits transferred by the
 * Control Port.
 */
#define CMD_DATA                        0xff

/**
 * Control Port Data bits:
 * These bits represent the data bits transferred by the
 * Control Port.
 */
#define VALUE_DATA                      0xff

/**
 * 6.2 Control Data Port (CDP) Register And Miscallaneous (MISC) register
 * 6.2.1 Control Data Port (CDP) Register
 * 6.2.2.1 Reset (RST) Register
 * -> p. 77
 */
#define MOST_PCI_RST_REG                0x04

/**
 * 6.2 Control Data Port (CDP) Register And Miscallaneous (MISC) register
 * 6.2.1 Control Data Port (CDP) Register
 * 6.2.2.2 Control (CTRL) Register
 * -> p. 79
 */
#define MOST_PCI_CTRL_REG               0x08

/**
 * MOST Source Port Write Enable bit:
 * - WREN = '0' -> Write Operations on Source Port of MOST Chip are deactivated.
 * - WREN = '1' -> During each SF Interval 8 write operations on the Source Port of the 
 *   MOST Chip are executed.
 *
 * 
 * Please note:
 * If a synchronous or asynchronous transmit transfer is started the write
 * operations are activated regardless if the WREN bit is set or not.
 */
#define WREN                            (1 << 3)

/**
 * MOST Source Port Read Enable bit:
 * - RDEN = '0' -> Read Operations on Source Port of MOST Chip are deactivated.
 * - RDEN = '1' -> During each SF Interval 8 Read Operations on the Source Port of the
 *   MOST Chip are executed.
 * By default, the RDEN bit is cleared ('0').
 * 
 * Please note:
 * If a synchronous or asynchronous receive transfer is started the read
 * operations are activated regardless if the RDEN bit is set or not.
 */
#define RDEN                            (1 << 2)

/**
 * Transmit Big Endian Enable bit:
 * This bit defines the data format of the transmitted synchronous data quadlets.
 * - TXBIGEN = `0' -> Little Endian data format
 * - TXBIGEN = `1' -> Big Endian data format
 */
#define TXBIGEN                         (1 << 1)

/**
 * Receive Big Endian Enable bit:
 * This bit defines the data format of the received synchronous and
 * asynchronous data quadlets.
 * - RXBIGEN = `0' -> Little Endian data format
 * - RXBIGEN = `1' -> Big Endian data format
 */
#define RXBIGEN                         (1 << 0)

/**
 * 6.2 Control Data Port (CDP) Register And Miscallaneous (MISC) register
 * 6.2.1 Control Data Port (CDP) Register
 * 6.2.2.3 MOST Lock Status (LOCKSTATUS) Register
 * -> p. 82
 */
#define MOST_PCI_LOCKSTATUS_REG         (0x0C)

/**
 * 6.2 Control Data Port (CDP) Register And Miscallaneous (MISC) register
 * 6.2.1 Control Data Port (CDP) Register
 * 6.2.2.4 Interrupt Mask (INTMASK) Register
 * -> p. 82
 */
#define MOST_PCI_INTMASK_REG            (0x10)

/**
 * Interrupt Enable bit for MOST Lock/Unlock Interrupt:
 * If this bit is set (`1'), a PCI Interrupt will be generated if the MOST
 * Transceiver is changing lock state.
 */
#define IEMLOCK                         (1 << 8)

/**
 * Interrupt Enable bit for MOST Asynchronous Interrupt:
 * If this bit is set (`1'), a PCI Interrupt will be generated if an
 * Asynchronous Interrupt on the MOST Chip occurs.
 */
#define IEMAINT                         (1 << 7)

/**
 * Interrupt Enable bit for MOST Interrupt:
 * If this bit is set (`1'), a PCI Interrupt will be generated if an
 * Interrupt on the MOST Chip occurs.
 */
#define IEMINT                          (1 << 6)

/**
 * Interrupt Enable bit for General Purpose Interrupt:
 * If this bit is set (`1'), a PCI Interrupt will be generated if an Interrupt
 * is pending at the GP_INT Pin of the OS8604.  The GP_INT pin is level
 * sensitive and active high.
 */
#define IEGP                            (1 << 5)

/**
 * Interrupt Enable bit for Synchronous RX Interrupt:
 * If set (`1'), a PCI Interrupt will be generated if the initiator changes
 * the Synchronous RX Buffer Page.
 */
#define IESRX                           (1 << 4)

/**
 * Interrupt Enable bit for Synchronous TX Interrupt:
 * If set (`1'), a PCI Interrupt will be generated if the initiator changes
 * the Synchronous TX Buffer Page.
 */
#define IESTX                           (1 << 3)

/**
 * Interrupt Enable bit for Asynchronous RX Interrupt:
 * If set (`1'), a PCI Interrupt will be generated if an asynchronous message
 * is received.  Interrupt Enable bit for Asynchronous TX Interrupt:
 */
#define IEARX                           (1 << 2)

/**
 * Interrupt Enable bit for Asynchronous TX Interrupt:
 * If set (`1'), a PCI Interrupt will be generated if an asynchronous message
 * is transmitted.
 */
#define IEATX                           (1 << 1)

/**
 * Interrupt Enable bit for Control Port Finite State Machine Interrupt:
 * If set (`1'), a PCI Interrupt will be generated if the Control Port State
 * Machine has executed a Control Port operation successfully.
 */
#define IECPFSM                         (1 << 0)

/**
 * 6.2 Control Data Port (CDP) Register And Miscallaneous (MISC) register
 * 6.2.1 Control Data Port (CDP) Register
 * 6.2.2.5 Interrupt Status (INTSTATUS) Register
 * -> p. 83
 */
#define MOST_PCI_INTSTATUS_REG          (0x14)

/**
 * Interrupt Status bit for MOST Asynchronous Interrupt:
 * This bit is `1', as long as the /AINT signal of the MOST Transceiver is
 * active (= `0'). If the IEMAINT bit in the Interrupt Mask register (INTMASK)
 * is set, a PCI Interrupt will be generated.
 *
 * Writing a `1' clears this bit. If the /AINT signal is still active the bit
 * is set again immediately.
 */
#define ISMAINT                         (1 << 7)

/**
 * Interrupt Status bit for MOST Interrupt:
 * This bit is `1', as long as the /INT signal of the MOST Transceiver is
 * active (='0'). If the IEMINT bit in the Interrupt Mask register (INTMASK)
 * is set, a PCI Interrupt will be generated.
 *
 * Writing a `1' clears this bit. If the /INT signal is still active the bit
 * is set again immediately.
 */
#define ISMINT                          (1 << 6)

/**
 * Interrupt Status bit for General Purpose Interrupt:
 * This bit is '1', if a GP_INT was active. Writing a `1' clears this bit.
 */
#define ISGP                            (1 << 5)

/**
 * Interrupt Status bit for Synchronous RX Interrupt:
 * This bit is `1', if the initiator has changed the Synchronous RX Buffer
 * Page. If the IESRX bit in the Interrupt Mask register (INTMASK) is set, a
 * PCI Interrupt will be generated. Writing a `1' clears this bit.
 */
#define ISSRX                           (1 << 4)

/**
 * Interrupt Status bit for Synchronous TX Interrupt:
 * This bit is `1', if the initiator has changed the Synchronous TX Buffer
 * Page. If the IESTX bit in the Interrupt Mask register (INTMASK) is set, a
 * PCI Interrupt will be generated. Writing a `1' clears this bit.
 */
#define ISSTX                           (1 << 3)

/**
 * Interrupt Status bit for Asynchronous RX Interrupt:
 * This bit is `1', if the initiator has received an asynchronous message. If
 * the IEARX bit in the Interrupt Mask register (INTMASK) is set, a PCI
 * Interrupt will be generated.  Writing a `1' clears this bit.
 */
#define ISARX                           (1 << 2)

/**
 * Interrupt Status bit for Asynchronous TX Interrupt:
 * This bit is `1', if the initiator has transmitted an asynchronous message.
 * If the IEATX bit in the Interrupt Mask register (INTMASK) is set, a PCI
 * Interrupt will be generated. Writing a `1' clears this bit.
 */
#define ISATX                           (1 << 1)

/**
 * Interrupt Status bit for Control Port Finite State Machine Interrupt:
 * This bit is set ('1'), if the Control Port State Machine has executed a
 * Control Port operation successfully. If the IECPFSM bit in the Interrupt
 * Mask register (INTMASK) is set, a PCI Interrupt will be generated. Writing
 * a `1' clears this bit.
 */
#define ISCPFSM                         (1 << 0)

/**
 * 6.2 Control Data Port (CDP) Register And Miscallaneous (MISC) register
 * 6.2.1 Control Data Port (CDP) Register
 * 6.2.2.6 Version (VERSION) Register
 * -> p. 83
 */
#define MOST_PCI_VERSION_REG            0x18

/**
 * 6.2 Control Data Port (CDP) Register And Miscallaneous (MISC) register
 * 6.2.1 Control Data Port (CDP) Register
 * 6.2.2.7 Synchronous Bandwidth And Node Position (SBC_NPOS) Register
 * -> p. 84
 */
#define MOST_PCI_SBC_NPOS_REG           0x1C

/**
 * 6.3 Synchronous Data Port (SDP) Register
 * 6.3.1 Synchronous TX Start Address (STXSA) Register
 * -> p. 85
 */
#define MOST_PCI_STXSA_REG              0x20

/**
 * 6.3 Synchronous Data Port (SDP) Register
 * 6.3.2 Synchronous Tx Page Size (STXPS) Register
 * -> p. 85
 */
#define MOST_PCI_STXPS_REG              0x24

/**
 * 6.3 Synchronous Data Port (SDP) Register
 * 6.3.3 Synchronous TX Control (STXCTRL) Register
 * -> p. 86
 */
#define MOST_PCI_STXCTRL_REG            0x28

/**
 * Synchronous TX Start Transfer bit:
 * If set (`1'), initiator starts transferring synchronous transmit data. If
 * cleared (`0'), transfer is stopped.
 */
#define STXST                           (1 << 1)

/**
 * Synchronous TX Page Pointer bit:
 * This bit represents the Synchronous TX Buffer Page currently accessed by the OS8604.
 * - STXPP = '0' -> OS8604 is accessing page 0
 * - STXPP = '1' -> OS8604 is accessing page 1
 */
#define STXPP                           (1 << 0)

/**
 * 6.3 Synchronous Data Port (SDP) Register
 * 6.3.4 Synchronous TX Channel Adjustment (STXCA) Register
 * -> p. 86
 */
#define MOST_PCI_STXCA_REG              0x2C

/**
 * 6.3 Synchronous Data Port (SDP) Register
 * 6.3.5 Synchronous RX Start Address (SRXSA) Register
 * -> p. 86
 */
#define MOST_PCI_SRXSA_REG              0x30

/**
 * 6.3 Synchronous Data Port (SDP) Register
 * 6.3.6 Synchronous RX Page Size (SRXPS) Register
 * -> p. 87
 */
#define MOST_PCI_SRXPS_REG              0x34

/**
 * 6.3 Synchronous Data Port (SDP) Register
 * 6.3.7 Synchronous RX Control (SRXCTRL) Register
 * -> p. 87
 */
#define MOST_PCI_SRXCTRL_REG            0x38

/**
 * Synchronous RX Start Transfer bit:
 * If set (`1') initiator starts transferring synchronous receive data to the
 * Synchronous RX Buffer Pages in the memory of the host system. If cleared
 * (`0') transfer is stopped.  Synchronous RX Page Pointer bit:
 */
#define SRXST                           (1 << 3)

/**
 * Synchronous RX Page Pointer bit:
 * This bit represents the Synchronous RX Buffer Page currently accessed by the OS8604.
 * - SRXPP = '0' -> OS8604 is accessing page 0
 * - SRXPP = '1' -> OS8604 is accessing page 1
 */
#define SRXPP                           (1 << 2)

/**
 * 6.3 Synchronous Data Port (SDP) Register
 * 6.3.8 Synchronous RX Channel Adjustment (SRXCA) Register
 * -> p. 87
 */
#define MOST_PCI_SRXCA_REG              0x3C

/**
 * 6.4 Asynchronous Data Port (ADP) Register
 * 6.4.1 Asynchronous TX Start Address (ATXSA) Register
 * -> p. 88
 */
#define MOST_PCI_ATXSA_REG              0x40

/**
 * Reserved
 */
#define MOST_PCI_RESERVED_0             0x44

/**
 * 6.4 Asynchronous Data Port (ADP) Register
 * 6.4.2 Asynchronous RX Control (ARXCTRL) Register
 * -> p. 89
 */
#define MOST_PCI_ARXCTRL_REG            0x48

/**
 * 6.4 Asynchronous Data Port (ADP) Register
 * 6.4.3 Asynchronous TX Control (ATXCTRL) Register
 * -> p. 89
 */
#define MOST_PCI_ATXCTRL_REG            0x4C

/**
 * 6.4 Asynchronous Data Port (ADP) Register
 * 6.4.4 Asynchronous RX Start Address (ARXSA) Register
 * -> p. 89
 */
#define MOST_PCI_ARXSA_REG              0x50

/**
 * Reserved
 */
#define MOST_PCI_RESERVED_1             0x54

/**
 * Reserved
 */
#define MOST_PCI_RESERVED_2             0x58

/**
 * 6.4 Asynchronous Data Port (ADP) Register
 * 6.4.5 Alternative Asynchronous RX Message Address (AARXMA) Register
 * -> p. 89
 */
#define MOST_PCI_AARXMA_REG             0x5C

/**
 * 6.4 Asynchronous Data Port (ADP) Register
 * 6.4.6 Asynchronous RX Message Address (ARXMA) Register
 * -> p. 90
 */
#define MOST_PCI_ARXMA_REG              0x60

/**
 * 6.4 Asynchronous Data Port (ADP) Register
 * 6.4.7 Asynchronous RX Message Group Address (ARXMGA) Register
 * -> p. 90
 */
#define MOST_PCI_ARXMGA_REG             0x64

/**
 * Reserved
 */
#define MOST_PCI_RESERVED_3             0x68

/**
 * 6.5 Special Feature (SF) Register
 * 6.5.1 Trigger (TRIG) Register
 * -> p. 91
 */
#define MOST_PCI_TRIG_REG               0x6C

/**
 * 6.5 Special Feature (SF) Register
 * 6.5.2 Dallas Silicon Key Control (DSCTRL) Register
 * -> p. 93
 */
#define MOST_PCI_DSCTRL_REG             0x70

/**
 * Reserved
 */
#define MOST_PCI_RESERVED_4             0x74

/**
 * Reserved
 */
#define MOST_PCI_RESERVED_5             0x78

/**
 * Set when MOST Asynchronous transfers are enabled.
 */
#define DSCTL_EN_ASYNC                  (1 << 11)

/**
 * Set when MOST Synchronous transfers are enabled.
 */
#define DSCTL_EN_SYNC                   (1 << 10)

/**
 * Set when MOST Control transfers are enabled.
 */
#define DSCTL_EN_CTRL                   (1 << 9)

/**
 * Set when master functions are enabled.
 */
#define DSCTL_EN_MASTER                 (1 << 8)

/**
 * ??
 */
#define DSCTL_CRC_RST                   (1 << 7)

/**
 * ??
 */
#define DSCTL_CRC_EN                    (1 << 6)

/**
 * Read Dallas:
 * This bit will be cleared automatically, after reading of Dallas chip.
 * - DS_RD = `0' -> No action
 * - DS_RD = `1' -> Generate read timing for Dallas
 */
#define DSCTL_RD                        (1 << 5)

/**
 * Dallas Read Value:
 * - DS_RD_VAL = `0' -> Read `0' from Dallas
 * - DS_RD_VAL = `1' -> Read `1' from Dallas
 */
#define DSCTL_RD_VAL                    (1 << 4)

/**
 * Write Dallas:
 * This bit will be cleared automatically, after writing Dallas.
 * - DS_WR = `0' -> No action
 * - DS_WR = `1' -> Generate write timing for Dallas
 */
#define DSCTL_WR                        (1 << 3)

/**
 * Dallas Write Value:
 * - DS_WR_VAL = `0' -> Write `0' to Dallas
 * - DS_WR_VAL = `1' -> Write `1' to Dallas
 */
#define DSCTL_WR_VAL                    (1 << 2)

/**
 * Reset Dallas:
 * This bit will be cleared automatically, after reset pulse.
 * - DS_RST = `0' -> No action
 * - DS_RST = `1' -> Generate reset pulse for Dallas
 */
#define DSCTL_RST                       (1 << 1)

/**
 * Dallas Present:
 * This bit will automatically indicate Dallas presence status after a reset pulse.
 * - DS_PRE = `0' -> Dallas not present
 * - DS_PRE = `1' -> Dallas present
 */
#define DSCTL_PRE                       (1 << 0)

/**
 * ??
 */
#define LICENSE_ADDRESS_HW	            0x00

/**
 * ??
 */
#define LICENSE_ADDRESS_SW	            0x08

/**
 * ??
 */
#define LICENSE_ADDRESS_TO	            0x10

/*
 * Register offset definitions of OS 8104 ---------------------------------------------------------
 */

/**
 * This register controls sending and receiving of control messages. The status of the
 * operations triggered by bMSGC is reported in the message status register bMSGS.
 *
 * See OS8104 MOST Network Transceiver Final Product Data Sheet, p. 115
 */
#define     MSGC            0x0085                        // Message Control

/**
 * LIC: Undocumented
 * License register.
 */
#define MOST_PCI_LICENSE_REG            0x74

/**
 * LIC: Undocumented
 * License compare register. Must be written to 1 to activate.
 */
#define MOST_PCI_LICCOMP_REG            0x78

/**
 * 6.5 Special Feature (SF) Register
 * 6.5.3 In-System PROM Programming (ISP) Register
 * -> p. 94
 */
#define MOST_PCI_ISP_REG                0x7C

/**
 * 6.5 Special Feature (SF) Register
 * 6.5.4 I2C Master (I2C) Register
 * -> p. 95
 */
#define MOST_PCI_I2C_REG                0x80

/**
 * 6.5 Special Feature (SF) Register
 * 6.5.5 I2S (I2S) Register
 * -> p. 95
 */
#define MOST_PCI_I2S_REG                0x84

/**
 * Reserved
 */
#define MOST_PCI_RESERVED_6             0x88

/**
 * Reserved
 */
#define MOST_PCI_RESERVED_7             0x8C

/**
 * 6.5 Special Feature (SF) Register
 * 6.5.6 TimeStamp Counter (TSCNTR) Register
 * -> p. 96
 */
#define MOST_PCI_TSCNTR_REG             0x90

/**
 * 6.5 Special Feature (SF) Register
 * 6.5.7 Feature Mask (FMASK) Register
 * -> p.96
 */
#define MOST_PCI_FMASK_REG              0xBC

/**
 * Reserved
 */
#define MOST_PCI_RESERVED_8             0xFF

/*
 * Register offset definitions of OS 8104 -------------------------------------
 */

/**
 * The memory location in which the page must be written to change the current page.
 * See p. 63 of OS 8104 datasheet.
 */
#define MOST_IF_PAGE                    0xFF

/*
 * Misc constants -------------------------------------------------------------
 */


/**
 * ??
 */
#define MOST_PCI_FW_NEW_PACKET_LEN_POS  (1 << 0)

/**
 * ??
 */
#define MOST_PCI_FW_FOT_STATUS          (1 << 1)

/**
 * ??
 */
#define MOST_PCI_FW_SEC_ASYNC_ADDR      (1 << 2)

/**
 * ??
 */
#define MOST_PCI_FW_CHANGE_3DB          (1 << 3)

/**
 * ??
 */
#define MOST_PCI_FW_RX_TX_BIG_ENABLE    (1 << 4)

/**
 * ??
 */
#define MOST_PCI_FW_CHANGE_FREQUENCY    (1 << 5)

/**
 * Synchronous Bandwidth Control register of ths OS8104 MOST transceiver
 * (not the OS8604 PCI interface)
 */
#define MOST_8104_SBC_REG               0x0096

/**
 * Number of minor ids for the driver.
 */
#define MOST_MINOR_IDS                  255

#endif /* MOST_CONSTANTS_H */


/* vim: set ts=4 et sw=4: */

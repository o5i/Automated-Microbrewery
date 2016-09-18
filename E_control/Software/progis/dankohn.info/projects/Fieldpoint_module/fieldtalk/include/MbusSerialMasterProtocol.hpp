/**
 * @internal
 * @file MbusSerialMasterProtocol.hpp
 *
 * @if NOTICE
 *
 * $Id: MbusSerialMasterProtocol.hpp,v 1.10 2004/01/23 22:54:41 henrik Exp $
 *
 * Copyright (c) 2002-2003 FOCUS Software Engineering Pty Ltd, Australia.
 * All rights reserved. <www.focus-sw.com>
 *
 * USE OF THIS SOFTWARE IS GOVERNED BY THE TERMS AND CONDITIONS OF A
 * SEPARATE LICENSE STATEMENT AND LIMITED WARRANTY.
 *
 * IN PARTICULAR, YOU WILL INDEMNIFY AND HOLD FOCUS SOFTWARE ENGINEERING,
 * ITS RELATED COMPANIES AND ITS SUPPLIERS, HARMLESS FROM AND AGAINST ANY
 * CLAIMS OR LIABILITIES ARISING OUT OF THE USE, REPRODUCTION, OR
 * DISTRIBUTION OF YOUR PROGRAMS, INCLUDING ANY CLAIMS OR LIABILITIES
 * ARISING OUT OF OR RESULTING FROM THE USE, MODIFICATION, OR DISTRIBUTION
 * OF PROGRAMS OR FILES CREATED FROM, BASED ON, AND/OR DERIVED FROM THIS
 * SOURCE CODE FILE.
 *
 * @endif
 */


#ifndef _MBUSSERIALMASTERPROTOCOL_H_INCLUDED
#define _MBUSSERIALMASTERPROTOCOL_H_INCLUDED

#ifndef __cplusplus
#  error Must use C++ to compile this module!
#endif

// Package header
#include "hmserio.hpp"
#include "MbusMasterFunctions.hpp"


/*****************************************************************************
 * MbusSerialMasterProtocol class declaration
 *****************************************************************************/

/**
 * @brief Base class for serial serial master protocols
 *
 * This base class realises the Modbus serial master protocols. It provides
 * functions to open and to close serial port as well as data and control
 * functions which can be used at any time after the protocol has been
 * opened. The data and control functions are organized different
 * conformance classes. For a more detailed description of the data and
 * control functions see section @ref mbusmaster.
 *
 * It is possible to instantiate multiple instances for establishing
 * multiple connections on different serial ports (They should be executed
 * in separate threads).
 *
 * @version 1.1
 * @see mbusmaster
 * @see MbusMasterFunctions
 */
class
#if defined (_WINDLL) || defined(__DLL__)
   __declspec(dllexport)
#endif
MbusSerialMasterProtocol: public MbusMasterFunctions
{

  public:

   enum
   {
      SER_DATABITS_7 = SerialPort::SER_DATABITS_7,   ///< 7 data bits
      SER_DATABITS_8 = SerialPort::SER_DATABITS_8    ///< 8 data bits
   };

   enum
   {
      SER_STOPBITS_1 = SerialPort::SER_STOPBITS_1,   ///< 1 stop bit
      SER_STOPBITS_2 = SerialPort::SER_STOPBITS_2    ///< 2 stop bits
   };

   enum
   {
      SER_PARITY_NONE = SerialPort::SER_PARITY_NONE, ///< No parity
      SER_PARITY_EVEN = SerialPort::SER_PARITY_EVEN, ///< Even parity
      SER_PARITY_ODD = SerialPort::SER_PARITY_ODD    ///< Odd parity
   };


  protected:

   SerialPort serialPort;

   enum
   {
      SER_RS232, ///< RS232 mode w/o RTS/CTS handshake
      SER_RS485  ///< RS485 mode: RTS enables/disables transmitter
   };
   int serialMode;
   int rtsDelay;

   MbusSerialMasterProtocol();


  public:

   /**
    * @name Serial Port Management Functions
    */
   //@{

   virtual int openProtocol(const char * const portName,
                            long baudRate, int dataBits,
                            int stopBits, int parity);

   virtual int openProtocol(const char * const portName, long baudRate);

   virtual void closeProtocol();

   virtual int isOpen();

   virtual int enableRs485Mode(int rtsDelay);

   //@}


  protected:

   virtual int deliverMessage(int address, int function,
                              char sendDataArr[], int sendDataLen,
                              char rcvDataArr[], int rcvDataLen);

   virtual int transceiveMessage(int address, int function,
                                 char sendDataArr[], int sendDataLen,
                                 char rcvDataArr[], int rcvDataLen) = 0;


  private:

   // Disable default operator and copy constructor
   MbusSerialMasterProtocol &operator= (MbusSerialMasterProtocol &);
   MbusSerialMasterProtocol (const MbusSerialMasterProtocol &);

};


#endif // ifdef ..._H_INCLUDED


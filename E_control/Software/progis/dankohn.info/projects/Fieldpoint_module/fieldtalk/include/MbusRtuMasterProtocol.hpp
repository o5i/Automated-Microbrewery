/**
 * @internal
 * @file MbusRtuMasterProtocol.hpp
 *
 * @if NOTICE
 *
 * $Id: MbusRtuMasterProtocol.hpp,v 1.8 2004/01/23 22:58:14 henrik Exp $
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


#ifndef _MBUSRTUMASTERPROTOCOL_H_INCLUDED
#define _MBUSRTUMASTERPROTOCOL_H_INCLUDED

#ifndef __cplusplus
#  error Must use C++ to compile this module!
#endif

// Package header
#include "MbusSerialMasterProtocol.hpp"


/*****************************************************************************
 * MbusRtuMasterProtocol class declaration
 *****************************************************************************/

/**
 * @brief Modbus RTU Master Protocol class
 *
 * This class realizes the Modbus RTU master protocol. It provides
 * functions to open and to close serial port as well as data and control
 * functions which can be used at any time after the protocol has been
 * opened. The data and control functions are organized different
 * conformance classes. For a more detailed description of the data and
 * control functions see section @ref mbusmaster.
 *
 * It is possible to instantiate multiple instances of this class for
 * establishing multiple connections on different serial ports (They should
 * be executed in separate threads).
 *
 * @ingroup mbusmasterserial
 * @version 1.1
 * @see mbusmaster
 * @see MbusSerialMasterProtocol, MbusMasterFunctions
 */
class
#if defined (_WINDLL) || defined(__DLL__)
   __declspec(dllexport)
#endif
MbusRtuMasterProtocol: public MbusSerialMasterProtocol
{

  private:

   enum
   {
      SER_DATABITS_7 = SerialPort::SER_DATABITS_7   // Hide this
   };

   enum
   {
      // RTU header length is 2 bytes address/function
      HDR_LEN = 2 ,
      // RTU frame length is header + 2 bytes crc
      FRAME_LEN = HDR_LEN + 2,
      // RTU exception message length is frame + 1 byte exception code
      EXC_MSG_LEN = FRAME_LEN + 1,
      // RTU max. message size is data size + frame length
      MAX_MSG_SIZE = MAX_DATA_SIZE + FRAME_LEN
   };

   unsigned long frameSilence;
   char bufferArr[MAX_MSG_SIZE];


  public:

   MbusRtuMasterProtocol();

   virtual int openProtocol(const char * const portName, long baudRate, int dataBits,
                            int stopBits, int parity);

   virtual int openProtocol(const char * const portName, long baudRate);


  private:

   virtual int transceiveMessage(int address, int function,
                                 char sendDataArr[], int sendDataLen,
                                 char rcvDataArr[], int rcvDataLen);


  private:

   // Disable default operator and copy constructor
   MbusRtuMasterProtocol &operator= (MbusRtuMasterProtocol &);
   MbusRtuMasterProtocol (const MbusRtuMasterProtocol &);

};


#endif // ifdef ..._H_INCLUDED

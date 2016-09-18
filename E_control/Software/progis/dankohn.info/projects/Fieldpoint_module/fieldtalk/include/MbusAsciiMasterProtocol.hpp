/**
 * @internal
 * @file MbusAsciiMasterProtocol.hpp
 *
 * @if NOTICE
 *
 * $Id: MbusAsciiMasterProtocol.hpp,v 1.10 2004/01/23 22:57:08 henrik Exp $
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


#ifndef _MBUSASCIIMASTERPROTOCOL_H_INCLUDED
#define _MBUSASCIIMASTERPROTOCOL_H_INCLUDED

#ifndef __cplusplus
#  error Must use C++ to compile this module!
#endif

// Package header
#include "MbusSerialMasterProtocol.hpp"


/*****************************************************************************
 * MbusAsciiMasterProtocol class declaration
 *****************************************************************************/

/**
 * @brief Modbus ASCII Master Protocol class
 *
 * This class realizes the Modbus ASCII master protocol. It provides
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
MbusAsciiMasterProtocol: public MbusSerialMasterProtocol
{

  private:

   enum
   {
      // ASCII header length is 1 byte colon + 2 byte address/function in hex
      HDR_LEN = 1 + 2 * 2 ,
      // ASCII frame length is header + 1 byte lrc in hex encoding + cr + lf
      FRAME_LEN = HDR_LEN + 1 * 2 + 2,
      // ASCII exception message size is frame plus the code in hex encoding
      EXC_MSG_LEN = FRAME_LEN + 1 * 2,
      // ASCII max. message length is data size in hex + frame len
      MAX_MSG_SIZE = MAX_DATA_SIZE * 2 + FRAME_LEN
   };

   char bufferArr[MAX_MSG_SIZE];


  public:

   MbusAsciiMasterProtocol();

   virtual int openProtocol(const char * const portName, long baudRate,
                            int dataBits, int stopBits, int parity);

   virtual int openProtocol(const char * const portName, long baudRate);


  private:

   virtual int transceiveMessage(int address, int function,
                                 char sendDataArr[], int sendDataLen,
                                 char rcvDataArr[], int rcvDataLen);


  private:

   // Disable default operator and copy constructor
   MbusAsciiMasterProtocol &operator= (MbusAsciiMasterProtocol &);
   MbusAsciiMasterProtocol (const MbusAsciiMasterProtocol &);

};


#endif // ifdef ..._H_INCLUDED

/**
 * @internal
 * @file MbusRtuOverTcpMasterProtocol.hpp
 *
 * @if NOTICE
 *
 * $Id: MbusRtuOverTcpMasterProtocol.hpp,v 1.3 2004/01/23 22:58:15 henrik Exp $
 *
 * Copyright (c) 2003 FOCUS Software Engineering Pty Ltd, Australia.
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


#ifndef _MBUSRTUOVERMASTERPROTOCOL_H_INCLUDED
#define _MBUSRTUOVERMASTERPROTOCOL_H_INCLUDED

#ifndef __cplusplus
#  error Must use C++ to compile this module!
#endif


// Package header
#include "hmtcpip.h"
#include "MbusMasterFunctions.hpp"
#include "MbusTcpMasterProtocol.hpp"


/*****************************************************************************
 * MbusRtuOverTcpMasterProtocol class declaration
 *****************************************************************************/

/**
 * Encapsulated Modbus RTU Master Protocol class
 *
 * This class realises the Encapsulated Modbus RTU master protocol.
 * This protocol is also known as RTU over TCP or RTU/IP and used for
 * example by ISaGraf® Soft-PLCs. This class provides functions to
 * establish and to close a TCP/IP connection to the
 * slave as well as data and control functions which can be used after a connection
 * to a slave device has been established successfully. The data and
 * control functions are organized different conformance classes. For a
 * more detailed description of the data and control functions see section
 * @ref mbusmaster.
 *
 * It is also possible to instantiate multiple instances of this class for
 * establishing multiple connections to either the same or different hosts.
 *
 * @ingroup mbusmasterrtuovertcp
 * @version 1.0
 * @see mbusmaster
 * @see MbusMasterFunctions
 */
class
#if defined (_WINDLL) || defined(__DLL__)
   __declspec(dllexport)
#endif
MbusRtuOverTcpMasterProtocol: public MbusTcpMasterProtocol
{
  private:

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

   char bufferArr[MAX_MSG_SIZE];


  public:

   MbusRtuOverTcpMasterProtocol();

   int openProtocol(const char * const hostName);

   int setPort(unsigned short portNo);


  private:

   // Not support here.
   int adamSendReceiveAsciiCmd(const char * const commandSz, char* responseSz);

   int transceiveMessage(int address, int function,
                         char sendDataArr[], int sendDataLen,
                         char rcvDataArr[], int rcvDataLen);


  private:

   // Disable default operator and copy constructor
   MbusRtuOverTcpMasterProtocol &operator= (MbusRtuOverTcpMasterProtocol &);
   MbusRtuOverTcpMasterProtocol (const MbusRtuOverTcpMasterProtocol &);

};


#endif // ifdef ..._H_INCLUDED

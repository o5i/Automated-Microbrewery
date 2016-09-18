/**
 * @internal
 * @file DiagnosticDataTable.hpp
 *
 * @if NOTICE
 *
 * $Id: DiagnosticDataTable.hpp,v 1.10 2006/10/14 04:59:45 henrik Exp $
 *
 * Copyright (c) 2002-2006 FOCUS Software Engineering, Australia.
 * All rights reserved. <www.focus-sw.com>
 *
 * This file is for demonstration purposes only.
 *
 * No part of this material may be reproduced or transmitted in any
 * form or by any means or used to make any derivative work without
 * express written consent from the copyright holders.
 *
 * This material is provided "AS IS", WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. Any use is at your own risk.
 *
 * @endif
 */


#ifndef _DIAGNOSTICDATATABLE_H_INCLUDED
#define _DIAGNOSTICDATATABLE_H_INCLUDED


// Platform header
#include <stdio.h>
#include <string.h>

// Package header
#include "MbusDataTableInterface.hpp"


/*****************************************************************************
 * DiagnosticMbusDataTable class declaration
 *****************************************************************************/

/**
 * @brief This base class implements a Modbus<sup>&reg;</sup> server (or slave).
 *
 * These methods apply to all protocol flavours via inheritance. For a more
 * detailed description see section @ref mbusslave.  It provides functions
 * to start-up, run and shutdown a Modbus server.  The server processes
 * data and control functions received from a Modbus master. This
 * implementation implements all Bit Access and 16 Bits Access
 * Function Codes.  In addition some
 * frequently used Diagnostics Funtion Codes are also
 * implemented. 
 *
 * @ingroup mbusslave
 * @see MbusSlaveServer
 * @see mbusslave
 */
class DiagnosticMbusDataTable: public MbusDataTableInterface
{

public:

   DiagnosticMbusDataTable(int slaveAddr)
   {
      this->slaveAddr = slaveAddr;
      memset(regData, 0, sizeof(regData));
      memset(bitData, 0, sizeof(bitData));
   }


   ~DiagnosticMbusDataTable()
   {
   }


   char readExceptionStatus()
   {
      printf("\rSlave %3d: readExceptionStatus\n", slaveAddr);
      return (0x55);
   }


   int readInputDiscretesTable(int startRef,
                               char bitArr[],
                               int refCnt)
   {
      printf("\rSlave %3d: readInputDiscretes from %d, %d references\n",
             slaveAddr, startRef, refCnt);

      // Adjust Modbus reference counting
      startRef--;

      //
      // Validate range
      //
      if (startRef + refCnt > int(sizeof(bitData) / sizeof(char)))
         return (0);

      //
      // Copy data
      //
      memcpy(bitArr, &bitData[startRef], refCnt * sizeof(char));
      return (1);
   }


   int readCoilsTable(int startRef,
                      char bitArr[],
                      int refCnt)
   {
      printf("\rSlave %3d: readCoils from %d, %d references\n",
             slaveAddr, startRef, refCnt);

      // Adjust Modbus reference counting
      startRef--;

      //
      // Validate range
      //
      if (startRef + refCnt > int(sizeof(bitData) / sizeof(char)))
         return (0);

      //
      // Copy data
      //
      memcpy(bitArr, &bitData[startRef], refCnt * sizeof(char));
      return (1);
   }


   int writeCoilsTable(int startRef,
                       const char bitArr[],
                       int refCnt)
   {
      printf("\rSlave %3d: writeCoils from %d, %d references\n",
             slaveAddr, startRef, refCnt);

      // Adjust Modbus reference counting
      startRef--;

      //
      // Validate range
      //
      if (startRef + refCnt > int(sizeof(bitData) / sizeof(char)))
         return (0);

      //
      // Copy data
      //
      memcpy(&bitData[startRef], bitArr, refCnt * sizeof(char));
      return (1);
   }


   int readInputRegistersTable(int startRef,
                               short regArr[],
                               int refCnt)
   {
      printf("\rSlave %3d: readInputRegisters from %d, %d references\n",
             slaveAddr, startRef, refCnt);

      // Adjust Modbus reference counting
      startRef--;

      //
      // Validate range
      //
      if (startRef + refCnt > int(sizeof(regData) / sizeof(short)))
         return (0);

      //
      // Copy data
      //
      memcpy(regArr, &regData[startRef], refCnt * sizeof(short));
      return (1);
   }


   int readHoldingRegistersTable(int startRef,
                                 short regArr[],
                                 int refCnt)
   {
      printf("\rSlave %3d: readHoldingRegisters from %d, %d references\n",
             slaveAddr, startRef, refCnt);

      // Adjust Modbus reference counting
      startRef--;

      //
      // Validate range
      //
      if (startRef + refCnt > int(sizeof(regData) / sizeof(short)))
         return (0);

      //
      // Copy data
      //
      memcpy(regArr, &regData[startRef], refCnt * sizeof(short));
      return (1);
   }


   int writeHoldingRegistersTable(int startRef,
                                  const short regArr[],
                                  int refCnt)
   {
      printf("\rSlave %3d: writeHoldingRegisters from %d, %d references\n",
             slaveAddr, startRef, refCnt);

      // Adjust Modbus reference counting
      startRef--;

      //
      // Validate range
      //
      if (startRef + refCnt > int(sizeof(regData) / sizeof(short)))
         return (0);

      //
      // Copy data
      //
      memcpy(&regData[startRef], regArr, refCnt * sizeof(short));
      return (1);
   }


   int validateMasterIpAddr(char* masterIpAddrSz)
   {
      printf("\nvalidateMasterIpAddr: accepting connection from %s\n",
             masterIpAddrSz);
      return (1);
   }


  private:

   int slaveAddr;
   short regData[0x10000];
   char bitData[2000];

};


#endif // ifdef ..._H_INCLUDED

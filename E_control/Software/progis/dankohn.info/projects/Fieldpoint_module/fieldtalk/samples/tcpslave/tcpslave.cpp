/**
 * @internal
 * @file tcpslave.cpp
 *
 * @if NOTICE
 *
 * $Id: tcpslave.cpp,v 1.1 2005/02/21 01:39:49 henrik Exp $
 *
 * Copyright (c) 2002 FOCUS Software Engineering, Australia.
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


// Platform header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include FieldTalk package header
#include "MbusTcpSlaveProtocol.hpp"


/*****************************************************************************
 * Modbus data table
 *****************************************************************************/

/**
 * This structure defines a sample data table with actual data of an
 * application. All application actuals which have to be accesible via
 * Modbus have to be declared in here.
 *
 * @note Make sure to pack and word align this structure to 16-bit!
 */
typedef struct
{
   short actTemp;           // Register 1
   short minTemp;           // Register 2
   long  scanCounter;       // Register 3 and 4
   float setPoint;          // Register 5 and 6
   short statusReg;         // Register 7
   short configType;        // Register 8
} MyDeviceData;

MyDeviceData deviceData;


/*****************************************************************************
 * Data provider
 *****************************************************************************/

class MyDataProvider: public MbusDataTableInterface
{

  public:

   int readHoldingRegistersTable(int startRef, short regArr[], int refCnt)
   {
      // Adjust Modbus reference counting
      startRef--;

      //
      // Validate range
      //
      if (startRef + refCnt > int(sizeof(deviceData) / sizeof(short)))
         return (0);

      //
      // Copy data
      //
      memcpy(regArr, &((short *) &deviceData)[startRef],
             refCnt * sizeof(short));
      return (1);
   }


   int writeHoldingRegistersTable(int startRef,
                                  const short regArr[],
                                  int refCnt)
   {
      // Adjust Modbus reference counting
      startRef--;

      //
      // Validate range
      //
      if (startRef + refCnt > int(sizeof(deviceData) / sizeof(short)))
         return (0);

      //
      // Copy data
      //
      memcpy(&((short *) &deviceData)[startRef],
             regArr, refCnt * sizeof(short));
      return (1);
   }

} dataProvider;


/*****************************************************************************
 * Modbus protocol declaration
 *****************************************************************************/

MbusTcpSlaveProtocol mbusServer;


/*****************************************************************************
 * Function implementation
 *****************************************************************************/

/**
 * Starts up server
 */
void startupServer()
{
   int result;

   mbusServer.setTimeout(3000); // 3 sec time-out
   result = mbusServer.addDataTable(1, &dataProvider); // Unit ID is 1
   if (result == FTALK_SUCCESS)
      result = mbusServer.startupServer();
   if (result != FTALK_SUCCESS)
   {
      fprintf(stderr, "Error starting server: %s!\n",
              getBusProtocolErrorText(result));
      exit(EXIT_FAILURE);
   }
}


/**
 * Shutdown server
 */
void shutdownServer()
{
   mbusServer.shutdownServer();
}


/**
 * Run server
 */
void runServer()
{
   int result = FTALK_SUCCESS;

   while (result == FTALK_SUCCESS)
   {
      result = mbusServer.serverLoop();
      if (result != FTALK_SUCCESS)
         fprintf(stderr, "%s!\n", getBusProtocolErrorText(result));
   }
}


/**
 * Main function.
 *
 * @return Error code: 0 = OK, else error
 */
int main()
{
   atexit(shutdownServer);
   startupServer();
   runServer();
   return (EXIT_FAILURE);
}

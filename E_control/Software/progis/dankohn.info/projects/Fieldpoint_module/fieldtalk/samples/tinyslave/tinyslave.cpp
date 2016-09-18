/**
 * @internal
 * @file tinyslave.cpp
 *
 * @if NOTICE
 *
 * $Id: tinyslave.cpp,v 1.5 2005/01/30 03:05:01 henrik Exp $
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
#include "MbusRtuSlaveProtocol.hpp"


/*****************************************************************************
 * Gobal data
 *****************************************************************************/

#if defined(__LINUX__)
   char *portName = "/dev/ttyS0";
#elif defined(__WIN32__) || defined(__CYGWIN__)
   char *portName = "COM1";
#elif defined(__FREEBSD__) || defined(__NETBSD__) || defined(__OPENBSD__)
   char *portName = "/dev/ttyd0";
#elif defined(__QNX__)
   char *portName = "/dev/ser1";
#elif defined(__VXWORKS__)
   char *portName = "/tyCo/0";
#elif defined(__IRIX__)
   char *portName = "/dev/ttyf1";
#elif defined(__SOLARIS__)
   char *portName = "/dev/ttya";
#elif defined(__OSF__)
   char *portName = "/dev/tty00";
#else
#  error Unknown platform, please add an entry for portName
#endif


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

MbusRtuSlaveProtocol mbusServer;


/*****************************************************************************
 * Function implementation
 *****************************************************************************/

/**
 * Starts up server
 */
void startupServer()
{
   int result;

   result = mbusServer.addDataTable(1, &dataProvider);
   if (result == FTALK_SUCCESS)
      result = mbusServer.startupServer(portName,
                                        9600L, // Baudrate
                                        8,     // Databits
                                        1,     // Stopbits
                                        0);    // Parity
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

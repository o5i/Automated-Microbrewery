/**
 * @internal
 * @file sersimple.cpp
 *
 * @if NOTICE
 *
 * $Id: sersimple.cpp,v 1.5 2004/02/16 08:21:02 henrik Exp $
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

// Include FieldTalk package header
#include "MbusAsciiMasterProtocol.hpp"
#include "MbusRtuMasterProtocol.hpp"


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

//MbusAsciiMasterProtocol mbusProtocol; // Use this declaration for ASCII
MbusRtuMasterProtocol mbusProtocol; // Use this declaration for RTU


/*****************************************************************************
 * Function implementation
 *****************************************************************************/

/**
 * Opens protocol
 */
void openProtocol()
{
   int result;

   result = mbusProtocol.openProtocol(portName,
                                      9600L, // Baudrate
                                      8,     // Databits
                                      1,     // Stopbits
                                      0);    // Parity
   if (result != FTALK_SUCCESS)
   {
      fprintf(stderr, "Error opening protocol: %s!\n",
                       getBusProtocolErrorText(result));
      exit(EXIT_FAILURE);
   }
}


/**
 * Closes protocol
 */
void closeProtocol()
{
   mbusProtocol.closeProtocol();
}


/**
 * Cyclic loop which polls every one second 10 registers starting at
 * reference 100 from slave # 1
 */
void runPollLoop()
{
   short dataArr[10];

   for (;;)
   {
      int i;
      int result;

      result = mbusProtocol.readMultipleRegisters(1, 100,
                                                  dataArr,
                                                  sizeof(dataArr) / 2);
      if (result == FTALK_SUCCESS)
         for (i = 0; i < int(sizeof(dataArr) / 2); i++)
            printf("[%d]: %hd\n", 100 + i, dataArr[i]);
      else
      {
         fprintf(stderr, "%s!\n", getBusProtocolErrorText(result));
         // Stop for fatal errors
         if (!(result & FTALK_BUS_PROTOCOL_ERROR_CLASS))
            return;
      }

#ifdef __WIN32__
      Sleep(1000);
#else
      sleep(1);
#endif
   }
}


/**
 * Main function.
 *
 * @return Error code: 0 = OK, else error
 */
int main()
{
   openProtocol();

   runPollLoop();

   closeProtocol();
   return (EXIT_SUCCESS);
}

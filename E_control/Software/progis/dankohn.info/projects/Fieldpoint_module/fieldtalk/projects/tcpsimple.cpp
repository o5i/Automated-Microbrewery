/**
 * @internal
 * @file tcpsimple.cpp
 *
 * @if NOTICE
 *
 * $Id: tcpsimple.cpp,v 1.4 2003/09/10 04:24:31 henrik Exp $
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
#include "MbusTcpMasterProtocol.hpp"


/*****************************************************************************
 * Gobal data
 *****************************************************************************/

char *hostName = "192.168.0.53";
MbusTcpMasterProtocol mbusProtocol;


/*****************************************************************************
 * Function implementation
 *****************************************************************************/

/**
 * Opens protocol
 */
void openProtocol()
{
   int result;

   result = mbusProtocol.openProtocol(hostName);
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

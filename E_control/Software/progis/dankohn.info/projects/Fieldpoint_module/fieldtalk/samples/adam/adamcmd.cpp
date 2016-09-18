/**
 * @internal
 * @file adamcmd.cpp
 *
 * @if NOTICE
 *
 * $Id: adamcmd.cpp,v 1.2 2003/12/19 05:21:36 henrik Exp $
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

MbusTcpMasterProtocol mbusProtocol;


/*****************************************************************************
 * Function implementation
 *****************************************************************************/

/**
 * Opens protocol
 */
void openProtocol(char *hostName)
{
   int result;

   result = mbusProtocol.openProtocol(hostName);
   if (result != FTALK_SUCCESS)
   {
      fprintf(stderr, "Error opening connection to %s: %s!\n",
              hostName, getBusProtocolErrorText(result));
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
void execCmd(char *cmdSz)
{
   char responseBufSz[256];
   int result;

   result = mbusProtocol.adamSendReceiveAsciiCmd(cmdSz, responseBufSz);
   if (result == FTALK_SUCCESS)
   {
       printf("ADAM replied: %s\n", responseBufSz);
   }
   else
   {
       fprintf(stderr, "%s!\n", getBusProtocolErrorText(result));
       // Stop for fatal errors
       if (!(result & FTALK_BUS_PROTOCOL_ERROR_CLASS))
           return;
   }
}


/**
 * Main function
 *
 * @param argc Command line argument count
 * @param argv Command line argument value string array
 * @return Error code: 0 = OK, else error
 */
int main (int argc, char **argv)
{

   if (argc != 3)
   {
       fprintf(stderr, "Usage: adamcd ipaddr asciicmd\n");
       return (EXIT_FAILURE);
   }
   openProtocol(argv[1]);

   execCmd(argv[2]);

   closeProtocol();
   return (EXIT_SUCCESS);
}

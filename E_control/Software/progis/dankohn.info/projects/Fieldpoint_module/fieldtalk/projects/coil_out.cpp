/**
 * @internal
 * @file coil_out.cpp
 *
 * @if NOTICE
 *
 * $Id: coil_out.cpp,v 1.0 2008/06/03 Daniel Kohn
 *
 * based on tcpsimple.cpp
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
 * String constants
 *****************************************************************************/

const char versionStr[]= "$Revision: 0.99 $";
const char progName[] = "coil_out";
const char bannerStr[] =
"%s - FieldTalk(tm) Modbus(R) Coil Output Utility\n"
"Copyright (c) 2002-2004 FOCUS Software Engineering Pty Ltd\n"
#ifdef __WIN32__
"Getopt Library Copyright (C) 1987-1997	Free Software Foundation, Inc.\n"
#endif
;

const char usageStr[] =
"%s [options] host \n"
"Arguments: \n"
"host          Host name or dotted ip address when using MODBUS/TCP protocol \n"
"General options: \n"
"-a #          Slave address (1-255 for RTU/ASCII, 0-255 for TCP, 1 is default)\n"
"-b #          bit (1 to highest coil number)\n"
"-v #          value to send (0 = off, 1 = on)\n"
"";

/*****************************************************************************
 * Gobal data
 *****************************************************************************/

char *hostName;
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
 * Main function.
 *
 * @return Error code: 0 = OK, else error
 */
int main(int argc, char *argv[])
{
   int result,c, slaveAddr, bitAddr, bitVal, dataArr[10];
   



   while((c = getopt(argc, argv, "a:b:v:")) != -1)
   {
      switch(c)
      {
         case'a':
            slaveAddr=strtol(optarg, NULL, 0);
            if ((slaveAddr<0) || (slaveAddr > 255))
            {
               printf("Invalid slave address parameter (-a) \n");
               exit(EXIT_FAILURE);
            }
         break;
         case'b':
            bitAddr=strtol(optarg, NULL, 0);
            if ((bitAddr<1) || (bitAddr > 24))
            {
               printf("Invalid bit address (-b) \n");
               exit(EXIT_FAILURE);
            }   
         break;
         case'v':
            bitVal=strtol(optarg, NULL, 0);
            if ((bitVal<0) || (bitVal > 1))
            {
               printf("Invalid bit value (-v) \n");
               exit(EXIT_FAILURE);
            }   
         break;
         default:
            printf("Unrecognized option or missing option parameter\n");
            exit(EXIT_FAILURE);
         break;
      }
   }

   hostName = argv[optind];

   printf(bannerStr, progName);
   
   openProtocol();

   result=mbusProtocol.writeCoil(slaveAddr, bitAddr, bitVal);

   result=mbusProtocol.readCoils(slaveAddr, bitAddr, dataArr, 1);

   printf("%i\n",dataArr[0]);

   closeProtocol();

   return (EXIT_SUCCESS);
}

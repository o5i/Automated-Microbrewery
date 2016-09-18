/**
 * @internal
 * @file diagslave.cpp
 *
 * @if NOTICE
 *
 * $Id: diagslave.cpp,v 1.14 2006/10/17 05:50:54 henrik Exp $
 *
 * Copyright (c) 2002-2006 FOCUS Software Engineering, Australia. All rights
 * reserved. <www.focus-sw.com>
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
 * The Windows version of this program is using the GNU getopt library
 * under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE.  You find a
 * copy of the GNU Library General Public License in the file COPYING.LIB.
 *
 * @endif
 */


// Platform header
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Include FieldTalk package header
#include "MbusRtuSlaveProtocol.hpp"
#include "MbusAsciiSlaveProtocol.hpp"
#include "MbusTcpSlaveProtocol.hpp"
#include "DiagnosticDataTable.hpp"

#ifdef _WIN32
#  include "getopt.h"
#else
#  include <unistd.h>
#endif


/*****************************************************************************
 * String constants
 *****************************************************************************/

const char versionStr[]= "$Revision: 1.14 $";
const char progName[] = "diagslave";
const char bannerStr[] =
"\n"
"%s - FieldTalk(tm) Modbus(R) Diagnostic Slave\n"
"Copyright (c) 2002-2006 FOCUS Software Engineering Pty Ltd\n"
"Visit http://www.modbusdriver.com for Modbus libraries and tools.\n"
"\n";

const char usageStr[] =
"%s [options] [serialport]\n"
"Arguments: \n"
"serialport    Serial port when using Modbus ASCII or Modbus RTU protocol \n"
"              COM1, COM2 ...                on Windows \n"
"              /dev/ttyS0, /dev/ttyS1 ...    on Linux \n"
"              /dev/ser1, /dev/ser2 ...      on QNX \n"
"General options:\n"
"-m ascii      Modbus ASCII protocol\n"
"-m rtu        Modbus RTU protocol (default)\n"
"-m tcp        MODBUS/TCP protocol\n"
"-t #          Master poll time-out in ms (0-100000, 3000 is default)\n"
"-a #          Slave address (1-255 for RTU/ASCII, 0-255 for TCP)\n"
"Options for MODBUS/TCP:\n"
"-p #          TCP port number (502 is default)\n"
"Options for Modbus ASCII and Modbus RTU:\n"
"-b #          Baudrate (e.g. 9600, 19200, ...) (9600 is default)\n"
"-d #          Databits (7 or 8 for ASCII protocol, 8 for RTU)\n"
"-s #          Stopbits (1 or 2, 1 is default)\n"
"-p none       No parity\n"
"-p even       Even parity (default)\n"
"-p odd        Odd parity\n"
"";


/*****************************************************************************
 * Enums
 *****************************************************************************/

enum
{
   RTU,   ///< Modbus RTU protocol
   ASCII, ///< Modbus ASCII protocol
   TCP    ///< MODBUS/TCP protocol
};


/*****************************************************************************
 * Gobal configuration data
 *****************************************************************************/

int address = -1;
long timeout = 3000;
long baudRate = 9600;
int dataBits = 8;
int stopBits = 1;
int parity = MbusSerialSlaveProtocol::SER_PARITY_EVEN;
int protocol = RTU;
char *portName = NULL;
int port = 502;


/*****************************************************************************
 * Protocol and data table
 *****************************************************************************/

DiagnosticMbusDataTable *dataTablePtrArr[256];
MbusSlaveServer *mbusServerPtr = NULL;


/*****************************************************************************
 * Function implementation
 *****************************************************************************/

/**
 * Prints a usage message on stdout and exits
 */
void printUsage()
{
   printf("Usage: ");
   printf(usageStr, progName);
   exit(EXIT_SUCCESS);
}


/**
 * Prints version info on stdout
 */
void printVersion()
{
   printf(bannerStr, progName);
   printf("Version: %s using FieldTalk package version %s\n",
          versionStr, MbusSlaveServer::getPackageVersion());
}


/**
 * Prints the current configuration on stdout
 */
void printConfig()
{
   printf(bannerStr, progName);
   printf("Protocol configuration: ");
   switch (protocol)
   {
      case RTU:
         printf("Modbus RTU\n");
      break;
      case ASCII:
         printf("Modbus ASCII\n");
      break;
      case TCP:
         printf("MODBUS/TCP\n");
      break;
      default:
         printf("unknown\n");
      break;
   }
   printf("Slave configuration: ");
   printf("Address = %d, ", address);
   printf("Master Time-out = %ld\n", timeout);
   if (protocol == TCP)
   {
      printf("TCP configuration: ");
      printf("Port = %d\n", port);
   }
   else
   {
      printf("Serial port configuration: ");
      printf("%s, ", portName);
      printf("%ld, ", baudRate);
      printf("%d, ", dataBits);
      printf("%d, ", stopBits);
      switch (parity)
      {
         case MbusSerialSlaveProtocol::SER_PARITY_NONE:
            printf("none\n");
         break;
         case MbusSerialSlaveProtocol::SER_PARITY_EVEN:
            printf("even\n");
         break;
         case MbusSerialSlaveProtocol::SER_PARITY_ODD:
            printf("odd\n");
         break;
         default:
            printf("unknown\n");
         break;
      }
   }
   printf("\n");
}


/**
 * Prints bad option error message and exits program
 *
 * @param text Option error message
 */
void exitBadOption(const char *const text)
{
   fprintf(stderr, "%s: %s! Try -h for help.\n", progName, text);
   exit(EXIT_FAILURE);
}


/**
 * Scans and parses the command line options.
 *
 * @param argc Argument count
 * @param argv Argument value string array
 */
void scanOptions(int argc, char **argv)
{
   int c;

   // Check for --version option
   for (c = 1; c < argc; c++)
   {
      if (strcmp (argv[c], "--version") == 0)
      {
         printVersion();
         exit(EXIT_SUCCESS);
      }
   }

   // Check for --help option
   for (c = 1; c < argc; c++)
   {
      if (strcmp (argv[c], "--help") == 0)
         printUsage();
   }

   opterr = 0; // Disable getopt's error messages
   for(;;)
   {
      c = getopt(argc, argv, "ha:b:d:s:p:m:");
      if (c == -1)
         break;

      switch (c)
      {
         case 'm':
            if (strcmp(optarg, "tcp") == 0)
            {
               protocol = TCP;
            }
            else
               if (strcmp(optarg, "rtu") == 0)
               {
                  protocol = RTU;
               }
               else
                  if (strcmp(optarg, "ascii") == 0)
                  {
                     protocol = ASCII;
                  }
                  else
                  {
                     exitBadOption("Invalid protocol parameter");
                  }
         break;
         case 'a':
            address = strtol(optarg, NULL, 0);
            if ((address < -1) || (address > 255))
               exitBadOption("Invalid address parameter");
         break;
         case 't':
            timeout = strtol(optarg, NULL, 0);
            if ((timeout < 0) || (timeout > 100000))
               exitBadOption("Invalid time-out parameter");
         break;
         case 'b':
            baudRate = strtol(optarg, NULL, 0);
            if (baudRate == 0)
               exitBadOption("Invalid baudrate parameter");
         break;
         case 'd':
            dataBits = (int) strtol(optarg, NULL, 0);
            if ((dataBits != 7) || (dataBits != 8))
               exitBadOption("Invalid databits parameter");
         break;
         case 's':
            stopBits = (int) strtol(optarg, NULL, 0);
            if ((stopBits != 1) || (stopBits != 2))
               exitBadOption("Invalid stopbits parameter");
         break;
         case 'p':
            if (strcmp(optarg, "none") == 0)
            {
               parity = MbusSerialSlaveProtocol::SER_PARITY_NONE;
            }
            else
               if (strcmp(optarg, "odd") == 0)
               {
                  parity = MbusSerialSlaveProtocol::SER_PARITY_ODD;
               }
               else
                  if (strcmp(optarg, "even") == 0)
                  {
                     parity = MbusSerialSlaveProtocol::SER_PARITY_EVEN;
                  }
                  else
                  {
                     port = strtol(optarg, NULL, 0);
                     if ((port <= 0) || (port > 0xFFFF))
                        exitBadOption("Invalid parity or port parameter");
                  }
         break;
         case 'h':
            printUsage();
         break;
         default:
            exitBadOption("Unrecognized option or missing option parameter");
         break;
      }
   }

   if (protocol == TCP)
   {
      if ((argc - optind) != 0)
         exitBadOption("Invalid number of parameters");
   }
   else
   {
      if ((argc - optind) != 1)
         exitBadOption("Invalid number of parameters");
      else
         portName = argv[optind];
   }
}




/**
 * Callback function which cecks a master's IP address and
 * either accepts or rejects a master's connection.
 *
 * @param masterIpAddrSz IPv4 Internet host address string
 * in the standard numbers-and-dots notation.
 *
 * @return Returns 1 to accept a connection or 0 to reject it.
 */
int validateMasterIpAddr(char* masterIpAddrSz)
{
   printf("\nvalidateMasterIpAddr: accepting connection from %s\n",
          masterIpAddrSz);
   return (1);
}


/**
 * Starts up server
 */
void startupServer()
{
   int i;
   int result = -1;

   switch (protocol)
   {
      case RTU:
         mbusServerPtr = new MbusRtuSlaveProtocol();
         if (address == -1)
         {
            for (i = 1; i < 255; i++)
            mbusServerPtr->addDataTable(i, dataTablePtrArr[i]);
         }
         else
            mbusServerPtr->addDataTable(address, dataTablePtrArr[address]);
         mbusServerPtr->setTimeout(timeout);
         result = ((MbusRtuSlaveProtocol *) mbusServerPtr)->startupServer(
                    portName, baudRate, dataBits, stopBits, parity);
      break;
      case ASCII:
         mbusServerPtr = new MbusAsciiSlaveProtocol();
         if (address == -1)
         {
            for (i = 1; i < 255; i++)
            mbusServerPtr->addDataTable(i, dataTablePtrArr[i]);
         }
         else
            mbusServerPtr->addDataTable(address, dataTablePtrArr[address]);
         mbusServerPtr->setTimeout(timeout);
         result = ((MbusAsciiSlaveProtocol *) mbusServerPtr)->startupServer(
                   portName, baudRate, dataBits, stopBits, parity);
      break;
      case TCP:
         mbusServerPtr = new MbusTcpSlaveProtocol();
         if (address == -1)
         {
            for (i = 0; i < 255; i++) // Note: TCP support slave addres of 0
               mbusServerPtr->addDataTable(i, dataTablePtrArr[i]);
         }
         else
            mbusServerPtr->addDataTable(address, dataTablePtrArr[address]);
         mbusServerPtr->setTimeout(timeout);
         ((MbusTcpSlaveProtocol *) mbusServerPtr)->installIpAddrValidationCallBack(validateMasterIpAddr);
         ((MbusTcpSlaveProtocol *) mbusServerPtr)->setPort((unsigned short) port);
         result = ((MbusTcpSlaveProtocol *) mbusServerPtr)->startupServer();
      break;
   }
   switch (result)
   {
      case FTALK_SUCCESS:
         printf("Server started up successfully.\n");
      break;
      case FTALK_ILLEGAL_ARGUMENT_ERROR:
         fprintf(stderr, "Configuration setting not supported!\n");
         exit(EXIT_FAILURE);
      break;
      default:
         fprintf(stderr, "%s!\n", getBusProtocolErrorText(result));
         exit(EXIT_FAILURE);
      break;
   }
}


/**
 * Shutdown server
 */
void shutdownServer()
{
   printf("Shutting down server.\n");
   delete mbusServerPtr;
}


/**
 * Run server
 */
void runServer()
{
   int result = FTALK_SUCCESS;

   printf("Listening to network (Ctrl-C to stop)\n");
   while (result == FTALK_SUCCESS)
   {
      result = mbusServerPtr->serverLoop();
      if (result != FTALK_SUCCESS)
         fprintf(stderr, "%s!\n", getBusProtocolErrorText(result));\
      else
      {
         printf(".");
         fflush(stdout);
      }
   }
}


/**
 * Main function
 *
 * @param argc Command line argument count
 * @param argv Command line argument value string array
 * @return Error code: 0 = OK, else error
 */
int main(int argc, char **argv)
{
   int i;

   // Construct data tables
   for (i = 0; i < 255; i++)
   {
      dataTablePtrArr[i] = new DiagnosticMbusDataTable(i);
   }

   scanOptions(argc, argv);
   printConfig();
   atexit(shutdownServer);
   startupServer();
   runServer();
   return (EXIT_FAILURE);
}

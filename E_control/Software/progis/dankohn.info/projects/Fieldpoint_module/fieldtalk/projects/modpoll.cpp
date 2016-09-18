/**
 * @internal
 * @file modpoll.cpp
 *
 * @if NOTICE
 *
 * $Id: modpoll.cpp,v 1.15 2004/05/26 01:39:07 henrik Exp $
 *
 * Copyright (c) 2002-2004 FOCUS Software Engineering Pty Ltd, Australia.
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
#include "MbusRtuMasterProtocol.hpp"
#include "MbusAsciiMasterProtocol.hpp"
#include "MbusTcpMasterProtocol.hpp"
#include "MbusRtuOverTcpMasterProtocol.hpp"

// Provide getopt on Win32 without using a separate lib
#ifdef __WIN32__
#  include "getopt.c"
#else
#  include <unistd.h>
#endif


/*****************************************************************************
 * String constants
 *****************************************************************************/

const char versionStr[]= "$Revision: 1.15 $";
const char progName[] = "modpoll";
const char bannerStr[] =
"%s - FieldTalk(tm) Modbus(R) Polling Utility\n"
"Copyright (c) 2002-2004 FOCUS Software Engineering Pty Ltd\n"
#ifdef __WIN32__
"Getopt Library Copyright (C) 1987-1997	Free Software Foundation, Inc.\n"
#endif
;

const char usageStr[] =
"%s [options] serialport|host \n"
"Arguments: \n"
"serialport    Serial port when using Modbus ASCII or Modbus RTU protocol \n"
"              /dev/ttyS0, /dev/ttyS1 ...    on Linux \n"
"              COM1, COM2 ...                on Win32 \n"
"              /dev/ser1, /dev/ser2 ...      on QNX \n"
"host          Host name or dotted ip address when using MODBUS/TCP protocol \n"
"General options: \n"
"-m ascii      Modbus ASCII protocol\n"
"-m rtu        Modbus RTU protocol (default)\n"
"-m tcp        MODBUS/TCP protocol\n"
"-m enc        Encapsulated Modbus RTU over TCP\n"
"-a #          Slave address (1-255 for RTU/ASCII, 0-255 for TCP, 1 is default)\n"
"-r #          Start reference (1-65536, 100 is default)\n"
"-c #          Number of values to poll (1-100, 1 is default)\n"
"-t 0          Discrete output (coil) data type\n"
"-t 1          Discrete input data type\n"
"-t 3          16-bit input register data type\n"
"-t 3:hex      16-bit input register data type with hex display\n"
"-t 3:int      32-bit integer data type in input register table\n"
"-t 3:mod      32-bit module 10000 data type in input register table\n"
"-t 3:float    32-bit float data type in input register table\n"
"-t 4          16-bit output (holding) register data type (default)\n"
"-t 4:hex      16-bit output (holding) register data type with hex display\n"
"-t 4:int      32-bit integer data type in output (holding) register table\n"
"-t 4:mod      32-bit module 10000 type in output (holding) register table\n"
"-t 4:float    32-bit float data type in output (holding) register table\n"
"-i            Slave operates on big-endian 32-bit integers\n"
"-f            Slave operates on big-endian 32-bit floats\n"
"-1            Poll only once, otherwise poll every second\n"
"Options for MODBUS/TCP:\n"
"-p #          TCP port number (502 is default)\n"
"Options for Modbus ASCII and Modbus RTU:\n"
"-b #          Baudrate (e.g. 9600, 19200, ...) (9600 is default)\n"
"-d #          Databits (7 or 8 for ASCII protocol, 8 for RTU)\n"
"-s #          Stopbits (1 or 2, 1 is default)\n"
"-p none       No parity (default)\n"
"-p even       Even parity\n"
"-p odd        Odd parity\n"
"-4 #          RS485 mode, RTS on while transmitting and another # ms after.\n"
"";


/*****************************************************************************
 * Enums
 *****************************************************************************/

enum
{
   RTU,       ///< Modbus RTU protocol
   ASCII,     ///< Modbus ASCII protocol
   TCP,       ///< MODBUS/TCP protocol
   RTUOVERTCP ///< Encapsulated RTU over TCP
};

enum
{
   T0_BOOL,     ///< boolean data types, discrete output table
   T1_BOOL,     ///< boolean data types, discrete input table
   T3_REG16,    ///< 16-bit register (short) types, input table
   T3_HEX16,    ///< 16-bit register types with hex display, input table
   T3_INT32,    ///< 32-bit integer types (2 16-bit registers), input table
   T3_MOD10000, ///< 32-bit modulo 10000 types, input table
   T3_FLOAT32,  ///< Float types (2 16-bit registers), input table
   T4_REG16,    ///< 16-bit register (short) types, output table
   T4_HEX16,    ///< 16-bit register types with hex display, output table
   T4_INT32,    ///< 32-bit integer types (2 16-bit registers), output table
   T4_MOD10000, ///< 32-bit modulo 10000 types, output table
   T4_FLOAT32   ///< Float types (2 16-bit registers), output table
};


/*****************************************************************************
 * Gobal configuration data
 *****************************************************************************/

int address = 1;
int ref = 100;
int refCnt = 1;
int pollCnt = -1;
long baudRate = 9600;
int dataBits = 8;
int stopBits = 1;
int parity = MbusSerialMasterProtocol::SER_PARITY_NONE;
int protocol = RTU;
int dataType = T4_REG16;
int swapInts = 0;
int swapFloats = 0;
char *portName = NULL;
int port = 502;
int rs485Mode = 0;


/*****************************************************************************
 * Protocol and data pointers
 *****************************************************************************/

MbusMasterFunctions *mbusPtr = NULL;
void *dataPtr = NULL;


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
          versionStr, MbusMasterFunctions::getPackageVersion());
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
      case RTUOVERTCP:
         printf("Encapsulated RTU over TCP\n");
      break;
      default:
         printf("unknown\n");
      break;
   }
   printf("Slave configuration: ");
   printf("Address = %d, ", address);
   printf("start reference = %d, ", ref);
   printf("count = %d\n", refCnt);
   if ((protocol == TCP) || (protocol == RTUOVERTCP))
   {
      printf("TCP/IP configuration: ");
      printf("Host = %s, ", portName);
      printf("port = %d\n", port);
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
         case MbusSerialMasterProtocol::SER_PARITY_NONE:
            printf("none\n");
         break;
         case MbusSerialMasterProtocol::SER_PARITY_EVEN:
            printf("even\n");
         break;
         case MbusSerialMasterProtocol::SER_PARITY_ODD:
            printf("odd\n");
         break;
         default:
            printf("unknown\n");
         break;
      }
   }
   printf("Data type: ");
   switch (dataType)
   {
      case T0_BOOL:
         printf("discrete output (coil)\n");
      break;
      case T1_BOOL:
         printf("discrete input\n");
      break;
      case T3_REG16:
         printf("16-bit register, input register table\n");
      break;
      case T3_HEX16:
         printf("16-bit register (hex), input register table\n");
      break;
      case T3_INT32:
         printf("32-bit integer, input register table\n");
      break;
      case T3_MOD10000:
         printf("32-bit module 10000, input register table\n");
      break;
      case T3_FLOAT32:
         printf("32-bit float, input register table\n");
      break;
      case T4_REG16:
         printf("16-bit register, output (holding) register table\n");
      break;
      case T4_HEX16:
         printf("16-bit register (hex), output (holding) register table\n");
      break;
      case T4_INT32:
         printf("32-bit integer, output (holding) register table\n");
      break;
      case T4_MOD10000:
         printf("32-bit module 10000, output (holding) register table\n");
      break;
      case T4_FLOAT32:
         printf("32-bit float, output (holding) register table\n");
      break;
      default:
         printf("unknown\n");
      break;
   }
   if (swapInts || swapFloats)
   {
      printf("Word swapping: Slave configured as big-endian");
      if (swapInts)
         printf(" word");
      if (swapInts && swapFloats)
         printf(" and");
      if (swapFloats)
         printf(" float");
      printf(" machine\n");
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
   for (;;)
   {
      c = getopt(argc, argv, "h14:fa:r:c:b:d:s:p:t:m:");
      if (c == -1)
         break;

      switch (c)
      {
         case '1':
            pollCnt = 1;
         break;
         case '4':
            rs485Mode = (int) strtol(optarg, NULL, 0);
            if ((rs485Mode <= 0) || (rs485Mode > 1000))
               exitBadOption("Invalid RTS delay parameter");
         break;
         case 'i':
            swapInts = 1;
         break;
         case 'f':
            swapFloats = 1;
         break;
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
                     if (strcmp(optarg, "enc") == 0)
                     {
                        protocol = RTUOVERTCP;
                     }
                     else
                     {
                        exitBadOption("Invalid protocol parameter");
                     }
         break;
         case 'a':
            address = strtol(optarg, NULL, 0);
            if ((address < 0) || (address > 255))
               exitBadOption("Invalid address parameter");
         break;
         case 'r':
            ref = strtol(optarg, NULL, 0);
             if ((ref <= 0) || (ref > 0x10000))
               exitBadOption("Invalid reference parameter");
         break;
         case 'c':
            refCnt = strtol(optarg, NULL, 0);
            if ((refCnt <= 0) || (refCnt >= 100))
               exitBadOption("Invalid count parameter");
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
               parity = MbusSerialMasterProtocol::SER_PARITY_NONE;
            }
            else
               if (strcmp(optarg, "odd") == 0)
               {
                  parity = MbusSerialMasterProtocol::SER_PARITY_ODD;
               }
               else
                  if (strcmp(optarg, "even") == 0)
                  {
                     parity = MbusSerialMasterProtocol::SER_PARITY_EVEN;
                  }
                  else
                  {
                     port = strtol(optarg, NULL, 0);
                     if ((port <= 0) || (port > 0xFFFF))
                        exitBadOption("Invalid parity or port parameter");
                  }
         break;
         case 't':
            if (strcmp(optarg, "0") == 0)
            {
               dataType = T0_BOOL;
            }
            else
            if (strcmp(optarg, "1") == 0)
            {
               dataType = T1_BOOL;
            }
            else
            if (strcmp(optarg, "3") == 0)
            {
               dataType = T3_REG16;
            }
            else
            if (strcmp(optarg, "3:hex") == 0)
            {
               dataType = T3_HEX16;
            }
            else
            if (strcmp(optarg, "3:int") == 0)
            {
               dataType = T3_INT32;
            }
            else
            if (strcmp(optarg, "3:mod") == 0)
            {
               dataType = T3_MOD10000;
            }
            else
            if (strcmp(optarg, "3:float") == 0)
            {
               dataType = T3_FLOAT32;
            }
            else
            if (strcmp(optarg, "4") == 0)
            {
               dataType = T4_REG16;
            }
            else
            if (strcmp(optarg, "4:hex") == 0)
            {
               dataType = T4_HEX16;
            }
            else
            if (strcmp(optarg, "4:int") == 0)
            {
               dataType = T4_INT32;
            }
            else
            if (strcmp(optarg, "4:mod") == 0)
            {
               dataType = T4_MOD10000;
            }
            else
            if (strcmp(optarg, "4:float") == 0)
            {
               dataType = T4_FLOAT32;
            }
            else
            {
               exitBadOption("Invalid data type parameter");
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

   if ((argc - optind) != 1)
         exitBadOption("Invalid number of parameters");
   else
      portName = argv[optind];
}


/**
 * Opens protocol and allocates data memory
 */
void openProtocol()
{
   int result = -1;

   switch (protocol)
   {
      case RTU:
         mbusPtr = new MbusRtuMasterProtocol();
         if (!mbusPtr)
         {
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
         }
         if (swapInts)
            mbusPtr->configureBigEndianInts();
         if (swapFloats)
            mbusPtr->configureSwappedFloats();
         mbusPtr->setRetryCnt(2);
         mbusPtr->setPollDelay(1000);
         if (rs485Mode > 0)
            ((MbusAsciiMasterProtocol *) mbusPtr)->enableRs485Mode(rs485Mode);
         result = ((MbusRtuMasterProtocol *) mbusPtr)->openProtocol(
                   portName, baudRate, dataBits, stopBits, parity);
      break;
      case ASCII:
         mbusPtr = new MbusAsciiMasterProtocol();
         if (!mbusPtr)
         {
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
         }
         if (swapInts)
            mbusPtr->configureBigEndianInts();
         if (swapFloats)
            mbusPtr->configureSwappedFloats();
         mbusPtr->setRetryCnt(2);
         mbusPtr->setPollDelay(1000);
         if (rs485Mode > 0)
            ((MbusAsciiMasterProtocol *) mbusPtr)->enableRs485Mode(rs485Mode);
         result = ((MbusAsciiMasterProtocol *) mbusPtr)->openProtocol(
                   portName, baudRate, dataBits, stopBits, parity);
      break;
      case TCP:
         mbusPtr = new MbusTcpMasterProtocol();
         if (!mbusPtr)
         {
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
         }
         if (swapInts)
            mbusPtr->configureBigEndianInts();
         if (swapFloats)
            mbusPtr->configureSwappedFloats();
         mbusPtr->setPollDelay(1000);
         ((MbusTcpMasterProtocol *) mbusPtr)->setPort(port);
         result = ((MbusTcpMasterProtocol *) mbusPtr)->openProtocol(portName);
      break;
      case RTUOVERTCP:
         mbusPtr = new MbusRtuOverTcpMasterProtocol();
         if (!mbusPtr)
         {
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
         }
         if (swapInts)
            mbusPtr->configureBigEndianInts();
         if (swapFloats)
            mbusPtr->configureSwappedFloats();
         mbusPtr->setPollDelay(1000);
         ((MbusRtuOverTcpMasterProtocol *) mbusPtr)->setPort(port);
         result = ((MbusRtuOverTcpMasterProtocol *) mbusPtr)->openProtocol(portName);
      break;
   }
   switch (result)
   {
      case FTALK_SUCCESS:
         printf("Protocol opened successfully.\n");
      break;
      case FTALK_ILLEGAL_ARGUMENT_ERROR:
         fprintf(stderr, "Configuration setting not supported!\n");
         exit(EXIT_FAILURE);
      break;
      case FTALK_TCPIP_CONNECT_ERR:
         fprintf(stderr, "Can't reach slave (check ip address)!\n");
         exit(EXIT_FAILURE);
      break;
      default:
         fprintf(stderr, "%s!\n", getBusProtocolErrorText(result));
         exit(EXIT_FAILURE);
      break;
   }

   switch (dataType)
   {
      case T3_HEX16:
      case T3_REG16:
      case T4_HEX16:
      case T4_REG16:
         dataPtr = new short[refCnt];
      break;
      case T0_BOOL:
      case T1_BOOL:
      case T3_INT32:
      case T4_INT32:
      case T3_MOD10000:
      case T4_MOD10000:
         dataPtr = new int[refCnt];
      break;
      case T3_FLOAT32:
      case T4_FLOAT32:
         dataPtr = new float[refCnt];
      break;
   }
   if (!dataPtr)
   {
      fprintf(stderr, "Out of memory!\n");
      exit(EXIT_FAILURE);
   }
}


/**
 * Closes protocol
 */
void closeProtocol()
{
   delete mbusPtr;
   delete [] dataPtr;
}


/**
 * Poll slave device
 */
void pollSlave()
{
   int i;
   int result = -1;

   while ((pollCnt == -1) || (pollCnt > 0))
   {
      if (pollCnt == -1)
         printf("Polling slave (Ctrl-C to stop) ...\n");
      else
      {
         printf("Polling slave ...\n");
         pollCnt--;
      }
      switch (dataType)
      {
         case T0_BOOL:
            result = mbusPtr->readCoils(address, ref,
                                        (int *) dataPtr, refCnt);
            if (result == FTALK_SUCCESS)
               for (i = 0; i < refCnt; i++)
                  printf("[%d]: %d\n", ref + i, ((int *) dataPtr)[i]);
         break;
         case T1_BOOL:
            result = mbusPtr->readInputDiscretes(address, ref,
                                                 (int *) dataPtr, refCnt);
            if (result == FTALK_SUCCESS)
               for (i = 0; i < refCnt; i++)
                  printf("[%d]: %d\n", ref + i, ((int *) dataPtr)[i]);
         break;
         case T4_REG16:
            result = mbusPtr->readMultipleRegisters(address, ref,
                                                    (short *) dataPtr, refCnt);
            if (result == FTALK_SUCCESS)
               for (i = 0; i < refCnt; i++)
                  printf("[%d]: %hd\n", ref + i, ((short *) dataPtr)[i]);
         break;
         case T4_HEX16:
            result = mbusPtr->readMultipleRegisters(address, ref,
                                                    (short *) dataPtr, refCnt);
            if (result == FTALK_SUCCESS)
               for (i = 0; i < refCnt; i++)
                  printf("[%d]: 0x%04hX\n", ref + i, ((short *) dataPtr)[i]);
         break;
         case T4_INT32:
            result = mbusPtr->readMultipleLongInts(address, ref,
                                                   (long *) dataPtr, refCnt);
            if (result == FTALK_SUCCESS)
               for (i = 0; i < refCnt; i++)
                  printf("[%d]: %d\n", ref + i * 2, ((int *) dataPtr)[i]);
         break;
         case T4_MOD10000:
            result = mbusPtr->readMultipleMod10000(address, ref,
                                                   (long *) dataPtr, refCnt);
            if (result == FTALK_SUCCESS)
               for (i = 0; i < refCnt; i++)
                  printf("[%d]: %d\n", ref + i * 2, ((int *) dataPtr)[i]);
         break;
         case T4_FLOAT32:
            result = mbusPtr->readMultipleFloats(address, ref,
                                                 (float *) dataPtr, refCnt);
            if (result == FTALK_SUCCESS)
               for (i = 0; i < refCnt; i++)
                  printf("[%d]: %f\n", ref + i * 2, ((float *) dataPtr)[i]);
         break;
         case T3_REG16:
            result = mbusPtr->readInputRegisters(address, ref,
                                                 (short *) dataPtr, refCnt);
            if (result == FTALK_SUCCESS)
               for (i = 0; i < refCnt; i++)
                  printf("[%d]: %hd\n", ref + i, ((short *) dataPtr)[i]);
         break;
         case T3_HEX16:
            result = mbusPtr->readInputRegisters(address, ref,
                                                 (short *) dataPtr, refCnt);
            if (result == FTALK_SUCCESS)
               for (i = 0; i < refCnt; i++)
                  printf("[%d]: 0x%04hX\n", ref + i, ((short *) dataPtr)[i]);
         break;
         case T3_INT32:
            result = mbusPtr->readInputLongInts(address, ref,
                                                (long *) dataPtr, refCnt);
            if (result == FTALK_SUCCESS)
               for (i = 0; i < refCnt; i++)
                  printf("[%d]: %d\n", ref + i * 2, ((int *) dataPtr)[i]);
         break;
         case T3_MOD10000:
            result = mbusPtr->readInputMod10000(address, ref,
                                                (long *) dataPtr, refCnt);
            if (result == FTALK_SUCCESS)
               for (i = 0; i < refCnt; i++)
                  printf("[%d]: %d\n", ref + i * 2, ((int *) dataPtr)[i]);
         break;
         case T3_FLOAT32:
            result = mbusPtr->readInputFloats(address, ref,
                                              (float *) dataPtr, refCnt);
            if (result == FTALK_SUCCESS)
               for (i = 0; i < refCnt; i++)
                  printf("[%d]: %f\n", ref + i * 2, ((float *) dataPtr)[i]);
         break;
      }
      if (result != FTALK_SUCCESS)
      {
         fprintf(stderr, "%s!\n", getBusProtocolErrorText(result));
         // Stop for fatal errors
         if (!(result & FTALK_BUS_PROTOCOL_ERROR_CLASS))
            return;
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
int main (int argc, char **argv)
{
   scanOptions(argc, argv);
   printConfig();
   atexit(closeProtocol);
   openProtocol();
   pollSlave();
   return (EXIT_SUCCESS);
}

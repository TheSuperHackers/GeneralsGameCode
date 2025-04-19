/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*************************************************************************** 
 *                                                                         * 
 *                 Project Name : Westwood Auto Registration App           * 
 *                                                                         * 
 *                    File Name : FIELD.CPP                                * 
 *                                                                         * 
 *                   Programmer : Philip W. Gorrow                         * 
 *                                                                         * 
 *                   Start Date : 04/22/96                                 * 
 *                                                                         * 
 *                  Last Update : April 22, 1996 [PWG]                     * 
 *                                                                         * 
 *  Actual member function for the field class.                            * 
 *-------------------------------------------------------------------------* 
 * Functions:                                                              * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include <string.h>
#include <sys/types.h>
#ifndef _WINDOWS
#include <netinet/in.h>
#else
#define Win32_Winsock
#include <windows.h>
#endif
#include "field.h"


// private member func
void FieldClass::Clear(void)
{
  delete[](Data);

  strcpy(ID,"");
  DataType=0;
  Size=0;
  Data=NULL;
  Next=NULL;
}


FieldClass::FieldClass(char *id, char data)
{
  Data=NULL;
  Set(id,data);
}
 
FieldClass::FieldClass(char *id, unsigned char data)
{
  Data=NULL;
  Set(id,data);
}
 
FieldClass::FieldClass(char *id, short data)
{
  Data=NULL;
  Set(id,data);
}
 
FieldClass::FieldClass(char *id, unsigned short data)
{
  Data=NULL;
  Set(id,data);
}
 
FieldClass::FieldClass(char *id, long data)
{
  Data=NULL;
  Set(id,data);
}
 
FieldClass::FieldClass(char *id, unsigned long data)
{
  Data=NULL;
  Set(id,data);
}
 
FieldClass::FieldClass(char *id, char *data)
{
  Data=NULL;
  Set(id,data);
}

FieldClass::FieldClass(char *id, void *data, int length)
{
  Data=NULL;
  Set(id,data,length);
}

void FieldClass::Set(char *id, char data)
{
  FieldClass     *Nextsave=Next;

  Clear();
  strncpy(ID, id, sizeof(ID));
  DataType      = TYPE_CHAR;
  Size		= sizeof(data);
  Data		= new char[Size];
  memcpy(Data, &data, Size);
  Next		= Nextsave;
}

void FieldClass::Set(char *id, unsigned char data)
{
  FieldClass     *Nextsave=Next;
  Clear();
  strncpy(ID, id, sizeof(ID));
  DataType = TYPE_UNSIGNED_CHAR;
  Size     = sizeof(data);
  Data     = new char[Size];
  memcpy(Data, &data, Size);
  Next     = Nextsave;
}

void FieldClass::Set(char *id, short data)
{
  FieldClass     *Nextsave=Next;
  Clear();
  strncpy(ID, id, sizeof(ID));
  DataType = TYPE_SHORT;
  Size    = sizeof(data);
  Data    = new char[Size];
  memcpy(Data, &data, Size);
  Next    = Nextsave;
}

void FieldClass::Set(char *id, unsigned short data)
{
  FieldClass     *Nextsave=Next;
  Clear();
  strncpy(ID, id, sizeof(ID));
  DataType = TYPE_UNSIGNED_SHORT;
  Size    = sizeof(data);
  Data    = new char[Size];
  memcpy(Data, &data, Size);
  Next    = Nextsave;
}

void FieldClass::Set(char *id, long data)
{
  FieldClass     *Nextsave=Next;
  Clear();
  strncpy(ID, id, sizeof(ID));
  DataType = TYPE_LONG;
  Size    = sizeof(data);
  Data    = new char[Size];
  memcpy(Data, &data, Size);
  Next    = Nextsave;
}

void FieldClass::Set(char *id, unsigned long data)
{
  FieldClass     *Nextsave=Next;
  Clear();
  strncpy(ID, id, sizeof(ID));
  DataType = TYPE_UNSIGNED_LONG;
  Size    = sizeof(data);
  Data    = new char[Size];
  memcpy(Data, &data, Size);
  Next    = Nextsave;
}

void FieldClass::Set(char *id, char *data)
{
  FieldClass     *Nextsave=Next;
  Clear();
  strncpy(ID, id, sizeof(ID));
  DataType = TYPE_STRING;
  Size    = (unsigned short)(strlen(data)+1);
  Data    = new char[Size];
  memcpy(Data, data, Size);
  Next    = Nextsave;
}


void FieldClass::Set(char *id, void *data, int length)
{
  FieldClass     *Nextsave=Next;
  Clear();
  strncpy(ID, id, sizeof(ID));
  DataType = TYPE_CHUNK;
  Size    = (unsigned short)length;
  Data    = new char[Size];
  memcpy(Data, data, Size);
  Next    = Nextsave;
}


FieldClass::~FieldClass()
{
  Clear();
}

// Fetch the datatype
int FieldClass::Get_Type(void)
{
  return(DataType);
}

void *FieldClass::Get_Data(void)
{
return(Data);
}

char *FieldClass::Get_ID(void)
{
return(ID);
}

/************************************************************************** 
 * PACKETCLASS::HOST_TO_NET_FIELD -- Converts host field to net format    *
 *                                                                        * 
 * INPUT:    FIELD   * to the data field we need to convert               *
 *                                                                        * 
 * OUTPUT:     none                                                       *
 *                                                                        * 
 * HISTORY:                                                               * 
 *   04/22/1996 PWG : Created.                                            * 
 *========================================================================*/
void FieldClass::Host_To_Net(void)
{
  //
  // Before we convert the data type, we should convert the actual data 
  //  sent.
  //
  switch (DataType) {
    case TYPE_CHAR:
    case TYPE_UNSIGNED_CHAR:
    case TYPE_STRING:
      break;

    case TYPE_SHORT:
    case TYPE_UNSIGNED_SHORT:
      *((unsigned short *)Data) = htons(*((unsigned short *)Data));
      break;

    case TYPE_LONG:
    case TYPE_UNSIGNED_LONG:
      *((unsigned long *)Data) = htonl(*((unsigned long *)Data));
      break;

    //
    // Might be good to insert some type of error message here for unknown
    //   datatypes -- but will leave that for later.
    //
    default:
      break;
  }
  //
  // Finally convert over the data type and the size of the packet.
  //
  DataType = htons(DataType);
  Size      = htons(Size);
}

/************************************************************************** 
 * PACKETCLASS::NET_TO_HOST_FIELD -- Converts net field to host format    *
 *                                                                        * 
 * INPUT:    FIELD   * to the data field we need to convert               *
 *                                                                        * 
 * OUTPUT:     none                                                       *
 *                                                                        * 
 * HISTORY:                                                               * 
 *   04/22/1996 PWG : Created.                                            * 
 *========================================================================*/
void FieldClass::Net_To_Host(void)
{
  //
  // Finally convert over the data type and the size of the packet.
  //
  DataType = ntohs(DataType);
  Size      = ntohs(Size);

  //
  // Before we convert the data type, we should convert the actual data 
  //  sent.
  //
  switch (DataType) {
    case TYPE_CHAR:
    case TYPE_UNSIGNED_CHAR:
    case TYPE_STRING:
      break;

    case TYPE_SHORT:
    case TYPE_UNSIGNED_SHORT:
      *((unsigned short *)Data) = ntohs(*((unsigned short *)Data));
      break;

    case TYPE_LONG:
    case TYPE_UNSIGNED_LONG:
      *((unsigned long *)Data) = ntohl(*((unsigned long *)Data));
      break;

    //
    // Might be good to insert some type of error message here for unknown
    //   datatypes -- but will leave that for later.
    //
    default:
      break;
  }
}


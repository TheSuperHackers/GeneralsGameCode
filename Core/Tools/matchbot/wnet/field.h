/*
**	Command & Conquer Generals Zero Hour(tm)
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
 *                    File Name : FIELD.H                                  * 
 *                                                                         * 
 *                   Programmer : Philip W. Gorrow                         * 
 *                                                                         * 
 *                   Start Date : 04/22/96                                 * 
 *                                                                         * 
 *                  Last Update : April 22, 1996 [PWG]                     * 
 *                                                                         * 
 * This module takes care of maintaining the field list used to process    *
 * packets.                                                                *
 *                                                                         * 
 *-------------------------------------------------------------------------* 
 * Functions:                                                              * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define FIELD_HEADER_SIZE	(sizeof(FieldClass) - (sizeof(void *) * 2))

#define TYPE_CHAR                       1
#define TYPE_UNSIGNED_CHAR              2
#define TYPE_SHORT                      3
#define TYPE_UNSIGNED_SHORT             4
#define TYPE_LONG                       5
#define TYPE_UNSIGNED_LONG              6
#define TYPE_STRING                     7
#define TYPE_CHUNK                      20

class PacketClass;

class FieldClass
{
 public:
   friend PacketClass;
   //
   // Define constructors to be able to create all the different kinds
   // of fields.
   //
   FieldClass(void) {};
   FieldClass(char *id, char data);
   FieldClass(char *id, unsigned char data);
   FieldClass(char *id, short data);
   FieldClass(char *id, unsigned short data);
   FieldClass(char *id, long data);
   FieldClass(char *id, unsigned long data);
   FieldClass(char *id, char *data);
   FieldClass(char *id, void *data, int length);

   ~FieldClass();

   // Change the field contents
   void Set(char *id, char data);
   void Set(char *id, unsigned char data);
   void Set(char *id, short data);
   void Set(char *id, unsigned short data);
   void Set(char *id, long data);
   void Set(char *id, unsigned long data);
   void Set(char *id, char *data);
   void Set(char *id, void *data, int length);

   int             Get_Type(void);    // get the datatype of this field
   unsigned short  Get_Size(void)  { return Size; }
   void *             Get_Data(void);    // get the datatype of this field
   char *             Get_ID(void);    // get the datatype of this field

   void            Host_To_Net(void);
   void            Net_To_Host(void);

 private:

   void            Clear(void);        // dealloc mem & zero safely

   char            ID[4];              // id value of this field
   unsigned short  DataType;           // id of the data type we are using
   unsigned short  Size;               // size of the data portion of this field
   char           *Data;               // pointer to the data portion of this field
   FieldClass     *Next;               // pointer to the next field in the field list
};




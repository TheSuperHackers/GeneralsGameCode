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

/* $Header: /VSS_Sync/wwlib/argv.h 7     8/29/01 10:25p Vss_sync $ */
/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Library                                                      * 
 *                                                                                             * 
 *                     $Archive:: /VSS_Sync/wwlib/argv.h                                      $* 
 *                                                                                             * 
 *                      $Author:: Vss_sync                                                    $* 
 *                                                                                             * 
 *                     $Modtime:: 8/29/01 10:24p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 7                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
					
#if defined(_MSC_VER)
#pragma once
#endif


#ifndef ARGV_H	
#define ARGV_H	 

#ifndef ALWAYS_H
#include "always.h"
#endif

#ifdef _UNIX
#include "osdep.h"
#endif

// Used to parse command line that is passed into WinMain.  
// It also has the ability to load a file with values to append to the command line.
// Normally in WinMain() there would be a call Argv::Init(lpCmdLine, fileprefix).  
// Once this is done, user can create an ArgvClass object (say argv) then argv.Find() can be called.
// If there is a arguement <fileprefix><fname> (for example @file.arg) then the fname is loaded up,
// parsed, and put into the command line.  The format of the parameter file is as follows:
// 	1. a semicolon (;) at the start of the line is a comment and will be ignored.
//		2. Each line is a seperate parameter.  This enables white space to be embeded.
//	In typical Argv implementation, the first argument is the name of the application.  This
// is not the case with this.
class ArgvClass
{				 
	public:			 
		// As passed into WinMain.
		// Should be called before any objects are created.
		// This can be called multible times.
		static int Init(char *lpCmdLine, const char *fileprefix = "@");
		static bool Load_File(const char *fname);
		static void Free();

		// Create an object that can search the args.
		ArgvClass(bool case_sensitive = false, bool exact_size = false);
		~ArgvClass() {}
		
		// Functions to find a value.
		const char *Find(const char *arg)  {
			CurrentPos = -1;
			return(Find_Again(arg));
		}
		// If NULL passed, original string will be used.
		const char *Find_Again(const char *arg = 0L);
										
		// Return pointer to data after 'arg'.
		// White space is skipped.
		// So give a line '-Pdata' or '-P data' and arg==-P, 'data' would be returned in
		// both cases.
		const char *Find_Value(const char *arg);
		
		// Similar to Find_Value, only gets value of current arg.  User needs to pass
		// in the strlen of the prefix (2 for -P example above) to skip.
		// Val_in_next is set if the value was extracted from the next Argv, 
		// this way the programmer can know if he needs to call an extra Next().
		const char *Get_Cur_Value(unsigned prefixlen, bool * val_in_next = 0);

		// Update an existing attrib to use this new value
		void Update_Value(const char *attrib, const char *value);

		// Add a new attrib value pair (or just an option)
		void Add_Value(const char *attrib, const char *value=NULL);

		// Remove an option (and its value)
		bool Remove_Value(const char *attrib);

		
		// First parameter.
		const char *First()  {
			CurrentPos = 0;
			return(Argv[0]);
		}

		// Next works after a Find() call also.
		const char *Next()  {
			CurrentPos++;	  
			if (CurrentPos < Argc) {
				return(Argv[CurrentPos]);
			}
			return(0L);
		}		  				
		// Can be called so Next() will return First()...
		void Reset()  {
			CurrentPos = -1;
		}
		
		const char *Cur()  {
			if (CurrentPos < Argc) {
				return(Argv[CurrentPos]);
			}
			return(0L);
		}

		// Allow user to change states.
		void Case_Sensitive(bool on) {Flag.CaseSensitive = on;}
		bool Is_Case_Sensitive() {return (Flag.CaseSensitive);}

		// Allow user to change states.
		void Exact_Size(bool on) {Flag.ExactSize = on;}
		bool Is_Exact_Size() {return (Flag.ExactSize);}
					 
	protected:	 
		// Current position to be used when Next or Find_Again are called.
		int			CurrentPos;
									 
		// Last arg that we are searching for.
		const char	*LastArg;

		union {
			unsigned Flags;
			struct {
				unsigned CaseSensitive:1;
				unsigned ExactSize:1;
			} Flag;
		};
		
		// Number of args.
		static int   Argc;	
		
		// The actual data.
		enum {MAX_ARGC = 256};
		static char  *Argv[MAX_ARGC];
};


#endif

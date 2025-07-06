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

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifdef _WIN32
#include <process.h>
#ifdef IN
#undef IN
#endif
#define IN const
#endif
#include <wstypes.h>
#include <configfile.h>
#include <critsec.h>
#include <threadfac.h>
#include <tcp.h>
#include "matcher.h"
#include "rand.h"

class GlobalClass
{
public:
	GlobalClass();

	ConfigFile config;
	bool ReadFile(const char *fname);

	bool GetString(const Wstring& key, Wstring& val);

	RandClass rnd;
};

extern GlobalClass Global;

// Log rotation functions
void rotateOutput(void);
void rotateParanoid(void);

#endif


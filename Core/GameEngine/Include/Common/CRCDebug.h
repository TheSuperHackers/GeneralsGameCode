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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// CRCDebug.h ///////////////////////////////////////////////////////////////
// Macros/functions/etc to help logging values for tracking down sync errors
// Author: Matthew D. Campbell, June 2002

// no #pragma once - we want to be able to conditionally define NO_DEBUG_CRC in indiv .cpp files

#ifndef __CRCDEBUG_H__
#define __CRCDEBUG_H__

#include "Common/Debug.h"

#ifndef NO_DEBUG_CRC
	#ifdef DEBUG_LOGGING
		#define DEBUG_CRC
	#endif
#endif

#ifdef DEBUG_CRC

#include "Common/AsciiString.h"
#include "GameLogic/GameLogic.h"
#include "Lib/BaseType.h"
#include "WWMath/vector3.h"
#include "WWMath/matrix3d.h"

	#define AS_INT(x) (*(Int *)(&x))
	#define DUMPVEL DUMPCOORD3DNAMED(&m_vel, "m_vel")
	#define DUMPACCEL DUMPCOORD3DNAMED(&m_accel, "m_accel")
	#define DUMPVECTOR3(x) DUMPVECTOR3NAMED(x, #x)
	#define DUMPVECTOR3NAMED(x, y) dumpVector3(x, y, __FILE__, __LINE__)
	#define DUMPCOORD3D(x) DUMPCOORD3DNAMED(x, #x)
	#define DUMPCOORD3DNAMED(x, y) dumpCoord3D(x, y, __FILE__, __LINE__)
	#define DUMPMATRIX3D(x) DUMPMATRIX3DNAMED(x, #x)
	#define DUMPMATRIX3DNAMED(x, y) dumpMatrix3D(x, y, __FILE__, __LINE__)
	#define DUMPREAL(x) DUMPREALNAMED(x, #x)
	#define DUMPREALNAMED(x, y) dumpReal(x, y, __FILE__, __LINE__)

	extern Int TheCRCFirstFrameToLog;
	extern UnsignedInt TheCRCLastFrameToLog;

	void dumpVector3(const Vector3 *v, AsciiString name, AsciiString fname, Int line);
	void dumpCoord3D(const Coord3D *c, AsciiString name, AsciiString fname, Int line);
	void dumpMatrix3D(const Matrix3D *m, AsciiString name, AsciiString fname, Int line);
	void dumpReal(Real r, AsciiString name, AsciiString fname, Int line);

	void outputCRCDebugLines( void );
	void CRCDebugStartNewGame( void );
	void outputCRCDumpLines( void );

	void addCRCDebugLine(const char *fmt, ...);
	void addCRCDebugLineNoCounter(const char *fmt, ...);
	void addCRCDumpLine(const char *fmt, ...);
	void addCRCGenLine(const char *fmt, ...);
	#define CRCDEBUG_LOG(x) addCRCDebugLine x
	#define CRCDUMP_LOG(x) addCRCDumpLine x
	#define CRCGEN_LOG(x) addCRCGenLine x

	class CRCVerification
	{
	public:
		CRCVerification();
		~CRCVerification();
	protected:
		UnsignedInt m_startCRC;
	};
	#define VERIFY_CRC CRCVerification crcVerification;

	extern Int lastCRCDebugFrame;
	extern Int lastCRCDebugIndex;
	
	extern Bool g_verifyClientCRC;
	extern Bool g_clientDeepCRC;

	extern Bool g_crcModuleDataFromClient;
	extern Bool g_crcModuleDataFromLogic;

	extern Bool g_keepCRCSaves;
	extern Bool g_saveDebugCRCPerFrame;
	extern AsciiString g_saveDebugCRCPerFrameDir;

	extern Bool g_logObjectCRCs;

#else // DEBUG_CRC

	#define DUMPVEL {}
	#define DUMPACCEL {}
	#define DUMPVECTOR3(x) {}
	#define DUMPVECTOR3NAMED(x, y) {}
	#define DUMPCOORD3D(x) {}
	#define DUMPCOORD3DNAMED(x, y) {}
	#define DUMPMATRIX3D(x) {}
	#define DUMPMATRIX3DNAMED(x, y) {}

	#define DUMPREAL(x) {}
	#define DUMPREALNAMED(x, y) {}

	#define CRCDEBUG_LOG(x) {}
	#define CRCDUMP_LOG(x) {}
	#define CRCGEN_LOG(x) {}

	#define VERIFY_CRC {}

#endif

extern Int NET_CRC_INTERVAL;
extern Int REPLAY_CRC_INTERVAL;
extern Bool TheDebugIgnoreSyncErrors;

#endif // __CRCDEBUG_H__
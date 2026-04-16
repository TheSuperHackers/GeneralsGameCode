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

// FILE: Win32GameEngine.h ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2001
// Description:
//   Device implementation of the game engine ... this is, of course, the
//   highest level of the game that creates the necessary interfaces to the
//   devices we need
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/GameEngine.h"
#include "GameClient/ParticleSys.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/NetworkInterface.h"
#include "MilesAudioDevice/MilesAudioManager.h"
#include "Win32Device/Common/Win32BIGFileSystem.h"
#include "Win32Device/Common/Win32LocalFileSystem.h"
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/GameLogic/W3DGameLogic.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "W3DDevice/GameClient/W3DWebBrowser.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/Common/W3DRadar.h"
#include "W3DDevice/Common/W3DThingFactory.h"




//-------------------------------------------------------------------------------------------------
/** Class declaration for the Win32 game engine */
//-------------------------------------------------------------------------------------------------
class Win32GameEngine : public GameEngine
{

public:

	Win32GameEngine();
	virtual ~Win32GameEngine() override;

	virtual void init() override;															///< initialization
	virtual void reset() override;															///< reset engine
	virtual void update() override;														///< update the game engine
	virtual void serviceWindowsOS() override;									///< allow windows maintenance in background

protected:

	virtual GameLogic *createGameLogic() override;							///< factory for game logic
 	virtual GameClient *createGameClient() override;						///< factory for game client
	virtual ModuleFactory *createModuleFactory() override;			///< factory for creating modules
	virtual ThingFactory *createThingFactory() override;				///< factory for the thing factory
	virtual FunctionLexicon *createFunctionLexicon() override; ///< factory for function lexicon
	virtual LocalFileSystem *createLocalFileSystem() override; ///< factory for local file system
	virtual ArchiveFileSystem *createArchiveFileSystem() override;	///< factory for archive file system
	virtual NetworkInterface *createNetwork();				///< Factory for the network
	virtual Radar *createRadar(Bool dummy) override;						///< Factory for radar
	virtual WebBrowser *createWebBrowser() override;						///< Factory for embedded browser
	virtual AudioManager *createAudioManager(Bool dummy) override;				///< Factory for audio device
	virtual ParticleSystemManager* createParticleSystemManager(Bool dummy) override;


protected:
	UINT m_previousErrorMode;
};

// INLINE -----------------------------------------------------------------------------------------
inline GameLogic *Win32GameEngine::createGameLogic() { return NEW W3DGameLogic; }
inline GameClient *Win32GameEngine::createGameClient() { return NEW W3DGameClient; }
inline ModuleFactory *Win32GameEngine::createModuleFactory() { return NEW W3DModuleFactory; }
inline ThingFactory *Win32GameEngine::createThingFactory() { return NEW W3DThingFactory; }
inline FunctionLexicon *Win32GameEngine::createFunctionLexicon() { return NEW W3DFunctionLexicon; }
inline LocalFileSystem *Win32GameEngine::createLocalFileSystem() { return NEW Win32LocalFileSystem; }
inline ArchiveFileSystem *Win32GameEngine::createArchiveFileSystem() { return NEW Win32BIGFileSystem; }
inline ParticleSystemManager* Win32GameEngine::createParticleSystemManager(Bool dummy)
{
	if (dummy)
		return NEW ParticleSystemManagerDummy;
	return NEW W3DParticleSystemManager;
}

inline NetworkInterface *Win32GameEngine::createNetwork() { return NetworkInterface::createNetwork(); }
inline Radar *Win32GameEngine::createRadar(Bool dummy)
{
	if (dummy)
		return NEW RadarDummy;
	return NEW W3DRadar;
}
inline WebBrowser *Win32GameEngine::createWebBrowser() { return NEW CComObject<W3DWebBrowser>; }
inline AudioManager *Win32GameEngine::createAudioManager(Bool dummy)
{
	if (dummy)
		return NEW MilesAudioManagerDummy;
	return NEW MilesAudioManager;
}

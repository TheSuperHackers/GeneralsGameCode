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

// FILE: GameEngine.h /////////////////////////////////////////////////////////
// The Game engine interface
// Author: Michael S. Booth, April 2001

#pragma once

#include "Common/SubsystemInterface.h"
#include "Common/GameType.h"

// forward declarations
class AudioManager;
class GameLogic;
class GameClient;
class MessageStream;															///< @todo Create a MessageStreamInterface abstract class
class FileSystem;
class Keyboard;
class LocalFileSystem;
class ArchiveFileSystem;
class FileSystem;
class Mouse;
class NetworkInterface;
class ModuleFactory;
class ThingFactory;
class FunctionLexicon;
class Radar;
class WebBrowser;
class ParticleSystemManager;

class GameEngine : public SubsystemInterface
{
public:

	GameEngine( void );
	virtual ~GameEngine();

	virtual void init( void );								///< Init engine by creating client and logic
	virtual void reset( void );								///< reset system to starting state
	virtual void update( void );							///< per frame update

	virtual void execute( void );											/**< The "main loop" of the game engine.
																								 It will not return until the game exits. */

	static Bool isTimeFrozen(); ///< Returns true if a script has frozen time.
	static Bool isGameHalted(); ///< Returns true if the game is paused or the network is stalling.

	virtual void setQuitting( Bool quitting );				///< set quitting status
	virtual Bool getQuitting(void);						///< is app getting ready to quit.

	virtual Bool isMultiplayerSession( void );
	virtual void serviceWindowsOS(void) {};		///< service the native OS
	virtual Bool isActive(void) {return m_isActive;}	///< returns whether app has OS focus.
	virtual void setIsActive(Bool isActive) { m_isActive = isActive; };

protected:

	virtual void resetSubsystems( void );

	Bool canUpdateGameLogic();
	Bool canUpdateNetworkGameLogic();
	Bool canUpdateRegularGameLogic();

	virtual FileSystem *createFileSystem( void );								///< Factory for FileSystem classes
	virtual LocalFileSystem *createLocalFileSystem( void ) = 0;	///< Factory for LocalFileSystem classes
	virtual ArchiveFileSystem *createArchiveFileSystem( void ) = 0;	///< Factory for ArchiveFileSystem classes
	virtual GameLogic *createGameLogic( void ) = 0;							///< Factory for GameLogic classes.
	virtual GameClient *createGameClient( void ) = 0;						///< Factory for GameClient classes.
	virtual MessageStream *createMessageStream( void );					///< Factory for the message stream
	virtual ModuleFactory *createModuleFactory( void ) = 0;			///< Factory for modules
	virtual ThingFactory *createThingFactory( void ) = 0;				///< Factory for the thing factory
	virtual FunctionLexicon *createFunctionLexicon( void ) = 0;	///< Factory for Function Lexicon
	virtual Radar *createRadar( void ) = 0;											///< Factory for radar
	virtual WebBrowser *createWebBrowser( void ) = 0;						///< Factory for embedded browser
	virtual ParticleSystemManager* createParticleSystemManager( void ) = 0;
	virtual AudioManager *createAudioManager( void ) = 0;				///< Factory for Audio Manager

	Real m_logicTimeAccumulator; ///< Frame time accumulated towards submitting a new logic frame

	Bool m_quitting; ///< true when we need to quit the game
	Bool m_isActive; ///< app has OS focus.
};

inline void GameEngine::setQuitting( Bool quitting ) { m_quitting = quitting; }
inline Bool GameEngine::getQuitting(void) { return m_quitting; }

// the game engine singleton
extern GameEngine *TheGameEngine;

/// This function creates a new game engine instance, and is device specific
extern GameEngine *CreateGameEngine( void );

/// The entry point for the game system
extern Int GameMain();

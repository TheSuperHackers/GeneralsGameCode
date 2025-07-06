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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// GameClient.h ///////////////////////////////////////////////////////////////
// GameClient singleton class - defines interface to GameClient methods and drawables
// Author: Michael S. Booth, March 2001

#pragma once

#ifndef _GAME_INTERFACE_H_
#define _GAME_INTERFACE_H_

#include "Common/GameType.h"
#include "Common/MessageStream.h"		// for GameMessageTranslator
#include "Common/Snapshot.h"
#include "Common/STLTypedefs.h"
#include "Common/SubsystemInterface.h"
#include "GameClient/CommandXlat.h"
#include "GameClient/Drawable.h"

// forward declarations
class AsciiString;
class Display;
class DisplayStringManager;
class Drawable;
class FontLibrary;
class GameWindowManager;
class InGameUI;
class Keyboard;
class Mouse;
class ParticleSystemManager;
class TerrainVisual;
class ThingTemplate;
class VideoPlayerInterface;
struct RayEffectData;

/// Function pointers for use by GameClient callback functions.
typedef void (*GameClientFuncPtr)( Drawable *draw, void *userData ); 
typedef std::hash_map<DrawableID, Drawable *, rts::hash<DrawableID>, rts::equal_to<DrawableID> > DrawablePtrHash;
typedef DrawablePtrHash::iterator DrawablePtrHashIt;

//-----------------------------------------------------------------------------
/** The Client message dispatcher, this is the last "translator" on the message
	* stream before the messages go to the network for processing.  It gives
	* the client itself the opportunity to respond to any messages on the stream
	* or create new ones to pass along to the network and logic */
class GameClientMessageDispatcher : public GameMessageTranslator
{
public:
	virtual GameMessageDisposition translateGameMessage(const GameMessage *msg);
	virtual ~GameClientMessageDispatcher() { }
};	


//-----------------------------------------------------------------------------
/**
 * The GameClient class is used to instantiate a singleton which 
 * implements the interface to all GameClient operations such as Drawable access and user-interface functions.
 */
class GameClient : public SubsystemInterface,
									 public Snapshot
{

public:

	GameClient();
	virtual ~GameClient();

	// subsystem methods
	virtual void init( void );																					///< Initialize resources
	virtual void update( void );																				///< Updates the GUI, display, audio, etc
	virtual void reset( void );																					///< reset system

	virtual void setFrame( UnsignedInt frame ) { m_frame = frame; }			///< Set the GameClient's internal frame number
	virtual void registerDrawable( Drawable *draw );										///< Given a drawable, register it with the GameClient and give it a unique ID

	void updateHeadless();

	void addDrawableToLookupTable( Drawable *draw );			///< add drawable ID to hash lookup table
	void removeDrawableFromLookupTable( Drawable *draw );	///< remove drawable ID from hash lookup table

	virtual Drawable *findDrawableByID( const DrawableID id );					///< Given an ID, return the associated drawable

	void setDrawableIDCounter( DrawableID nextDrawableID ) { m_nextDrawableID = nextDrawableID; }
	DrawableID getDrawableIDCounter( void ) { return m_nextDrawableID; }

	virtual Drawable *firstDrawable( void ) { return m_drawableList; }

	virtual GameMessage::Type evaluateContextCommand( Drawable *draw, 
																										const Coord3D *pos, 
																										CommandTranslator::CommandEvaluateType cmdType );
	void addTextBearingDrawable( Drawable *tbd );
	void flushTextBearingDrawables( void);
	
	virtual void removeFromRayEffects( Drawable *draw );  ///< remove the drawable from the ray effect system if present
	virtual void getRayEffectData( Drawable *draw, RayEffectData *effectData );  ///< get ray effect data for a drawable
	virtual void createRayEffectByTemplate( const Coord3D *start, const Coord3D *end, const ThingTemplate* tmpl ) = 0;  ///< create effect needing start and end location

	virtual void addScorch(const Coord3D *pos, Real radius, Scorches type) = 0;

	virtual Bool loadMap( AsciiString mapName );  ///< load a map into our scene
	virtual void unloadMap( AsciiString mapName );  ///< unload the specified map from our scene

	virtual void iterateDrawablesInRegion( Region3D *region, GameClientFuncPtr userFunc, void *userData );		///< Calls userFunc for each drawable contained within the region

	virtual Drawable *friend_createDrawable( const ThingTemplate *thing, DrawableStatus statusBits = DRAWABLE_STATUS_NONE ) = 0;
	virtual void destroyDrawable( Drawable *draw );											///< Destroy the given drawable

	virtual void setTimeOfDay( TimeOfDay tod );													///< Tell all the drawables what time of day it is now

	virtual void selectDrawablesInGroup( Int group );									///< select all drawables belong to the specifies group
	virtual void assignSelectedDrawablesToGroup( Int group );						///< assign all selected drawables to the specified group
	//---------------------------------------------------------------------------------------
	virtual UnsignedInt getFrame( void ) { return m_frame; }						///< Returns the current simulation frame number

	//---------------------------------------------------------------------------
	virtual void setTeamColor( Int red, Int green, Int blue ) = 0;  ///< @todo superhack for demo, remove!!!
	virtual void adjustLOD( Int adj ) = 0; ///< @todo hack for evaluation, remove.

	virtual void releaseShadows(void);	///< frees all shadow resources used by this module - used by Options screen.
	virtual void allocateShadows(void); ///< create shadow resources if not already present. Used by Options screen.

  virtual void preloadAssets( TimeOfDay timeOfDay );									///< preload assets

	virtual Drawable *getDrawableList( void ) { return m_drawableList; }
	
	void resetRenderedObjectCount() { m_renderedObjectCount = 0; }
	UnsignedInt getRenderedObjectCount() const { return m_renderedObjectCount; }
	void incrementRenderedObjectCount() { m_renderedObjectCount++; }

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	// @todo Should there be a separate GameClient frame counter?
	UnsignedInt m_frame;																				///< Simulation frame number from server

	Drawable *m_drawableList;																		///< All of the drawables in the world
	DrawablePtrHash m_drawableHash;															///< Used for DrawableID lookups

	DrawableID m_nextDrawableID;																///< For allocating drawable id's
	DrawableID allocDrawableID( void );													///< Returns a new unique drawable id

	enum { MAX_CLIENT_TRANSLATORS = 32 };
	TranslatorID m_translators[ MAX_CLIENT_TRANSLATORS ];				///< translators we have used
	UnsignedInt m_numTranslators;																///< number of translators in m_translators[]
	CommandTranslator *m_commandTranslator;											///< the command translator on the message stream

private:

	UnsignedInt m_renderedObjectCount;													///< Keeps track of the number of rendered objects -- resets each frame.

	//---------------------------------------------------------------------------

	virtual Display *createGameDisplay( void ) = 0;							///< Factory for Display classes. Called during init to instantiate TheDisplay.
	virtual InGameUI *createInGameUI( void ) = 0;								///< Factory for InGameUI classes. Called during init to instantiate TheInGameUI
	virtual GameWindowManager *createWindowManager( void ) = 0; ///< Factory to window manager
	virtual FontLibrary *createFontLibrary( void ) = 0;					///< Factory for font library
	virtual DisplayStringManager *createDisplayStringManager( void ) = 0;  ///< Factory for display strings
	virtual VideoPlayerInterface *createVideoPlayer( void ) = 0;///< Factory for video device
	virtual TerrainVisual *createTerrainVisual( void ) = 0;			///< Factory for TerrainVisual classes. Called during init to instance TheTerrainVisual
	virtual Keyboard *createKeyboard( void ) = 0;								///< factory for the keyboard
	virtual Mouse *createMouse( void ) = 0;											///< factory for the mouse

	virtual void setFrameRate(Real msecsPerFrame) = 0;

	// ----------------------------------------------------------------------------------------------
	struct DrawableTOCEntry
	{
		AsciiString name;
		UnsignedShort id;
	};
	typedef std::list< DrawableTOCEntry > DrawableTOCList;
	typedef DrawableTOCList::iterator DrawableTOCListIterator;
	DrawableTOCList m_drawableTOC;														///< the drawable TOC
	void addTOCEntry( AsciiString name, UnsignedShort id );		///< add a new name/id TOC pair
	DrawableTOCEntry *findTOCEntryByName( AsciiString name );	///< find DrawableTOC by name
	DrawableTOCEntry *findTOCEntryById( UnsignedShort id );		///< find DrawableTOC by id
	void xferDrawableTOC( Xfer *xfer );												///< save/load drawable TOC for current state of map
	
	typedef std::list< Drawable* > TextBearingDrawableList;
	typedef TextBearingDrawableList::iterator TextBearingDrawableListIterator;
	TextBearingDrawableList m_textBearingDrawableList;	///< the drawables that have registered here during drawablepostdraw
};

//Kris: Try not to use this if possible. In every case I found in the code base, the status was always Drawable::SELECTED.
//      There is another iterator already in game that stores JUST selected drawables. Take a look at the efficient 
//      example, InGameUI::getAllSelectedDrawables(). 
#define BEGIN_ITERATE_DRAWABLES_WITH_STATUS(STATUS, DRAW) \
	do \
	{ \
		Drawable* _xq_nextDrawable; \
		for (Drawable* DRAW = TheGameClient->firstDrawable(); DRAW != NULL; DRAW = _xq_nextDrawable ) \
		{ \
			_xq_nextDrawable = DRAW->getNextDrawable(); \
			if (DRAW->getStatusFlags() & (STATUS)) \
			{ 

#define END_ITERATE_DRAWABLES \
			} \
		} \
	} while (0);


// the singleton
extern GameClient *TheGameClient;


// TheSuperHackers @logic-client-separation helmutbuhler 11/04/2025
// Some information about the architecture and headless mode:
// The game is structurally separated into GameLogic and GameClient.
// The Logic is responsible for everything that affects the game mechanic and what is synchronized over
// the network. The Client is responsible for rendering, input, audio and similar stuff.
//
// Unfortunately there are some places in the code that make the Logic depend on the Client.
// (Search for @logic-client-separation)
// That means if we want to run the game headless, we cannot just disable the Client. We need to disable
// the parts in the Client that don't work in headless mode and need to keep the parts that are needed
// to run the Logic.
// The following describes which parts we disable in headless mode:
//
//	GameEngine:
//		TheGameClient is partially disabled:
//			TheKeyboard = NULL
//			TheMouse = NULL
//			TheDisplay is partially disabled:
//				m_3DInterfaceScene = NULL
//				m_2DScene = NULL
//				m_3DScene = NULL
//				(m_assetManager remains!)
//			TheWindowManager = GameWindowManagerDummy
//			TheIMEManager = NULL
//			TheTerrainVisual is partially disabled:
//				TheTerrainTracksRenderObjClassSystem = NULL
//				TheW3DShadowManager = NULL
//				TheWaterRenderObj = NULL
//				TheSmudgeManager = NULL
//				TheTerrainRenderObject is partially disabled:
//					m_treeBuffer = NULL
//					m_propBuffer = NULL
//					m_bibBuffer = NULL
//					m_bridgeBuffer is partially disabled:
//						m_vertexBridge = NULL
//						m_indexBridge = NULL
//						m_vertexMaterial = NULL
//					m_waypointBuffer = NULL
//					m_roadBuffer = NULL
//					m_shroud = NULL
//		TheRadar = RadarDummy

#endif // _GAME_INTERFACE_H_

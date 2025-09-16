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

// FILE: Anim2D.h /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, July 2002
// Desc:   A collection of 2D images to make animation
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __ANIM_2D_H_
#define __ANIM_2D_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/Snapshot.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Image;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
enum Anim2DMode CPP_11(: Int)
{

	ANIM_2D_INVALID = 0,
	ANIM_2D_ONCE,
	ANIM_2D_ONCE_BACKWARDS,
	ANIM_2D_LOOP,
	ANIM_2D_LOOP_BACKWARDS,
	ANIM_2D_PING_PONG,
	ANIM_2D_PING_PONG_BACKWARDS,
	// dont' forget to add new animation mode names to Anim2DModeNames[] below

	ANIM_2D_NUM_MODES

};
#ifdef DEFINE_ANIM_2D_MODE_NAMES
static const char *const Anim2DModeNames[] =
{
	"NONE",
	"ONCE",
	"ONCE_BACKWARDS",
	"LOOP",
	"LOOP_BACKWARDS",
	"PING_PONG",
	"PING_PONG_BACKWARDS",
	NULL
};
static_assert(ARRAY_SIZE(Anim2DModeNames) == ANIM_2D_NUM_MODES + 1, "Incorrect array size");
#endif

// ------------------------------------------------------------------------------------------------
/** A template of a 2D animation */
// ------------------------------------------------------------------------------------------------
class Anim2DTemplate : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(Anim2DTemplate, "Anim2DTemplate")
public:

	Anim2DTemplate( AsciiString name );
	//virtual ~Anim2DTemplate( void );

	AsciiString getName( void ) const { return m_name; }
	const Image *getFrame( UnsignedShort frameNumber ) const;
	UnsignedShort getNumFrames( void ) const { return m_numFrames; }
	UnsignedShort getNumFramesBetweenUpdates( void ) const { return m_framesBetweenUpdates; }
	Anim2DMode getAnimMode( void ) const { return m_animMode; }
	Bool isRandomizedStartFrame( void ) const { return m_randomizeStartFrame; }

	// list access for use by the Anim2DCollection only
	void friend_setNextTemplate( Anim2DTemplate *animTemplate ) { m_nextTemplate = animTemplate; }
	Anim2DTemplate *friend_getNextTemplate( void ) const { return m_nextTemplate; };

	// INI methods
	const FieldParse *getFieldParse( void ) const { return s_anim2DFieldParseTable; }
	void storeImage( const Image *image );								///< store image in next available slot
	void allocateImages( UnsignedShort numFrames );	///< allocate the array of image pointers to use

protected:

	static void parseImage( INI *ini, void *instance, void *store, const void *userData );
	static void parseNumImages( INI *ini, void *instance, void *store, const void *userData );
	static void parseImageSequence( INI *ini, void *instance, void *store, const void *userData );

protected:
	enum { NUM_FRAMES_INVALID = 0 };			///< initialization value for num frames

	Anim2DTemplate*	m_nextTemplate;				///< next animation in collections animation list
	AsciiString			m_name;										///< name of this 2D animation
	const Image**		m_images;											///< array of image pointers that make up this animation
	UnsignedShort		m_numFrames;						///< total number of frames in this animation
	UnsignedShort		m_framesBetweenUpdates;	///< frames between frame updates
	Anim2DMode			m_animMode;								///< the animation mode
	Bool						m_randomizeStartFrame;						///< randomize animation instance start frames

protected:
	static const FieldParse s_anim2DFieldParseTable[];		///< the parse table for INI definition

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
enum Anim2DStatus CPP_11(: Int)
{
	ANIM_2D_STATUS_NONE			= 0x00,
	ANIM_2D_STATUS_FROZEN		= 0x01,
	ANIM_2D_STATUS_REVERSED = 0x02,  // used for ping pong direction tracking
	ANIM_2D_STATUS_COMPLETE = 0x04,	 // set when uni-directional things reach their last frame
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class Anim2D : public MemoryPoolObject,
							 public Snapshot
{

friend class Anim2DCollection;

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( Anim2D, "Anim2D" );

public:

	Anim2D( Anim2DTemplate *animTemplate, Anim2DCollection *collectionSystem );
	// virtual destructor prototype provided by memory pool object

	UnsignedShort getCurrentFrame( void ) const { return m_currentFrame; }		///< get our current frame #
	void setCurrentFrame( UnsignedShort frame );				///< set the current frame #
	void randomizeCurrentFrame( void );									///< randomize the current frame #
	void reset( void );																	///< reset the current frame to the "start"
	void setStatus( UnsignedByte statusBits );					///< set status bit(s)
	void clearStatus( UnsignedByte statusBits );				///< clear status bit(s)
	UnsignedByte getStatus( void ) const { return m_status; }	///< return status bits(s)
	void setAlpha( Real alpha ) { m_alpha = alpha; }		///< set alpha value
	Real getAlpha( void ) const { return m_alpha; }						///< return the current alpha value

	//Allows you to play a segment of an animation.
	void setMinFrame( UnsignedShort frame ) { m_minFrame = frame; }
	void setMaxFrame( UnsignedShort frame ) { m_maxFrame = frame; }

	// info about the size of the current frame
	UnsignedInt getCurrentFrameWidth( void ) const;			///< return natural width of image in the current frame
	UnsignedInt getCurrentFrameHeight( void ) const;		///< return natural height of image in the current frame
	const Anim2DTemplate *getAnimTemplate( void ) const { return m_template; }	///< return our template

	void draw( Int x, Int y );													///< draw iamge at location using natural width/height
	void draw( Int x, Int y, Int width, Int height );		///< draw image at location using forced width/height

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer ) { }
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void ) { }

	void tryNextFrame( void );						///< we've just drawn ... try to update our frame if necessary

	UnsignedShort m_currentFrame;					///< current frame of our animation
	UnsignedInt m_lastUpdateFrame;				///< last frame we updated on
	Anim2DTemplate *m_template;						///< pointer back to the template that defines this animation
	UnsignedByte m_status;								///< status bits (see Anim2DStatus)
	UnsignedShort m_minFrame;							///< min animation frame used inclusively.
	UnsignedShort m_maxFrame;							///< max animation frame used inclusively.
	UnsignedInt m_framesBetweenUpdates;		///< duration between each frame.
	Real m_alpha;

	Anim2DCollection *m_collectionSystem;	///< system collection (if any) we're registered with
	Anim2D *m_collectionSystemNext;				///< system instance tracking list
	Anim2D *m_collectionSystemPrev;				///< system instance tracking list

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class Anim2DCollection : public SubsystemInterface
{

public:

	Anim2DCollection( void );
	virtual ~Anim2DCollection( void );

	virtual void init( void );						///< initialize system
	virtual void reset( void ) { };				///< reset system
	virtual void update( void );					///< update system

	Anim2DTemplate *findTemplate( const AsciiString& name );				///< find animation template
	Anim2DTemplate *newTemplate( const AsciiString& name );				///< allocate a new template to be loaded

	void registerAnimation( Anim2D *anim );									///< register animation with system
	void unRegisterAnimation( Anim2D *anim );								///< un-register animation from system

	Anim2DTemplate* getTemplateHead() const { return m_templateList; }
	Anim2DTemplate* getNextTemplate( Anim2DTemplate *animTemplate ) const;

protected:

	Anim2DTemplate *m_templateList;				///< list of available animation templates
	Anim2D *m_instanceList;								///< list of all the anim 2D instance we're tracking

};

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern Anim2DCollection *TheAnim2DCollection;

#endif  // end __ANIM_2D_H_

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

// FILE: Anim2D.cpp ///////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, July 2002
// Desc:   A collection of 2D images to make animation
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#define DEFINE_ANIM_2D_MODE_NAMES
#include "Common/RandomValue.h"
#include "Common/Xfer.h"
#include "GameClient/Anim2D.h"
#include "GameClient/Display.h"
#include "GameClient/Image.h"
#include "GameLogic/GameLogic.h"


// GLOBAL /////////////////////////////////////////////////////////////////////////////////////////
Anim2DCollection *TheAnim2DCollection = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Anim2DTemplate::Anim2DTemplate( AsciiString name )
{

	m_name = name;
	m_images = NULL;
	m_numFrames = NUM_FRAMES_INVALID;
	m_framesBetweenUpdates = 0;
	m_animMode = ANIM_2D_LOOP;
	m_randomizeStartFrame = FALSE;
	m_nextTemplate = NULL;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Anim2DTemplate::~Anim2DTemplate( void )
{

	delete [] m_images;

}

// ------------------------------------------------------------------------------------------------
/** Field parse table for 2D animation templates */
// ---------i---------------------------------------------------------------------------------------
const FieldParse Anim2DTemplate::s_anim2DFieldParseTable[] =
{

	{ "NumberImages",					Anim2DTemplate::parseNumImages,			NULL,							0 },
	{ "Image",								Anim2DTemplate::parseImage,					NULL,							0 },
	{ "ImageSequence",				Anim2DTemplate::parseImageSequence, NULL,							0 },
	{ "AnimationMode",				INI::parseIndexList,								Anim2DModeNames,	offsetof( Anim2DTemplate, m_animMode ) },
	{ "AnimationDelay",				INI::parseDurationUnsignedShort,		NULL,							offsetof( Anim2DTemplate, m_framesBetweenUpdates ) },
	{ "RandomizeStartFrame",	INI::parseBool,											NULL,							offsetof( Anim2DTemplate, m_randomizeStartFrame ) },
	{ NULL,										NULL,																NULL,							0 }

};

// ------------------------------------------------------------------------------------------------
/** Parse the number of images we will have in this animation and allocate the array for them */
// ------------------------------------------------------------------------------------------------
void Anim2DTemplate::parseNumImages( INI *ini, void *instance, void *store, const void *userData )
{

	// parse the integer data from the file
	UnsignedInt numFrames;
	ini->parseUnsignedInt( ini, instance, &numFrames, userData );

	// get the template we are to store into
	Anim2DTemplate *animTemplate = (Anim2DTemplate *)instance;

	// animations must have a minimum # of frames
	Int minimumFrames = 1;
	if( numFrames < minimumFrames )
	{

		DEBUG_CRASH(( "Anim2DTemplate::parseNumImages - Invalid animation '%s', animations must have '%d' or more frames defined",
									 animTemplate->getName().str(), minimumFrames ));
		throw INI_INVALID_DATA;

	}

	// allocate the image array
	animTemplate->allocateImages( (UnsignedShort)numFrames );

}

// ------------------------------------------------------------------------------------------------
/** Allocate the image array for an animation template and store the number of frames we have */
// ------------------------------------------------------------------------------------------------
void Anim2DTemplate::allocateImages( UnsignedShort numFrames )
{

	// store the number of frames
	m_numFrames = numFrames;

	// allocate an array to hold the image pointers
	m_images = NEW const Image *[ m_numFrames ];	// pool[]ify

	// set all the images to NULL;
	for( Int i = 0; i < m_numFrames; ++i )
		m_images[ i ] = NULL;

}

// ------------------------------------------------------------------------------------------------
/** Parsing a single image definition for an animation */
// ------------------------------------------------------------------------------------------------
void Anim2DTemplate::parseImage( INI *ini, void *instance, void *store, const void *userData )
{

	// parse the image name from the file and store as an image pointer
	const Image *image;
	ini->parseMappedImage( ini, instance, &image, userData );

	// sanity
	if( image == NULL )
	{

		//We don't care if we're in the builder
		//DEBUG_CRASH(( "Anim2DTemplate::parseImage - Image not found" ));
		//throw INI_INVALID_DATA;

	}

	//
	// assign the image to the animation template list of images ... note since we've pre-allocated
	// the array of images and the index an image is loaded into depends on its order specified
	// in INI, we need to get the number of images currently loaded into this animation template
	// so that we can put it at the next free image spot ... and then tell the animation
	// template we've loaded one more
	//
	Anim2DTemplate *animTemplate = (Anim2DTemplate *)instance;
	animTemplate->storeImage( image );

}

// ------------------------------------------------------------------------------------------------
/** This will parse the image sequence of an animation.  You can use this as a shortcut to
	* specifying a series of images for an animation instead of having to specify them all
	* individually.  Image names will be assumed to start with an appended "000" to the end
	* end of the first image name and incremented up to the number of images for the
	* animation.  NOTE: That the number images *must* have already been specified before
	* we can parse this entry so we know how many images to allocate and look for */
// ------------------------------------------------------------------------------------------------
/*static*/ void Anim2DTemplate::parseImageSequence( INI *ini, void *instance,
																										void *store, const void *userData )
{

	// get the animation template
	Anim2DTemplate *animTemplate = (Anim2DTemplate *)instance;

	//
	// before we can read, allocate, and find all the images for the sequence ... we must
	// know how many total images are in this animation.  Verify that now
	//
	if( animTemplate->getNumFrames() == NUM_FRAMES_INVALID )
	{

		DEBUG_CRASH(( "Anim2DTemplate::parseImageSequence - You must specify the number of animation frames for animation '%s' *BEFORE* specifying the image sequence name",
									animTemplate->getName().str() ));
		throw INI_INVALID_DATA;

	}

	//
	// the image storage has already been allocated, all we have to do now is count from
	// 0 to numImages - 1 and add those images to the template
	//
	AsciiString imageBaseName = ini->getNextAsciiString();
	AsciiString imageName;
	const Image *image;
	for( Int i = 0; i < animTemplate->getNumFrames(); ++i )
	{

		// construct this name
		imageName.format( "%s%03d", imageBaseName.str(), i );

		// search for this image
		image = TheMappedImageCollection->findImageByName( imageName );

		// sanity
		if( image == NULL )
		{

			DEBUG_CRASH(( "Anim2DTemplate::parseImageSequence - Image '%s' not found for animation '%s'.  Check the number of images specified in INI and also make sure all the actual images exist.",
										imageName.str(), animTemplate->getName().str() ));
			throw INI_INVALID_DATA;

		}

		// store the image in the next free frame
		animTemplate->storeImage( image );

	}

}

// ------------------------------------------------------------------------------------------------
/** Store the image at the next open image slot for the animation */
// ------------------------------------------------------------------------------------------------
void Anim2DTemplate::storeImage( const Image *image )
{

	// sanity
	if( image == NULL )
		return;

	// search through the image list and store at the next free spot
	for( Int i = 0; i < m_numFrames; ++i )
	{

		if( m_images[ i ] == NULL )
		{

			m_images[ i ] = image;
			return;

		}

	}

	// if we got here we tried to store an image in an array that was too small
	DEBUG_CRASH(( "Anim2DTemplate::storeImage - Unable to store image '%s' into animation '%s' because the animation is setup to only support '%d' image frames",
								image->getName().str(), getName().str(), m_numFrames ));
	throw INI_INVALID_DATA;

}

// ------------------------------------------------------------------------------------------------
/** Return the Image* for the frame number requested */
// ------------------------------------------------------------------------------------------------
const Image* Anim2DTemplate::getFrame( UnsignedShort frameNumber ) const
{

	// sanity
	DEBUG_ASSERTCRASH( m_images != NULL,
										 ("Anim2DTemplate::getFrame - Image data is NULL for animation '%s'",
										  getName().str()) );

	// sanity
	if( frameNumber < 0 || frameNumber >= m_numFrames )
	{

		DEBUG_CRASH(( "Anim2DTemplate::getFrame - Illegal frame number '%d' for animation '%s'",
									frameNumber, getName().str() ));
		return NULL;

	}
	else
	{

		// return the image frame
		return m_images[ frameNumber ];

	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Anim2D::Anim2D( Anim2DTemplate *animTemplate, Anim2DCollection *collectionSystem )
{

	// sanity
	DEBUG_ASSERTCRASH( animTemplate != NULL, ("Anim2D::Anim2D - NULL template") );

	//Added By Sadullah Nader
	//Initialization

	m_currentFrame = 0;

	//

	// set the template
	m_template = animTemplate;

	// initialize members
	m_status = ANIM_2D_STATUS_NONE;
	m_alpha = 1.0f;

	// set the initial frame for the animation based on the type of animation mode or randomize
	if( m_template->isRandomizedStartFrame() )
		randomizeCurrentFrame();
	else
		reset();

	m_minFrame = 0;
	m_maxFrame = m_template->getNumFrames() - 1;
	m_framesBetweenUpdates = m_template->getNumFramesBetweenUpdates();

	//added by Sadullah Nader
	// initializing pointers to NULL, and clearing Frame counters before
	// we register ourselves to the System
	m_collectionSystemNext = NULL;
	m_collectionSystemPrev = NULL;

	m_lastUpdateFrame = 0;

	// if a system is present, register ourselves with that system
	m_collectionSystem = collectionSystem;
	if( m_collectionSystem )
		m_collectionSystem->registerAnimation( this );


}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Anim2D::~Anim2D( void )
{

	// if we were registered with a system, un-register ourselves
	if( m_collectionSystem )
		m_collectionSystem->unRegisterAnimation( this );

}

// ------------------------------------------------------------------------------------------------
/** Set the current animation frame */
// ------------------------------------------------------------------------------------------------
void Anim2D::setCurrentFrame( UnsignedShort frame )
{

	// sanity
	DEBUG_ASSERTCRASH( m_template != NULL, ("Anim2D::reset - No template for animation") );

	// sanity
	DEBUG_ASSERTCRASH( TheGameLogic != NULL,
										 ("Anim2D::setCurrentFrame - TheGameLogic must exist to use animation instances (%s)",
										  m_template->getName().str()) );

	// sanity
	DEBUG_ASSERTCRASH( frame >= 0 && frame < m_template->getNumFrames(),
										 ("Anim2D::setCurrentFrame - Illegal frame number '%d' in animation",
										 frame, m_template->getName().str()) );

	// set the frame
	m_currentFrame = frame;

	// record the frame of this update to our current frame
	m_lastUpdateFrame = TheGameLogic->getFrame();

}

// ------------------------------------------------------------------------------------------------
/** Randomize the current frame */
// ------------------------------------------------------------------------------------------------
void Anim2D::randomizeCurrentFrame( void )
{

	// sanity
	DEBUG_ASSERTCRASH( m_template != NULL, ("Anim2D::reset - No template for animation") );

	// set the current frame to a random frame
	setCurrentFrame( GameClientRandomValue( 0, m_template->getNumFrames() - 1 ) );

}

// ------------------------------------------------------------------------------------------------
/** Reset this animation instance to the "start" of the animation */
// ------------------------------------------------------------------------------------------------
void Anim2D::reset( void )
{

	// sanity
	DEBUG_ASSERTCRASH( m_template != NULL, ("Anim2D::reset - No template for animation") );

	switch( m_template->getAnimMode() )
	{

		// --------------------------------------------------------------------------------------------
		case ANIM_2D_ONCE:
		case ANIM_2D_LOOP:
		case ANIM_2D_PING_PONG:
			setCurrentFrame( m_minFrame );
			break;

		// --------------------------------------------------------------------------------------------
		case ANIM_2D_ONCE_BACKWARDS:
		case ANIM_2D_LOOP_BACKWARDS:
		case ANIM_2D_PING_PONG_BACKWARDS:
			setCurrentFrame( m_maxFrame );
			break;

		// --------------------------------------------------------------------------------------------
		default:
			DEBUG_CRASH(( "Anim2D::reset - Unknown animation mode '%d' for '%s'",
										m_template->getAnimMode(), m_template->getName().str() ));
			break;

	}

}

// ------------------------------------------------------------------------------------------------
/** This is called after we are drawn ... if sufficient time has passed since our last
	* frame update we will update our current frame */
// ------------------------------------------------------------------------------------------------
void Anim2D::tryNextFrame( void )
{

	// sanity
	DEBUG_ASSERTCRASH( TheGameLogic != NULL,
										 ("Anim2D::tryNextFrame - TheGameLogic must exist to use animation instances (%s)",
										  m_template->getName().str()) );

	// how many frames have passed since our last update
	if( TheGameLogic->getFrame() - m_lastUpdateFrame >= m_framesBetweenUpdates )
	{

		switch( m_template->getAnimMode() )
		{

			// ------------------------------------------------------------------------------------------
			case ANIM_2D_ONCE:
			{

				if( m_currentFrame < m_maxFrame )
					setCurrentFrame( m_currentFrame + 1 );
				else
					setStatus( ANIM_2D_STATUS_COMPLETE );
				break;

			}

			// -------------------------------------------------------------------------------------------
			case ANIM_2D_ONCE_BACKWARDS:
			{

				if( m_currentFrame > m_minFrame )
					setCurrentFrame( m_currentFrame - 1 );
				else
					setStatus( ANIM_2D_STATUS_COMPLETE );
				break;

			}

			// -------------------------------------------------------------------------------------------
			case ANIM_2D_LOOP:
			{

				if( m_currentFrame == m_maxFrame )
					setCurrentFrame( m_minFrame );
				else
					setCurrentFrame( m_currentFrame + 1 );
				break;

			}

			// -------------------------------------------------------------------------------------------
			case ANIM_2D_LOOP_BACKWARDS:
			{

				if( m_currentFrame > m_minFrame )
					setCurrentFrame( m_currentFrame - 1 );
				else
					setCurrentFrame( m_maxFrame );
				break;

			}

			// -------------------------------------------------------------------------------------------
			case ANIM_2D_PING_PONG:
			case ANIM_2D_PING_PONG_BACKWARDS:
			{

				if( BitIsSet( m_status, ANIM_2D_STATUS_REVERSED ) )
				{
					//
					// decrement frame, unless we're at frame 0 in which case we
					// increment and reverse directions
					//
					if( m_currentFrame == m_minFrame )
					{

						setCurrentFrame( m_currentFrame + 1 );
						clearStatus( ANIM_2D_STATUS_REVERSED );

					}
					else
					{

						setCurrentFrame( m_currentFrame - 1 );

					}

				}
				else
				{

					//
					// increment frame, unless we're at the end in which case we decrement
					// and reverse directions
					//
					if( m_currentFrame == m_maxFrame )
					{

						setCurrentFrame( m_currentFrame - 1 );
						setStatus( ANIM_2D_STATUS_REVERSED );

					}
					else
					{

						setCurrentFrame( m_currentFrame + 1 );

					}

				}

				break;

			}

			// -------------------------------------------------------------------------------------------
			default:
			{

				DEBUG_CRASH(( "Anim2D::tryNextFrame - Unknown animation mode '%d' for '%s'",
											m_template->getAnimMode(), m_template->getName().str() ));
				break;

			}

		}

	}

}

// ------------------------------------------------------------------------------------------------
/** Set status bit */
// ------------------------------------------------------------------------------------------------
void Anim2D::setStatus( UnsignedByte statusBits )
{

	// set the bits
	BitSet( m_status, statusBits );

}

// ------------------------------------------------------------------------------------------------
/** Clear status bit */
// ------------------------------------------------------------------------------------------------
void Anim2D::clearStatus( UnsignedByte statusBits )
{

	// clear bits
	BitClear( m_status, statusBits );

}

// ------------------------------------------------------------------------------------------------
/** Return the "natural" width of the image for our current frame */
// ------------------------------------------------------------------------------------------------
UnsignedInt Anim2D::getCurrentFrameWidth( void ) const
{
	const Image *currentFrameImage = m_template->getFrame( m_currentFrame );

	if( currentFrameImage )
		return currentFrameImage->getImageWidth();

	return 0;

}

// ------------------------------------------------------------------------------------------------
/** Return the "natural" height of the image for our current frame */
// ------------------------------------------------------------------------------------------------
UnsignedInt Anim2D::getCurrentFrameHeight( void ) const
{
	const Image *currentFrameImage = m_template->getFrame( m_currentFrame );

	if( currentFrameImage )
		return currentFrameImage->getImageHeight();

	return 0;

}

// ------------------------------------------------------------------------------------------------
/** Draw an Anim2D using the natural width and height of the image data */
// ------------------------------------------------------------------------------------------------
void Anim2D::draw( Int x, Int y )
{

	// get the current image
	const Image *image = m_template->getFrame( m_currentFrame );

	// sanity
	DEBUG_ASSERTCRASH( image != NULL, ("Anim2D::draw - Image not found for frame '%d' on animation '%s'",
										 m_currentFrame, m_template->getName().str()) );

	// get the natural width and height of this image
	const ICoord2D *imageSize = image->getImageSize();

	// draw the image
	Color color = GameMakeColor( 255, 255, 255, 255 * m_alpha );
	TheDisplay->drawImage( image, x, y, x + imageSize->x, y + imageSize->y, color );

	//
	// see if it's time for us to go to the next frame in the sequence, we do not update
	// frame numbers for animation instances that are registered with a system as the
	// system will update them during its update phase
	//
 	if( m_collectionSystem == NULL && BitIsSet( m_status, ANIM_2D_STATUS_FROZEN ) == FALSE )
		tryNextFrame();

}

// ------------------------------------------------------------------------------------------------
/** Drawing an Anim2D using a forced width and height */
// ------------------------------------------------------------------------------------------------
void Anim2D::draw( Int x, Int y, Int width, Int height )
{

	// get the current image
	const Image *image = m_template->getFrame( m_currentFrame );

	// sanity
	DEBUG_ASSERTCRASH( image != NULL, ("Anim2D::draw - Image not found for frame '%d' on animation '%s'",
										 m_currentFrame, m_template->getName().str()) );


	// draw image to the display
	Color color = GameMakeColor( 255, 255, 255, 255 * m_alpha );
	TheDisplay->drawImage( image, x, y, x + width, y + height, color );

	//
	// see if it's time for us to go to the next frame in the sequence, we do not update
	// frame numbers for animation instances that are registered with a system as the
	// system will update them during its update phase
	//
 	if( m_collectionSystem == NULL && BitIsSet( m_status, ANIM_2D_STATUS_FROZEN ) == FALSE )
		tryNextFrame();

}

// ------------------------------------------------------------------------------------------------
/** Xfer Method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void Anim2D::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// current frame
	xfer->xferUnsignedShort( &m_currentFrame );

	// last update frame
	xfer->xferUnsignedInt( &m_lastUpdateFrame );

	// status
	xfer->xferUnsignedByte( &m_status );

	// min frame
	xfer->xferUnsignedShort( &m_minFrame );

	// max frame
	xfer->xferUnsignedShort( &m_maxFrame );

	// frames between updates
	xfer->xferUnsignedInt( &m_framesBetweenUpdates );

	// alpha
	xfer->xferReal( &m_alpha );

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Anim2DCollection::Anim2DCollection( void )
{

	m_templateList = NULL;
	m_instanceList = NULL;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Anim2DCollection::~Anim2DCollection( void )
{

	// there should not be any animation instances registered with us since we're being destroyed
	DEBUG_ASSERTCRASH( m_instanceList == NULL, ("Anim2DCollection - instance list is not NULL") );

	// delete all the templates
	Anim2DTemplate *nextTemplate;
	while( m_templateList )
	{

		// get next template
		nextTemplate = m_templateList->friend_getNextTemplate();

		// delete this template
		deleteInstance(m_templateList);

		// set the head of our list to the next template
		m_templateList = nextTemplate;

	}

}

// ------------------------------------------------------------------------------------------------
/** Initialize 2D animation collection */
// ------------------------------------------------------------------------------------------------
void Anim2DCollection::init( void )
{
	INI ini;

	ini.loadFileDirectory( "Data\\INI\\Animation2D", INI_LOAD_OVERWRITE, NULL );

}

// ------------------------------------------------------------------------------------------------
/** System update phase */
// ------------------------------------------------------------------------------------------------
void Anim2DCollection::update( void )
{
	Anim2D *anim;

	// go through all our animations
	for( anim = m_instanceList; anim; anim = anim->m_collectionSystemNext )
	{

		// try to update the frame
		if( BitIsSet( anim->getStatus(), ANIM_2D_STATUS_FROZEN ) == FALSE )
			anim->tryNextFrame();

	}

}

// ------------------------------------------------------------------------------------------------
/** Search the template list for a template with a matching name */
// ------------------------------------------------------------------------------------------------
Anim2DTemplate *Anim2DCollection::findTemplate( const AsciiString& name )
{

	// search the list
	for( Anim2DTemplate *animTemplate = m_templateList;
			 animTemplate;
			 animTemplate = animTemplate->friend_getNextTemplate() )
	{

		if( animTemplate->getName() == name )
			return animTemplate;

	}

	return NULL;  // template not found

}

//-------------------------------------------------------------------------------------------------
Anim2DTemplate* Anim2DCollection::getNextTemplate( Anim2DTemplate *animTemplate ) const
{
	if( animTemplate )
	{
		return animTemplate->friend_getNextTemplate();
	}
	return NULL;
}

// ------------------------------------------------------------------------------------------------
/** Allocate a new template, assign name, and link to our internal list */
// ------------------------------------------------------------------------------------------------
Anim2DTemplate *Anim2DCollection::newTemplate( const AsciiString& name )
{

	// allocate a new template
	Anim2DTemplate *animTemplate = newInstance(Anim2DTemplate)( name );

	// link to our template list
	animTemplate->friend_setNextTemplate( m_templateList );
	m_templateList = animTemplate;

	// return the new template
	return animTemplate;

}

// ------------------------------------------------------------------------------------------------
/** Register animation instance with us.  When an animation instance is registered it can
	* be updated even when it's not drawn */
// ------------------------------------------------------------------------------------------------
void Anim2DCollection::registerAnimation( Anim2D *anim )
{

	// sanity
	if( anim == NULL )
		return;

	// sanity
	DEBUG_ASSERTCRASH( anim->m_collectionSystemNext == NULL &&
										 anim->m_collectionSystemPrev == NULL,
										 ("Registering animation instance, instance '%s' is already in a system",
										 anim->getAnimTemplate()->getName().str()) );

	// tie to our list
	anim->m_collectionSystemPrev = NULL;
	anim->m_collectionSystemNext = m_instanceList;
	if( m_instanceList )
		m_instanceList->m_collectionSystemPrev = anim;
	m_instanceList = anim;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void Anim2DCollection::unRegisterAnimation( Anim2D *anim )
{

	// sanity
	if( anim == NULL )
		return;

	// if animation is not registered with us do nothing
	if( anim->m_collectionSystem != this )
		return;

	// unlink from our instnace list
	if( anim->m_collectionSystemNext )
		anim->m_collectionSystemNext->m_collectionSystemPrev = anim->m_collectionSystemPrev;
	if( anim->m_collectionSystemPrev )
		anim->m_collectionSystemPrev->m_collectionSystemNext = anim->m_collectionSystemNext;
	else
		m_instanceList = anim->m_collectionSystemNext;

}


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

// FILE: Image.h //////////////////////////////////////////////////////////////////////////////////
// Created:    Colin Day, June 2001
// Desc:       High level representation of images, this is currently being
//						 written so we have a way to refer to images in the windows GUI.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __IMAGE_H_
#define __IMAGE_H_

#include "Common/AsciiString.h"
#include "Common/GameMemory.h"
#include "Common/SubsystemInterface.h"
#include <map>

struct FieldParse;
class INI;

//-------------------------------------------------------------------------------------------------
/** Image status bits.  Keep in sync with imageStatusNames[] */
//-------------------------------------------------------------------------------------------------
typedef enum
{
	IMAGE_STATUS_NONE									= 0x00000000,
	IMAGE_STATUS_ROTATED_90_CLOCKWISE	= 0x00000001,  // image should be treated as rotated
	IMAGE_STATUS_RAW_TEXTURE					= 0x00000002,  // image struct contains raw texture data

} ImageStatus;
#ifdef DEFINE_IMAGE_STATUS_NAMES
static const char *imageStatusNames[] =
{
	"ROTATED_90_CLOCKWISE",
	"RAW_TEXTURE",
	NULL
};
#endif  // end DEFINE_IMAGE_STATUS_NAMES

//-------------------------------------------------------------------------------------------------
/** Image bitmap information */
//-------------------------------------------------------------------------------------------------
class Image : public MemoryPoolObject
{

MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( Image, "Image" );

public:

	Image( void );
	// virtual desctructor defined by memory pool object

	void setName( AsciiString name );							///< set image name
	AsciiString getName( void ) const;									///< return name
	void setFilename( AsciiString filename );			///< set filename
	AsciiString getFilename( void ) const;							///< return filename
	void setUV( Region2D *uv );										///< set UV coord range
	const Region2D *getUV( void ) const;											///< get UV coords
	void setTextureWidth( Int width );						///< set width of texture page this image is on
	void setTextureHeight( Int height );					///< set height of texture page this image is on
	const ICoord2D *getTextureSize( void ) const;							///< return the texture size defined
	void setImageSize( ICoord2D *size );					///< set image width and height
	const ICoord2D *getImageSize( void ) const;								///< get size
	Int getImageWidth( void ) const;										///< get width
	Int getImageHeight( void ) const;										///< get height
	void setRawTextureData( void *data );					///< set raw texture data
	const void *getRawTextureData( void ) const;							///< get raw texture data
	UnsignedInt setStatus( UnsignedInt bit );			///< set status bit
	UnsignedInt clearStatus( UnsignedInt bit );		///< clear status bit
	UnsignedInt getStatus( void ) const;								///< get status bits


	// for parsing from INI
	const FieldParse *getFieldParse( void ) const { return m_imageFieldParseTable; }
	static void parseImageCoords( INI* ini, void *instance, void *store, const void* /*userData*/ );
	static void parseImageStatus( INI* ini, void *instance, void *store, const void* /*userData*/ );

protected:

friend class ImageCollection;

	AsciiString m_name;				///< name for this image
	AsciiString m_filename;		///< texture filename this image is in
	ICoord2D m_textureSize;		///< size of the texture this image is a part of
	Region2D m_UVCoords;			///< texture UV coords for image
	ICoord2D m_imageSize;			///< dimensions of image
	void *m_rawTextureData;		///< raw texture data
	UnsignedInt m_status;			///< status bits from ImageStatus

	static const FieldParse m_imageFieldParseTable[];		///< the parse table for INI definition

};

//-------------------------------------------------------------------------------------------------
/** A collection of images */
//-------------------------------------------------------------------------------------------------
class ImageCollection : public SubsystemInterface
{

public:

	ImageCollection( void );
	virtual ~ImageCollection( void );

	virtual void init( void ) { };				///< initialize system
	virtual void reset( void ) { };				///< reset system
	virtual void update( void ) { };			///< update system

	void load( Int textureSize );												 ///< load images

	const Image *findImageByName( const AsciiString& name );					 ///< find image based on name

  /// adds the given image to the collection, transfers ownership to this object
  void addImage(Image *image);

  /// enumerates the list of existing images
  Image *Enum(unsigned index)
  {
    for (std::map<unsigned,Image *>::iterator i=m_imageMap.begin();i!=m_imageMap.end();++i)
      if (!index--)
        return i->second;
    return NULL;
  }

protected:
  std::map<unsigned,Image *> m_imageMap;  ///< maps named keys to images
};

// INLINING ///////////////////////////////////////////////////////////////////////////////////////
inline void Image::setName( AsciiString name ) { m_name = name; }
inline AsciiString Image::getName( void ) const { return m_name; }
inline void Image::setFilename( AsciiString name ) { m_filename = name; }
inline AsciiString Image::getFilename( void ) const { return m_filename; }
inline void Image::setUV( Region2D *uv ) { if( uv ) m_UVCoords = *uv; }
inline const Region2D *Image::getUV( void ) const { return &m_UVCoords; }
inline void Image::setTextureWidth( Int width ) { m_textureSize.x = width; }
inline void Image::setTextureHeight( Int height ) { m_textureSize.y = height; }
inline void Image::setImageSize( ICoord2D *size ) { m_imageSize = *size; }
inline const ICoord2D *Image::getImageSize( void ) const { return &m_imageSize; }
inline const ICoord2D *Image::getTextureSize( void ) const { return &m_textureSize; }
inline Int Image::getImageWidth( void ) const { return m_imageSize.x; }
inline Int Image::getImageHeight( void ) const { return m_imageSize.y; }
inline void Image::setRawTextureData( void *data ) { m_rawTextureData = data; }
inline const void *Image::getRawTextureData( void ) const { return m_rawTextureData; }
inline UnsignedInt Image::getStatus( void ) const { return m_status; }

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern ImageCollection *TheMappedImageCollection;  ///< mapped images

#endif // __IMAGE_H_


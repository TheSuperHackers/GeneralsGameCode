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

// FILE: ImagePacker.h ////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    ImagePacker
//
// File name:  ImagePacker.h
//
// Created:    Colin Day, August 2001
//
// Desc:       Image packer tool
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <windows.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"
#include "WWLib/TARGA.h"
#include "ImageDirectory.h"
#include "ImageInfo.h"
#include "TexturePage.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define MAX_OUTPUT_FILE_LEN 128
#define DEFAULT_TARGET_SIZE 512

// ImagePacker ----------------------------------------------------------------
// Class interface for running the image packer */
//-----------------------------------------------------------------------------
class ImagePacker
{

public:

	enum
	{
		GAP_METHOD_EXTEND_RGB	= 0x00000001,  ///< extend RGB (no alpha) of image on all sides
		GAP_METHOD_GUTTER			= 0x00000002,  ///< put transparent gutter on right and bottom side of image
	};

public:

	ImagePacker();
	virtual ~ImagePacker();

	Bool init();  ///< initialize the system
	Bool process();  ///< run the process
	Bool getSettingsFromDialog( HWND dialog );  ///< get the options for exection

	void setWindowHandle( HWND hWnd );  ///< set window handle for 'dialog' app
	HWND getWindowHandle();  ///< get window handle for 'dialog' app

	ICoord2D *getTargetSize();  ///< get target size
	Int getTargetWidth();  ///< get target width
	Int getTargetHeight();  ///< bet target height

	void statusMessage( const char *message );  ///< set a status message

	UnsignedInt getImageCount();  ///< get image count
	ImageInfo *getImage( Int index );  ///< get image
	TexturePage *getFirstTexturePage();  ///< get first texture page

	UnsignedInt getPageCount();  ///< get the count of texutre pages

	void setTargetPreviewPage( Int page );  ///< set the target preview page to view
	Int getTargetPreviewPage();  ///< get the target preview page to view

	void setGutter( UnsignedInt size );  ///< set gutter size in pixels
	UnsignedInt getGutter();  ///< get gutter size in pixels
	void setGapMethod( UnsignedInt methodBit );  ///< set gap method option
	void clearGapMethod( UnsignedInt methodBit );  ///< clear gap method option
	UnsignedInt getGapMethod();  ///< get gap method option

	void setOutputAlpha( Bool outputAlpha );  ///< set output alpha option
	Bool getOutputAlpha();  ///< get output alpha option

	void setPreviewWindow( HWND window );  ///< assign preview window handle
	HWND getPreviewWindow();  ///< get the preview window handle

	void setUseTexturePreview( Bool use );  ///< use the real image data in preview
	Bool getUseTexturePreview();  ///< get texture preview option

	void setINICreate( Bool create );  ///< set create INI file option
	Bool createINIFile();  ///< get create INI option

	char *getOutputFile();  ///< get output filename
	char *getOutputDirectory();  ///< get output directory

	void setCompressTextures( Bool compress );  ///< set compress textures option
	Bool getCompressTextures();  ///< get compress textures option

protected:

	void setTargetSize( Int width, Int height );  ///< set the size of the output target image
	Bool checkOutputDirectory();  ///< verify output directory is OK

	void resetImageDirectoryList();  ///< clear the image directory list
	void resetImageList();  ///< clear the image list
	void resetPageList();  ///< clear the texture page list
	void addDirectory( char *path, Bool subDirs );  ///< add directory to directory list
	void addImagesInDirectory( char *dir );  ///< add all images from the specified directory
	void addImage( char *path );  ///< add image to image list
	Bool validateImages();  ///< validate that the loaded images can all be processed
	Bool packImages();  ///< do the packing
	void writeFinalTextures();  ///< write the packed textures

	Bool generateINIFile();  ///< generate the INI file for this image set

	TexturePage *createNewTexturePage();  ///< create a new texture page

	void sortImageList();  ///< sort the image list

	HWND m_hWnd;  ///< window handle for app
	ICoord2D m_targetSize;  ///< the target size
	Bool m_useSubFolders;  ///< use subfolders option
	char m_outputFile[ MAX_OUTPUT_FILE_LEN ];  ///< output filename
	char m_outputDirectory[ _MAX_PATH ];  ///< destination for texture files

	ImageDirectory *m_dirList;  ///< the directory list
	UnsignedInt m_dirCount;  ///< length of dirList
	UnsignedInt m_imagesInDirs;  ///< number of images in all directories
	ImageInfo **m_imageList;  ///< the image list
	UnsignedInt m_imageCount;  ///< length of imageList
	char m_statusBuffer[ 1024 ];  ///< for printing status messages
	TexturePage *m_pageTail;  ///< end of the texture page list
	TexturePage *m_pageList;  ///< the final images generated from the packer
	UnsignedInt m_pageCount;  ///< length of page list
	UnsignedInt m_gapMethod;  ///< gap method option bits
	UnsignedInt m_gutterSize;  ///< gutter gaps between images in pixels
	Bool m_outputAlpha;  ///< final image files will have an alpha channel
	Bool m_createINI;  ///< create the INI file from compressed image data

	Int m_targetPreviewPage;  ///< preview page we're looking at
	HWND m_hWndPreview;  ///< the preview window
	Bool m_showTextureInPreview;  ///< show actual texture in preview window

	Targa *m_targa;  ///< targa for loading file headers
	Bool m_compressTextures;  ///< compress the final textures

};

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
inline void ImagePacker::setTargetSize( Int width, Int height ) { m_targetSize.x = width; m_targetSize.y = height; }
inline ICoord2D *ImagePacker::getTargetSize() { return &m_targetSize; }
inline Int ImagePacker::getTargetWidth() { return m_targetSize.x; }
inline Int ImagePacker::getTargetHeight() { return m_targetSize.y; }
inline void ImagePacker::setWindowHandle( HWND hWnd ) { m_hWnd = hWnd; }
inline HWND ImagePacker::getWindowHandle() { return m_hWnd; }
inline UnsignedInt ImagePacker::getImageCount() { return m_imageCount; }
inline ImageInfo *ImagePacker::getImage( Int index ) { return m_imageList[ index ]; }
inline void ImagePacker::setTargetPreviewPage( Int page ) { m_targetPreviewPage = page; }
inline Int ImagePacker::getTargetPreviewPage() { return m_targetPreviewPage; }
inline UnsignedInt ImagePacker::getPageCount() { return m_pageCount; }
inline void ImagePacker::setPreviewWindow( HWND window ) { m_hWndPreview = window; }
inline HWND ImagePacker::getPreviewWindow() { return m_hWndPreview; }
inline void ImagePacker::setGutter( UnsignedInt size ) { m_gutterSize = size; }
inline UnsignedInt ImagePacker::getGutter() { return m_gutterSize; }
inline void ImagePacker::setOutputAlpha( Bool outputAlpha ) { m_outputAlpha = outputAlpha; }
inline Bool ImagePacker::getOutputAlpha() { return m_outputAlpha; }
inline TexturePage *ImagePacker::getFirstTexturePage() { return m_pageList; }
inline void ImagePacker::setUseTexturePreview( Bool use ) { m_showTextureInPreview = use; }
inline Bool ImagePacker::getUseTexturePreview() { return m_showTextureInPreview; }
inline void ImagePacker::setINICreate( Bool create ) { m_createINI = create; };
inline Bool ImagePacker::createINIFile() { return m_createINI; }
inline char *ImagePacker::getOutputFile() { return m_outputFile; }
inline char *ImagePacker::getOutputDirectory() { return m_outputDirectory; }
inline void ImagePacker::setCompressTextures( Bool compress ) { m_compressTextures = compress; }
inline Bool ImagePacker::getCompressTextures() { return m_compressTextures; }
inline void ImagePacker::setGapMethod( UnsignedInt methodBit ) { BitSet( m_gapMethod, methodBit ); }
inline void ImagePacker::clearGapMethod( UnsignedInt methodBit ) { BitClear( m_gapMethod, methodBit ); }
inline UnsignedInt ImagePacker::getGapMethod() { return m_gapMethod; }

///////////////////////////////////////////////////////////////////////////////
// EXTERNALS //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern ImagePacker *TheImagePacker;

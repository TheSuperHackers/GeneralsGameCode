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

// FILE: ImagePacker.cpp //////////////////////////////////////////////////////
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
// File name:  ImagePacker.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Entry point for the image packer.  This program takes
//						 separate image files and combines them into a single 
//						 image as close as possible so that we can conserve texture
//						 memory
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdio.h>
#include <io.h>
#include <assert.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "WWLib/Targa.h"
#include "Resource.h"
#include "ImagePacker.h"
#include "WinMain.h"
#include "WindowProc.h"

// DEFINES ////////////////////////////////////////////////////////////////////
char *gAppPrefix = "ip_";	// So IP can have a different debug log file name if we need it.

// PRIVATE TYPES //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ImagePacker *TheImagePacker = NULL;

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// ImagePacker::createNewTexturePage ==========================================
/** Create a new texture page and add to the list */
//=============================================================================
TexturePage *ImagePacker::createNewTexturePage( void )
{
	TexturePage *page;

	// allocate new page
	page = new TexturePage( getTargetWidth(), getTargetHeight() );
	if( page == NULL )
	{

		DEBUG_ASSERTCRASH( page, ("Unable to allocate new texture page.\n") );
		return NULL;

	}  // end if

	// link page to list
	page->m_prev = NULL;
	page->m_next = m_pageList;
	if( m_pageList )
		m_pageList->m_prev = page;
	m_pageList = page;

	// add the tail pointer if this is the first page
	if( m_pageTail == NULL )
		m_pageTail = page;

	// we got a new page now
	m_pageCount++;

	// set page id as the current page count
	page->setID( m_pageCount );

	return page;

}  // end createNewTexturePage

// ImagePacker::validateImages ================================================
/** Check all the images in the image list, if any of them cannot be
	* processed we will flag them as so.  If we have some images that can't
	* be processed, we will warn the user of these images and ask them
	* whether or not to proceed.
	*
	* Returns TRUE to proceed
	* Returns FALSE to cancel build
	*/
//=============================================================================
Bool ImagePacker::validateImages( void )
{
	UnsignedInt i;
	ImageInfo *image;
	Bool errors = FALSE;
	Bool proceed = TRUE;

	// loop through all images
	for( i = 0; i < m_imageCount; i++ )
	{

		// get this image
		image = m_imageList[ i ];

		// sanity
		if( image == NULL )
		{

			DEBUG_ASSERTCRASH( image, ("Image in imagelist is NULL") );
			continue;  // should never happen

		}  // end if

		//
		// if this image is too big to fit in the target page size as a whole
		// then there is nothing we can do about it
		//
		if( image->m_size.x > getTargetWidth() ||
				image->m_size.y > getTargetHeight() )
		{

			errors = TRUE;
			BitSet( image->m_status, ImageInfo::TOOBIG );
			BitSet( image->m_status, ImageInfo::CANTPROCESS );

		}  // end if

		//
		// if this image is not the right format we can't process it, at 
		// present we only understand 32 and 24 bit images
		//
		if( image->m_colorDepth != 32 && image->m_colorDepth != 24 )
		{
		
			errors = TRUE;
			BitSet( image->m_status, ImageInfo::INVALIDCOLORDEPTH );
			BitSet( image->m_status, ImageInfo::CANTPROCESS );
			
		}  // end if

	}  // end for i

	//
	// if we have errors, build a list and show them to the user
	//
	if( errors == TRUE )
	{

		proceed = DialogBox( ApplicationHInstance, 
												 (LPCTSTR)IMAGE_ERRORS,
												 TheImagePacker->getWindowHandle(), 
												 (DLGPROC)ImageErrorProc );

	}  // end if

	return proceed;

}  // end validateImages

// ImagePacker::packImages ====================================================
/** Pack all the images in the image list, starting from the top and
	* working from there */
//=============================================================================
Bool ImagePacker::packImages( void )
{
	UnsignedInt i;
	TexturePage *page = NULL;
	ImageInfo *image = NULL;

	//
	// first sanity check all images loaded, if there are images that cannot
	// be processed the user will be given a list of these and asked wether
	// or not to proceed
	//
	Bool proceed;
	proceed = validateImages();
	if( proceed == FALSE )
	{

		statusMessage( "Build Cancelled By User." );
		return FALSE;

	}  // end if

	// loop through all images
	for( i = 0; i < m_imageCount; i++ )
	{

		// update status
		sprintf( m_statusBuffer, "Fitting Image %d of %d.", i, m_imageCount );
		statusMessage( m_statusBuffer );
		 
		// get this image out of the list
		image = m_imageList[ i ];

		// ignore images that we cannot process
		if( BitIsSet( image->m_status, ImageInfo::CANTPROCESS) )
			continue;

		// try to put image on each page
		for( page = m_pageTail; page; page = page->m_prev )
		{

			if( page->addImage( image ) == TRUE )
				break;  // page added, stop trying to add into pages

		}  // end for page

		// if image was not able to go on any existing page create a new page for it
		if( page == NULL )
		{

			page = createNewTexturePage();
			if( page == NULL )
				return FALSE;

			// try to add the image to this page
			if( page->addImage( image ) == FALSE )
			{
				char buffer[ _MAX_PATH ];

				sprintf( buffer, "Unable to add image '%s' to a brand new page!\n", image->m_path );
				DEBUG_ASSERTCRASH( 0, (buffer) );
				MessageBox( NULL, buffer, "Internal Error", MB_OK | MB_ICONERROR );
				return FALSE;

			}  // end if

		}  // end if

	}  // end for i

	return TRUE;  // success

}  // end packImages

// ImagePacker::writeFinalTextures ============================================
/** Generate and write the final textures to the output directory
	* of the packed images along with a definition file for which images
	* are where on the page */
//=============================================================================
void ImagePacker::writeFinalTextures( void )
{
	TexturePage *page;
	Bool errors = FALSE;
	char buffer[ 128 ];

	//
	// go through each page, let's start from the end of the list since
	// that's where we packed first, but it doesn't matter
	//
	for( page = m_pageTail; page; page = page->m_prev )
	{

		// update status message
		sprintf( buffer, "Generating texture #%d of %d.", 
						 page->getID(), m_pageCount );
		statusMessage( buffer );

		// generate the final texture for this page
		if( page->generateTexture() == FALSE )
		{
			
			errors = TRUE;
			continue;  // could not generate this page, but try to continue

		}  // end if

		//
		// write this page out to a file using the filename given by
		// the user and the texture page ID to keep it unique
		//
		if( page->writeFile( m_outputFile ) == FALSE )
		{

			errors = TRUE;
			continue;  // could not write page, but try to go on

		}  // end if

	}  // end for page

	// check for any errors and notify the user
	if( errors == TRUE )
	{

		DialogBox( ApplicationHInstance, 
							 (LPCTSTR)PAGE_ERRORS,
							 TheImagePacker->getWindowHandle(), 
							 (DLGPROC)PageErrorProc );

	}  // end if

}  // end writeFinalTextures

// sortImageCompare ===========================================================
/** Compare function for qsort
	* -1 item1 less than item2 
	*	0 item1 identical to item2 
	*	1 item1 greater than item2
	*/
//=============================================================================
static Int sortImageCompare( const void *aa, const void *bb )
{
	const ImageInfo **a = (const ImageInfo **)aa;
	const ImageInfo **b = (const ImageInfo **)bb;

	if( (*a)->m_area < (*b)->m_area )
		return 1;
	else if( (*a)->m_area > (*b)->m_area )
		return -1;
	else
		return 0;

}  // end sortImageCompare

// ImagePacker::sortImageList =================================================
/** Sort the image list */
//=============================================================================
void ImagePacker::sortImageList( void )
{

	// sort all images so that largest area ones are first
	qsort( (void *)m_imageList, m_imageCount, sizeof( ImageInfo *), sortImageCompare );

}  // end sortImageList

// ImagePacker::addImagesInDirectory ==========================================
/** Add all the images in the specified directory */
//=============================================================================
void ImagePacker::addImagesInDirectory( char *dir )
{

	// sanity
	if( dir == NULL )
		return;

	char currDir[ _MAX_PATH ];
	char filePath[ _MAX_PATH ];
	WIN32_FIND_DATA item;  // search item
	HANDLE hFile;  // handle for search resources
	Int len;

	// save the current directory
	GetCurrentDirectory( _MAX_PATH, currDir );

	// change into the directory
	SetCurrentDirectory( dir );

	// go through each item in the output directory		
	hFile = FindFirstFile( "*", &item);
	if( hFile != INVALID_HANDLE_VALUE )
	{  

		// if this is a file count it
		if( !(item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
				strcmp( item.cFileName, "." ) && 
				strcmp( item.cFileName, ".." ) )
		{

			len = strlen( item.cFileName );
			if( len > 4 && 
					item.cFileName[ len - 4 ] == '.' &&
					(item.cFileName[ len - 3 ] == 't' || item.cFileName[ len - 3 ] == 'T') &&
					(item.cFileName[ len - 2 ] == 'g' || item.cFileName[ len - 2 ] == 'G') &&
					(item.cFileName[ len - 1 ] == 'a' || item.cFileName[ len - 1 ] == 'A') )
			{

				sprintf( filePath, "%s%s", dir, item.cFileName );
				addImage( filePath );

			}  // end if

		}  // end if

		// find the rest of the files
		while( FindNextFile( hFile, &item ) != 0 )
		{

			// if this is a file count it
			if( !(item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
					strcmp( item.cFileName, "." ) && 
					strcmp( item.cFileName, ".." ) )
			{

				len = strlen( item.cFileName );
				if( len > 4 && 
						item.cFileName[ len - 4 ] == '.' &&
						(item.cFileName[ len - 3 ] == 't' || item.cFileName[ len - 3 ] == 'T') &&
						(item.cFileName[ len - 2 ] == 'g' || item.cFileName[ len - 2 ] == 'G') &&
						(item.cFileName[ len - 1 ] == 'a' || item.cFileName[ len - 1 ] == 'A') )
				{

					sprintf( filePath, "%s%s", dir, item.cFileName );
					addImage( filePath );

				}  // end if

			}  // end if

		}  // end while

		// close search
		FindClose( hFile );

	}  //end if, items found

	// restore our current directory
	SetCurrentDirectory( currDir );

}  // end addImagesInDirectory

// ImagePacker::checkOutputDirectory ==========================================
/** Verify that there are no files in the output directory ... if there
	* are give the user the option to delete them, cancel the operation,
	* or proceed with possibly overwriting any files there
	*
	* Returns TRUE to proceed with the process, FALSE if the user wants
	* to cancel the process
	*/
//=============================================================================
Bool ImagePacker::checkOutputDirectory( void )
{
	WIN32_FIND_DATA item;  // search item
	HANDLE hFile;  // handle for search resources
	Int fileCount = 0;
	char currDir[ _MAX_PATH ];
	
	// get the working directory
	GetCurrentDirectory( _MAX_PATH, currDir );

	// create the output directory if it does not exist
	CreateDirectory( m_outputDirectory, NULL );

	// change into the output directory
	SetCurrentDirectory( m_outputDirectory );

	// go through each item in the output directory		
	hFile = FindFirstFile( "*", &item);
	if( hFile != INVALID_HANDLE_VALUE )
	{  

		// if this is a file count it
		if( !(item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
				 strcmp( item.cFileName, "." ) && 
				 strcmp( item.cFileName, ".." ) )
			fileCount++;

		// find the rest of the files
		while( FindNextFile( hFile, &item ) != 0 )
		{

			// if this is a file count it
			if( !(item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
					 strcmp( item.cFileName, "." ) && 
					 strcmp( item.cFileName, ".." ) )
				fileCount++;

		}  // end while

		// close search
		FindClose( hFile );

	}  //end if, items found

	// switch back to the current directory
	SetCurrentDirectory( currDir );

	if( fileCount != 0 )
	{
		char buffer[ 256 ];
		Int response;

		sprintf( buffer, "The output directory (%s) must be empty before proceeding.  Delete '%d' files and continue with build process?",
						 m_outputDirectory, fileCount );
		response = MessageBox( NULL, buffer, 
													 "Delete files to continue?", 
													 MB_YESNO | MB_ICONWARNING );

		// if they said no, do not delete the files and abort the pack process
		if( response == IDNO )
			return FALSE;

		//
		// they said yes, delete all the files in the output directory
		//

		// change into the output directory
		SetCurrentDirectory( m_outputDirectory );
		
		// go through each item in the output directory		
		hFile = FindFirstFile( "*", &item);
		if( hFile != INVALID_HANDLE_VALUE )
		{  

			// if this is a file count it
			if( !(item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
					 strcmp( item.cFileName, "." ) && 
					 strcmp( item.cFileName, ".." ) )
				DeleteFile( item.cFileName );

			// find the rest of the files
			while( FindNextFile( hFile, &item ) != 0 )
			{

				// if this is a file count it
				if( !(item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
						 strcmp( item.cFileName, "." ) && 
						 strcmp( item.cFileName, ".." ) )
					DeleteFile( item.cFileName );

			}  // end while

			// close search
			FindClose( hFile );

		}  //end if, items found

		// switch back to the current directory
		SetCurrentDirectory( currDir );

	}  // end if

	return TRUE;  // proceed

}  // end checkOutputDirectory

// ImagePacker::resetPageList =================================================
/** Clear the page list */
//=============================================================================
void ImagePacker::resetPageList( void )
{
	TexturePage *next;

	while( m_pageList )
	{

		next = m_pageList->m_next;
		delete m_pageList;
		m_pageList = next;

	}  // end while

	m_pageTail = NULL;
	m_pageCount = 0;
	m_targetPreviewPage = 1;

}  // end resetPageList

// ImagePacker::resetImageDirectoryList =======================================
/** Clear the image directory list */
//=============================================================================
void ImagePacker::resetImageDirectoryList( void )
{
	ImageDirectory *next;

	while( m_dirList )
	{

		next = m_dirList->m_next;
		delete m_dirList;
		m_dirList = next;
			
	}  // end while

	m_dirCount = 0;
	m_imagesInDirs = 0;

}  // end resetImageDirectoryList

// ImagePacker::resetImageList ================================================
/** Clear the image list */
//=============================================================================
void ImagePacker::resetImageList( void )
{

	if( m_imageList )
		delete [] m_imageList;
	m_imageList = NULL;
	m_imageCount = 0;

}  // end resetImageList

// ImagePacker::addDirectory ==================================================
/** Add the directory to the directory list, do not add it if it is already
	* in the directory list.  We want to have that sanity check so that
	* we can be assured that each image being added to the image list from
	* each directory will be unique and we therefore don't have to do
	* any further checking for duplicates */
//=============================================================================
void ImagePacker::addDirectory( char *path, Bool subDirs )
{
	char currDir[ _MAX_PATH ];
	WIN32_FIND_DATA item;  // search item
	HANDLE hFile;  // handle for search resources

	// santiy
	if( path == NULL )
		return;

	// check to see if path is already in list
	ImageDirectory *dir;
	for( dir = m_dirList; dir; dir = dir->m_next )
		if( stricmp( dir->m_path, path ) == 0 )
			return;  // already in list

	// save our current directory
	GetCurrentDirectory( _MAX_PATH, currDir );

	// set our directory to this one
	if( SetCurrentDirectory( path ) == 0 )
		return;  // directory does not exist

	// image is not in list, make a new entry and link to the list
	dir = new ImageDirectory;
	if( dir == NULL )
	{
		
		MessageBox( NULL, "Unable to allocate image directory", "Error", 
								MB_OK | MB_ICONERROR );
		return;

	}  // end if

	// allocate space for the path
	Int len = strlen( path );
	dir->m_path = new char[ len + 1 ];
	strcpy( dir->m_path, path );
	if( dir->m_path == NULL )
	{

		MessageBox( NULL, "Unable to allocate path for directory", "Error",
								MB_OK | MB_ICONERROR );
		delete dir;
		return;

	}  // end if

	// tie to list
	dir->m_prev = NULL;
	dir->m_next = m_dirList;
	if( m_dirList )
		m_dirList->m_prev = dir;
	m_dirList = dir;
	
	// increase our directory count
	m_dirCount++;

	// update status
	sprintf( m_statusBuffer, "Folder Added: %d.", m_dirCount );
	statusMessage( m_statusBuffer );

	// count how many image files are in this directory
	hFile = FindFirstFile( "*", &item);
	if( hFile != INVALID_HANDLE_VALUE )
	{  

		// if this is a file count it
		if( !(item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
				strcmp( item.cFileName, "." ) && 
				strcmp( item.cFileName, ".." ) )
		{

			len = strlen( item.cFileName );
			if( len > 4 && 
					item.cFileName[ len - 4 ] == '.' &&
					(item.cFileName[ len - 3 ] == 't' || item.cFileName[ len - 3 ] == 'T') &&
					(item.cFileName[ len - 2 ] == 'g' || item.cFileName[ len - 2 ] == 'G') &&
					(item.cFileName[ len - 1 ] == 'a' || item.cFileName[ len - 1 ] == 'A') )
				dir->m_imageCount++;

		}  // end if

		// find the rest of the files
		while( FindNextFile( hFile, &item ) != 0 )
		{

			// if this is a file count it
			if( !(item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
					strcmp( item.cFileName, "." ) && 
					strcmp( item.cFileName, ".." ) )
			{

				len = strlen( item.cFileName );
				if( len > 4 && 
						item.cFileName[ len - 4 ] == '.' &&
						(item.cFileName[ len - 3 ] == 't' || item.cFileName[ len - 3 ] == 'T') &&
						(item.cFileName[ len - 2 ] == 'g' || item.cFileName[ len - 2 ] == 'G') &&
						(item.cFileName[ len - 1 ] == 'a' || item.cFileName[ len - 1 ] == 'A') )
					dir->m_imageCount++;

			}  // end if

		}  // end while

		// close search
		FindClose( hFile );

	}  //end if, items found

	// add the image count of this directory to the total image count
	m_imagesInDirs += dir->m_imageCount;

	// if we are adding subdirectories add them all
	if( subDirs )
	{
		char subDir[ _MAX_PATH ];

		// go through each item in the output directory		
		hFile = FindFirstFile( "*", &item);
		if( hFile != INVALID_HANDLE_VALUE )
		{  

			// if this is a file count it
			if( (item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
					strcmp( item.cFileName, "." ) && 
					strcmp( item.cFileName, ".." ) )
			{

				sprintf( subDir, "%s%s\\", path, item.cFileName );
				addDirectory( subDir, subDirs );

			}  // end if

			// find the rest of the files
			while( FindNextFile( hFile, &item ) != 0 )
			{

				// if this is a file count it
				if( (item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
						strcmp( item.cFileName, "." ) && 
						strcmp( item.cFileName, ".." ) )
				{

					sprintf( subDir, "%s%s\\", path, item.cFileName );
					addDirectory( subDir, subDirs );

				}  // end if

			}  // end while

			// close search
			FindClose( hFile );

		}  //end if, items found

	}  // end if

	// restore our current directory
	SetCurrentDirectory( currDir );

}  // end addDirectory

// ImagePacker::addImage ======================================================
/** Add the image to the image list */
//=============================================================================
void ImagePacker::addImage( char *path )
{

	// sanity
	if( path == NULL )
		return;

	// allocate a new entry
	ImageInfo *info = new ImageInfo;
	if( info == NULL )
	{

		MessageBox( NULL, "Unable to allocate image info", "Error",
								MB_OK | MB_ICONERROR );
		return;

	}  // end if

	// allocate space for the path
	Int len = strlen( path );
	info->m_path = new char[ len + 1 ];
	strcpy( info->m_path, path );
	if( info->m_path == NULL )
	{

		MessageBox( NULL, "Unable to allcoate image path info", "Error",
								MB_OK | MB_ICONERROR );
		delete info;
		return;

	}  // end if

	// load just the header information from the targa
	m_targa->Load( info->m_path, 0, TRUE );

	// get the data we need out of the targa header
	info->m_colorDepth = m_targa->Header.PixelDepth;
	info->m_size.x = m_targa->Header.Width;
	info->m_size.y = m_targa->Header.Height;
	info->m_area = info->m_size.x * info->m_size.y;

	// save the filename only without path
	Int i;
	char *c;
	for( i = len - 1; i >= 0; i-- )
	{

		if( path[ i ] == '\\' )
		{
			c = &path[ i + 1 ];
			break;
		}

	}  // end for i

	Int nameLen = strlen( c );
	info->m_filenameOnly = new char[ nameLen + 1 ];
	strcpy( info->m_filenameOnly, c );

	info->m_filenameOnlyNoExt = new char[ nameLen - 4 + 1 ];
	strncpy( info->m_filenameOnlyNoExt, c, nameLen - 4 );
	info->m_filenameOnlyNoExt[ nameLen - 4 ] = '\0';

	// assign to array
	m_imageList[ m_imageCount++ ] = info;

	// update status
	sprintf( m_statusBuffer, "Loading Image %d of %d.", 
					 m_imageCount, m_imagesInDirs );
	statusMessage( m_statusBuffer );

}  // end addImage

// ImagePacker::generateINIFile ===============================================
/** Generate the INI image file definition for the final packed images */
//=============================================================================
Bool ImagePacker::generateINIFile( void )
{
	FILE *fp;
	char filename[ _MAX_PATH ];

	// construct filename we'll use
	sprintf( filename, "%s%s.INI", m_outputDirectory, m_outputFile );

	// open the file
	fp = fopen( filename, "w" );
	if( fp == NULL )
	{
		char buffer[ _MAX_PATH + 64 ];

		sprintf( buffer, "Cannot open INI file '%s' for writing.", filename );
		MessageBox( NULL, buffer, "Error Opening File", MB_OK | MB_ICONERROR );
		return FALSE;

	}  // end if

	// print header for file
	fprintf( fp, "; ------------------------------------------------------------\n" );
	fprintf( fp, "; Do NOT edit by hand, ImagePacker.exe auto generated INI file\n" );
	fprintf( fp, "; ------------------------------------------------------------\n\n" );

	//
	// loop through all the pages so that we write image definitions that
	// are on the same page close together in the file, note we're
	// going backwards through the page list because page 1 is at the 
	// tail and I want them to print out in number order, but it
	// doesn't really matter
	//
	TexturePage *page;
	ImageInfo *image;
	for( page = m_pageTail; page; page = page->m_prev )
	{

		// ignore texture pages that generated errors
		if( BitIsSet( page->m_status, TexturePage::PAGE_ERROR ) )
			continue;

		// go through each image on this page
		for( image = page->getFirstImage(); 
				 image; 
				 image = image->m_nextPageImage )
		{

			//
			// write the item definition, note when we output the texture coords
			// we add on to the right and bottom to include that pixel in the
			// texture calculations ... need to do this since we are using a zero
			// based region for the "filled regions" in the image packer
			//
			fprintf( fp, "MappedImage %s\n", image->m_filenameOnlyNoExt );
			fprintf( fp, "  Texture = %s_%03d.tga\n", m_outputFile, page->getID() );
			fprintf( fp, "  TextureWidth = %d\n", page->getWidth() );
			fprintf( fp, "  TextureHeight = %d\n", page->getHeight() );
			fprintf( fp, "  Coords = Left:%d Top:%d Right:%d Bottom:%d\n",
							 image->m_pagePos.lo.x, image->m_pagePos.lo.y,
							 image->m_pagePos.hi.x + 1, image->m_pagePos.hi.y + 1 );
			fprintf( fp, "  Status = %s\n", 
							 BitIsSet( image->m_status, ImageInfo::ROTATED90C ) ? 
												"ROTATED_90_CLOCKWISE" : "NONE" );
			fprintf( fp, "End\n\n" );

		}  // end for image

	}  // end for page

	// close the file
	fclose( fp );

	return TRUE;  // success

}  // end generateINIFile

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// ImagePacker::getSettingsFromDialog =========================================
/** Given the current state of the option dialog passed in, get all the
	* settings we need for the image packer from the GUI and validate them */
//=============================================================================
Bool ImagePacker::getSettingsFromDialog( HWND dialog )
{
	Int i;

	// sanity
	if( dialog == NULL )
		return FALSE;

	// if we are using a user target image size, it must be a power of 2
	if( IsDlgButtonChecked( dialog, RADIO_TARGET_OTHER ) )
	{
		UnsignedInt size, val;
		Int bitCount = 0;

		size = GetDlgItemInt( dialog, EDIT_WIDTH, NULL, FALSE );
		for( val = size; val; val >>= 1 )
			if( BitIsSet( val, 0x1 ) )
				bitCount++;

		//
		// if bit count was not 1, this is not a power of 2 ... it also
		// guards us from entering a size of zero :)
		//
		if( bitCount != 1 )
		{

			MessageBox( NULL, "The target image size must be a power of 2.",
									"Must Be Power Of 2", MB_OK | MB_ICONERROR );
			return FALSE;

		}  // end if

		// set the size for the image packer
		setTargetSize( size, size );

	}  // end if
	else if( IsDlgButtonChecked( dialog, RADIO_128X128 ) )
		setTargetSize( 128, 128 );
	else if( IsDlgButtonChecked( dialog, RADIO_256X256 ) )
		setTargetSize( 256, 256 );
	else if( IsDlgButtonChecked( dialog, RADIO_512X512 ) )
		setTargetSize( 512, 512 );
	else
	{

		MessageBox( NULL, "Internal Error. Target Size Unknown.",
								"Error", MB_OK | MB_ICONERROR );
		return FALSE;

	}  // end else
	
	// get alpha option
	Bool outputAlpha = FALSE;
	if( IsDlgButtonChecked( dialog, CHECK_ALPHA ) == BST_CHECKED )
		outputAlpha = TRUE;
	TheImagePacker->setOutputAlpha( outputAlpha );

	// get create INI option
	Bool createINI = FALSE;
	if( IsDlgButtonChecked( dialog, CHECK_INI ) == BST_CHECKED )
		createINI = TRUE;
	TheImagePacker->setINICreate( createINI );

	// get preview with image option
	Bool useBitmap = FALSE;
	if( IsDlgButtonChecked( dialog, CHECK_BITMAP_PREVIEW ) == BST_CHECKED )
		useBitmap = TRUE;
	TheImagePacker->setUseTexturePreview( useBitmap );

	// get option to compress final textures
	Bool compress = FALSE;
	if( IsDlgButtonChecked( dialog, CHECK_COMPRESS ) == BST_CHECKED )
		compress = TRUE;
	TheImagePacker->setCompressTextures( compress );

	// get options for the gap options
	TheImagePacker->clearGapMethod( ImagePacker::GAP_METHOD_EXTEND_RGB );
	if( IsDlgButtonChecked( dialog, CHECK_GAP_EXTEND_RGB ) == BST_CHECKED )
		TheImagePacker->setGapMethod( ImagePacker::GAP_METHOD_EXTEND_RGB );
	TheImagePacker->clearGapMethod( ImagePacker::GAP_METHOD_GUTTER );
	if( IsDlgButtonChecked( dialog, CHECK_GAP_GUTTER ) == BST_CHECKED )
		TheImagePacker->setGapMethod( ImagePacker::GAP_METHOD_GUTTER );

	// get gutter size whether we are using that option or not
	Int gutter = GetDlgItemInt( dialog, EDIT_GUTTER, NULL, FALSE );
	if( gutter < 0 )
		gutter = 0;
	setGutter( gutter );
						
	// save the filename to output
	GetDlgItemText( dialog, EDIT_FILENAME, m_outputFile, MAX_OUTPUT_FILE_LEN - 1 );
	
	// check for illegal characters in the output name
	Int len = strlen( m_outputFile );
	for( i = 0; i < len; i++ )
	{
		char *illegal = "/\\:*?<>|";
		Int illegalLen = strlen( illegal );
		
		for( Int j = 0; j < illegalLen; j++ )
		{

			if( m_outputFile[ i ] == illegal[ j ] )
			{
				char buffer[ 256 ];

				sprintf( buffer, "Output filename '%s' contains one or more of the following illegal characters:\n\n%s", 
								 m_outputFile, illegal );
				MessageBox( NULL, buffer, "Illegal Filename", MB_OK | MB_ICONERROR );
				return FALSE;

			}  // end if

		}  // end for j

	}  // end for i

	// get the work on sub-folders option
	m_useSubFolders = IsDlgButtonChecked( dialog, CHECK_USE_SUB_FOLDERS );

	// clear our list of image directories
	resetImageDirectoryList();

	// set a status message
	statusMessage( "Gathering Directory Information, Please Wait ..." );

	// add all the image directories specified in the folder listbox
	Int count = SendDlgItemMessage( dialog, LIST_FOLDERS, LB_GETCOUNT, 0, 0 );
	char buffer[ _MAX_PATH ];
	for( i = 0; i < count; i++ )
	{

		// get text from the listbox
		SendDlgItemMessage( dialog, LIST_FOLDERS,		
												LB_GETTEXT, i, (LPARAM)buffer );

		// add the directory
		addDirectory( buffer, m_useSubFolders );

	}  // end for i

	// all done
	return TRUE;

}  // end getSettingsFromDialog

// ImagePacker::ImagePacker ===================================================
/** */
//=============================================================================
ImagePacker::ImagePacker( void )
{

	m_hWnd = NULL;
	m_targetSize.x = DEFAULT_TARGET_SIZE;
	m_targetSize.y = DEFAULT_TARGET_SIZE;
	m_useSubFolders = TRUE;
	strcpy( m_outputFile, "" );
	strcpy( m_outputDirectory, "" );
	m_dirList = NULL;
	m_dirCount = 0;
	m_imagesInDirs = 0;
	m_imageList = NULL;
	m_imageCount = 0;
	strcpy( m_statusBuffer, "" );
	m_pageList = NULL;
	m_pageTail = NULL;
	m_pageCount = 0;
	m_gapMethod = GAP_METHOD_EXTEND_RGB;
	m_gutterSize = 1;
	m_outputAlpha = TRUE;
	m_createINI = TRUE;

	m_targetPreviewPage = 1;
	m_hWndPreview = NULL;
	m_showTextureInPreview = FALSE;

	m_targa = NULL;
	m_compressTextures = FALSE;

}  // end ImagePacker

// ImagePacker::~ImagePacker ==================================================
/** */
//=============================================================================
ImagePacker::~ImagePacker( void )
{

	// delete our lists
	resetImageDirectoryList();
	resetImageList();
	resetPageList();

	// delete our targa header loader
	if( m_targa )
		delete m_targa;

}  // end ~ImagePacker

// ImagePacker::init ==========================================================
/** Initialize the image packer system */
//=============================================================================
Bool ImagePacker::init( void )
{

	// allocate a targa to read the headers for the images
	m_targa = new Targa;
	if( m_targa == NULL )
	{
		
		DEBUG_ASSERTCRASH( m_targa, ("Unable to allocate targa header during init\n") );
		MessageBox( NULL, "ImagePacker can't init, unable to create targa",
								"Internal Error", MB_OK | MB_ICONERROR );
		return FALSE;

	}  // end if

	return TRUE;

}  // end init

// ImagePacker::statusMessage =================================================
/** Status message for the program */
//=============================================================================
void ImagePacker::statusMessage( char *message )
{

	SetDlgItemText( getWindowHandle(), STATIC_STATUS, message );

}  // end statusMessage

// ImagePacker::process =======================================================
/** Run the packing process */
//=============================================================================
Bool ImagePacker::process( void )
{

	// build the output directory based on the base name of the output images
	char currDir[ _MAX_PATH ];
	GetCurrentDirectory( _MAX_PATH, currDir );
	sprintf( m_outputDirectory, "%s\\ImagePackerOutput\\", currDir );
	CreateDirectory( m_outputDirectory, NULL );

	// subdir of output directory based on output image name
	strcat( m_outputDirectory, m_outputFile );
	strcat( m_outputDirectory, "\\" );

	//
	// check for existing images in the output directory ... if we have
	// some then ask the user if they want to delete them or proceed with
	// a possible overwrite
	//
	if( checkOutputDirectory() == FALSE )
	{

		statusMessage( "Build Process Cancelled." );
		return FALSE;

	}  // end if

	// reset the contents of our image list and existing textures
	resetImageList();
	resetPageList();

	// set a status message
	statusMessage( "Gathering Image Information, Please Wait ..." );

	// allocate an array to hold all the images
	m_imageList = new ImageInfo *[ m_imagesInDirs ];

	// load our image list with all the art files from the specified directories
	ImageDirectory *dir;
	for( dir = m_dirList; dir; dir = dir->m_next )
		addImagesInDirectory( dir->m_path );

	// sort the images with the largest biggest images at the top of the list
	sortImageList();

	// pack all images
	if( packImages() )
	{

		// generate the actual final textures and write them out to the file
		writeFinalTextures();

		// generate the INI definition file if requested
		if( createINIFile() == TRUE )
			generateINIFile();

		// update preview window
		UpdatePreviewWindow();

		// all done
		sprintf( m_statusBuffer, "Image Packing Complete: '%d' Texture Pages Generated from '%d' Images in '%d' Folder(s)",
						 m_pageCount, m_imageCount, m_dirCount );
		statusMessage( m_statusBuffer );

	}  // end if

	return TRUE;

}  // end process


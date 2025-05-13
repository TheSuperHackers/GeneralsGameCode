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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/surfaceclass.cpp                       $*
 *                                                                                             *
 *              Original Author:: Nathaniel Hoffman                                            *
 *                                                                                             *
 *                      $Author:: Greg_h2                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/30/01 2:01p                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SurfaceClass::Clear -- Clears a surface to 0                                              *
 *   SurfaceClass::Copy -- Copies a region from one surface to another of the same format      *
 *   SurfaceClass::FindBBAlpha -- Finds the bounding box of non zero pixels in the region (x0, *
 *   PixelSize -- Helper Function to find the size in bytes of a pixel                         *
 *   SurfaceClass::Is_Transparent_Column -- Tests to see if the column is transparent or not   *
 *   SurfaceClass::Copy -- Copies from a byte array to the surface                             *
 *   SurfaceClass::CreateCopy -- Creates a byte array copy of the surface                      *
 *   SurfaceClass::DrawHLine -- draws a horizontal line                                        *
 *   SurfaceClass::DrawPixel -- draws a pixel                                                  *
 *   SurfaceClass::Copy -- Copies a block of system ram to the surface                         *
 *   SurfaceClass::Hue_Shift -- changes the hue of the surface                                 *
 *   SurfaceClass::Is_Monochrome -- Checks if surface is monochrome or not                     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "surfaceclass.h"
#include "formconv.h"
#include "dx8wrapper.h"
#include "vector2i.h"
#include "colorspace.h"
#include "bound.h"
#include <d3dx8.h>

/***********************************************************************************************
 * PixelSize -- Helper Function to find the size in bytes of a pixel                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/

unsigned int PixelSize(const SurfaceClass::SurfaceDescription &sd)
{
	unsigned int size=0;

	switch (sd.Format)
	{	
	case WW3D_FORMAT_A8R8G8B8:
	case WW3D_FORMAT_X8R8G8B8:
		size=4;
		break;
	case WW3D_FORMAT_R8G8B8:
		size=3;
		break;
	case WW3D_FORMAT_R5G6B5:
	case WW3D_FORMAT_X1R5G5B5:
	case WW3D_FORMAT_A1R5G5B5:
	case WW3D_FORMAT_A4R4G4B4:
	case WW3D_FORMAT_A8R3G3B2:
	case WW3D_FORMAT_X4R4G4B4:
	case WW3D_FORMAT_A8P8:	
	case WW3D_FORMAT_A8L8:
		size=2;
		break;
	case WW3D_FORMAT_R3G3B2:
	case WW3D_FORMAT_A8:
	case WW3D_FORMAT_P8:
	case WW3D_FORMAT_L8:
	case WW3D_FORMAT_A4L4:
		size=1;
		break;
	}

	return size;
}

void Convert_Pixel(Vector3 &rgb, const SurfaceClass::SurfaceDescription &sd, const unsigned char * pixel)
{
	const float scale=1/255.0f;
	switch (sd.Format)
	{	
	case WW3D_FORMAT_A8R8G8B8:
	case WW3D_FORMAT_X8R8G8B8:
	case WW3D_FORMAT_R8G8B8:
		{
			rgb.X=pixel[2]; // R
			rgb.Y=pixel[1]; // G
			rgb.Z=pixel[0]; // B
		}
		break;
	case WW3D_FORMAT_A4R4G4B4:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)&pixel[0];
			rgb.X=((tmp&0x0f00)>>4);   // R
			rgb.Y=((tmp&0x00f0));		// G
			rgb.Z=((tmp&0x000f)<<4);	// B			
		}
		break;
	case WW3D_FORMAT_A1R5G5B5:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)&pixel[0];			
			rgb.X=(tmp>>7)&0xf8; // R
			rgb.Y=(tmp>>2)&0xf8; // G
			rgb.Z=(tmp<<3)&0xf8; // B			
		}
		break;
	case WW3D_FORMAT_R5G6B5:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)&pixel[0];			
			rgb.X=(tmp>>8)&0xf8;
			rgb.Y=(tmp>>3)&0xfc;
			rgb.Z=(tmp<<3)&0xf8;
		}
		break;

	default:
		// TODO: Implement other pixel formats
		WWASSERT(0);
	}
	rgb*=scale;
}

// Note: This function must never overwrite the original alpha
void Convert_Pixel(unsigned char * pixel,const SurfaceClass::SurfaceDescription &sd, const Vector3 &rgb)
{
	unsigned char r,g,b;
	r=(unsigned char) (rgb.X*255.0f);
	g=(unsigned char) (rgb.Y*255.0f);
	b=(unsigned char) (rgb.Z*255.0f);
	switch (sd.Format)
	{	
	case WW3D_FORMAT_A8R8G8B8:
	case WW3D_FORMAT_X8R8G8B8:
	case WW3D_FORMAT_R8G8B8:
		pixel[0]=b;
		pixel[1]=g;
		pixel[2]=r;
		break;
	case WW3D_FORMAT_A4R4G4B4:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)&pixel[0];
			tmp&=0xF000;
			tmp|=(r&0xF0) << 4;
			tmp|=(g&0xF0);
			tmp|=(b&0xF0) >> 4;			
			*(unsigned short*)&pixel[0]=tmp;
		}
		break;
	case WW3D_FORMAT_A1R5G5B5:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)&pixel[0];
			tmp&=0x8000;
			tmp|=(r&0xF8) << 7;
			tmp|=(g&0xF8) << 2;
			tmp|=(b&0xF8) >> 3;			
			*(unsigned short*)&pixel[0]=tmp;
		}
		break;
	case WW3D_FORMAT_R5G6B5:
		{
			unsigned short tmp;			
			tmp=(r&0xf8) << 8;
			tmp|=(g&0xfc) << 3;
			tmp|=(b&0xf8) >> 3;
			*(unsigned short*)&pixel[0]=tmp;
		}
		break;
	default:
		// TODO: Implement other pixel formats
		WWASSERT(0);
	}
}

/*************************************************************************
**                             SurfaceClass
*************************************************************************/
SurfaceClass::SurfaceClass(unsigned width, unsigned height, WW3DFormat format):
	D3DSurface(NULL),
	SurfaceFormat(format)
{
	WWASSERT(width);
	WWASSERT(height);
	D3DSurface = DX8Wrapper::_Create_DX8_Surface(width, height, format);
}

SurfaceClass::SurfaceClass(const char *filename):
	D3DSurface(NULL)
{
	D3DSurface = DX8Wrapper::_Create_DX8_Surface(filename);
	SurfaceDescription desc;
	Get_Description(desc);
	SurfaceFormat=desc.Format;
}

SurfaceClass::SurfaceClass(IDirect3DSurface8 *d3d_surface)	:
	D3DSurface (NULL)
{
	Attach (d3d_surface);
	SurfaceDescription desc;
	Get_Description(desc);
	SurfaceFormat=desc.Format;
}

SurfaceClass::~SurfaceClass(void)
{
	if (D3DSurface) {
		D3DSurface->Release();
		D3DSurface = NULL;
	}
}

void SurfaceClass::Get_Description(SurfaceDescription &surface_desc)
{
	D3DSURFACE_DESC d3d_desc;
	::ZeroMemory(&d3d_desc, sizeof(D3DSURFACE_DESC));
	DX8_ErrorCode(D3DSurface->GetDesc(&d3d_desc));
	surface_desc.Format = D3DFormat_To_WW3DFormat(d3d_desc.Format);
	surface_desc.Height = d3d_desc.Height;
	surface_desc.Width = d3d_desc.Width;
}

void * SurfaceClass::Lock(int * pitch)
{
	D3DLOCKED_RECT lock_rect;	
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect, 0, 0));
	*pitch = lock_rect.Pitch;
	return (void *)lock_rect.pBits;
}

void SurfaceClass::Unlock(void)
{
	DX8_ErrorCode(D3DSurface->UnlockRect());
}

/***********************************************************************************************
 * SurfaceClass::Clear -- Clears a surface to 0                                                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Clear()
{
	SurfaceDescription sd;
	Get_Description(sd);

	// size of each pixel in bytes
	unsigned int size=PixelSize(sd);

	D3DLOCKED_RECT lock_rect;	
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,0,0));
	unsigned int i;
	unsigned char *mem=(unsigned char *) lock_rect.pBits;

	for (i=0; i<sd.Height; i++)
	{
		memset(mem,0,size*sd.Width);
		mem+=lock_rect.Pitch;
	}
	
	DX8_ErrorCode(D3DSurface->UnlockRect());
}


/***********************************************************************************************
 * SurfaceClass::Copy -- Copies from a byte array to the surface                               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Copy(const unsigned char *other)
{
	SurfaceDescription sd;
	Get_Description(sd);

	// size of each pixel in bytes
	unsigned int size=PixelSize(sd);

	D3DLOCKED_RECT lock_rect;	
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,0,0));
	unsigned int i;
	unsigned char *mem=(unsigned char *) lock_rect.pBits;

	for (i=0; i<sd.Height; i++)
	{
		memcpy(mem,&other[i*sd.Width*size],size*sd.Width);		
		mem+=lock_rect.Pitch;
	}
	
	DX8_ErrorCode(D3DSurface->UnlockRect());
}


/***********************************************************************************************
 * SurfaceClass::Copy -- Copies a block of system ram to the surface                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/2/2001   hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Copy(Vector2i &min,Vector2i &max, const unsigned char *other)
{
	SurfaceDescription sd;
	Get_Description(sd);

	// size of each pixel in bytes
	unsigned int size=PixelSize(sd);

	D3DLOCKED_RECT lock_rect;	
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	RECT rect;
	rect.left=min.I;
	rect.right=max.I;
	rect.top=min.J;
	rect.bottom=max.J;
	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,&rect,0));
	int i;
	unsigned char *mem=(unsigned char *) lock_rect.pBits;	
	int dx=max.I-min.I;

	for (i=min.J; i<max.J; i++)
	{
		memcpy(mem,&other[(i*sd.Width+min.I)*size],size*dx);		
		mem+=lock_rect.Pitch;
	}
	
	DX8_ErrorCode(D3DSurface->UnlockRect());
}


/***********************************************************************************************
 * SurfaceClass::CreateCopy -- Creates a byte array copy of the surface                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/16/2001  hy : Created.                                                                  *
 *=============================================================================================*/
unsigned char *SurfaceClass::CreateCopy(int *width,int *height,int*size,bool flip)
{
	SurfaceDescription sd;
	Get_Description(sd);

	// size of each pixel in bytes
	unsigned int mysize=PixelSize(sd);

	*width=sd.Width;
	*height=sd.Height;
	*size=mysize;

	unsigned char *other=W3DNEWARRAY unsigned char [sd.Height*sd.Width*mysize];

	D3DLOCKED_RECT lock_rect;	
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,0,D3DLOCK_READONLY));
	unsigned int i;
	unsigned char *mem=(unsigned char *) lock_rect.pBits;

	for (i=0; i<sd.Height; i++)
	{
		if (flip)
		{
			memcpy(&other[(sd.Height-i-1)*sd.Width*mysize],mem,mysize*sd.Width);		
		} else
		{
			memcpy(&other[i*sd.Width*mysize],mem,mysize*sd.Width);		
		}
		mem+=lock_rect.Pitch;
	}
	
	DX8_ErrorCode(D3DSurface->UnlockRect());

	return other;
}


/***********************************************************************************************
 * SurfaceClass::Copy -- Copies a region from one surface to another                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Copy(
	unsigned int dstx, unsigned int dsty,
	unsigned int srcx, unsigned int srcy, 
	unsigned int width, unsigned int height,
	const SurfaceClass *other)
{
	WWASSERT(other);
	WWASSERT(width);
	WWASSERT(height);

	SurfaceDescription sd,osd;
	Get_Description(sd);
	const_cast <SurfaceClass*>(other)->Get_Description(osd);

	RECT src;
	src.left=srcx;
	src.right=srcx+width;
	src.top=srcy;
	src.bottom=srcy+height;

	if (src.right>int(osd.Width)) src.right=int(osd.Width);
	if (src.bottom>int(osd.Height)) src.bottom=int(osd.Height);	

	if (sd.Format==osd.Format && sd.Width==osd.Width && sd.Height==osd.Height)
	{
		POINT dst;
		dst.x=dstx;
		dst.y=dsty;	
		DX8Wrapper::_Copy_DX8_Rects(other->D3DSurface,&src,1,D3DSurface,&dst);
	}
	else
	{
		RECT dest;
		dest.left=dstx;
		dest.right=dstx+width;
		dest.top=dsty;
		dest.bottom=dsty+height;

		if (dest.right>int(sd.Width)) dest.right=int(sd.Width);
		if (dest.bottom>int(sd.Height)) dest.bottom=int(sd.Height);

		DX8_ErrorCode(D3DXLoadSurfaceFromSurface(D3DSurface,NULL,&dest,other->D3DSurface,NULL,&src,D3DX_FILTER_NONE,0));
	}
}

/***********************************************************************************************
 * SurfaceClass::Copy -- Copies a region from one surface to another                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Stretch_Copy(
	unsigned int dstx, unsigned int dsty, unsigned int dstwidth, unsigned int dstheight,
	unsigned int srcx, unsigned int srcy, unsigned int srcwidth, unsigned int srcheight,
	const SurfaceClass *other)
{
	WWASSERT(other);

	SurfaceDescription sd,osd;
	Get_Description(sd);
	const_cast <SurfaceClass*>(other)->Get_Description(osd);

	RECT src;
	src.left=srcx;
	src.right=srcx+srcwidth;
	src.top=srcy;	
	src.bottom=srcy+srcheight;

	RECT dest;
	dest.left=dstx;
	dest.right=dstx+dstwidth;
	dest.top=dsty;
	dest.bottom=dsty+dstheight;

	DX8_ErrorCode(D3DXLoadSurfaceFromSurface(D3DSurface,NULL,&dest,other->D3DSurface,NULL,&src,D3DX_FILTER_TRIANGLE ,0));
}

/***********************************************************************************************
 * SurfaceClass::FindBB -- Finds the bounding box of non zero pixels in the region             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::FindBB(Vector2i *min,Vector2i*max)
{
	SurfaceDescription sd;
	Get_Description(sd);

	WWASSERT(Has_Alpha(sd.Format));

	int alphabits=Alpha_Bits(sd.Format);
	int mask=0;
	switch (alphabits)
	{
	case 1: mask=1;
		break;
	case 4: mask=0xf;
		break;
	case 8: mask=0xff;
		break;
	}

	D3DLOCKED_RECT lock_rect;
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	RECT rect;
	::ZeroMemory(&rect, sizeof(RECT));

	rect.bottom=max->J;
	rect.top=min->J;
	rect.left=min->I;
	rect.right=max->I;

	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,&rect,D3DLOCK_READONLY));

	int x,y;
	unsigned int size=PixelSize(sd);
	Vector2i realmin=*max;
	Vector2i realmax=*min;	
	
	// the assumption here is that whenever a pixel has alpha it's in the MSB
	for (y = min->J; y < max->J; y++) {
		for (x = min->I; x < max->I; x++) {

			// HY - this is not endian safe
			unsigned char *alpha=(unsigned char*) ((unsigned int)lock_rect.pBits+(y-min->J)*lock_rect.Pitch+(x-min->I)*size);
			unsigned char myalpha=alpha[size-1];
			myalpha=(myalpha>>(8-alphabits)) & mask;
			if (myalpha) {
				realmin.I = MIN(realmin.I, x);
				realmax.I = MAX(realmax.I, x);
				realmin.J = MIN(realmin.J, y);
				realmax.J = MAX(realmax.J, y);
			}
		}
	}

	DX8_ErrorCode(D3DSurface->UnlockRect());

	*max=realmax;
	*min=realmin;
}


/***********************************************************************************************
 * SurfaceClass::Is_Transparent_Column -- Tests to see if the column is transparent or not     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
bool SurfaceClass::Is_Transparent_Column(unsigned int column)
{
	SurfaceDescription sd;
	Get_Description(sd);

	WWASSERT(column<sd.Width);
	WWASSERT(Has_Alpha(sd.Format));

	int alphabits=Alpha_Bits(sd.Format);
	int mask=0;
	switch (alphabits)
	{
	case 1: mask=1;
		break;
	case 4: mask=0xf;
		break;
	case 8: mask=0xff;
		break;
	}

	unsigned int size=PixelSize(sd);

	D3DLOCKED_RECT lock_rect;
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	RECT rect;
	::ZeroMemory(&rect, sizeof(RECT));

	rect.bottom=sd.Height;
	rect.top=0;
	rect.left=column;
	rect.right=column+1;

	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,&rect,D3DLOCK_READONLY));

	int y;	
	
	// the assumption here is that whenever a pixel has alpha it's in the MSB
	for (y = 0; y < (int) sd.Height; y++)
	{
		// HY - this is not endian safe
		unsigned char *alpha=(unsigned char*) ((unsigned int)lock_rect.pBits+y*lock_rect.Pitch);		
		unsigned char myalpha=alpha[size-1];		
		myalpha=(myalpha>>(8-alphabits)) & mask;		
		if (myalpha) {
			DX8_ErrorCode(D3DSurface->UnlockRect());
			return false;			
		}		
	}

	DX8_ErrorCode(D3DSurface->UnlockRect());
	return true;
}

/***********************************************************************************************
 * SurfaceClass::Get_Pixel -- Returns the pixel's RGB valus to the caller							  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Get_Pixel(Vector3 &rgb, int x,int y)
{
	SurfaceDescription sd;
	Get_Description(sd);

	x = min(x,(int)sd.Width - 1);
	y = min(y,(int)sd.Height - 1);

	D3DLOCKED_RECT lock_rect;
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	RECT rect;
	::ZeroMemory(&rect, sizeof(RECT));

	rect.bottom=y+1;
	rect.top=y;
	rect.left=x;
	rect.right=x+1;

	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,&rect,D3DLOCK_READONLY));	
	Convert_Pixel(rgb,sd,(unsigned char *) lock_rect.pBits);
	DX8_ErrorCode(D3DSurface->UnlockRect());	
}

/***********************************************************************************************
 * SurfaceClass::Attach -- Attaches a surface pointer to the object, releasing the current ptr.*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/27/2001  pds : Created.                                                                 *
 *=============================================================================================*/
void SurfaceClass::Attach (IDirect3DSurface8 *surface)
{
	Detach ();
	D3DSurface = surface;
	
	//
	//	Lock a reference onto the object
	//
	if (D3DSurface != NULL) {
		D3DSurface->AddRef ();
	}

	return ;
}


/***********************************************************************************************
 * SurfaceClass::Detach -- Releases the reference on the internal surface ptr, and NULLs it.	 .*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/27/2001  pds : Created.                                                                 *
 *=============================================================================================*/
void SurfaceClass::Detach (void)
{
	//
	//	Release the hold we have on the D3D object
	//
	if (D3DSurface != NULL) {
		D3DSurface->Release ();
	}

	D3DSurface = NULL;
	return ;
}


/***********************************************************************************************
 * SurfaceClass::DrawPixel -- draws a pixel                                                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void SurfaceClass::DrawPixel(const unsigned int x,const unsigned int y, unsigned int color)
{
	SurfaceDescription sd;
	Get_Description(sd);

	unsigned int size=PixelSize(sd);

	D3DLOCKED_RECT lock_rect;
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	RECT rect;
	::ZeroMemory(&rect, sizeof(RECT));

	rect.bottom=y+1;
	rect.top=y;
	rect.left=x;
	rect.right=x+1;

	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,&rect,0));
	unsigned char *cptr=(unsigned char*)lock_rect.pBits;
	unsigned short *sptr=(unsigned short*)lock_rect.pBits;
	unsigned int *lptr=(unsigned int*)lock_rect.pBits;

	switch (size)
	{
	case 1:
		*cptr=(unsigned char) (color & 0xFF);
		break;
	case 2:
		*sptr=(unsigned short) (color & 0xFFFF);
		break;
	case 4:
		*lptr=color;
		break;
	}

	DX8_ErrorCode(D3DSurface->UnlockRect());
}

/***********************************************************************************************
 * SurfaceClass::DrawHLine -- draws a horizontal line                                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/9/2001   hy : Created.                                                                  *
 *   4/9/2001   hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::DrawHLine(const unsigned int y,const unsigned int x1, const unsigned int x2, unsigned int color)
{ 
	SurfaceDescription sd;
	Get_Description(sd);

	unsigned int size=PixelSize(sd);

	D3DLOCKED_RECT lock_rect;
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	RECT rect;
	::ZeroMemory(&rect, sizeof(RECT));

	rect.bottom=y+1;
	rect.top=y;
	rect.left=x1;
	rect.right=x2+1;

	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,&rect,0));
	unsigned char *cptr=(unsigned char*)lock_rect.pBits;
	unsigned short *sptr=(unsigned short*)lock_rect.pBits;
	unsigned int *lptr=(unsigned int*)lock_rect.pBits;

	unsigned int x;
	// the assumption here is that whenever a pixel has alpha it's in the MSB
	for (x=x1; x<=x2; x++)
	{		
		switch (size)
		{
		case 1:
			*cptr++=(unsigned char) (color & 0xFF);
			break;
		case 2:
			*sptr++=(unsigned short) (color & 0xFFFF);
			break;
		case 4:
			*lptr++=color;
			break;
		}
	}

	DX8_ErrorCode(D3DSurface->UnlockRect());
}


/***********************************************************************************************
 * SurfaceClass::Is_Monochrome -- Checks if surface is monochrome or not                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2001   hy : Created.                                                                  *
 *=============================================================================================*/
bool SurfaceClass::Is_Monochrome(void)
{
	unsigned int x,y;
	SurfaceDescription sd;
	Get_Description(sd);
	bool is_compressed = false;

	switch (sd.Format)
	{
		// these formats are always monochrome
		case WW3D_FORMAT_A8L8:	
		case WW3D_FORMAT_A8:		
		case WW3D_FORMAT_L8:
		case WW3D_FORMAT_A4L4:
			return true;
		break;
		// these formats cannot be determined to be monochrome or not
		case WW3D_FORMAT_UNKNOWN:
		case WW3D_FORMAT_A8P8:
		case WW3D_FORMAT_P8:	
		case WW3D_FORMAT_U8V8:		// Bumpmap
		case WW3D_FORMAT_L6V5U5:	// Bumpmap
		case WW3D_FORMAT_X8L8V8U8:	// Bumpmap
			return false;
		break;
		// these formats need decompression first	
		case WW3D_FORMAT_DXT1:
		case WW3D_FORMAT_DXT2:
		case WW3D_FORMAT_DXT3:
		case WW3D_FORMAT_DXT4:
		case WW3D_FORMAT_DXT5:
			is_compressed = true;
		break;
	}

	// if it's in some compressed texture format, be sure to decompress first	
	if (is_compressed) {
		WW3DFormat new_format = Get_Valid_Texture_Format(sd.Format, false);
		SurfaceClass *new_surf = NEW_REF( SurfaceClass, (sd.Width, sd.Height, new_format) );
		new_surf->Copy(0, 0, 0, 0, sd.Width, sd.Height, this);
		bool result = new_surf->Is_Monochrome();
		REF_PTR_RELEASE(new_surf);
		return result;
	}

	int pitch,size;

	size=PixelSize(sd);
	unsigned char *bits=(unsigned char*) Lock(&pitch);

	Vector3 rgb;
	bool mono=true;

	for (y=0; y<sd.Height; y++)
	{
		for (x=0; x<sd.Width; x++)
		{
			Convert_Pixel(rgb,sd,&bits[x*size]);
			mono&=(rgb.X==rgb.Y);
			mono&=(rgb.X==rgb.Z);
			mono&=(rgb.Z==rgb.Y);
			if (!mono)
			{
				Unlock();
				return false;
			}
		}
		bits+=pitch;
	}

	Unlock();

	return true;
}

/***********************************************************************************************
 * SurfaceClass::Hue_Shift -- changes the hue of the surface                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001   hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Hue_Shift(const Vector3 &hsv_shift)
{
	unsigned int x,y;
	SurfaceDescription sd;
	Get_Description(sd);
	int pitch,size;

	size=PixelSize(sd);
	unsigned char *bits=(unsigned char*) Lock(&pitch);

	Vector3 rgb;

	for (y=0; y<sd.Height; y++)
	{
		for (x=0; x<sd.Width; x++)
		{
			Convert_Pixel(rgb,sd,&bits[x*size]);
			Recolor(rgb,hsv_shift);
			rgb.X=Bound(rgb.X,0.0f,1.0f);
			rgb.Y=Bound(rgb.Y,0.0f,1.0f);
			rgb.Z=Bound(rgb.Z,0.0f,1.0f);
			Convert_Pixel(&bits[x*size],sd,rgb);
		}
		bits+=pitch;
	}

	Unlock();
}
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

#include "bitmaphandler.h"
#include "wwdebug.h"
#include "colorspace.h"

void Bitmap_Assert(bool condition)
{
	WWASSERT(condition);
}

void BitmapHandlerClass::Create_Mipmap_B8G8R8A8(
	unsigned char* dest_surface,
	unsigned dest_surface_pitch,
	unsigned char* src_surface,
	unsigned src_surface_pitch,
	unsigned width,
	unsigned height)
{
	unsigned src_pitch=src_surface_pitch/4;
	for (unsigned y=0;y<height;y+=2) {
		unsigned* dest=(unsigned*)dest_surface;
		dest_surface+=dest_surface_pitch;
		unsigned* src=(unsigned*)src_surface;
		src_surface+=src_surface_pitch;
		for (unsigned x=0;x<width;x+=2) {
			unsigned bgra3=src[src_pitch];
			unsigned bgra1=*src++;
			unsigned bgra4=src[src_pitch];
			unsigned bgra2=*src++;
			*dest++=Combine_A8R8G8B8(bgra1,bgra2,bgra3,bgra4);
		}
	}
}

void BitmapHandlerClass::Copy_Image_Generate_Mipmap(
	unsigned width,
	unsigned height,
	unsigned char* dest_surface,
	unsigned dest_pitch,
	WW3DFormat dest_format,
	unsigned char* src_surface,
	unsigned src_pitch,
	WW3DFormat src_format,
	unsigned char* mip_surface,
	unsigned mip_pitch,
	const Vector3& hsv_shift)
{
	// Optimized loop if source and destination are 32 bit
	bool has_hsv_shift = hsv_shift[0]!=0.0f || hsv_shift[1]!=0.0f || hsv_shift[2]!=0.0f;
	if (src_format==dest_format && src_format==WW3D_FORMAT_A8R8G8B8) {
		dest_pitch/=4;
		src_pitch/=4;
		mip_pitch/=4;
		for (unsigned y=0;y<height/2;++y) {
			unsigned* dest_ptr=(unsigned*)dest_surface;
			dest_ptr+=2*y*dest_pitch;
			unsigned* src_ptr=(unsigned*)src_surface;
			src_ptr+=y*2*src_pitch;
			unsigned* mip_ptr=(unsigned*)mip_surface;
			mip_ptr+=y*mip_pitch;
			unsigned b8g8r8a8_00;
			unsigned b8g8r8a8_01;
			unsigned b8g8r8a8_10;
			unsigned b8g8r8a8_11;
			for (unsigned x=0;x<width/2;x++) {
				b8g8r8a8_10=src_ptr[src_pitch];
				dest_ptr[dest_pitch]=b8g8r8a8_10;
				b8g8r8a8_00=*src_ptr++;
				*dest_ptr++=b8g8r8a8_00;

				b8g8r8a8_11=src_ptr[src_pitch];
				dest_ptr[dest_pitch]=b8g8r8a8_11;
				b8g8r8a8_01=*src_ptr++;
				*dest_ptr++=b8g8r8a8_01;

				unsigned b8g8r8a8=Combine_A8R8G8B8(b8g8r8a8_00,b8g8r8a8_01,b8g8r8a8_10,b8g8r8a8_11);
				if (has_hsv_shift) {
					Recolor(b8g8r8a8,hsv_shift);
				}
				*mip_ptr++=b8g8r8a8;
			}
		}
		return;
	}

	WWASSERT(src_format!=WW3D_FORMAT_P8);		// This function doesn't support paletted formats
	unsigned src_bpp=Get_Bytes_Per_Pixel(src_format);
	unsigned dest_bpp=Get_Bytes_Per_Pixel(dest_format);

	for (unsigned y=0;y<height/2;++y) {
		unsigned char* dest_ptr=dest_surface+2*y*dest_pitch;
		unsigned char* src_ptr=src_surface+y*2*src_pitch;
		unsigned char* mip_ptr=mip_surface+y*mip_pitch;
		unsigned b8g8r8a8_00;
		unsigned b8g8r8a8_01;
		unsigned b8g8r8a8_10;
		unsigned b8g8r8a8_11;
		for (unsigned x=0;x<width/2;x++,dest_ptr+=dest_bpp*2,src_ptr+=src_bpp*2,mip_ptr+=dest_bpp) {
			Read_B8G8R8A8(b8g8r8a8_00,src_ptr,src_format,NULL,0);
			Write_B8G8R8A8(dest_ptr,dest_format,b8g8r8a8_00);

			Read_B8G8R8A8(b8g8r8a8_01,src_ptr+src_bpp,src_format,NULL,0);
			Write_B8G8R8A8(dest_ptr+dest_bpp,dest_format,b8g8r8a8_01);

			Read_B8G8R8A8(b8g8r8a8_10,src_ptr+src_pitch,src_format,NULL,0);
			Write_B8G8R8A8(dest_ptr+dest_pitch,dest_format,b8g8r8a8_10);

			Read_B8G8R8A8(b8g8r8a8_11,src_ptr+src_bpp+src_pitch,src_format,NULL,0);
			Write_B8G8R8A8(dest_ptr+dest_bpp+dest_pitch,dest_format,b8g8r8a8_11);

			unsigned b8g8r8a8=Combine_A8R8G8B8(b8g8r8a8_00,b8g8r8a8_01,b8g8r8a8_10,b8g8r8a8_11);
			if (has_hsv_shift) {
				Recolor(b8g8r8a8,hsv_shift);
			}

			Write_B8G8R8A8(mip_ptr,dest_format,b8g8r8a8);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Copy image from source surface to destination surface with stretch and color
// space conversion if needed. If 'generate_mip_level' is set, process image
// in 2x2 blocks and generate mipmap on top of the original source image while
// copying.
//
// ----------------------------------------------------------------------------

void BitmapHandlerClass::Copy_Image(
	unsigned char* dest_surface,
	unsigned dest_surface_width,
	unsigned dest_surface_height,
	unsigned dest_surface_pitch,
	WW3DFormat dest_surface_format,
	unsigned char* src_surface,
	unsigned src_surface_width,
	unsigned src_surface_height,
	unsigned src_surface_pitch,
	WW3DFormat src_surface_format,
	const unsigned char* src_palette,
	unsigned src_palette_bpp,
	bool generate_mip_level,
	const Vector3& hsv_shift)
{
	WWASSERT(dest_surface_width);
	WWASSERT(dest_surface_height);

	// Bumpmap?
	if (dest_surface_format==WW3D_FORMAT_U8V8 ||
		dest_surface_format==WW3D_FORMAT_L6V5U5 ||
		dest_surface_format==WW3D_FORMAT_X8L8V8U8) {

		unsigned src_bpp=Get_Bytes_Per_Pixel(src_surface_format);

		for( unsigned y=0; y<dest_surface_height; y++ ) {
			unsigned char* dest_ptr=dest_surface;
			dest_ptr+=y*dest_surface_pitch;
			unsigned char* src_ptr_mid=src_surface;
			src_ptr_mid+=y*src_surface_pitch;
			unsigned char* src_ptr_next_line = ( src_ptr_mid + src_surface_pitch );
			unsigned char* src_ptr_prev_line = ( src_ptr_mid - src_surface_pitch );

			if( y == src_surface_height-1 )  // Don't go past the last line
				src_ptr_next_line = src_ptr_mid;
			if( y == 0 )               // Don't go before first line
				src_ptr_prev_line = src_ptr_mid;

			for( unsigned x=0; x<dest_surface_width; x++ ) {
				unsigned pixel00;
				unsigned pixel01;
				unsigned pixelM1;
				unsigned pixel10;
				unsigned pixel1M;

				Read_B8G8R8A8(pixel00,src_ptr_mid,src_surface_format,NULL,0);
				Read_B8G8R8A8(pixel01,src_ptr_mid+src_bpp,src_surface_format,NULL,0);
				Read_B8G8R8A8(pixelM1,src_ptr_mid-src_bpp,src_surface_format,NULL,0);
				Read_B8G8R8A8(pixel10,src_ptr_prev_line,src_surface_format,NULL,0);
				Read_B8G8R8A8(pixel1M,src_ptr_next_line,src_surface_format,NULL,0);

				// Convert to luminance
				unsigned char bv00;
				unsigned char bv01;
				unsigned char bvM1;
				unsigned char bv10;
				unsigned char bv1M;
				Write_B8G8R8A8(&bv00,WW3D_FORMAT_L8,pixel00);
				Write_B8G8R8A8(&bv01,WW3D_FORMAT_L8,pixel01);
				Write_B8G8R8A8(&bvM1,WW3D_FORMAT_L8,pixelM1);
				Write_B8G8R8A8(&bv10,WW3D_FORMAT_L8,pixel10);
				Write_B8G8R8A8(&bv1M,WW3D_FORMAT_L8,pixel1M);
				int v00=bv00,v01=bv01,vM1=bvM1,v10=bv10,v1M=bv1M;

				int iDu = (vM1-v01); // The delta-u bump value
				int iDv = (v1M-v10); // The delta-v bump value

				if( (v00 < vM1) && (v00 < v01) ) {  // If we are at valley
					 iDu = vM1-v00;                 // Choose greater of 1st order diffs
					 if( iDu < v00-v01 )
						  iDu = v00-v01;
				}

				// The luminance bump value (land masses are less shiny)
				unsigned short uL = ( v00>1 ) ? 63 : 127;

				switch(dest_surface_format) {
				case WW3D_FORMAT_U8V8:
					*dest_ptr++ = (unsigned char)iDu;
					*dest_ptr++ = (unsigned char)iDv;
					break;

				case WW3D_FORMAT_L6V5U5:
					*(unsigned short*)dest_ptr  = (unsigned short)( ( (iDu>>3) & 0x1f ) <<  0 );
					*(unsigned short*)dest_ptr |= (unsigned short)( ( (iDv>>3) & 0x1f ) <<  5 );
					*(unsigned short*)dest_ptr |= (unsigned short)( ( ( uL>>2) & 0x3f ) << 10 );
					dest_ptr += 2;
					break;

				case WW3D_FORMAT_X8L8V8U8:
					*dest_ptr++ = (unsigned char)iDu;
					*dest_ptr++ = (unsigned char)iDv;
					*dest_ptr++ = (unsigned char)uL;
					*dest_ptr++ = (unsigned char)0L;
					break;

				default:
					WWASSERT(0);	// Unknown bumpmap format
					break;
				}

				// Move one pixel to the left (src is 32-bpp)
				src_ptr_mid+=src_bpp;
				src_ptr_prev_line+=src_bpp;
				src_ptr_next_line+=src_bpp;
			}
		}
		return;
	}

	bool has_hsv_shift = hsv_shift[0]!=0.0f || hsv_shift[1]!=0.0f || hsv_shift[2]!=0.0f;
	if (src_surface_format==dest_surface_format && (src_surface_format==WW3D_FORMAT_A8R8G8B8 || src_surface_format==WW3D_FORMAT_X8R8G8B8)) {
		// One-to-one copy or scaling?
		dest_surface_pitch/=4;
		src_surface_pitch/=4;
		if (dest_surface_width==src_surface_width && dest_surface_height==src_surface_height) {
			// Generate the next mip level while copying the current surface?
			if (generate_mip_level) {
				if (dest_surface_width==1) {
					unsigned b8g8r8a8=*(unsigned*)src_surface;
					if (has_hsv_shift) Recolor(b8g8r8a8,hsv_shift);
					*(unsigned*)dest_surface=b8g8r8a8;
				}
				else {
					for (unsigned y=0;y<dest_surface_height/2;++y) {
						unsigned* dest_ptr=(unsigned*)dest_surface;
						dest_ptr+=2*y*dest_surface_pitch;
						unsigned* src_ptr=(unsigned*)src_surface;
						unsigned* mip_ptr=src_ptr;
						src_ptr+=y*2*src_surface_pitch;
						mip_ptr+=y*src_surface_pitch;
						unsigned b8g8r8a8_00;
						unsigned b8g8r8a8_01;
						unsigned b8g8r8a8_10;
						unsigned b8g8r8a8_11;
						for (unsigned x=0;x<dest_surface_width/2;x++) {
							// Read four pixels from the source
							b8g8r8a8_10=src_ptr[src_surface_pitch];
							b8g8r8a8_00=*src_ptr++;
							b8g8r8a8_11=src_ptr[src_surface_pitch];
							b8g8r8a8_01=*src_ptr++;
							// Recolor if necessary
							if (has_hsv_shift) {
								Recolor(b8g8r8a8_00,hsv_shift);
								Recolor(b8g8r8a8_01,hsv_shift);
								Recolor(b8g8r8a8_10,hsv_shift);
								Recolor(b8g8r8a8_11,hsv_shift);
							}

							// Write the four pixels to the destination
							dest_ptr[dest_surface_pitch]=b8g8r8a8_10;
							*dest_ptr++=b8g8r8a8_00;
							dest_ptr[dest_surface_pitch]=b8g8r8a8_11;
							*dest_ptr++=b8g8r8a8_01;

							// Write combined four pixels to the destination mip map level
							*mip_ptr++=Combine_A8R8G8B8(b8g8r8a8_00,b8g8r8a8_01,b8g8r8a8_10,b8g8r8a8_11);
						}
					}
				}
			}
			else {
				for (unsigned y=0;y<dest_surface_height;++y) {
					unsigned* dest_ptr=(unsigned*)dest_surface;
					dest_ptr+=y*dest_surface_pitch;
					const unsigned* src_ptr=(unsigned*)src_surface;
					src_ptr+=y*src_surface_pitch;

					if (has_hsv_shift) {
						for (unsigned x=0;x<dest_surface_width;++x) {
							unsigned b8g8r8a8=*src_ptr++;
							Recolor(b8g8r8a8,hsv_shift);
							*dest_ptr++=b8g8r8a8;
						}
					}
					else {
						for (unsigned x=0;x<dest_surface_width;++x) {
							*dest_ptr++=*src_ptr++;
						}
					}
				}
			}
		}
		else {

			// For now do only point-sampling
			for (unsigned y=0;y<dest_surface_height;++y) {
				unsigned* dest_ptr=(unsigned*)dest_surface;
				dest_ptr+=y*dest_surface_pitch;
				unsigned src_y=y*src_surface_height/dest_surface_height;
				const unsigned* src_ptr=(unsigned*)src_surface;
				src_ptr+=src_y*src_surface_pitch;
				for (unsigned x=0;x<dest_surface_width;++x) {
					unsigned src_x=x*src_surface_width/dest_surface_width;
					unsigned b8g8r8a8=src_ptr[src_x];
					if (has_hsv_shift) {
						Recolor(b8g8r8a8,hsv_shift);
					}
					*dest_ptr++=b8g8r8a8;
				}
			}
		}
		return;
	}

	unsigned dest_bpp=Get_Bytes_Per_Pixel(dest_surface_format);
	unsigned src_bpp=Get_Bytes_Per_Pixel(src_surface_format);

	// One-to-one copy or scaling?
	if (dest_surface_width==src_surface_width && dest_surface_height==src_surface_height) {
		// Generate the next mip level while copying the current surface?
		if (generate_mip_level) {
			WWASSERT(src_surface_format!=WW3D_FORMAT_P8);	// Paletted textures can't be mipmapped
			if (dest_surface_width==1) {
				unsigned char* dest_ptr=dest_surface;
				unsigned char* src_ptr=src_surface;
				unsigned b8g8r8a8;
				Read_B8G8R8A8(b8g8r8a8,src_ptr,src_surface_format,src_palette,src_palette_bpp);
				if (has_hsv_shift) {
					Recolor(b8g8r8a8,hsv_shift);
				}
				Write_B8G8R8A8(dest_ptr,dest_surface_format,b8g8r8a8);
			}
			else {
				for (unsigned y=0;y<dest_surface_height/2;++y) {
					unsigned char* dest_ptr=dest_surface+2*y*dest_surface_pitch;
					unsigned char* src_ptr=src_surface+y*2*src_surface_pitch;
					unsigned char* mip_ptr=src_surface+y*src_surface_pitch;
					unsigned b8g8r8a8_00;
					unsigned b8g8r8a8_01;
					unsigned b8g8r8a8_10;
					unsigned b8g8r8a8_11;
					for (unsigned x=0;x<dest_surface_width/2;x++,dest_ptr+=dest_bpp*2,src_ptr+=src_bpp*2,mip_ptr+=src_bpp) {
						// Read four pixels from the source
						Read_B8G8R8A8(b8g8r8a8_00,src_ptr,src_surface_format,src_palette,src_palette_bpp);
						Read_B8G8R8A8(b8g8r8a8_01,src_ptr+src_bpp,src_surface_format,src_palette,src_palette_bpp);
						Read_B8G8R8A8(b8g8r8a8_10,src_ptr+src_surface_pitch,src_surface_format,src_palette,src_palette_bpp);
						Read_B8G8R8A8(b8g8r8a8_11,src_ptr+src_bpp+src_surface_pitch,src_surface_format,src_palette,src_palette_bpp);

						// Recolor if necessary
						if (has_hsv_shift) {
							Recolor(b8g8r8a8_00,hsv_shift);
							Recolor(b8g8r8a8_01,hsv_shift);
							Recolor(b8g8r8a8_10,hsv_shift);
							Recolor(b8g8r8a8_11,hsv_shift);
						}

						// Write the four pixels to the destination
						Write_B8G8R8A8(dest_ptr,dest_surface_format,b8g8r8a8_00);
						Write_B8G8R8A8(dest_ptr+dest_bpp,dest_surface_format,b8g8r8a8_01);
						Write_B8G8R8A8(dest_ptr+dest_surface_pitch,dest_surface_format,b8g8r8a8_10);
						Write_B8G8R8A8(dest_ptr+dest_bpp+dest_surface_pitch,dest_surface_format,b8g8r8a8_11);

						// Write combined four pixels to the destination mip map level
						unsigned b8g8r8a8=Combine_A8R8G8B8(b8g8r8a8_00,b8g8r8a8_01,b8g8r8a8_10,b8g8r8a8_11);
						Write_B8G8R8A8(mip_ptr,src_surface_format,b8g8r8a8);
					}
				}
			}
		}
		else {
			for (unsigned y=0;y<dest_surface_height;++y) {
				unsigned char* dest_ptr=dest_surface+y*dest_surface_pitch;
				const unsigned char* src_ptr=src_surface+y*src_surface_pitch;
				if (has_hsv_shift) {
					for (unsigned x=0;x<dest_surface_width;++x,dest_ptr+=dest_bpp,src_ptr+=src_bpp) {
						Copy_Pixel(dest_ptr,dest_surface_format,src_ptr,src_surface_format,src_palette,src_palette_bpp,hsv_shift);
					}
				}
				else {
					for (unsigned x=0;x<dest_surface_width;++x,dest_ptr+=dest_bpp,src_ptr+=src_bpp) {
						Copy_Pixel(dest_ptr,dest_surface_format,src_ptr,src_surface_format,src_palette,src_palette_bpp);
					}
				}
			}
		}
	}
	else {

		// For now do only point-sampling
		for (unsigned y=0;y<dest_surface_height;++y) {
			unsigned char* dest_ptr=dest_surface+y*dest_surface_pitch;
			unsigned src_y=y*src_surface_height/dest_surface_height;
			const unsigned char* src_ptr=src_surface+src_y*src_surface_pitch;
			if (has_hsv_shift) {
				for (unsigned x=0;x<dest_surface_width;++x,dest_ptr+=dest_bpp) {
					unsigned src_x=x*src_surface_width/dest_surface_width;
					src_x*=src_bpp;
					Copy_Pixel(dest_ptr,dest_surface_format,src_ptr+src_x,src_surface_format,src_palette,src_palette_bpp,hsv_shift);
				}
			}
			else {
				for (unsigned x=0;x<dest_surface_width;++x,dest_ptr+=dest_bpp) {
					unsigned src_x=x*src_surface_width/dest_surface_width;
					src_x*=src_bpp;
					Copy_Pixel(dest_ptr,dest_surface_format,src_ptr+src_x,src_surface_format,src_palette,src_palette_bpp);
				}
			}
		}
	}
}

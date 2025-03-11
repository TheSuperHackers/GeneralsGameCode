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

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Library/blitblit.h                                $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef BLITBLIT_H
#define BLITBLIT_H


/*
**	This module contains the pixel-pushing blitter objects. These objects only
**	serve one purpose. That is, to move pixels from one location to another. These
**	are prime candidates for optimization since they are called frequently and
**	loop greatly.
**
**	The large variety of blitter objects is necessary because there is a rich
**	set of pixel operations required by the game engine. Complicating this is that
**	the game engine must support both 16 bit and 8 bit pixel formats. Some of these
**	blitter objects are templates (this reduces the need for both 8 and 16 bit 
**	counterparts if the algorithm is constant between pixel formats). Also note
**	that there are some assembly implementations where it seems appropriate.
**
**	If the blitter object has "Xlat" in the name, then this means that the source
**	pixel is 8 bit and the destination pixel is 16 bit (probably). This hybrid system 
**	allows the game artwork to be shared between the two pixel format displays. To 
**	accomplish this, a translation table is supplied to the blit operation so that
**	the 8 bit pixel can be converted into the appropriate 16 bit destination pixel.
**	If the destination surface is also 8 bit, then the translation table converts
**	the pixel to the logical palette color index appropriate for the display.
*/


#include	"blitter.h"
#include	<assert.h>
#include	<string.h>


/*
** Blits without translation and source and dest are same pixel format. Note that
** this uses the memcpy and memmove routines. The C library has optimized these for
** maximum performance. This includes alignment issues and performing REP MOVSD
** instruction. This might be further optimized by using MMX instructions. However,
**	this blitter process is not often required by the game.
*/
template<class T>
class BlitPlain : public Blitter {
	public:
		virtual void BlitForward(void * dest, void const * source, int length) const {memcpy(dest, source, length*sizeof(T));}
		virtual void BlitBackward(void * dest, void const * source, int length) const {memmove(dest, source, length*sizeof(T));}
};


/*
** Blits with transparency checking when and source and dest are same pixel format.
**	This process is not often used.
*/
template<class T>
class BlitTrans : public Blitter {
	public:
		virtual void BlitForward(void * dest, void const * source, int len) const
		{
			for (int index = 0; index < len; index++) {
				T color = *(T const *)source;
				source = ((T *)source) + 1;
				if (color != 0) *((T *)dest) = color;
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length) const {BlitForward(dest, source, length);}
};


/*
** Blits when source 8 bits and dest is T. This process is typically used
**	when loading screen bitmaps or perform other non-transparent image blitting.
**	It is used fairly frequently and is a good candidate for optimization.
*/
template<class T>
class BlitPlainXlat : public Blitter {
	public:
		BlitPlainXlat(T const * translator) : TranslateTable(translator) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int len) const
		{
			for (int index = 0; index < len; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				*((T *)dest) = TranslateTable[color];
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length) const {BlitForward(dest, source, length);}

	private:
		T const * TranslateTable;
};


/*
** Blits with source 8 bit with transparency and dest is T. This process is used
**	frequently by trees and other terrain objects. It is a good candidate for
**	optimization.
*/
template<class T>
class BlitTransXlat : public Blitter {
	public:
		BlitTransXlat(T const * translator) : TranslateTable(translator) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int len) const
		{
			for (int index = 0; index < len; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				if (color != 0) {
					*((T *)dest) = TranslateTable[color];
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length) const {BlitForward(dest, source, length);}

	private:
		T const * TranslateTable;
};


/*
**	Blits with source 8 bit, transparency check, then translate to pixel format T. This
**	is occasionally used to render special remapping effects. Since the remap table is
**	not doubly indirected, it is fixed to only using the remap table specified in the
**	constructor. As such, it has limited value.
*/
template<class T>
class BlitTransRemapXlat : public Blitter {
	public:
		BlitTransRemapXlat(unsigned char const * remapper, T const * translator) : RemapTable(remapper), TranslateTable(translator) {assert(RemapTable != NULL);assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length) const
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				if (color != 0) {
					*((T *)dest) = TranslateTable[RemapTable[color]];
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length) const {BlitForward(dest, source, length);}

	private:
		unsigned char const * RemapTable;
		T const * TranslateTable;
};


/*
**	Blits with source 8 bit with transparency then remap and dest is T. This is probably
**	the most used blitter process. Units, infantry, buildings, and aircraft use this for
**	their normal drawing needs. If any blitter process is to be optimized, this would be
**	the one. Take note that the remapper table is doubly indirected. This allows a single
**	blitter object to dynamically use alternate remap tables.
*/
template<class T>
class BlitTransZRemapXlat : public Blitter {
	public:
		BlitTransZRemapXlat(unsigned char const * const * remapper, T const * translator) : RemapTable(remapper), TranslateTable(translator) {assert(RemapTable != NULL);assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length) const
		{
			unsigned char const * rtable = *RemapTable;
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				if (color != 0) {
					*((T *)dest) = TranslateTable[rtable[color]];
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length) const {BlitForward(dest, source, length);}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
};


/*
**	Algorithmic darkening of hicolor pixels controlled by the source pixels. The source
**	pixels are examined only to determine if the destination pixel should be darkened.
**	If the source pixel is transparent, then the dest pixel is skipped. The darkening
**	algorithm works only for hicolor pixels.
*/
template<class T>
class BlitTransDarken : public Blitter {
	public:
		BlitTransDarken(T mask) : Mask(mask) {}
		virtual void BlitForward(void * dest, void const * source, int length) const
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				if (color != 0) {
					*((T *)dest) = (T)((((*(T *)dest) >> 1) & Mask));
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length) const {BlitForward(dest, source, length);}

	private:
		T Mask;
};


/*
**	This will remap the destination pixels but under the control of the source pixels.
**	Where the source pixel is not transparent, the dest pixel is remapped. This algorithm
**	really only applies to lowcolor display.
*/
template<class T>
class BlitTransRemapDest : public Blitter {
	public:
		BlitTransRemapDest(T const * remap) : RemapTable(remap) {}
		virtual void BlitForward(void * dest, void const * source, int length) const
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				if (color != 0) {
					*((T *)dest) = RemapTable[*((T *)dest)];
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length) const {BlitForward(dest, source, length);}

	private:
		T const * RemapTable;
};


/*
**	This is similar to BlitTransDarken but instead of examining the source to determine what
**	pixels should be darkened, every destination pixel is darkened. This means that the source
**	pointer is unused.
*/
template<class T>
class BlitDarken : public Blitter {
	public:
		BlitDarken(T mask) : Mask(mask) {}
		virtual void BlitForward(void * dest, void const * , int length) const
		{
			for (int index = 0; index < length; index++) {
				*((T *)dest) = (T)(((*(T *)dest) >> 1) & Mask);
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length) const {BlitForward(dest, source, length);}

	private:
		T Mask;
};


/*
**	This blitter performs 50% translucency as it draws. It is commonly used for animation
**	effects and other stealth like images. It only works with hicolor pixels but is a good
**	candidate for optimization.
*/
template<class T>
class BlitTransLucent50 : public Blitter {
	public:
		BlitTransLucent50(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length) const
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char *)source) + 1;
				if (color != 0) {
					*((T *)dest) = (T)((((*(T *)dest) >> 1) & Mask) + ((TranslateTable[color] >> 1) & Mask));
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length) const {BlitForward(dest, source, length);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
**	This blitter performs 25% translucency as it draws. This effect is less than spectacular,
**	but there are some uses for it. It only works with hicolor pixels.
*/
template<class T>
class BlitTransLucent25 : public Blitter {
	public:
		BlitTransLucent25(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length) const
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char *)source) + 1;
				if (color != 0) {
					T qsource = (T)(((TranslateTable[color] >> 2) & Mask));
					T qdest = (T)((((*(T *)dest) >> 2) & Mask));
					*((T *)dest) = (T)(qdest + qsource + qsource + qsource);
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length) const {BlitForward(dest, source, length);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
**	This blitter performs 75% translucency as it draws. This is quite useful for explosions and
**	other gas animation effects. It only works with hicolor pixels and is a good candidate
**	for optimization.
*/
template<class T>
class BlitTransLucent75 : public Blitter {
	public:
		BlitTransLucent75(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length) const
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char *)source) + 1;
				if (color != 0) {
					T qsource = (T)(((TranslateTable[color] >> 2) & Mask));
					T qdest = (T)(((*(T *)dest) >> 2) & Mask);
					*((T *)dest) = (T)(qdest + qdest + qdest + qsource);
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length) const {BlitForward(dest, source, length);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
**	Assembly versions of some of the templated blitter object functions. Borland and 
**	Visual C++ support a compatible inline-assembly formats. However, Borland compiler
**	does not allow inline-assembly to be part of an inline function -- go figure. 
**	It will still compile, it just generates warning messages.
*/
#if defined(_MSC_VER)

inline void BlitTrans<unsigned char>::BlitForward(void * dest, void const * source, int len) const
{
	__asm {
		mov	esi,[source]
		mov	edi,[dest]
		mov	ecx,[len]
		dec	edi
		inc	ecx
	}
again:
	__asm {
		dec	ecx
		jz		fini
		mov	al,[esi]
		inc	edi
		inc	esi
		test	al,al
		jz		again
		mov	[edi],al
		jmp	again
	}
fini:;
}


inline void BlitTransXlat<unsigned short>::BlitForward(void * dest, void const * source, int len) const
{
	unsigned short const * xlator = TranslateTable;

	__asm {
		mov	ebx,[xlator]
		mov	ecx,[len]
		inc	ecx
		mov	edi,[dest]
		sub	edi,2
		mov	esi,[source]
		xor	eax,eax
	}
again:
	__asm {
		dec	ecx
		jz		over
		add	edi,2
		mov	al,[esi]
		inc	esi
		or		al,al
		jz		again
		mov	dx,[ebx+eax*2]
		mov	[edi],dx
		jmp	again
	}
over:;
}


inline void BlitTransRemapXlat<unsigned short>::BlitForward(void * dest, void const * source, int len) const
{
	unsigned short const * translator = TranslateTable;
	unsigned char const * remapper = RemapTable;

	__asm {
		mov	ecx,[len]
		mov	edi,[dest]
		sub	edi,2
		mov	esi,[source]
		mov	ebx,[remapper]
		mov	edx,[translator]
		xor	eax,eax
	}

	/*
	**	This block is 11 cycles per pixel, if not transparent, and 5
	**	cycles per pixel, if transparent.
	*/
again:
	__asm {
		dec	ecx
		jz		over
		add	edi,2
		xor	eax,eax
		lodsb
		or		al,al
		jz		again
		mov	al,[ebx+eax]				// First remap step (8 bit to 8 bit).
		mov	ax,[edx+eax*2]				// Second remap step (8 bit to 16 bit).
		mov	[edi],ax
		jmp	again
	}
over:;
}


inline void BlitTransZRemapXlat<unsigned short>::BlitForward(void * dest, void const * source, int len) const
{
	unsigned short const * translator = TranslateTable;
	unsigned char const * remapper = *RemapTable;

	__asm {
		mov	ecx,[len]
		mov	edi,[dest]
		sub	edi,2
		mov	esi,[source]
		mov	ebx,[remapper]
		mov	edx,[translator]
		xor	eax,eax
	}

	/*
	**	This block is 11 cycles per pixel, if not transparent, and 5
	**	cycles per pixel, if transparent.
	*/
again:
	__asm {
		dec	ecx
		jz		over
		add	edi,2
		xor	eax,eax
		lodsb
		or		al,al
		jz		again
		mov	al,[ebx+eax]				// First remap step (8 bit to 8 bit).
		mov	ax,[edx+eax*2]				// Second remap step (8 bit to 16 bit).
		mov	[edi],ax
		jmp	again
	}
over:;
}


inline void BlitPlainXlat<unsigned short>::BlitForward(void * dest, void const * source, int len) const
{
	unsigned short const * remapper = TranslateTable;
	__asm {
		mov	ebx,[remapper]
		mov	ecx,[len]
		mov	esi,[source]
		mov	edi,[dest]
		sub	edi,2
	}
again:
	/*
	**	This block processes pixels at 7 clocks per pixel.
	*/
	__asm {
		xor	eax,eax
		add	edi,2
		mov	al,[esi]
		inc	esi
		mov	ax,[ebx+eax*2]
		mov	[edi],ax
		dec	ecx
		jnz	again
	}
}


#endif


#endif

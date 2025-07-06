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
 *                 Project Name : wwmath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWMath/vp.cpp                                $*
 *                                                                                             *
 *                    Org Author:: Hector Yee                                                  *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*/

#include "vp.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "wwdebug.h"
#include "cpudetect.h"
#include <memory.h>

#define SHUFFLE(x, y, z, w)	(((x)&3)<< 6|((y)&3)<<4|((z)&3)<< 2|((w)&3))
#define	BROADCAST(XMM, INDEX)	__asm	shufps	XMM,XMM,(((INDEX)&3)<< 6|((INDEX)&3)<<4|((INDEX)&3)<< 2|((INDEX)&3))

#define TRANSPOSE(BX, BY, BZ, BW, TV)					\
		__asm	movaps		TV,BZ						\
		__asm	unpcklps	BZ,BW						\
		__asm	unpckhps	TV,BW						\
		__asm	movaps		BW,BX						\
		__asm	unpcklps	BX,BY						\
		__asm	unpckhps	BW,BY						\
		__asm	movaps		BY,BX						\
		__asm	shufps		BX,BZ,SHUFFLE(1, 0, 1, 0)	\
		__asm	shufps		BY,BZ,SHUFFLE(3, 2, 3, 2)	\
		__asm	movaps		BZ,BW						\
		__asm	shufps		BZ,TV,SHUFFLE(1, 0, 1, 0)	\
		__asm	shufps		BW,TV,SHUFFLE(3, 2, 3, 2)


void VectorProcessorClass::Prefetch(void* address)
{
#if defined (__ICL)    // Detect Intel compiler
	if (CPUDetectClass::_Has_SSE_Instruction_Set()) {
		__asm {
//			mov edx,address
//			mov eax,[edx]
//			prefetchT1 address
		}
	}
#endif
}

static Vector4 lastrow(0.0f,0.0f,0.0f,1.0f);
void VectorProcessorClass::Transform (Vector3* dst,const Vector3 *src, const Matrix3D& mtx, const int count)
{
	if (count<=0) return;

#if defined (__ICL)    // Detect Intel compiler
	if (CPUDetectClass::_Has_SSE_Instruction_Set()) {

		__asm	{
			mov		edx,dst
			mov		eax,src
			mov		ebx,mtx
			mov		edi,count

			movups	xmm4,[ebx+0]
			movups	xmm5,[ebx+16]
			movups	xmm6,[ebx+32]
			movups	xmm7,lastrow	//[ebx+48]

			TRANSPOSE(xmm4, xmm5, xmm6, xmm7, xmm0);

			shufps	xmm4,xmm4,SHUFFLE(2,1,0,0)
			shufps	xmm5,xmm5,SHUFFLE(2,1,0,0)
			shufps	xmm6,xmm6,SHUFFLE(2,1,0,0)
			shufps	xmm7,xmm7,SHUFFLE(2,1,0,0)

			mov		esi,edx
		_lp:
			test	edi,edi
			jz		_ulos
			test	esi,0xf
			jz		_aligned
			movss	xmm0,[eax]
			movss	xmm1,[eax+4]
			movss	xmm2,[eax+8]
			BROADCAST(xmm0,0)
			BROADCAST(xmm1,0)
			BROADCAST(xmm2,0)
			mulps	xmm0,xmm4
			mulps	xmm1,xmm5
			mulps	xmm2,xmm6
			addps	xmm0,xmm1
			addps	xmm0,xmm2
			addps	xmm0,xmm7
			movss	[edx],xmm0
			movhps	[edx+4],xmm0
			add		eax,12
			add		edx,12
			add		esi,12
			dec		edi
			jmp		_lp
		_aligned:

			mov		esi,1
		
			mov		ecx,edi
			and		edi,3
			and		ecx,~3
			jz		_lp
			
			lea		ecx,[ecx+ecx*2]
			shl		ecx,2
			add		eax,ecx
			add		edx,ecx
			neg		ecx

			cmp		dword ptr [ebx+12],0
			jne		_xlatelp
			cmp		dword ptr [ebx+28],0
			jne		_xlatelp
			cmp		dword ptr [ebx+44],0
			jne		_xlatelp
			jmp		_noxlatelp

			align	16

			_noxlatelp:
				prefetchnta	[eax+ecx+48]
				prefetchnta	[eax+ecx+48+32]

				movss	xmm0,[eax+ecx]
				BROADCAST(xmm0,0)
				movss	xmm1,[eax+ecx+4]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+8]
				BROADCAST(xmm2,0)
				mulps	xmm0,xmm4
				mulps	xmm1,xmm5
				mulps	xmm2,xmm6
				addps	xmm0,xmm1
				addps	xmm0,xmm2

				movss	xmm1,[eax+ecx+12]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+16]
				BROADCAST(xmm2,0)
				movss	xmm3,[eax+ecx+20]
				BROADCAST(xmm3,0)
				mulps	xmm1,xmm4
				mulps	xmm2,xmm5
				mulps	xmm3,xmm6
				addps	xmm1,xmm2
				addps	xmm3,xmm1

				movss	xmm0,xmm3
				shufps	xmm0,xmm0,SHUFFLE(0,3,2,1)

				movaps	[edx+ecx],xmm0

				prefetcht0	[edx+ecx+48]
				prefetcht0	[edx+ecx+48+32]

				movss	xmm0,[eax+ecx+24]
				BROADCAST(xmm0,0)
				movss	xmm1,[eax+ecx+24+4]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+24+8]
				BROADCAST(xmm2,0)
				mulps	xmm0,xmm4
				mulps	xmm1,xmm5
				mulps	xmm2,xmm6
				addps	xmm0,xmm1
				addps	xmm0,xmm2

				shufps	xmm3,xmm0,SHUFFLE(2,1,3,2)
				movaps	[edx+ecx+16],xmm3

				movss	xmm1,[eax+ecx+24+12]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+24+16]
				BROADCAST(xmm2,0)
				movss	xmm3,[eax+ecx+24+20]
				BROADCAST(xmm3,0)
				mulps	xmm1,xmm4
				mulps	xmm2,xmm5
				mulps	xmm3,xmm6
				addps	xmm1,xmm2
				addps	xmm1,xmm3

				shufps	xmm0,xmm0,SHUFFLE(2,1,0,3)
				movss	xmm1,xmm0
				movaps	[edx+ecx+32],xmm1

				add		ecx,48
				js		_noxlatelp

			jmp	_lp

			align	16

			_xlatelp:
				prefetchnta	[eax+ecx+48]
				prefetchnta	[eax+ecx+48+32]

				movss	xmm0,[eax+ecx]
				BROADCAST(xmm0,0)
				movss	xmm1,[eax+ecx+4]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+8]
				BROADCAST(xmm2,0)
				mulps	xmm0,xmm4
				mulps	xmm1,xmm5
				mulps	xmm2,xmm6
				addps	xmm0,xmm1
				addps	xmm0,xmm2
				addps	xmm0,xmm7

				movss	xmm1,[eax+ecx+12]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+16]
				BROADCAST(xmm2,0)
				movss	xmm3,[eax+ecx+20]
				BROADCAST(xmm3,0)
				mulps	xmm1,xmm4
				mulps	xmm2,xmm5
				mulps	xmm3,xmm6
				addps	xmm1,xmm2
				addps	xmm3,xmm1
				addps	xmm3,xmm7

				movss	xmm0,xmm3
				shufps	xmm0,xmm0,SHUFFLE(0,3,2,1)
				movaps	[edx+ecx],xmm0

				prefetcht0	[edx+ecx+48]
				prefetcht0	[edx+ecx+48+32]

				movss	xmm0,[eax+ecx+24]
				BROADCAST(xmm0,0)
				movss	xmm1,[eax+ecx+24+4]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+24+8]
				BROADCAST(xmm2,0)
				mulps	xmm0,xmm4
				mulps	xmm1,xmm5
				mulps	xmm2,xmm6
				addps	xmm0,xmm1
				addps	xmm0,xmm2
				addps	xmm0,xmm7

				shufps	xmm3,xmm0,SHUFFLE(2,1,3,2)
				movaps	[edx+ecx+16],xmm3

				movss	xmm1,[eax+ecx+24+12]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+24+16]
				BROADCAST(xmm2,0)
				movss	xmm3,[eax+ecx+24+20]
				BROADCAST(xmm3,0)
				mulps	xmm1,xmm4
				mulps	xmm2,xmm5
				mulps	xmm3,xmm6
				addps	xmm1,xmm2
				addps	xmm1,xmm3
				addps	xmm1,xmm7

				shufps	xmm0,xmm0,SHUFFLE(2,1,0,3)
				movss	xmm1,xmm0

				movaps	[edx+ecx+32],xmm1
				
				add		ecx,48
				js		_xlatelp

			jmp	_lp
		_ulos:
		}

	}
	else
#endif
	{
		mtx.mulVector3Array(src, dst, count);
	}
}

void VectorProcessorClass::Transform(Vector4* dst,const Vector3 *src, const Matrix4x4& matrix, const int count)
{
	if (count<=0) return;

	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=matrix*src[i];
	}
}

void VectorProcessorClass::Copy(Vector2 *dst, const Vector2 *src, int count)
{
	if (count<=0) return;
	memcpy(dst,src,sizeof(Vector2)*count);
}

void VectorProcessorClass::Copy(unsigned *dst, const unsigned *src, int count)
{
	if (count<=0) return;
	memcpy(dst,src,sizeof(unsigned)*count);
}

void VectorProcessorClass::Copy(Vector3 *dst, const Vector3 *src, int count)
{
	if (count<=0) return;
	memcpy(dst,src,sizeof(Vector3)*count);
}

void VectorProcessorClass::Copy(Vector4 *dst, const Vector4 *src, int count)
{
	if (count<=0) return;
	memcpy(dst,src,sizeof(Vector4)*count);
}

void VectorProcessorClass::Copy(Vector4 *dst,const Vector3 *src, const float * srca, const int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i].X=src[i].X;
		dst[i].Y=src[i].Y;
		dst[i].Z=src[i].Z;
		dst[i].W=srca[i];
	}
}

void VectorProcessorClass::Copy(Vector4 *dst,const Vector3 *src, const float srca, const int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i].X=src[i].X;
		dst[i].Y=src[i].Y;
		dst[i].Z=src[i].Z;
		dst[i].W=srca;
	}
}

void VectorProcessorClass::Copy(Vector4 *dst,const Vector3 &src, const float * srca, const int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i].X=src.X;
		dst[i].Y=src.Y;
		dst[i].Z=src.Z;
		dst[i].W=srca[i];
	}
}

void VectorProcessorClass::CopyIndexed (unsigned *dst,const unsigned *src, const unsigned int *index, int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=src[index[i]];
	}
}

void VectorProcessorClass::CopyIndexed (Vector2 *dst,const Vector2 *src, const unsigned int *index, int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=src[index[i]];
	}
}

void VectorProcessorClass::CopyIndexed (Vector3 *dst,const Vector3 *src, const unsigned int *index, int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=src[index[i]];
	}
}

void VectorProcessorClass::CopyIndexed (Vector4 *dst,const Vector4 *src, const unsigned int *index, int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=src[index[i]];
	}
}

void VectorProcessorClass::CopyIndexed(unsigned char* dst, const unsigned char* src, const unsigned int *index, int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=src[index[i]];
	}
}

void VectorProcessorClass::CopyIndexed(float* dst, float* src, const unsigned int *index, int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=src[index[i]];
	}
}

void VectorProcessorClass::Clamp(Vector4 *dst,const Vector4 *src, const float min, const float max, const int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i].X=(src[i].X<min)?min:src[i].X;
		dst[i].X=(src[i].X>max)?max:src[i].X;

		dst[i].Y=(src[i].Y<min)?min:src[i].Y;
		dst[i].Y=(src[i].Y>max)?max:src[i].Y;

		dst[i].Z=(src[i].Z<min)?min:src[i].Z;
		dst[i].Z=(src[i].Z>max)?max:src[i].Z;

		dst[i].W=(src[i].W<min)?min:src[i].W;
		dst[i].W=(src[i].W>max)?max:src[i].W;
	}
}

void VectorProcessorClass::Clear(Vector3*dst, const int count)
{
	if (count<=0) return;
	memset(dst,0,sizeof(Vector3)*count);
}


void VectorProcessorClass::Normalize(Vector3 *dst, const int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
		dst[i].Normalize();
}

void VectorProcessorClass::MinMax(Vector3 *src, Vector3 &min, Vector3 &max, const int count)
{
	if (count<=0) return;
	min=*src;
	max=*src;

	int i;

	for (i=1; i<count; i++)
	{
		min.X=MIN(min.X,src[i].X);
		min.Y=MIN(min.Y,src[i].Y);
		min.Z=MIN(min.Z,src[i].Z);

		max.X=MAX(max.X,src[i].X);
		max.Y=MAX(max.Y,src[i].Y);
		max.Z=MAX(max.Z,src[i].Z);
	}
}

void VectorProcessorClass::MulAdd(float * dest,float multiplier,float add,int count)
{
	for (int i=0; i<count; i++) {
		dest[i] = dest[i] * multiplier + add;
	}
}

void VectorProcessorClass::DotProduct(float *dst, const Vector3 &a, const Vector3 *b,const int count)
{
	for (int i=0; i<count; i++)
		dst[i]=Vector3::Dot_Product(a,b[i]);
}

void VectorProcessorClass::ClampMin(float *dst, float *src, const float min, const int count)
{
	for (int i=0; i<count; i++)
		dst[i]=(src[i]>min?src[i]:min);
}

void VectorProcessorClass::Power(float *dst, float *src, const float pow, const int count)
{
	for (int i=0; i<count; i++)
		dst[i]=powf(src[i],pow);
}
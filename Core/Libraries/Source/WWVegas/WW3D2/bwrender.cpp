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
 *                 Project Name : ww3d2                                                        *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/bwrender.cpp                           $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/04/01 10:14a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "bwrender.h"
#include "vp.h"


BWRenderClass::Buffer::Buffer(unsigned char* buffer_, int scale_)
	:
	buffer(buffer_),
	scale(scale_),
	minv(3),
	maxv(scale_-3)
{
}

BWRenderClass::Buffer::~Buffer()
{
}

void BWRenderClass::Buffer::Set_H_Line(int start_x, int end_x, int y)
{
	if (y<minv || y>=maxv || end_x<minv || start_x>=maxv) return;
	if (start_x<minv) start_x=minv;
	if (end_x>=maxv) end_x=maxv-1;
	unsigned char* ptr=buffer+scale*y+start_x;
	int w=end_x-start_x;
	if (w) {
		::memset(ptr,0x00,w);

/*		// Blurring (test)
		*(ptr-1)&=0x80;
		*(ptr-2)&=0xc0;
		*(ptr+w)&=0x80;
		*(ptr+w+1)&=0xc0;
		for (int a=0;a<w;++a) {
			*(ptr-scale+a)&=0xc0;
			*(ptr+scale+a)&=0xc0;
		}
*/
	}
}

void BWRenderClass::Buffer::Fill(unsigned char c)
{
	memset(buffer,c,scale*scale);
}

// ------------------------------------------------------------------------------

BWRenderClass::BWRenderClass(unsigned char* buffer, int buffer_scale)
	:
	pixel_buffer(buffer,buffer_scale)
{
}

BWRenderClass::~BWRenderClass()
{
}

void BWRenderClass::Fill(unsigned char c)
{
	pixel_buffer.Fill(c);
}

// Sets the vertex coordinate buffer location and scales the vertex locations to the kjkj
void BWRenderClass::Set_Vertex_Locations(Vector2* vertices_,int count)
{
	vertices=vertices_;

	float half_scale=pixel_buffer.Scale()*0.5f;
	VectorProcessorClass::MulAdd(
		reinterpret_cast<float*>(vertices),
		half_scale,
		half_scale,
		count*2);
}

// --------------------------------------------------------------------

static inline bool Cull(
	const Vector2& c1,
	const Vector2& c2,
	const Vector2& c3)
{
	float x1=c2[0]-c1[0];
	float y1=c2[1]-c1[1];
	float x2=c3[0]-c1[0];
	float y2=c3[1]-c1[1];
	float r=x1*y2-x2*y1;
	if (r<0) return false;
	return true;
}

void BWRenderClass::Render_Triangle_Strip(const unsigned long* indices,int index_count)
{
	index_count-=2;
	bool b=false;
	for (int n=0;n<index_count;++n) {
		b=!b;
		int idx_1=indices[0];
		int idx_2=indices[1];
		int idx_3=indices[2];
		indices++;

		if (Cull(vertices[idx_1],vertices[idx_2],vertices[idx_3])==b) continue;

		Vector2 corner_1(vertices[idx_1][0],vertices[idx_1][1]);
		Vector2 corner_2(vertices[idx_2][0],vertices[idx_2][1]);
		Vector2 corner_3(vertices[idx_3][0],vertices[idx_3][1]);

		// Sort the corners on y axis

		if (corner_2[1]<corner_1[1]) Swap(corner_1,corner_2);
		if (corner_3[1]<corner_1[1]) Swap(corner_1,corner_3);
		if (corner_3[1]<corner_2[1]) Swap(corner_2,corner_3);

		Vector3i yci(WWMath::Float_To_Long(corner_1[1]),WWMath::Float_To_Long(corner_2[1]),WWMath::Float_To_Long(corner_3[1]));
		Vector3 xcf(corner_1[0],corner_2[0],corner_3[0]);

		Render_Preprocessed_Triangle(xcf,yci);
	}
}

void BWRenderClass::Render_Triangles(const unsigned long* indices,int index_count)
{
	index_count/=3;
	for (int n=0;n<index_count;++n) {
		int idx_1=*indices++;
		int idx_2=*indices++;
		int idx_3=*indices++;

		if (Cull(vertices[idx_1],vertices[idx_2],vertices[idx_3])) continue;

		Vector2 corner_1(vertices[idx_1][0],vertices[idx_1][1]);
		Vector2 corner_2(vertices[idx_2][0],vertices[idx_2][1]);
		Vector2 corner_3(vertices[idx_3][0],vertices[idx_3][1]);

		// Sort the corners on y axis

		if (corner_2[1]<corner_1[1]) Swap(corner_1,corner_2);
		if (corner_3[1]<corner_1[1]) Swap(corner_1,corner_3);
		if (corner_3[1]<corner_2[1]) Swap(corner_2,corner_3);

		Vector3i yci(WWMath::Float_To_Long(corner_1[1]),WWMath::Float_To_Long(corner_2[1]),WWMath::Float_To_Long(corner_3[1]));
		Vector3 xcf(corner_1[0],corner_2[0],corner_3[0]);

		Render_Preprocessed_Triangle(xcf,yci);
	}
}

void BWRenderClass::Render_Preprocessed_Triangle(Vector3& xcf,Vector3i& yci)
{
	float x_left=xcf[0];
	float x_right=x_left;
	int ycnt=yci[1]-yci[0];
	int y=yci[0];
	if (ycnt) {
		float x_step_1=(xcf[1]-xcf[0])/float(ycnt);
		float x_step_2=(xcf[2]-xcf[0])/float(yci[2]-y);
		if (x_step_1>x_step_2) {
			float t=x_step_1;
			x_step_1=x_step_2;
			x_step_2=t;
		}

		while (ycnt>0) {
			pixel_buffer.Set_H_Line(WWMath::Float_To_Long(x_left),WWMath::Float_To_Long(x_right),y);

			x_left+=x_step_1;
			x_right+=x_step_2;
			ycnt--;
			y++;
		}
	}
	else {
		if (xcf[0]<xcf[1]) {
			x_left=xcf[0];
			x_right=xcf[1];
		}
		else {
			x_right=xcf[0];
			x_left=xcf[1];
		}

	}

	ycnt=yci[2]-yci[1];
	y=yci[1];

	if (ycnt) {
		float one_per_ycnt=1.0f/float(ycnt);
		float x_step_1=(xcf[2]-x_left)*one_per_ycnt;
		float x_step_2=(xcf[2]-x_right)*one_per_ycnt;
		while (ycnt>0) {
			pixel_buffer.Set_H_Line(WWMath::Float_To_Long(x_left),WWMath::Float_To_Long(x_right),y);
			x_left+=x_step_1;
			x_right+=x_step_2;
			ycnt--;
			y++;
		}
	}

}


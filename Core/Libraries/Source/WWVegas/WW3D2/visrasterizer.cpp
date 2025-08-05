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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/visrasterizer.cpp                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/24/01 5:42p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "visrasterizer.h"
#include "camera.h"
#include "plane.h"
#include "vp.h"


/**
** VisPolyClass - This class is used to clip polygons as they are
** sent through the vis rasterization system
*/
class VisPolyClass
{
public:
	void Reset(void);
	void Add_Vertex(const Vector3 & point);
	void Clip(const PlaneClass & plane,VisPolyClass & dest) const;

	SimpleDynVecClass<Vector3> Verts;
};


void VisPolyClass::Reset(void)
{
	Verts.Delete_All(false);
}

void VisPolyClass::Add_Vertex(const Vector3 & point)
{
	Verts.Add(point);
}

void VisPolyClass::Clip(const PlaneClass & plane,VisPolyClass & dest) const
{
	dest.Reset();

	// temporary variables used in clipping
	int i = 0;
	int vcount = Verts.Count();
	int iprev = vcount - 1;
	bool cur_point_in_front;
	bool prev_point_in_front;
	float alpha;
	Vector3 int_point;

	if (vcount <= 2) return;

	// perform clipping
	prev_point_in_front = !plane.In_Front(Verts[iprev]);		// Note, plane normal is outward so we invert this test
	for (int j=0; j<vcount; j++) {

		cur_point_in_front = !plane.In_Front(Verts[i]);			// Note, plane nomral is out so we invert this test
		if (prev_point_in_front) {

			if (cur_point_in_front) {

				// Previous vertex was in front of plane and this vertex is in
				// front of the plane so we emit this vertex.
				dest.Add_Vertex(Verts[i]);

			} else {

				// Previous vert was in front, this vert is behind, compute
				// the intersection and emit the point.
				plane.Compute_Intersection(Verts[iprev],Verts[i],&alpha);
				Vector3::Lerp(Verts[iprev],Verts[i],alpha,&int_point);
				dest.Add_Vertex(int_point);

			}

		} else {

			if (cur_point_in_front) {

				// segment is going from the back halfspace to the front halfspace
				// compute the intersection and emit it, then continue
				// the edge into the front halfspace and emit the end point.
				plane.Compute_Intersection(Verts[iprev],Verts[i],&alpha);
				Vector3::Lerp(Verts[iprev],Verts[i],alpha,&int_point);
				dest.Add_Vertex(int_point);
				dest.Add_Vertex(Verts[i]);

			}
		}

		prev_point_in_front = cur_point_in_front;
		iprev = i;

		//i = (i+1)%(Verts.Count());
		i++;
		if (i>=vcount) {
			i = 0;
		}
	}
}

static VisPolyClass _VisPoly0;
static VisPolyClass _VisPoly1;



/*********************************************************************************************

  VisRasterizerClass Implementation

*********************************************************************************************/


VisRasterizerClass::VisRasterizerClass(void) :
	ModelTransform(1),
	Camera(NULL),
	MVTransform(1)
{
}

VisRasterizerClass::~VisRasterizerClass(void)
{
	REF_PTR_RELEASE(Camera);
}

void VisRasterizerClass::Set_Resolution(int width,int height)
{
	IDBuffer.Set_Resolution(width,height);
}

void VisRasterizerClass::Get_Resolution(int * set_width,int * set_height)
{
	IDBuffer.Get_Resolution(set_width,set_height);
}

void	VisRasterizerClass::Set_Model_Transform(const Matrix3D & model)
{
	ModelTransform = model;
	Update_MV_Transform();
}

void	VisRasterizerClass::Set_Camera(CameraClass * camera)
{
	REF_PTR_SET(Camera,camera);
	Update_MV_Transform();
}

CameraClass * VisRasterizerClass::Get_Camera(void)
{
	if (Camera != NULL) {
		Camera->Add_Ref();
	}
	return Camera;
}

CameraClass * VisRasterizerClass::Peek_Camera(void)
{
	return Camera;
}

void VisRasterizerClass::Update_MV_Transform(void)
{
	Matrix3D view_tm(1);

	if (Camera) {
		Camera->Get_View_Matrix(&view_tm);
	}

#ifdef ALLOW_TEMPORARIES
	MVTransform = view_tm * ModelTransform;
#else
	MVTransform.mul(view_tm, ModelTransform);
#endif
}

const Matrix3D & VisRasterizerClass::Get_MV_Transform(void)
{
	// TODO: optimize this
	Update_MV_Transform();  // the user can and does mess with the camera directly!
	return MVTransform;
}

Vector3 * VisRasterizerClass::Get_Temp_Vertex_Buffer(int count)
{
	TempVertexBuffer.Uninitialised_Grow(count);
	return &(TempVertexBuffer[0]);
}


bool VisRasterizerClass::Render_Triangles
(
	const Vector3 * verts,
	int vcount,
	const TriIndex * tris,
	int tcount,const
	AABoxClass & bounds
)
{
	WWASSERT(verts != NULL);
	WWASSERT(tris != NULL);
	WWASSERT(vcount > 0);
	WWASSERT(tcount > 0);

	/*
	** if the user supplied bounds, check if we need to clip
	*/
	if (CollisionMath::Overlap_Test(Camera->Get_Frustum(),bounds) == CollisionMath::INSIDE) {
		return Render_Triangles_No_Clip(verts,vcount,tris,tcount);
	} else {
		return Render_Triangles_Clip(verts,vcount,tris,tcount);
	}
}


bool VisRasterizerClass::Render_Triangles_No_Clip
(
	const Vector3 * verts,
	int vcount,
	const TriIndex * tris,
	int tcount
)
{
	bool pixel_passed = false;

	/*
	** 1. transform verts into homogeneous view space and project
	*/
	const Matrix3D & tm = Get_MV_Transform();
	Vector3 * tverts = Get_Temp_Vertex_Buffer(vcount);

	VectorProcessorClass::Transform(tverts,verts,tm,vcount);

	for (int i=0; i<vcount; i++) {
		Camera->Project_Camera_Space_Point(tverts[i],tverts[i]);
	}

	/*
	** 2. Pass triangles on to the ID buffer for scan conversion
	*/
	for (int tri_index=0; tri_index<tcount; tri_index++) {

		const TriIndex & tri = tris[tri_index];
		pixel_passed |= IDBuffer.Render_Triangle(tverts[tri.I],tverts[tri.J],tverts[tri.K]);
		if (pixel_passed && (IDBuffer.Get_Render_Mode() == IDBufferClass::NON_OCCLUDER_MODE)) {
			return true;
		}
	}
	return pixel_passed;
}


bool VisRasterizerClass::Render_Triangles_Clip
(
	const Vector3 * verts,
	int vcount,
	const TriIndex * tris,
	int tcount
)
{
	bool pixel_passed = false;

	/*
	** 1. transform triangles into homogeneous view space
	*/
	const Matrix3D & tm = Get_MV_Transform();
	Vector3 * tverts = Get_Temp_Vertex_Buffer(vcount);

	VectorProcessorClass::Transform(tverts,verts,tm,vcount);

	/*
	** 2. get the frustum clipping planes
	*/
	const PlaneClass *planes = Camera->Get_View_Space_Frustum_Planes();

	/*
	** 3. Clip triangles to the view volume and pass on to the ID buffer for scan conversion
	*/
	for (int tri_index=0; tri_index<tcount; tri_index++) {

		/*
		** Copy triangle data into the vis clipping structure
		*/
		_VisPoly0.Reset();
		_VisPoly0.Add_Vertex(tverts[tris[tri_index].I]);
		_VisPoly0.Add_Vertex(tverts[tris[tri_index].J]);
		_VisPoly0.Add_Vertex(tverts[tris[tri_index].K]);

		/*
		** Clip against the view frustum
		*/
		_VisPoly0.Clip(planes[0],_VisPoly1);
		_VisPoly1.Clip(planes[1],_VisPoly0);
		_VisPoly0.Clip(planes[2],_VisPoly1);
		_VisPoly1.Clip(planes[3],_VisPoly0);
		_VisPoly0.Clip(planes[4],_VisPoly1);
		_VisPoly1.Clip(planes[5],_VisPoly0);

		/*
		** Project the vertices
		*/
		int final_vcount = _VisPoly0.Verts.Count();

		if (final_vcount >= 3) {

			Vector3 * final_verts = &(_VisPoly0.Verts[0]);

			int i;
			for (i=0; i<final_vcount; i++) {
				Camera->Project_Camera_Space_Point(final_verts[i],final_verts[i]);
			}

			/*
			** Pass the resulting triangle fan to the IDBuffer
			*/
			for (i=1; i<final_vcount - 1; i++) {

				pixel_passed |= IDBuffer.Render_Triangle(final_verts[0],final_verts[i],final_verts[i+1]);
				if (pixel_passed && (IDBuffer.Get_Render_Mode() == IDBufferClass::NON_OCCLUDER_MODE)) {
					return true;
				}
			}
		}
	}
	return pixel_passed;
}





/*********************************************************************************************

  IDBufferClass Implementation

*********************************************************************************************/

IDBufferClass::IDBufferClass(void) :
	BackfaceID(0),
	FrontfaceID(0),
	CurID(0),
	RenderMode(OCCLUDER_MODE),
	TwoSidedRenderingEnabled(false),
	PixelCounter(0),
	ResWidth(0),
	ResHeight(0),
	IDBuffer(NULL),
	ZBuffer(NULL)
{
}


IDBufferClass::~IDBufferClass(void)
{
	Reset();
}

void IDBufferClass::Set_Resolution(int w,int h)
{
	if ((w == ResWidth) && (h == ResHeight)) {
		return;
	} else {
		ResWidth = w;
		ResHeight = h;
		Allocate_Buffers();
	}
}

void IDBufferClass::Get_Resolution(int * get_w,int * get_h)
{
	if (get_w != NULL) { *get_w = ResWidth; }
	if (get_h != NULL) { *get_h = ResHeight; }
}

void IDBufferClass::Reset(void)
{
	if (IDBuffer!=NULL) {
		delete[] IDBuffer;
		IDBuffer = NULL;
	}
	if (ZBuffer != NULL) {
		delete[] ZBuffer;
		ZBuffer = NULL;
	}
	PixelCounter = 0;
}

void IDBufferClass::Allocate_Buffers(void)
{
	Reset();

	int bufsize = ResWidth * ResHeight;

	if (bufsize > 0) {
		IDBuffer = W3DNEWARRAY uint32 [bufsize];
		ZBuffer = W3DNEWARRAY float [bufsize];
	}
}

void IDBufferClass::Clear(void)
{
	if ((ResWidth > 0) && (ResHeight > 0)) {
		int byte_count = ResWidth * ResWidth * sizeof(uint32);

		WWASSERT(IDBuffer != NULL);
		WWASSERT(ZBuffer != NULL);
		memset(IDBuffer,0,byte_count);
		memset(ZBuffer,0,byte_count);
	}
}


/**
** GradientsStruct
** Computes the gradients for a triangle.
*/
struct GradientsStruct
{
	GradientsStruct(const Vector3 * verts)
	{
		float oodx = 1 / ( ((verts[1].X - verts[2].X) * (verts[0].Y - verts[2].Y)) -
										((verts[0].X - verts[2].X) * (verts[1].Y - verts[2].Y)));
		float oody = -oodx;

		for(int i=0; i<3; i++) {
			OOZ[i] = 1/verts[i].Z;
		}

		DOOZ_DX = oodx * ( ((OOZ[1] - OOZ[2]) * (verts[0].Y - verts[2].Y)) -
								 ((OOZ[0] - OOZ[2]) * (verts[1].Y - verts[2].Y)));

		DOOZ_DY = oody * ( ((OOZ[1] - OOZ[2]) * (verts[0].X - verts[2].X)) -
								 ((OOZ[0] - OOZ[2]) * (verts[1].X - verts[2].X)));
	}

	float OOZ[3];			// 1/z for each vertex
	float DOOZ_DX;			// change in 1/z per change in x
	float DOOZ_DY;			// change in 1/z per change in y
};


/**
** EdgeStruct
** Can accurately scan convert an edge of a triangle.
*/
struct EdgeStruct
{
	EdgeStruct(const GradientsStruct & grad,const Vector3 * verts,int top,int bottom)
	{
		Y = WWMath::Ceil(verts[top].Y);
		Height = WWMath::Ceil(verts[bottom].Y) - Y;

		float y_prestep = Y - verts[top].Y;
		float real_height = verts[bottom].Y - verts[top].Y;
		float real_width  = verts[bottom].X - verts[top].X;

		X = ((real_width * y_prestep)/real_height) + verts[top].X;
		XStep = real_width / real_height;
		float x_prestep = X - verts[top].X;

		OOZ = grad.OOZ[top] + y_prestep * grad.DOOZ_DY + x_prestep * grad.DOOZ_DX;
		OOZStep = XStep * grad.DOOZ_DX + grad.DOOZ_DY;
	}

	inline int Step(void)
	{
		X+=XStep;
		Y++;
		Height--;
		OOZ+=OOZStep;
		return Height;
	}

	float		X;				// fractional x coord
	float		XStep;		// change in x per scanline
	int		Y;				// current y coord
	int		Height;		// number of scanlines left
	float		OOZ;			// current 1/z
	float		OOZStep;		// change in 1/z per scanline
};


bool IDBufferClass::Render_Triangle(const Vector3 & p0,const Vector3 & p1,const Vector3 & p2)
{
	if ((ZBuffer == NULL) || (IDBuffer == NULL)) {
		return false;
	}

	int pixels_passed = 0;
	bool is_backfacing = Is_Backfacing(p0,p1,p2);

	if ((is_backfacing) && (TwoSidedRenderingEnabled == false)) {
		if (RenderMode == NON_OCCLUDER_MODE) {
			return false;
		}
		CurID = BackfaceID;
	} else {
		CurID = FrontfaceID;
	}



	/*
	** Transform the coordinates to device coords
	** All coordinates come in with the range -1 -> +1
	*/
	Vector3 points[3];
	points[0].X = 0.5f * (p0.X + 1.0f) * ResWidth;
	points[1].X = 0.5f * (p1.X + 1.0f) * ResWidth;
	points[2].X = 0.5f * (p2.X + 1.0f) * ResWidth;

	points[0].Y = 0.5f * (1.0f - p0.Y) * ResHeight;
	points[1].Y = 0.5f * (1.0f - p1.Y) * ResHeight;
	points[2].Y = 0.5f * (1.0f - p2.Y) * ResHeight;

	points[0].Z = 0.5f * (p0.Z + 1.001f) * 1000.0f;
	points[1].Z = 0.5f * (p1.Z + 1.001f) * 1000.0f;
	points[2].Z = 0.5f * (p2.Z + 1.001f) * 1000.0f;

	/*
	** Sort points based on Y
	*/
	float y0 = points[0].Y;
	float y1 = points[1].Y;
	float y2 = points[2].Y;

	int top,middle,bottom,middle_for_compare,bottom_for_compare;

	if(y0 < y1) {
		if(y2 < y0) {
			top = 2; middle = 0; bottom = 1;
			middle_for_compare = 0; bottom_for_compare = 1;
		} else {
			top = 0;
			if(y1 < y2) {
				middle = 1; bottom = 2;
				middle_for_compare = 1; bottom_for_compare = 2;
			} else {
				middle = 2; bottom = 1;
				middle_for_compare = 2; bottom_for_compare = 1;
			}
		}
	} else {
		if(y2 < y1) {
			top = 2; middle = 1; bottom = 0;
			middle_for_compare = 1; bottom_for_compare = 0;
		} else {
			top = 1;
			if(y0 < y2) {
				middle = 0; bottom = 2;
				middle_for_compare = 3; bottom_for_compare = 2;
			} else {
				middle = 2; bottom = 0;
				middle_for_compare = 2; bottom_for_compare = 3;
			}
		}
	}

	/*
	** Compute the gradients and set up the edge structures
	*/
	GradientsStruct grads(points);
	EdgeStruct top_to_bottom_edge(grads,points,top,bottom);
	EdgeStruct top_to_middle_edge(grads,points,top,middle);
	EdgeStruct middle_to_bottom_edge(grads,points,middle,bottom);

	EdgeStruct * left_edge = NULL;
	EdgeStruct * right_edge = NULL;

	bool middle_is_left = false;
	if (bottom_for_compare > middle_for_compare) {
		middle_is_left = 1 ^ is_backfacing;
	} else {
		middle_is_left = 0 ^ is_backfacing;
	}
	if (middle_is_left) {
		left_edge = &top_to_middle_edge;
		right_edge = &top_to_bottom_edge;
	} else {
		left_edge = &top_to_bottom_edge;
		right_edge = &top_to_middle_edge;
	}

	/*
	** Fill scanlines
	*/
	int height = top_to_middle_edge.Height;

	while (height--) {
		if (RenderMode == OCCLUDER_MODE) {
			pixels_passed += Render_Occluder_Scanline(grads,left_edge,right_edge);
		} else {
			pixels_passed += Render_Non_Occluder_Scanline(grads,left_edge,right_edge);
		}
		left_edge->Step();
		right_edge->Step();
	}

	if (middle_is_left) {
		left_edge = &middle_to_bottom_edge;
		right_edge = &top_to_bottom_edge;
	} else {
		left_edge = &top_to_bottom_edge;
		right_edge = &middle_to_bottom_edge;
	}

	height = middle_to_bottom_edge.Height;

	while (height--) {
		if (RenderMode == OCCLUDER_MODE) {
			pixels_passed += Render_Occluder_Scanline(grads,left_edge,right_edge);
		} else {
			pixels_passed += Render_Non_Occluder_Scanline(grads,left_edge,right_edge);
		}
		left_edge->Step();
		right_edge->Step();
	}
	return (pixels_passed > 0);
}


int IDBufferClass::Render_Occluder_Scanline(GradientsStruct & grads,EdgeStruct * left,EdgeStruct * right)
{
	if ((left->Y < 1) || (left->Y >= ResHeight)) {
		return 0;
	}

	int xstart = WWMath::Float_To_Long(WWMath::Max(WWMath::Ceil(left->X),1.0f));
	int width = WWMath::Float_To_Long(WWMath::Ceil(right->X)) - xstart;
	if (xstart + width > ResWidth) {
		width = ResWidth - xstart;
	}

	float xprestep = (float)xstart - left->X;
	int address = Pixel_Coords_To_Address(xstart,left->Y);
	float ooz = left->OOZ + xprestep * grads.DOOZ_DX;
	int pixel_counter = 0;

	/*
	** Two separate loops, backfaces only render when LESS THAN
	*/
	if (CurID == BackfaceID) {
		while (width-- > 0) {
			if (ooz > ZBuffer[address]) {
				IDBuffer[address] = CurID;
				ZBuffer[address] = ooz;
				pixel_counter++;
			}
			ooz += grads.DOOZ_DX;
			address++;
		}
	/*
	** Front faces render when LESS THAN OR EQUAL TO
	*/
	} else {
		while (width-- > 0) {
			if (ooz >= ZBuffer[address]) {
				IDBuffer[address] = CurID;
				ZBuffer[address] = ooz;
				pixel_counter++;
			}
			ooz += grads.DOOZ_DX;
			address++;
		}
	}

	PixelCounter += pixel_counter;
	return pixel_counter;
}


int IDBufferClass::Render_Non_Occluder_Scanline(GradientsStruct & grads,EdgeStruct * left,EdgeStruct * right)
{
	if ((left->Y < 1) || (left->Y >= ResHeight)) {
		return 0;
	}

	int xstart = WWMath::Float_To_Long(WWMath::Max(WWMath::Ceil(left->X),1));
	int width = WWMath::Float_To_Long(WWMath::Ceil(right->X)) - xstart;
	if (xstart + width > ResWidth) {
		width = ResWidth - xstart;
	}

	float xprestep = (float)xstart - left->X;
	int address = Pixel_Coords_To_Address(xstart,left->Y);
	float ooz = left->OOZ + xprestep * grads.DOOZ_DX;

	while (width-- > 0) {
		if (ooz >= ZBuffer[address]) {
			PixelCounter++;
			return 1;
		}
		ooz += grads.DOOZ_DX;
		address++;
	}

	return 0;
}


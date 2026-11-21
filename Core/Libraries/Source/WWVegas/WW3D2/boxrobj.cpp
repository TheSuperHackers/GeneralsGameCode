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
 *                     $Archive:: /Commando/Code/ww3d2/boxrobj.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/19/02 12:57p                                              $*
 *                                                                                             *
 *                    $Revision:: 35                                                          $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "boxrobj.h"
#include "w3d_util.h"
#include "WWDebug/wwdebug.h"
#include "vertmaterial.h"
#include "ww3d.h"
#include "WWLib/chunkio.h"
#include "rinfo.h"
#include "coltest.h"
#include "inttest.h"
#include "dx8wrapper.h"
#include "Renderer.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8fvf.h"
#include "sortingrenderer.h"
#include "visrasterizer.h"
#include "meshgeometry.h"


#define NUM_BOX_VERTS	8
#define NUM_BOX_FACES	12

// Face Connectivity
static TriIndex					_BoxFaces[NUM_BOX_FACES] =
{
	TriIndex( 0,1,2 ),		// +z faces
	TriIndex( 0,2,3 ),
	TriIndex( 4,7,6 ),		// -z faces
	TriIndex( 4,6,5 ),
	TriIndex( 0,3,7 ),		// +x faces
	TriIndex( 0,7,4 ),
	TriIndex( 1,5,6 ),		// -x faces
	TriIndex( 1,6,2 ),
	TriIndex( 4,5,1 ),		// +y faces
	TriIndex( 4,1,0 ),
	TriIndex( 3,2,6 ),		// -y faces
	TriIndex( 3,6,7 )
};

// Vertex Positions as a function of the box extents
static Vector3						_BoxVerts[NUM_BOX_VERTS] =
{
	Vector3(  1.0f, 1.0f, 1.0f ),		// +z ring of 4 verts
	Vector3( -1.0f, 1.0f, 1.0f ),
	Vector3( -1.0f,-1.0f, 1.0f ),
	Vector3(  1.0f,-1.0f, 1.0f ),

	Vector3(  1.0f, 1.0f,-1.0f ),		// -z ring of 4 verts;
	Vector3( -1.0f, 1.0f,-1.0f ),
	Vector3( -1.0f,-1.0f,-1.0f ),
	Vector3(  1.0f,-1.0f,-1.0f ),
};

// Vertex Normals
static Vector3						_BoxVertexNormals[NUM_BOX_VERTS] =
{
	Vector3( WWMATH_OOSQRT3, WWMATH_OOSQRT3, WWMATH_OOSQRT3 ),
	Vector3(-WWMATH_OOSQRT3, WWMATH_OOSQRT3, WWMATH_OOSQRT3 ),
	Vector3(-WWMATH_OOSQRT3,-WWMATH_OOSQRT3, WWMATH_OOSQRT3 ),
	Vector3( WWMATH_OOSQRT3,-WWMATH_OOSQRT3, WWMATH_OOSQRT3 ),

	Vector3( WWMATH_OOSQRT3, WWMATH_OOSQRT3,-WWMATH_OOSQRT3 ),
	Vector3(-WWMATH_OOSQRT3, WWMATH_OOSQRT3,-WWMATH_OOSQRT3 ),
	Vector3(-WWMATH_OOSQRT3,-WWMATH_OOSQRT3,-WWMATH_OOSQRT3 ),
	Vector3( WWMATH_OOSQRT3,-WWMATH_OOSQRT3,-WWMATH_OOSQRT3 ),
};



bool										BoxRenderObjClass::IsInitted			= false;
int										BoxRenderObjClass::DisplayMask		= 0;
static VertexMaterialClass *		_BoxMaterial								= nullptr;
static ShaderClass					_BoxShader;


/*
** BoxRenderObjClass Implementation
*/


BoxRenderObjClass::BoxRenderObjClass()
{
	memset(Name,0,sizeof(Name));
	Color.Set(1,1,1);
	Opacity = 0.25f;
	ObjSpaceCenter.Set(0,0,0);
	ObjSpaceExtent.Set(1,1,1);
}


BoxRenderObjClass::BoxRenderObjClass(const W3dBoxStruct & def)
{
	Set_Name(def.Name);
	W3dUtilityClass::Convert_Color(def.Color,&Color);
	W3dUtilityClass::Convert_Vector(def.Center,&ObjSpaceCenter);
	W3dUtilityClass::Convert_Vector(def.Extent,&ObjSpaceExtent);
	int col_bits = (def.Attributes & W3D_BOX_ATTRIBUTE_COLLISION_TYPE_MASK) >> W3D_BOX_ATTRIBUTE_COLLISION_TYPE_SHIFT;
	Set_Collision_Type(col_bits<<1);
	Opacity = 0.25f;
}


BoxRenderObjClass::BoxRenderObjClass(const BoxRenderObjClass & src)
{
	*this = src;
}


BoxRenderObjClass & BoxRenderObjClass::operator = (const BoxRenderObjClass & that)
{
	if (this != &that) {
		RenderObjClass::operator = (that);
		Set_Name(that.Get_Name());
		Color.Set(that.Color);
		ObjSpaceCenter.Set(that.ObjSpaceCenter);
		ObjSpaceExtent.Set(that.ObjSpaceExtent);
	}
	return *this;
}


int BoxRenderObjClass::Get_Num_Polys() const
{
	return 12;
}


const char * BoxRenderObjClass::Get_Name() const
{
	return Name;
}


void BoxRenderObjClass::Set_Name(const char * name)
{
	WWASSERT(name != nullptr);
	size_t nameLen = strlcpy(Name, name, ARRAY_SIZE(Name));
	(void)nameLen; WWASSERT(nameLen < ARRAY_SIZE(Name));
}


void BoxRenderObjClass::Set_Color(const Vector3 & color)
{
	Color = color;
}


// Global initialization needed for boxes to work. Allocates materials which all boxes share.
// Initializes vertex tables, etc.
void BoxRenderObjClass::Init()
{
	WWASSERT(IsInitted == false);

	/*
	** Set up the materials
	*/
	WWASSERT(_BoxMaterial == nullptr);
	_BoxMaterial = NEW_REF(VertexMaterialClass,());
	_BoxMaterial->Set_Ambient(0,0,0);
	_BoxMaterial->Set_Diffuse(0,0,0);
	_BoxMaterial->Set_Specular(0,0,0);
	_BoxMaterial->Set_Emissive(1,1,1);
	_BoxMaterial->Set_Opacity(1.0f);		// uses vertex alpha...
	_BoxMaterial->Set_Shininess(0.0f);

	_BoxShader = ShaderClass::_PresetAlphaSolidShader; //_PresetAdditiveSolidShader;

	IsInitted = true;
}


// Releases resources allocated in Init.
// NOTE: This is a static function that should only be called by the system.
void BoxRenderObjClass::Shutdown()
{
	WWASSERT(IsInitted == true);
	REF_PTR_RELEASE(_BoxMaterial);

	IsInitted = false;
}


// Boxes are debug objects and usually used for collision. This mask is 'AND'ed with each box's
// collision type to determine whether the box should be rendered.
void BoxRenderObjClass::Set_Box_Display_Mask(int mask)
{
	DisplayMask = mask;
}


int BoxRenderObjClass::Get_Box_Display_Mask()
{
	return DisplayMask;
}


// Submits the box to the GERD.
void BoxRenderObjClass::render_box(RenderInfoClass & rinfo,const Vector3 & center,const Vector3 & extent)
{
	if (!IsInitted) return;
	if (DisplayMask & Get_Collision_Type()) {

		static Vector3 verts[NUM_BOX_VERTS];

		// compute the vertex positions
		for (int ivert=0; ivert<NUM_BOX_VERTS; ivert++) {
			verts[ivert].X = center.X + _BoxVerts[ivert][0] * extent.X;
			verts[ivert].Y = center.Y + _BoxVerts[ivert][1] * extent.Y;
			verts[ivert].Z = center.Z + _BoxVerts[ivert][2] * extent.Z;
		}

		/*
		** Dump the box vertices into the sorting dynamic vertex buffer.
		*/
		DWORD color = DX8Wrapper::Convert_Color(Color,Opacity);

		int buffer_type = BUFFER_TYPE_DYNAMIC_DX8;

		DynamicVBAccessClass vbaccess(buffer_type,dynamic_fvf_type,NUM_BOX_VERTS);
		{
			DynamicVBAccessClass::WriteLockClass lock(&vbaccess);
			//unsigned char *vb=(unsigned char *) lock.Get_Vertex_Array();
			VertexFormatXYZNDUV2* vb=lock.Get_Formatted_Vertex_Array();

			for (int i=0; i<NUM_BOX_VERTS; i++) {

				// Locations
				vb->x=verts[i][0];
				vb->y=verts[i][1];
				vb->z=verts[i][2];

				// Normals
				vb->nx=_BoxVertexNormals[i][0];
				vb->ny=_BoxVertexNormals[i][1];
				vb->nz=_BoxVertexNormals[i][2];

				// Colors
				vb->diffuse=color;

				vb++;
			}
		}

		/*
		** Dump the faces into the sorting dynamic index buffer.
		*/
		DynamicIBAccessClass ibaccess(buffer_type,NUM_BOX_FACES*3);
		{
			DynamicIBAccessClass::WriteLockClass lock(&ibaccess);
			unsigned short * indices = lock.Get_Index_Array();
			for (int i=0; i<NUM_BOX_FACES; i++) {
				indices[3*i] = _BoxFaces[i][0];
				indices[3*i+1] = _BoxFaces[i][1];
				indices[3*i+2] = _BoxFaces[i][2];
			}
		}

		/*
		** Apply the shader and material
		*/
		DX8Wrapper::Set_Material(_BoxMaterial);
		DX8Wrapper::Set_Shader(_BoxShader);
		DX8Wrapper::Set_Texture(0,nullptr);

		DX8Wrapper::Set_Index_Buffer(ibaccess,0);
		DX8Wrapper::Set_Vertex_Buffer(vbaccess);

		DX8Wrapper::Draw_Triangles(buffer_type,0,NUM_BOX_FACES,0,NUM_BOX_VERTS);
	}
}


// Submits box to the GERD for VIS. This renders the box with the specified VIS-ID.
void BoxRenderObjClass::vis_render_box(SpecialRenderInfoClass & rinfo,const Vector3 & center,const Vector3 & extent)
{
	if (!IsInitted) return;

	static Vector3 verts[NUM_BOX_VERTS];

	// compute the vertex positions
	for (int ivert=0; ivert<NUM_BOX_VERTS; ivert++) {
		verts[ivert].X = center.X + _BoxVerts[ivert][0] * extent.X;
		verts[ivert].Y = center.Y + _BoxVerts[ivert][1] * extent.Y;
		verts[ivert].Z = center.Z + _BoxVerts[ivert][2] * extent.Z;
	}

	// render!
	rinfo.VisRasterizer->Render_Triangles(verts,NUM_BOX_VERTS,_BoxFaces,NUM_BOX_FACES,Get_Bounding_Box());
}

/*
** AABoxRenderObjClass Implementation
*/

AABoxRenderObjClass::AABoxRenderObjClass()
{
	update_cached_box();
}


AABoxRenderObjClass::AABoxRenderObjClass(const W3dBoxStruct & def) :
	BoxRenderObjClass(def)
{
	update_cached_box();
}


AABoxRenderObjClass::AABoxRenderObjClass(const AABoxRenderObjClass & src)
{
	*this = src;
}


AABoxRenderObjClass::AABoxRenderObjClass(const AABoxClass & box)
{
	ObjSpaceCenter.Set(0,0,0);
	ObjSpaceExtent.Set(box.Extent);
	Set_Position(box.Center);
	update_cached_box();
}


AABoxRenderObjClass & AABoxRenderObjClass::operator = (const AABoxRenderObjClass & that)
{
	if (this != &that) {
		BoxRenderObjClass::operator = (that);
		CachedBox = that.CachedBox;
	}
	return *this;
}


RenderObjClass * AABoxRenderObjClass::Clone() const
{
	return W3DNEW AABoxRenderObjClass(*this);
}


int AABoxRenderObjClass::Class_ID() const
{
	return RenderObjClass::CLASSID_AABOX;
}


void AABoxRenderObjClass::Render(RenderInfoClass & rinfo)
{
	Matrix3D temp(1);
	temp.Translate(Transform.Get_Translation());
	Renderer::Set_Transform(RenderTransform_World,temp);
	render_box(rinfo,ObjSpaceCenter,ObjSpaceExtent);
}


void AABoxRenderObjClass::Special_Render(SpecialRenderInfoClass & rinfo)
{
	if (rinfo.RenderType == SpecialRenderInfoClass::RENDER_VIS) {
		WWASSERT(rinfo.VisRasterizer != nullptr);
		Matrix3D temp(1);
		temp.Translate(Transform.Get_Translation());
		rinfo.VisRasterizer->Set_Model_Transform(temp);
		vis_render_box(rinfo,ObjSpaceCenter,ObjSpaceExtent);
	}
}


void AABoxRenderObjClass::Set_Transform(const Matrix3D &m)
{
	RenderObjClass::Set_Transform(m);
	update_cached_box();
}


void AABoxRenderObjClass::Set_Position(const Vector3 &v)
{
	RenderObjClass::Set_Position(v);
	update_cached_box();
}


void AABoxRenderObjClass::update_cached_box()
{
	CachedBox.Center = Transform.Get_Translation() + ObjSpaceCenter;
	CachedBox.Extent = ObjSpaceExtent;
}


bool AABoxRenderObjClass::Cast_Ray(RayCollisionTestClass & raytest)
{
	if ((Get_Collision_Type() & raytest.CollisionType) == 0) return false;
	if (Is_Animation_Hidden()) return false;
	if (raytest.Result->StartBad) return false;

	if (CollisionMath::Collide(raytest.Ray,CachedBox,raytest.Result)) {
		raytest.CollidedRenderObj = this;
		return true;
	}
	return false;
}


bool AABoxRenderObjClass::Cast_AABox(AABoxCollisionTestClass & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;
	if (boxtest.Result->StartBad) return false;

	if (CollisionMath::Collide(boxtest.Box,boxtest.Move,CachedBox,boxtest.Result)) {
		boxtest.CollidedRenderObj = this;
		return true;
	}
	return false;
}


bool AABoxRenderObjClass::Cast_OBBox(OBBoxCollisionTestClass & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;
	if (boxtest.Result->StartBad) return false;

	if (CollisionMath::Collide(boxtest.Box,boxtest.Move,CachedBox,Vector3(0,0,0),boxtest.Result)) {
		boxtest.CollidedRenderObj = this;
		return true;
	}
	return false;
}


bool AABoxRenderObjClass::Intersect_AABox(AABoxIntersectionTestClass & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;
	return CollisionMath::Intersection_Test(CachedBox,boxtest.Box);
}


bool AABoxRenderObjClass::Intersect_OBBox(OBBoxIntersectionTestClass & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;
	return CollisionMath::Intersection_Test(CachedBox,boxtest.Box);
}


void AABoxRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	sphere.Init(ObjSpaceCenter,ObjSpaceExtent.Length());
}


void AABoxRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	box.Init(ObjSpaceCenter,ObjSpaceExtent);
}


OBBoxRenderObjClass::OBBoxRenderObjClass()
{
	update_cached_box();
}


OBBoxRenderObjClass::OBBoxRenderObjClass(const W3dBoxStruct & def) :
	BoxRenderObjClass(def)
{
	update_cached_box();
}


OBBoxRenderObjClass::OBBoxRenderObjClass(const OBBoxRenderObjClass & that)
{
	*this = that;
}


OBBoxRenderObjClass::OBBoxRenderObjClass(const OBBoxClass & box)
{
	ObjSpaceCenter.Set(Vector3(0,0,0));
	ObjSpaceExtent.Set(box.Extent);
	Set_Transform(Matrix3D(box.Basis,box.Center));
	update_cached_box(); // cached box should == box!
}


OBBoxRenderObjClass & OBBoxRenderObjClass::operator = (const OBBoxRenderObjClass & that)
{
	if (this != &that) {
		BoxRenderObjClass::operator = (that);
		CachedBox = that.CachedBox;
	}
	return *this;
}


RenderObjClass * OBBoxRenderObjClass::Clone() const
{
	return W3DNEW OBBoxRenderObjClass(*this);
}


int OBBoxRenderObjClass::Class_ID() const
{
	return RenderObjClass::CLASSID_OBBOX;
}


void OBBoxRenderObjClass::Render(RenderInfoClass & rinfo)
{
	Renderer::Set_Transform(RenderTransform_World,Transform);
	render_box(rinfo,ObjSpaceCenter,ObjSpaceExtent);
}


void OBBoxRenderObjClass::Special_Render(SpecialRenderInfoClass & rinfo)
{
	if (rinfo.RenderType == SpecialRenderInfoClass::RENDER_VIS) {
		WWASSERT(rinfo.VisRasterizer != nullptr);
		rinfo.VisRasterizer->Set_Model_Transform(Transform);
		vis_render_box(rinfo,ObjSpaceCenter,ObjSpaceExtent);
	}
}


void OBBoxRenderObjClass::Set_Transform(const Matrix3D &m)
{
	RenderObjClass::Set_Transform(m);
	update_cached_box();
}


void OBBoxRenderObjClass::Set_Position(const Vector3 &v)
{
	RenderObjClass::Set_Position(v);
	update_cached_box();
}


void OBBoxRenderObjClass::update_cached_box()
{
	Matrix3D::Transform_Vector(Transform,ObjSpaceCenter,&CachedBox.Center);
	CachedBox.Extent.Set(ObjSpaceExtent);
	CachedBox.Basis.Set(Transform);
}


bool OBBoxRenderObjClass::Cast_Ray(RayCollisionTestClass & raytest)
{
	if ((Get_Collision_Type() & raytest.CollisionType) == 0) return false;
	if (Is_Animation_Hidden()) return false;
	if (raytest.Result->StartBad) return false;

	if (CollisionMath::Collide(raytest.Ray,CachedBox,raytest.Result)) {
		raytest.CollidedRenderObj = this;
		return true;
	}
	return false;
}


bool OBBoxRenderObjClass::Cast_AABox(AABoxCollisionTestClass & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;
	if (boxtest.Result->StartBad) return false;

	if (CollisionMath::Collide(boxtest.Box,boxtest.Move,CachedBox,Vector3(0,0,0),boxtest.Result)) {
		boxtest.CollidedRenderObj = this;
		return true;
	}
	return false;
}


bool OBBoxRenderObjClass::Cast_OBBox(OBBoxCollisionTestClass & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;
	if (boxtest.Result->StartBad) return false;

	if (CollisionMath::Collide(boxtest.Box,boxtest.Move,CachedBox,Vector3(0,0,0),boxtest.Result)) {
		boxtest.CollidedRenderObj = this;
		return true;
	}
	return false;
}


bool OBBoxRenderObjClass::Intersect_AABox(AABoxIntersectionTestClass & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;
	return CollisionMath::Intersection_Test(CachedBox,boxtest.Box);
}


bool OBBoxRenderObjClass::Intersect_OBBox(OBBoxIntersectionTestClass & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;
	return CollisionMath::Intersection_Test(CachedBox,boxtest.Box);
}


void OBBoxRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	sphere.Init(ObjSpaceCenter,ObjSpaceExtent.Length());
}


void OBBoxRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	box.Init(ObjSpaceCenter,ObjSpaceExtent);
}


// Returns the cached world-space box.
OBBoxClass & OBBoxRenderObjClass::Get_Box()
{
	Validate_Transform();
	update_cached_box();
	return CachedBox;
}

/*
** BoxLoaderClass Implementation
*/
PrototypeClass * BoxLoaderClass::Load_W3D(ChunkLoadClass & cload)
{
	W3dBoxStruct box;
	cload.Read(&box,sizeof(box));
	return W3DNEW BoxPrototypeClass(box);
}

/*
** BoxPrototypeClass Implementation
*/
BoxPrototypeClass::BoxPrototypeClass(W3dBoxStruct box)
{
	Definition = box;
}

const char * BoxPrototypeClass::Get_Name() const
{
	return Definition.Name;
}

int BoxPrototypeClass::Get_Class_ID() const
{
	if (Definition.Attributes & W3D_BOX_ATTRIBUTE_ORIENTED) {
		return RenderObjClass::CLASSID_OBBOX;
	} else {
		return RenderObjClass::CLASSID_AABOX;
	}
}

RenderObjClass * BoxPrototypeClass::Create()
{
	if (Definition.Attributes & W3D_BOX_ATTRIBUTE_ORIENTED) {
		return NEW_REF( OBBoxRenderObjClass, (Definition) );
	} else {
		return NEW_REF( AABoxRenderObjClass, (Definition) );
	}
}

/*
** Global instance of the box loader
*/
BoxLoaderClass _BoxLoader;



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
 *                     $Archive:: /Commando/Code/ww3d2/ringobj.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/24/01 6:17p                                              $*
 *                                                                                             *
 *                    $Revision:: 27                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   RingRenderObjClass::RingRenderObjClass -- Constructor                                     *
 *   RingRenderObjClass::RingRenderObjClass -- Constructor - init from a definition            *
 *   RingRenderObjClass::RingRenderObjClass -- Copy constructor                                *
 *   RingRenderObjClass::~RingRenderObjClass -- destructor                                     *
 *   RingRenderObjClass::operator -- assignment operator												     *
 *   RingRenderObjClass::Get_Num_Polys -- returns number of polygons									  *
 *   RingRenderObjClass::Get_Name -- returns name															  *
 *   RingRenderObjClass::Set_Name -- sets the name                                             *
 *   RingRenderObjClass::Set_Color -- Sets the color of the Ring                               *
 *   RingRenderObjClass::Init_Ring_Render_System -- global initialization needed for Ring		  *
 *   RingRenderObjClass::Shutdown_Ring_Render_System -- cleanup Ring render system				  *
 *   RingRenderObjClass::Set_Ring_Display_Mask -- Sets global display mask for all Ringes		  *
 *   RingRenderObjClass::Get_Ring_Display_Mask -- returns the display mask							  *
 *   RingRenderObjClass::update_mesh_data -- Updates vertex positions, etc                     *
 *   RingRenderObjClass::render_Ring -- submits the Ring to the GERD									  *
 *   RingRenderObjClass::vis_render_Ring -- submits Ring to the GERD for VIS						  *
 *   RingRenderObjClass::RingRenderObjClass -- constructor												  *
 *   RingRenderObjClass::RingRenderObjClass -- Constructor - init from a definition				  *
 *   RingRenderObjClass::RingRenderObjClass -- copy constructor										  *
 *   RingRenderObjClass::RingRenderObjClass -- Constructor from a wwmath aaRing					  *
 *   RingRenderObjClass::operator -- assignment operator                                       *
 *   RingRenderObjClass::Clone -- clones the Ring															  *
 *   RingRenderObjClass::Class_ID -- returns the class-id for AARing's								  *
 *   RingRenderObjClass::Render -- render this Ring														  *
 *   RingRenderObjClass::Special_Render -- special render this Ring (vis)							  *
 *   RingRenderObjClass::Set_Transform -- set the transform for this Ring							  *
 *   RingRenderObjClass::Set_Position -- Set the position of this Ring								  *
 *   RingRenderObjClass::update_cached_Ring -- update the world-space version of this Ring	  *
 *   RingRenderObjClass::Cast_Ray -- cast a ray against this Ring										  *
 *   RingRenderObjClass::Cast_AARing -- cast an AARing against this Ring							  *
 *   RingRenderObjClass::Cast_OBRing -- cast an OBRing against this Ring							  *
 *   RingRenderObjClass::Get_Obj_Space_Bounding_Sphere -- return the object-space bounding sper*
 *   RingRenderObjClass::Get_Obj_Space_Bounding_Box -- returns the obj-space bounding box      *
 *   RingRenderObjClass::Scale -- scales ring uniformly.                                       *
 *   RingRenderObjClass::Scale -- scales ring non-uniformly.                                   *
 *   RingRenderObjClass::Update_Cached_Bounding_Volumes -- Updates world-space bounding volum  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "ringobj.h"
#include "w3d_util.h"
#include "wwdebug.h"
#include "vertmaterial.h"
#include "ww3d.h"
#include "chunkio.h"
#include "rinfo.h"
#include "coltest.h"
#include "inttest.h"
#include	"matrix3.h"
#include	"wwmath.h"
#include "assetmgr.h"
#include "wwstring.h"
#include "bound.h"
#include "camera.h"
#include "statistics.h"
#include "predlod.h"
#include "dx8wrapper.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "sortingrenderer.h"
#include "Vector3i.h"
#include "visrasterizer.h"


static bool Ring_Array_Valid = false;


/**
** RingMeshClass
** A set of LODs of RingMeshes are re-used by all RingRenderObjClasses.
*/
class RingMeshClass
{
friend class RingRenderObjClass;

public:
	// Constructor
	RingMeshClass(void);
	RingMeshClass(float radius, int slices);
	// Destructor
	~RingMeshClass(void);

	void Generate(float radius, int slices);

	int	Get_Num_Polys(void) { return face_ct; };

	void	Scale (const Vector2 &inner_scale, const Vector2 &outer_scale);
	void	Set_Tiling (int count);

private:

	void	Free(void);

	float	Radius;
	int	Slices;
	int	face_ct;			//# of faces

	int		TileCount;
	Vector2	InnerScale;
	Vector2	OuterScale;

	int		Vertex_ct;		// vertex count
	Vector3	*vtx;				// array of vertices
	Vector2	*orig_vtx;		// array of vertices representing a 'unit' ring
	Vector3	*vtx_normal;	// array of vertex normals
	Vector2	*vtx_uv;			// array of vertex uv coordinates

	TriIndex	*tri_poly;	// array of triangle polys, vertex indices (can be discard if switch to fan renderer)
};


RingMeshClass RingMeshArray[RING_NUM_LOD];
float RingLODCosts[RING_NUM_LOD + 1];	// RING_NUM_LOD doesn't include the null LOD



/*
** RingRenderObjClass Implementation
*/


/***********************************************************************************************
 * RingRenderObjClass::RingRenderObjClass -- Constructor                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
RingRenderObjClass::RingRenderObjClass(void)
	:	anim_time (0.0F),
		IsAnimating(false),
		LODBias(1.0f),
		CurrentLOD(RING_NUM_LOD),	// RING_NUM_LOD does not include the null LOD
		AnimDuration (0.0F),
		RingMaterial (NULL),
		RingTexture (NULL),
		Color (0.75F,0.75F,0.75F),
		InnerScale (1, 1),
		OuterScale (1, 1),
		Alpha (1.0F),
		Flags(0),
		TextureTileCount(5),
		ObjSpaceCenter (0, 0, 0),
		ObjSpaceExtent (1, 1, 1),
		InnerExtent (0.5F, 0.5F),
		OuterExtent (1.0F, 1.0F)
{
	Generate_Shared_Mesh_Arrays ();

	memset(Name,0,sizeof(Name));

	Init_Material () ;

	// So that the object is ready for use after construction, we will
	// complete its initialization by initializing its value array
	// according to a screen area of 1.
	calculate_value_array(1.0f, Value);
}

/***********************************************************************************************
 * RingRenderObjClass::RingRenderObjClass -- Constructor - init from a definition          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
RingRenderObjClass::RingRenderObjClass(const W3dRingStruct & def)
	:	anim_time (0.0F),
		IsAnimating(false),
		LODBias(1.0f),
		CurrentLOD(RING_NUM_LOD),	// RING_NUM_LOD does not include the null LOD
		AnimDuration (0.0F),
		RingMaterial (NULL),
		RingTexture (NULL),
		Color (0.75F,0.75F,0.75F),
		InnerScale (1, 1),
		OuterScale (1, 1),
		Alpha (1.0F),
		Flags(0),
		TextureTileCount(5),
		ObjSpaceCenter (0, 0, 0),
		ObjSpaceExtent (1, 1, 1),
		InnerExtent (0.5F, 0.5F),
		OuterExtent (1.0F, 1.0F)
{
	Generate_Shared_Mesh_Arrays ();
	Init_Material ();

	//
	//	Initialize from the defintion
	//
	Set_Name(def.Name);
	Set_Local_Center_Extent (	Vector3 (def.Center.X, def.Center.Y, def.Center.Z),
										Vector3 (def.Extent.X, def.Extent.Y, def.Extent.Z));
	if (def.TextureName[0] != 0) {
		RingTexture = WW3DAssetManager::Get_Instance ()->Get_Texture (def.TextureName);
	}

	// So that the object is ready for use after construction, we will
	// complete its initialization by initializing its value array
	// according to a screen area of 1.
	calculate_value_array(1.0f, Value);
}


/***********************************************************************************************
 * RingRenderObjClass::RingRenderObjClass -- Copy constructor                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
RingRenderObjClass::RingRenderObjClass(const RingRenderObjClass & src)
	:	anim_time (0.0F),
		IsAnimating(false),
		LODBias(1.0f),
		CurrentLOD(RING_NUM_LOD),	// RING_NUM_LOD does not include the null LOD
		AnimDuration (0.0F),
		RingMaterial (NULL),
		RingTexture (NULL),
		Color (0.75F,0.75F,0.75F),
		InnerScale (1, 1),
		OuterScale (1, 1),
		Alpha (1.0F),
		Flags(0),
		TextureTileCount(5),
		ObjSpaceCenter (0, 0, 0),
		ObjSpaceExtent (1, 1, 1),
		InnerExtent (0.5F, 0.5F),
		OuterExtent (1.0F, 1.0F)
{
	Generate_Shared_Mesh_Arrays ();
	Init_Material ();

	*this = src;

	// So that the object is ready for use after construction, we will
	// complete its initialization by initializing its value array
	// according to a screen area of 1.
	calculate_value_array(1.0f, Value);
}


/***********************************************************************************************
 * RingRenderObjClass::~RingRenderObjClass -- destructor                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/7/2001   gth : Created.                                                                 *
 *=============================================================================================*/
RingRenderObjClass::~RingRenderObjClass()
{
	REF_PTR_RELEASE(RingMaterial);
	REF_PTR_RELEASE(RingTexture);
}


/***********************************************************************************************
 * RingRenderObjClass::operator -- assignment operator                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
RingRenderObjClass & RingRenderObjClass::operator = (const RingRenderObjClass & that)
{
	if (this != &that) {
		RenderObjClass::operator = (that);
		Set_Name(that.Get_Name());
		Color					= that.Color;
		Alpha					= that.Alpha;
		InnerScale			= that.InnerScale;
		OuterScale			= that.OuterScale;
		Flags					= that.Flags;
		RingShader			= that.RingShader;
		CachedBox			= that.CachedBox;
		anim_time			= that.anim_time;
		AnimDuration		= that.AnimDuration;
		ObjSpaceCenter		= that.ObjSpaceCenter;
		ObjSpaceExtent		= that.ObjSpaceExtent;
		ColorChannel		= that.ColorChannel;
		AlphaChannel		= that.AlphaChannel;
		InnerScaleChannel	= that.InnerScaleChannel;
		OuterScaleChannel	= that.OuterScaleChannel;
		InnerExtent			= that.InnerExtent;
		OuterExtent			= that.OuterExtent;
		TextureTileCount	= that.TextureTileCount;

		Set_Texture (that.RingTexture);
	}

	return *this;
}


/***********************************************************************************************
 * RingRenderObjClass::Generate_Shared_Mesh_Arrays  -- Generates mesh LOD arrays.				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/08/00    pds : Created.                                                                *
 *=============================================================================================*/
void RingRenderObjClass::Generate_Shared_Mesh_Arrays (void)
{
	// Generate shared Mesh Arrays
	if (!Ring_Array_Valid) {

		float size = RING_LOWEST_LOD;
		float step = (RING_HIGHEST_LOD - RING_LOWEST_LOD);
		step /= RING_NUM_LOD;

		// For NULL LOD set Cost to a small nonzero amount to avoid divisions by zero.
		RingLODCosts[0] = 0.000001f;
		for(int i=0; i < RING_NUM_LOD; i++) {

			RingMeshArray[i].Generate(1.0f, size);
			RingLODCosts[i + 1] = RingMeshArray[i].Get_Num_Polys();

			size+=step;
		}

		Ring_Array_Valid = true;
	}

	return ;
}


// This is used both by Prepare_LOD and Calculate_Cost_Value_Arrays.
void RingRenderObjClass::calculate_value_array(float screen_area, float *values) const
{
	values[0] = AT_MIN_LOD;
	for (int lod = 1; lod <= RING_NUM_LOD; lod++) {
		float polycount = RingLODCosts[lod];
		float benefit_factor = 1 - (0.5f / (polycount * polycount));
		values[lod] = (benefit_factor * screen_area * LODBias) / polycount;
	}
	values[RING_NUM_LOD + 1] = AT_MAX_LOD; 	// Post-inc value will flag max LOD.
}


/***********************************************************************************************
 * RingRenderObjClass::Init_Material -- Sets up the material and default shader for the Ring.*
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/08/00    pds : Created.                                                                *
 *=============================================================================================*/
void RingRenderObjClass::Init_Material (void)
{
	REF_PTR_RELEASE (RingMaterial);

	RingMaterial = NEW_REF(VertexMaterialClass,());
	RingMaterial->Set_Ambient(0,0,0);
	RingMaterial->Set_Diffuse(0,0,0);
	RingMaterial->Set_Specular(0,0,0);
	RingMaterial->Set_Emissive(1,1,1);
	RingMaterial->Set_Opacity(0.25f);
	RingMaterial->Set_Shininess(0.0f);

	// Texturing, zbuffer, primary gradient, alpha blending
	RingShader = ShaderClass::_PresetAlphaShader;

}



/***********************************************************************************************
 * RingRenderObjClass::Get_Num_Polys -- returns number of polygons                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
int RingRenderObjClass::Get_Num_Polys(void) const
{
	return RingLODCosts[CurrentLOD];
}

/***********************************************************************************************
 * RingRenderObjClass::Set_Texture						                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::Set_Texture(TextureClass *tf)
{
	REF_PTR_SET(RingTexture,tf);
}


/***********************************************************************************************
 * RingRenderObjClass::Get_Name -- returns name                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
const char * RingRenderObjClass::Get_Name(void) const
{
	return Name;
}


/***********************************************************************************************
 * RingRenderObjClass::Set_Name -- sets the name                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::Set_Name(const char * name)
{
	WWASSERT(name != NULL);
	WWASSERT(strlen(name) < 2*W3D_NAME_LEN);
	strcpy(Name,name);
}

/***********************************************************************************************
 * RingRenderObjClass::render_ring -- submits the box to the GERD                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::render_ring(RenderInfoClass & rinfo,const Vector3 & center,const Vector3 & extent)
{
	// Should never get here with NULL LOD
	if (CurrentLOD == 0) {
		WWASSERT(0);
		return;
	}

	RingMeshClass & ring = RingMeshArray[CurrentLOD - 1];

	if (RingTexture) {
		RingShader.Set_Texturing (ShaderClass::TEXTURING_ENABLE);
	} else {
		RingShader.Set_Texturing (ShaderClass::TEXTURING_DISABLE);
	}
	DX8Wrapper::Set_Shader(RingShader);
	DX8Wrapper::Set_Texture(0,RingTexture);
	DX8Wrapper::Set_Material(RingMaterial);

	// Enable sorting if the primitive is translucent, alpha testing is not enabled, and sorting is enabled globally.
	const bool sort = (RingShader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO) && (RingShader.Get_Alpha_Test() == ShaderClass::ALPHATEST_DISABLE) && (WW3D::Is_Sorting_Enabled());
	const unsigned int buffer_type = sort ? BUFFER_TYPE_DYNAMIC_SORTING : BUFFER_TYPE_DYNAMIC_DX8;

	DynamicVBAccessClass vb(buffer_type, dynamic_fvf_type, ring.Vertex_ct);
	{
		DynamicVBAccessClass::WriteLockClass Lock(&vb);
		VertexFormatXYZNDUV2 *vb = Lock.Get_Formatted_Vertex_Array();

		//
		// set up the vertex color+alpha
		//
		unsigned color;
		if (RingShader.Get_Dst_Blend_Func () == ShaderClass::DSTBLEND_ONE) {
			color = DX8Wrapper::Convert_Color(Alpha * Color,1.0f);
		} else {
			color = DX8Wrapper::Convert_Color(Color,Alpha);
		}

		for (int i=0; i<ring.Vertex_ct; i++)
		{
			vb->x = ring.vtx[i].X;
			vb->y = ring.vtx[i].Y;
			vb->z = ring.vtx[i].Z;

			vb->nx = ring.vtx_normal[i].X;		// may not need this!
			vb->ny = ring.vtx_normal[i].Y;
			vb->nz = ring.vtx_normal[i].Z;

			vb->diffuse = color;

			if (RingTexture) {
				vb->u1 = ring.vtx_uv[i].X;
				vb->v1 = ring.vtx_uv[i].Y;
			}
			vb++;
		}
	}

	DynamicIBAccessClass ib(buffer_type, ring.face_ct * 3);
	{
		DynamicIBAccessClass::WriteLockClass Lock(&ib);
		unsigned short *mem=Lock.Get_Index_Array();
		for (int i=0; i<ring.face_ct; i++)
		{
			mem[3*i]=ring.tri_poly[i].I;
			mem[3*i+1]=ring.tri_poly[i].J;
			mem[3*i+2]=ring.tri_poly[i].K;
		}
	}

	DX8Wrapper::Set_Vertex_Buffer(vb);
	DX8Wrapper::Set_Index_Buffer(ib,0);

	if (sort) {
		SortingRendererClass::Insert_Triangles(Get_Bounding_Sphere(), 0, ring.face_ct, 0, ring.Vertex_ct);
	} else {
		DX8Wrapper::Draw_Triangles(0, ring.face_ct, 0, ring.Vertex_ct);
	}

}


/***********************************************************************************************
 * RingRenderObjClass::vis_render_ring -- submits box to the GERD for VIS                      *
 *                                                                                             *
 * this renders the ring with the specified VIS-ID.                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::vis_render_ring(SpecialRenderInfoClass & rinfo,const Vector3 & center,const Vector3 & extent)
{
	WWASSERT(0);
}


/***********************************************************************************************
 * RingRenderObjClass::Clone -- clones the ring                                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass * RingRenderObjClass::Clone(void) const
{
	return W3DNEW RingRenderObjClass(*this);
}


/***********************************************************************************************
 * RingRenderObjClass::Class_ID -- returns the class-id for Rings's                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
int RingRenderObjClass::Class_ID(void) const
{
	return RenderObjClass::CLASSID_RING;
}


/***********************************************************************************************
 * RingRenderObjClass::Render -- render this ring                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::Render(RenderInfoClass & rinfo)
{
	// NULL LOD
	if (CurrentLOD == 0) return;

	if (Is_Not_Hidden_At_All() == false) {
		return;
	}

	unsigned int sort_level = SORT_LEVEL_NONE;

	if (!WW3D::Is_Sorting_Enabled()) {
		sort_level = RingShader.Guess_Sort_Level();
	}

	if (WW3D::Are_Static_Sort_Lists_Enabled() && sort_level != SORT_LEVEL_NONE) {

		WW3D::Add_To_Static_Sort_List(this, sort_level);

	} else {

		// Process texture reductions:
//		if (RingTexture) RingTexture->Process_Reduction();

		Matrix3D temp = Transform;

		// Do Time Based Animation
		animate ();

		//
		//	Scale the inner and outer parts of the ring.
		//
		Vector2 inner_scale (InnerExtent.X * InnerScale.X, InnerExtent.Y * InnerScale.Y);
		Vector2 outer_scale (OuterExtent.X * OuterScale.X, OuterExtent.Y * OuterScale.Y);
		RingMeshArray[CurrentLOD - 1].Scale(inner_scale, outer_scale);

		//
		//	Make sure this mesh uses the correct UV tiling
		//
		if (RingTexture != NULL) {
			RingMeshArray[CurrentLOD - 1].Set_Tiling (TextureTileCount);
		}

		//
		//	Should we force the ring to be camera aligned?
		// (this will cause the ring to be parallel to the screen)
		//
		if (Flags & USE_CAMERA_ALIGN) {
			Vector3 obj_position;
			Vector3 camera_z_vector;

			rinfo.Camera.Get_Transform().Get_Z_Vector(&camera_z_vector);
			Transform.Get_Translation(&obj_position);

			Matrix3D temp;
			temp.Look_At(obj_position, obj_position + camera_z_vector, 0.0f);
			DX8Wrapper::Set_Transform(D3DTS_WORLD, temp);
		} else {
			DX8Wrapper::Set_Transform(D3DTS_WORLD, Transform);
		}

		//
		//	Pass the geometry on to DX8
		//
		render_ring (rinfo, ObjSpaceCenter, ObjSpaceExtent);
	}
}


/***********************************************************************************************
 * RingRenderObjClass::Get_Default_Color - get the default (or first frame) value				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/13/2000    pds : Created.                                                               *
 *=============================================================================================*/
Vector3 RingRenderObjClass::Get_Default_Color(void) const
{
	Vector3 value;

	if (ColorChannel.Get_Key_Count () > 0) {
		value = ColorChannel.Get_Key (0).Get_Value ();
	} else {
		value = Color;
	}

	return value;
}


/***********************************************************************************************
 * RingRenderObjClass::Get_Default_Alpha - get the default (or first frame) value				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/13/2000    pds : Created.                                                               *
 *=============================================================================================*/
float RingRenderObjClass::Get_Default_Alpha(void) const
{
	float  value;

	if (AlphaChannel.Get_Key_Count () > 0) {
		value = AlphaChannel.Get_Key (0).Get_Value ();
	} else {
		value = Alpha;
	}

	return value;
}


/***********************************************************************************************
 * RingRenderObjClass::Get_Default_Inner_Scale - get the default (or first frame) value		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/13/2000    pds : Created.                                                               *
 *=============================================================================================*/
Vector2 RingRenderObjClass::Get_Default_Inner_Scale(void) const
{
	Vector2 value;

	if (InnerScaleChannel.Get_Key_Count () > 0) {
		value = InnerScaleChannel.Get_Key (0).Get_Value ();
	} else {
		value = InnerScale;
	}

	return value;
}


/***********************************************************************************************
 * RingRenderObjClass::Get_Default_Outer_Scale - get the default (or first frame) value		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/13/2000    pds : Created.                                                               *
 *=============================================================================================*/
Vector2 RingRenderObjClass::Get_Default_Outer_Scale(void) const
{
	Vector2 value;

	if (OuterScaleChannel.Get_Key_Count () > 0) {
		value = OuterScaleChannel.Get_Key (0).Get_Value ();
	} else {
		value = OuterScale;
	}

	return value;
}


/***********************************************************************************************
 * RingRenderObjClass::Special_Render -- special render this box (vis)                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
, *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::Special_Render(SpecialRenderInfoClass & rinfo)
{
	Matrix3D temp(1);
	temp.Translate(Transform.Get_Translation());

	if (rinfo.RenderType == SpecialRenderInfoClass::RENDER_VIS) {
		WWASSERT(rinfo.VisRasterizer != NULL);
		rinfo.VisRasterizer->Set_Model_Transform(temp);
		vis_render_ring(rinfo,ObjSpaceCenter,ObjSpaceExtent);
	}
}


/***********************************************************************************************
 * RingRenderObjClass::Set_Transform -- set the transform for this box                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::Set_Transform(const Matrix3D &m)
{
	RenderObjClass::Set_Transform(m);
	update_cached_box();
}


/***********************************************************************************************
 * RingRenderObjClass::Set_Position -- Set the position of this box                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::Set_Position(const Vector3 &v)
{
	RenderObjClass::Set_Position(v);
	update_cached_box();
}


/***********************************************************************************************
 * RingRenderObjClass::update_cached_box -- update the world-space version of this box        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::update_cached_box(void)
{
	CachedBox.Center = Transform.Get_Translation() + ObjSpaceCenter;
	CachedBox.Extent = ObjSpaceExtent;
}


/***********************************************************************************************
 * RingRenderObjClass::Get_Obj_Space_Bounding_Ring -- return the object-space bounding sphe *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	sphere.Center = ObjSpaceCenter;
	sphere.Radius = ObjSpaceExtent.Length();
}


/***********************************************************************************************
 * RingRenderObjClass::Get_Obj_Space_Bounding_Box -- returns the obj-space bounding box       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	box.Center = ObjSpaceCenter;
	box.Extent = ObjSpaceExtent;
}


void RingRenderObjClass::Prepare_LOD(CameraClass &camera)
{
	if (Is_Not_Hidden_At_All() == false) {
		return;
	}

	calculate_value_array(Get_Screen_Size(camera), Value);

	PredictiveLODOptimizerClass::Add_Object(this);
}

void RingRenderObjClass::Increment_LOD(void)
{
	if (CurrentLOD < RING_NUM_LOD) CurrentLOD++;
}

void RingRenderObjClass::Decrement_LOD(void)
{
	if (CurrentLOD > 0) CurrentLOD--;
}

float RingRenderObjClass::Get_Cost(void) const
{
	return Get_Num_Polys();	// Currently cost == polys
}

float RingRenderObjClass::Get_Value(void) const
{
	return Value[CurrentLOD];
}

float RingRenderObjClass::Get_Post_Increment_Value(void) const
{
	return Value[CurrentLOD + 1];
}

void RingRenderObjClass::Set_LOD_Level(int lod)
{
	CurrentLOD = Bound(lod, 0, RING_NUM_LOD);	// RING_NUM_LOD doesn't include the null LOD
}

int RingRenderObjClass::Get_LOD_Level(void) const
{
	return CurrentLOD;
}

int RingRenderObjClass::Get_LOD_Count(void) const
{
	return RING_NUM_LOD + 1;	// RING_NUM_LOD doesn't include the null LOD
}

int RingRenderObjClass::Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const
{
	calculate_value_array(screen_area, values);
	for (int lod = 0; lod <= RING_NUM_LOD; lod++) {
		costs[lod] = RingLODCosts[lod];
	}

	// We currently don't support max screen size or minimum LOD clamping for ring objects
	return 0;
}


/***********************************************************************************************
 * RingRenderObjClass::Scale -- scales ring uniformly.                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/01/02    NH : Created.                                                                  *
 *=============================================================================================*/
void RingRenderObjClass::Scale(float scale)
{
	if (scale == 1.0f) return;

	// Scale default values
	InnerScale *= scale;
	OuterScale *= scale;

	// Loop over all keys in inner and outer scale channel and scale values.
	int i;
	int inner_count = InnerScaleChannel.Get_Key_Count();
	for (i = 0; i < inner_count; i++) {
		const PrimitiveAnimationChannelClass<Vector2>::KeyClass &key = InnerScaleChannel.Get_Key(i);
		InnerScaleChannel.Set_Key_Value(i, key.Get_Value() * scale);
	}
	int outer_count = OuterScaleChannel.Get_Key_Count();
	for (i = 0; i < outer_count; i++) {
		const PrimitiveAnimationChannelClass<Vector2>::KeyClass &key = OuterScaleChannel.Get_Key(i);
		OuterScaleChannel.Set_Key_Value(i, key.Get_Value() * scale);
	}
}


/***********************************************************************************************
 * RingRenderObjClass::Scale -- scales ring non-uniformly.                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/01/02    NH : Created.                                                                  *
 *=============================================================================================*/
void RingRenderObjClass::Scale(float scalex, float scaley, float scalez)
{
	// Scale default values
	InnerScale.X *= scalex;
	InnerScale.Y *= scaley;
	OuterScale.X *= scalex;
	OuterScale.Y *= scaley;

	// Loop over all keys in inner and outer scale channel and scale values.
	int i;
	int inner_count = InnerScaleChannel.Get_Key_Count();
	for (i = 0; i < inner_count; i++) {
		const PrimitiveAnimationChannelClass<Vector2>::KeyClass &key = InnerScaleChannel.Get_Key(i);
		Vector2 key_val = key.Get_Value();
		key_val.X *= scalex;
		key_val.Y *= scaley;
		InnerScaleChannel.Set_Key_Value(i, key_val);
	}
	int outer_count = OuterScaleChannel.Get_Key_Count();
	for (i = 0; i < outer_count; i++) {
		const PrimitiveAnimationChannelClass<Vector2>::KeyClass &key = OuterScaleChannel.Get_Key(i);
		Vector2 key_val = key.Get_Value();
		key_val.X *= scalex;
		key_val.Y *= scaley;
		OuterScaleChannel.Set_Key_Value(i, key_val);
	}
}


/***********************************************************************************************
 * RingRenderObjClass::Update_On_Visibilty -- Either starts or stops the animation based on vis*
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/04/00    pds : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::Update_On_Visibilty(void)
{
	// Simply start or stop the animation based on
	// the visibility state of the primitive.
	if (Is_Not_Hidden_At_All () && Is_Animating () == false) {
		Start_Animating ();
	} else if ((Is_Not_Hidden_At_All () == false) && Is_Animating ()) {
		Stop_Animating ();
	}

	return ;
}

/***********************************************************************************************
 * RingRenderObjClass::animate	-- Update Current Display state										  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/07/00    jga : Created.                                                                 *
 *=============================================================================================*/
void RingRenderObjClass::animate()
{
	if (Is_Animating ()) {

		if (	ColorChannel.Get_Key_Count () > 0 ||
				AlphaChannel.Get_Key_Count () > 0 ||
				InnerScaleChannel.Get_Key_Count () > 0 ||
				OuterScaleChannel.Get_Key_Count () > 0)
		{
			if (AnimDuration > 0) {
				float frametime = WW3D::Get_Logic_Frame_Time_Seconds();
				anim_time += frametime / AnimDuration;
			} else {
				anim_time = 1.0F;
			}
			WWASSERT (anim_time >= 0.0F);

			if ((Flags & USE_ANIMATION_LOOP) && anim_time > 1.0F) {
				anim_time -= 1.0F;
			}

			if (ColorChannel.Get_Key_Count () > 0) {
				Color	= ColorChannel.Evaluate (anim_time);
			}

			if (AlphaChannel.Get_Key_Count () > 0) {
				Alpha	= AlphaChannel.Evaluate (anim_time);
			}

			if (InnerScaleChannel.Get_Key_Count () > 0) {
				InnerScale	= InnerScaleChannel.Evaluate (anim_time);
			}

			if (OuterScaleChannel.Get_Key_Count () > 0) {
				OuterScale = OuterScaleChannel.Evaluate (anim_time);
				ObjSpaceExtent.X = OuterScale.X * OuterExtent.X;
				ObjSpaceExtent.Y = OuterScale.Y * OuterExtent.Y;
				ObjSpaceExtent.Z = 0;
				Update_Cached_Bounding_Volumes ();
			}
		}
	}

	return ;
}


/*
** RingLoaderClass Implementation
*/
PrototypeClass * RingLoaderClass::Load_W3D(ChunkLoadClass & cload)
{
	RingPrototypeClass *prototype = W3DNEW RingPrototypeClass ();
	prototype->Load (cload);
	return prototype;
}

/*
** RingPrototypeClass Implementation
*/
RingPrototypeClass::RingPrototypeClass (void)
{
	::memset (&Definition, 0, sizeof (Definition));
	return ;
}

RingPrototypeClass::RingPrototypeClass(RingRenderObjClass *ring)
{
	::memset (&Definition, 0, sizeof (Definition));
	::strcpy (Definition.Name, ring->Get_Name ());

	Definition.AnimDuration			= ring->AnimDuration;
	Definition.Attributes			= ring->Get_Flags ();
	Definition.DefaultAlpha			= ring->Get_Default_Alpha ();
	Definition.DefaultInnerScale	= ring->Get_Default_Inner_Scale ();
	Definition.DefaultOuterScale	= ring->Get_Default_Outer_Scale ();
	Definition.TextureTileCount	= ring->Get_Texture_Tiling ();

	Definition.InnerExtent			= ring->Get_Inner_Extent ();
	Definition.OuterExtent			= ring->Get_Outer_Extent ();

	Vector3 def_color = ring->Get_Default_Color ();
	W3dUtilityClass::Convert_Vector (def_color, &Definition.DefaultColor);

	W3dUtilityClass::Convert_Vector (ring->Get_Box ().Center, &Definition.Center);
	W3dUtilityClass::Convert_Vector (ring->Get_Box ().Extent, &Definition.Extent);
	W3dUtilityClass::Convert_Shader (ring->RingShader, &Definition.Shader);

	//
	//	Determine the texture name for this ring
	//
	if (ring->RingTexture != NULL) {
		StringClass name = ring->RingTexture->Get_Full_Path();
		const char *filename = ::strrchr (name, '\\');
		if (filename != NULL) {
			filename ++;
		} else {
			filename = name;
		}

		::strcpy (Definition.TextureName, filename);
	}

	//
	//	Save the animateable channels
	//
	ColorChannel		= ring->Peek_Color_Channel ();
	AlphaChannel		= ring->Peek_Alpha_Channel ();
	InnerScaleChannel	= ring->Peek_Inner_Scale_Channel ();
	OuterScaleChannel	= ring->Peek_Outer_Scale_Channel ();
	return ;
}

RingPrototypeClass::~RingPrototypeClass (void)
{
	return ;
}

const char * RingPrototypeClass::Get_Name(void) const
{
	return Definition.Name;
}

int RingPrototypeClass::Get_Class_ID(void) const
{
	return RenderObjClass::CLASSID_RING;
}

enum
{
	CHUNKID_RING_DEF						= 1,
	CHUNKID_COLOR_CHANNEL,
	CHUNKID_ALPHA_CHANNEL,
	CHUNKID_INNER_SCALE_CHANNEL,
	CHUNKID_OUTER_SCALE_CHANNEL
};

bool RingPrototypeClass::Load (ChunkLoadClass &cload)
{
	ColorChannel.Reset ();
	AlphaChannel.Reset ();
	InnerScaleChannel.Reset ();
	OuterScaleChannel.Reset ();

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_RING_DEF:
				cload.Read (&Definition, sizeof (Definition));
				break;

			case CHUNKID_COLOR_CHANNEL:
				ColorChannel.Load (cload);
				break;

			case CHUNKID_ALPHA_CHANNEL:
				AlphaChannel.Load (cload);
				break;

			case CHUNKID_INNER_SCALE_CHANNEL:
				InnerScaleChannel.Load (cload);
				break;

			case CHUNKID_OUTER_SCALE_CHANNEL:
				OuterScaleChannel.Load (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}

bool RingPrototypeClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (W3D_CHUNK_RING);

		csave.Begin_Chunk (CHUNKID_RING_DEF);
			csave.Write (&Definition, sizeof (Definition));
		csave.End_Chunk ();

		if (ColorChannel.Get_Key_Count () > 0) {
			csave.Begin_Chunk (CHUNKID_COLOR_CHANNEL);
			ColorChannel.Save (csave);
			csave.End_Chunk ();
		}

		if (AlphaChannel.Get_Key_Count () > 0) {
			csave.Begin_Chunk (CHUNKID_ALPHA_CHANNEL);
			AlphaChannel.Save (csave);
			csave.End_Chunk ();
		}

		if (InnerScaleChannel.Get_Key_Count () > 0) {
			csave.Begin_Chunk (CHUNKID_INNER_SCALE_CHANNEL);
			InnerScaleChannel.Save (csave);
			csave.End_Chunk ();
		}

		if (OuterScaleChannel.Get_Key_Count () > 0) {
			csave.Begin_Chunk (CHUNKID_OUTER_SCALE_CHANNEL);
			OuterScaleChannel.Save (csave);
			csave.End_Chunk ();
		}

	csave.End_Chunk ();
	return true;
}

RenderObjClass * RingPrototypeClass::Create(void)
{
	//
	//	Create the new render object
	//
	RingRenderObjClass *ring = W3DNEW RingRenderObjClass (Definition);

	//
	//	Configure the ring
	//
	W3dUtilityClass::Convert_Shader (Definition.Shader, &ring->RingShader);
	if (WW3DAssetManager::Get_Instance()->Get_Activate_Fog_On_Load()) {
		ring->RingShader.Enable_Fog ("RingPrototypeClass");
		ring->RingShader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
	}
	W3dUtilityClass::Convert_Vector (Definition.DefaultColor, &ring->Color);
	ring->InnerScale = Definition.DefaultInnerScale;
	ring->OuterScale = Definition.DefaultOuterScale;
	ring->Set_Animation_Duration (Definition.AnimDuration);
	ring->Alpha = Definition.DefaultAlpha;
	ring->Set_Flags (Definition.Attributes);
	ring->Set_Inner_Extent (Definition.InnerExtent);
	ring->Set_Outer_Extent (Definition.OuterExtent);
	ring->Set_Texture_Tiling (Definition.TextureTileCount);

	//
	//	Initialize the render object with the keyframe arrays
	//
	ring->Set_Color_Channel (ColorChannel);
	ring->Set_Alpha_Channel (AlphaChannel);
	ring->Set_Inner_Scale_Channel (InnerScaleChannel);
	ring->Set_Outer_Scale_Channel (OuterScaleChannel);
	return ring;
}

/*
** Global instance of the box loader
*/
RingLoaderClass _RingLoader;


//
// Vertices are ordered as such
//  center, followed by outer ring
//
/***********************************************************************************************
 * RingMeshClass::RingMeshClass -- Constructor                                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/07/00    jga : Created.                                                                 *
 *=============================================================================================*/
RingMeshClass::RingMeshClass(float radius, int slices):
Radius(radius),
Slices(slices),
Vertex_ct(0),			// 1 vertex minimum, for center
vtx(NULL),
orig_vtx(NULL),
vtx_normal(NULL),
vtx_uv(NULL),
face_ct(0),
tri_poly(NULL),
TileCount (5),
InnerScale (1.0F, 1.0F),
OuterScale (1.0F, 1.0F)
{
	Generate(radius, slices);
}

/***********************************************************************************************
 * RingMeshClass::RingMeshClass -- Constructor                                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/07/00    jga : Created.                                                                 *
 *=============================================================================================*/
RingMeshClass::RingMeshClass(void):
Radius(0.0f),
Slices(0),
Vertex_ct(0),			// 1 vertex minimum, for center
vtx(NULL),
orig_vtx(NULL),
vtx_normal(NULL),
vtx_uv(NULL),
face_ct(0),
tri_poly(NULL),
TileCount (5),
InnerScale (1.0F, 1.0F),
OuterScale (1.0F, 1.0F)
{
}


void RingMeshClass::Set_Tiling (int count)
{
	if (TileCount != count) {
		TileCount = count;

		//
		//	Calculate the increment 'u' increment value (as in UV).
		//
		float u_inc			= ((float)TileCount) / ((float)(Slices));
		float u_value		= 0.0F;

		//
		//	Reassign the UVs
		//
		for (int index = 0; index < Vertex_ct; index += 2) {
			vtx_uv[index].Set (u_value, 0.0F);
			vtx_uv[index + 1].Set (u_value, 1.0F);
			u_value += u_inc;
		}
	}

	return ;
}


void RingMeshClass::Scale (const Vector2 &inner_scale, const Vector2 &outer_scale)
{
	bool do_inner = (inner_scale != InnerScale);
	bool do_outer = (outer_scale != OuterScale);

	//
	//	Only scale the inner ring if necessary
	//
	if (do_inner) {

		for (int index = 0; index < Vertex_ct; index += 2) {
			vtx[index].X = orig_vtx[index].X * inner_scale.X;
			vtx[index].Y = orig_vtx[index].Y * inner_scale.Y;
			vtx[index].Z = 0;
		}

		InnerScale = inner_scale;
	}

	//
	//	Only scale the outer ring if necessary
	//
	if (do_outer) {

		for (int index = 1; index < Vertex_ct; index += 2) {
			vtx[index].X = orig_vtx[index].X * outer_scale.X;
			vtx[index].Y = orig_vtx[index].Y * outer_scale.Y;
			vtx[index].Z = 0;
		}

		OuterScale = outer_scale;
	}

	return ;
}


/***********************************************************************************************
 * RingMeshClass::Generate - Create Ring Geometry                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/07/00    jga : Created.                                                                 *
 *=============================================================================================*/
void RingMeshClass::Generate(float radius, int slices)
{
	Free ();

	Slices = slices;
	Radius = radius;

	face_ct		= (Slices * 2);
	Vertex_ct	= (Slices * 2) + 2;

	vtx			= W3DNEWARRAY Vector3[Vertex_ct];
	orig_vtx		= W3DNEWARRAY Vector2[Vertex_ct];
	vtx_normal	= W3DNEWARRAY Vector3[Vertex_ct];
	vtx_uv		= W3DNEWARRAY Vector2[Vertex_ct];
	tri_poly		= W3DNEWARRAY TriIndex[face_ct];

	//
	//	Generate vertices, normals, and uv's
	//
	float angle_inc	= DEG_TO_RADF (360) / ((float)Slices);
	float u_inc			= ((float)TileCount) / ((float)(Slices));
	float u_value		= 0.0F;
	float angle			= 0;
	int	index			= 0;

	for (index = 0; index < Vertex_ct; index += 2) {

		float x_pos = -WWMath::Sin (angle);
		float y_pos = WWMath::Cos (angle);

		//
		//	Place the inner index
		//
		vtx[index].X = orig_vtx[index].X = x_pos;
		vtx[index].Y = orig_vtx[index].Y = y_pos;
		vtx[index].Z = 0;

		//
		//	Place the outer index
		//
		vtx[index+1].X = orig_vtx[index+1].X = x_pos;
		vtx[index+1].Y = orig_vtx[index+1].Y = y_pos;
		vtx[index+1].Z = 0;

		//
		//	Normals will just point straight up from the ring...
		//
		vtx_normal[index].Set (0, 0, 1);
		vtx_normal[index + 1].Set (0, 0, 1);

		//
		//	Assign the UVs
		//
		vtx_uv[index].Set (u_value, 0.0F);
		vtx_uv[index + 1].Set (u_value, 1.0F);
		u_value += u_inc;

		angle += angle_inc;
	}

	//
	//	Make the triangle strip...
	//
	for (index = 0; index < face_ct; index ++) {
		tri_poly[index].I = index;
		tri_poly[index].J = index+1;
		tri_poly[index].K = index+2;
	}

	return ;
}


/***********************************************************************************************
 * RingMeshClass::~RingMeshClass -- Destructor                                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/07/00    jga : Created.                                                                 *
 *=============================================================================================*/
RingMeshClass::~RingMeshClass(void)
{
	Free();
}

/***********************************************************************************************
 * RingMeshClass::Free - Release Memory, that the ring mesh is using                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/07/00    jga : Created.                                                                 *
 *=============================================================================================*/
void RingMeshClass::Free(void)
{

	delete vtx;
	delete orig_vtx;
	delete vtx_normal;
	delete vtx_uv;
	delete tri_poly;

	vtx			= NULL;
	orig_vtx		= NULL;
	vtx_normal	= NULL;
	vtx_uv		= NULL;
	tri_poly		= NULL;

}

// EOF - ringobj.cpp

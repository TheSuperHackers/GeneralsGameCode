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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

#include "always.h"
#include "hanim.h"
#include "proto.h"
#include "rendobj.h"
#include "lightenvironment.h"
#include "w3d_file.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "shader.h"
#include "vertmaterial.h"
#include "Lib/BaseType.h"

#pragma once

#ifndef __W3DGRANNY_H_
#define __W3DGRANNY_H_

#ifdef	INCLUDE_GRANNY_IN_BUILD

#include "granny.h"

class GrannyRenderObjSystem;	///<forward reference
class GrannyPrototypeClass;	///<forward reference

class GrannyLoaderClass
{
public:
	GrannyLoaderClass(void);
	~GrannyLoaderClass(void);

	PrototypeClass *	Load_W3D(const char *filename);
	granny_material_library *Get_Material_Library(void) {return MaterialLibrary;}
private:

    granny_texture_library *TextureLibrary;
    granny_material_library *MaterialLibrary;
};

extern GrannyLoaderClass		_GrannyLoader;

/// Custom animation type specific to Granny.
class GrannyAnimClass : public HAnimClass
{
	friend class GrannyRenderObjClass;

	public:
	
	enum
	{
		OK,
		LOAD_ERROR
	};

	GrannyAnimClass(void);
	~GrannyAnimClass(void);

	const char *		Get_Name(void) const	{ return Name;}
	const char *		Get_HName(void) const	{ return Name;}
	int					Get_Num_Frames(void)	{ return NumFrames;}
	float				Get_Frame_Rate()		{ return FrameRate;}
	float				Get_Total_Time()		{ return (float)NumFrames / FrameRate;}
	void				Get_Translation(Vector3& translation, int pividx,float frame) const {translation=Vector3(0,0,0);}
	void				Get_Orientation(Quaternion& orientation, int pividx,float frame) const {orientation=Quaternion(1);}
	void				Get_Transform(Matrix3D& mtx, int pividx, float frame) const {mtx=Matrix3D(1);}
	Bool				Get_Visibility(int pividx,float frame)	{	return 1;}	//assume always visible
	int					Get_Num_Pivots(void) const	{ return 1;}
	Bool				Is_Node_Motion_Present(int pividx) {return true;}
	int					Load_W3D(const char *name);

private:
	granny_animation	*Animation;
	granny_file			*File;
	int NumFrames;
	float FrameRate;
	char	Name[2*W3D_NAME_LEN];
};	

class GrannyAnimManagerClass
{

public:
	GrannyAnimManagerClass(void);
	~GrannyAnimManagerClass(void);

	int			 		Load_Anim(const char *name);
	GrannyAnimClass *		Get_Anim(const char * name);
	GrannyAnimClass *		Peek_Anim(const char * name);
	Bool					Add_Anim(GrannyAnimClass *new_anim);
	void			 		Free_All_Anims(void);

	void					Register_Missing( const char * name );
	Bool					Is_Missing( const char * name );
	void					Reset_Missing( void );

private:
	int					Load_Raw_Anim(const char *name);

	HashTableClass	*	AnimPtrTable;
	HashTableClass	*	MissingAnimTable;

	friend	class		GrannyAnimManagerIterator;
};

/*
** An Iterator to get to all loaded HAnims in a HAnimManager
*/
class GrannyAnimManagerIterator : public HashTableIteratorClass {
public:
	GrannyAnimManagerIterator( GrannyAnimManagerClass & manager ) : HashTableIteratorClass( *manager.AnimPtrTable ) {}
	GrannyAnimClass * Get_Current_Anim( void );
};

/// Custom render object that draws tracks on the terrain.
/**
This render object handles drawing tracks left by objects moving on the terrain.  It will only work
with rectangular planar surfaces and was tuned with an emphasis on water.
Since skies are only visible in reflections, this code will also
render clouds and sky bodies.
*/
class GrannyRenderObjClass : public RenderObjClass
{	
	friend class GrannyRenderObjSystem;
	friend class W3DShadow;

public:

	GrannyRenderObjClass(const GrannyPrototypeClass &proto);
	~GrannyRenderObjClass(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface (W3D methods)
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
    virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & aabox) const;
	virtual void					Set_Animation( HAnimClass * motion,	float frame, int anim_mode = ANIM_MODE_MANUAL);
	void Set_ObjectScale(float scale);	///<adjust internal scale factor applied to rendered asset.
	Bool Cast_Ray(RayCollisionTestClass & raytest);	///<returns intersection of ray and picking box.

	Int freeResources(void);	///<free W3D assets used for this track
	const GrannyPrototypeClass & getPrototype(void) const { return m_prototype;}

protected:
    granny_model_instance *m_modelInstance;	///<granny instance used to control model
	granny_control *m_animationControl;		///<current animation
	const GrannyPrototypeClass &m_prototype;		///<prototype that was used to create this model.
	Int			AnimMode;				///<type of animation playing
	Real		LastSyncTime;			///<time of last animation update
	SphereClass	m_boundingSphere;		///<bounding sphere of TerrainTracks
	AABoxClass	m_boundingBox;			///<bounding box of TerrainTracks
	Int			m_vertexCount;			///<number of vertices in model
	
	GrannyRenderObjClass	*m_nextSystem;			///<next track system
	GrannyRenderObjClass *m_prevSystem;			///<previous track system
};

#define GRANNY_MAX_MESHES_PER_MODEL 4
 
/** Prototype or definition of a Granny model.  Contains all info need to instance a model of this type.*/
class GrannyPrototypeClass : public PrototypeClass
{
	friend class GrannyRenderObjClass;
	friend class GrannyRenderObjSystem;

public:
	GrannyPrototypeClass(granny_file *file)
	{	m_file = file; assert(m_file); m_vertexCount=0;
		m_name[0]='\0';
		m_meshCount=0;
		m_vertexMaterial=NEW_REF(VertexMaterialClass,());
		m_vertexMaterial->Set_Shininess(0.0);
		m_vertexMaterial->Set_Specular(0,0,0);
		m_vertexMaterial->Set_Lighting(true);
	}

	struct grannyMeshDesc
	{
		const UnsignedInt *index;	///< pointer to pool of face indices
		Int indexCount;	///< number of face indices in this mesh
		const granny_pnt332_vertex *vertex;	///< pointer to pool of mesh vertices
		Int vertexCount;	///< number of vertices in this mesh.
	};

	virtual ~GrannyPrototypeClass(void)							{ if (m_vertexMaterial) REF_PTR_RELEASE (m_vertexMaterial); if (m_file) GrannyFreeFile(m_file); }						 
	virtual const char *			Get_Name(void)	const			{ return m_name; }	
	virtual int						Get_Class_ID(void) const	{ return RenderObjClass::CLASSID_UNKNOWN; }
	virtual RenderObjClass *	Create(void)					{ return NEW_REF( GrannyRenderObjClass, (*this) ); }
	void	Set_Name(char *name)	{strcpy(m_name,name);}
	void	setBoundingBox(AABoxClass & box)	{m_boundingBox=box;}
	void	setBoundingSphere(SphereClass & sphere)	{m_boundingSphere=sphere;}
	void	setVertexCount(Int vertexCount) {m_vertexCount=vertexCount;}
	void	setIndexCount(Int indexCount)	{m_indexCount=indexCount;}
	void	setMeshCount(Int meshCount)	{m_meshCount=meshCount;}
	void	setMeshData(grannyMeshDesc &meshdesc, Int index) {m_meshData[index]=meshdesc;}
	const	UnsignedInt *getMeshIndexList(int index) const	{ if (index < m_meshCount) return m_meshData[0].index; else return NULL;}
	const	granny_pnt332_vertex *getMeshVertexList(int index) const { if (index < m_meshCount) return m_meshData[0].vertex; else return NULL;}
	const   Int getIndexCount(void)	const {return m_indexCount;}	//return total number of indices in model

private:
	granny_file *m_file;			///<pointer to Granny allocated model data.
	char	m_name[32];				///<prototype name
	AABoxClass m_boundingBox;		///<bounding volume used for picking and frustum culling.
	SphereClass m_boundingSphere;	///<bounding volume used for picking and frustum culling.
	VertexMaterialClass	*m_vertexMaterial; ///<vertex lighting material used for this model.
	Int m_vertexCount;	///< total/maximum number of vertices that will be needed to draw complete model.
	Int m_indexCount;	///< total/maximum number of indices that will be needed to draw the complete model.
	Int m_meshCount;	///< total number of meshes in model - meshes can have different materials
	grannyMeshDesc m_meshData[GRANNY_MAX_MESHES_PER_MODEL];	///<descriptions of all meshes in this model
};

#define MAX_GRANNY_RENDERSTATES	32		//maximum number of render states available to granny models per frame.
#define MAX_VISIBLE_GRANNY_MODELS	200	//maximum number of granny models to render per frame

class GrannyRenderObjSystem
{
public:
	void AddRenderObject(RenderInfoClass & rinfo, GrannyRenderObjClass *robj);	///<buffers model for rendering at frame flush.
	void Flush(void);	///<renders all objects queued up for this frame.
	void queueUpdate(void) {m_doUpdate=TRUE;}	///<tell Granny to update all animations - do this only once per frame.
protected:
	struct RenderStateModelList{
		GrannyRenderObjClass *list;	///<start of Granny models using this renderstate
	};

	RenderStateModelList m_renderStateModelList[MAX_GRANNY_RENDERSTATES];	///<list of render states used during this frame.
	LightEnvironmentClass   m_renderLocalLightEnv[MAX_VISIBLE_GRANNY_MODELS];	///<pool of lighting settings available to granny
	Int m_renderStateCount;	///<number of different render states requested in current frame.
	Int m_renderObjectCount;	///<number of granny models to render this frame.
	Bool m_doUpdate;	///<flag to indicate if animation time needs to be updated this frame.
};

extern GrannyRenderObjSystem *TheGrannyRenderObjSystem; ///< singleton for drawing all Granny models.

#if 0	///@todo: Will have to implement an optimized granny rendering system
/// System for loading, drawing, and updating Granny models
/**
This system keeps track of all the active granny objects and renders any models
that were submitted in current frame.
*/

class GrannyRenderObjClassSystem
{
	friend class GrannyRenderObjClass;

public:

	GrannyRenderObjClassSystem( void );
	~GrannyRenderObjClassSystem( void );

	void ReleaseResources(void);	///< Release all dx8 resources so the device can be reset.
	void ReAcquireResources(void);  ///< Reacquire all resources after device reset.

	void flush (void);	///<draw all models that were requested for rendering.
	void update(void);	///<update the state of all models.

	void init();	///< pre-allocate resources and initialize granny.
	void shutdown( void );	///< release all pre-allocated resources and kill Granny.

	RenderObjClass *createRenderObj(const char * name);	///< create an instance of the model

protected:
	DX8VertexBufferClass		*m_vertexBuffer;	///<vertex buffer used to draw all tracks
	DX8IndexBufferClass			*m_indexBuffer;	///<indices defining triangles in maximum length track
	VertexMaterialClass	  	  *m_vertexMaterialClass;	///< vertex lighting material
	ShaderClass m_shaderClass; ///<shader or rendering state for heightmap

	GrannyRenderObjClass *m_usedModules;	///<active objects being rendered in the scene
	GrannyRenderObjClass *m_freeModules;	//<unused modules that are free to use again

};  // end class GrannyRenderObjClassSystem

extern GrannyRenderObjClassSystem *TheGrannyRenderObjClassSystem; ///< singleton for track drawing system.

#endif	//end of GrannyRenderObjClassSystem

#endif	//INCLUDE_GRANNY_IN_BUILD

#endif  // end __W3DGRANNY_H_

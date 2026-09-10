#include <functional>
#include <functional>
import Graphics.Frame.RenderClock;
import Graphics.Scene.Views.CameraMatrices;
import Graphics.Materials.MeshTextureMapping;
import Assets.Images.PixelEncoding;
#include "WWMath/matrix4.h"
import Graphics.Scene.Props.Renderer;
import Assets.Adapters.W3D.Chunks;
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

;////////////////////////////////////////////////////////////////////////////////
;//																																						 //
;//  (c) 2001-2003 Electronic Arts Inc.																				 //
;//																																						 //
;////////////////////////////////////////////////////////////////////////////////

// FILE: W3DTextureShadow.cpp ///////////////////////////////////////////////////////////
//
// Texture based shadow representation.
//
// Author: Mark Wilczynski, February 2002
//
//
///////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "WWLib/always.h"
#include "WWLib/hash.h"
#include "GameClient/View.h"
#include "W3DDevice/GameClient/W3DCamera.h"

#include "W3DDevice/GameClient/W3DHierarchyRenderObject.h"
#include "W3DDevice/GameClient/W3DMeshRenderObject.h"
#include "W3DDevice/GameClient/W3DMeshResource.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"

#include "Lib/BaseType.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/W3DProjectedShadow.h"
#include "Common/Debug.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"
#include "GameClient/Drawable.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DGraphicsResources.h"
#include "W3DDevice/GameClient/W3DObjectGraphics.h"
#include <vector>
#include <array>
#include <cstdint>
#include <cstring>
import Graphics.Scene.Shadows.Projected;
import Graphics.Scene.Shadows.ProjectedCapture;
import Graphics.Materials.TextureProjector;
import Graphics.Scene.Props.Material;
import Graphics.Materials.ProceduralPass;
import Graphics.Frame.Runtime;


/** @todo: We're going to have a pool of a couple rendertargets to use
in rare cases when dynamic shadows need to be generated.  Maybe we can
even get away with a single one that gets used immediately to render, then
recycled.  For most of the objects, we need to have a static texture that
is reused for all instances on the level.

Need to add support for loading textures from disk instead of generating them in
code.  Could allow for a single non-distinct blob to be used for everything.

Instead of projecting onto arbitrary geometry, could allow for terrain only.
Maybe project onto a deformed terrain patch that molds to trays/bibs.
*/

#define DEFAULT_RENDER_TARGET_WIDTH			512
#define DEFAULT_RENDER_TARGET_HEIGHT		512

namespace
{
Graphics::TextureProjectorFit Build_Shadow_Fit(W3DRenderObject &object, const Vector3 &light_position)
{
	AABoxClass object_box;
	object.Get_Obj_Space_Bounding_Box(object_box);
	Graphics::TextureProjectorBounds bounds;
	bounds.center = {object_box.Center.X, object_box.Center.Y, object_box.Center.Z};
	bounds.extent = {object_box.Extent.X, object_box.Extent.Y, object_box.Extent.Z};
	Graphics::Matrix4x4 object_transform = Graphics::Matrix4x4::Identity();
	const Matrix3D &transform = object.Get_Transform();
	for (unsigned row = 0; row < 3; ++row)
		for (unsigned column = 0; column < 4; ++column)
			object_transform.values[row * 4 + column] = transform[row][column];
	return Graphics::Fit_Perspective_Texture_Projector(bounds, object_transform,
		{light_position.X, light_position.Y, light_position.Z});
}

void Configure_Shadow_Camera(W3DCamera &camera, const Graphics::TextureProjectorFit &fit,
	unsigned texture_width, unsigned texture_height)
{
	Matrix3D transform;
	for (unsigned row = 0; row < 3; ++row)
		for (unsigned column = 0; column < 4; ++column)
			transform[row][column] = fit.camera_transform.values[row * 4 + column];
	camera.Set_Transform(transform);
	camera.Set_Projection_Type(W3DCamera::PERSPECTIVE);
	camera.Set_View_Plane(fit.horizontal_fov, fit.vertical_fov);
	camera.Set_Clip_Planes(0.01f, fit.fitting_clip_end);
	const Vector2 viewport_min(1.0f / static_cast<float>(texture_width),
		1.0f / static_cast<float>(texture_height));
	const Vector2 viewport_max((static_cast<float>(texture_width) - 1.0f)
		/ static_cast<float>(texture_width),
		(static_cast<float>(texture_height) - 1.0f) / static_cast<float>(texture_height));
	camera.Set_Viewport(viewport_min, viewport_max);
}
}

W3DProjectedShadowManager *TheW3DProjectedShadowManager=nullptr;	//global singleton
ProjectedShadowManager	*TheProjectedShadowManager;				//global singleton with simpler interface.
extern const FrustumClass *shadowCameraFrustum;	//defined in W3DShadow.
struct SHADOW_DECAL_VERTEX
{
    float x,y,z;
    UnsignedInt diffuse;
    float u,v;
};

struct W3DProjectedShadowManager::GraphicsState
{
    std::vector<SHADOW_DECAL_VERTEX> vertices;
    std::vector<std::uint32_t> indices;
    Graphics::PropMeshHandle decal_mesh;
    Graphics::PropMeshHandle projection_mesh;
    W3DCamera* camera = nullptr;

    void Release()
    {
        auto& renderer = Graphics::Get_Prop_Renderer();
        renderer.Destroy_Mesh(decal_mesh);
        renderer.Destroy_Mesh(projection_mesh);
        decal_mesh = {}; projection_mesh = {};
        vertices.clear(); indices.clear();
    }
};


class W3DShadowTexture;	//forward reference
class W3DShadowTextureManager;	//forward reference


/** This class will manage shadow textures for each render object.  Shadow textures may
be based on render geometry but don't need to be.  This allows lower detail 'blob' textures
to be substituted to improve performance.*/
class W3DShadowTextureManager
{
public:
	W3DShadowTextureManager();
	~W3DShadowTextureManager();

	int			 		createTexture(W3DRenderObject *robj, const char *name);
	W3DShadowTexture *		getTexture(const char * name);
	W3DShadowTexture *		peekTexture(const char * name);
	Bool					addTexture(W3DShadowTexture *new_texture);
	void			 		freeAllTextures();
	void					invalidateCachedLightPositions();

	void					registerMissing( const char * name );
	Bool					isMissing( const char * name );
	void					resetMissing();

private:

	HashTableClass	*	texturePtrTable;
	HashTableClass	*	missingTextureTable;

	friend	class		W3DShadowTextureManagerIterator;
};

class W3DShadowTexture : public RefCountClass, public	HashableClass
{

	public:

		W3DShadowTexture()
		{	m_lastLightPosition.Set(0,0,0); m_lastObjectOrientation.Make_Identity();
			m_shadowUV[0].Set(1.0f,0.0f,0.0f);	//u runs along world x axis
			m_shadowUV[1].Set(0.0f,-1.0f,0.0f);	//v runs along world -y axis
		}
		virtual ~W3DShadowTexture() override { REF_PTR_RELEASE(m_texture);}

		virtual	const char * Get_Key() override { return m_namebuf;	}

		Int init (W3DRenderObject *robj);

		const char *		Get_Name() const	{ return m_namebuf;}
		void				Set_Name(const char *name)
		{
			strlcpy(m_namebuf,name,sizeof(m_namebuf));
		}
		W3DTextureHandle	*getTexture()	{ return m_texture;}
		void					 setTexture(W3DTextureHandle *texture)	{m_texture = texture;}
		void					 setLightPosHistory(Vector3 &pos) {m_lastLightPosition=pos;}	///<updates the last position of light
		Vector3&			 getLightPosHistory() {return m_lastLightPosition;}
		void					 setObjectOrientationHistory(Matrix3x3 &mat) {m_lastObjectOrientation=mat;}	///<updates the last position of light
		Matrix3x3&			 getObjectOrientationHistory() {return m_lastObjectOrientation;}
		SphereClass&	 getBoundingSphere()	{return m_areaEffectSphere;}
		AABoxClass&		 getBoundingBox()		{return m_areaEffectBox;}
		void	 setBoundingSphere(SphereClass &sphere)	{m_areaEffectSphere=sphere;}
		void	 setBoundingBox(AABoxClass &box)		{m_areaEffectBox=box;}
		void	 updateBounds(Vector3 &lightPos, W3DRenderObject *robj);	///<update extent of shadow
		void	 setDecalUVAxis(Vector3 &u, Vector3 &v)	{ m_shadowUV[0]=u; m_shadowUV[1]=v;}
		void	 getDecalUVAxis(Vector3 *u, Vector3 *v)	{ *u=m_shadowUV[0]; *v=m_shadowUV[1];}

	private:

		char m_namebuf[2*Assets::W3D::W3DNameLength];	///<name of model hierarchy

		W3DTextureHandle *m_texture; ///<texture holding the shadow for this renderobject
		Vector3		m_lastLightPosition;		///<position of light source at time of last texture update.
		Matrix3x3	m_lastObjectOrientation;	///<orientation of shadow casting object when texture was generated.
		AABoxClass	m_areaEffectBox;			///<boundary defining object-space volume affected by shadow.
		SphereClass	m_areaEffectSphere;			///<boundary defining object-space volume affected by shadow.
		Vector3		m_shadowUV[2];		///world-space vectors defining the u and v texture coordinate axis.
};

/*
** An Iterator to get to all loaded W3DShadowGeometries in a W3DShadowGeometryManager
*/
class W3DShadowTextureManagerIterator : public HashTableIteratorClass {
public:
	W3DShadowTextureManagerIterator( W3DShadowTextureManager & manager ) : HashTableIteratorClass( *manager.texturePtrTable ) {}
	W3DShadowTexture * getCurrentTexture() { 	return (W3DShadowTexture *)Get_Current();}
};


/******************** Start of W3DProjectedShadowManager implementation ***********************/
W3DProjectedShadowManager::W3DProjectedShadowManager()
{
    m_graphics = new GraphicsState;
    m_dynamicRenderTarget = nullptr;
    m_renderTargetHasAlpha = FALSE;
	m_shadowList = nullptr;
	m_decalList = nullptr;
	m_numDecalShadows = 0;
	m_numProjectionShadows  = 0;
	m_W3DShadowTextureManager = nullptr;
	m_shadowCamera = nullptr;
	m_shadowContext= nullptr;
	m_drawEdgeX = 0;
	m_drawEdgeY = 0;
	m_drawStartX = 0;
	m_drawStartY = 0;
}

W3DProjectedShadowManager::~W3DProjectedShadowManager()
{

	ReleaseResources();
    delete m_graphics;
	m_dynamicRenderTarget = nullptr;
	m_renderTargetHasAlpha = FALSE;
	delete m_shadowContext;
	REF_PTR_RELEASE(m_shadowCamera);
	delete m_W3DShadowTextureManager;
	m_W3DShadowTextureManager = nullptr;

	//all shadows should be freed up at this point but check anyway
	DEBUG_ASSERTCRASH(m_shadowList == nullptr, ("Destroy of non-empty projected shadow list"));
	DEBUG_ASSERTCRASH(m_decalList == nullptr, ("Destroy of non-empty projected decal list"));
}

void W3DProjectedShadowManager::reset()
{

	DEBUG_ASSERTCRASH(m_shadowList == nullptr, ("Reset of non-empty projected shadow list"));
	DEBUG_ASSERTCRASH(m_decalList == nullptr, ("Reset of non-empty projected decal list"));

	m_W3DShadowTextureManager->freeAllTextures();

}

Bool W3DProjectedShadowManager::init()
{
	m_W3DShadowTextureManager = NEW W3DShadowTextureManager;
	m_shadowCamera = NEW_REF( W3DCamera, () );
	m_shadowContext= NEW W3DRenderContext(*m_shadowCamera);
	m_shadowContext->light_environment = &m_shadowLightEnv;

	return TRUE;
}


Bool W3DProjectedShadowManager::ReAcquireResources()
{
	//grab assets which don't survive a device reset and need
	//to be present for duration of game.

	///@todo: We should allocate our render target pool here.

	DEBUG_ASSERTCRASH(m_dynamicRenderTarget == nullptr, ("Acquire of existing shadow render target"));

	m_renderTargetHasAlpha=TRUE;
    m_dynamicRenderTarget=new W3DTextureHandle(DEFAULT_RENDER_TARGET_WIDTH,DEFAULT_RENDER_TARGET_HEIGHT,
        Assets::PixelEncoding::BGRA8,MIP_LEVELS_1,W3DTextureHandle::POOL_DEFAULT,true,false);
    if (!m_dynamicRenderTarget->Is_Initialized()) REF_PTR_RELEASE(m_dynamicRenderTarget);

    return m_dynamicRenderTarget != nullptr && Graphics::Shared_Frame_Device() != nullptr;
}

void W3DProjectedShadowManager::ReleaseResources()
{
    if (m_W3DShadowTextureManager) invalidateCachedLightPositions();
    REF_PTR_RELEASE(m_dynamicRenderTarget);
    m_graphics->Release();
}

void W3DProjectedShadowManager::invalidateCachedLightPositions()
{
	m_W3DShadowTextureManager->invalidateCachedLightPositions();
}

void W3DProjectedShadowManager::updateRenderTargetTextures()
{
	///@todo: Don't update texture for shadows that can't be seen!!

	W3DProjectedShadow *shadow;
	if (!m_shadowList)
		return;	//there are no shadows to render.

	if (!TheGlobalData->m_useShadowDecals)
		return;

	if (m_numProjectionShadows)
	for( shadow = m_shadowList; shadow; shadow = shadow->m_next )
	{	//decals don't need any updates on a per-frame basis since
		//the image never changes.
		if (shadow->m_type != SHADOW_DECAL)
			shadow->update();
	}
}

///Renders shadow on part of terrain covered by world-space bounding box.
Int W3DProjectedShadowManager::renderProjectedTerrainShadow(W3DProjectedShadow *shadow, AABoxClass &box)
{
    auto *device = Graphics::Shared_Frame_Device();
    if (!device || !TheTerrainRenderObject || !m_graphics->camera || shadow == nullptr) return 0;
    auto *mapping = shadow->getShadowMapping();
    auto *material = shadow->getShadowMaterial();
    auto *shadow_texture = shadow->m_shadowTexture[0] != nullptr
        ? shadow->m_shadowTexture[0]->getTexture() : nullptr;
    if (mapping == nullptr || material == nullptr || shadow_texture == nullptr
        || !shadow_texture->Ensure_Render_Backend_Texture()) return 0;
    auto* hmap = TheTerrainRenderObject->getMap();
    const int startX = __max(0, REAL_TO_INT_FLOOR((box.Center.X-box.Extent.X)/MAP_XY_FACTOR));
    const int startY = __max(0, REAL_TO_INT_FLOOR((box.Center.Y-box.Extent.Y)/MAP_XY_FACTOR));
    const int endX = __min(hmap->getXExtent()-1, REAL_TO_INT_CEIL((box.Center.X+box.Extent.X)/MAP_XY_FACTOR));
    const int endY = __min(hmap->getYExtent()-1, REAL_TO_INT_CEIL((box.Center.Y+box.Extent.Y)/MAP_XY_FACTOR));
    if (endX <= startX || endY <= startY) return 0;
    const int width = endX-startX+1;
    std::vector<Graphics::PropVertex> vertices(width*(endY-startY+1));
    std::vector<std::uint32_t> indices;
    indices.reserve((endX-startX)*(endY-startY)*6);
    for (int y=startY;y<=endY;++y) for (int x=startX;x<=endX;++x) {
        auto& vertex = vertices[(y-startY)*width+x-startX];
        vertex.position = {float(x)*MAP_XY_FACTOR,float(y)*MAP_XY_FACTOR,float(hmap->getHeight(x,y))*MAP_HEIGHT_SCALE};
        Graphics::Apply_Prop_Material(vertex, material->parameters);
        if (x==endX || y==endY) continue;
        const std::uint32_t i=(y-startY)*width+x-startX;
        UnsignedByte alpha[4]; float u[4],v[4]; Bool flip;
        hmap->getAlphaUVData(x,y,u,v,alpha,&flip);
        if (flip) indices.insert(indices.end(),{i+1,i+width,i,i+1,i+1+width,i+width});
        else indices.insert(indices.end(),{i,i+1+width,i+width,i,i+1,i+1+width});
    }
    auto& renderer = Graphics::Get_Prop_Renderer();
    auto& mesh = m_graphics->projection_mesh;
    if (!mesh.Is_Valid()) mesh=renderer.Create_Mesh(vertices,indices);
    else if (!renderer.Update_Mesh(mesh,vertices,indices)) return 0;
    Graphics::PropParameters parameters;
    parameters.view_projection = Make_Surface_Parameters(*m_graphics->camera).view_projection;
    Matrix3D view; m_graphics->camera->Get_View_Matrix(&view);
    const Matrix4x4 view4(view);
    std::memcpy(parameters.view.data(),&view4,sizeof(view4));
    parameters.primary_gradient = 1.0f;
    parameters.alpha_cutoff = 96.0f/255.0f;
    const std::array<Graphics::RHITextureHandle,2> textures{
        Resolve_Graphics_Texture(shadow_texture), Graphics::RHITextureHandle{}};
    Graphics::Extract_Mesh_Texture_Mappings(parameters, material, Graphics::Get_Render_Clock().Sync_Time(),
        Graphics::Get_Camera_Matrices().view.values, Graphics::Get_Camera_Matrices().projection.values);
    parameters.secondary_texture = 0.0f;
    const bool drawn = Graphics::Draw_Projected_Shadow(renderer,device->Immediate_Command_List(),mesh,parameters,textures);
    return drawn ? 1 : 0;
}

void W3DProjectedShadowManager::flushDecals(W3DShadowTexture *texture, ShadowType type)
{
    auto& state = *m_graphics;
    auto* device = Graphics::Shared_Frame_Device();
    if (state.indices.empty()) return;
    auto* source = texture ? texture->getTexture() : nullptr;
    const auto image = Resolve_Graphics_Texture(source);
    if (device && state.camera && image.Is_Valid()) {
        std::vector<Graphics::PropVertex> vertices(state.vertices.size());
        for (std::size_t i=0;i<vertices.size();++i) {
            const auto& src=state.vertices[i]; auto& dst=vertices[i];
            dst.position={src.x,src.y,src.z}; dst.uv={src.u,src.v};
            dst.color={float((src.diffuse>>16)&255)/255,float((src.diffuse>>8)&255)/255,
                float(src.diffuse&255)/255,float(src.diffuse>>24)/255};
        }
        auto& renderer = Graphics::Get_Prop_Renderer();
        if (!state.decal_mesh.Is_Valid()) state.decal_mesh=renderer.Create_Mesh(vertices,state.indices);
        else renderer.Update_Mesh(state.decal_mesh,vertices,state.indices);
        Graphics::PropParameters parameters;
        parameters.view_projection=Make_Surface_Parameters(*state.camera).view_projection;
        const auto blend = type==SHADOW_ALPHA_DECAL ? Graphics::DecalBlend::Alpha
            : type==SHADOW_ADDITIVE_DECAL ? Graphics::DecalBlend::Additive : Graphics::DecalBlend::Multiply;
        Graphics::Draw_Decal(renderer,device->Immediate_Command_List(),state.decal_mesh,parameters,image,blend);
    }
    state.vertices.clear(); state.indices.clear();
}

#define BRIDGE_OFFSET_FACTOR 1.5f
/**Decals have a low poly count so its better to render large numbers at once.  This system will queue them
up until the buffers fill up.  It will then flush the buffer (draw decals) and be ready for new decals.  This
is an optimized system that only uses the render objects bounding box to determine shadow visibility.
*/
void W3DProjectedShadowManager::queueDecal(W3DProjectedShadow *shadow)
{
	int i,j,k;
	Vector3 hmapVertex,objPos;
	AABoxClass box;
	Matrix3D   objXform(1);
	Real cx,cy,dx,dy;
	Real mapScaleInv=1.0f/MAP_XY_FACTOR;
	static Vector3 objCenter(0,0,0);
	Vector3 uVector,vVector;
	Real uOffset,vOffset,vecLength;
	Int borderSize;
	W3DRenderObject *robj=shadow->m_robj;
	Real layerHeight=0;

	if (TheTerrainRenderObject)
	{

		if (Graphics::Shared_Frame_Device() == nullptr)
			return;

		WorldHeightMap *hmap=TheTerrainRenderObject->getMap();
		borderSize=hmap->getBorderSizeInline();
		if (robj)
		{
			objPos=robj->Get_Position();
			objXform=robj->Get_Transform();
			if (robj->Get_User_Data())
			{
				Drawable *draw=((DrawableInfo *)robj->Get_User_Data())->m_drawable;
				const Object *object=draw->getObject();
				PathfindLayerEnum objectLayer;
				if (object && (objectLayer=object->getLayer()) != LAYER_GROUND)
				{	//check if object that this decal belongs to is not on the ground (bridge?)
					layerHeight=BRIDGE_OFFSET_FACTOR+TheTerrainLogic->getLayerHeight(objPos.X,objPos.Y,objectLayer);
				}
			}
		}
		else
		{	//no render object so use shadow's local position and default orientation
			objPos.Set(shadow->m_x,shadow->m_y,shadow->m_z);
			objXform.Rotate_Z(shadow->m_localAngle);
		}

		//Find size of heightmap sub-rectangle affected by shadow
		//If user supplied size values, ignore bounding box

		objPos.Z=0.0f;	//we don't care about object height since shadows project top-down

		uVector=objXform.Get_X_Vector();

		uVector.Z=0.0f;
		vecLength=uVector.Length();
		if (vecLength != 0.0f)	//prevent divide by zero
		{	uVector *= 1.0f/vecLength;
			vVector = uVector;
			vVector.Rotate_Z(-1.0f,0.0f);	//rotate u vector by -90 degrees to get v vector.
		}
		else
		{
			vVector=objXform.Get_Y_Vector();
			vVector.Z=0.0f;
			vecLength=vVector.Length();
			if (vecLength != 0.0f)	//prevent divide by zero
				vVector *= 1.0f/vecLength;
			else
				vVector.Set(0.0f,-1.0f,0.0f); //Point uvector in default direction

			uVector = vVector;
			uVector.Rotate_Z(1.0f,0.0f);	//rotate v vector by 90 degrees to get u vector.
		}

		//Compute bounding box of projection
		Vector3 boxCorners[4];	//top-left, top-right, bottom-right, bottom-left
		dx = shadow->m_decalSizeX;
		dy = shadow->m_decalSizeY;
		Vector3 left_x=-dx * (uVector * (0.5f + shadow->m_decalOffsetU));
		Vector3 right_x = dx * (uVector * (0.5f - shadow->m_decalOffsetU));
		Vector3 top_y = -dy * (vVector * (0.5f + shadow->m_decalOffsetV));
		Vector3 bottom_y = dy * (vVector * (0.5f - shadow->m_decalOffsetV));
		///@todo: Optimize this bounding box calculation to use transformed extents
		//Also skip bounding box calculation if object has not moved.
		boxCorners[0] = left_x + top_y;
		boxCorners[1] = right_x + top_y;
		boxCorners[2] = right_x + bottom_y;
		boxCorners[3] = left_x + bottom_y;

		Real min_x,max_x,min_y,max_y;
		max_x=min_x=boxCorners[0].X;
		max_y=min_y=boxCorners[0].Y;

		for (Int bi=1; bi<4; bi++)
		{	max_x = __max(max_x,boxCorners[bi].X);
			min_x = __min(min_x,boxCorners[bi].X);
			max_y = __max(max_y,boxCorners[bi].Y);
			min_y = __min(min_y,boxCorners[bi].Y);
		}

		uVector *= shadow->m_oowDecalSizeX;
		vVector *= shadow->m_oowDecalSizeY;
		uOffset = shadow->m_decalOffsetU + 0.5f;
		vOffset = shadow->m_decalOffsetV + 0.5f;

/*		{	//This version will stretch to fit orientation of object
			///@todo: Most of the values below can be cached in shadow object
			shadow->m_robj->Get_Obj_Space_Bounding_Box(box);
			decalSizeX=box.Extent.X*2.0f;	//use local space bounding box to determine shadow size
			decalSizeY=box.Extent.Y*2.0f;
//			box=shadow->m_robj->Get_Bounding_Box();	//get world-space bounding box
			objPos = box.Center;
			box.Init(objCenter,Vector3(decalSizeX*0.5f,decalSizeY*0.5f,1.0f));
			box.Transform(objXform);	//transform box from object space to world space
			box.Translate(objPos=objXform.Rotate_Vector(objPos));
			objPos += shadow->m_robj->Get_Position();
		}
		*/
//	  Experimental code to try and get a better fitting bounding box around shadow
	/*  Experimental code to try and get a better fitting bounding box around shadow
	{	//use the object's bounding box to determine shadow extent
		///@todo: Most of the values below can be cached in shadow object
		uVector=objXform.Get_X_Vector();
		uVector.Z=0;
		uVector.Normalize();

		vVector=objXform.Get_Y_Vector();	//invert direction since v axis runs right relative to u.
		vVector.Z=0;
		vVector.Normalize();

		shadow->m_robj->Get_Obj_Space_Bounding_Box(box);
		decalSizeX = box.Extent.X * 2.0f;
		decalSizeY = box.Extent.Y * 2.0f;

		Real newExtentX = fabs(uVector * box.Extent) + fabs(uVector * box.Center);	//get new extent for object orientation
		Real newExtentY = fabs(vVector * box.Extent) + fabs(vVector * box.Center);  //get new extent for object orientation

		objPos += uVector * box.Center.X;
		objPos += vVector * box.Center.Y;
//			objPos.Y = objPos.Y + vVector * box.Center;
		objPos.Z = 0.0f;

		//set new oriented bounding box extents
		box.Extent.Set(newExtentX, newExtentY, 0);
		box.Center.Set(objPos.X,objPos.Y,objPos.Z);
	}
	*/

		cx=box.Center.X;
		cy=box.Center.Y;
		dx=box.Extent.X;
		dy=box.Extent.Y;

		//Get terrain cell index for area with shadow
		Int startX=REAL_TO_INT_FLOOR(((objPos.X+min_x)*mapScaleInv)) + borderSize;
		Int endX=REAL_TO_INT_CEIL(((objPos.X+max_x)*mapScaleInv)) + borderSize;
		Int	startY=REAL_TO_INT_FLOOR(((objPos.Y+min_y)*mapScaleInv)) + borderSize;
		Int endY=REAL_TO_INT_CEIL(((objPos.Y+max_y)*mapScaleInv)) + borderSize;

		startX = __max(startX,m_drawStartX);
		startX = __min(startX,m_drawEdgeX);
		startY = __max(startY,m_drawStartY);
		startY = __min(startY,m_drawEdgeY);

		endX = __max(endX,m_drawStartX);
		endX = __min(endX,m_drawEdgeX);
		endY = __max(endY,m_drawStartY);
		endY = __min(endY,m_drawEdgeY);

		//Check if decal too large to fit inside 65536 index buffer.
		//try clipping each direction to < 104 since that's more than
		//enough to cover typical map.
		Int numExtraX=(endX - startX+1)-104;
		if (numExtraX > 0)
		{	//figure out how much to clip out at each edge of decal
			Int numStartExtraX=REAL_TO_INT_FLOOR((float)numExtraX/2.0f);
			Int numEdgeExtraX=numExtraX-numStartExtraX;
			startX+=numStartExtraX;
			endX-=numEdgeExtraX;
		}
		Int numExtraY=(endY - startY+1)-104;
		if (numExtraY > 0)
		{
			Int numStartExtraY=REAL_TO_INT_FLOOR((float)numExtraY/2.0f);
			Int numEdgeExtraY=numExtraY-numStartExtraY;

			startY+=numStartExtraY;
			endY-=numEdgeExtraY;
		}

		Int vertsPerRow=endX - startX+1;	//number of cells +1
		Int vertsPerColumn=endY-startY+1;	//number of cells +1

		if (vertsPerRow <= 1 || vertsPerColumn <= 1)
			return;	//nothing to render

		Int numVerts = vertsPerRow *vertsPerColumn;	//number of terrain vertices
		Int numIndex=(endX - startX) * (endY-startY)*6;	//6 indices per terrain cell (2 triangles).

        if (m_graphics->vertices.size()+numVerts > 32768 || m_graphics->indices.size()+numIndex > 65536)
            flushDecals(shadow->m_shadowTexture[0],shadow->m_type);
        const auto nShadowDecalVertsInBatch = static_cast<std::uint32_t>(m_graphics->vertices.size());
        const auto indexStart = m_graphics->indices.size();
        m_graphics->vertices.resize(m_graphics->vertices.size()+numVerts);
        m_graphics->indices.resize(indexStart+numIndex);
        auto* pvVertices = m_graphics->vertices.data()+nShadowDecalVertsInBatch;
        auto* pvIndices = m_graphics->indices.data()+indexStart;

		//code to deal with rotated shadows based on sun direction, fix this later.  For now shadow rotates with object rotation.
		//shadow->m_shadowTexture[0]->getDecalUVAxis(&uVector,&vVector);
		//uVector *= 20.0f/dx;//shadow->m_decalRadius;	//scale texture to fit object
		//vVector *= 20.0f/dy;//1.2f;//shadow->m_decalRadius;	//scale texture to fit object
/*		uVector=objXform.Get_X_Vector();
		uVector.Normalize();
		uVector /= decalSizeX + (1.0f+4.0f/64.0f);	//texture has 1 pixel transparent border so we strtech up to make sure solid pixels reach extent..
		vVector=objXform.Get_Y_Vector() * -1.0f;	//invert direction since v axis runs right relative to u.
		vVector.Normalize();
		vVector /= decalSizeY + (1.0f+4.0f/64.0f);
		*/
		DEBUG_ASSERTCRASH(numVerts == ((endY-startY+1)*(endX-startX+1)), ("queueDecal VB size mismatch"));

		if(pvVertices)
		{
			if (layerHeight)
				for (j=startY; j <= endY; j++)
				{
					hmapVertex.Y=(float)(j-borderSize) * MAP_XY_FACTOR;

					for (i=startX; i <= endX; i++)
					{
						hmapVertex.X=(float)(i-borderSize)*MAP_XY_FACTOR;
						hmapVertex.Z=__max((float)hmap->getHeight(i,j)*MAP_HEIGHT_SCALE,layerHeight);
						pvVertices->x=hmapVertex.X;
						pvVertices->y=hmapVertex.Y;
						pvVertices->z=hmapVertex.Z;
						pvVertices->diffuse=shadow->m_diffuse;
						pvVertices->u=Vector3::Dot_Product(uVector, (hmapVertex-objPos))+uOffset;
						pvVertices->v=Vector3::Dot_Product(vVector, (hmapVertex-objPos))+vOffset;
						pvVertices++;
					}
				}
			else
			//insert each cell's bottom/left edge vertex
			for (j=startY; j <= endY; j++)
			{
				hmapVertex.Y=(float)(j-borderSize) * MAP_XY_FACTOR;

				for (i=startX; i <= endX; i++)
				{
					hmapVertex.X=(float)(i-borderSize)*MAP_XY_FACTOR;
					hmapVertex.Z=(float)hmap->getHeight(i,j)*MAP_HEIGHT_SCALE+0.01f * MAP_XY_FACTOR;
					pvVertices->x=hmapVertex.X;
					pvVertices->y=hmapVertex.Y;
					pvVertices->z=hmapVertex.Z;
					pvVertices->diffuse=shadow->m_diffuse;
					pvVertices->u=Vector3::Dot_Product(uVector, (hmapVertex-objPos))+uOffset;
					pvVertices->v=Vector3::Dot_Product(vVector, (hmapVertex-objPos))+vOffset;
					pvVertices++;
				}
			}
		}

		if(pvIndices)
		{	//fill each cell's vertex indices
			Int rowStart;
			for (j=startY,rowStart=0; j<endY; j++,rowStart+=vertsPerRow)
			{
				for (i=rowStart,k=startX; k<endX; i++,k++)
				{
					///@todo: fix the winding order in heightmap to be in strip order like above!
					if (hmap->getFlipState(k,j))
					{	pvIndices[0]=i+1+nShadowDecalVertsInBatch;
						pvIndices[1]=i+vertsPerRow+nShadowDecalVertsInBatch;
						pvIndices[2]=i+nShadowDecalVertsInBatch;
						pvIndices[3]=i+1+nShadowDecalVertsInBatch;
						pvIndices[4]=i+1+vertsPerRow+nShadowDecalVertsInBatch;
						pvIndices[5]=i+vertsPerRow+nShadowDecalVertsInBatch;
					}
					else
					{	pvIndices[0]=i+nShadowDecalVertsInBatch;
						pvIndices[1]=i+1+vertsPerRow+nShadowDecalVertsInBatch;
						pvIndices[2]=i+vertsPerRow+nShadowDecalVertsInBatch;
						pvIndices[3]=i+nShadowDecalVertsInBatch;
						pvIndices[4]=i+1+nShadowDecalVertsInBatch;
						pvIndices[5]=i+1+vertsPerRow+nShadowDecalVertsInBatch;
					}
					pvIndices += 6;
				}
			}
		}


		return;
	}

}

/**Simpler/faster decal system that always uses 2 triangles that are roughly oriented
to terrain.  Since they are not projected onto terrain, there may be clipping
artifacts in certain situations.
TODO: Too much clipping.  Need to check terrain heights at all 4 corners and adjust tilt to match*/
///@todo: We should have a pre-made static filled index buffer since we always send down 2 triangles.
void W3DProjectedShadowManager::prepareShadows()
{
	if (!TheTerrainRenderObject)
		return;

	WorldHeightMap *hmap=TheTerrainRenderObject->getMap();

	if (!hmap)
		return;

	//Find extents of visible terrain
	m_drawEdgeY=hmap->getDrawOrgY()+hmap->getDrawHeight()-1;
	m_drawEdgeX=hmap->getDrawOrgX()+hmap->getDrawWidth()-1;
	if (m_drawEdgeX > (hmap->getXExtent()-1))
		m_drawEdgeX = hmap->getXExtent()-1;
	if (m_drawEdgeY > (hmap->getYExtent()-1))
		m_drawEdgeY = hmap->getYExtent()-1;
	m_drawStartX=hmap->getDrawOrgX();
	m_drawStartY=hmap->getDrawOrgY();
}

Int W3DProjectedShadowManager::renderShadows(W3DRenderContext & rinfo)
{
	Int projectionCount=0;

	if (!TheTerrainRenderObject)
		return projectionCount;

	if (!m_shadowList && !m_decalList)
		return	projectionCount;	//there are no shadows to render.

	W3DProjectedShadow *shadow;
	static AABoxClass aaBox;
	static SphereClass sphere;

    m_graphics->camera = &rinfo.Camera;
    m_graphics->vertices.clear(); m_graphics->indices.clear();

	if (TheGlobalData->m_useShadowDecals)
	{
		// Render the object


		//keep track of active decal texture so we can render all decals at once.
		W3DShadowTexture *lastShadowDecalTexture=nullptr;
		ShadowType lastShadowType = SHADOW_NONE;

		for( shadow = m_shadowList; shadow; shadow = shadow->m_next )
		{
			if (shadow->m_isEnabled && !shadow->m_isInvisibleEnabled)
			{
				if (shadow->m_shadowTexture[0] == nullptr ||
					shadow->m_shadowTexture[0]->getTexture() == nullptr)
				{
					continue;
				}

				if (shadow->m_type & SHADOW_DECAL)
				{
					if (lastShadowDecalTexture == nullptr)
						lastShadowDecalTexture=shadow->m_shadowTexture[0];
					if (lastShadowType == SHADOW_NONE)
						lastShadowType = shadow->m_type;

					if (shadow->m_shadowTexture[0] != lastShadowDecalTexture ||
						shadow->m_type != lastShadowType)
					{	flushDecals(lastShadowDecalTexture,lastShadowType);	//switched to a new texture, need to render polys using last texture.
						lastShadowDecalTexture=shadow->m_shadowTexture[0];
						lastShadowType=shadow->m_type;
					}
					///@todo: may need to fix this if shadows are large enough to be seen while object is not visible
					if (shadow->m_robj->Is_Really_Visible())
					{	//queueSimpleDecal(shadow);
						queueDecal(shadow);	//only draw shadow if casting object is visible
						projectionCount++;
					}
					continue;
				}

				//First test if shadow is visible on screen
				sphere=shadow->m_shadowTexture[0]->getBoundingSphere();
				sphere.Center += shadow->m_robj->Get_Position();

				CollisionMath::OverlapType result=CollisionMath::Overlap_Test(*shadowCameraFrustum,sphere);
				if (result == CollisionMath::OVERLAPPED)
				{	//do a more accurate test against bounding box.
					aaBox=shadow->m_shadowTexture[0]->getBoundingBox();
					aaBox.Translate(shadow->m_robj->Get_Position());	//translate bounding box to world space.
					if (CollisionMath::Overlap_Test(*shadowCameraFrustum,aaBox) == CollisionMath::OUTSIDE)
						continue;
				}
				else
				if (result == CollisionMath::OUTSIDE)
					continue;

				//Shadow is visible on screen.  Figure out which visible objects it may affect.

				//Check if bounding sphere was inside so bounding box never initialized
				if (result == CollisionMath::INSIDE)
				{		aaBox=shadow->m_shadowTexture[0]->getBoundingBox();
						aaBox.Translate(shadow->m_robj->Get_Position());	//translate bounding box to world space.
				}

				if (shadow->m_type == SHADOW_PROJECTION)
				{
					//build inverse camera/view transforms needed for projection
					shadow->updateProjectionParameters(rinfo.Camera.Get_Transform());

					//terrain is always visible and affected by all shadows so must render


					if (renderProjectedTerrainShadow(shadow, aaBox))
						projectionCount++;


				}
			}
		}

		flushDecals(lastShadowDecalTexture,lastShadowType);	//make sure there are not any unrendered decals left over.

	}
	if (m_decalList)
	{
		//keep track of active decal texture so we can render all decals at once.
		W3DShadowTexture *lastShadowDecalTexture=nullptr;
		ShadowType lastShadowType = SHADOW_NONE;

		for( shadow = m_decalList; shadow; shadow = shadow->m_next )
		{
			if (shadow->m_isEnabled && !shadow->m_isInvisibleEnabled)
			{
				if (shadow->m_shadowTexture[0] == nullptr ||
					shadow->m_shadowTexture[0]->getTexture() == nullptr)
				{
					continue;
				}

				if (lastShadowDecalTexture == nullptr)
					lastShadowDecalTexture=shadow->m_shadowTexture[0];
				if (lastShadowType == SHADOW_NONE)
					lastShadowType = shadow->m_type;

				if (shadow->m_shadowTexture[0] != lastShadowDecalTexture ||
					shadow->m_type != lastShadowType)
				{	flushDecals(lastShadowDecalTexture,lastShadowType);	//switched to a new texture, need to render polys using last texture.
					lastShadowDecalTexture=shadow->m_shadowTexture[0];
					lastShadowType=shadow->m_type;
				}
				///@todo: may need to fix this if shadows are large enough to be seen while object is not visible
				if (!(shadow->m_robj && !shadow->m_robj->Is_Really_Visible()))
				{	//queueSimpleDecal(shadow);
					queueDecal(shadow);	//only draw shadow if casting object is visible
					projectionCount++;
				}
			}
		}

		flushDecals(lastShadowDecalTexture,lastShadowType);	//make sure there are not any unrendered decals left over.
	}
	return projectionCount;
}

/** Generic function which can be used to create arbitrary decals that don't have to be used for shadows.
Some examples: Scorch marks, blood, stains, selection/status indicators, etc.*/
Shadow* W3DProjectedShadowManager::addDecal(Shadow::ShadowTypeInfo *shadowInfo)
{
	W3DShadowTexture *st=nullptr;
	ShadowType shadowType=SHADOW_NONE;		/// type of projection
	Bool	allowWorldAlign=FALSE;	/// wrap shadow around world geometry - else align perpendicular to local z-axis.
	Real	decalSizeX=0.0f;
	Real	decalSizeY=0.0f;

	Bool	allowSunDirection=FALSE;
	Char texture_name[ARRAY_SIZE(shadowInfo->m_ShadowName)];

	if (!shadowInfo)
		return nullptr;	//right now we require hardware render-to-texture support

	//simple decal using the premade texture specified.
	//can be always perpendicular to model's z-axis or projected
	//onto world geometry.
	static_assert(ARRAY_SIZE(texture_name) >= ARRAY_SIZE(shadowInfo->m_ShadowName), "Incorrect array size");
	strcpy(texture_name, shadowInfo->m_ShadowName);
	strlcat(texture_name, ".tga", ARRAY_SIZE(texture_name));

	//Check if we previously added a decal using this texture
	st=m_W3DShadowTextureManager->getTexture(texture_name);
	if (st == nullptr)
	{
		//Adding a new decal texture
		W3DTextureHandle *w3dTexture=W3DAssetCatalog::Get_Instance()->Get_Texture(texture_name);
		DEBUG_ASSERTCRASH(w3dTexture != nullptr, ("Could not load decal texture: %s",texture_name));
		if (!w3dTexture)
			return nullptr;

		w3dTexture->Get_Sampling().address[0] = Graphics::RHISamplerAddress::Clamp;
		w3dTexture->Get_Sampling().address[1] = Graphics::RHISamplerAddress::Clamp;
		w3dTexture->Get_Sampling().mipmap = Graphics::SamplingFilter::Disabled;

		st = NEW W3DShadowTexture;	// poolify
		SET_REF_OWNER( st );
		st->Set_Name(texture_name);
		m_W3DShadowTextureManager->addTexture( st );
		st->setTexture(w3dTexture);
	}

	shadowType=shadowInfo->m_type;
	allowWorldAlign=shadowInfo->allowWorldAlign;
	allowSunDirection=shadowInfo->m_type & SHADOW_DIRECTIONAL_PROJECTION;
	decalSizeX=shadowInfo->m_sizeX;
	decalSizeY=shadowInfo->m_sizeY;

	W3DProjectedShadow *shadow = NEW W3DProjectedShadow;	// poolify

	// sanity
	if( shadow == nullptr )
		return nullptr;

	shadow->setRenderObject(nullptr);
	shadow->setTexture(0,st);	///@todo: Fix projected shadows to allow multiple lights
	shadow->m_type = shadowType;		/// type of projection
	shadow->m_allowWorldAlign=allowWorldAlign;	/// wrap shadow around world geometry - else align perpendicular to local z-axis.

	shadow->m_oowDecalSizeX = 1.0f/decalSizeX;	//one over width
	shadow->m_oowDecalSizeY = 1.0f/decalSizeY;	//one over height

	//Prestore some values used during projection to optimize out division.
	shadow->m_decalSizeX = decalSizeX;	//width
	shadow->m_decalSizeY = decalSizeY;	//height

	shadow->m_decalOffsetU=0;
	shadow->m_decalOffsetV=0;

	shadow->m_flags	= allowSunDirection;

	shadow->init();

	// add to our shadow list through the shadow next links, insert next to other shadows using same texture

	W3DProjectedShadow *nextShadow=nullptr,*prevShadow=nullptr;
	for( nextShadow = m_decalList; nextShadow; prevShadow=nextShadow,nextShadow = nextShadow->m_next )
	{
		if (nextShadow->m_shadowTexture[0]==st)
		{	//found start of other shadows using same texture, insert new shadow here.
			shadow->m_next=nextShadow;
			if (prevShadow)
			{	prevShadow->m_next=shadow;
			}
			else
				m_decalList=shadow;
			break;
		}
	}

	if (nextShadow==nullptr)
	{	//shadow with new texture. Add to top of list.
		shadow->m_next = m_decalList;
		m_decalList = shadow;
	}

	updateShadowNumbers(shadow->m_type, +1);
	return shadow;
}

/** Generic function which can be used to create arbitrary decals that follow the renderObject but don't have to be used for shadows.
Some examples: Scorch marks, blood, stains, selection/status indicators, etc.*/
Shadow* W3DProjectedShadowManager::addDecal(W3DRenderObject *robj, Shadow::ShadowTypeInfo *shadowInfo)
{
	W3DShadowTexture *st=nullptr;
	ShadowType shadowType=SHADOW_NONE;		/// type of projection
	Bool	allowWorldAlign=FALSE;	/// wrap shadow around world geometry - else align perpendicular to local z-axis.
	Real	decalSizeX=0.0f;
	Real	decalSizeY=0.0f;
	Real	decalOffsetX=0.0f;
	Real	decalOffsetY=0.0f;

	Bool	allowSunDirection=FALSE;
	Char texture_name[ARRAY_SIZE(shadowInfo->m_ShadowName)];

	if (!robj || !shadowInfo)
		return nullptr;	//right now we require hardware render-to-texture support

	//simple decal using the premade texture specified.
	//can be always perpendicular to model's z-axis or projected
	//onto world geometry.
	static_assert(ARRAY_SIZE(texture_name) >= ARRAY_SIZE(shadowInfo->m_ShadowName), "Incorrect array size");
	strcpy(texture_name, shadowInfo->m_ShadowName);
	strlcat(texture_name, ".tga", ARRAY_SIZE(texture_name));

	//Check if we previously added a decal using this texture
	st=m_W3DShadowTextureManager->getTexture(texture_name);
	if (st == nullptr)
	{
		//Adding a new decal texture
		W3DTextureHandle *w3dTexture=W3DAssetCatalog::Get_Instance()->Get_Texture(texture_name);
		DEBUG_ASSERTCRASH(w3dTexture != nullptr, ("Could not load decal texture: %s",texture_name));
		if (!w3dTexture)
			return nullptr;

		w3dTexture->Get_Sampling().address[0] = Graphics::RHISamplerAddress::Clamp;
		w3dTexture->Get_Sampling().address[1] = Graphics::RHISamplerAddress::Clamp;
		w3dTexture->Get_Sampling().mipmap = Graphics::SamplingFilter::Disabled;

		st = NEW W3DShadowTexture;
		SET_REF_OWNER( st );
		st->Set_Name(texture_name);
		m_W3DShadowTextureManager->addTexture( st );
		st->setTexture(w3dTexture);
	}

	shadowType=shadowInfo->m_type;
	allowWorldAlign=shadowInfo->allowWorldAlign;
	allowSunDirection=shadowInfo->m_type & SHADOW_DIRECTIONAL_PROJECTION;
	decalSizeX=shadowInfo->m_sizeX;
	decalSizeY=shadowInfo->m_sizeY;
	decalOffsetX=shadowInfo->m_offsetX;
	decalOffsetY=shadowInfo->m_offsetY;

	W3DProjectedShadow *shadow = NEW W3DProjectedShadow;

	// sanity
	if( shadow == nullptr )
		return nullptr;

	shadow->setRenderObject(robj);
	shadow->setTexture(0,st);	///@todo: Fix projected shadows to allow multiple lights
	shadow->m_type = shadowType;		/// type of projection
	shadow->m_allowWorldAlign=allowWorldAlign;	/// wrap shadow around world geometry - else align perpendicular to local z-axis.

	AABoxClass box;

	robj->Get_Obj_Space_Bounding_Box(box);

	//Check if app is overriding any of the default texture stretch factors.
	if (!decalSizeX)
		decalSizeX=box.Extent.X*2.0f;//use bounding box to determine size
	shadow->m_oowDecalSizeX = 1.0f/decalSizeX;	//one over width

	if (!decalSizeY)
		decalSizeY=box.Extent.Y*2.0f;//world space distance to stretch full texture
	shadow->m_oowDecalSizeY = 1.0f/decalSizeY;	//one over height

	if (decalOffsetX)
		decalOffsetX=decalOffsetX*shadow->m_oowDecalSizeX;

	if (decalOffsetY)
		decalOffsetY=decalOffsetY*shadow->m_oowDecalSizeY;

	//Prestore some values used during projection to optimize out division.
	shadow->m_decalSizeX = decalSizeX;	//width
	shadow->m_decalSizeY = decalSizeY;	//height

	shadow->m_decalOffsetU= decalOffsetX;
	shadow->m_decalOffsetV= decalOffsetY;

	shadow->m_flags	= allowSunDirection;

	shadow->init();

	// add to our shadow list through the shadow next links, insert next to other shadows using same texture

	W3DProjectedShadow *nextShadow=nullptr,*prevShadow=nullptr;
	for( nextShadow = m_decalList; nextShadow; prevShadow=nextShadow,nextShadow = nextShadow->m_next )
	{
		if (nextShadow->m_shadowTexture[0]==st)
		{	//found start of other shadows using same texture, insert new shadow here.
			shadow->m_next=nextShadow;
			if (prevShadow)
			{	prevShadow->m_next=shadow;
			}
			else
				m_decalList=shadow;
			break;
		}
	}

	if (nextShadow==nullptr)
	{	//shadow with new texture. Add to top of list.
		shadow->m_next = m_decalList;
		m_decalList = shadow;
	}

	updateShadowNumbers(shadow->m_type, +1);
	return shadow;
}

W3DProjectedShadow* W3DProjectedShadowManager::addShadow(W3DRenderObject *robj, Shadow::ShadowTypeInfo *shadowInfo, Drawable *draw)
{
	W3DShadowTexture *st=nullptr;
	static char	defaultDecalName[]={"shadow.tga"};
	ShadowType shadowType=SHADOW_NONE;		/// type of projection
	Bool	allowWorldAlign=FALSE;	/// wrap shadow around world geometry - else align perpendicular to local z-axis.
	Real	decalSizeX=0.0f;
	Real	decalSizeY=0.0f;
	Real	decalOffsetX=0.0f;
	Real	decalOffsetY=0.0f;

	Bool	allowSunDirection=FALSE;
	Char texture_name[ARRAY_SIZE(shadowInfo->m_ShadowName)];


	if (!m_dynamicRenderTarget || !robj || !TheGlobalData->m_useShadowDecals)
		return nullptr;	//right now we require hardware render-to-texture support


	if (shadowInfo)
	{
		//determine what kind of shadow is needed
		if (shadowInfo->m_type==SHADOW_DECAL)
		{		//simple decal using the premade texture specified.
				//can be always perpendicular to model's z-axis or projected
				//onto world geometry.
				if (strlen(shadowInfo->m_ShadowName) <= 1)	//no texture name given, use same as object
				{
					static_assert(ARRAY_SIZE(texture_name) >= ARRAY_SIZE(defaultDecalName), "Incorrect array size");
					strcpy(texture_name, defaultDecalName);
				}
				else
				{
					static_assert(ARRAY_SIZE(texture_name) >= ARRAY_SIZE(shadowInfo->m_ShadowName), "Incorrect array size");
					strcpy(texture_name, shadowInfo->m_ShadowName);
					strlcat(texture_name, ".tga", ARRAY_SIZE(texture_name));
				}

				st=m_W3DShadowTextureManager->getTexture(texture_name);
				if (st == nullptr)
				{
					//need to add this texture without creating it from a real renderobject
					W3DTextureHandle *w3dTexture=W3DAssetCatalog::Get_Instance()->Get_Texture(texture_name);
					DEBUG_ASSERTCRASH(w3dTexture != nullptr, ("Could not load decal texture"));
					if (!w3dTexture)
						return nullptr;

					w3dTexture->Get_Sampling().address[0] = Graphics::RHISamplerAddress::Clamp;
					w3dTexture->Get_Sampling().address[1] = Graphics::RHISamplerAddress::Clamp;
					w3dTexture->Get_Sampling().mipmap = Graphics::SamplingFilter::Disabled;

					st = NEW W3DShadowTexture;	// poolify
					SET_REF_OWNER( st );
					st->Set_Name(texture_name);
					m_W3DShadowTextureManager->addTexture( st );
					st->setTexture(w3dTexture);
				}
				shadowType=SHADOW_DECAL;
				allowSunDirection=shadowInfo->m_type & SHADOW_DIRECTIONAL_PROJECTION;
				decalSizeX=shadowInfo->m_sizeX;
				decalSizeY=shadowInfo->m_sizeY;
				decalOffsetX=shadowInfo->m_offsetX;
				decalOffsetY=shadowInfo->m_offsetY;
		}
		else
		if (shadowInfo->m_type==SHADOW_PROJECTION)
		{		//projection of object geometry into a texture.
				//can be applied on a plane horizontal to model's z-axis or
				//projected onto world geometry.
				if (shadowInfo->m_ShadowName[0] != '\0')
				{	//the shadow will be based on another render object
					//to allow multiple models to share same shadow - for
					//example, all trees could use same shadow even if slightly
					//different color, etc.
					static_assert(ARRAY_SIZE(texture_name) >= ARRAY_SIZE(shadowInfo->m_ShadowName), "Incorrect array size");
					strcpy(texture_name, shadowInfo->m_ShadowName);
				}
				else
					strlcpy(texture_name, robj->Get_Name(), ARRAY_SIZE(texture_name));	//not texture name give, assume model name.

				st=m_W3DShadowTextureManager->getTexture(texture_name);
				if (st == nullptr)
				{	//texture doesn't exist, use current render object to create it
					m_W3DShadowTextureManager->createTexture(robj,texture_name);
					//try loading again
					st=m_W3DShadowTextureManager->getTexture(texture_name);

					DEBUG_ASSERTCRASH(st != nullptr, ("Could not create shadow texture"));

					if (st==nullptr)
						return nullptr;	//could not create the shadow texture
				}
				shadowType=SHADOW_PROJECTION;
		}
	}
	else
	{	//no shadow info, assume user wants a projected shadow
		strlcpy(texture_name, robj->Get_Name(), ARRAY_SIZE(texture_name));

		st=m_W3DShadowTextureManager->getTexture(texture_name);

		if (st==nullptr)
		{	//did not find a cached copy of the shadow geometry, create a new one
			m_W3DShadowTextureManager->createTexture(robj,texture_name);
			//try loading again
			st=m_W3DShadowTextureManager->getTexture(texture_name);
			if (st==nullptr)
				return nullptr;	//could not create the shadow texture
		}
		shadowType=SHADOW_PROJECTION;
	}

	W3DProjectedShadow *shadow = NEW W3DProjectedShadow;

	// sanity
	if( shadow == nullptr )
		return nullptr;

	shadow->setRenderObject(robj);
	shadow->setTexture(0,st);	///@todo: Fix projected shadows to allow multiple lights
	shadow->m_type = shadowType;		/// type of projection
	shadow->m_allowWorldAlign=allowWorldAlign;	/// wrap shadow around world geometry - else align perpendicular to local z-axis.

	AABoxClass box;

	robj->Get_Obj_Space_Bounding_Box(box);

	//Check if app is overriding any of the default texture stretch factors.
	if (decalSizeX)
		decalSizeX=1.0f/decalSizeX; //world space distance to stretch full texture scale
	else
		decalSizeX=1.0f/(box.Extent.X*2.0f);//use bounding box to determine size

	if (decalSizeY)
		decalSizeY=-1.0f/decalSizeY;
	else
		decalSizeY=-1.0f/(box.Extent.Y*2.0f);//world space distance to stretch full texture

	if (decalOffsetX)
		decalOffsetX=-decalOffsetX*decalSizeX;
	else
		decalOffsetX=0.0f;//-box.Center.X*decalSizeX;

	if (decalOffsetY)
		decalOffsetY=-decalOffsetY*decalSizeY;
	else
		decalOffsetY=0.0f;//-box.Center.Y*decalSizeY;

	//Prestore some values used during projection to optimize out division.
	shadow->m_oowDecalSizeX = decalSizeX;	//one over width
	shadow->m_oowDecalSizeY = decalSizeY;	//one over height
	shadow->m_decalSizeX = 1.0f/decalSizeX;	//width
	shadow->m_decalSizeY = 1.0f/decalSizeY;	//height

	shadow->m_decalOffsetU= decalOffsetX;
	shadow->m_decalOffsetV= decalOffsetY;

	shadow->m_flags	= allowSunDirection;

	shadow->init();

	// add to our shadow list through the shadow next links, insert next to other shadows using same texture

	W3DProjectedShadow *nextShadow=nullptr,*prevShadow=nullptr;
	for( nextShadow = m_shadowList; nextShadow; prevShadow=nextShadow,nextShadow = nextShadow->m_next )
	{
		if (nextShadow->m_shadowTexture[0]==st)
		{	//found start of other shadows using same texture, insert new shadow here.
			shadow->m_next=nextShadow;
			if (prevShadow)
			{	prevShadow->m_next=shadow;
			}
			else
				m_shadowList=shadow;
			break;
		}
	}

	if (nextShadow==nullptr)
	{	//shadow with new texture. Add to top of list.
		shadow->m_next = m_shadowList;
		m_shadowList = shadow;
	}

	updateShadowNumbers(shadow->m_type, +1);
	return shadow;
}

W3DProjectedShadow* W3DProjectedShadowManager::createDecalShadow(Shadow::ShadowTypeInfo *shadowInfo)
{
	W3DShadowTexture *st=nullptr;
	static char	defaultDecalName[]={"shadow.tga"};
	ShadowType shadowType=SHADOW_DECAL;		/// type of projection
	Bool	allowWorldAlign=FALSE;	/// wrap shadow around world geometry - else align perpendicular to local z-axis.
	Real	decalSizeX=0.0f;
	Real	decalSizeY=0.0f;
	Real	decalOffsetX=0.0f;
	Real	decalOffsetY=0.0f;
	const Real defaultWidth = 10.0f;
	Char texture_name[ARRAY_SIZE(shadowInfo->m_ShadowName)];

	//simple decal using the premade texture specified.
	//can be always perpendicular to model's z-axis or projected
	//onto world geometry.
	if (strlen(shadowInfo->m_ShadowName) <= 1)	//no texture name given, use same as object
	{
		static_assert(ARRAY_SIZE(texture_name) >= ARRAY_SIZE(defaultDecalName), "Incorrect array size");
		strcpy(texture_name, defaultDecalName);
	}
	else
	{
		static_assert(ARRAY_SIZE(texture_name) >= ARRAY_SIZE(shadowInfo->m_ShadowName), "Incorrect array size");
		strcpy(texture_name, shadowInfo->m_ShadowName);
		strlcat(texture_name, ".tga", ARRAY_SIZE(texture_name));
	}

	st=m_W3DShadowTextureManager->getTexture(texture_name);
	if (st == nullptr)
	{
		//need to add this texture without creating it from a real renderobject
		W3DTextureHandle *w3dTexture=W3DAssetCatalog::Get_Instance()->Get_Texture(texture_name);
		DEBUG_ASSERTCRASH(w3dTexture != nullptr, ("Could not load decal texture"));
		if (!w3dTexture)
			return nullptr;

		w3dTexture->Get_Sampling().address[0] = Graphics::RHISamplerAddress::Clamp;
		w3dTexture->Get_Sampling().address[1] = Graphics::RHISamplerAddress::Clamp;
		w3dTexture->Get_Sampling().mipmap = Graphics::SamplingFilter::Disabled;

		st = NEW W3DShadowTexture;	// poolify
		SET_REF_OWNER( st );
		st->Set_Name(texture_name);
		m_W3DShadowTextureManager->addTexture( st );
		st->setTexture(w3dTexture);
	}
	shadowType=SHADOW_DECAL;
	decalSizeX=shadowInfo->m_sizeX;
	decalSizeY=shadowInfo->m_sizeY;
	decalOffsetX=shadowInfo->m_offsetX;
	decalOffsetY=shadowInfo->m_offsetY;

	W3DProjectedShadow *shadow = NEW W3DProjectedShadow;

	// sanity
	if( shadow == nullptr )
		return nullptr;

	shadow->setTexture(0,st);
	shadow->m_type = shadowType;		/// type of projection
	shadow->m_allowWorldAlign=allowWorldAlign;	/// wrap shadow around world geometry - else align perpendicular to local z-axis.


	//Check if app is overriding any of the default texture stretch factors.
	if (decalSizeX)
		decalSizeX=1.0f/decalSizeX; //world space distance to stretch full texture scale
	else
		decalSizeX=1.0f/(defaultWidth*2.0f);//use bounding box to determine size

	if (decalSizeY)
		decalSizeY=-1.0f/decalSizeY;
	else
		decalSizeY=-1.0f/(defaultWidth*2.0f);//world space distance to stretch full texture

	if (decalOffsetX)
		decalOffsetX=-decalOffsetX*decalSizeX;
	else
		decalOffsetX=0.0f;//-box.Center.X*decalSizeX;

	if (decalOffsetY)
		decalOffsetY=-decalOffsetY*decalSizeY;
	else
		decalOffsetY=0.0f;//-box.Center.Y*decalSizeY;

	//Prestore some values used during projection to optimize out division.
	shadow->m_oowDecalSizeX = decalSizeX;	//one over width
	shadow->m_oowDecalSizeY = decalSizeY;	//one over height
	shadow->m_decalSizeX = 1.0f/decalSizeX;	//width
	shadow->m_decalSizeY = 1.0f/decalSizeY;	//height

	shadow->m_decalOffsetU= decalOffsetX;
	shadow->m_decalOffsetV= decalOffsetY;

	shadow->m_flags	= 0;

	shadow->init();

	return shadow;
}

void W3DProjectedShadowManager::removeShadow (W3DProjectedShadow *shadow)
{
	W3DProjectedShadow *prev_shadow=nullptr;
	W3DProjectedShadow *next_shadow=nullptr;

	if (shadow->m_type & (SHADOW_ALPHA_DECAL|SHADOW_ADDITIVE_DECAL))
	{
		for( next_shadow = m_decalList; next_shadow; prev_shadow=next_shadow, next_shadow = next_shadow->m_next )
		{
			if (next_shadow == shadow)
			{
				if (prev_shadow)
					prev_shadow->m_next=shadow->m_next;
				else
					m_decalList=shadow->m_next;

				updateShadowNumbers(shadow->m_type, -1);
				delete shadow;
				return;
			}
		}
	}

	//search for this shadow
	for( next_shadow = m_shadowList; next_shadow; prev_shadow=next_shadow, next_shadow = next_shadow->m_next )
	{
		if (next_shadow == shadow)
		{
			if (prev_shadow)
				prev_shadow->m_next=shadow->m_next;
			else
				m_shadowList=shadow->m_next;

			updateShadowNumbers(shadow->m_type, -1);
			delete shadow;
			return;
		}
	}
}

void W3DProjectedShadowManager::removeAllShadows()
{

	W3DProjectedShadow *cur_shadow=nullptr;
	W3DProjectedShadow *next_shadow=m_shadowList;
	m_shadowList = nullptr;
	m_numDecalShadows  = 0;
	m_numProjectionShadows = 0;

	//search for this shadow
	for( cur_shadow = next_shadow; cur_shadow; cur_shadow = next_shadow )
	{
		next_shadow = cur_shadow->m_next;
		cur_shadow->m_next = nullptr;
		delete cur_shadow;
	}

	next_shadow=m_decalList;
	cur_shadow=nullptr;
	m_decalList=nullptr;
	for( cur_shadow = next_shadow; cur_shadow; cur_shadow = next_shadow )
	{
		next_shadow = cur_shadow->m_next;
		cur_shadow->m_next = nullptr;
		delete cur_shadow;
	}
}

void W3DProjectedShadowManager::updateShadowNumbers(ShadowType shadowType, Int addNum)
{
	switch (shadowType)
	{
		case SHADOW_DECAL:
		case SHADOW_ALPHA_DECAL:
		case SHADOW_ADDITIVE_DECAL:
			m_numDecalShadows += addNum;
			break;
		case SHADOW_PROJECTION:
			m_numProjectionShadows += addNum;
			break;
		default:
			break;
	}
}

#if defined(RTS_DEBUG)
void W3DProjectedShadow::getRenderCost(RenderCost & rc) const
{
	if (TheGlobalData->m_useShadowDecals && m_isEnabled && !m_isInvisibleEnabled)
		rc.addShadowDrawCalls(1);
}
#endif

W3DProjectedShadow::W3DProjectedShadow()
{
	m_diffuse=0xffffffff;
	m_lastObjPosition.Set(0,0,0);
	m_type = SHADOW_NONE;		/// type of projection
	m_allowWorldAlign = FALSE;	/// wrap shadow around world geometry - else align perpendicular to local z-axis.
	m_isEnabled = TRUE;
	m_isInvisibleEnabled = FALSE;
	for (Int i=0; i<MAX_SHADOW_LIGHTS; i++)
		m_shadowTexture[i]=nullptr;
}

W3DProjectedShadow::~W3DProjectedShadow()
{
	for (Int i=0; i<MAX_SHADOW_LIGHTS; i++)
		REF_PTR_RELEASE(m_shadowTexture[i]);
}

void W3DProjectedShadow::init()
{
	if (m_type == SHADOW_PROJECTION)
	{
		if (m_shadowTexture[0] == nullptr || m_shadowTexture[0]->getTexture() == nullptr)
		{
			m_isEnabled = FALSE;
			return;
		}

		m_shadowMapping = Graphics::TextureMapping::Create_Projection();
		m_shadowMapping->Projection()->type = Graphics::TextureProjection::Perspective;
		m_shadowMaterial = std::make_shared<Graphics::MeshMaterial>();
		m_shadowMaterial->parameters.ambient = {0.0f, 0.0f, 0.0f};
		m_shadowMaterial->parameters.diffuse = {0.0f, 0.0f, 0.0f};
		m_shadowMaterial->parameters.specular = {0.0f, 0.0f, 0.0f};
		m_shadowMaterial->parameters.emissive = {0.6f, 0.6f, 0.6f};
		m_shadowMaterial->parameters.opacity = 1.0f;
		m_shadowMaterial->parameters.lighting = true;
		m_shadowMaterial->mappings[0] = m_shadowMapping;
	}
}

#define DECAL_TEXELS_PER_WORLD_UNIT	(64.0f/20.0f)	//64 texels per 2 terrain cells (20 units)

void W3DProjectedShadow::updateTexture(Vector3 &lightPos)
{
	W3DRenderContext *context;
	if (m_shadowTexture[0] == nullptr || m_shadowTexture[0]->getTexture() == nullptr)
	{
		return;
	}

	//default uv coordinates before rotation starting at top/left going clockwise
	static Vector2 uvData[4]={Vector2(-0.5,-0.5f),Vector2(-0.5,0.5f),Vector2(0.5f,0.5f),Vector2(-0.5f,0.5f)};

	//force light always 2000 units from object - for some reason projection fails if
	//light is too far.
	///@todo: See why infinite light sources don't project shadows correctly.

	if (m_type == SHADOW_PROJECTION)
	{	//projected shadows use custom runtime generated textures based on object geometry
		W3DTextureHandle *shadow_texture = m_shadowTexture[0]->getTexture();
		W3DTextureHandle *render_target = TheW3DProjectedShadowManager != nullptr ?
			TheW3DProjectedShadowManager->getRenderTarget() : nullptr;
		if (m_robj == nullptr || m_shadowMapping == nullptr || m_shadowMaterial == nullptr ||
			!shadow_texture->Ensure_Render_Backend_Texture() ||
			render_target == nullptr || !render_target->Ensure_Render_Backend_Texture())
		{
			return;
		}

		Vector3 objPos=m_robj->Get_Position();
		if (objPos == Vector3(0,0,0))
			return; //render object does not have a valid position (never rendered).
		Vector3 objToLight=lightPos - objPos;
		objToLight.Normalize();
		objToLight =  objPos + objToLight * 2000.0f;

		m_shadowFit = Build_Shadow_Fit(*m_robj, objToLight);
		if (!m_shadowFit.valid) return;
		Assets::ImageDescription target_description;
		render_target->Get_Level_Description(target_description);
		if (target_description.width <= 0 || target_description.height <= 0) return;

		//Set ambient to 0, so we get a black shadow on solid background

		context=TheW3DProjectedShadowManager->getRenderContext();
		if (context == nullptr || context->light_environment == nullptr)
		{
			return;
		}
		Configure_Shadow_Camera(context->Camera, m_shadowFit,
			static_cast<unsigned>(target_description.width), static_cast<unsigned>(target_description.height));

		context->light_environment->Reset({(m_robj->Get_Position()).X,(m_robj->Get_Position()).Y,(m_robj->Get_Position()).Z}, {0,0,0});

		const bool captured = Graphics::Capture_Projected_Texture(
			Graphics::Get_Attachment_Bindings(), render_target->Peek_Render_Backend_Texture(), *context,
			[this, target_width = static_cast<unsigned>(target_description.width),
				target_height = static_cast<unsigned>(target_description.height)](W3DRenderContext &info) {
				info.Camera.Apply();
				// W3DCamera::Apply converts its normalized viewport against the
				// default screen target. The generated texture can have a different
				// size, so restore the exact one-pixel border in target pixels.
				const auto viewport = Graphics::Projected_Texture_Viewport(target_width,
					target_height);
				if (!Graphics::Get_Attachment_Bindings().Set_Viewport(viewport)) return false;
				W3DObjectGraphics graphics;
				Graphics::PropLighting lighting{};
				return graphics.Render(*m_robj, info, lighting, nullptr);
			},
			[](W3DRenderContext &) { return true; });
		if (!captured) return;

		//Need to copy generated texture into permanent texture.
		Graphics::TextureEdit *oldSurface=shadow_texture->Get_Surface_Level();
		Graphics::TextureEdit *newSurface=render_target->Get_Surface_Level();
		if (oldSurface == nullptr || newSurface == nullptr)
		{
			delete newSurface; newSurface = nullptr;
			delete oldSurface; oldSurface = nullptr;
			return;
		}

		//Copy shadow from temporary video-memory surface into a permanent texture
		oldSurface->Copy_From(*newSurface,
            {0,0,DEFAULT_RENDER_TARGET_WIDTH,DEFAULT_RENDER_TARGET_HEIGHT},
            {0,0,DEFAULT_RENDER_TARGET_WIDTH,DEFAULT_RENDER_TARGET_HEIGHT});
		delete newSurface; newSurface = nullptr;
		delete oldSurface; oldSurface = nullptr;
		m_shadowTexture[0]->updateBounds(TheW3DShadowManager->getLightPosWorld(0),m_robj);	//update local shadow bounds
	}
	else
	if (m_type == SHADOW_DECAL)
	{	//decal shadows use artist supplied textures.  We just need to tweak the uv coordinates to match
		//the light direction.
		if (m_robj == nullptr)
		{
			m_shadowTexture[0]->setLightPosHistory(lightPos);
			return;
		}

		Vector3 objPos=m_robj->Get_Position();
		Vector3 objectToLight;
		if (m_flags & SHADOW_DIRECTIONAL_PROJECTION)
		{	objectToLight=lightPos-objPos;
			//we're ignoring sun's distance from horizon, so drop vertical component
			objectToLight.Z=0;
			objectToLight.Normalize();
		}
		else
			objectToLight.Set(1.0f,0.0f,0.0f);

		Assets::ImageDescription surface_desc;
		m_shadowTexture[0]->getTexture()->Get_Level_Description(surface_desc);
		if (surface_desc.width <= 0 || surface_desc.height <= 0)
		{
			return;
		}
		//default shadow texture points along world -x axis (west).  Rotate uv coordinates to fit actual light direction
		Vector3 uVec = objectToLight * DECAL_TEXELS_PER_WORLD_UNIT / (float)surface_desc.width;
		objectToLight.Rotate_Z(-1.0f,0.0f);	//rotate u vector by -90 degrees to get v vector.
		Vector3 vVec = objectToLight * DECAL_TEXELS_PER_WORLD_UNIT / (float)surface_desc.height;

		m_shadowTexture[0]->setDecalUVAxis(uVec, vVec);

		///@todo: tweak decal bounding volumes to something sensible
		AABoxClass	box;
		SphereClass	sphere;
		m_robj->Get_Obj_Space_Bounding_Box(box);	//shadow uses same bounding box as object
		m_robj->Get_Obj_Space_Bounding_Sphere(sphere);

		m_shadowTexture[0]->setBoundingSphere(sphere);
		m_shadowTexture[0]->setBoundingBox(box);
	}

	m_shadowTexture[0]->setLightPosHistory(lightPos);	//store position of light at time of texture update.

}


void W3DProjectedShadow::updateProjectionParameters(const Matrix3D &cameraXform)
{
	if (m_type != SHADOW_PROJECTION || m_shadowMapping == nullptr || !m_shadowFit.valid)
		return;
	Graphics::Matrix4x4 camera_transform = Graphics::Matrix4x4::Identity();
	for (unsigned row = 0; row < 3; ++row)
		for (unsigned column = 0; column < 4; ++column)
			camera_transform.values[row * 4 + column] = cameraXform[row][column];
	const auto view_to_texture = Graphics::Make_Texture_Projector_View_Transform(
		m_shadowFit, camera_transform);
	Assets::ImageDescription texture_description;
	if (m_shadowTexture[0] == nullptr || m_shadowTexture[0]->getTexture() == nullptr)
		return;
	m_shadowTexture[0]->getTexture()->Get_Level_Description(texture_description);
	if (texture_description.width <= 0) return;
	std::array<float,16> transform = view_to_texture.values;
	m_shadowMapping->Projection()->Set_Texture_Transform(transform,
		static_cast<float>(texture_description.width));
}

void W3DProjectedShadow::update()
{
	if (m_shadowTexture[0] == nullptr)
	{
		return;
	}

	if (m_shadowTexture[0]->getLightPosHistory() != TheW3DShadowManager->getLightPosWorld(0))
	{	//light has moved since last time this shadow was calculated. Need update
		updateTexture(TheW3DShadowManager->getLightPosWorld(0));
	}
	if (m_robj != nullptr && m_lastObjPosition != m_robj->Get_Position())
	{	//object has moved.  Texture stays the same but projection matrix needs updating.
		//force light always 2000 units from object - for some reason projection fails if
		//light is too far.
		///@todo: See why infinite light sources don't project shadows correctly.
		if (m_type == SHADOW_PROJECTION)
		{
			Vector3 object_to_light = TheW3DShadowManager->getLightPosWorld(0)
				- m_robj->Get_Position();
			object_to_light.Normalize();
			object_to_light = m_robj->Get_Position() + object_to_light * 2000.0f;
			m_shadowFit = Build_Shadow_Fit(*m_robj, object_to_light);
		}
		setObjPosHistory(m_robj->Get_Position());
	}
}

Int W3DShadowTexture::init(W3DRenderObject *robj)
{
	///@todo: implement this function
	if (TheW3DProjectedShadowManager == nullptr)
	{
		return FALSE;
	}

	W3DTextureHandle *render_target = TheW3DProjectedShadowManager->getRenderTarget();
	if (render_target == nullptr || !render_target->Ensure_Render_Backend_Texture())
	{
		return FALSE;
	}

	Assets::ImageDescription surface_desc{};

	render_target->Get_Level_Description(surface_desc);
	if (surface_desc.width <= 0 || surface_desc.height <= 0 ||
		surface_desc.encoding == Assets::PixelEncoding::Unknown)
	{
		return FALSE;
	}

	W3DTextureHandle *new_texture = MSGNEW("W3DTextureHandle") W3DTextureHandle(surface_desc.width,surface_desc.height,surface_desc.encoding,MIP_LEVELS_1);
	if (new_texture == nullptr || !new_texture->Ensure_Render_Backend_Texture())
	{
		REF_PTR_RELEASE(new_texture);
		return FALSE;
	}

	setTexture(new_texture);

	return TRUE;
}

void W3DShadowTexture::updateBounds(Vector3 &lightPos, W3DRenderObject *robj)
{
		AABoxClass	&box=m_areaEffectBox;	///@todo: fix for multiple lights
		Vector3			objPos;
		Vector3 Corners[8];
		Vector3 lightRay;
		Real floorZ;
		Real vectorScale,vectorScaleTemp, vectorScaleMax,length;

		//calculate local bounding box of shadow projection
		objPos=robj->Get_Position();
		box=robj->Get_Bounding_Box();
		floorZ = objPos.Z - 2.0f;	//lower slightly so shadows go under ground.

		//project each box corner to base of object to determine rough extent of shadow
		//Get vertices of top of bounding box
		Corners[0]=box.Center+box.Extent;	//top right corner
		Corners[1]=Corners[0];
		Corners[1].X -= 2.0f*box.Extent.X;		//top left corner
		Corners[2]=Corners[1];
		Corners[2].Y -= 2.0f*box.Extent.Y;		//bottom left corner
		Corners[3]=Corners[2];
		Corners[3].X += 2.0f*box.Extent.X;		//bottom right corner

		//Project top volume corners onto ground plane
		lightRay = Corners[0] - lightPos;	//vector light to corner
		length= 1.0f/lightRay.Length();
		lightRay *= length;
		vectorScaleMax=vectorScale=(Real)fabs((Corners[0].Z-floorZ)/lightRay.Z);	//length of vector from top corner to ground.
		Corners[4]=Corners[0]+lightRay*vectorScale;
		vectorScaleMax *= length;

		lightRay = Corners[1] - lightPos;	//vector light to corner
		length= 1.0f/lightRay.Length();
		lightRay *= length;
		vectorScaleTemp=(Real)fabs((Corners[1].Z-floorZ)/lightRay.Z);	//length of vector from top corner to ground.
		Corners[5]=Corners[1]+lightRay*vectorScaleTemp;
		vectorScaleTemp *= length;

		if (vectorScaleTemp > vectorScaleMax)
			vectorScaleMax=vectorScaleTemp;	//keep track of maximum required extrusion length.

		lightRay = Corners[2] - lightPos;	//vector light to corner
		length= 1.0f/lightRay.Length();
		lightRay *= length;
		vectorScale=(Real)fabs((Corners[2].Z-floorZ)/lightRay.Z);	//length of vector from top corner to ground.
		Corners[6]=Corners[2]+lightRay*vectorScale;
		vectorScale *= length;

		if (vectorScale > vectorScaleMax)
			vectorScaleMax=vectorScale;	//keep track of maximum required extrusion length.

		lightRay = Corners[3] - lightPos;	//vector light to corner
		length= 1.0f/lightRay.Length();
		lightRay *= length;
		vectorScaleTemp=(Real)fabs((Corners[3].Z-floorZ)/lightRay.Z);	//length of vector from top corner to ground.
		Corners[7]=Corners[3]+lightRay*vectorScaleTemp;
		vectorScaleTemp *= length;

		if (vectorScaleTemp > vectorScaleMax)
			vectorScaleMax=vectorScaleTemp;	//keep track of maximum required extrusion length.

		box.Init(Corners, 8);	//generate a new bounding box to fit the shadow projection
		m_areaEffectSphere.Init(box.Center,box.Extent.Length());

		m_areaEffectSphere.Center -= objPos;	//translate sphere to object space.
		box.Translate(-objPos);	//translate box to object space.
}

W3DShadowTextureManager::W3DShadowTextureManager()
{
	// Create the hash tables
	texturePtrTable = NEW HashTableClass( 2048 );
	missingTextureTable = NEW HashTableClass( 2048 );
}

W3DShadowTextureManager::~W3DShadowTextureManager()
{
	freeAllTextures();

	delete texturePtrTable;
	texturePtrTable = nullptr;

	delete missingTextureTable;
	missingTextureTable = nullptr;
}

/** Release all loaded textures */
void W3DShadowTextureManager::freeAllTextures()
{
	// Make an iterator, and release all ptrs
	W3DShadowTextureManagerIterator it( *this );
	for( it.First(); !it.Is_Done(); it.Next() ) {
		W3DShadowTexture *text = it.getCurrentTexture();
		text->Release_Ref();
	}

	// Then clear the table
	texturePtrTable->Reset();
}

/** Find texture in cache */
W3DShadowTexture * W3DShadowTextureManager::peekTexture(const char * name)
{
	return (W3DShadowTexture*)texturePtrTable->Find( name );
}

/** Get texture from cache and increment its reference count */
W3DShadowTexture * W3DShadowTextureManager::getTexture(const char * name)
{
	W3DShadowTexture * text = peekTexture( name );
	if ( text != nullptr ) {
		text->Add_Ref();
	}
	return text;
}

/** Add texture to cache */
Bool W3DShadowTextureManager::addTexture(W3DShadowTexture *newTexture)
{
	WWASSERT (newTexture != nullptr);

	// Increment the refcount on the new texture and add it to our table.
	newTexture->Add_Ref ();
	texturePtrTable->Add( newTexture );

	return true;
}

void W3DShadowTextureManager::invalidateCachedLightPositions()
{
	// step through each of our shadow textures and update previous light position.
	Vector3 idVec(0,0,0);

	W3DShadowTextureManagerIterator it( *this );
	for( it.First(); !it.Is_Done(); it.Next() )
	{
		W3DShadowTexture *text = it.getCurrentTexture();
		text->setLightPosHistory(idVec);
	}
}

/*
** An entry for a table of textures not found, so we can quickly determine their loss
*/
class MissingTextureClass : public HashableClass {

public:
	MissingTextureClass( const char * name ) : Name( name ) {}
	virtual	~MissingTextureClass() override {}

	virtual	const char * Get_Key() override { return Name;	}

private:
	StringClass	Name;

};

/*
** Missing Textures
**
** The idea here, allow the system to register which textures are determined to be missing
** so that if they are asked for again, we can quickly return nullptr, without searching again.
*/
void	W3DShadowTextureManager::registerMissing( const char * name )
{
	missingTextureTable->Add( NEW MissingTextureClass( name ) );
}

Bool	W3DShadowTextureManager::isMissing( const char * name )
{
	return ( missingTextureTable->Find( name ) != nullptr );
}

/** Create shadow geometry from a reference W3D RenderObject*/
int W3DShadowTextureManager::createTexture(W3DRenderObject *robj, const char *name)
{
	Bool res=FALSE;

	W3DShadowTexture * newTexture = NEW W3DShadowTexture;

	if (newTexture == nullptr) {
		goto Error;
	}

	SET_REF_OWNER( newTexture );
	newTexture->Set_Name(name);

	res=newTexture->init(robj);

	if (res != TRUE)
	{	// load failed!
		newTexture->Release_Ref();
		goto Error;
	} else if (peekTexture(newTexture->Get_Name()) != nullptr)
	{	// duplicate exists!
		newTexture->Release_Ref();	// Release the one we just loaded
		goto Error;
	} else
	{	addTexture( newTexture );
		newTexture->Release_Ref();
	}

	return 0;

Error:
	return 1;
}

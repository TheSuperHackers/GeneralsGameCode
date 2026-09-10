#pragma once

#include "Lib/BaseType.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"

import Graphics.Materials.MeshMaterial;
import Graphics.Resources.Textures.Edit;

class W3DRenderObject;
class W3DTextureHandle;

// Game-specific policy and house-color customization around the generic W3D
// asset catalog. Generic callers use W3DAssetCatalog directly.
class W3DAssetManager final
{
public:
	W3DAssetManager();
	~W3DAssetManager();

	W3DAssetManager(const W3DAssetManager &) = delete;
	W3DAssetManager &operator=(const W3DAssetManager &) = delete;

	W3DAssetCatalog &Catalog() noexcept { return m_catalog; }
	const W3DAssetCatalog &Catalog() const noexcept { return m_catalog; }

	W3DRenderObject *Create_Render_Obj(const char *name, float scale, int color,
		const char *oldTexture = nullptr, const char *newTexture = nullptr);
	int replacePrototypeTexture(W3DRenderObject *robj, const char *oldname, const char *newname);

private:
	void Make_Mesh_Unique(W3DRenderObject *robj, Bool geometry, Bool colors);
	void Make_HLOD_Unique(W3DRenderObject *robj, Bool geometry, Bool colors);
	void Make_Unique(W3DRenderObject *robj, Bool geometry, Bool colors);

	int Recolor_Asset(W3DRenderObject *robj, int color);
	int Recolor_Mesh(W3DRenderObject *robj, int color);
	int Recolor_HLOD(W3DRenderObject *robj, int color);
	void Recolor_Vertex_Material(Graphics::MeshMaterial *vmat, int color);
	W3DTextureHandle *Find_Texture(const char *name, int color);
	W3DTextureHandle *Recolor_Texture(W3DTextureHandle *texture, int color);
	W3DTextureHandle *Recolor_Texture_One_Time(W3DTextureHandle *texture, int color);
	void Remap_Palette(Graphics::TextureEdit *surface, int color, Bool doPaletteOnly, Bool useAlpha);
	int replaceAssetTexture(W3DRenderObject *robj, W3DTextureHandle *oldTex, W3DTextureHandle *newTex);
	int replaceHLODTexture(W3DRenderObject *robj, W3DTextureHandle *oldTex, W3DTextureHandle *newTex);
	int replaceMeshTexture(W3DRenderObject *robj, W3DTextureHandle *oldTex, W3DTextureHandle *newTex);

	W3DAssetCatalog m_catalog;
};

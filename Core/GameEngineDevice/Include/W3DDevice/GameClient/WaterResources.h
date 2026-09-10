/*
** Command & Conquer Generals Zero Hour(tm)
**
** Water resource boundary for the modern water render system. The renderer
** consumes W3DTextureHandle handles; legacy asset/surface construction is
** contained in the implementation of this adapter.
*/

#pragma once

class W3DTextureHandle;

W3DTextureHandle *Load_Water_Texture(const char *name);
W3DTextureHandle *Create_Water_White_Texture();
W3DTextureHandle *Create_Water_Depth_Lut_Texture();
void Reinitialize_Water_Procedural_Texture(W3DTextureHandle *texture,
	bool depth_lut);
unsigned Get_Water_Texture_Width(const W3DTextureHandle *texture);

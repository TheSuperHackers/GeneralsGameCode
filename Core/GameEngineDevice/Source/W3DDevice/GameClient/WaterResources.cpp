/*
** Command & Conquer Generals Zero Hour(tm)
*/

import Assets.Images.PixelEncoding;
#include "Common/GameMemory.h"
#include "W3DDevice/GameClient/WaterResources.h"

#include "W3DDevice/GameClient/W3DAssetCatalog.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"

static void Initialize_Water_Depth_Lut(W3DTextureHandle *texture)
{
	if (texture == nullptr)
		return;

	Graphics::TextureEdit *surface = texture->Get_Surface_Level();
	if (surface == nullptr)
		return;

    const auto mapping=surface->Map();
    const auto encoding=surface->Image().Encoding();
    const unsigned stride=Assets::Pixel_Size(encoding);
    if (!mapping.bytes.empty() && stride) {
        for (unsigned x=0;x<256;++x)
            Assets::Write_Image_Pixel(mapping.bytes.subspan(x*stride,stride),encoding,
                0xff000000u | (x<<16) | (x<<8) | x);
    }
	surface->Unmap();
	delete surface; surface = nullptr;
}

static void Initialize_Water_White_Texture(W3DTextureHandle *texture)
{
	if (texture == nullptr)
		return;

	Graphics::TextureEdit *surface = texture->Get_Surface_Level();
	if (surface == nullptr)
		return;

    const auto mapping=surface->Map();
    if (!mapping.bytes.empty()) Assets::Write_Image_Pixel(mapping.bytes,surface->Image().Encoding(),0xffffffff);
	surface->Unmap();
	delete surface; surface = nullptr;
}

W3DTextureHandle *Load_Water_Texture(const char *name)
{
	return W3DAssetCatalog::Get_Instance()->Get_Texture(name);
}

W3DTextureHandle *Create_Water_White_Texture()
{
	W3DTextureHandle *texture = MSGNEW("W3DTextureHandle") W3DTextureHandle(
		1, 1, Assets::PixelEncoding::BGRA4444, MIP_LEVELS_1);
	Initialize_Water_White_Texture(texture);
	return texture;
}

W3DTextureHandle *Create_Water_Depth_Lut_Texture()
{
	W3DTextureHandle *texture = MSGNEW("W3DTextureHandle") W3DTextureHandle(
		256, 1, Assets::PixelEncoding::BGRA8, MIP_LEVELS_1);
	Initialize_Water_Depth_Lut(texture);
	return texture;
}

void Reinitialize_Water_Procedural_Texture(W3DTextureHandle *texture,
	bool depth_lut)
{
	if (texture == nullptr || texture->Is_Initialized())
		return;

	texture->Init();
	if (depth_lut)
		Initialize_Water_Depth_Lut(texture);
	else
		Initialize_Water_White_Texture(texture);
}

unsigned Get_Water_Texture_Width(const W3DTextureHandle *texture)
{
	return texture == nullptr ? 0u : static_cast<unsigned>(texture->Get_Width());
}

module;

#include <cstddef>
#include <vector>
#include <span>
#include <utility>

export module Graphics.Scene.Models.ModelAssetBinding;

export import Graphics.Scene.Models.ModelAssetGeometry;

import Assets.Models;
import Assets.Cache;
import Assets.Materials;
import Assets.Textures;

namespace Graphics
{

// The adapter retains these handles alongside its material handles and releases
// materials before textures. A named texture must be ready and have pixel data.
export bool Create_Model_Asset_Textures(StaticMeshRenderer& renderer, Assets::AssetCache& assets,
    const Assets::ModelAsset& model, std::vector<TextureHandle>& textures)
{
    std::vector<TextureHandle> created(model.Materials().size());
    const auto clear=[&]() {
        for (auto texture : created) if (texture.Is_Valid()) renderer.Destroy_Texture(texture);
    };
    for (std::size_t i=0;i<model.Materials().size();++i) {
        const auto& description=model.Materials()[i];
        if (!description.texturing) continue;
        const auto* material=assets.Try_Get_Material(description.asset_handle);
        if (!material) { clear(); return false; }
        const auto handle=material->Primary_Texture();
        if (!handle.Is_Valid()) continue;
        assets.Wait(handle);
        const auto* source=assets.Try_Get_Texture(handle);
        if (!source || !source->Has_Pixels()) { clear(); return false; }
        Texture texture;
        texture.width=source->Width(); texture.height=source->Height();
        texture.mip_count=1; texture.format=TextureFormat::RGBA8_UNorm;
        texture.usage=TextureUsage::Sampled;
        texture.pixel_data=source->Pixels(); texture.row_pitch=source->Row_Pitch();
        created[i]=renderer.Create_Texture(texture);
        if (!created[i].Is_Valid()) { clear(); return false; }
    }
    textures=std::move(created);
    return true;
}

export bool Create_Model_Asset_Binding(
	StaticMeshRenderer &renderer,
	const Assets::ModelAsset &asset,
	const RenderTransform &transform,
	RenderInstanceFlags flags,
	SubmeshVisibilityMask visibility_mask,
	StaticMeshBinding &binding,
	std::vector<MaterialHandle> &materials,
	std::span<const TextureHandle> primary_textures = {},
	SkeletonHandle skeleton = {},
	AnimationClipHandle animation = {},
	AnimationPlaybackMode animation_mode = AnimationPlaybackMode::Loop,
	float animation_time = 0.0f);

bool Create_Model_Asset_Binding(
	StaticMeshRenderer &renderer,
	const Assets::ModelAsset &asset,
	const RenderTransform &transform,
	RenderInstanceFlags flags,
	SubmeshVisibilityMask visibility_mask,
	StaticMeshBinding &binding,
	std::vector<MaterialHandle> &materials,
	std::span<const TextureHandle> primary_textures,
	SkeletonHandle skeleton,
	AnimationClipHandle animation,
	AnimationPlaybackMode animation_mode,
	float animation_time)
{
    for (std::size_t i=0;i<asset.Materials().size();++i) {
        const auto& material=asset.Materials()[i];
        if (material.texturing && !material.primary_texture.canonical_name.empty()
            && (i>=primary_textures.size() || !renderer.Is_Texture_Valid(primary_textures[i]))) return false;
    }
	ModelAssetGeometry geometry;
	if (!Build_Model_Asset_Geometry(asset, geometry))
		return false;

	std::vector<MaterialHandle> new_materials;
	new_materials.reserve(asset.Materials().size());
	for (std::size_t material_index = 0; material_index < asset.Materials().size(); ++material_index) {
		const Assets::ModelMaterial &asset_material = asset.Materials()[material_index];
		Material material;
		material.parameters.values[0] = asset_material.base_color.r;
		material.parameters.values[1] = asset_material.base_color.g;
		material.parameters.values[2] = asset_material.base_color.b;
		material.parameters.values[3] = asset_material.opacity;
		material.flags = MaterialFlags::VertexColor;
		switch (asset_material.render_mode) {
		case Assets::MaterialRenderMode::AlphaTest:
			material.flags = material.flags | MaterialFlags::AlphaTest;
			break;
		case Assets::MaterialRenderMode::AlphaBlend:
			material.flags = material.flags | MaterialFlags::Transparent;
			break;
		case Assets::MaterialRenderMode::Additive:
			material.flags = material.flags | MaterialFlags::Transparent | MaterialFlags::Additive;
			break;
		case Assets::MaterialRenderMode::Multiply:
			material.flags = material.flags | MaterialFlags::Transparent | MaterialFlags::Multiply;
			break;
		case Assets::MaterialRenderMode::Opaque:
			break;
		}
		if (asset_material.texturing && material_index < primary_textures.size() && primary_textures[material_index].Is_Valid())
			material.textures[0] = primary_textures[material_index];
		if (asset_material.opacity < 0.999f && asset_material.render_mode == Assets::MaterialRenderMode::Opaque)
			material.flags = material.flags | MaterialFlags::Transparent;
		const MaterialHandle handle = renderer.Create_Material(material);
		if (!handle.Is_Valid()) {
			for (const MaterialHandle created : new_materials)
				renderer.Destroy_Material(created);
			return false;
		}
		new_materials.push_back(handle);
	}

	for (MeshPart &part : geometry.parts)
		part.material = new_materials[part.pass_key];
	geometry.source.parts = std::span<const MeshPart>(geometry.parts);
	if (!binding.Replace(renderer, geometry.source, transform, geometry.bounds, new_materials.front(),
		flags, visibility_mask, skeleton, animation, animation_mode, animation_time)) {
		for (const MaterialHandle created : new_materials)
			renderer.Destroy_Material(created);
		return false;
	}

	materials = std::move(new_materials);
	return true;
}

}

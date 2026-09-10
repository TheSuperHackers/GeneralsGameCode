module;
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <utility>
export module Graphics.Scene.Props.AssetGeometry;
export import Graphics.Scene.Props.Geometry;
export import Graphics.Scene.Props.Skinning;
import Assets.Models;

namespace Graphics {
export struct PropAssetPart final {
    std::string name;
    std::uint32_t material_index=0;
    std::vector<PropVertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<PropSkinInfluences> skinning;
};

export bool Build_Prop_Asset_Geometry(const Assets::ModelAsset& model,
    std::vector<PropAssetPart>& result,std::string& error)
{
    error.clear();
    bool skin_described=false;
    for(const auto& part:model.Submeshes()) skin_described|=part.skinned;
    if(model.Skin_Bone_Count()!=0 && !skin_described) { error="model requires explicit submesh skin pose bindings"; return false; }
    std::vector<PropAssetPart> parts;
    const auto vertices=model.Vertices();
    const auto indices=model.Indices();
    PropBatchBuilder batch;
    for(const auto& submesh:model.Submeshes()) {
        std::vector<PropSkinInfluences> skinning;
        if(submesh.material_index>=model.Materials().size() || submesh.first_index>indices.size()
            || submesh.index_count>indices.size()-submesh.first_index || submesh.index_count%3!=0) {
            error="invalid model submesh"; return false;
        }
        const auto& material=model.Materials()[submesh.material_index];
        if(!batch.Begin(vertices.size(),submesh.index_count)) { error="model geometry exceeds buffer capacity"; return false; }
        for(auto index:indices.subspan(submesh.first_index,submesh.index_count)) {
            if(index>=vertices.size()) { error="model references an invalid vertex"; return false; }
            if(!batch.Append(index,[&](std::uint32_t source_index) {
                const auto& source=vertices[source_index];
                if(submesh.skinned) skinning.push_back({source.bone_indices,source.bone_weights});
                PropVertex vertex;
                vertex.position={source.position.x,source.position.y,source.position.z};
                vertex.normal={source.normal.x,source.normal.y,source.normal.z};
                vertex.tangent={source.tangent.x,source.tangent.y,source.tangent.z,source.tangent_sign};
                vertex.uv={source.texcoord.x,source.texcoord.y};
                vertex.color={source.color.r,source.color.g,source.color.b,source.color.a};
                vertex.material_ambient={material.ambient_color.r,material.ambient_color.g,material.ambient_color.b,1};
                vertex.material_diffuse={material.base_color.r,material.base_color.g,material.base_color.b,material.opacity};
                vertex.material_specular={material.specular_color.r,material.specular_color.g,material.specular_color.b,material.shininess};
                vertex.material_emissive={material.emissive_color.r,material.emissive_color.g,material.emissive_color.b,0};
                return vertex;
            })) { error="could not build model material batch"; return false; }
        }
        if(!Finite_Prop_Vertices(batch.Vertices())) { error="nonfinite model vertex"; return false; }
        PropAssetPart part;
        part.name=submesh.name; part.material_index=submesh.material_index;
        part.vertices.assign(batch.Vertices().begin(),batch.Vertices().end());
        part.indices.assign(batch.Indices().begin(),batch.Indices().end());
        part.skinning=std::move(skinning);
        parts.push_back(std::move(part));
    }
    if(parts.empty()) { error="model has no drawable parts"; return false; }
    result=std::move(parts); return true;
}
}

module;
#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
export module Graphics.Scene.Props.AssetBinding;
export import Graphics.Scene.Props.Submission;
export import Graphics.Scene.Models.AssetPose;
import Graphics.Scene.Props.AssetGeometry;
import Assets.Cache;
import Assets.Models;
import Assets.Materials;
import Assets.Textures;
import Assets.Handles;

namespace Graphics {
// Own GPU versions for a resolved asset. Draws use immutable prepared state;
// source IO and asset-name lookup never occur during submission.
export class PropAssetBinding final {
public:
    PropAssetBinding()=default;
    PropAssetBinding(const PropAssetBinding&)=delete;
    PropAssetBinding& operator=(const PropAssetBinding&)=delete;
    ~PropAssetBinding() { Clear(); }
    void Clear() noexcept {
        if(m_renderer) for(const auto& part:m_parts) m_renderer->Destroy_Mesh(part.mesh);
        if(m_device) for(const auto& texture:m_textures) m_device->Destroy_Texture(texture.second);
        m_parts.clear(); m_textures.clear(); m_device=nullptr; m_renderer=nullptr; m_rest_pose={};
    }
    bool Load(Device& device,PropRenderer& renderer,Assets::AssetCache& assets,
        Assets::ModelAssetHandle model_handle,std::string& error) {
        const auto* model=assets.Try_Get_Model(model_handle);
        if(!model) { error="model asset is not ready"; return false; }
        std::vector<PropAssetPart> geometry;
        if(!Build_Prop_Asset_Geometry(*model,geometry,error)) return false;
        PropAssetBinding next;
        next.m_device=&device; next.m_renderer=&renderer;
        if(!model->Rig().skeleton_name.empty() && !next.m_rest_pose.Initialize(model->Rig(),error)) return false;
        for(const auto& source:geometry) {
            const auto* material=assets.Try_Get_Material(model->Materials()[source.material_index].asset_handle);
            if(!material) { error="model material is not ready"; return false; }
            Part part; part.name=source.name;
            part.skinning=source.skinning;
            if(!part.skinning.empty()) {
                if(!next.m_rest_pose.Bone_Count()) { error="skin requires a resolved skeleton"; return false; }
                part.bind_vertices=source.vertices; part.indices=source.indices;
            }
            if(next.m_rest_pose.Bone_Count()) {
                part.bone=0;
                if(!model->Rig().attachments.empty()) {
                    bool found=false;
                    for(const auto& attachment:model->Rig().attachments) {
                        const auto dot=attachment.object_name.find('.');
                        const auto name=attachment.object_name.substr(dot==std::string::npos ? 0 : dot+1);
                        if(name!=source.name || (found && attachment.lod>=part.lod)) continue;
                        part.bone=attachment.bone; part.lod=attachment.lod; found=true;
                    }
                    if(!found) { error="model part has no hierarchy attachment"; return false; }
                }
            }
            part.style.depth_write=material->Depth_Write();
            part.textured=material->Texturing() && material->Primary_Texture().Is_Valid();
            if(!next.Upload(assets,material->Primary_Texture(),part.textures[0])) { error="could not upload base map"; return false; }
            if(material->Secondary_Texture().Is_Valid()) { error="secondary stage requires an explicit stage binding"; return false; }
            std::uint32_t maps=0;
            for(std::size_t role=0;role<PropSurfaceTextureCount;++role) {
                const auto texture=material->Surface_Texture(static_cast<Assets::MaterialTextureRole>(role));
                if(texture.Is_Valid()) maps|=1u<<role;
                if(!next.Upload(assets,texture,part.textures[PropSurfaceTextureFirst+role])) { error="could not upload surface map"; return false; }
            }
            if(!Configure_Prop_Surface(material->Surface(),maps,part.surface)) { error="invalid surface description"; return false; }
            switch(material->Render_Mode()) {
            case Assets::MaterialRenderMode::AlphaTest: part.alpha_cutoff=material->Surface().alpha_cutoff; break;
            case Assets::MaterialRenderMode::AlphaBlend:
                part.style.source_blend=RHIBlendFactor::SourceAlpha; part.style.destination_blend=RHIBlendFactor::InverseSourceAlpha; break;
            case Assets::MaterialRenderMode::Additive:
                part.style.source_blend=RHIBlendFactor::One; part.style.destination_blend=RHIBlendFactor::One; break;
            case Assets::MaterialRenderMode::Multiply:
                part.style.source_blend=RHIBlendFactor::Zero; part.style.destination_blend=RHIBlendFactor::SourceColor; break;
            default: break;
            }
            part.mesh=renderer.Create_Mesh(source.vertices,source.indices);
            if(!part.mesh.Is_Valid()) { error="could not create model geometry"; return false; }
            next.m_parts.push_back(std::move(part));
        }
        std::swap(m_device,next.m_device); std::swap(m_renderer,next.m_renderer);
        m_parts.swap(next.m_parts); m_textures.swap(next.m_textures);
        std::swap(m_rest_pose,next.m_rest_pose);
        error.clear(); return true;
    }
    std::size_t Part_Count() const noexcept { return m_parts.size(); }
    std::size_t Texture_Count() const noexcept { return m_textures.size(); }
    std::string_view Part_Name(std::size_t part) const noexcept { return part<m_parts.size() ? m_parts[part].name : std::string_view{}; }
    bool Draw_Part(CommandList& commands,std::size_t index,PropParameters parameters,
        const ModelAssetPose* pose=nullptr,std::uint32_t lod=0) const {
        if(!m_renderer || index>=m_parts.size()) return false;
        const auto& part=m_parts[index];
        if(part.lod!=lod) return true;
        bool visible=true;
        if(!Prepare_Pose(part,pose,parameters,visible)) return false;
        if(!visible) return true;
        Prepare(part,parameters);
        MeshVersion mesh{m_renderer,part.mesh,false};
        if(!Mesh_For_Pose(part,pose,mesh)) return false;
        return m_renderer->Draw(commands,mesh.handle,part.style,parameters,part.textures);
    }
    bool Submit_Part(PropSubmission& submission,std::size_t index,PropParameters parameters,
        PropDrawPhase phase,const std::array<float,4>& camera_depth={},
        const ModelAssetPose* pose=nullptr,std::uint32_t lod=0) const {
        if(!m_device || index>=m_parts.size()) return false;
        const auto& part=m_parts[index];
        if(part.lod!=lod) return true;
        bool visible=true;
        if(!Prepare_Pose(part,pose,parameters,visible)) return false;
        if(!visible) return true;
        Prepare(part,parameters);
        MeshVersion mesh{m_renderer,part.mesh,false};
        if(!Mesh_For_Pose(part,pose,mesh)) return false;
        std::size_t retained=0;
        for(;retained<part.textures.size();++retained) {
            const auto texture=part.textures[retained];
            if(texture.Is_Valid() && !m_device->Retain_Texture(texture)) break;
        }
        if(retained==part.textures.size() && submission.Submit(mesh.handle,part.style,parameters,part.textures,phase,camera_depth)) return true;
        for(std::size_t i=0;i<retained;++i) if(part.textures[i].Is_Valid()) m_device->Destroy_Texture(part.textures[i]);
        return false;
    }
private:
    struct MeshVersion {
        PropRenderer* renderer;
        PropMeshHandle handle;
        bool temporary;
        ~MeshVersion() { if(temporary && handle.Is_Valid()) renderer->Destroy_Mesh(handle); }
    };
    struct Part {
        std::string name;
        PropMeshHandle mesh;
        PropStyle style;
        PropSurfaceParameters surface;
        float alpha_cutoff=0;
        bool textured=false;
        std::uint32_t bone=Invalid_Bone_Index;
        std::uint32_t lod=0;
        std::array<RHITextureHandle,PropTextureCount> textures{};
        std::vector<PropVertex> bind_vertices;
        std::vector<std::uint32_t> indices;
        std::vector<PropSkinInfluences> skinning;
    };
    bool Mesh_For_Pose(const Part& part,const ModelAssetPose* instance,MeshVersion& result) const {
        if(part.skinning.empty()) return true;
        const auto& pose=instance ? *instance : m_rest_pose;
        std::vector<RenderTransform> bones(pose.Bone_Count());
        for(std::size_t i=0;i<bones.size();++i) if(!pose.Bone_Transform(static_cast<std::uint32_t>(i),bones[i])) return false;
        std::vector<PropVertex> vertices;
        if(!Pose_Prop_Vertices(part.bind_vertices,part.skinning,bones,vertices)) return false;
        result.handle=m_renderer->Create_Mesh(vertices,part.indices); result.temporary=true;
        return result.handle.Is_Valid();
    }
    bool Prepare_Pose(const Part& part,const ModelAssetPose* instance,PropParameters& parameters,bool& visible) const {
        if(part.bone==Invalid_Bone_Index) return instance==nullptr;
        const auto& pose=instance ? *instance : m_rest_pose;
        if(pose.Skeleton_Name()!=m_rest_pose.Skeleton_Name() || pose.Bone_Count()!=m_rest_pose.Bone_Count()) return false;
        RenderTransform bone;
        if(!pose.Bone_Transform(part.bone,bone)) return false;
        visible=pose.Visible(part.bone);
        // Skin vertices already contain the evaluated skeleton transform.
        if(!part.skinning.empty()) return true;
        const auto world=parameters.world;
        for(unsigned r=0;r<4;++r) for(unsigned c=0;c<4;++c) {
            float value=0;for(unsigned k=0;k<4;++k)value+=world[r*4+k]*bone.matrix[k*4+c];
            parameters.world[r*4+c]=value;
        }
        return true;
    }
    static void Prepare(const Part& part,PropParameters& parameters) {
        const auto team_color=parameters.surface.team_color;
        parameters.surface=part.surface; parameters.surface.team_color=team_color;
        parameters.textured=part.textured ? 1 : 0;
        parameters.alpha_cutoff=part.alpha_cutoff;
    }
    bool Upload(Assets::AssetCache& assets,Assets::TextureAssetHandle handle,RHITextureHandle& result) {
        if(!handle.Is_Valid()) { result={}; return true; }
        for(const auto& entry:m_textures) if(entry.first==handle) { result=entry.second; return true; }
        const auto* source=assets.Try_Get_Texture(handle);
        if(!source || !source->Has_Pixels()) return false;
        result=m_device->Create_Texture_Initialized({source->Width(),source->Height(),1,RHITextureFormat::RGBA8_UNorm},
            {source->Pixels(),source->Row_Pitch()});
        if(!result.Is_Valid()) return false;
        m_textures.emplace_back(handle,result); return true;
    }
    Device* m_device=nullptr;
    PropRenderer* m_renderer=nullptr;
    ModelAssetPose m_rest_pose;
    std::vector<Part> m_parts;
    std::vector<std::pair<Assets::TextureAssetHandle,RHITextureHandle>> m_textures;
};
}

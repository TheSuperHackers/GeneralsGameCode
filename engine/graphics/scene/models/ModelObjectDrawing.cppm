module;
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <utility>
#include <vector>
export module Graphics.Scene.Models.ObjectDrawing;
import Graphics.RHI;
import Graphics.Materials.State;
import Graphics.Materials.Fog;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.MeshTextureMapping;
import Graphics.Resources.Textures.Sampling;
import Graphics.Scene.Models.MeshMaterialBindings;
import Graphics.Scene.Props.Geometry;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Material;
import Graphics.Scene.Props.MaterialDrawState;
import Graphics.Scene.Props.MaterialSubmission;
import Graphics.Scene.Props.Lighting;
import Graphics.Scene.DrawParameters;

namespace Graphics {
export struct ModelObjectDrawContext final {
    std::array<float,16> view_projection{};
    std::array<float,16> view{};
    std::array<float,16> projection{};
    std::array<float,4> shroud_projection{};
    std::array<float,3> camera{};
    PropLighting lighting;
    SceneFog fog;
    RHITextureHandle shroud;
    std::uint32_t milliseconds = 0;
    bool reflection = false;
};
// An object snapshot owns its converted geometry and material resources until
// cleared. Scene traversal and deciding when a snapshot changes belong to callers.
export template<class TextureOwner>
class ModelObjectDrawing final {
    struct Batch final {
        PropMeshHandle mesh;
        MaterialState shader;
        PropStyle style;
        PropParameters parameters;
        std::array<TextureOwner,2> textures;
        std::shared_ptr<MeshMaterial> material;
        MaterialFogMode fog = MaterialFogMode::Disabled;
        bool background = false;
    };
public:
    explicit ModelObjectDrawing(PropRenderer& renderer) : m_renderer(renderer) {}
    ~ModelObjectDrawing() { Clear(); }
    ModelObjectDrawing(const ModelObjectDrawing&) = delete;
    ModelObjectDrawing& operator=(const ModelObjectDrawing&) = delete;
    void Clear() {
        for (const auto& batch : m_batches) m_renderer.Destroy_Mesh(batch.mesh);
        m_batches.clear();
    }
    bool Valid() const {
        for (const auto& batch : m_batches) if (!m_renderer.Mesh_Geometry(batch.mesh)) return false;
        return true;
    }
    template<class Position,class Triangle,class UV>
    bool Append(std::span<const Position> positions, std::span<const Position> normals,
        std::span<const Triangle> triangles, MeshMaterialBindings<TextureOwner,UV>& materials,
        const std::array<float,16>& world, bool background)
    {
        const auto color = [](unsigned packed) {
            return std::array<float,4>{((packed>>16)&255)/255.f,((packed>>8)&255)/255.f,
                (packed&255)/255.f,((packed>>24)&255)/255.f};
        };
        for (int pass=0;pass<materials.Get_Pass_Count();++pass) {
            std::vector<PropVertex> vertices;
            std::vector<std::uint32_t> indices;
            Batch batch;
            bool pending=false;
            const auto flush = [&] {
                if (indices.empty()) return true;
                batch.mesh=m_renderer.Create_Mesh(vertices,indices);
                if (!batch.mesh.Is_Valid()) return false;
                m_batches.push_back(batch);
                vertices.clear(); indices.clear(); return true;
            };
            const auto* primary=materials.Peek_DCG_Array(pass);
            const auto* secondary=materials.Peek_DIG_Array(pass);
            const auto* uv=materials.Peek_UV_Array(pass,0);
            const auto* secondary_uv=materials.Peek_UV_Array(pass,1);
            for (std::size_t polygon=0;polygon<triangles.size();++polygon) {
                const auto shader=materials.Get_Shader(polygon,pass);
                const auto* first=materials.Peek_Texture(polygon,pass,0);
                const auto* second=materials.Peek_Texture(polygon,pass,1);
                const auto* polygon_material=materials.Peek_Material(triangles[polygon][0],pass);
                if (!pending || batch.shader!=shader || first!=std::to_address(batch.textures[0])
                    || second!=std::to_address(batch.textures[1]) || polygon_material!=batch.material.get()) {
                    if (!flush()) return false;
                    batch={}; pending=true; batch.shader=shader;
                    batch.textures={materials.Get_Texture(polygon,pass,0),materials.Get_Texture(polygon,pass,1)};
                    batch.material=materials.Get_Material(triangles[polygon][0],pass);
                    batch.background=background;
                    constexpr std::array fog_modes{MaterialFogMode::Disabled,MaterialFogMode::Scene,MaterialFogMode::Black,MaterialFogMode::White};
                    batch.fog=fog_modes[shader.Get_Fog_Func()];
                    batch.style=Resolve_Prop_Material_State(shader,{},false,batch.parameters);
                    batch.style.depth_write=!background && batch.style.depth_write;
                    batch.style.depth_test=!background;
                    if (background) batch.style.cull=RHICullMode::None;
                    batch.parameters.textured=first && shader.Get_Texturing()!=MaterialState::TEXTURING_DISABLE ? 1.f : 0.f;
                    batch.parameters.secondary_texture=second && shader.Get_Texturing()!=MaterialState::TEXTURING_DISABLE ? 1.f : 0.f;
                }
                for (unsigned corner=0;corner<3;++corner) {
                    const auto index=triangles[polygon][corner];
                    PropVertex vertex;
                    for (unsigned axis=0;axis<3;++axis) {
                        const auto row=axis*4;
                        vertex.position[axis]=world[row]*positions[index][0]+world[row+1]*positions[index][1]
                            +world[row+2]*positions[index][2]+world[row+3];
                        if (!normals.empty()) vertex.normal[axis]=world[row]*normals[index][0]+world[row+1]*normals[index][1]
                            +world[row+2]*normals[index][2];
                    }
                    if (primary) vertex.color=color(primary[index]);
                    if (secondary) vertex.secondary_color=color(secondary[index]);
                    if (const auto* material=materials.Peek_Material(index,pass)) {
                        auto values=material->parameters;
                        values.lighting=values.lighting && !background;
                        Apply_Prop_Material(vertex,values);
                    }
                    if (uv) vertex.uv={uv[index][0],uv[index][1]};
                    if (secondary_uv) vertex.secondary_uv={secondary_uv[index][0],secondary_uv[index][1]};
                    indices.push_back(static_cast<std::uint32_t>(vertices.size())); vertices.push_back(vertex);
                }
            }
            if (!flush()) return false;
        }
        return true;
    }
    template<class Resolve>
    bool Draw(CommandList& commands,const ModelObjectDrawContext& context,Resolve&& resolve) {
        for (const auto& batch:m_batches) {
            auto parameters=batch.parameters;
            parameters.view_projection=context.view_projection; parameters.view=context.view;
            parameters.shroud_projection=context.shroud_projection;
            parameters.camera_position={context.camera[0],context.camera[1],context.camera[2],1};
            const auto fog=Resolve_Material_Fog(batch.background ? SceneFog{} : context.fog,batch.fog);
            parameters.fog_state=fog.state; parameters.fog_color=fog.color;
            Extract_Mesh_Texture_Mappings(parameters,batch.material.get(),context.milliseconds,context.view,context.projection);
            parameters.scene_ambient={context.lighting.ambient[0],context.lighting.ambient[1],context.lighting.ambient[2],0};
            for (unsigned i=0;i<4;++i) {
                const auto& light=context.lighting.lights[i]; const auto& direction=light.direction;
                const float enabled=direction[0]*direction[0]+direction[1]*direction[1]+direction[2]*direction[2]>0.000001f ? 1.f : 0.f;
                parameters.light_direction[i]={direction[0],direction[1],direction[2],enabled};
                parameters.light_diffuse[i]={light.diffuse[0],light.diffuse[1],light.diffuse[2],0};
                parameters.light_specular[i]={light.specular[0],light.specular[1],light.specular[2],0};
            }
            std::array<RHITextureHandle,4> textures{RHITextureHandle{},RHITextureHandle{},RHITextureHandle{},context.shroud};
            auto style=batch.style;
            for (unsigned stage=0;stage<2;++stage) {
                if (!batch.textures[stage]) continue;
                const auto texture=resolve(std::to_address(batch.textures[stage]));
                textures[stage]=texture.texture;
                style.samplers[stage]=Resolve_Texture_Sampling(texture.sampling,Get_Texture_Sampling_Settings(),stage==0);
                if (batch.background) style.samplers[stage].address[0]=style.samplers[stage].address[1]=RHISamplerAddress::Clamp;
            }
            if (style.cull!=RHICullMode::None) style.front_counter_clockwise=!context.reflection;
            if (!Draw_Prop(m_renderer,commands,batch.mesh,style,parameters,textures,context.shroud.Is_Valid() && !batch.background)) return false;
        }
        return true;
    }
private:
    PropRenderer& m_renderer;
    std::vector<Batch> m_batches;
};
}

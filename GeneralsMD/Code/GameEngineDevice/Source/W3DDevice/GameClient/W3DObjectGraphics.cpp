import Graphics.Frame.RenderClock;
#include "W3DDevice/GameClient/W3DRenderServices.h"
import Graphics.Scene.AffineTransform;
import Graphics.Scene.Props.MaterialSubmission;
import Graphics.Scene.Models.ObjectDrawing;
import Graphics.Materials.MeshTextureMapping;
import Graphics.Materials.State;
#include <array>
#include "rts/profile.h"
#include <span>
#include <vector>
#include <cstring>
#include <algorithm>
#include "W3DDevice/GameClient/W3DObjectGraphics.h"
#include "W3DDevice/GameClient/W3DGraphicsResources.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/W3DMeshRenderObject.h"
#include "W3DDevice/GameClient/W3DMeshResource.h"
#include "W3DDevice/GameClient/W3DHierarchyRenderObject.h"
import Graphics.Scene.Props.Renderer;
import Graphics.Materials.MeshMaterial;
import Graphics.Scene.Props.Material;
#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DSceneClass.h"

#include <algorithm>
import Graphics.Scene.Views.CameraMatrices;
import Graphics.Scene.DrawParameters;
import Graphics.Frame.Runtime;
import Graphics.Materials.Fog;

struct W3DObjectGraphics::State
{
    Graphics::ModelObjectDrawing<RefCountPtr<W3DTextureHandle>> drawing{Graphics::Get_Prop_Renderer()};
    bool dirty = true;
    bool animated = false;
    bool background = false;
    Matrix3D transform{true};
    Vector3 camera{0,0,0};
    Graphics::PropLighting lighting{};
    int lod = -1;

    void Clear() { drawing.Clear(); }

    bool Extract(W3DRenderObject& object, W3DRenderContext& info)
    {
        PROFILER_SECTION_NAME("Graphics.Objects.Extract");
        if (object.Is_Hidden()) return true;
        object.Validate_Transform();
        if (object.Class_ID() == W3DRenderObject::CLASSID_HLOD) {
            animated = true;
            auto& hierarchy = static_cast<W3DHierarchyRenderObject&>(object);
            hierarchy.Get_Bone_Transform(0);
            const int level = hierarchy.Get_LOD_Level();
            for (int i=0;i<hierarchy.Get_Lod_Model_Count(level);++i)
                if (!Extract(*hierarchy.Peek_Lod_Model(level,i),info)) return false;
            for (int i=0;i<hierarchy.Get_Additional_Model_Count();++i)
                if (!Extract(*hierarchy.Peek_Additional_Model(i),info)) return false;
            return true;
        }
        if (object.Class_ID() != W3DRenderObject::CLASSID_MESH) {
            for (int i=0;i<object.Get_Num_Sub_Objects();++i) {
                W3DRenderObject* child = object.Get_Sub_Object(i);
                const bool success = child == nullptr || Extract(*child,info);
                REF_PTR_RELEASE(child);
                if (!success) return false;
            }
            return true;
        }
        auto& mesh = static_cast<W3DMeshRenderObject&>(object);
        W3DMeshResource* model = mesh.Peek_Model();
        if (!model || model->Get_Vertex_Count()==0) return true;
        const auto* positions = model->Peek_Vertex_Array();
        const auto* normals = model->Get_Vertex_Normal_Array();
        const auto* polygons = model->Get_Polygon_Array();
        Matrix3D world = object.Get_Transform();
        std::vector<Vector3> deformed_positions;
        std::vector<Vector3> deformed_normals;
        if (model->Get_Flag(W3DMeshGeometry::ALIGNED)) {
            animated = true;
            Vector3 direction;
            info.Camera.Get_Transform().Get_Z_Vector(&direction);
            const Vector3 position=world.Get_Translation();
            world.Obj_Look_At(position,position+direction,0.0f);
        } else if (model->Get_Flag(W3DMeshGeometry::ORIENTED)) {
            animated = true;
            const Vector3 position=world.Get_Translation();
            world.Obj_Look_At(position,info.Camera.Get_Position(),0.0f);
        } else if (model->Get_Flag(W3DMeshGeometry::SKIN)) {
            animated = true;
            deformed_positions.resize(model->Get_Vertex_Count());
            deformed_normals.resize(model->Get_Vertex_Count());
            mesh.Get_Deformed_Vertices(deformed_positions.data(),deformed_normals.data());
            positions = deformed_positions.data();
            normals = deformed_normals.data();
            world.Make_Identity();
        }
        return drawing.Append(std::span(positions,static_cast<std::size_t>(model->Get_Vertex_Count())),
            std::span(normals,normals ? static_cast<std::size_t>(model->Get_Vertex_Count()) : 0),
            std::span(polygons,static_cast<std::size_t>(model->Get_Polygon_Count())), model->Material_Bindings(),
            Graphics::Import_Affine_Transform(world).matrix, background);
    }
};

W3DObjectGraphics::W3DObjectGraphics() : m_state(std::make_unique<State>()) {}
W3DObjectGraphics::~W3DObjectGraphics() = default;
void W3DObjectGraphics::Mark_Muzzle_Flash(W3DRenderObject& object)
{
    if (object.Class_ID() == W3DRenderObject::CLASSID_MESH) {
        static_cast<W3DMeshRenderObject&>(object).Set_Muzzle_Flash_Designation(
            Graphics::MuzzleFlashDesignation::Rotating);
        return;
    }
    for (int index=0; index<object.Get_Num_Sub_Objects(); ++index) {
        W3DRenderObject* child = object.Get_Sub_Object(index);
        if (child != nullptr) {
            Mark_Muzzle_Flash(*child);
            child->Release_Ref();
        }
    }
}
void W3DObjectGraphics::Invalidate() { m_state->dirty = true; }
bool W3DObjectGraphics::Render(W3DRenderObject& object, W3DRenderContext& info,
    const Graphics::PropLighting& lighting, W3DShroud* shroud, bool background)
{
    auto* device = Graphics::Shared_Frame_Device();
    if (!device) return false;
    const auto& transform = object.Get_Transform();
    const auto camera = info.Camera.Get_Position();
    if (m_state->dirty || m_state->animated || !m_state->drawing.Valid() || m_state->background!=background
        || m_state->lod!=object.Get_LOD_Level()
        || std::memcmp(&m_state->transform,&transform,sizeof(transform))!=0) {
        m_state->Clear();
        m_state->transform = transform; m_state->camera = camera; m_state->lighting = lighting;
        m_state->background = background; m_state->lod = object.Get_LOD_Level();
        m_state->animated = false;
        if (!m_state->Extract(object,info)) { m_state->dirty=true; return false; }
        m_state->dirty = false;
    }
    auto surface = Make_Surface_Parameters(info.Camera);
    const auto shroud_texture = Set_Surface_Shroud(surface,shroud);
    Graphics::ModelObjectDrawContext context;
    context.view_projection = surface.view_projection;
    context.shroud_projection = surface.shroud_projection;
    context.camera = {camera.X,camera.Y,camera.Z};
    context.view = Graphics::Get_Camera_Matrices().view.values;
    context.projection = Graphics::Get_Camera_Matrices().projection.values;
    context.lighting = lighting;
    context.shroud = shroud_texture;
    context.milliseconds = Graphics::Get_Render_Clock().Sync_Time();
    context.reflection = Get_W3D_Render_Services().Is_Reflection_Render_Pass();
    if (!background && Graphics::Get_Scene_Draw_Parameters().fog.enabled) {
        W3DScene* scene = object.Peek_Scene();
        if (!scene) scene = info.Camera.Peek_Scene();
        if (!scene && TheTerrainRenderObject) scene = TheTerrainRenderObject->Peek_Scene();
        if (scene) {
            scene->Get_Fog_Range(&context.fog.start,&context.fog.end);
            context.fog.enabled = true;
            const auto& fog = scene->Get_Fog_Color();
            context.fog.color = {fog.X,fog.Y,fog.Z,1};
        }
    }
    return m_state->drawing.Draw(device->Immediate_Command_List(),context,[](W3DTextureHandle* texture) {
        return Graphics::PropMaterialTexture{Resolve_Graphics_Texture(texture),texture->Get_Sampling()};
    });
}

import Graphics.Frame.RenderClock;
import Graphics.Frame.RenderSettings;
#include "W3DDevice/GameClient/W3DRenderServices.h"
#include <optional>
#include <array>
import Graphics.Frame.Runtime;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Props.MaterialSubmission;
#include <memory>
#include <span>
#include <vector>
#include "rts/profile.h"
#include "W3DDevice/GameClient/W3DMeshDrawing.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "W3DDevice/GameClient/W3DMeshRenderObject.h"
#include "W3DDevice/GameClient/W3DMeshResource.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"

import Graphics.Materials.ProceduralPass;
import Graphics.Scene.Models.MeshDrawing;
import Graphics.Scene.Views.CameraMatrices;
import Graphics.Scene.Props.Extraction;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.SkinPalettes;
import Graphics.Scene.Props.LightingParameters;

bool Draw_W3D_Mesh(W3DMeshRenderObject& mesh, W3DRenderContext& info, const Graphics::ModelMeshDrawOverrides& overrides)
{
    PROFILER_SECTION_NAME("Graphics.Mesh.ExtractDraw");
    if (mesh.Get_Muzzle_Flash_Designation() != Graphics::MuzzleFlashDesignation::None && overrides.shadow_capture) return true;
    auto* model = mesh.Peek_Model();
    if (!model || model->Get_Vertex_Count() == 0 || model->Get_Polygon_Count() == 0) return true;
    mesh.Validate_Transform();
    Matrix3D world = mesh.Get_Transform();
    const auto* positions = model->Peek_Vertex_Array();
    const auto* normals = model->Get_Vertex_Normal_Array();
    const auto source_revision = model->Geometry_Revision();
    const auto geometry_revision = source_revision;
    Graphics::PropSkinPaletteHandle skin_palette{};
    std::span<const std::uint16_t> bone_links;
    const bool skin = model->Get_Flag(W3DMeshGeometry::SKIN) != 0;
    if (model->Get_Flag(W3DMeshGeometry::ALIGNED)) {
        Vector3 direction;
        info.Camera.Get_Transform().Get_Z_Vector(&direction);
        const auto position = world.Get_Translation();
        world.Obj_Look_At(position, position + direction, 0);
    } else if (model->Get_Flag(W3DMeshGeometry::ORIENTED)) {
        const auto position = world.Get_Translation();
        world.Obj_Look_At(position, info.Camera.Get_Position(), 0);
    } else if (skin) {
        PROFILER_SECTION_NAME("Graphics.Mesh.SkinPalette");
        const auto vertex_count = static_cast<std::size_t>(model->Get_Vertex_Count());
        auto* container = mesh.Get_Container();
        WWASSERT(container && container->Get_Model_Hierarchy());
        const auto& hierarchy=*container->Get_Model_Hierarchy();
        bone_links=std::span(model->Get_Vertex_Bone_Links(),vertex_count);
        skin_palette=mesh.Graphics_Skin().Update(Graphics::Get_Prop_Renderer().Instances().Palettes(),
            static_cast<std::size_t>(hierarchy.Bone_Count()),
            [&](std::size_t bone) -> const auto& { return hierarchy.World_Transform(static_cast<int>(bone)).matrix; });
        world.Make_Identity();
    }
    const Graphics::PropSkinLease skin_lease(Graphics::Get_Prop_Renderer().Instances().Palettes(),skin_palette);
    const auto& camera = Graphics::Get_Camera_Matrices();
    const auto view_projection = Graphics::Compose_Matrices(camera.projection, camera.view);
    Graphics::ModelMeshDrawContext context;
    context.parameters.normal_in_world_space = normals ? 0.0f : 1.0f;
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < 4; ++column)
            context.parameters.world[row * 4 + column] = world[row][column];
    context.parameters.view = camera.view.values;
    context.parameters.view_projection = view_projection.values;
    const auto camera_position = info.Camera.Get_Position();
    context.parameters.camera_position = {camera_position.X, camera_position.Y, camera_position.Z, 1};
    Graphics::Set_Prop_Lighting(context.parameters, mesh.Get_Lighting_Environment());
    context.projection = camera.projection.values;
    context.milliseconds = Graphics::Get_Render_Clock().Sync_Time();
    context.sorted = model->Get_Flag(W3DMeshGeometry::SORT) && Graphics::Get_Render_Settings().Is_Sorting_Enabled();
    context.additional_only = info.Has_Override(Graphics::DrawOverride::AdditionalPassesOnly);
    context.shadow = info.Has_Override(Graphics::DrawOverride::ShadowRendering);
    context.two_sided = info.Has_Override(Graphics::DrawOverride::ForceTwoSided);
    context.additive = mesh.Is_Additive();
    context.translucent = mesh.Is_Translucent();
    context.muzzle_flash = mesh.Get_Muzzle_Flash_Designation();
    context.overrides = overrides;
    context.bone_links = bone_links;
    const auto* user_data = mesh.Get_User_Data();
    if (user_data && *static_cast<const int*>(user_data) == W3DRenderObject::USER_DATA_MATERIAL_OVERRIDE) {
        const auto& offset = static_cast<const W3DRenderObject::Material_Override*>(user_data)->customUVOffset;
        context.uv_offset = {offset.X, offset.Y};
    }
    auto& state = model->Graphics_Mesh_State();
    if (!state) state = std::make_unique<Graphics::ModelMeshState<RefCountPtr<W3DTextureHandle>>>();
    const auto vertex_count = static_cast<std::size_t>(model->Get_Vertex_Count());
    Graphics::ModelMeshDrawing drawing(std::span(positions, vertex_count),
        std::span(normals, normals ? vertex_count : 0),
        std::span(model->Get_Polygon_Array(), static_cast<std::size_t>(model->Get_Polygon_Count())),
        geometry_revision, model->Material_Bindings(), *state,
        Graphics::Get_Prop_Renderer(), Graphics::Get_Prop_Extraction_Cache(), context, source_revision);
    auto* device = Graphics::Shared_Frame_Device();
    if (!device) return false;
    Graphics::PropMaterialDrawContext submission_context;
    submission_context.batchable = true;
    submission_context.scene = Graphics::Get_Scene_Draw_Parameters();
    submission_context.reflection = Get_W3D_Render_Services().Is_Reflection_Render_Pass();
    submission_context.milliseconds = context.milliseconds;
    if (context.sorted) submission_context.sorting_depth = {camera.view.values[8], camera.view.values[9],
        camera.view.values[10], camera.view.values[11]};
    const auto submit = [&](auto vertices, auto indices, auto shader, auto textures,
        auto parameters, auto draw_overrides) {
        draw_overrides.instance = &mesh.Graphics_Instance();
        draw_overrides.skin = skin_palette;
        return Graphics::Submit_Prop_Material(*device, Graphics::Get_Prop_Renderer(), Graphics::Get_Prop_Submission(),
            vertices, indices, shader, textures,
            [](W3DTextureHandle* source, bool load) -> std::optional<Graphics::PropMaterialTexture> {
                if (load && !source->Ensure_Render_Backend_Texture()) return std::nullopt;
                return Graphics::PropMaterialTexture{source->Peek_Graphics_Texture(), source->Get_Sampling()};
            }, parameters, submission_context, draw_overrides);
    };
    bool success = drawing.Draw_Base(submit);
    for (int pass_index = 0; !overrides.shadow_capture && pass_index < info.Additional_Pass_Count(); ++pass_index) {
        auto* pass = info.Peek_Additional_Pass(pass_index);
        if (!drawing.Accepts_Additional_Pass(pass->enabled_on_translucent)) continue;
        NativeMaterialPass::Description description;
        if (!pass->Describe(description)) { success = false; continue; }
        if (!skin && pass->cull_bounds) {
            SimpleDynVecClass<uint32> selected;
            Matrix3D inverse;
            mesh.Get_Transform().Get_Orthogonal_Inverse(inverse);
            OBBoxClass local_box;
            OBBoxClass::Transform(inverse, *pass->cull_bounds, &local_box);
            Vector3 direction;
            local_box.Basis.Get_Z_Vector(&direction);
            direction = -direction;
            if (model->Has_Cull_Tree()) model->Generate_Rigid_APT(local_box, direction, selected);
            else model->Generate_Rigid_APT(direction, selected);
            if (!drawing.Draw_Additional(description,
                std::span<const uint32>(selected.Count() ? &selected[0] : nullptr, static_cast<std::size_t>(selected.Count())),
                pass_index, submit)) success = false;
        } else {
            const auto polygons = state->Complete_Polygons(static_cast<std::size_t>(model->Get_Polygon_Count()));
            if (!drawing.Draw_Additional(description,polygons,pass_index,submit,
                state->Complete_Polygon_Revision())) success = false;
        }
    }
    return success;
}

void Flush_Before_W3D_Object_Draw(const W3DRenderObject& object)
{
    const auto type = object.Class_ID();
    if (type != W3DRenderObject::CLASSID_MESH && type != W3DRenderObject::CLASSID_HLOD
        && type != W3DRenderObject::CLASSID_COLLECTION)
        Graphics::Get_Prop_Submission().Flush_Batches();
}

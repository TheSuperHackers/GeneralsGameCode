import Graphics.Frame.RenderClock;
#include "W3DDevice/GameClient/W3DRenderServices.h"
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
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include "W3DDevice/GameClient/W3DDazzleRenderObject.h"
#include "W3DDevice/GameClient/W3DDazzleResources.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DSceneClass.h"

#include "WWLib/chunkio.h"
import Assets.Adapters.W3D.Dazzle;
import Graphics.Frame.Runtime;
import Graphics.Frame.AttachmentBindings;
import Graphics.Scene.Views.CameraMatrices;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Props.MaterialSubmission;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Props.Renderer;

namespace {
W3DDazzleLayer* current_layer = nullptr;
W3DDazzleVisibility default_visibility;
const W3DDazzleVisibility* visibility_handler = &default_visibility;
bool rendering_enabled = true;
}
W3DDazzleRenderObject::W3DDazzleRenderObject(unsigned type) : m_type(type) {
    const auto& resources = Get_Dazzle_Resources();
    if (type < resources.Size()) m_radius = resources.Definition(type).radius;
    m_state.creation_time = Graphics::Get_Render_Clock().Sync_Time();
}
W3DDazzleRenderObject::W3DDazzleRenderObject(const char* name) : W3DDazzleRenderObject(Get_Type_ID(name)) {}
W3DDazzleRenderObject::W3DDazzleRenderObject(const W3DDazzleRenderObject& source)
    : m_type(source.m_type), m_radius(source.m_radius), m_state(source.m_state) {
    m_state.halo = 0;
    m_state.screen_position = {};
    m_state.creation_time = Graphics::Get_Render_Clock().Sync_Time();
}
W3DDazzleRenderObject& W3DDazzleRenderObject::operator=(const W3DDazzleRenderObject& source) {
    m_type = source.m_type;
    m_radius = source.m_radius;
    const auto screen_position = m_state.screen_position;
    const float halo = m_state.halo;
    m_state = source.m_state;
    m_state.screen_position = screen_position;
    m_state.halo = halo;
    m_state.creation_time = Graphics::Get_Render_Clock().Sync_Time();
    return *this;
}
W3DRenderObject* W3DDazzleRenderObject::Clone() const { return NEW_REF(W3DDazzleRenderObject, (*this)); }
void W3DDazzleRenderObject::Get_Obj_Space_Bounding_Sphere(SphereClass& sphere) const {
    sphere.Center.Set(0, 0, 0);
    sphere.Radius = m_radius*m_state.scale;
}
void W3DDazzleRenderObject::Get_Obj_Space_Bounding_Box(AABoxClass& box) const {
    box.Center.Set(0, 0, 0);
    box.Extent.Set(m_radius, m_radius, m_radius);
    box.Extent *= m_state.scale;
}
void W3DDazzleRenderObject::Set_Transform(const Matrix3D& transform) {
    W3DRenderObject::Set_Transform(transform);
    const auto& resources = Get_Dazzle_Resources();
    if (m_type >= resources.Size()) return;
    std::array<float, 16> matrix{};
    std::copy_n(&transform[0][0], 12, matrix.begin());
    matrix[15] = 1;
    Graphics::Set_Dazzle_Direction(m_state, resources.Definition(m_type), matrix);
}
void W3DDazzleRenderObject::Render(W3DRenderContext& info) {
    if (!Is_Not_Hidden_At_All() || !rendering_enabled || Graphics::Get_Attachment_Bindings().Offscreen()) {
        m_state.visibility = 0;
        return;
    }
    const auto& resources = Get_Dazzle_Resources();
    if (m_type >= resources.Size()) return;
    Graphics::DazzleView view;
    const auto& matrices = Graphics::Get_Camera_Matrices();
    view.view = matrices.view.values;
    view.projection = matrices.projection.values;
    const auto camera = info.Camera.Get_Position();
    view.camera_position = {camera.X, camera.Y, camera.Z};
    view.milliseconds = Graphics::Get_Render_Clock().Sync_Time();
    view.frame_milliseconds = Graphics::Get_Render_Clock().Logic_Frame_Time_Milliseconds();
    const auto position = Get_Position();
    if (Graphics::Prepare_Dazzle(m_state, resources.Definition(m_type), {position.X, position.Y, position.Z}, view,
        [&] { return visibility_handler->Compute_Dazzle_Visibility(info, this, position); })) Set_Layer(current_layer);
}
void W3DDazzleRenderObject::Set_Layer(W3DDazzleLayer* layer) {
    if (m_membership.queued) return;
    WWASSERT(layer);
    if (!layer || m_type >= Get_Dazzle_Resources().Size()) return;
    layer->m_layer.Queue(m_type, m_membership, [&] { return RefCountPtr<W3DDazzleRenderObject>::Create_Add_Ref(this); });
}
unsigned W3DDazzleRenderObject::Get_Type_ID(const char* name) { return Get_Dazzle_Resources().Find(name ? name : ""); }
const char* W3DDazzleRenderObject::Get_Type_Name(unsigned type) {
    const auto& resources = Get_Dazzle_Resources();
    return type < resources.Size() ? resources.Definition(type).name.c_str() : "DEFAULT";
}
void W3DDazzleRenderObject::Set_Current_Dazzle_Layer(W3DDazzleLayer* layer) { current_layer = layer; }
void W3DDazzleRenderObject::Install_Dazzle_Visibility_Handler(const W3DDazzleVisibility* handler) {
    visibility_handler = handler ? handler : &default_visibility;
}
void W3DDazzleRenderObject::Enable_Dazzle_Rendering(bool enabled) { rendering_enabled = enabled; }
bool W3DDazzleRenderObject::Is_Dazzle_Rendering_Enabled() { return rendering_enabled; }

W3DDazzleLayer::W3DDazzleLayer() : m_layer(Get_Dazzle_Resources().Size()) {}
W3DDazzleLayer::~W3DDazzleLayer() { if (current_layer == this) current_layer = nullptr; }
void W3DDazzleLayer::Render(W3DCamera* camera) {
    if (!camera) return;
    camera->Apply();
    auto* device = Graphics::Shared_Frame_Device();
    auto& resources = Get_Dazzle_Resources();
    const auto& viewport = Graphics::Get_Attachment_Bindings().Default().viewport;
    Graphics::PropMaterialDrawContext context;
    context.scene = Graphics::Get_Scene_Draw_Parameters();
    context.reflection = Get_W3D_Render_Services().Is_Reflection_Render_Pass();
    context.milliseconds = Graphics::Get_Render_Clock().Sync_Time();
    m_layer.Draw_All([&](const RefCountPtr<W3DDazzleRenderObject>& object) {
        if (!device) return;
        const unsigned type = object->m_type;
        m_drawing.Prepare(resources.Definition(type), object->m_state, resources.Sprites(type), viewport.width, viewport.height);
        [[maybe_unused]] const bool drawn = m_drawing.Draw(*device, Graphics::Get_Prop_Renderer(), Graphics::Get_Prop_Submission(),
            [&](Graphics::DazzleImage image) { return resources.Texture(type, image, Acquire_Dazzle_Texture); },
            [](W3DTextureHandle* source, bool load) -> std::optional<Graphics::PropMaterialTexture> {
                if (load && !source->Ensure_Render_Backend_Texture()) return std::nullopt;
                return Graphics::PropMaterialTexture{source->Peek_Graphics_Texture(), source->Get_Sampling()};
            }, context);
        WWASSERT(drawn);
    });
}
float W3DDazzleVisibility::Compute_Dazzle_Visibility(W3DRenderContext& info, W3DDazzleRenderObject* dazzle, const Vector3& point) const {
    W3DScene* scene = dazzle->Get_Scene();
    auto* container = dazzle->Get_Container();
    while (!scene && container) { scene = container->Get_Scene(); container = container->Get_Container(); }
    if (!scene) return 1;
    const float visibility = scene->Compute_Point_Visibility(info, point);
    scene->Release_Ref();
    return visibility;
}
Graphics::ModelFactory<W3DRenderObject>* Load_Dazzle_Factory(ChunkLoadClass& chunks) {
    std::vector<std::byte> bytes(chunks.Cur_Chunk_Length());
    if (chunks.Read(bytes.data(), static_cast<unsigned>(bytes.size())) != bytes.size()) return nullptr;
    Assets::W3D::W3DDazzleReference definition;
    if (!Assets::W3D::W3DRead_Dazzle(bytes, definition)) return nullptr;
    unsigned type = W3DDazzleRenderObject::Get_Type_ID(definition.type.c_str());
    if (type == Graphics::Invalid_Dazzle_Type) type = 0;
    return new Graphics::ModelFactory<W3DRenderObject>(std::move(definition.name), W3DRenderObject::CLASSID_DAZZLE,
        [type] { return NEW_REF(W3DDazzleRenderObject, (type)); });
}

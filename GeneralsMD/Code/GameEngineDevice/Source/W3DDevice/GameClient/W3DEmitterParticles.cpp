import Graphics.Frame.RenderClock;
import Graphics.Frame.RenderSettings;
#include "W3DDevice/GameClient/W3DRenderServices.h"
#include <array>
#include <climits>
#include <optional>
#include <span>
import Graphics.Frame.Runtime;
import Graphics.Materials.Ordering;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.OrderedDraws;
import Graphics.Scene.Props.MaterialSubmission;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Views.CameraMatrices;
#include "W3DDevice/GameClient/W3DEmitterParticles.h"
#include "W3DDevice/GameClient/W3DEmitterRenderObject.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DSceneClass.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"

#include "WWLib/RANDOM.h"
#include "WWMath/matrix4.h"

namespace {

Random4Class visual_random;

std::array<float, 1> Sample_Scalar(std::array<float, 1> scale)
{
    return {visual_random * scale[0]};
}

std::array<float, 3> Sample_Color(std::array<float, 3> scale)
{
    const Vector3 value(visual_random * scale[0], visual_random * scale[1], visual_random * scale[2]);
    return {value.X, value.Y, value.Z};
}

std::array<float, 3> Acceleration(const Assets::EmitterAssetDesc &description)
{
    const Vector3 value = Vector3(description.acceleration.x, description.acceleration.y,
        description.acceleration.z) / 1000000.0f;
    return {value.X, value.Y, value.Z};
}

std::uint32_t Lifetime(const Assets::EmitterAssetDesc &description)
{
    return static_cast<std::uint32_t>(1000 * (description.lifetime > 0 ? description.lifetime : 1.0f));
}

template<class Matrix>
Graphics::Matrix4x4 Copy_Matrix(const Matrix &source)
{
    Graphics::Matrix4x4 matrix;
    for (unsigned row = 0; row < 4; ++row)
        for (unsigned column = 0; column < 4; ++column)
            matrix.values[row * 4 + column] = source[row][column];
    return matrix;
}

struct LineRandom final {
    std::optional<Random3Class> frozen;
    std::optional<Vector3SolidBoxRandomizer> randomizer;
    void Reset(std::size_t)
    {
        frozen.emplace();
        randomizer.emplace(Vector3(1, 1, 1));
    }
    std::array<float, 3> Next(bool freeze)
    {
        Vector3 value;
        if (freeze) {
            const float inverse = 1.0f / static_cast<float>(INT_MAX);
            value.Set(*frozen * inverse, *frozen * inverse, *frozen * inverse);
        } else
            randomizer->Get_Vector(value);
        return {value.X, value.Y, value.Z};
    }
};

}

W3DEmitterParticles::W3DEmitterParticles(const Assets::EmitterAssetDesc &description,
    W3DTextureHandle *texture, Graphics::MaterialState shader, W3DEmitterRenderObject *emitter)
    : m_kinematics(Graphics::Emitter_Buffer_Capacity(description), Lifetime(description),
          Acceleration(description), false, Graphics::Get_Render_Clock().Sync_Time()),
      m_visuals(description, m_kinematics.Capacity(), m_kinematics.Lifetime(), Sample_Scalar, Sample_Color),
      m_renderer(description, shader, m_kinematics.Capacity(), Graphics::Get_Render_Clock().Logic_Time_Milliseconds()),
      m_detail(m_kinematics.Capacity(), description.geometry_mode), m_emitter(emitter)
{
    m_texture.Assign_Add_Ref(texture);
    // Trail endpoints are not part of the retained head-position bounds.
    if (m_visuals.Is_Line_Group())
        Set_Force_Visible(1);
}

W3DEmitterParticles::W3DEmitterParticles(const W3DEmitterParticles &source,
    W3DEmitterRenderObject *emitter)
    : W3DRenderObject(source), m_kinematics(source.m_kinematics, Graphics::Get_Render_Clock().Sync_Time()),
      m_visuals(source.m_visuals), m_renderer(source.m_renderer), m_detail(source.m_detail),
      m_texture(source.m_texture), m_emitter(emitter)
{
}

W3DEmitterParticles::~W3DEmitterParticles() = default;
W3DRenderObject *W3DEmitterParticles::Clone() const { return new W3DEmitterParticles(*this, nullptr); }

void W3DEmitterParticles::Notify_Added(W3DScene *scene)
{
    W3DRenderObject::Notify_Added(scene);
    scene->Register(this, W3DScene::ON_FRAME_UPDATE);
}

void W3DEmitterParticles::Notify_Removed(W3DScene *scene)
{
    scene->Unregister(this, W3DScene::ON_FRAME_UPDATE);
    W3DRenderObject::Notify_Removed(scene);
}

void W3DEmitterParticles::On_Frame_Update()
{
    Invalidate_Cached_Bounding_Volumes();
    if (m_emitter)
        m_emitter->Emit(m_kinematics);
    if (Is_Complete())
        Scene->Register(this, W3DScene::RELEASE);
}

void W3DEmitterParticles::Render(W3DRenderContext &info)
{
    const auto layer = Graphics::Get_Render_Settings().Is_Sorting_Enabled() ? 0
        : Graphics::Material_Ordered_Layer(m_renderer.Shader());
    if (layer != 0 && Graphics::Get_Scene_Draw_Queue().Is_Enabled()) {
        Graphics::Get_Scene_Draw_Queue().Enqueue<Extract_Ordered_Draw>(layer, *this);
        return;
    }
    if (m_kinematics.Advance(Graphics::Get_Render_Clock().Sync_Time(), Get_W3D_Render_Services().Frame_Count()))
        m_bounds_dirty = true;
    if (m_detail.Decimation() < m_detail.Count() - 1)
        m_visuals.Evaluate(m_kinematics, Graphics::Get_Render_Clock().Sync_Time(), Get_W3D_Render_Services().Frame_Count());
    Update_Bounds();
    Submit(info);
}

void W3DEmitterParticles::Submit(W3DRenderContext &info)
{
    auto *device = Graphics::Shared_Frame_Device();
    if (!device)
        return;
    Graphics::EmitterDrawInput input;
    Matrix4x4 projection;
    info.Camera.Get_Backend_Projection_Matrix(&projection);
    input.projection = Copy_Matrix(projection);
    input.view = Graphics::Get_Camera_Matrices().view;
    input.world = Copy_Matrix(Matrix4x4(Get_Transform()));
    Matrix3D rotation = info.Camera.Get_Transform();
    rotation.Set_Translation(Vector3(0, 0, 0));
    rotation.Get_Orthogonal_Inverse(rotation);
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < 3; ++column)
            input.line_rotation[row * 3 + column] = rotation[row][column];
    input.scene = Graphics::Get_Scene_Draw_Parameters();
    input.rendered_frame = Get_W3D_Render_Services().Frame_Count();
    input.sync_time = Graphics::Get_Render_Clock().Sync_Time();
    input.logic_time = Graphics::Get_Render_Clock().Logic_Time_Milliseconds();
    input.reflection = Get_W3D_Render_Services().Is_Reflection_Render_Pass();
    input.sorting_enabled = Graphics::Get_Render_Settings().Is_Sorting_Enabled();
    if (m_emitter) {
        const Vector3 position = m_emitter->Get_Position();
        input.source_position = {position.X, position.Y, position.Z};
        input.source_group = m_emitter->Group();
        input.source_active = !m_emitter->Is_Stopped();
    }
    const auto resolve = [](W3DTextureHandle *source, bool load)
        -> std::optional<Graphics::PropMaterialTexture> {
        if (load && !source->Ensure_Render_Backend_Texture())
            return std::nullopt;
        return Graphics::PropMaterialTexture{source->Peek_Graphics_Texture(), source->Get_Sampling()};
    };
    LineRandom random;
    m_renderer.Submit(*device, Graphics::Get_Prop_Renderer(), Graphics::Get_Prop_Submission(),
        m_kinematics, m_visuals, m_detail.Decimation(), input, m_texture.Peek(), resolve,
        [&](bool freeze) { return random.Next(freeze); },
        [&](std::size_t chunk) { random.Reset(chunk); });
}

void W3DEmitterParticles::Scale(float scale)
{
    m_visuals.Scale(scale);
    m_renderer.Scale(scale);
    m_kinematics.Scale_Acceleration(scale);
}

void W3DEmitterParticles::Update_Bounds()
{
    if (m_kinematics.Advance(Graphics::Get_Render_Clock().Sync_Time(), Get_W3D_Render_Services().Frame_Count()))
        m_bounds_dirty = true;
    if (!m_bounds_dirty)
        return;
    const auto bounds = m_kinematics.Bounds(m_visuals.Max_Size(), Get_W3D_Render_Services().Frame_Count());
    m_bounds.Init(MinMaxAABoxClass(Vector3(bounds.minimum.x, bounds.minimum.y, bounds.minimum.z),
        Vector3(bounds.maximum.x, bounds.maximum.y, bounds.maximum.z)));
    m_bounds_dirty = false;
}

void W3DEmitterParticles::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    const_cast<W3DEmitterParticles *>(this)->Update_Bounds();
    sphere.Init(m_bounds.Center, m_bounds.Extent.Length());
}

void W3DEmitterParticles::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    const_cast<W3DEmitterParticles *>(this)->Update_Bounds();
    box = m_bounds;
}

void W3DEmitterParticles::Update_Cached_Bounding_Volumes() const
{
    const_cast<W3DEmitterParticles *>(this)->Update_Bounds();
    CachedBoundingSphere.Init(m_bounds.Center, m_bounds.Extent.Length());
    CachedBoundingBox = m_bounds;
    Validate_Cached_Bounding_Volumes();
}

void W3DEmitterParticles::Prepare_LOD(W3DCamera &camera)
{
    if (!Is_Not_Hidden_At_All())
        return;
    Vector2 minimum, maximum;
    camera.Get_View_Plane(minimum, maximum);
    const auto viewport = camera.Get_Viewport();
    const auto &sphere = Get_Bounding_Sphere();
    m_detail.Prepare((sphere.Center - camera.Get_Position()).Length(), sphere.Radius, m_visuals.Max_Size(),
        viewport.Width() / (maximum.X - minimum.X), viewport.Height() / (maximum.Y - minimum.Y));
}

int W3DEmitterParticles::Calculate_Cost_Value_Arrays(float area, float *values, float *costs) const
{
    return m_detail.Calculate(area, std::span(values, m_detail.Count() + 1), std::span(costs, m_detail.Count()));
}

#include <functional>
#include "W3DDevice/GameClient/W3DAssetCatalog.h"
import Graphics.Frame.RenderClock;
import Graphics.Frame.RenderSettings;
#include "W3DDevice/GameClient/W3DRenderServices.h"
import Graphics.Materials.State;
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

import Assets.Adapters.W3D.Ring;
import Graphics.Frame.Runtime;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.OrderedDraws;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Views.CameraMatrices;
import Graphics.Scene.Views.View;

#include "W3DDevice/GameClient/W3DRingLoader.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"

#include "WWLib/chunkio.h"

namespace
{

Assets::RingAssetDesc Make_Effective_Description(const Assets::RingAssetDesc &source)
{
	Assets::RingAssetDesc description = source;
	if (auto *catalog = W3DAssetCatalog::Get_Instance(); catalog != nullptr &&
		catalog->Get_Fog_On_Load()) {
		// RingPrototypeClass delegated fog selection to Graphics::MaterialState::Enable_Fog.
		// Preserve its blend-pair table; unsupported pairs leave the authored fog
		// mode unchanged while fog activation still disables culling.
		switch (description.material.source_blend) {
		case Assets::RingBlendFactor::Zero:
			if (description.material.destination_blend == Assets::RingBlendFactor::SourceColor)
				description.material.fog = Assets::RingFogMode::White;
			break;
		case Assets::RingBlendFactor::One:
			switch (description.material.destination_blend) {
			case Assets::RingBlendFactor::Zero:
				description.material.fog = Assets::RingFogMode::Enabled;
				break;
			case Assets::RingBlendFactor::One:
			case Assets::RingBlendFactor::InverseSourceColor:
				description.material.fog = Assets::RingFogMode::ScaleFragment;
				break;
			default:
				break;
			}
			break;
		case Assets::RingBlendFactor::SourceAlpha:
			if (description.material.destination_blend == Assets::RingBlendFactor::InverseSourceAlpha)
				description.material.fog = Assets::RingFogMode::Enabled;
			break;
		case Assets::RingBlendFactor::InverseSourceAlpha:
			if (description.material.destination_blend == Assets::RingBlendFactor::SourceAlpha)
				description.material.fog = Assets::RingFogMode::Enabled;
			break;
		default:
			break;
		}
		description.material.cull_enabled = false;
	}
	return description;
}

template <typename Matrix>
std::array<float, 16> Copy_Matrix(const Matrix &matrix) noexcept
{
	std::array<float, 16> result{};
	for (std::size_t row = 0; row < 4; ++row)
		for (std::size_t column = 0; column < 4; ++column)
			result[row * 4 + column] = matrix[row][column];
	return result;
}

}

W3DRingRenderObject::W3DRingRenderObject() = default;

W3DRingRenderObject::W3DRingRenderObject(const Assets::RingAssetDesc &description)
	: m_name(description.name),
	  m_runtime(Make_Effective_Description(description))
{
	if (auto *catalog = W3DAssetCatalog::Get_Instance(); catalog != nullptr &&
		!description.texture_name.empty())
		m_texture.Assign_No_Add_Ref(catalog->Get_Texture(description.texture_name.c_str()));
}

W3DRingRenderObject::W3DRingRenderObject(const W3DRingRenderObject &source)
	: W3DRenderObject(source),
	  m_name(source.m_name),
	  m_runtime(source.m_runtime),
	  m_texture(source.m_texture)
{
}

W3DRingRenderObject &W3DRingRenderObject::operator=(const W3DRingRenderObject &source)
{
	if (this == &source)
		return *this;
	m_graphics.Release(Graphics::Get_Prop_Renderer());
	W3DRenderObject::operator=(source);
	m_name = source.m_name;
	m_runtime = source.m_runtime;
	m_texture = source.m_texture;
	return *this;
}

W3DRenderObject *W3DRingRenderObject::Clone() const
{
	return W3DNEW W3DRingRenderObject(*this);
}

void W3DRingRenderObject::Render(W3DRenderContext &rinfo)
{
	if (Get_LOD_Level() == 0 || Is_Not_Hidden_At_All() == false ||
		!m_runtime.Is_Authored_Visible())
		return;

	const unsigned sort_level = Graphics::Get_Render_Settings().Is_Sorting_Enabled() ? 0u :
		m_runtime.Ordered_Layer(m_runtime.Description().material);
	if (Graphics::Get_Scene_Draw_Queue().Is_Enabled() && sort_level != 0u) {
		Graphics::Get_Scene_Draw_Queue().Enqueue<Extract_Ordered_Draw>(sort_level, *this);
		return;
	}

	m_runtime.Advance(Graphics::Get_Render_Clock().Logic_Frame_Time_Seconds());
	const auto &camera = Graphics::Get_Camera_Matrices();
	Graphics::AuthoredRingDrawInput input;
	input.view_projection = Graphics::Compose_Matrices(camera.projection, camera.view).values;
	input.view = camera.view.values;
	input.world = Copy_Matrix(Matrix4x4(Get_Transform()));
	const Vector3 camera_z = rinfo.Camera.Get_Transform().Get_Z_Vector();
	input.camera_z = {camera_z.X, camera_z.Y, camera_z.Z};
	input.scene = Graphics::Get_Scene_Draw_Parameters();
	input.sorting_enabled = Graphics::Get_Render_Settings().Is_Sorting_Enabled();
	input.front_counter_clockwise = !Get_W3D_Render_Services().Is_Reflection_Render_Pass();
	if (m_texture.Peek() != nullptr)
		input.texture_sampling = m_texture->Get_Sampling();

	auto *device = Graphics::Shared_Frame_Device();
	W3DTextureHandle *texture_object = m_texture.Peek();
	if (!m_runtime.Description().texture_name.empty() && texture_object == nullptr)
		return;
	Graphics::RHITextureHandle texture;
	if (texture_object != nullptr) {
		if (device == nullptr || !texture_object->Ensure_Render_Backend_Texture())
			return;
		texture = texture_object->Peek_Graphics_Texture();
		if (!texture.Is_Valid() || !device->Retain_Texture(texture))
			return;
	}
	const std::array<Graphics::RHITextureHandle, 1> textures{texture};
	const std::span<const Graphics::RHITextureHandle> texture_span = texture.Is_Valid()
		? std::span<const Graphics::RHITextureHandle>(textures) : std::span<const Graphics::RHITextureHandle>{};
	const bool submitted = m_graphics.Submit(Graphics::Get_Prop_Renderer(),
		Graphics::Get_Prop_Submission(), m_runtime, input, texture_span);
	if (!submitted && texture.Is_Valid()) {
		device->Destroy_Texture(texture);
	}
}

void W3DRingRenderObject::Set_Transform(const Matrix3D &transform)
{
	W3DRenderObject::Set_Transform(transform);
}

void W3DRingRenderObject::Set_Position(const Vector3 &position)
{
	W3DRenderObject::Set_Position(position);
}

void W3DRingRenderObject::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
	const auto bounds = m_runtime.Bounds();
	sphere.Center.Set(bounds.center.x, bounds.center.y, bounds.center.z);
	sphere.Radius = Vector3(bounds.extent.x, bounds.extent.y, bounds.extent.z).Length();
}

void W3DRingRenderObject::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
	const auto bounds = m_runtime.Bounds();
	box.Center.Set(bounds.center.x, bounds.center.y, bounds.center.z);
	box.Extent.Set(bounds.extent.x, bounds.extent.y, bounds.extent.z);
}

void W3DRingRenderObject::Prepare_LOD(W3DCamera &camera)
{
	if (Is_Not_Hidden_At_All() == false)
		return;
	m_runtime.Prepare_LOD(Get_Screen_Size(camera));
}

void W3DRingRenderObject::Increment_LOD()
{
	m_runtime.Increment_LOD();
}

void W3DRingRenderObject::Decrement_LOD()
{
	m_runtime.Decrement_LOD();
}

float W3DRingRenderObject::Get_Cost() const
{
	return m_runtime.Cost();
}

float W3DRingRenderObject::Get_Value() const
{
	return m_runtime.Value();
}

float W3DRingRenderObject::Get_Post_Increment_Value() const
{
	return m_runtime.Post_Increment_Value();
}

void W3DRingRenderObject::Set_LOD_Level(int lod)
{
	m_runtime.Set_LOD_Level(lod);
}

int W3DRingRenderObject::Get_LOD_Level() const
{
	return m_runtime.LOD_Level();
}

int W3DRingRenderObject::Get_LOD_Count() const
{
	return m_runtime.LOD_Count();
}

void W3DRingRenderObject::Set_LOD_Bias(float bias)
{
	m_runtime.Set_LOD_Bias(bias);
}

int W3DRingRenderObject::Calculate_Cost_Value_Arrays(float screen_area,
	float *values, float *costs) const
{
	if (values == nullptr || costs == nullptr)
		return -1;
	return m_runtime.Calculate_Cost_Value_Arrays(screen_area,
		{values, Graphics::AuthoredRingValueCount},
		{costs, Graphics::AuthoredRingCostCount}) ? 0 : -1;
}

void W3DRingRenderObject::Scale(float scale)
{
	m_runtime.Scale(scale);
}

void W3DRingRenderObject::Scale(float scalex, float scaley, float scalez)
{
	m_runtime.Scale(scalex, scaley, scalez);
}

void W3DRingRenderObject::Set_Hidden(int onoff)
{
	W3DRenderObject::Set_Hidden(onoff);
	m_runtime.Set_Hidden(onoff != 0);
}

void W3DRingRenderObject::Set_Visible(int onoff)
{
	W3DRenderObject::Set_Visible(onoff);
	m_runtime.Set_Visible(onoff != 0);
}

void W3DRingRenderObject::Set_Animation_Hidden(int onoff)
{
	W3DRenderObject::Set_Animation_Hidden(onoff);
	m_runtime.Set_Animation_Hidden(onoff != 0);
}

void W3DRingRenderObject::Set_Force_Visible(int onoff)
{
	W3DRenderObject::Set_Force_Visible(onoff);
	m_runtime.Set_Force_Visible(onoff != 0);
}

void W3DRingRenderObject::Set_Name(const char *name)
{
	if (name != nullptr)
		m_name = name;
}

Graphics::ModelFactory<W3DRenderObject> *Load_Ring_Factory(ChunkLoadClass &cload)
{
	std::vector<std::byte> bytes(cload.Cur_Chunk_Length());
	if (cload.Read(bytes.data(), static_cast<unsigned>(bytes.size())) != bytes.size())
		return nullptr;

	Assets::RingAssetDesc description;
	std::string error;
	if (!Assets::W3D::W3DRead_Ring(bytes, description, error))
		return nullptr;
	auto data = std::make_shared<const Assets::RingAssetDesc>(std::move(description));
	return new Graphics::ModelFactory<W3DRenderObject>(data->name,
		W3DRenderObject::CLASSID_RING, [data] {
			return NEW_REF(W3DRingRenderObject, (*data));
		});
}

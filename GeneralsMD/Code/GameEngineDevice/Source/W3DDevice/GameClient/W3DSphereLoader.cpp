#include <functional>
import Graphics.Frame.RenderClock;
import Graphics.Frame.RenderSettings;
#include "W3DDevice/GameClient/W3DRenderServices.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

import Assets.Adapters.W3D.Sphere;
import Graphics.Frame.Runtime;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.OrderedDraws;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Views.CameraMatrices;
import Graphics.Scene.Views.View;

#include "W3DDevice/GameClient/W3DSphereLoader.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"

#include "WWLib/chunkio.h"

namespace
{

template <typename Matrix>
std::array<float, 16> Copy_Matrix(const Matrix &matrix) noexcept
{
	std::array<float, 16> result{};
	for (std::size_t row = 0; row < 4; ++row)
		for (std::size_t column = 0; column < 4; ++column)
			result[row * 4 + column] = matrix[row][column];
	return result;
}

Assets::SphereAssetDesc Default_Sphere_Description()
{
	Assets::SphereAssetDesc description;
	description.name = "sphere";
	description.material.cull_enabled = false;
	return description;
}

}

Assets::SphereAssetDesc W3DSphereRenderObject::With_Load_Fog(
	const Assets::SphereAssetDesc &description)
{
	Assets::SphereAssetDesc result = description;
	W3DAssetCatalog *catalog = W3DAssetCatalog::Get_Instance();
	if (catalog != nullptr && catalog->Get_Fog_On_Load()) {
		switch (result.material.source_blend) {
		case Assets::SphereBlendFactor::Zero:
			if (result.material.destination_blend == Assets::SphereBlendFactor::SourceColor)
				result.material.fog = Assets::SphereFogMode::White;
			break;
		case Assets::SphereBlendFactor::One:
			switch (result.material.destination_blend) {
			case Assets::SphereBlendFactor::Zero:
				result.material.fog = Assets::SphereFogMode::Enabled;
				break;
			case Assets::SphereBlendFactor::One:
			case Assets::SphereBlendFactor::InverseSourceColor:
				result.material.fog = Assets::SphereFogMode::ScaleFragment;
				break;
			default:
				break;
			}
			break;
		case Assets::SphereBlendFactor::SourceAlpha:
			if (result.material.destination_blend == Assets::SphereBlendFactor::InverseSourceAlpha)
				result.material.fog = Assets::SphereFogMode::Enabled;
			break;
		case Assets::SphereBlendFactor::InverseSourceAlpha:
			if (result.material.destination_blend == Assets::SphereBlendFactor::SourceAlpha)
				result.material.fog = Assets::SphereFogMode::Enabled;
			break;
		default:
			break;
		}
		// Sphere vertices use the source's backwards normal winding.
		result.material.cull_enabled = false;
	}
	return result;
}

W3DSphereRenderObject::W3DSphereRenderObject()
	: m_name("sphere"),
	  m_sphere(With_Load_Fog(Default_Sphere_Description()))
{
}

W3DSphereRenderObject::W3DSphereRenderObject(
	const Assets::SphereAssetDesc &description)
	: m_name(description.name),
	  m_sphere(With_Load_Fog(description))
{
	if (W3DAssetCatalog *catalog = W3DAssetCatalog::Get_Instance();
		catalog != nullptr && !description.texture_name.empty())
		m_texture.Assign_No_Add_Ref(catalog->Get_Texture(description.texture_name.c_str()));
}

W3DSphereRenderObject::W3DSphereRenderObject(
	const W3DSphereRenderObject &source)
	: W3DRenderObject(source),
	  m_name(source.m_name),
	  m_sphere(source.m_sphere),
	  m_texture(source.m_texture)
{
}

W3DSphereRenderObject &W3DSphereRenderObject::operator=(
	const W3DSphereRenderObject &source)
{
	if (this == &source)
		return *this;
	m_graphics.Release(Graphics::Get_Prop_Renderer());
	W3DRenderObject::operator=(source);
	m_name = source.m_name;
	m_sphere = source.m_sphere;
	m_texture = source.m_texture;
	return *this;
}

W3DRenderObject *W3DSphereRenderObject::Clone() const
{
	return W3DNEW W3DSphereRenderObject(*this);
}

void W3DSphereRenderObject::Render(W3DRenderContext &rinfo)
{
	if (m_sphere.LOD_Level() == 0 || !W3DRenderObject::Is_Not_Hidden_At_All())
		return;

	if (!Graphics::Get_Render_Settings().Is_Sorting_Enabled()) {
		const std::uint32_t sort_level =
			Graphics::Sphere_Ordered_Layer(m_sphere.Asset().material);
		if (sort_level != 0 && Graphics::Get_Scene_Draw_Queue().Is_Enabled()
			&& Graphics::Get_Scene_Draw_Queue().Enqueue<Extract_Ordered_Draw>(sort_level, *this))
			return;
	}

	(void)m_sphere.Update(Graphics::Get_Render_Clock().Logic_Frame_Time_Seconds());
	const auto &camera = Graphics::Get_Camera_Matrices();
	Graphics::SphereDrawInput input;
	input.view_projection = Graphics::Compose_Matrices(camera.projection, camera.view).values;
	input.projection = camera.projection.values;
	input.view = camera.view.values;
	input.world = Copy_Matrix(Matrix4x4(Get_Transform()));
	const Vector3 camera_position = rinfo.Camera.Get_Transform().Get_Translation();
	input.camera_position = {camera_position.X, camera_position.Y, camera_position.Z, 1.0f};
	input.scene = Graphics::Get_Scene_Draw_Parameters();
	input.sorting_enabled = Graphics::Get_Render_Settings().Is_Sorting_Enabled();
	input.front_counter_clockwise = !Get_W3D_Render_Services().Is_Reflection_Render_Pass();
	if (m_texture.Peek() != nullptr)
		input.texture_sampling = m_texture->Get_Sampling();

	auto *device = Graphics::Shared_Frame_Device();
	W3DTextureHandle *texture_object = m_texture.Peek();
	if (!m_sphere.Asset().texture_name.empty() && texture_object == nullptr)
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
		? std::span<const Graphics::RHITextureHandle>(textures)
		: std::span<const Graphics::RHITextureHandle>{};
	const bool submitted = m_graphics.Submit(Graphics::Get_Prop_Renderer(),
		Graphics::Get_Prop_Submission(), m_sphere, input, texture_span);
	if (!submitted && texture.Is_Valid())
		device->Destroy_Texture(texture);
}

void W3DSphereRenderObject::Set_Transform(const Matrix3D &transform)
{
	W3DRenderObject::Set_Transform(transform);
}

void W3DSphereRenderObject::Set_Position(const Vector3 &position)
{
	W3DRenderObject::Set_Position(position);
}

void W3DSphereRenderObject::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
	const auto &asset = m_sphere.Asset();
	sphere.Init(Vector3(asset.center.x, asset.center.y, asset.center.z),
		Vector3(asset.extent.x, asset.extent.y, asset.extent.z).Length());
}

void W3DSphereRenderObject::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
	const auto &asset = m_sphere.Asset();
	box.Init(Vector3(asset.center.x, asset.center.y, asset.center.z),
		Vector3(asset.extent.x, asset.extent.y, asset.extent.z));
}

void W3DSphereRenderObject::Update_Cached_Bounding_Volumes() const
{
	const auto &asset = m_sphere.Asset();
	const auto &scale = m_sphere.State().scale;
	CachedBoundingBox.Extent.Set(asset.extent.x * scale.x,
		asset.extent.y * scale.y, asset.extent.z * scale.z);
	CachedBoundingSphere.Center = CachedBoundingBox.Center = Get_Position() +
		Vector3(asset.center.x, asset.center.y, asset.center.z);
	CachedBoundingSphere.Radius = CachedBoundingBox.Extent.Length();
	Validate_Cached_Bounding_Volumes();
}

void W3DSphereRenderObject::Prepare_LOD(W3DCamera &camera)
{
	if (!W3DRenderObject::Is_Not_Hidden_At_All())
		return;
	m_sphere.Prepare_LOD(Get_Screen_Size(camera));
}

void W3DSphereRenderObject::Increment_LOD()
{
	m_sphere.Increment_LOD();
}

void W3DSphereRenderObject::Decrement_LOD()
{
	m_sphere.Decrement_LOD();
}

float W3DSphereRenderObject::Get_Cost() const
{
	return m_sphere.Cost();
}

float W3DSphereRenderObject::Get_Value() const
{
	return m_sphere.Value();
}

float W3DSphereRenderObject::Get_Post_Increment_Value() const
{
	return m_sphere.Post_Increment_Value();
}

void W3DSphereRenderObject::Set_LOD_Level(int lod)
{
	m_sphere.Set_LOD_Level(lod < 0 ? 0u : static_cast<std::uint32_t>(lod));
}

int W3DSphereRenderObject::Get_LOD_Level() const
{
	return static_cast<int>(m_sphere.LOD_Level());
}

int W3DSphereRenderObject::Get_LOD_Count() const
{
	return static_cast<int>(m_sphere.LOD_Count());
}

void W3DSphereRenderObject::Set_LOD_Bias(float bias)
{
	m_sphere.Set_LOD_Bias(bias);
}

int W3DSphereRenderObject::Calculate_Cost_Value_Arrays(float screen_area,
	float *values, float *costs) const
{
	if (values == nullptr || costs == nullptr)
		return -1;
	return m_sphere.Calculate_Cost_Value_Arrays(screen_area,
		{values, Graphics::SphereLODValueCount},
		{costs, Graphics::SphereLODCount + 1}) ? 0 : -1;
}

void W3DSphereRenderObject::Scale(float scale)
{
	m_sphere.Scale(scale);
}

void W3DSphereRenderObject::Scale(float scalex, float scaley, float scalez)
{
	m_sphere.Scale(scalex, scaley, scalez);
}

void W3DSphereRenderObject::Set_Hidden(int onoff)
{
	W3DRenderObject::Set_Hidden(onoff);
	m_sphere.Set_Hidden(onoff != 0);
}

void W3DSphereRenderObject::Set_Visible(int onoff)
{
	W3DRenderObject::Set_Visible(onoff);
	m_sphere.Set_Visible(onoff != 0);
}

void W3DSphereRenderObject::Set_Animation_Hidden(int onoff)
{
	W3DRenderObject::Set_Animation_Hidden(onoff);
	m_sphere.Set_Animation_Hidden(onoff != 0);
}

void W3DSphereRenderObject::Set_Force_Visible(int onoff)
{
	W3DRenderObject::Set_Force_Visible(onoff);
	m_sphere.Set_Force_Visible(onoff != 0);
}

int W3DSphereRenderObject::Get_Num_Polys() const
{
	return static_cast<int>(m_sphere.Num_Polys());
}

void W3DSphereRenderObject::Set_Name(const char *name)
{
	if (name != nullptr)
		m_name = name;
}

void W3DSphereRenderObject::Set_Texture(W3DTextureHandle *texture)
{
	m_texture.Assign_Add_Ref(texture);
}

Graphics::ModelFactory<W3DRenderObject> *Load_Sphere_Factory(ChunkLoadClass &cload)
{
	std::vector<std::byte> bytes(cload.Cur_Chunk_Length());
	if (cload.Read(bytes.data(), static_cast<unsigned>(bytes.size())) != bytes.size())
		return nullptr;

	Assets::SphereAssetDesc description;
	std::string error;
	if (!Assets::W3D::W3DRead_Sphere(bytes, description, error))
		return nullptr;
	auto data = std::make_shared<const Assets::SphereAssetDesc>(std::move(description));
	return new Graphics::ModelFactory<W3DRenderObject>(data->name,
		W3DRenderObject::CLASSID_SPHERE, [data] {
			return NEW_REF(W3DSphereRenderObject, (*data));
		});
}

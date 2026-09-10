module;

#define BOOST_TEST_MODULE GraphicsW3DE2ETests

#include <boost/test/included/unit_test.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

export module Graphics.TestScenarios.W3D.E2E.Tests;

import Assets.Adapters.W3D;
import Assets.Adapters.W3D.Chunks;
import Assets.Cache;
import Assets.Handles;
import Assets.Identity;
import Assets.Materials;
import Assets.Models;
import Assets.Math;
import Assets.States;
import Assets.Textures;
import Graphics.Tests.Device;
import Graphics.Scene.Models.ModelAssetBinding;
import Graphics.Testing.VisualRegression;

#ifndef GRAPHICS_W3D_E2E_MODEL_DIRECTORY
#define GRAPHICS_W3D_E2E_MODEL_DIRECTORY "."
#endif

#ifndef GRAPHICS_W3D_E2E_TEXTURE_DIRECTORY
#define GRAPHICS_W3D_E2E_TEXTURE_DIRECTORY "."
#endif

#ifndef GRAPHICS_W3D_E2E_REFERENCE_DIRECTORY
#define GRAPHICS_W3D_E2E_REFERENCE_DIRECTORY "."
#endif

#ifndef GRAPHICS_W3D_E2E_FAILURE_DIRECTORY
#define GRAPHICS_W3D_E2E_FAILURE_DIRECTORY "."
#endif

#ifndef GRAPHICS_W3D_E2E_SHADER_DIRECTORY
#define GRAPHICS_W3D_E2E_SHADER_DIRECTORY "."
#endif

namespace
{

using Byte = std::byte;

std::vector<Byte> Read_File(const std::filesystem::path &path)
{
	std::ifstream stream(path, std::ios::binary);
	if (!stream)
		return {};

	const std::vector<char> chars((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	std::vector<Byte> bytes;
	bytes.reserve(chars.size());
	for (const char value : chars)
		bytes.push_back(static_cast<Byte>(value));
	return bytes;
}

std::filesystem::path Find_Asset(const Assets::AssetIdentity &identity)
{
	const std::filesystem::path file_name(identity.canonical_name);
	if (identity.type == Assets::AssetType::Model)
		return std::filesystem::path(GRAPHICS_W3D_E2E_MODEL_DIRECTORY) / file_name.filename();
	if (identity.type == Assets::AssetType::Texture) {
		const std::filesystem::path texture_directory(GRAPHICS_W3D_E2E_TEXTURE_DIRECTORY);
		const std::filesystem::path dds_path = texture_directory / (file_name.stem().string() + ".dds");
		return std::filesystem::exists(dds_path) ? dds_path : texture_directory / file_name.filename();
	}
	return {};
}

struct W3DTestSource final
{
	std::vector<Byte> operator()(const Assets::AssetIdentity &identity) const
	{
		const std::filesystem::path path = Find_Asset(identity);
		return path.empty() ? std::vector<Byte>{} : Read_File(path);
	}
};

struct W3DHierarchy final
{
	std::vector<Graphics::SkeletonBone> bones;
	std::uint32_t requested_bone = Graphics::Invalid_Bone_Index;
};

Graphics::RenderTransform Read_Pivot_Transform(Assets::W3D::W3DByteSpan bytes, std::size_t offset) noexcept
{
	Graphics::RenderTransform transform;
	float qx = 0.0f;
	float qy = 0.0f;
	float qz = 0.0f;
	float qw = 1.0f;
	if (!Assets::W3D::W3DRead_F32(bytes, offset + 20, transform.matrix[3])
		|| !Assets::W3D::W3DRead_F32(bytes, offset + 24, transform.matrix[7])
		|| !Assets::W3D::W3DRead_F32(bytes, offset + 28, transform.matrix[11])
		|| !Assets::W3D::W3DRead_F32(bytes, offset + 44, qx)
		|| !Assets::W3D::W3DRead_F32(bytes, offset + 48, qy)
		|| !Assets::W3D::W3DRead_F32(bytes, offset + 52, qz)
		|| !Assets::W3D::W3DRead_F32(bytes, offset + 56, qw))
		return {};

	transform.matrix[0] = 1.0f - 2.0f * (qy * qy + qz * qz);
	transform.matrix[1] = 2.0f * (qx * qy - qz * qw);
	transform.matrix[2] = 2.0f * (qz * qx + qy * qw);
	transform.matrix[4] = 2.0f * (qx * qy + qz * qw);
	transform.matrix[5] = 1.0f - 2.0f * (qz * qz + qx * qx);
	transform.matrix[6] = 2.0f * (qy * qz - qx * qw);
	transform.matrix[8] = 2.0f * (qz * qx - qy * qw);
	transform.matrix[9] = 2.0f * (qy * qz + qx * qw);
	transform.matrix[10] = 1.0f - 2.0f * (qy * qy + qx * qx);
	transform.matrix[15] = 1.0f;
	return transform;
}

bool Read_W3D_Hierarchy(const std::vector<Byte> &source, std::string_view requested_bone, W3DHierarchy &hierarchy)
{
	std::uint32_t pivot_count = 0;
	Assets::W3D::W3DByteSpan pivot_bytes;
	if (!Assets::W3D::W3DVisit_Chunks(source, [&pivot_count, &pivot_bytes](const Assets::W3D::W3DChunkView &chunk) {
		if (chunk.id != Assets::W3D::W3DChunkHierarchy || !chunk.contains_children)
			return true;
		return Assets::W3D::W3DVisit_Chunks(chunk.payload, [&pivot_count, &pivot_bytes](const Assets::W3D::W3DChunkView &child) {
			if (child.id == Assets::W3D::W3DChunkHierarchyHeader)
				return Assets::W3D::W3DRead_U32(child.payload, 20, pivot_count);
			if (child.id == Assets::W3D::W3DChunkPivots) {
				pivot_bytes = child.payload;
				return true;
			}
			return true;
		});
	}) || pivot_count == 0 || pivot_bytes.size() != static_cast<std::size_t>(pivot_count) * 60u)
		return false;

	hierarchy.bones.resize(pivot_count);
	for (std::uint32_t index = 0; index < pivot_count; ++index) {
		const std::size_t offset = static_cast<std::size_t>(index) * 60u;
		std::uint32_t parent = Assets::W3D::W3DInvalidIndex;
		if (!Assets::W3D::W3DRead_U32(pivot_bytes, offset + 16, parent))
			return false;
		hierarchy.bones[index].parent = parent == Assets::W3D::W3DInvalidIndex
			? Graphics::Invalid_Bone_Index : parent;
		hierarchy.bones[index].rest_transform = Read_Pivot_Transform(pivot_bytes, offset);
		if (hierarchy.bones[index].parent != Graphics::Invalid_Bone_Index
			&& hierarchy.bones[index].parent >= index)
			return false;
		const std::string name = Assets::W3D::W3DRead_Fixed_String(pivot_bytes.subspan(offset, 60), 0, 16);
		if (name == requested_bone)
			hierarchy.requested_bone = index;
	}
	return hierarchy.requested_bone != Graphics::Invalid_Bone_Index;
}

bool Has_Staged_Assets() noexcept
{
	return std::filesystem::exists(std::filesystem::path(GRAPHICS_W3D_E2E_MODEL_DIRECTORY) / "CVTank.W3D")
		&& std::filesystem::exists(std::filesystem::path(GRAPHICS_W3D_E2E_TEXTURE_DIRECTORY) / "cvtank.dds");
}

Graphics::RenderTransform Make_Fit_Transform(const Assets::Bounds3f &bounds, float x_offset, float y_offset, float scale_factor) noexcept
{
	const float center_x = (bounds.minimum.x + bounds.maximum.x) * 0.5f;
	const float center_y = (bounds.minimum.y + bounds.maximum.y) * 0.5f;
	const float center_z = (bounds.minimum.z + bounds.maximum.z) * 0.5f;
	const float extent_x = bounds.maximum.x - bounds.minimum.x;
	const float extent_y = bounds.maximum.y - bounds.minimum.y;
	const float extent_z = bounds.maximum.z - bounds.minimum.z;
	const float maximum_extent = (std::max)({extent_x, extent_y, extent_z, 0.001f});
	const float scale = scale_factor / maximum_extent;

	Graphics::RenderTransform transform;
	transform.matrix = Graphics::Matrix4x4::Identity().values;
	transform.matrix[0] = scale;
	transform.matrix[5] = scale;
	transform.matrix[10] = scale;
	transform.matrix[3] = x_offset - center_x * scale;
	transform.matrix[7] = y_offset - center_y * scale;
	transform.matrix[11] = 0.25f - center_z * scale;
	return transform;
}

struct ModelScenario final
{
	Graphics::StaticMeshBinding binding;
	std::vector<Graphics::TextureHandle> textures;
	std::vector<Graphics::MaterialHandle> materials;
	Graphics::MeshHandle attachment_mesh{};
	Graphics::MaterialHandle attachment_material{};
	Graphics::InstanceHandle attachment_instance{};
	Graphics::AttachmentLinkHandle attachment_link{};
	std::uint32_t requested_bone = Graphics::Invalid_Bone_Index;
};

struct E2EContext final
{
	Assets::AssetCache assets{W3DTestSource{}};
	Graphics::GraphicsTestDevice device{{true}};
	Graphics::StaticMeshRenderer renderer;
	ModelScenario scenario;

	bool Initialize(std::string_view model_name, float x_offset, float y_offset, float scale_factor,
		std::string_view hierarchy_name = {}, std::string_view requested_bone = {}, bool add_attachment_marker = false)
	{
		if (!device.Is_Valid()
			|| !assets.Register_Model_Adapter(std::make_shared<Assets::W3DAdapter>())
			|| !renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_W3D_E2E_SHADER_DIRECTORY), 64, 8, 64, 64))
			return false;

		const Assets::ModelAssetHandle model_handle = assets.Request_Model(model_name);
		assets.Wait(model_handle);
		const Assets::AssetState model_state = assets.Get_State(model_handle);
		const Assets::ModelAsset *model = assets.Try_Get_Model(model_handle);
		if (!model_handle.Is_Valid())
			return false;
		if (model == nullptr)
			return false;
		if (model_state != Assets::AssetState::Ready)
			return false;
		if (model->Materials().empty())
			return false;
		if (model_name == "NVMig.W3D") {
			if (model->Materials().size() != 4
				|| model->Materials()[0].primary_texture.canonical_name != "extnkmzl01.tga"
				|| model->Materials()[1].primary_texture.canonical_name != "extnkmzl01.tga"
				|| model->Materials()[2].primary_texture.canonical_name != "housecolor2.tga"
				|| model->Materials()[3].primary_texture.canonical_name != "nvmig.tga"
				|| model->Materials()[0].render_mode != Assets::MaterialRenderMode::Additive
				|| model->Materials()[1].render_mode != Assets::MaterialRenderMode::Additive
				|| model->Materials()[2].render_mode != Assets::MaterialRenderMode::Opaque
				|| model->Materials()[3].render_mode != Assets::MaterialRenderMode::Opaque)
				return false;
		}
		scenario.textures.resize(model->Materials().size());
		for (std::size_t index = 0; index < model->Materials().size(); ++index) {
			const Assets::ModelMaterial &model_material = model->Materials()[index];
			const Assets::MaterialAsset *material_asset = assets.Try_Get_Material(model_material.asset_handle);
			if (material_asset == nullptr)
				return false;

			const Assets::TextureAsset *texture_asset = assets.Try_Get_Texture(material_asset->Primary_Texture());
			if (texture_asset == nullptr || !texture_asset->Has_Pixels())
				return false;

			Graphics::Texture texture;
			texture.width = texture_asset->Width();
			texture.height = texture_asset->Height();
			texture.mip_count = 1;
			texture.format = Graphics::TextureFormat::RGBA8_UNorm;
			texture.usage = Graphics::TextureUsage::Sampled;
			texture.pixel_data = texture_asset->Pixels();
			texture.row_pitch = texture_asset->Row_Pitch();
			scenario.textures[index] = renderer.Create_Texture(texture);
			if (!scenario.textures[index].Is_Valid())
				return false;
		}

		Graphics::SkeletonHandle skeleton;
		if (!hierarchy_name.empty()) {
			W3DHierarchy hierarchy;
			if (!Read_W3D_Hierarchy(Read_File(std::filesystem::path(GRAPHICS_W3D_E2E_MODEL_DIRECTORY) / hierarchy_name), requested_bone, hierarchy))
				return false;
			scenario.requested_bone = hierarchy.requested_bone;
			skeleton = renderer.Create_Skeleton(hierarchy.bones);
			if (!skeleton.Is_Valid())
				return false;
		}

		const Graphics::RenderTransform transform = Make_Fit_Transform(model->Bounds(), x_offset, y_offset, scale_factor);
		if (!Graphics::Create_Model_Asset_Binding(
			renderer,
			*model,
			transform,
			Graphics::RenderInstanceFlags::CastsShadow | Graphics::RenderInstanceFlags::DoubleSided,
			Graphics::All_Submeshes_Visible,
			scenario.binding,
			scenario.materials,
			scenario.textures,
			skeleton))
			return false;
		if (skeleton.Is_Valid() && scenario.binding.Skeleton() != skeleton)
			return false;
		if (scenario.requested_bone != Graphics::Invalid_Bone_Index) {
			Graphics::RenderTransform bone_transform;
			if (!scenario.binding.Get_Bone_Transform(
				renderer,
				Graphics::BoneHandle(scenario.requested_bone, 1),
				bone_transform))
				return false;
		}
		if (add_attachment_marker && scenario.requested_bone != Graphics::Invalid_Bone_Index) {
			const std::array<Graphics::StaticMeshVertex, 3> marker_vertices = {{
				{{0.0f, -0.75f, 0.0f}, {1.0f, 0.15f, 0.05f, 1.0f}, {0.0f, 0.0f}},
				{{0.0f, 0.75f, 0.0f}, {1.0f, 0.15f, 0.05f, 1.0f}, {0.5f, 1.0f}},
				{{1.0f, 0.0f, 0.0f}, {1.0f, 0.15f, 0.05f, 1.0f}, {1.0f, 0.5f}}}};
			const std::array<std::uint16_t, 3> marker_indices = {0, 1, 2};
			const std::array<Graphics::MeshPart, 1> marker_parts = {{{0, 3, 0, {}, 0, 0}}};
			const Graphics::StaticMeshSource marker_source{
				3,
				3,
				static_cast<std::uint32_t>(sizeof(Graphics::StaticMeshVertex)),
				Graphics::MeshIndexFormat::UInt16,
				std::as_bytes(std::span<const Graphics::StaticMeshVertex>(marker_vertices)),
				std::as_bytes(std::span<const std::uint16_t>(marker_indices)),
				{0.0f, 0.0f, 0.0f},
				1.0f,
				marker_parts,
				Graphics::MeshVertexFormat::Position3Color4UV2,
				0};
			scenario.attachment_mesh = renderer.Create_Mesh(marker_source);
			Graphics::Material marker_material;
			marker_material.parameters.values = {1.0f, 1.0f, 1.0f, 1.0f};
			marker_material.flags = Graphics::MaterialFlags::VertexColor;
			scenario.attachment_material = renderer.Create_Material(marker_material);
			Graphics::RenderInstance marker_instance;
			marker_instance.transform = Graphics::Identity_Render_Transform();
			marker_instance.bounds = {{0.0f, 0.0f, 0.0f}, 1.0f};
			marker_instance.mesh = scenario.attachment_mesh;
			marker_instance.material = scenario.attachment_material;
			marker_instance.flags = Graphics::RenderInstanceFlags::DoubleSided;
			scenario.attachment_instance = renderer.Create_Instance(marker_instance);
			const Graphics::AttachmentTarget target{
				Graphics::AttachmentTargetKind::Bone,
				Graphics::BoneHandle(scenario.requested_bone, 1),
				{},
				Graphics::Identity_Render_Transform()};
			scenario.attachment_link = renderer.Attach_Instance(
				scenario.attachment_instance,
				scenario.binding.Instance(),
				target);
			if (!scenario.attachment_mesh.Is_Valid() || !scenario.attachment_material.Is_Valid()
				|| !scenario.attachment_instance.Is_Valid() || !scenario.attachment_link.Is_Valid())
				return false;
		}
		renderer.Set_View({
			Graphics::Matrix4x4::Identity(),
			Graphics::Matrix4x4::Identity(),
			{},
			{0.0f, 0.0f, 256.0f, 144.0f, 0.0f, 1.0f}
		});
		renderer.Set_Fog({}, 0.0f, 0.0f, 1.0f, false);
		return true;
	}

	void Shutdown() noexcept
	{
		if (scenario.attachment_instance.Is_Valid())
			renderer.Destroy_Instance(scenario.attachment_instance);
		if (scenario.attachment_mesh.Is_Valid())
			renderer.Destroy_Mesh(scenario.attachment_mesh);
		if (scenario.attachment_material.Is_Valid())
			renderer.Destroy_Material(scenario.attachment_material);
		scenario.binding.Destroy(renderer);
		for (const Graphics::MaterialHandle handle : scenario.materials) {
			if (handle.Is_Valid())
				renderer.Destroy_Material(handle);
		}
		for (const Graphics::TextureHandle handle : scenario.textures) {
			if (handle.Is_Valid())
				renderer.Destroy_Texture(handle);
		}
		scenario.materials.clear();
		scenario.textures.clear();
		scenario.attachment_mesh = {};
		scenario.attachment_material = {};
		scenario.attachment_instance = {};
		scenario.attachment_link = {};
		scenario.requested_bone = Graphics::Invalid_Bone_Index;
		renderer.Shutdown();
	}
};

bool Render_Model(Graphics::Device &, Graphics::CommandList &commands, Graphics::RHITextureHandle color_target,
	Graphics::RHITextureHandle depth_target, Graphics::RHIViewport viewport, void *context) noexcept
{
	Graphics::StaticMeshRenderer &renderer = *static_cast<Graphics::StaticMeshRenderer *>(context);
	return renderer.Render(commands, {{color_target, viewport.width, viewport.height}, {depth_target, viewport.width, viewport.height}});
}

void Run_Model_Snapshot(
	std::string_view model_name,
	std::string_view snapshot_name,
	float x_offset,
	float y_offset,
	float scale_factor,
	std::string_view hierarchy_name = {},
	std::string_view requested_bone = {},
	bool add_attachment_marker = false)
{
	if (!Has_Staged_Assets()) {
		BOOST_TEST_MESSAGE("Skipping W3D end-to-end rendering: staged GeneralsMD Run assets are unavailable.");
		return;
	}

	E2EContext context;
	BOOST_REQUIRE_MESSAGE(context.Initialize(
		model_name,
		x_offset,
		y_offset,
		scale_factor,
		hierarchy_name,
		requested_bone,
		add_attachment_marker), "real W3D asset setup failed");
	Graphics::VisualRegressionHarness harness({
		256,
		144,
		3,
		std::filesystem::path(GRAPHICS_W3D_E2E_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_W3D_E2E_FAILURE_DIRECTORY)
	});
	const Graphics::VisualComparisonResult result = harness.Run(context.device, snapshot_name, Render_Model, &context.renderer);
	BOOST_CHECK_MESSAGE(result.expected_loaded, "missing colocated W3D reference image");
	BOOST_CHECK_MESSAGE(result.matched, "W3D end-to-end visual regression mismatch");
	context.Shutdown();
}

}

BOOST_AUTO_TEST_CASE(w3d_cvtank_textured_opaque)
{
	Run_Model_Snapshot("CVTank.W3D", "w3d_cvtank_textured_opaque", 0.0f, 0.0f, 1.35f);
}

BOOST_AUTO_TEST_CASE(w3d_cvtank_damaged_textured_variant)
{
	Run_Model_Snapshot("CVTank_d1.W3D", "w3d_cvtank_damaged_textured_variant", 0.0f, 0.0f, 1.35f);
}

BOOST_AUTO_TEST_CASE(w3d_nvmig_textured_airframe)
{
	Run_Model_Snapshot("NVMig.W3D", "w3d_nvmig_textured_airframe", 0.0f, 0.0f, 1.35f);
}

BOOST_AUTO_TEST_CASE(w3d_aihero_skinned_character)
{
	Run_Model_Snapshot(
		"AIHERO_SKN.W3D",
		"w3d_aihero_skinned_character",
		0.0f,
		0.0f,
		1.35f,
		"AIHERO_SKL.W3D",
		"AIHERO1 HEAD");
}

BOOST_AUTO_TEST_CASE(w3d_emperor_tank_weapon_bone_attachment)
{
	Run_Model_Snapshot(
		"NVOvrlrdT.W3D",
		"w3d_emperor_tank_weapon_bone_attachment",
		0.0f,
		0.0f,
		1.35f,
		"NVOvrlrdT.W3D",
		"MUZZLE01",
		true);
}

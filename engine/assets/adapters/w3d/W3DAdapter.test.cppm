module;

#define BOOST_TEST_MODULE GeneralsAssetsW3DAdapterTests

#include <boost/test/included/unit_test.hpp>

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

export module Assets.Tests.W3DAdapter;

import Assets.Adapters.W3D;
import Assets.Adapters.W3D.Chunks;
import Assets.Adapters.W3D.PassBindings;
import Assets.Adapters.W3D.Materials;
import Assets.Adapters.W3D.Rig;
import Assets.Cache;
import Assets.Handles;
import Assets.Identity;
import Assets.Materials;
import Assets.Models;
import Assets.States;
import Assets.Textures;

BOOST_AUTO_TEST_CASE(decodes_material_passes_from_external_game_model_directory)
{
    // Optional corpus supplied by the user's game installation; proprietary
    // model data is never stored alongside the test.
    const char *directory = std::getenv("GENERALS_W3D_PASS_DIRECTORY");
    if (!directory) {
        BOOST_TEST_MESSAGE("Set GENERALS_W3D_PASS_DIRECTORY to audit game model pass payloads.");
        return;
    }
    using namespace Assets::W3D;
    std::size_t files = 0, passes = 0, materials = 0, textures = 0, shaders = 0, malformed_trees = 0;
    for (const auto &entry : std::filesystem::directory_iterator(directory)) {
        auto extension = entry.path().extension().string();
        if (extension != ".w3d" && extension != ".W3D") continue;
        std::ifstream stream(entry.path(), std::ios::binary | std::ios::ate);
        BOOST_REQUIRE(stream.good());
        const auto size = stream.tellg();
        BOOST_REQUIRE(size > 0);
        std::vector<std::byte> bytes(static_cast<std::size_t>(size));
        stream.seekg(0);
        BOOST_REQUIRE(stream.read(reinterpret_cast<char *>(bytes.data()), size).good());
        ++files;
        if (!W3DValidate_Chunk_Tree(bytes)) ++malformed_trees;
        // Audit valid pass payloads even when an unrelated sibling contains a
        // malformed subtree. Whole-model acceptance is a separate contract.
        W3DVisit_Chunks(bytes, [&](const W3DChunkView &mesh) {
            if (mesh.id != W3DChunkMesh || !mesh.contains_children) return true;
            std::uint32_t vertices = 0, triangles = 0;
            bool has_header = false;
            W3DVisit_Chunks(mesh.payload, [&](const W3DChunkView &chunk) {
                if (chunk.id == W3DChunkMeshHeader3)
                    has_header = W3DRead_U32(chunk.payload,44,vertices)
                        && W3DRead_U32(chunk.payload,40,triangles);
                return true;
            });
            if (!has_header) return true;
            const auto visit = [&](auto &&self, W3DByteSpan chunks, unsigned depth) -> void {
                BOOST_REQUIRE_LT(depth, 64u);
                W3DVisit_Chunks(chunks, [&](const W3DChunkView &chunk) {
                    if (chunk.id == W3DChunkTexture) {
                        W3DTextureData texture;
                        BOOST_TEST_CONTEXT(entry.path().filename().string() << " texture " << textures) {
                            BOOST_REQUIRE(W3DRead_Texture(chunk.payload,texture));
                        }
                        ++textures;
                    } else if (chunk.id == W3DChunkShaders) {
                        std::vector<W3DShaderSettings> records;
                        BOOST_TEST_CONTEXT(entry.path().filename().string() << " shaders " << shaders) {
                            BOOST_REQUIRE(W3DRead_Shaders(chunk.payload,records));
                        }
                        shaders += records.size();
                    } else if (chunk.id == W3DChunkVertexMaterial) {
                        W3DVertexMaterialData material;
                        BOOST_TEST_CONTEXT(entry.path().filename().string() << " vertex material " << materials) {
                            BOOST_REQUIRE(W3DRead_Vertex_Material(chunk.payload,material));
                        }
                        ++materials;
                    } else if (chunk.id == W3DChunkMaterialPass) {
                        W3DPassBindings bindings;
                        BOOST_TEST_CONTEXT(entry.path().filename().string() << " pass " << passes) {
                            BOOST_REQUIRE(W3DRead_Pass_Bindings(chunk.payload,vertices,triangles,bindings));
                        }
                        ++passes;
                    } else if (chunk.contains_children) self(self,chunk.payload,depth+1);
                    return true;
                });
            };
            visit(visit,mesh.payload,0);
            return true;
        });
    }
    BOOST_REQUIRE_GT(files, 0u);
    BOOST_REQUIRE_GT(passes, 0u);
    BOOST_REQUIRE_GT(materials, 0u);
    BOOST_REQUIRE_GT(textures, 0u);
    BOOST_REQUIRE_GT(shaders, 0u);
    BOOST_TEST_MESSAGE("Audited " << passes << " material passes, " << materials << " vertex materials, "
        << textures << " textures, and " << shaders << " shaders across " << files
        << " files; " << malformed_trees << " files have malformed chunk trees outside this acceptance check.");
}

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
	for (const char character : chars)
		bytes.push_back(static_cast<Byte>(character));
	return bytes;
}

std::filesystem::path Integration_Asset_Path()
{
	if (const char *environment_path = std::getenv("GENERALS_W3D_INTEGRATION_ASSET"); environment_path != nullptr)
		return environment_path;
#ifdef ASSETS_W3D_INTEGRATION_ASSET
	if (std::filesystem::exists(ASSETS_W3D_INTEGRATION_ASSET))
		return ASSETS_W3D_INTEGRATION_ASSET;
#endif
#ifdef ASSETS_W3D_STAGED_ASSET
	if (std::filesystem::exists(ASSETS_W3D_STAGED_ASSET))
		return ASSETS_W3D_STAGED_ASSET;
#endif
	return {};
}

void Check_Deterministic(const Assets::ModelAssetDesc &left, const Assets::ModelAssetDesc &right)
{
	BOOST_CHECK(left.name == right.name);
	BOOST_CHECK(left.container_name == right.container_name);
	BOOST_CHECK(left.source_format == right.source_format);
	BOOST_CHECK(left.vertices.size() == right.vertices.size());
	BOOST_CHECK(left.indices == right.indices);
	BOOST_CHECK(left.submeshes.size() == right.submeshes.size());
	BOOST_CHECK(left.materials.size() == right.materials.size());
	BOOST_CHECK(left.dependencies.size() == right.dependencies.size());
	for (std::size_t index = 0; index < left.vertices.size() && index < right.vertices.size(); ++index) {
		BOOST_CHECK(left.vertices[index].position.x == right.vertices[index].position.x);
		BOOST_CHECK(left.vertices[index].position.y == right.vertices[index].position.y);
		BOOST_CHECK(left.vertices[index].position.z == right.vertices[index].position.z);
	}
}

}

BOOST_AUTO_TEST_CASE(real_generals_w3d_model_loads_and_is_deterministic)
{
	const std::filesystem::path path = Integration_Asset_Path();
	if (path.empty() || !std::filesystem::exists(path)) {
		BOOST_TEST_MESSAGE(
			"Skipping real-file integration check: set GENERALS_W3D_INTEGRATION_ASSET or "
			"RTS_W3D_INTEGRATION_ASSET. The default staged asset is "
			"Art/W3D/ABArFrcCmd_A2.W3D.");
		return;
	}

	const std::vector<Byte> source = Read_File(path);
	BOOST_REQUIRE(!source.empty());
	const Assets::AssetIdentity identity{
		Assets::AssetType::Model,
		Assets::Canonicalize_Asset_Name(path.string())};
	Assets::W3DAdapter adapter;
	BOOST_CHECK(adapter.Can_Import(identity, source));
	const auto first = adapter.Import(identity, source);
	const auto second = adapter.Import(identity, source);
	BOOST_REQUIRE(first.description != nullptr);
	BOOST_REQUIRE(second.description != nullptr);
	BOOST_REQUIRE(!first.description->vertices.empty());
	BOOST_REQUIRE(!first.description->indices.empty());
	BOOST_REQUIRE(!first.description->submeshes.empty());
	BOOST_REQUIRE(!first.description->materials.empty());
	BOOST_CHECK(first.description->bounds.Is_Valid());
	for (const auto &submesh : first.description->submeshes) {
		BOOST_CHECK(submesh.index_count != 0);
		BOOST_CHECK(submesh.first_index + submesh.index_count <= first.description->indices.size());
		BOOST_CHECK(submesh.material_index < first.description->materials.size());
	}
	for (const auto &material : first.description->materials)
		BOOST_CHECK(!material.name.empty());
	Check_Deterministic(*first.description, *second.description);
}

BOOST_AUTO_TEST_CASE(real_generals_w3d_model_requests_material_and_texture_dependencies)
{
	const std::filesystem::path path = Integration_Asset_Path();
	if (path.empty() || !std::filesystem::exists(path)) {
		BOOST_TEST_MESSAGE(
			"Skipping real-file dependency integration check: set GENERALS_W3D_INTEGRATION_ASSET or "
			"provide the documented staged asset.");
		return;
	}

	const std::vector<Byte> source = Read_File(path);
	BOOST_REQUIRE(!source.empty());
	std::mutex request_mutex;
	std::vector<Assets::AssetIdentity> requests;
	Assets::AssetCache cache([&source, &request_mutex, &requests](const Assets::AssetIdentity &identity) {
		{
			std::lock_guard lock(request_mutex);
			requests.push_back(identity);
		}
		if (identity.type == Assets::AssetType::Model)
			return source;
		if (identity.type == Assets::AssetType::Texture)
			return std::vector<Byte>{Byte{1}};
		return std::vector<Byte>{};
	});
	BOOST_REQUIRE(cache.Register_Model_Adapter(std::make_shared<Assets::W3DAdapter>()));

	const Assets::ModelAssetHandle model_handle = cache.Request_Model(path.string());
	cache.Wait(model_handle);
	BOOST_REQUIRE(cache.Get_State(model_handle) == Assets::AssetState::Ready);
	const Assets::ModelAsset *model = cache.Try_Get_Model(model_handle);
	BOOST_REQUIRE(model != nullptr);
	BOOST_REQUIRE(!model->Materials().empty());
	BOOST_REQUIRE(!cache.Model_Material_Dependencies(model_handle).empty());

	bool saw_texture_dependency = false;
	for (const Assets::ModelMaterial &model_material : model->Materials()) {
		BOOST_REQUIRE(model_material.asset_handle.Is_Valid());
		BOOST_CHECK(cache.Get_State(model_material.asset_handle) == Assets::AssetState::Ready);
		const Assets::MaterialAsset *material = cache.Try_Get_Material(model_material.asset_handle);
		BOOST_REQUIRE(material != nullptr);
		if (model_material.primary_texture.canonical_name.empty())
			continue;

		saw_texture_dependency = true;
		const Assets::TextureAssetHandle texture_handle = material->Primary_Texture();
		BOOST_REQUIRE(texture_handle.Is_Valid());
		BOOST_CHECK(cache.Get_State(texture_handle) == Assets::AssetState::Ready);
		BOOST_REQUIRE(cache.Try_Get_Texture(texture_handle) != nullptr);
	}
	BOOST_REQUIRE(saw_texture_dependency);
	BOOST_REQUIRE(cache.Texture_Count() != 0);

	std::lock_guard lock(request_mutex);
	BOOST_CHECK(std::any_of(
		requests.begin(),
		requests.end(),
		[](const Assets::AssetIdentity &identity) { return identity.type == Assets::AssetType::Model; }));
	BOOST_CHECK(std::any_of(
		requests.begin(),
		requests.end(),
		[](const Assets::AssetIdentity &identity) { return identity.type == Assets::AssetType::Texture; }));
}

BOOST_AUTO_TEST_CASE(scripted_evolution_airfield_mesh_loads_with_real_material_maps)
{
	// Generated by workspace genevo_mesh_codec.py from the user's extracted
	// ABAIRFIELD_SKN.USA_AIRFIELD.w3x. This validates mesh/material conversion,
	// not Zero Hour family placement, rigs, animations or gameplay replacement.
	const char *asset_path = std::getenv("GENERALS_W3D_SURFACE_ASSET");
	const char *texture_root = std::getenv("GENERALS_W3D_SURFACE_TEXTURES");
	if (!asset_path || !texture_root) {
		BOOST_TEST_MESSAGE("Set GENERALS_W3D_SURFACE_ASSET and GENERALS_W3D_SURFACE_TEXTURES for the scripted airfield fixture");
		return;
	}
	using namespace Assets;
	const auto bytes = Read_File(asset_path);
	BOOST_REQUIRE(!bytes.empty());
	AssetCache cache([&bytes, texture_root](const AssetIdentity &identity) {
		if (identity.type == AssetType::Model) return bytes;
		if (identity.type == AssetType::Texture)
			return Read_File(std::filesystem::path(texture_root) / identity.canonical_name);
		return std::vector<Byte>{};
	});
	BOOST_REQUIRE(cache.Register_Model_Adapter(std::make_shared<W3DAdapter>()));
	const auto handle = cache.Request_Model("airfield_surface_fixture.w3d");
	cache.Wait(handle);
	BOOST_REQUIRE_MESSAGE(cache.Get_State(handle) == AssetState::Ready, cache.Get_Error(handle));
	const auto *model = cache.Try_Get_Model(handle);
	BOOST_REQUIRE(model != nullptr);
	BOOST_TEST(model->Indices().size() == 10742u * 3u);
	BOOST_REQUIRE_EQUAL(model->Materials().size(), 1u);
	BOOST_REQUIRE_EQUAL(model->Submeshes().size(), 1u);
	const auto *material = cache.Try_Get_Material(model->Materials()[0].asset_handle);
	BOOST_REQUIRE(material != nullptr);
	BOOST_CHECK(material->Surface().shading_model == MaterialShadingModel::SpecularGlossiness);
	BOOST_CHECK(material->Surface().specular_channel == MaterialTextureChannel::Red);
	BOOST_CHECK(material->Surface_Texture(MaterialTextureRole::TeamColor) == material->Surface_Texture(MaterialTextureRole::Specular));
	BOOST_TEST(cache.Model_Texture_Dependencies(handle).size() == 3u);
	for (const auto texture_handle : cache.Model_Texture_Dependencies(handle)) {
		const auto *texture = cache.Try_Get_Texture(texture_handle);
		BOOST_REQUIRE(texture != nullptr);
		BOOST_TEST(texture->Has_Pixels());
		BOOST_TEST(texture->Width() > 0u);
	}
	for (const auto index : model->Indices()) {
		BOOST_REQUIRE(index < model->Vertices().size());
		const auto &vertex = model->Vertices()[index];
		const float length = vertex.tangent.x * vertex.tangent.x + vertex.tangent.y * vertex.tangent.y + vertex.tangent.z * vertex.tangent.z;
		BOOST_TEST(std::isfinite(length));
		BOOST_TEST(length == 1.0f, boost::test_tools::tolerance(.001f));
		BOOST_TEST(std::abs(vertex.tangent_sign) == 1.0f);
	}
}

BOOST_AUTO_TEST_CASE(scripted_airfield_door_parts_load_with_surface_dependencies)
{
	// Workspace genevo_airfield_doors.py stages these from the user's Evolution
	// doors and original Zero Hour rigs. This checks imported geometry/materials;
	// animation evaluation and aperture fit require separate runtime validation.
	const char *directory = std::getenv("GENERALS_W3D_DOOR_DIRECTORY");
	const char *texture_root = std::getenv("GENERALS_W3D_SURFACE_TEXTURES");
	if (!directory || !texture_root) {
		BOOST_TEST_MESSAGE("Set GENERALS_W3D_DOOR_DIRECTORY and GENERALS_W3D_SURFACE_TEXTURES for converted door coverage");
		return;
	}
	using namespace Assets;
	for (const auto *suffix : {"A2", "A3", "A7", "A8"}) {
		BOOST_TEST_CONTEXT("Converted airfield door " << suffix) {
			const auto filename = std::string("ABArFrcCmd_") + suffix + ".W3D";
			const auto bytes = Read_File(std::filesystem::path(directory) / filename);
			BOOST_REQUIRE(!bytes.empty());
			AssetCache cache([&bytes, texture_root](const AssetIdentity &identity) {
				if (identity.type == AssetType::Model) return bytes;
				if (identity.type == AssetType::Texture)
					return Read_File(std::filesystem::path(texture_root) / identity.canonical_name);
				return std::vector<Byte>{};
			});
			BOOST_REQUIRE(cache.Register_Model_Adapter(std::make_shared<W3DAdapter>()));
			const auto handle = cache.Request_Model(filename);
			cache.Wait(handle);
			BOOST_REQUIRE_MESSAGE(cache.Get_State(handle) == AssetState::Ready, cache.Get_Error(handle));
			const auto *model = cache.Try_Get_Model(handle);
			BOOST_REQUIRE(model);
			BOOST_TEST(model->Rig().bones.size() == (suffix[1] < '7' ? 4u : 2u));
			BOOST_REQUIRE_EQUAL(model->Rig().animations.size(), 1u);
			BOOST_TEST(model->Rig().animations[0].frame_count == 41u);
			BOOST_TEST(model->Rig().animations[0].channels_available);
			const auto expected_parts = suffix[1] < '7' ? 3u : 1u;
			BOOST_TEST(model->Submeshes().size() == expected_parts);
			BOOST_TEST(model->Materials().size() == expected_parts);
			BOOST_TEST(cache.Model_Texture_Dependencies(handle).size() == 3u);
			for (const auto &part : model->Submeshes()) {
				BOOST_TEST(part.name.starts_with("DOOR"));
				BOOST_TEST(part.index_count >= 36u);
			}
			for (const auto &binding : model->Materials()) {
				const auto *material = cache.Try_Get_Material(binding.asset_handle);
				BOOST_REQUIRE(material);
				BOOST_CHECK(material->Render_Mode() == MaterialRenderMode::AlphaTest);
				BOOST_CHECK(material->Surface().shading_model == MaterialShadingModel::SpecularGlossiness);
				BOOST_TEST(material->Surface().alpha_cutoff == 96.0f / 255.0f);
				BOOST_CHECK(material->Surface_Texture(MaterialTextureRole::Normal).Is_Valid());
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(scripted_weighted_assembly_loads_full_precision_skin_bindings)
{
    const char *path=std::getenv("GENERALS_W3D_SKIN_ASSET");
    const char *texture_root=std::getenv("GENERALS_W3D_SURFACE_TEXTURES");
    if (!path || !texture_root) { BOOST_TEST_MESSAGE("Set GENERALS_W3D_SKIN_ASSET for weighted assembly coverage"); return; }
    using namespace Assets;
    const auto bytes=Read_File(path);
    BOOST_REQUIRE(!bytes.empty());
    AssetCache cache([&](const AssetIdentity &identity) {
        if (identity.type==AssetType::Model) return bytes;
        if (identity.type==AssetType::Texture) return Read_File(std::filesystem::path(texture_root)/identity.canonical_name);
        return std::vector<Byte>{};
    });
    BOOST_REQUIRE(cache.Register_Model_Adapter(std::make_shared<W3DAdapter>()));
    const auto handle=cache.Request_Model("weighted_assembly.w3d"); cache.Wait(handle);
    BOOST_REQUIRE_MESSAGE(cache.Get_State(handle)==AssetState::Ready,cache.Get_Error(handle));
    const auto *model=cache.Try_Get_Model(handle); BOOST_REQUIRE(model);
    std::size_t blended=0;
    for (const auto &vertex:model->Vertices()) {
        float total=0;
        for (std::size_t i=0;i<4;++i) {
            total+=vertex.bone_weights[i];
            BOOST_REQUIRE(vertex.bone_indices[i]<model->Rig().bones.size());
        }
        BOOST_TEST(total==1.0f,boost::test_tools::tolerance(.00001f));
        if (vertex.bone_weights[1]>0) ++blended;
    }
    BOOST_TEST(blended>10u);
    BOOST_TEST(model->Skin_Bone_Count()>0u);
    BOOST_TEST(model->Skin_Bone_Count()<=model->Rig().bones.size());
}

BOOST_AUTO_TEST_CASE(external_game_rigs_decode_without_changing_legacy_geometry)
{
	const char *directory=std::getenv("GENERALS_W3D_PASS_DIRECTORY");
	if (!directory) { BOOST_TEST_MESSAGE("Set GENERALS_W3D_PASS_DIRECTORY for original rig coverage"); return; }
	std::size_t files=0;
	for (const auto &entry:std::filesystem::directory_iterator(directory)) {
		const auto extension=entry.path().extension().string();
		if(extension!=".W3D" && extension!=".w3d") continue;
		const auto bytes=Read_File(entry.path());
		if(!Assets::W3D::W3DValidate_Chunk_Tree(bytes)) continue;
		Assets::ModelRigDesc rig; std::string error;
		BOOST_TEST_CONTEXT(entry.path().filename().string()) {
			BOOST_CHECK_MESSAGE(Assets::W3D::W3DRead_Model_Rig(bytes,rig,error),error);
		}
		++files;
	}
	BOOST_TEST(files>8000u);
}

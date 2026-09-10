#define BOOST_TEST_MODULE GraphicsW3DAssetCatalogTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

import Assets.Adapters.W3D.Null;
import Assets.Cache.Animations;
import Assets.Images.PixelEncoding;
import Graphics.RHI;
import Graphics.Resources.Textures.Resource;
import Graphics.Scene.Props.Constants;
import Graphics.Scene.Props.Geometry;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;

#include "W3DDevice/GameClient/NullRenderObject.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "WWLib/RAMFILE.h"
#include "WWLib/chunkio.h"
#include "WWLib/ref_ptr.h"
#include "WWSaveLoad/persistfactory.h"
#include "WWSaveLoad/saveload.h"

#ifndef GRAPHICS_W3D_CATALOG_SHADER_DIRECTORY
#define GRAPHICS_W3D_CATALOG_SHADER_DIRECTORY "."
#endif

namespace
{

using Byte = std::byte;
using Prototype = W3DAssetCatalog::Prototype;

void Store_U32(std::vector<Byte> &bytes, std::size_t offset, std::uint32_t value)
{
	BOOST_REQUIRE(offset <= bytes.size() && bytes.size() - offset >= sizeof(value));
	for (unsigned shift = 0; shift < 32; shift += 8)
		bytes[offset + shift / 8] = static_cast<Byte>((value >> shift) & 0xffu);
}

std::vector<Byte> Make_Null_W3D(std::string_view name)
{
	constexpr std::size_t header_size = 8;
	std::vector<Byte> bytes(header_size + Assets::W3D::W3DNullPayloadSize);
	Store_U32(bytes, 0, Assets::W3D::W3DChunkNullObject);
	Store_U32(bytes, 4, static_cast<std::uint32_t>(Assets::W3D::W3DNullPayloadSize));
	Store_U32(bytes, header_size, 1u);
	Store_U32(bytes, header_size + 4, 0u);
	const std::size_t length = (std::min)(name.size(), Assets::W3D::W3DNullNameSize - 1);
	std::memcpy(bytes.data() + header_size + Assets::W3D::W3DNullNameOffset,
		name.data(), length);
	return bytes;
}

class MemoryFileFactory final : public FileFactoryClass
{
public:
	void Add_File(std::string name, std::vector<Byte> contents)
	{
		m_files.insert_or_assign(std::move(name), std::move(contents));
	}

	const std::vector<std::string> &Requests() const noexcept
	{
		return m_requests;
	}

	FileClass *Get_File(const char *filename) override
	{
		m_requests.emplace_back(filename == nullptr ? "" : filename);
		const auto found = m_files.find(m_requests.back());
		if (found == m_files.end())
			return nullptr;

		return new RAMFileClass(found->second.data(), static_cast<int>(found->second.size()));
	}

	void Return_File(FileClass *file) override
	{
		delete file;
	}

private:
	std::unordered_map<std::string, std::vector<Byte>> m_files;
	std::vector<std::string> m_requests;
};

std::unique_ptr<Prototype> Make_Null_Prototype(std::string identity)
{
	const std::string factory_name = identity;
	return std::make_unique<Prototype>(factory_name, W3DRenderObject::CLASSID_NULL,
		[identity = std::move(identity)] {
			return new NullRenderObject(identity.c_str());
		});
}

void Install_Null_Decoder(W3DAssetCatalog &catalog)
{
	BOOST_REQUIRE(catalog.Register_Model_Decoder(
		Assets::W3D::W3DChunkNullObject, Load_Null_Factory));
}

void Install_Reserved_Null(W3DAssetCatalog &catalog)
{
	BOOST_REQUIRE(catalog.Install_Reserved_Model_Factory(
		std::unique_ptr<Prototype>(Create_Null_Render_Object_Factory())));
}

int g_first_decoder_calls = 0;
int g_second_decoder_calls = 0;

Prototype *Decode_First(ChunkLoadClass &chunks)
{
	++g_first_decoder_calls;
	if (chunks.Seek(chunks.Cur_Chunk_Length()) != chunks.Cur_Chunk_Length())
		return nullptr;
	return Make_Null_Prototype("FirstDecoder").release();
}

Prototype *Decode_Second(ChunkLoadClass &chunks)
{
	++g_second_decoder_calls;
	if (chunks.Seek(chunks.Cur_Chunk_Length()) != chunks.Cur_Chunk_Length())
		return nullptr;
	return Make_Null_Prototype("SecondDecoder").release();
}

struct DestructionCounter final
{
	int *count = nullptr;
	~DestructionCounter()
	{
		if (count != nullptr)
			++*count;
	}
};

struct CatalogTextureFixture final
{
	Graphics::GraphicsTestDevice device{{true}};
	W3DAssetCatalog catalog;
	Graphics::PropRenderer renderer;
	Graphics::RHITextureHandle target{};
	Graphics::RHITextureHandle depth{};
	Graphics::PropMeshHandle mesh{};

	CatalogTextureFixture()
	{
		BOOST_REQUIRE(device.Is_Valid());
		BOOST_REQUIRE(renderer.Initialize(device,
			Graphics::Test_Shader_Directory(GRAPHICS_W3D_CATALOG_SHADER_DIRECTORY)));
		target = device.Create_Texture({8, 8, 1, Graphics::RHITextureFormat::RGBA8_UNorm,
			static_cast<std::uint32_t>(Graphics::RHITextureUsage::RenderTarget)});
		depth = device.Create_Texture({8, 8, 1, Graphics::RHITextureFormat::D32_Float,
			static_cast<std::uint32_t>(Graphics::RHITextureUsage::DepthStencil)});
		BOOST_REQUIRE(target.Is_Valid());
		BOOST_REQUIRE(depth.Is_Valid());

		std::array<Graphics::PropVertex, 4> vertices{};
		vertices[0].position = {-1.0f, -1.0f, 0.5f};
		vertices[1].position = {1.0f, -1.0f, 0.5f};
		vertices[2].position = {1.0f, 1.0f, 0.5f};
		vertices[3].position = {-1.0f, 1.0f, 0.5f};
		vertices[0].uv = {0.0f, 1.0f};
		vertices[1].uv = {1.0f, 1.0f};
		vertices[2].uv = {1.0f, 0.0f};
		vertices[3].uv = {0.0f, 0.0f};
		const std::array<std::uint32_t, 6> indices{0, 1, 2, 0, 2, 3};
		mesh = renderer.Create_Mesh(vertices, indices);
		BOOST_REQUIRE(mesh.Is_Valid());
	}

	~CatalogTextureFixture()
	{
		if (mesh.Is_Valid())
			renderer.Destroy_Mesh(mesh);
		renderer.Shutdown();
		if (target.Is_Valid())
			device.Destroy_Texture(target);
		if (depth.Is_Valid())
			device.Destroy_Texture(depth);
	}

	std::array<int, 4> Draw_Texture(Graphics::RHITextureHandle texture,
		const std::array<float, 4> &clear_color)
	{
		Graphics::PropStyle style;
		style.blend = Graphics::RHIBlendMode::Disabled;
		style.depth_test = false;
		style.depth_write = false;
		style.cull = Graphics::RHICullMode::None;
		Graphics::PropParameters parameters;
		parameters.view_projection = {1, 0, 0, 0, 0, 1, 0, 0,
			0, 0, 1, 0, 0, 0, 0, 1};
		parameters.primary_gradient = 0;
		parameters.textured = 1;

		Graphics::CommandList &commands = device.Immediate_Command_List();
		BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
		BOOST_REQUIRE(commands.Set_Viewport({0, 0, 8, 8}));
		BOOST_REQUIRE(commands.Clear(clear_color, 1.0f));
		BOOST_REQUIRE(renderer.Draw(commands, mesh, style, parameters, {texture}));

		std::array<Byte, 8 * 8 * 4> pixels{};
		BOOST_REQUIRE(device.Readback_Texture(target, pixels, 8 * 4));
		const std::size_t offset = (4 * 8 + 4) * 4;
		return {
			std::to_integer<int>(pixels[offset]),
			std::to_integer<int>(pixels[offset + 1]),
			std::to_integer<int>(pixels[offset + 2]),
			std::to_integer<int>(pixels[offset + 3])};
	}
};

RefCountPtr<W3DRenderObject> Roundtrip_Render_Object(W3DRenderObject &source)
{
	RAMFileClass file(nullptr, 1024);
	if (!file.Open(FileClass::WRITE))
		return {};
	ChunkSaveClass save(&file);
	if (!save.Begin_Chunk(source.Get_Factory().Chunk_ID()))
		return {};
	source.Get_Factory().Save(save, &source);
	if (!save.End_Chunk())
		return {};
	file.Close();

	if (!file.Open(FileClass::READ))
		return {};
	ChunkLoadClass load(&file);
	if (!load.Open_Chunk())
		return {};
	PersistFactoryClass *factory =
		SaveLoadSystemClass::Find_Persist_Factory(load.Cur_Chunk_ID());
	if (factory == nullptr)
		return {};
	PersistClass *persisted = factory->Load(load);
	if (!load.Close_Chunk())
		return {};
	file.Close();
	return Create_No_Add_Ref(static_cast<W3DRenderObject *>(persisted));
}

}

BOOST_AUTO_TEST_CASE(available_model_file_is_not_retried_for_a_missing_asset)
{
	MemoryFileFactory files;
	files.Add_File("Primary.w3d", Make_Null_W3D("Primary"));
	std::vector<std::string> observed;
	W3DAssetCatalog catalog(&files, {}, [&](std::string_view filename) {
		observed.emplace_back(filename);
	});
	Install_Null_Decoder(catalog);
	catalog.Set_Load_On_Demand(true);

	RefCountPtr<W3DRenderObject> missing =
		Create_No_Add_Ref(catalog.Create_Render_Obj("Primary.Missing"));
	BOOST_CHECK(missing == nullptr);
	BOOST_REQUIRE_EQUAL(files.Requests().size(), 1u);
	BOOST_CHECK_EQUAL(files.Requests()[0], "Primary.w3d");
	BOOST_REQUIRE_EQUAL(observed.size(), 1u);
	BOOST_CHECK_EQUAL(observed[0], "Primary.w3d");
	BOOST_CHECK(catalog.Render_Obj_Exists("Primary"));

	BOOST_REQUIRE(catalog.Load_3D_Assets("Primary.w3d"));
	BOOST_CHECK_EQUAL(files.Requests().size(), 1u);
	BOOST_CHECK_EQUAL(observed.size(), 1u);
}

BOOST_AUTO_TEST_CASE(missing_primary_model_file_uses_parent_directory_once)
{
	MemoryFileFactory files;
	files.Add_File("..\\Tank.w3d", Make_Null_W3D("Tank.Turret"));
	W3DAssetCatalog catalog(&files);
	Install_Null_Decoder(catalog);
	catalog.Set_Load_On_Demand(true);

	RefCountPtr<W3DRenderObject> object =
		Create_No_Add_Ref(catalog.Create_Render_Obj("Tank.Turret"));
	BOOST_REQUIRE(object != nullptr);
	BOOST_CHECK_EQUAL(std::string(object->Get_Name()), "Tank.Turret");
	BOOST_REQUIRE_EQUAL(files.Requests().size(), 2u);
	BOOST_CHECK_EQUAL(files.Requests()[0], "Tank.w3d");
	BOOST_CHECK_EQUAL(files.Requests()[1], "..\\Tank.w3d");
}

BOOST_AUTO_TEST_CASE(animation_missing_is_cached_and_suppresses_follow_up_io_and_reports)
{
	Assets::Get_Animation_Cache().Reset_Missing();
	MemoryFileFactory files;
	W3DAssetCatalog catalog(&files);
	catalog.Set_Load_On_Demand(true);
	catalog.Report().Enable_Load_On_Demand_Reporting(true);

	BOOST_CHECK(!catalog.Acquire_Animation("Rig.Missing" ).Is_Valid());
	BOOST_CHECK(Assets::Get_Animation_Cache().Is_Missing("rig.missing"));
	BOOST_REQUIRE_EQUAL(files.Requests().size(), 2u);
	BOOST_CHECK_EQUAL(files.Requests()[0], "Missing.w3d");
	BOOST_CHECK_EQUAL(files.Requests()[1], "..\\Missing.w3d");
	BOOST_CHECK_EQUAL(catalog.Report().Load_On_Demand_Count(
		Assets::AssetReportCategory::Animation, "Rig.Missing"), 1u);
	BOOST_CHECK_EQUAL(catalog.Report().Missing_Count(
		Assets::AssetReportCategory::Animation, "Rig.Missing"), 1u);

	BOOST_CHECK(!catalog.Acquire_Animation("RIG.MISSING").Is_Valid());
	BOOST_CHECK_EQUAL(files.Requests().size(), 2u);
	BOOST_CHECK_EQUAL(catalog.Report().Load_On_Demand_Count(
		Assets::AssetReportCategory::Animation, "Rig.Missing"), 1u);
	BOOST_CHECK_EQUAL(catalog.Report().Missing_Count(
		Assets::AssetReportCategory::Animation, "Rig.Missing"), 1u);
}

BOOST_AUTO_TEST_CASE(decoder_registration_is_first_wins)
{
	g_first_decoder_calls = 0;
	g_second_decoder_calls = 0;
	MemoryFileFactory files;
	std::vector<Byte> contents = Make_Null_W3D("Ignored");
	const std::vector<Byte> second = Make_Null_W3D("Ignored");
	contents.insert(contents.end(), second.begin(), second.end());
	files.Add_File("Decoder.w3d", std::move(contents));
	W3DAssetCatalog catalog(&files);
	BOOST_REQUIRE(catalog.Register_Model_Decoder(
		Assets::W3D::W3DChunkNullObject, Decode_First));
	BOOST_CHECK(!catalog.Register_Model_Decoder(
		Assets::W3D::W3DChunkNullObject, Decode_Second));
	BOOST_REQUIRE(catalog.Load_3D_Assets("Decoder.w3d"));
	BOOST_CHECK_EQUAL(g_first_decoder_calls, 2);
	BOOST_CHECK_EQUAL(g_second_decoder_calls, 0);
	BOOST_CHECK(catalog.Render_Obj_Exists("FirstDecoder"));
	BOOST_CHECK(!catalog.Render_Obj_Exists("SecondDecoder"));
	BOOST_CHECK_EQUAL(catalog.Prototype_Count(), 1u);
}

BOOST_AUTO_TEST_CASE(duplicate_factory_is_destroyed_and_reserved_null_survives_free)
{
	W3DAssetCatalog catalog;
	Install_Reserved_Null(catalog);
	Install_Null_Decoder(catalog);
	BOOST_REQUIRE(catalog.Add_Prototype(Make_Null_Prototype("Authored.Null")));
	Prototype *reserved = catalog.Find_Prototype("NULL");
	BOOST_REQUIRE(reserved != nullptr);

	int destroyed = 0;
	auto lifetime = std::make_shared<DestructionCounter>();
	lifetime->count = &destroyed;
	auto duplicate = std::make_unique<Prototype>("NULL", W3DRenderObject::CLASSID_NULL,
		[lifetime] { return new NullRenderObject("duplicate"); });
	BOOST_CHECK(catalog.Add_Prototype(std::move(duplicate)));
	BOOST_CHECK_EQUAL(destroyed, 0);
	lifetime.reset();
	BOOST_CHECK_EQUAL(destroyed, 0);

	catalog.Free_Assets();
	BOOST_CHECK_EQUAL(destroyed, 1);
	BOOST_CHECK(catalog.Find_Prototype("null") == reserved);
	BOOST_CHECK(catalog.Find_Prototype("Authored.Null") == nullptr);
}

BOOST_AUTO_TEST_CASE(public_prototype_insertion_shadows_and_removal_reveals)
{
	W3DAssetCatalog catalog;
	Install_Reserved_Null(catalog);

	int first_created = 0;
	int second_created = 0;
	auto first = std::make_unique<Prototype>("Shared.Null", W3DRenderObject::CLASSID_NULL,
		[&first_created] {
			++first_created;
			return new NullRenderObject("first");
		});
	auto second = std::make_unique<Prototype>("SHARED.NULL", W3DRenderObject::CLASSID_NULL,
		[&second_created] {
			++second_created;
			return new NullRenderObject("second");
		});
	BOOST_REQUIRE(catalog.Add_Prototype(std::move(first)));
	BOOST_REQUIRE(catalog.Add_Prototype(std::move(second)));

	RefCountPtr<W3DRenderObject> newest =
		Create_No_Add_Ref(catalog.Create_Render_Obj("shared.null"));
	BOOST_REQUIRE(newest != nullptr);
	BOOST_CHECK_EQUAL(std::string(newest->Get_Name()), "second");
	BOOST_CHECK_EQUAL(first_created, 0);
	BOOST_CHECK_EQUAL(second_created, 1);

	BOOST_REQUIRE(catalog.Remove_Prototype("SHARED.NULL"));
	RefCountPtr<W3DRenderObject> earlier =
		Create_No_Add_Ref(catalog.Create_Render_Obj("Shared.Null"));
	BOOST_REQUIRE(earlier != nullptr);
	BOOST_CHECK_EQUAL(std::string(earlier->Get_Name()), "first");
	BOOST_CHECK_EQUAL(first_created, 1);
	BOOST_CHECK_EQUAL(second_created, 1);
	BOOST_CHECK(!catalog.Remove_Prototype("NULL"));
}

BOOST_AUTO_TEST_CASE(texture_first_request_and_cached_type_policy_are_retained)
{
	bool policy_called = false;
	W3DAssetCatalog catalog(nullptr,
		[&](std::string_view filename, bool &allow_reduction) {
			policy_called = filename == "ZHCInfantry.tga";
			allow_reduction = false;
		});

	W3DTextureHandle *first = catalog.Get_Texture("ZHCInfantry.tga", MIP_LEVELS_ALL,
		Assets::PixelEncoding::Unknown, true, W3DTextureHandle::TEX_REGULAR, true);
	BOOST_REQUIRE(first != nullptr);
	BOOST_CHECK(policy_called);
	BOOST_CHECK(!first->Is_Reducible());
	BOOST_CHECK_EQUAL(first->Get_Mip_Level_Count(), MIP_LEVELS_ALL);
	BOOST_CHECK_EQUAL(first->Num_Refs(), 2);

	W3DTextureHandle *second = catalog.Get_Texture("zhcinfantry.tga", MIP_LEVELS_1,
		Assets::PixelEncoding::BGRA8, false, W3DTextureHandle::TEX_DEPTH, true);
	BOOST_REQUIRE(second == first);
	BOOST_CHECK_EQUAL(first->Get_Mip_Level_Count(), MIP_LEVELS_ALL);
	BOOST_CHECK_EQUAL(first->Num_Refs(), 3);
	second->Release_Ref();
	first->Release_Ref();
	catalog.Release_Unused_Textures();
	BOOST_CHECK_EQUAL(catalog.Texture_Count(), 0u);

	BOOST_CHECK(catalog.Get_Texture("new-depth.tga", MIP_LEVELS_1,
		Assets::PixelEncoding::Unknown, true, W3DTextureHandle::TEX_DEPTH, true) == nullptr);
}

BOOST_AUTO_TEST_CASE(game_asset_manager_installs_texture_request_policy_on_its_catalog)
{
	W3DAssetManager manager;
	W3DAssetCatalog *catalog = W3DAssetCatalog::Get_Instance();
	BOOST_REQUIRE(catalog == &manager.Catalog());

	W3DTextureHandle *house_color = W3DAssetCatalog::Get_Instance()->Get_Texture("ZhCInfantry.tga",
		MIP_LEVELS_ALL, Assets::PixelEncoding::Unknown, true,
		W3DTextureHandle::TEX_REGULAR, true);
	BOOST_REQUIRE(house_color != nullptr);
	BOOST_CHECK(!house_color->Is_Reducible());

	W3DTextureHandle *ordinary = W3DAssetCatalog::Get_Instance()->Get_Texture("ordinary.tga",
		MIP_LEVELS_ALL, Assets::PixelEncoding::Unknown, true,
		W3DTextureHandle::TEX_REGULAR, true);
	BOOST_REQUIRE(ordinary != nullptr);
	BOOST_CHECK(ordinary->Is_Reducible());

	house_color->Release_Ref();
	ordinary->Release_Ref();
}

BOOST_AUTO_TEST_CASE(adopted_recolored_handles_are_visible_and_collision_safe)
{
	W3DAssetCatalog catalog;
	RefCountPtr<W3DTextureHandle> created =
		Create_No_Add_Ref(new W3DTextureHandle("recolored.tga", nullptr,
			MIP_LEVELS_1, Assets::PixelEncoding::Unknown, true, false));
	W3DTextureHandle *adopted = catalog.Adopt_Texture(created);
	BOOST_REQUIRE(adopted != nullptr);
	BOOST_CHECK(created == nullptr);
	BOOST_CHECK(catalog.Find_Texture_Borrowed("RECOLORED.TGA") == adopted);
	BOOST_CHECK_EQUAL(adopted->Num_Refs(), 2);

	RefCountPtr<W3DTextureHandle> duplicate =
		Create_No_Add_Ref(new W3DTextureHandle("RECOLORED.TGA", nullptr,
			MIP_LEVELS_ALL, Assets::PixelEncoding::BGRA8, false, true));
	W3DTextureHandle *existing = catalog.Adopt_Texture(duplicate);
	BOOST_CHECK(existing == adopted);
	BOOST_CHECK(duplicate == nullptr);
	BOOST_CHECK_EQUAL(adopted->Num_Refs(), 3);

	W3DTextureHandle *repeated = catalog.Get_Texture("recolored.tga", MIP_LEVELS_1,
		Assets::PixelEncoding::RGBA8, false);
	BOOST_CHECK(repeated == adopted);
	BOOST_CHECK_EQUAL(adopted->Num_Refs(), 4);
	repeated->Release_Ref();
	existing->Release_Ref();
	BOOST_CHECK_EQUAL(adopted->Num_Refs(), 2);
	catalog.Release_Unused_Textures();
	BOOST_CHECK_EQUAL(catalog.Texture_Count(), 1u);
	adopted->Release_Ref();
	catalog.Release_Unused_Textures();
	BOOST_CHECK_EQUAL(catalog.Texture_Count(), 0u);
}

BOOST_AUTO_TEST_CASE(texture_owner_keeps_gpu_resource_resident_and_drawable)
{
	CatalogTextureFixture fixture;
	W3DTextureHandle *first = fixture.catalog.Get_Texture("catalog-color.tga", MIP_LEVELS_1,
		Assets::PixelEncoding::Unknown, false, W3DTextureHandle::TEX_REGULAR, false);
	BOOST_REQUIRE(first != nullptr);
	W3DTextureHandle *retained = fixture.catalog.Get_Texture("CATALOG-COLOR.TGA",
		MIP_LEVELS_ALL, Assets::PixelEncoding::BGRA8, true,
		W3DTextureHandle::TEX_CUBEMAP, true);
	BOOST_REQUIRE(retained == first);

	Graphics::TextureResource *resource = Graphics::TextureResource::Create(
		&fixture.device, {1, 1, 1}, Assets::PixelEncoding::RGBA8);
	BOOST_REQUIRE(resource != nullptr);
	const std::array<Byte, 4> color{Byte{220}, Byte{70}, Byte{35}, Byte{180}};
	BOOST_REQUIRE(fixture.device.Update_Texture(resource->Handle(),
		{std::as_bytes(std::span(color)), 4}));
	resource->Retain();
	std::shared_ptr<Graphics::TextureResource> external_resource(resource,
		[](Graphics::TextureResource *value) { value->Release(); });
	BOOST_CHECK_EQUAL(external_resource->Reference_Count(), 2u);
	first->Set_Render_Backend_Texture(resource);
	const Graphics::RHITextureHandle graphics_texture = first->Peek_Graphics_Texture();
	BOOST_REQUIRE(graphics_texture.Is_Valid());

	first->Release_Ref();
	BOOST_REQUIRE_EQUAL(first->Num_Refs(), 2);
	fixture.catalog.Release_Unused_Textures();
	BOOST_CHECK_EQUAL(fixture.catalog.Texture_Count(), 1u);
	BOOST_CHECK_EQUAL(external_resource->Reference_Count(), 2u);
	BOOST_CHECK(first->Peek_Graphics_Texture() == graphics_texture);

	const std::array<int, 4> pixel = fixture.Draw_Texture(graphics_texture,
		{0.02f, 0.04f, 0.06f, 1.0f});
	BOOST_CHECK_SMALL(pixel[0] - 220, 2);
	BOOST_CHECK_SMALL(pixel[1] - 70, 2);
	BOOST_CHECK_SMALL(pixel[2] - 35, 2);
	BOOST_CHECK_SMALL(pixel[3] - 180, 2);

	fixture.catalog.Free_Assets();
	BOOST_CHECK_EQUAL(fixture.catalog.Texture_Count(), 0u);
	BOOST_CHECK_EQUAL(external_resource->Reference_Count(), 2u);
	BOOST_CHECK(first->Peek_Graphics_Texture() == graphics_texture);
	const std::array<int, 4> pixel_after_free = fixture.Draw_Texture(graphics_texture,
		{0.02f, 0.04f, 0.06f, 1.0f});
	BOOST_CHECK_SMALL(pixel_after_free[0] - 220, 2);
	BOOST_CHECK_SMALL(pixel_after_free[1] - 70, 2);
	BOOST_CHECK_SMALL(pixel_after_free[2] - 35, 2);
	BOOST_CHECK_SMALL(pixel_after_free[3] - 180, 2);

	retained->Release_Ref();
	retained = nullptr;
	BOOST_CHECK_EQUAL(external_resource->Reference_Count(), 1u);
	external_resource.reset();
	BOOST_CHECK(!fixture.device.Retain_Texture(graphics_texture));
	BOOST_CHECK(!fixture.device.Destroy_Texture(graphics_texture));
}

BOOST_AUTO_TEST_CASE(more_than_one_batch_of_unused_textures_is_fully_released)
{
	W3DAssetCatalog catalog;
	W3DTextureHandle *retained = nullptr;
	for (unsigned index = 0; index < 301; ++index) {
		const std::string name = "bulk-" + std::to_string(index) + ".tga";
		W3DTextureHandle *texture = catalog.Get_Texture(name.c_str(), MIP_LEVELS_1,
			Assets::PixelEncoding::Unknown, true);
		BOOST_REQUIRE(texture != nullptr);
		if (index == 0)
			retained = texture;
		else
			texture->Release_Ref();
	}

	BOOST_REQUIRE(retained != nullptr);
	BOOST_CHECK_EQUAL(catalog.Texture_Count(), 301u);
	BOOST_CHECK_EQUAL(retained->Num_Refs(), 2);
	catalog.Release_Unused_Textures();
	BOOST_CHECK_EQUAL(catalog.Texture_Count(), 1u);
	BOOST_CHECK(catalog.Find_Texture_Borrowed("BULK-0.TGA") == retained);
	BOOST_CHECK_EQUAL(retained->Num_Refs(), 2);
	retained->Release_Ref();
	catalog.Release_Unused_Textures();
	BOOST_CHECK_EQUAL(catalog.Texture_Count(), 0u);
}

BOOST_AUTO_TEST_CASE(render_object_persistence_uses_named_and_reserved_catalog_factories)
{
	W3DAssetCatalog catalog;
	Install_Reserved_Null(catalog);
	BOOST_REQUIRE(catalog.Add_Prototype(Make_Null_Prototype("Saved.Null")));

	RefCountPtr<W3DRenderObject> named_source =
		Create_No_Add_Ref(new NullRenderObject("Saved.Null"));
	named_source->Set_Position(Vector3(7, 8, 9));
	RefCountPtr<W3DRenderObject> named = Roundtrip_Render_Object(*named_source);
	BOOST_REQUIRE(named != nullptr);
	BOOST_CHECK_EQUAL(std::string(named->Get_Name()), "Saved.Null");
	BOOST_CHECK(named->Get_Position() == Vector3(7, 8, 9));

	RefCountPtr<W3DRenderObject> missing_source =
		Create_No_Add_Ref(new NullRenderObject("Missing.Null"));
	missing_source->Set_Position(Vector3(2, 4, 6));
	RefCountPtr<W3DRenderObject> missing = Roundtrip_Render_Object(*missing_source);
	BOOST_REQUIRE(missing != nullptr);
	BOOST_CHECK_EQUAL(std::string(missing->Get_Name()), "NULL");
	BOOST_CHECK(missing->Get_Position() == Vector3(2, 4, 6));
}

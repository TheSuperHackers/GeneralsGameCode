
#define BOOST_TEST_MODULE NullRenderObjectTests
#include <boost/test/included/unit_test.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <span>
#include <string>
#include <vector>

import Assets.Adapters.W3D.Null;
import Graphics.Tests.Device;
import Graphics.RHI;
import Graphics.Scene.Models.Factory;

#include "W3DDevice/GameClient/NullRenderObject.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DSceneQueryMask.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "WWMath/aabox.h"
#include "WWMath/lineseg.h"
#include "WWMath/obbox.h"
#include "WWLib/RAMFILE.h"
#include "WWLib/chunkio.h"
#include "WWLib/ref_ptr.h"
#include "WWSaveLoad/persistfactory.h"
#include "WWSaveLoad/saveload.h"

namespace
{

constexpr std::size_t NullNameCapacity = 31;

using NullPayload = std::array<std::byte, Assets::W3D::W3DNullPayloadSize>;

void Store_U32(NullPayload &bytes, std::size_t offset, std::uint32_t value)
{
	BOOST_REQUIRE(offset <= bytes.size() && bytes.size() - offset >= sizeof(value));
	bytes[offset] = static_cast<std::byte>(value & 0xffu);
	bytes[offset + 1] = static_cast<std::byte>((value >> 8) & 0xffu);
	bytes[offset + 2] = static_cast<std::byte>((value >> 16) & 0xffu);
	bytes[offset + 3] = static_cast<std::byte>((value >> 24) & 0xffu);
}

class NullChunkFile final
{
public:
	explicit NullChunkFile(const char *name)
		: m_file(nullptr, static_cast<int>(sizeof(ChunkHeader) + Assets::W3D::W3DNullPayloadSize))
	{
		NullPayload payload{};
		Store_U32(payload, 0, 0x00010000u);
		Store_U32(payload, 4, 0u);
		const std::size_t length = (std::min)(std::strlen(name), Assets::W3D::W3DNullNameSize - 1);
		std::memcpy(payload.data() + Assets::W3D::W3DNullNameOffset, name, length);

		m_ready = m_file.Open(FileClass::WRITE) != 0;
		if (m_ready) {
			ChunkSaveClass save(&m_file);
			m_ready = save.Begin_Chunk(Assets::W3D::W3DChunkNullObject)
				&& save.Write(payload.data(), static_cast<std::uint32_t>(payload.size())) == payload.size()
				&& save.End_Chunk();
			m_file.Close();
		}
	}

	bool Load(W3DAssetCatalog &catalog)
	{
		return m_ready && catalog.Load_3D_Assets(m_file);
	}

private:
	RAMFileClass m_file;
	bool m_ready = false;
};

void Install_Null_Decoder(W3DAssetCatalog &catalog)
{
	catalog.Register_Model_Decoder(
		static_cast<int>(Assets::W3D::W3DChunkNullObject), Load_Null_Factory);
}

void Install_Reserved_Null(W3DAssetCatalog &catalog)
{
	BOOST_REQUIRE(catalog.Install_Reserved_Model_Factory(
		std::unique_ptr<Graphics::ModelFactory<W3DRenderObject>>(
			Create_Null_Render_Object_Factory())));
}

void Add_Dynamic_Null(W3DAssetCatalog &catalog, const char *name)
{
	const std::string identity(name);
	catalog.Add_Prototype(std::make_unique<Graphics::ModelFactory<W3DRenderObject>>(identity,
		W3DRenderObject::CLASSID_NULL, [identity] {
			return NEW_REF(NullRenderObject, (identity.c_str()));
		}));
}

void Check_Vector(const Vector3 &actual, const Vector3 &expected)
{
	BOOST_CHECK_EQUAL(actual.X, expected.X);
	BOOST_CHECK_EQUAL(actual.Y, expected.Y);
	BOOST_CHECK_EQUAL(actual.Z, expected.Z);
}

}

BOOST_AUTO_TEST_CASE(null_object_preserves_class_identity_and_truncates_direct_names)
{
	const std::string long_name(64, 'N');
	NullRenderObject object(long_name.c_str());

	BOOST_CHECK_EQUAL(object.Class_ID(), W3DRenderObject::CLASSID_NULL);
	BOOST_CHECK_EQUAL(object.Get_Num_Polys(), 0);
	BOOST_CHECK_EQUAL(std::string(object.Get_Name()),
		std::string(NullNameCapacity, 'N'));

	SphereClass sphere(Vector3(4, 5, 6), 7.0f);
	object.Get_Obj_Space_Bounding_Sphere(sphere);
	Check_Vector(sphere.Center, Vector3(0, 0, 0));
	BOOST_CHECK_EQUAL(sphere.Radius, 0.1f);

	AABoxClass box(Vector3(4, 5, 6), Vector3(7, 8, 9));
	object.Get_Obj_Space_Bounding_Box(box);
	Check_Vector(box.Center, Vector3(0, 0, 0));
	Check_Vector(box.Extent, Vector3(0.1f, 0.1f, 0.1f));
}

BOOST_AUTO_TEST_CASE(null_clone_copies_name_but_starts_with_default_render_state)
{
	NullRenderObject source("source");
	Matrix3D transform(true);
	transform.Set_Translation(Vector3(10, 20, 30));
	source.Set_Transform(transform);
	source.Set_Hidden(true);
	source.Set_Animation_Hidden(true);
	source.Set_Force_Visible(true);
	source.Set_Collision_Type(SCENE_QUERY_PROJECTILE);
	source.Set_ObjectScale(3.5f);

	RefCountPtr<W3DRenderObject> clone = Create_No_Add_Ref(source.Clone());
	BOOST_REQUIRE(clone != nullptr);
	BOOST_CHECK_EQUAL(std::string(clone->Get_Name()), "source");
	Check_Vector(clone->Get_Position(), Vector3(0, 0, 0));
	BOOST_CHECK(!clone->Is_Hidden());
	BOOST_CHECK(!clone->Is_Animation_Hidden());
	BOOST_CHECK(!clone->Is_Force_Visible());
	BOOST_CHECK(!clone->Is_Visible());
	BOOST_CHECK_EQUAL(clone->Get_ObjectScale(), 1.0f);
	BOOST_CHECK_EQUAL(clone->Get_Collision_Type(), SCENE_QUERY_ALL);
}

BOOST_AUTO_TEST_CASE(null_assignment_copies_render_flags_without_replacing_transform)
{
	NullRenderObject source("source");
	source.Set_Hidden(true);
	source.Set_Animation_Hidden(true);
	source.Set_Force_Visible(true);
	source.Set_Collision_Type(SCENE_QUERY_PROJECTILE);
	source.Set_Native_Screen_Size(3.5f);

	NullRenderObject target("target");
	target.Set_Position(Vector3(9, 8, 7));
	target = source;

	BOOST_CHECK_EQUAL(std::string(target.Get_Name()), "source");
	Check_Vector(target.Get_Position(), Vector3(9, 8, 7));
	BOOST_CHECK(target.Is_Hidden());
	BOOST_CHECK(target.Is_Animation_Hidden());
	BOOST_CHECK(target.Is_Force_Visible());
	BOOST_CHECK_EQUAL(target.Get_Collision_Type(), SCENE_QUERY_PROJECTILE | SCENE_QUERY_ALL);
	BOOST_CHECK_EQUAL(target.Get_Native_Screen_Size(), 3.5f);
}

BOOST_AUTO_TEST_CASE(null_object_does_not_report_collision_queries)
{
	NullRenderObject object;
	const LineSegClass line(Vector3(-1, 0, 0), Vector3(1, 0, 0));

	CastResultStruct ray_result;
	W3DRayCastQuery ray(line, &ray_result, SCENE_QUERY_ALL);
	BOOST_CHECK(!object.Cast_Ray(ray));

	CastResultStruct aa_cast_result;
	W3DBoxCastQuery moving_aa(
		AABoxClass(Vector3(-1, 0, 0), Vector3(0.25f, 0.25f, 0.25f)),
		Vector3(2, 0, 0), &aa_cast_result, SCENE_QUERY_ALL);
	BOOST_CHECK(!object.Cast_AABox(moving_aa));

	CastResultStruct ob_cast_result;
	W3DOrientedBoxCastQuery moving_ob(
		OBBoxClass(Vector3(-1, 0, 0), Vector3(0.25f, 0.25f, 0.25f)),
		Vector3(2, 0, 0), &ob_cast_result, SCENE_QUERY_ALL);
	BOOST_CHECK(!object.Cast_OBBox(moving_ob));

	W3DBoxIntersectionQuery aa_intersection(
		AABoxClass(Vector3(0, 0, 0), Vector3(1, 1, 1)), SCENE_QUERY_ALL);
	BOOST_CHECK(!object.Intersect_AABox(aa_intersection));

	W3DOrientedBoxIntersectionQuery ob_intersection(
		OBBoxClass(Vector3(0, 0, 0), Vector3(1, 1, 1)), SCENE_QUERY_ALL);
	BOOST_CHECK(!object.Intersect_OBBox(ob_intersection));
}

BOOST_AUTO_TEST_CASE(null_factory_creates_default_null_objects)
{
	const std::unique_ptr<Graphics::ModelFactory<W3DRenderObject>> factory(
		Create_Null_Render_Object_Factory());
	BOOST_REQUIRE(factory != nullptr);
	BOOST_CHECK_EQUAL(factory->name, "NULL");
	BOOST_CHECK_EQUAL(factory->class_id, W3DRenderObject::CLASSID_NULL);

	RefCountPtr<W3DRenderObject> object = Create_No_Add_Ref(factory->Instantiate());
	BOOST_REQUIRE(object != nullptr);
	BOOST_CHECK_EQUAL(object->Class_ID(), W3DRenderObject::CLASSID_NULL);
	BOOST_CHECK_EQUAL(std::string(object->Get_Name()), "NULL");
}

BOOST_AUTO_TEST_CASE(null_reserved_factory_is_case_insensitive_and_instances_are_independent)
{
	W3DAssetCatalog catalog;
	Install_Reserved_Null(catalog);

	Graphics::ModelFactory<W3DRenderObject> *reserved = catalog.Find_Prototype("NULL");
	BOOST_REQUIRE(reserved != nullptr);
	BOOST_CHECK(catalog.Find_Prototype("nUlL") == reserved);
	BOOST_CHECK(catalog.Render_Obj_Exists("null"));

	RefCountPtr<W3DRenderObject> first =
		Create_No_Add_Ref(catalog.Create_Render_Obj("NULL"));
	RefCountPtr<W3DRenderObject> second =
		Create_No_Add_Ref(catalog.Create_Render_Obj("null"));
	BOOST_REQUIRE(first != nullptr);
	BOOST_REQUIRE(second != nullptr);
	BOOST_CHECK(first.Peek() != second.Peek());
	BOOST_CHECK_EQUAL(std::string(first->Get_Name()), "NULL");
	BOOST_CHECK_EQUAL(std::string(second->Get_Name()), "NULL");

	first->Set_Position(Vector3(4, 5, 6));
	Check_Vector(first->Get_Position(), Vector3(4, 5, 6));
	Check_Vector(second->Get_Position(), Vector3(0, 0, 0));

	// NULL inherits W3DRenderObject's deliberately empty Set_Name contract.
	first->Set_Name("renamed");
	BOOST_CHECK_EQUAL(std::string(first->Get_Name()), "NULL");
}

BOOST_AUTO_TEST_CASE(null_reserved_factory_survives_purge_and_named_nulls_load_from_ram)
{
	W3DAssetCatalog catalog;
	Install_Reserved_Null(catalog);
	Install_Null_Decoder(catalog);
	Graphics::ModelFactory<W3DRenderObject> *reserved = catalog.Find_Prototype("NULL");
	BOOST_REQUIRE(reserved != nullptr);

	NullChunkFile authored("Authored.Null");
	BOOST_REQUIRE(authored.Load(catalog));
	Graphics::ModelFactory<W3DRenderObject> *authored_factory =
		catalog.Find_Prototype("authored.null");
	BOOST_REQUIRE(authored_factory != nullptr);
	BOOST_CHECK_EQUAL(authored_factory->class_id, W3DRenderObject::CLASSID_NULL);
	RefCountPtr<W3DRenderObject> authored_object =
		Create_No_Add_Ref(authored_factory->Instantiate());
	BOOST_REQUIRE(authored_object != nullptr);
	BOOST_CHECK_EQUAL(std::string(authored_object->Get_Name()), "Authored.Null");

	NullChunkFile collision("NULL");
	BOOST_REQUIRE(collision.Load(catalog));
	BOOST_CHECK(catalog.Find_Prototype("NULL") == reserved);

	const std::vector<std::string> exclusions{"Authored"};
	catalog.Free_Assets_With_Exclusion_List(exclusions);
	BOOST_CHECK(catalog.Find_Prototype("NULL") == reserved);
	BOOST_CHECK(catalog.Find_Prototype("AUTHORED.NULL") == authored_factory);

	catalog.Free_Assets();
	BOOST_CHECK(catalog.Find_Prototype("null") == reserved);
	BOOST_CHECK(catalog.Find_Prototype("Authored.Null") == nullptr);
}

BOOST_AUTO_TEST_CASE(null_reserved_factory_is_not_iterated_and_authored_collision_is_rejected)
{
	W3DAssetCatalog catalog;
	Install_Reserved_Null(catalog);
	Install_Null_Decoder(catalog);
	Add_Dynamic_Null(catalog, "Dynamic.One");
	Add_Dynamic_Null(catalog, "Dynamic.Two");

	std::vector<std::string> names;
	for (std::size_t index = 0; index < catalog.Prototype_Count(); ++index) {
		const auto *prototype = catalog.Prototype_At(index);
		if (prototype != nullptr)
			names.emplace_back(prototype->name);
	}
	BOOST_CHECK_EQUAL(names.size(), 2u);
	BOOST_CHECK(std::find(names.begin(), names.end(), "NULL") == names.end());
	BOOST_CHECK(std::find(names.begin(), names.end(), "Dynamic.One") != names.end());
	BOOST_CHECK(std::find(names.begin(), names.end(), "Dynamic.Two") != names.end());

	Graphics::ModelFactory<W3DRenderObject> *reserved = catalog.Find_Prototype("NULL");
	BOOST_REQUIRE(reserved != nullptr);
	NullChunkFile collision("NULL");
	BOOST_REQUIRE(collision.Load(catalog));
	BOOST_CHECK(catalog.Find_Prototype("null") == reserved);

	std::vector<std::string> names_after_collision;
	for (std::size_t index = 0; index < catalog.Prototype_Count(); ++index) {
		const auto *prototype = catalog.Prototype_At(index);
		if (prototype != nullptr)
			names_after_collision.emplace_back(prototype->name);
	}
	BOOST_CHECK_EQUAL_COLLECTIONS(names_after_collision.begin(), names_after_collision.end(),
		names.begin(), names.end());
}

BOOST_AUTO_TEST_CASE(null_reserved_factory_rejects_replacement_and_removal_is_pointer_safe)
{
	W3DAssetCatalog catalog;
	Install_Reserved_Null(catalog);
	Graphics::ModelFactory<W3DRenderObject> *reserved = catalog.Find_Prototype("NULL");
	BOOST_REQUIRE(reserved != nullptr);

	auto replacement = std::make_unique<Graphics::ModelFactory<W3DRenderObject>>(
		"Replacement", W3DRenderObject::CLASSID_NULL, [] {
			return NEW_REF(NullRenderObject, ("Replacement"));
		});
	BOOST_CHECK(!catalog.Install_Reserved_Model_Factory(std::move(replacement)));
	BOOST_CHECK(replacement == nullptr);
	BOOST_CHECK(catalog.Find_Prototype("replacement") == nullptr);
	BOOST_CHECK(catalog.Find_Prototype("NULL") == reserved);

	Add_Dynamic_Null(catalog, "Pointer.Null");
	Add_Dynamic_Null(catalog, "Name.Null");
	Graphics::ModelFactory<W3DRenderObject> *pointer_factory =
		catalog.Find_Prototype("pointer.null");
	BOOST_REQUIRE(pointer_factory != nullptr);
	BOOST_CHECK(!catalog.Release_Prototype(nullptr));
	catalog.Remove_Prototype("NULL");
	BOOST_CHECK(catalog.Find_Prototype("NULL") == reserved);

	auto pointer_owner = catalog.Release_Prototype(pointer_factory);
	BOOST_REQUIRE(pointer_owner != nullptr);
	pointer_owner.reset();
	BOOST_CHECK(catalog.Find_Prototype("Pointer.Null") == nullptr);
	catalog.Remove_Prototype("name.null");
	BOOST_CHECK(catalog.Find_Prototype("Name.Null") == nullptr);
	catalog.Remove_Prototype("NULL");
	BOOST_CHECK(catalog.Find_Prototype("NULL") == reserved);
}

BOOST_AUTO_TEST_CASE(null_render_does_not_submit_and_preserves_an_offscreen_clear)
{
	Graphics::GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	const Graphics::RHITextureHandle color_target = device.Create_Texture({
		2, 2, 1, Graphics::RHITextureFormat::RGBA8_UNorm,
		static_cast<std::uint32_t>(Graphics::RHITextureUsage::RenderTarget)});
	const Graphics::RHITextureHandle depth_target = device.Create_Texture({
		2, 2, 1, Graphics::RHITextureFormat::D32_Float,
		static_cast<std::uint32_t>(Graphics::RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(color_target.Is_Valid());
	BOOST_REQUIRE(depth_target.Is_Valid());

	Graphics::CommandList &commands = device.Immediate_Command_List();
	BOOST_REQUIRE(commands.Set_Render_Targets(color_target, depth_target));
	BOOST_REQUIRE(commands.Clear({0.125f, 0.25f, 0.5f, 1.0f}, 1.0f));
	const Graphics::RHISubmissionCounts before = commands.Submission_Counts();

	W3DCamera camera;
	W3DRenderContext render_info(camera);
	NullRenderObject object("offscreen");
	object.Render(render_info);

	const Graphics::RHISubmissionCounts after = commands.Submission_Counts();
	BOOST_CHECK_EQUAL(after.draw_calls, before.draw_calls);
	BOOST_CHECK_EQUAL(after.triangles, before.triangles);
	BOOST_CHECK_EQUAL(after.vertex_invocations, before.vertex_invocations);

	std::array<std::byte, 2 * 2 * 4> pixels{};
	BOOST_REQUIRE(device.Readback_Texture(color_target, pixels, 2 * 4));
	for (std::size_t offset = 0; offset < pixels.size(); offset += 4) {
		BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset]), 32u);
		BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset + 1]), 64u);
		// Half intensity lies between two UNorm8 values on the tested devices.
		const unsigned blue = std::to_integer<unsigned>(pixels[offset + 2]);
		BOOST_CHECK(blue >= 127u && blue <= 128u);
		BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset + 3]), 255u);
	}

	BOOST_REQUIRE(device.Destroy_Texture(color_target));
	BOOST_REQUIRE(device.Destroy_Texture(depth_target));
}

BOOST_AUTO_TEST_CASE(null_render_object_persistence_falls_back_to_reserved_factory)
{
	W3DAssetCatalog catalog;
	Install_Reserved_Null(catalog);
	Graphics::ModelFactory<W3DRenderObject> *reserved = catalog.Find_Prototype("NULL");
	BOOST_REQUIRE(reserved != nullptr);

	NullRenderObject source("Saved.Null");
	source.Set_Position(Vector3(7, 8, 9));
	RAMFileClass file(nullptr, 512);
	BOOST_REQUIRE(file.Open(FileClass::WRITE));
	ChunkSaveClass save(&file);
	BOOST_REQUIRE(save.Begin_Chunk(source.Get_Factory().Chunk_ID()));
	source.Get_Factory().Save(save, &source);
	BOOST_REQUIRE(save.End_Chunk());
	file.Close();

	BOOST_REQUIRE(file.Open(FileClass::READ));
	ChunkLoadClass load(&file);
	BOOST_REQUIRE(load.Open_Chunk());
	PersistFactoryClass *factory =
		SaveLoadSystemClass::Find_Persist_Factory(load.Cur_Chunk_ID());
	BOOST_REQUIRE(factory != nullptr);
	PersistClass *persisted = factory->Load(load);
	BOOST_REQUIRE(load.Close_Chunk());
	file.Close();

	RefCountPtr<W3DRenderObject> loaded =
		Create_No_Add_Ref(static_cast<W3DRenderObject *>(persisted));
	BOOST_REQUIRE(loaded != nullptr);
	BOOST_CHECK_EQUAL(std::string(loaded->Get_Name()), "NULL");
	Check_Vector(loaded->Get_Position(), Vector3(7, 8, 9));
	BOOST_CHECK(catalog.Find_Prototype("NULL") == reserved);
}

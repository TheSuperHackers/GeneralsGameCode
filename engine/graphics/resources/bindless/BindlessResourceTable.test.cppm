module;

#define BOOST_TEST_MODULE GraphicsBindlessResourceTableTests

#include <boost/test/included/unit_test.hpp>

export module Graphics.Resources.Bindless.BindlessResourceTable.Tests;

import Graphics.Resources.Bindless.BindlessResourceTable;

using namespace Graphics;

BOOST_AUTO_TEST_CASE(resource_indices_are_stable_and_updates_keep_their_slot)
{
	BindlessResourceTable table;
	table.Reserve(8, 2, 2, 2, 2);

	const TextureHandle texture(3, 7);
	const RHITextureHandle first_texture(11, 2);
	const RHITextureHandle second_texture(12, 3);
	const ResourceIndex index = table.Register_Texture(texture, first_texture);
	const ResourceIndex repeated_index = table.Register_Texture(texture, first_texture);

	BOOST_REQUIRE(index.Is_Valid());
	BOOST_CHECK(index == repeated_index);
	BOOST_CHECK(table.Texture_Index(texture) == index);
	BOOST_CHECK(table.Resolve(index).type == RHIResourceType::Texture);
	BOOST_CHECK(table.Resolve(index).texture == first_texture);

	BOOST_REQUIRE(table.Update_Texture(texture, second_texture));
	BOOST_CHECK(table.Texture_Index(texture) == index);
	BOOST_CHECK(table.Resolve(index).texture == second_texture);
	BOOST_CHECK(!table.Update_Texture(TextureHandle(3, 6), second_texture));
}

BOOST_AUTO_TEST_CASE(resource_categories_use_typed_source_mappings)
{
	BindlessResourceTable table;
	const RHIBufferHandle buffer(1, 1);
	const SamplerHandle sampler(2, 1);
	const MaterialHandle material(3, 1);

	const ResourceIndex buffer_index = table.Register_Buffer(buffer);
	const ResourceIndex sampler_index = table.Register_Sampler(sampler);
	const ResourceIndex material_index = table.Register_Material(material, RHIBufferHandle(4, 1));

	BOOST_REQUIRE(buffer_index.Is_Valid());
	BOOST_REQUIRE(sampler_index.Is_Valid());
	BOOST_REQUIRE(material_index.Is_Valid());
	BOOST_CHECK(table.Resolve(buffer_index).type == RHIResourceType::Buffer);
	BOOST_CHECK(table.Resolve(buffer_index).buffer == buffer);
	BOOST_CHECK(table.Resolve(sampler_index).type == RHIResourceType::Sampler);
	BOOST_CHECK(table.Resolve(material_index).type == RHIResourceType::Material);
	BOOST_CHECK(table.Resolve(material_index).buffer == RHIBufferHandle(4, 1));
	BOOST_CHECK(table.Buffer_Index(RHIBufferHandle(1, 2)) == ResourceIndex{});
	BOOST_CHECK(table.Material_Index(MaterialHandle(3, 2)) == ResourceIndex{});
}

BOOST_AUTO_TEST_CASE(destroyed_indices_are_invalid_and_reuse_changes_generation)
{
	BindlessResourceTable table;
	table.Reserve(1, 0, 2);

	const TextureHandle old_handle(5, 1);
	const TextureHandle new_handle(5, 2);
	const ResourceIndex old_index = table.Register_Texture(old_handle, RHITextureHandle(8, 1));
	BOOST_REQUIRE(old_index.Is_Valid());
	BOOST_REQUIRE(table.Destroy_Texture(old_handle));

	BOOST_CHECK(!table.Is_Valid(old_index));
	BOOST_CHECK(table.Resolve(old_index).type == RHIResourceType::Invalid);
	BOOST_CHECK(table.Texture_Index(old_handle) == ResourceIndex{});
	BOOST_CHECK(!table.Destroy_Texture(old_handle));
	BOOST_CHECK(!table.Update_Texture(old_handle, RHITextureHandle(9, 1)));
	BOOST_CHECK(table.Register_Texture(old_handle, RHITextureHandle(9, 1)) == ResourceIndex{});

	const ResourceIndex new_index = table.Register_Texture(new_handle, RHITextureHandle(9, 1));
	BOOST_REQUIRE(new_index.Is_Valid());
	BOOST_CHECK(new_index.Get_Index() == old_index.Get_Index());
	BOOST_CHECK(new_index != old_index);
	BOOST_CHECK(!table.Is_Valid(old_index));
	BOOST_CHECK(table.Is_Valid(new_index));
	BOOST_CHECK(table.Texture_Index(old_handle) == ResourceIndex{});
	BOOST_CHECK(table.Resolve(new_index).texture == RHITextureHandle(9, 1));
}

BOOST_AUTO_TEST_CASE(texture_indices_use_the_reserved_texture_range)
{
	BindlessResourceTable table;
	table.Reserve(2, 1, 8, 0, 8);

	const ResourceIndex buffer_index = table.Register_Buffer(RHIBufferHandle(1, 1));
	BOOST_REQUIRE(buffer_index.Is_Valid());

	for (std::uint32_t index = 0; index < 8; ++index) {
		const TextureHandle texture_handle(index, 1);
		const MaterialHandle material_handle(index, 1);
		const ResourceIndex texture_index = table.Register_Texture(texture_handle, RHITextureHandle(index + 2, 1));
		const ResourceIndex material_index = table.Register_Material(material_handle, RHIBufferHandle(index + 2, 1));
		BOOST_REQUIRE(texture_index.Is_Valid());
		BOOST_REQUIRE(material_index.Is_Valid());
		BOOST_CHECK(texture_index.Get_Index() < 8);
		BOOST_CHECK(table.Is_Valid(texture_index));
		BOOST_CHECK(table.Is_Valid(material_index));
	}
}

BOOST_AUTO_TEST_CASE(sparse_owner_handles_use_compact_resource_slots)
{
    BindlessResourceTable table;
    table.Reserve(4, 0, 4);
    const TextureHandle dense(1, 1);
    const TextureHandle sparse(0x40000001u, 1);
    const auto dense_index = table.Register_Texture(dense, RHITextureHandle(1, 1));
    const auto sparse_index = table.Register_Texture(sparse, RHITextureHandle(2, 1));
    BOOST_REQUIRE(dense_index.Is_Valid());
    BOOST_REQUIRE(sparse_index.Is_Valid());
    BOOST_CHECK(dense_index != sparse_index);
    BOOST_CHECK(sparse_index.Get_Index() < 4);
    BOOST_CHECK(table.Texture_Index(sparse) == sparse_index);
    BOOST_REQUIRE(table.Update_Texture(sparse, RHITextureHandle(3, 1)));
    BOOST_CHECK(table.Resolve(sparse_index).texture == RHITextureHandle(3, 1));
    BOOST_REQUIRE(table.Destroy_Texture(sparse));
    BOOST_CHECK(!table.Texture_Index(sparse).Is_Valid());
    BOOST_CHECK(table.Texture_Index(dense) == dense_index);
    BOOST_CHECK(!table.Register_Texture(sparse, RHITextureHandle(4, 1)).Is_Valid());
    BOOST_CHECK(table.Register_Texture(TextureHandle(0x40000001u, 2), RHITextureHandle(4, 1)).Is_Valid());
}

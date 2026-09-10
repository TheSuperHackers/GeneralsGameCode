module;

#define BOOST_TEST_MODULE GeneralsAssetsTextureLoadTaskTests

#include <boost/test/included/unit_test.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

export module Assets.Tests.TextureLoadTask;

import Assets.Cache.TextureLoadTask;
import Assets.Identity;
import Assets.States;

namespace
{

void Write_U32(std::vector<std::byte> &data, std::size_t offset, std::uint32_t value)
{
	data[offset + 0] = static_cast<std::byte>(value);
	data[offset + 1] = static_cast<std::byte>(value >> 8);
	data[offset + 2] = static_cast<std::byte>(value >> 16);
	data[offset + 3] = static_cast<std::byte>(value >> 24);
}

std::vector<std::byte> Make_DXT1_Red()
{
	std::vector<std::byte> data(136);
	data[0] = std::byte{'D'};
	data[1] = std::byte{'D'};
	data[2] = std::byte{'S'};
	data[3] = std::byte{' '};
	Write_U32(data, 4, 124);
	Write_U32(data, 12, 4);
	Write_U32(data, 16, 4);
	Write_U32(data, 76, 32);
	Write_U32(data, 80, 4);
	Write_U32(data, 84, 0x31545844);
	data[128] = static_cast<std::byte>(0x00);
	data[129] = static_cast<std::byte>(0xf8);
	data[130] = data[128];
	data[131] = data[129];
	return data;
}

}

BOOST_AUTO_TEST_CASE(texture_load_task_builds_immutable_runtime_metadata)
{
	const Assets::AssetIdentity identity{Assets::AssetType::Texture, "textures/paint.tga"};
	const Assets::TextureLoadResult result = Assets::Load_Texture_Asset(
		identity,
		[](const Assets::AssetIdentity &) { return std::vector<std::byte>{std::byte{1}, std::byte{2}}; });

	BOOST_REQUIRE(result.Succeeded());
	BOOST_REQUIRE(result.asset != nullptr);
	BOOST_CHECK(result.asset->Identity() == identity);
	BOOST_CHECK(result.asset->Source_Format() == "tga");
	BOOST_CHECK(result.asset->Source_Size() == 2);
}

BOOST_AUTO_TEST_CASE(texture_load_task_decodes_dxt1_pixels)
{
	const Assets::AssetIdentity identity{Assets::AssetType::Texture, "textures/paint.dds"};
	const std::vector<std::byte> source = Make_DXT1_Red();
	const Assets::TextureLoadResult result = Assets::Load_Texture_Asset(identity, [&source](const Assets::AssetIdentity &) {
		return source;
	});

	BOOST_REQUIRE(result.Succeeded());
	BOOST_REQUIRE(result.asset != nullptr);
	BOOST_CHECK(result.asset->Has_Pixels());
	BOOST_CHECK(result.asset->Width() == 4u);
	BOOST_CHECK(result.asset->Height() == 4u);
	BOOST_CHECK(result.asset->Row_Pitch() == 16u);
	BOOST_REQUIRE(result.asset->Pixels().size() == 64u);
	BOOST_CHECK(std::to_integer<std::uint8_t>(result.asset->Pixels()[0]) == 255u);
	BOOST_CHECK(std::to_integer<std::uint8_t>(result.asset->Pixels()[1]) == 0u);
	BOOST_CHECK(std::to_integer<std::uint8_t>(result.asset->Pixels()[2]) == 0u);
	BOOST_CHECK(std::to_integer<std::uint8_t>(result.asset->Pixels()[3]) == 255u);
}

BOOST_AUTO_TEST_CASE(texture_load_task_reads_color_after_compressed_alpha)
{
    for (const std::uint32_t format : {0x33545844u,0x35545844u}) {
        auto source=Make_DXT1_Red();
        source.resize(144);
        Write_U32(source,84,format);
        Write_U32(source,128,format==0x33545844u ? 0x88888888u : 0x00008080u);
        Write_U32(source,132,format==0x33545844u ? 0x88888888u : 0u);
        Write_U32(source,136,0x07e007e0u);
        Write_U32(source,140,0);
        const auto result=Assets::Load_Texture_Asset({Assets::AssetType::Texture,"green.dds"},
            [&](const Assets::AssetIdentity&) { return source; });
        BOOST_REQUIRE(result.Succeeded()); BOOST_REQUIRE(result.asset->Has_Pixels());
        const auto pixels=result.asset->Pixels();
        for (unsigned pixel=0;pixel<16;++pixel) {
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[pixel*4]),0u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[pixel*4+1]),255u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[pixel*4+2]),0u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[pixel*4+3]),format==0x33545844u ? 136u : 128u);
        }
    }
}

BOOST_AUTO_TEST_CASE(texture_load_task_uses_indexed_tga_adapter_and_rejects_partial_pixels)
{
    std::vector<std::byte> source(23);
    source[1]=std::byte{1}; source[2]=std::byte{1}; source[5]=std::byte{1}; source[7]=std::byte{32};
    source[12]=std::byte{1}; source[14]=std::byte{1}; source[16]=std::byte{8}; source[17]=std::byte{32};
    source[18]=std::byte{30}; source[19]=std::byte{20}; source[20]=std::byte{10}; source[21]=std::byte{80};
    const Assets::AssetIdentity identity{Assets::AssetType::Texture,"indexed.tga"};
    const auto load=[&] { return Assets::Load_Texture_Asset(identity,[&](const auto&) { return source; }); };
    const auto decoded=load();
    BOOST_REQUIRE(decoded.Succeeded()); BOOST_REQUIRE(decoded.asset->Has_Pixels());
    const auto pixels=decoded.asset->Pixels();
    BOOST_REQUIRE_EQUAL(pixels.size(),4u);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[0]),10u);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[1]),20u);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[2]),30u);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[3]),80u);
    source.pop_back();
    const auto malformed=load();
    BOOST_REQUIRE(malformed.Succeeded()); // Undecoded sources retain metadata only.
    BOOST_CHECK(!malformed.asset->Has_Pixels());
    BOOST_CHECK_EQUAL(malformed.asset->Width(),0u);
    BOOST_CHECK_EQUAL(malformed.asset->Height(),0u);
}

#define BOOST_TEST_MODULE GraphicsW3DValueConversionTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <bit>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

import Graphics.Tests.Device;
import Graphics.Frame.RenderClock;
import Graphics.RHI;
import Graphics.Scene.Props.Geometry;
import Graphics.Scene.Props.Material;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.MaterialDrawState;
import Graphics.Materials.TextureMapping;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.State;
import Graphics.Materials.W3DMeshMaterial;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Beams.RibbonPipeline;
import Graphics.Scene.Views.CameraMatrices;
import Graphics.Scene.Models.Materials;
import Graphics.Scene.Models.MeshMaterialBindings;
import Assets.Adapters.W3D.Materials;
import Assets.Adapters.W3D.Chunks;
import Assets.Adapters.W3D.TextureMapping;
#include "W3DDevice/GameClient/W3DEmitterRenderObject.h"
#include "W3DDevice/GameClient/W3DEmitterParticles.h"

#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DSceneClass.h"
#include "WWLib/ref_ptr.h"
#include "WWMath/v3_rnd.h"
#include "WWMath/quat.h"
#include "WWLib/RANDOM.h"
#include "W3DDevice/GameClient/W3DSegmentedLineRenderObject.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include "W3DDevice/GameClient/W3DSceneQueryMask.h"

#include "W3DDevice/GameClient/W3DLight.h"
#include "WWLib/RAMFILE.h"
#include "WWLib/chunkio.h"

#ifndef GRAPHICS_W3D_VALUE_CONVERSION_SHADER_DIRECTORY
#define GRAPHICS_W3D_VALUE_CONVERSION_SHADER_DIRECTORY "."
#endif

BOOST_AUTO_TEST_CASE(cloned_model_resources_remap_single_and_vertex_materials_without_mutating_source)
{
    using MaterialBindings = Graphics::MeshMaterialBindings<RefCountPtr<W3DTextureHandle>, Vector2>;
    MaterialBindings destination;
    std::weak_ptr<Graphics::MeshMaterial> original;
    std::weak_ptr<Graphics::MeshMaterial> cloned;
    {
        Graphics::ModelMaterials<RefCountPtr<W3DTextureHandle>> resources;
        resources.materials = {std::make_shared<Graphics::MeshMaterial>(),
            std::make_shared<Graphics::MeshMaterial>()};
        resources.materials[0]->parameters.opacity = .625f;
        resources.materials[1]->parameters.opacity = .875f;
        original = resources.materials[0];
        MaterialBindings source;
        source.Reset(2,4,2);
        source.Set_Single_Material(resources.materials[0],0);
        source.Set_Material(0,resources.materials[0],1);
        source.Set_Material(1,resources.materials[1],1);
        source.Set_Material(2,nullptr,1);
        source.Set_Material(3,resources.materials[0],1);
        destination = source;
        auto clone = resources.Clone(1000);
        cloned = clone.materials[0];
        destination.Remap_Resources(source,resources,clone);
        BOOST_CHECK(!destination.Has_Material_Array(0));
        BOOST_CHECK(destination.Has_Material_Array(1));
        BOOST_CHECK(destination.Peek_Single_Material(0) == clone.materials[0].get());
        BOOST_CHECK(destination.Peek_Material(0,1) == clone.materials[0].get());
        BOOST_CHECK(destination.Peek_Material(1,1) == clone.materials[1].get());
        BOOST_CHECK(!destination.Peek_Material(2,1));
        BOOST_CHECK(destination.Peek_Material(3,1) == clone.materials[0].get());
        clone.materials[0]->parameters.opacity = .125f;
        BOOST_CHECK_EQUAL(source.Peek_Single_Material(0)->parameters.opacity,.625f);
        BOOST_CHECK_EQUAL(source.Peek_Material(0,1)->parameters.opacity,.625f);
        clone.Reset();
        resources.Reset();
    }
    BOOST_CHECK(original.expired());
    BOOST_CHECK(!cloned.expired());
    BOOST_CHECK_EQUAL(destination.Peek_Single_Material(0)->parameters.opacity,.125f);
    BOOST_CHECK_EQUAL(destination.Peek_Material(3,1)->parameters.opacity,.125f);
    BOOST_CHECK_EQUAL(destination.Peek_Material(1,1)->parameters.opacity,.875f);
    destination.Reset(0,0,0);
    BOOST_CHECK(cloned.expired());
}

BOOST_AUTO_TEST_CASE(material_mapping_clones_restart_independently_and_slots_retain_ownership)
{
    constexpr unsigned now=100;
    std::weak_ptr<Graphics::TextureMapping> weak;
    std::shared_ptr<Graphics::TextureMapping> retained;
    {
        Graphics::MeshMaterial original;
        Assets::W3D::W3DVertexMaterialData description;
        description.mappings[0]=Assets::W3D::W3DRead_Texture_Mapping(
            0x00040000,0,"UOffset=.25\nVOffset=.5\nUPerSec=.5");
        Graphics::Apply_W3D_Mesh_Material(original,description,now);
        auto* mapping=original.mappings[0].get();
        BOOST_REQUIRE(mapping);
        auto* scroll=mapping->Linear_Scroll();
        BOOST_REQUIRE(scroll);
        scroll->offset={.75f,.875f};
        scroll->rate_per_millisecond={};
        const auto copied=original.Clone(now);
        BOOST_REQUIRE(copied->mappings[0]);
        BOOST_CHECK(copied->mappings[0].get()!=mapping);
        BOOST_CHECK_SMALL(copied->mappings[0]->Evaluate(now).transform[3]-.25f,.00001f);
        BOOST_CHECK_SMALL(copied->mappings[0]->Evaluate(now).transform[7]-.5f,.00001f);
        original.Reset_Mappings(now);
        BOOST_CHECK_SMALL(mapping->Evaluate(now).transform[3],.00001f);
        BOOST_CHECK_SMALL(copied->mappings[0]->Evaluate(now+1000).transform[3]-.25f,.00001f);
        retained=original.mappings[0]; weak=retained;
        Graphics::Apply_W3D_Mesh_Material(original,{},now);
        BOOST_CHECK(original.mappings[0]==retained);
        original.mappings[0].reset();
        BOOST_CHECK(!weak.expired());
    }
    BOOST_CHECK(!weak.expired());
    retained.reset();
    BOOST_CHECK(weak.expired());
}

BOOST_AUTO_TEST_CASE(vertex_material_wire_record_installs_both_mapping_stages_and_authored_values)
{
    std::vector<std::byte> bytes;
    const auto append_chunk=[&](std::uint32_t id,std::span<const std::byte> payload) {
        const auto size=static_cast<std::uint32_t>(payload.size());
        for (unsigned shift=0;shift<32;shift+=8) bytes.push_back(std::byte((id>>shift)&255));
        for (unsigned shift=0;shift<32;shift+=8) bytes.push_back(std::byte((size>>shift)&255));
        bytes.insert(bytes.end(),payload.begin(),payload.end());
    };
    std::array<std::byte,32> info{};
    info[0]=std::byte{5}; info[1]=std::byte{4}; info[2]=std::byte{4};
    info[8]=std::byte{128}; info[9]=std::byte{64}; info[10]=std::byte{255};
    info[22]=std::byte{0x88}; info[23]=std::byte{0x41};
    info[26]=std::byte{0x20}; info[27]=std::byte{0x3f};
    append_chunk(0x2d,info);
    const std::string_view arguments0="UOffset=.25\nVOffset=.5";
    const std::string_view arguments1="UOffset=.75";
    append_chunk(0x2e,std::as_bytes(std::span(arguments0.data(),arguments0.size())));
    append_chunk(0x2f,std::as_bytes(std::span(arguments1.data(),arguments1.size())));
    Assets::W3D::W3DVertexMaterialData decoded;
    BOOST_REQUIRE(Assets::W3D::W3DRead_Vertex_Material(bytes,decoded));
    Graphics::MeshMaterial material;
    material.name="retained";
    material.flags=2;
    Graphics::Apply_W3D_Mesh_Material(material,decoded,100);
    BOOST_CHECK_EQUAL(material.name,"retained");
    BOOST_CHECK_EQUAL(material.flags,7u);
    BOOST_CHECK_SMALL(material.parameters.diffuse[0]-.5019608f,.000001f);
    BOOST_CHECK_SMALL(material.parameters.diffuse[1]-.2509804f,.000001f);
    BOOST_CHECK_SMALL(material.parameters.diffuse[2]-1.0f,.000001f);
    BOOST_CHECK_SMALL(material.parameters.opacity-.625f,.000001f);
    BOOST_CHECK_SMALL(material.parameters.shininess-17.0f,.000001f);
    BOOST_REQUIRE(material.mappings[0]); BOOST_REQUIRE(material.mappings[1]);
    BOOST_CHECK_SMALL(material.mappings[0]->Evaluate(100).transform[3]-.25f,.000001f);
    BOOST_CHECK_SMALL(material.mappings[0]->Evaluate(100).transform[7]-.5f,.000001f);
    BOOST_CHECK_SMALL(material.mappings[1]->Evaluate(100).transform[3]-.75f,.000001f);
}

BOOST_AUTO_TEST_CASE(material_identity_and_mesh_owners_preserve_contents_and_lifetime)
{
    auto source=std::make_shared<Graphics::MeshMaterial>();
    auto equivalent=std::make_shared<Graphics::MeshMaterial>();
    source->name="first"; equivalent->name="second";
    BOOST_CHECK(source->Content_Key()==equivalent->Content_Key());
    equivalent->parameters.emissive[0]=-0.0f;
    BOOST_CHECK(!(source->Content_Key()==equivalent->Content_Key()));
    equivalent->parameters.emissive[0]=0.0f;
    equivalent->Make_Unique();
    BOOST_CHECK(!(source->Content_Key()==equivalent->Content_Key()));
    std::weak_ptr<Graphics::MeshMaterial> weak=source;
    using MaterialBindings = Graphics::MeshMaterialBindings<RefCountPtr<W3DTextureHandle>, Vector2>;
    MaterialBindings mesh;
    mesh.Reset(2,3,1);
    mesh.Set_Single_Material(source);
    mesh.Set_Material(1,equivalent);
    NativeMaterialPass pass;
    pass.material = source;
    source.reset();
    BOOST_CHECK(!weak.expired());
    BOOST_CHECK(mesh.Peek_Single_Material()==pass.material.get());
    BOOST_CHECK(mesh.Get_Material(1)==equivalent);
    // An allocated empty slot stays empty; only an out-of-range peek falls
    // back to the single material.
    BOOST_CHECK(mesh.Peek_Material(0)==nullptr);
    BOOST_CHECK(mesh.Get_Material(0)==nullptr);
    BOOST_CHECK(mesh.Peek_Material(3)==pass.material.get());
    BOOST_CHECK(mesh.Get_Material(3)==nullptr);
    mesh.Set_Single_Material(nullptr);
    BOOST_CHECK(!weak.expired());
    pass.material.reset();
    BOOST_CHECK(weak.expired());
}

BOOST_AUTO_TEST_CASE(render_info_accepts_thirty_one_passes_and_balances_rejected_pushes)
{
    W3DCamera camera;
    W3DRenderContext info(camera);
    std::array<std::shared_ptr<NativeMaterialPass>, 31> accepted;
    for (unsigned index = 0; index < accepted.size(); ++index) {
        accepted[index] = std::make_shared<NativeMaterialPass>();
        info.Push_Material_Pass(accepted[index]);
        BOOST_CHECK_EQUAL(info.Additional_Pass_Count(), static_cast<int>(index + 1));
        BOOST_CHECK(info.Peek_Additional_Pass(static_cast<int>(index)) == accepted[index].get());
    }
    BOOST_CHECK_EQUAL(info.Additional_Pass_Count(), 31);

    std::weak_ptr<NativeMaterialPass> rejected_owner;
    {
        auto rejected = std::make_shared<NativeMaterialPass>();
        rejected_owner = rejected;
        info.Push_Material_Pass(rejected);
        BOOST_CHECK_EQUAL(info.Additional_Pass_Count(), 31);
        BOOST_CHECK(info.Peek_Additional_Pass(30) == accepted[30].get());
        info.Pop_Material_Pass();
        BOOST_CHECK_EQUAL(info.Additional_Pass_Count(), 31);
    }
    BOOST_CHECK(rejected_owner.expired());

    info.Pop_Material_Pass();
    BOOST_CHECK_EQUAL(info.Additional_Pass_Count(), 30);
    BOOST_CHECK(info.Peek_Additional_Pass(29) == accepted[29].get());
}

namespace
{

constexpr float kFloatTolerance = 0.0001f;

struct SerializedW3D final
{
	std::array<std::byte, 8192> bytes{};
	std::size_t size = 0;
};

// These are the small wire records used by W3DLight::Load_W3D and
// W3DLight::Save_W3D. Keep the fixtures local to this test so it does not
// depend on the retired WW3D2 file-format header. The packed layouts match
// the on-disk records byte for byte.
constexpr std::uint32_t kW3DChunkLight = 0x00000460;
constexpr std::uint32_t kW3DChunkLightInfo = 0x00000461;
constexpr std::uint32_t kW3DChunkSpotLightInfo = 0x00000462;
constexpr std::uint32_t kW3DChunkNearAttenuation = 0x00000463;
constexpr std::uint32_t kW3DChunkFarAttenuation = 0x00000464;
constexpr std::uint32_t kW3DLightAttributePoint = 0x00000001;
constexpr std::uint32_t kW3DLightAttributeSpot = 0x00000003;
constexpr std::uint32_t kW3DLightAttributeCastShadows = 0x00000100;

#pragma pack(push, 1)
struct W3DLightRGBFixture final
{
	std::uint8_t R = 0;
	std::uint8_t G = 0;
	std::uint8_t B = 0;
	std::uint8_t pad = 0;
};

struct W3DLightInfoFixture final
{
	std::uint32_t Attributes = 0;
	std::uint32_t Unused = 0;
	W3DLightRGBFixture Ambient;
	W3DLightRGBFixture Diffuse;
	W3DLightRGBFixture Specular;
	float Intensity = 0.0f;
};

struct W3DVectorFixture final
{
	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;
};

struct W3DSpotLightFixture final
{
	W3DVectorFixture SpotDirection;
	float SpotAngle = 0.0f;
	float SpotExponent = 0.0f;
};

struct W3DLightAttenuationFixture final
{
	float Start = 0.0f;
	float End = 0.0f;
};
#pragma pack(pop)

static_assert(sizeof(W3DLightRGBFixture) == 4);
static_assert(sizeof(W3DLightInfoFixture) == 24);
static_assert(sizeof(W3DVectorFixture) == 12);
static_assert(sizeof(W3DSpotLightFixture) == 20);
static_assert(sizeof(W3DLightAttenuationFixture) == 8);

float Byte_To_Float(std::uint8_t value) noexcept
{
	return static_cast<float>(value) / 255.0f;
}

float Byte_Product(std::uint8_t color, std::uint8_t coefficient) noexcept
{
	return Byte_To_Float(color) * Byte_To_Float(coefficient);
}

void Check_Vector(const Vector3 &actual, const Vector3 &expected)
{
	BOOST_CHECK_SMALL(actual.X - expected.X, kFloatTolerance);
	BOOST_CHECK_SMALL(actual.Y - expected.Y, kFloatTolerance);
	BOOST_CHECK_SMALL(actual.Z - expected.Z, kFloatTolerance);
}

RefCountPtr<W3DEmitterRenderObject> Make_Name_Test_Emitter()
{
    Assets::EmitterAssetDesc description;
    description.emission_rate = 1;
    description.burst_size = 1;
    description.lifetime = 1;
    description.color.start = {1, 1, 1, 1};
    description.opacity.start = 1;
    description.size.start = 1;
    auto position = std::make_unique<Vector3SolidBoxRandomizer>(Vector3(0, 0, 0));
    auto velocity = std::make_unique<Vector3SolidBoxRandomizer>(Vector3(0, 0, 0));
    return RefCountPtr<W3DEmitterRenderObject>::Create_No_Add_Ref(new W3DEmitterRenderObject(
        description, nullptr, Graphics::MaterialState::AdditiveSprite(), std::move(position), std::move(velocity)));
}

std::array<std::byte,44> Test_Material3()
{
    // Independent W3D Material3 wire fixture: four-byte color records and
    // little-endian scalar fields, including bytes not used by rendering.
    std::array<std::byte,44> bytes{};
    const auto color=[&](unsigned offset,unsigned r,unsigned g,unsigned b) {
        bytes[offset]=std::byte(r); bytes[offset+1]=std::byte(g); bytes[offset+2]=std::byte(b);
    };
    color(4,128,127,254);
    color(8,255,254,0);
    color(12,1,0,127);
    color(16,128,1,0);
    color(20,128,127,0);
    color(24,1,0,255);
    bytes[30]=std::byte{0x88}; bytes[31]=std::byte{0x41}; // 17.0f
    bytes[34]=std::byte{0x20}; bytes[35]=std::byte{0x3f}; // .625f
    return bytes;
}

bool Load_Test_Material(Graphics::MeshMaterial& material)
{
    Assets::W3D::W3DMaterial3Data decoded;
    if (!Assets::W3D::W3DRead_Material3(Test_Material3(),decoded))
        return false;
    // This helper represents the original raw Material3 value conversion.
    // Map and shader policy is covered by the dedicated converter fixture.
    Graphics::Apply_Mesh_Material_Values(material, decoded.material);
    return true;
}

void Append_U32(std::vector<std::byte>& bytes, std::uint32_t value)
{
    bytes.push_back(std::byte{static_cast<unsigned char>(value & 0xff)});
    bytes.push_back(std::byte{static_cast<unsigned char>((value >> 8) & 0xff)});
    bytes.push_back(std::byte{static_cast<unsigned char>((value >> 16) & 0xff)});
    bytes.push_back(std::byte{static_cast<unsigned char>((value >> 24) & 0xff)});
}

void Write_U32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value)
{
    bytes[offset + 0] = std::byte{static_cast<unsigned char>(value & 0xff)};
    bytes[offset + 1] = std::byte{static_cast<unsigned char>((value >> 8) & 0xff)};
    bytes[offset + 2] = std::byte{static_cast<unsigned char>((value >> 16) & 0xff)};
    bytes[offset + 3] = std::byte{static_cast<unsigned char>((value >> 24) & 0xff)};
}

void Append_Chunk(std::vector<std::byte>& bytes, std::uint32_t id,
    std::span<const std::byte> payload, bool contains_children)
{
    Append_U32(bytes,id);
    Append_U32(bytes,static_cast<std::uint32_t>(payload.size())
        | (contains_children ? Assets::W3D::W3DChunkContainsChildren : 0u));
    bytes.insert(bytes.end(),payload.begin(),payload.end());
}

std::vector<std::byte> Make_Converter_Material3_Container()
{
    std::vector<std::byte> info(44,std::byte{0});
    Write_U32(info,0,Assets::W3D::W3DMaterialUseAlpha);
    info[4]=info[5]=info[6]=std::byte{255};
    info[20]=std::byte{64}; info[21]=std::byte{128}; info[22]=std::byte{192};
    Write_U32(info,28,std::bit_cast<std::uint32_t>(4.0f));
    Write_U32(info,32,std::bit_cast<std::uint32_t>(1.0f));
    Write_U32(info,36,std::bit_cast<std::uint32_t>(0.0f));
    Write_U32(info,40,std::bit_cast<std::uint32_t>(0.0f));

    const std::array<std::byte,15> name{
        std::byte{'c'},std::byte{'o'},std::byte{'n'},std::byte{'v'},std::byte{'e'},
        std::byte{'r'},std::byte{'t'},std::byte{'e'},std::byte{'r'},std::byte{'_'},
        std::byte{'g'},std::byte{'p'},std::byte{'u'},std::byte{'\0'},std::byte{0}};
    const std::array<std::byte,8> map_info{
        std::byte{0},std::byte{0},std::byte{1},std::byte{0},
        std::byte{0},std::byte{0},std::byte{0},std::byte{0}};
    const auto make_name=[](std::string_view value) {
        std::vector<std::byte> bytes;
        bytes.reserve(value.size()+1);
        for (const char character:value)
            bytes.push_back(std::byte{static_cast<unsigned char>(character)});
        bytes.push_back(std::byte{0});
        return bytes;
    };
    const auto make_map=[&](std::string_view filename) {
        std::vector<std::byte> map;
        const auto map_name=make_name(filename);
        Append_Chunk(map,Assets::W3D::W3DChunkMap3Filename,map_name,false);
        Append_Chunk(map,Assets::W3D::W3DChunkMap3Info,std::span<const std::byte>(map_info),false);
        return map;
    };

    std::vector<std::byte> record;
    Append_Chunk(record,Assets::W3D::W3DChunkMaterial3Name,std::span<const std::byte>(name).first(14),false);
    Append_Chunk(record,Assets::W3D::W3DChunkMaterial3Info,std::span<const std::byte>(info),false);
    const auto first_map=make_map("first.tga");
    const auto second_map=make_map("second.tga");
    Append_Chunk(record,Assets::W3D::W3DChunkMaterial3DiffuseColorMap,
        std::span<const std::byte>(first_map),true);
    Append_Chunk(record,Assets::W3D::W3DChunkMaterial3DiffuseColorMap,
        std::span<const std::byte>(second_map),true);

    std::vector<std::byte> container;
    Append_Chunk(container,Assets::W3D::W3DChunkMaterial3,std::span<const std::byte>(record),true);
    return container;
}

void Configure_Test_Light(W3DLight &light)
{
	light.Set_Intensity(1.0f);
	light.Set_Ambient(Vector3(0.5f, 0.25f, 0.75f));
	light.Set_Diffuse(Vector3(0.25f, 0.5f, 0.75f));
	light.Set_Specular(Vector3(0.75f, 0.5f, 0.25f));
	light.Enable_Shadows(true);
	light.Set_Spot_Direction(Vector3(0.0f, 0.0f, -1.0f));
	light.Set_Spot_Angle(0.75f);
	light.Set_Spot_Exponent(2.5f);
	light.Set_Near_Attenuation_Range(2.0, 5.0);
	light.Set_Far_Attenuation_Range(10.0, 20.0);
	light.Set_Flag(W3DLight::FAR_ATTENUATION, true);
}

bool Serialize_Light(W3DLight &light, SerializedW3D &serialized)
{
	RAMFileClass file(serialized.bytes.data(), static_cast<int>(serialized.bytes.size()));
	if (!file.Open(FileClass::WRITE))
		return false;

	ChunkSaveClass save(&file);
	const bool saved = light.Save_W3D(save);
	file.Close();
	serialized.size = static_cast<std::size_t>(file.Size());
	return saved && serialized.size != 0;
}

bool Write_Light_Chunk(ChunkSaveClass &save, std::uint32_t id, const void *data, std::size_t size)
{
	return save.Begin_Chunk(id)
		&& save.Write(data, static_cast<uint32>(size)) == size
		&& save.End_Chunk();
}

bool Write_Light_Load_Fixture(
	const W3DLightInfoFixture &light_info,
	const W3DSpotLightFixture &spot_info,
	const W3DLightAttenuationFixture &near_attenuation,
	const W3DLightAttenuationFixture &far_attenuation,
	SerializedW3D &serialized)
{
	RAMFileClass file(serialized.bytes.data(), static_cast<int>(serialized.bytes.size()));
	if (!file.Open(FileClass::WRITE))
		return false;

	ChunkSaveClass save(&file);
	bool written = save.Begin_Chunk(kW3DChunkLight);
	if (written)
		written = Write_Light_Chunk(save, kW3DChunkLightInfo, &light_info, sizeof(light_info));
	if (written)
		written = Write_Light_Chunk(save, kW3DChunkSpotLightInfo, &spot_info, sizeof(spot_info));
	if (written)
		written = Write_Light_Chunk(save, kW3DChunkNearAttenuation, &near_attenuation, sizeof(near_attenuation));
	if (written)
		written = Write_Light_Chunk(save, kW3DChunkFarAttenuation, &far_attenuation, sizeof(far_attenuation));
	if (written)
		written = save.End_Chunk();

	file.Close();
	serialized.size = static_cast<std::size_t>(file.Size());
	return written && serialized.size != 0;
}

bool Read_Light_Info(SerializedW3D &serialized, W3DLightInfoFixture &light_info)
{
	RAMFileClass file(serialized.bytes.data(), static_cast<int>(serialized.size));
	if (!file.Open(FileClass::READ))
		return false;

	ChunkLoadClass load(&file);
	bool read = load.Open_Chunk() && load.Cur_Chunk_ID() == kW3DChunkLight;
	if (read)
		read = load.Open_Chunk() && load.Cur_Chunk_ID() == kW3DChunkLightInfo;
	if (read)
		read = load.Cur_Chunk_Length() == sizeof(light_info)
			&& load.Read(&light_info, sizeof(light_info)) == sizeof(light_info);
	if (load.Cur_Chunk_Depth() > 1)
		load.Close_Chunk();
	if (load.Cur_Chunk_Depth() > 0)
		load.Close_Chunk();
	file.Close();
	return read;
}

bool Load_Light(SerializedW3D &serialized, W3DLight &light)
{
	RAMFileClass file(serialized.bytes.data(), static_cast<int>(serialized.size));
	if (!file.Open(FileClass::READ))
		return false;

	ChunkLoadClass load(&file);
	bool loaded = load.Open_Chunk() && load.Cur_Chunk_ID() == kW3DChunkLight;
	if (loaded)
		loaded = light.Load_W3D(load);
	if (loaded)
		loaded = load.Close_Chunk();
	file.Close();
	return loaded;
}

void Check_Light_Color(const W3DLight &light, const Vector3 &expected_ambient,
	const Vector3 &expected_diffuse, const Vector3 &expected_specular)
{
	Vector3 ambient;
	Vector3 diffuse;
	Vector3 specular;
	light.Get_Ambient(&ambient);
	light.Get_Diffuse(&diffuse);
	light.Get_Specular(&specular);
	Check_Vector(ambient, expected_ambient);
	Check_Vector(diffuse, expected_diffuse);
	Check_Vector(specular, expected_specular);
}

class RecordingScene final : public W3DSimpleScene
{
public:
	void Register(W3DRenderObject *object, RegType type) override
	{
		if (type == LIGHT)
			++light_register_count;
        if (type == RELEASE)
            ++release_register_count;
		W3DSimpleScene::Register(object, type);
	}

	void Unregister(W3DRenderObject *object, RegType type) override
	{
		if (type == LIGHT)
			++light_unregister_count;
		W3DSimpleScene::Unregister(object, type);
	}

	unsigned light_register_count = 0;
	unsigned light_unregister_count = 0;
    unsigned release_register_count = 0;
};

}

BOOST_AUTO_TEST_CASE(w3d_camera_assignment_preserves_destination_transform_aspect_and_depth)
{
	W3DCamera source;
	source.Set_Projection_Type(W3DCamera::ORTHO);
	source.Set_Clip_Planes(2.0f, 50.0f);
	source.Set_View_Plane({-4.0f, -2.0f}, {4.0f, 2.0f});
	source.Set_Viewport({0.1f, 0.2f}, {0.8f, 0.9f});
	source.Set_Zbuffer_Range(0.15f, 0.85f);
	Matrix3D source_transform = Matrix3D::RotateZ90;
	source_transform.Set_Translation(Vector3(10.0f, 20.0f, 30.0f));
	source.Set_Transform(source_transform);

	W3DCamera destination;
	destination.Set_Projection_Type(W3DCamera::PERSPECTIVE);
	destination.Set_Clip_Planes(1.0f, 10.0f);
	destination.Set_View_Plane({-2.0f, -1.0f}, {2.0f, 1.0f});
	destination.Set_Aspect_Ratio(1.75f);
	destination.Set_Viewport({0.0f, 0.0f}, {1.0f, 1.0f});
	destination.Set_Zbuffer_Range(0.35f, 0.65f);
	Matrix3D destination_transform = Matrix3D::RotateZ90;
	destination_transform.Set_Translation(Vector3(-6.0f, -7.0f, -8.0f));
	destination.Set_Transform(destination_transform);

	destination = source;
	BOOST_CHECK(destination.Get_Transform() == destination_transform);
	BOOST_CHECK(destination.Get_Projection_Type() == W3DCamera::ORTHO);
	float near_clip = 0.0f;
	float far_clip = 0.0f;
	destination.Get_Clip_Planes(near_clip, far_clip);
	BOOST_CHECK_CLOSE_FRACTION(near_clip, 2.0f, kFloatTolerance);
	BOOST_CHECK_CLOSE_FRACTION(far_clip, 50.0f, kFloatTolerance);
	BOOST_CHECK_CLOSE_FRACTION(destination.Get_Aspect_Ratio(), 1.75f, kFloatTolerance);
	float near_depth = 0.0f;
	float far_depth = 0.0f;
	destination.Get_Zbuffer_Range(near_depth, far_depth);
	BOOST_CHECK_CLOSE_FRACTION(near_depth, 0.35f, kFloatTolerance);
	BOOST_CHECK_CLOSE_FRACTION(far_depth, 0.65f, kFloatTolerance);
	Vector2 minimum;
	Vector2 maximum;
	destination.Get_View_Plane(minimum, maximum);
	BOOST_CHECK_CLOSE_FRACTION(minimum.X, -4.0f, kFloatTolerance);
	BOOST_CHECK_CLOSE_FRACTION(minimum.Y, -2.0f, kFloatTolerance);
	BOOST_CHECK_CLOSE_FRACTION(maximum.X, 4.0f, kFloatTolerance);
	BOOST_CHECK_CLOSE_FRACTION(maximum.Y, 2.0f, kFloatTolerance);
	BOOST_CHECK(destination.Get_Viewport().Min == source.Get_Viewport().Min);
	BOOST_CHECK(destination.Get_Viewport().Max == source.Get_Viewport().Max);

	Matrix4x4 source_projection;
	Matrix4x4 destination_projection;
	source.Get_Projection_Matrix(&source_projection);
	destination.Get_Projection_Matrix(&destination_projection);
	for (int row = 0; row < 4; ++row)
		for (int column = 0; column < 4; ++column)
			BOOST_CHECK_CLOSE_FRACTION(destination_projection[row][column],
				source_projection[row][column], kFloatTolerance);

	Vector3 view_space;
	destination.Transform_To_View_Space(view_space, destination_transform.Get_Translation());
	Check_Vector(view_space, Vector3(0.0f, 0.0f, 0.0f));
	BOOST_CHECK(destination.Get_Frustum().CameraTransform == destination_transform);
	const Vector3 visible_center = destination_transform.Get_Translation()
		+ destination.Get_Forward_Dir() * 10.0f;
	BOOST_CHECK(!destination.Cull_Sphere(SphereClass(visible_center, 0.1f)));
}

BOOST_AUTO_TEST_CASE(w3d_render_object_copy_preserves_state_but_resets_object_scale)
{
	W3DCamera source;
	Matrix3D source_transform = Matrix3D::RotateZ90;
	source_transform.Set_Translation(Vector3(10.0f, 20.0f, 30.0f));
	source.Set_Transform(source_transform);
	source.Set_Visible(1);
	source.Set_Hidden(1);
	source.Set_Animation_Hidden(1);
	source.Set_Force_Visible(1);
	source.Set_Translucent(1);
	source.Set_Alpha(1);
	source.Set_Additive(1);
	source.Set_Collision_Type(SCENE_QUERY_PROJECTILE);
	source.Set_Native_Screen_Size(3.5f);
	source.Set_ObjectScale(3.5f);

	W3DCamera copy(source);
	BOOST_CHECK(copy.Get_Transform() == source_transform);
	BOOST_CHECK(copy.Is_Visible());
	BOOST_CHECK(copy.Is_Hidden());
	BOOST_CHECK(copy.Is_Animation_Hidden());
	BOOST_CHECK(copy.Is_Force_Visible());
	BOOST_CHECK(copy.Is_Translucent());
	BOOST_CHECK(copy.Is_Alpha());
	BOOST_CHECK(copy.Is_Additive());
	BOOST_CHECK_EQUAL(copy.Get_Collision_Type(), source.Get_Collision_Type());
	BOOST_CHECK_EQUAL(copy.Get_Native_Screen_Size(), 3.5f);
	BOOST_CHECK_EQUAL(copy.Get_ObjectScale(), 1.0f);
}

BOOST_AUTO_TEST_CASE(w3d_queries_preserve_bounds_touching_and_transformed_ownership)
{
	CastResultStruct result;
	W3DLight hit_object(W3DLight::POINT);
	W3DRenderObject *const hit = &hit_object;

	const LineSegClass source_ray(Vector3(1, 2, 3), Vector3(5, -1, 7));
	W3DRayCastQuery ray(source_ray, &result, SCENE_QUERY_PROJECTILE, true, true);
	ray.CollidedRenderObj = hit;
	const Matrix3D rotate_translate(
		0, -1, 0, 10,
		1, 0, 0, -20,
		0, 0, 1, 30);
	W3DRayCastQuery transformed_ray(ray, rotate_translate);
	BOOST_CHECK(transformed_ray.Result == &result);
	BOOST_CHECK(transformed_ray.CollidedRenderObj == hit);
	BOOST_CHECK_EQUAL(transformed_ray.CollisionType, SCENE_QUERY_PROJECTILE);
	BOOST_CHECK(transformed_ray.CheckTranslucent);
	BOOST_CHECK(transformed_ray.CheckHidden);
	Check_Vector(transformed_ray.Ray.Get_P0(), Vector3(8, -19, 33));
	Check_Vector(transformed_ray.Ray.Get_P1(), Vector3(11, -15, 37));

	const AABoxClass moving_box(Vector3(1, -2, 3), Vector3(2, 3, 4));
	const Vector3 movement(4, 5, 6);
	W3DBoxCastQuery axis_box(moving_box, movement, &result, SCENE_QUERY_PHYSICAL);
	axis_box.CollidedRenderObj = hit;
	Check_Vector(axis_box.SweepMin, Vector3(-1, -5, -1));
	Check_Vector(axis_box.SweepMax, Vector3(7, 6, 13));
	BOOST_CHECK(!axis_box.Cull(AABoxClass(Vector3(8, 0, 0), Vector3(1, 1, 1))));
	BOOST_CHECK(axis_box.Cull(AABoxClass(Vector3(8.01f, 0, 0), Vector3(1, 1, 1))));
	W3DBoxCastQuery copied_axis_box(axis_box);
	BOOST_CHECK(copied_axis_box.Result == &result);
	BOOST_CHECK(copied_axis_box.CollidedRenderObj == hit);

	const OBBoxClass oriented_box(Vector3(1, -2, 3), Vector3(2, 3, 4), Matrix3x3::RotateZ90);
	W3DOrientedBoxCastQuery oriented(oriented_box, movement, &result, SCENE_QUERY_VEHICLE);
	oriented.CollidedRenderObj = hit;
	// The source basis rotates unequal extents into (3, 2, 4), and the
	// conservative cast padding is part of the retained W3D query contract.
	Check_Vector(oriented.SweepMin, Vector3(-2.01f, -4.01f, -1.01f));
	Check_Vector(oriented.SweepMax, Vector3(8.01f, 5.01f, 13.01f));
	W3DOrientedBoxCastQuery transformed_oriented(oriented, rotate_translate);
	BOOST_CHECK(transformed_oriented.Result == &result);
	BOOST_CHECK(transformed_oriented.CollidedRenderObj == hit);
	BOOST_CHECK_EQUAL(transformed_oriented.CollisionType, SCENE_QUERY_VEHICLE);
	Check_Vector(transformed_oriented.Box.Center, Vector3(12, -19, 33));
	Check_Vector(transformed_oriented.Box.Extent, Vector3(2, 3, 4));
	Check_Vector(transformed_oriented.Move, Vector3(-5, 4, 6));
	// These extrema are calculated from the source sweep bounds and the
	// explicit rotate-plus-translate matrix above.
	Check_Vector(transformed_oriented.SweepMin, Vector3(4.99f, -22.01f, 28.99f));
	Check_Vector(transformed_oriented.SweepMax, Vector3(14.01f, -11.99f, 43.01f));

	W3DBoxIntersectionQuery axis_intersection(
		AABoxClass(Vector3(7, 0, 0), Vector3(1, 1, 1)), SCENE_QUERY_CAMERA);
	BOOST_CHECK(!axis_intersection.Cull(AABoxClass(Vector3(9, 0, 0), Vector3(1, 1, 1))));
	BOOST_CHECK(axis_intersection.Cull(AABoxClass(Vector3(9.01f, 0, 0), Vector3(1, 1, 1))));
	W3DOrientedBoxIntersectionQuery transformed_intersection(axis_intersection, rotate_translate);
	BOOST_CHECK_EQUAL(transformed_intersection.CollisionType, SCENE_QUERY_CAMERA);
	Check_Vector(transformed_intersection.Box.Center, Vector3(10, -13, 30));
	Check_Vector(transformed_intersection.BoundingBox.Center, Vector3(10, -13, 30));
	Check_Vector(transformed_intersection.BoundingBox.Extent, Vector3(1, 1, 1));
}

BOOST_AUTO_TEST_CASE(particle_emitter_names_own_storage_across_copy_clone_and_source_release)
{
	RefCountPtr<W3DEmitterRenderObject> copied;
	RefCountPtr<W3DEmitterRenderObject> cloned;
	{
		auto source = Make_Name_Test_Emitter();
		BOOST_REQUIRE(source.Peek() != nullptr);
		BOOST_REQUIRE(source->Get_Name() != nullptr);
		BOOST_CHECK(std::strcmp(source->Get_Name(), "ParticleEmitter") == 0);

		source->Set_Name(nullptr);
		BOOST_CHECK(source->Get_Name() == nullptr);
		source->Set_Name("");
		BOOST_REQUIRE(source->Get_Name() != nullptr);
		BOOST_CHECK(std::strcmp(source->Get_Name(), "") == 0);

		char authored_name[] = "SourceName";
		source->Set_Name(authored_name);
		authored_name[0] = 'X';
		BOOST_REQUIRE(source->Get_Name() != nullptr);
		BOOST_CHECK(std::strcmp(source->Get_Name(), "SourceName") == 0);

		copied = RefCountPtr<W3DEmitterRenderObject>::Create_No_Add_Ref(
			new W3DEmitterRenderObject(*source));
		cloned = RefCountPtr<W3DEmitterRenderObject>::Create_No_Add_Ref(
			static_cast<W3DEmitterRenderObject *>(source->Clone()));
		BOOST_REQUIRE(copied.Peek() != nullptr);
		BOOST_REQUIRE(cloned.Peek() != nullptr);
		BOOST_CHECK(std::strcmp(copied->Get_Name(), "SourceName") == 0);
		BOOST_CHECK(std::strcmp(cloned->Get_Name(), "SourceName") == 0);

		source->Set_Name("SourceMutated");
		BOOST_CHECK(std::strcmp(copied->Get_Name(), "SourceName") == 0);
		BOOST_CHECK(std::strcmp(cloned->Get_Name(), "SourceName") == 0);
		source.Clear();
	}

	BOOST_REQUIRE(copied.Peek() != nullptr);
	BOOST_REQUIRE(cloned.Peek() != nullptr);
	BOOST_CHECK(std::strcmp(copied->Get_Name(), "SourceName") == 0);
	BOOST_CHECK(std::strcmp(cloned->Get_Name(), "SourceName") == 0);

	copied->Set_Name(nullptr);
	BOOST_CHECK(copied->Get_Name() == nullptr);
	BOOST_CHECK(std::strcmp(cloned->Get_Name(), "SourceName") == 0);
	cloned->Set_Name("");
	BOOST_REQUIRE(cloned->Get_Name() != nullptr);
	BOOST_CHECK(std::strcmp(cloned->Get_Name(), "") == 0);
	BOOST_CHECK(copied->Get_Name() == nullptr);
}

BOOST_AUTO_TEST_CASE(material3_conversion_preserves_channel_products_and_scalar_values)
{
	Graphics::MeshMaterial material;
	BOOST_REQUIRE(Load_Test_Material(material));

	Vector3 diffuse;
	Vector3 specular;
	Vector3 emissive;
	Vector3 ambient;
	diffuse.Set(material.parameters.diffuse[0],material.parameters.diffuse[1],material.parameters.diffuse[2]);
	specular.Set(material.parameters.specular[0],material.parameters.specular[1],material.parameters.specular[2]);
	emissive.Set(material.parameters.emissive[0],material.parameters.emissive[1],material.parameters.emissive[2]);
	ambient.Set(material.parameters.ambient[0],material.parameters.ambient[1],material.parameters.ambient[2]);
	Check_Vector(diffuse, Vector3(
		Byte_Product(128, 128), Byte_Product(127, 127), Byte_Product(254, 0)));
	Check_Vector(specular, Vector3(
		Byte_Product(255, 1), Byte_Product(254, 0), Byte_Product(0, 255)));
	Check_Vector(emissive, Vector3(
		Byte_To_Float(1), Byte_To_Float(0), Byte_To_Float(127)));
	Check_Vector(ambient, Vector3(
		Byte_To_Float(128), Byte_To_Float(1), Byte_To_Float(0)));
	BOOST_CHECK_SMALL(material.parameters.shininess - 17.0f, kFloatTolerance);
	BOOST_CHECK_SMALL(material.parameters.opacity - 0.625f, kFloatTolerance);

	const Graphics::PropMaterial &converted = material.parameters;
	for (unsigned channel = 0; channel < 3; ++channel) {
		BOOST_CHECK_SMALL(converted.diffuse[channel] - diffuse[channel], kFloatTolerance);
		BOOST_CHECK_SMALL(converted.specular[channel] - specular[channel], kFloatTolerance);
		BOOST_CHECK_SMALL(converted.emissive[channel] - emissive[channel], kFloatTolerance);
		BOOST_CHECK_SMALL(converted.ambient[channel] - ambient[channel], kFloatTolerance);
	}
	BOOST_CHECK_SMALL(converted.shininess - 17.0f, kFloatTolerance);
	BOOST_CHECK_SMALL(converted.opacity - 0.625f, kFloatTolerance);
}

BOOST_AUTO_TEST_CASE(material3_rejects_truncated_records_and_keeps_format_flags_separate)
{
    auto bytes=Test_Material3();
    bytes[0]=std::byte{0x35};
    Assets::W3D::W3DMaterial3Data decoded;
    BOOST_REQUIRE(Assets::W3D::W3DRead_Material3(bytes,decoded));
    BOOST_CHECK_EQUAL(decoded.attributes,0x35u);
    Graphics::MeshMaterial material;
    material.flags=4;
    Graphics::Apply_Mesh_Material_Values(material,decoded.material);
    BOOST_CHECK_EQUAL(material.flags,4u);
    BOOST_CHECK(!Assets::W3D::W3DRead_Material3(std::span<const std::byte>(bytes).first(43),decoded));
    BOOST_CHECK_EQUAL(decoded.attributes,0u);
}

BOOST_AUTO_TEST_CASE(material3_runtime_adapter_preserves_alpha_sort_and_authored_map_order)
{
	Assets::W3D::W3DMaterial3Data source;
	source.attributes = Assets::W3D::W3DMaterialUseAlpha;
	source.material.name = "alpha_material";
	source.material.base_color = {0, 0, 0, 1};
	source.maps = {
		{Assets::W3D::W3DMaterial3MapKind::SpecularColor, "ignored_sc.tga"},
		{Assets::W3D::W3DMaterial3MapKind::DiffuseIllumination, "ignored_di.tga"},
		{Assets::W3D::W3DMaterial3MapKind::SpecularIllumination, "first_si.tga"},
		{Assets::W3D::W3DMaterial3MapKind::DiffuseColor, "middle_dc.tga"},
		{Assets::W3D::W3DMaterial3MapKind::SpecularIllumination, "last_si.tga"}};

	Graphics::MeshMaterial material;
	const auto runtime = Graphics::Apply_W3D_Material3(material, source);
	BOOST_CHECK(runtime.requires_sort);
	BOOST_CHECK_EQUAL(runtime.shader.Get_Dst_Blend_Func(), Graphics::MaterialState::DSTBLEND_ONE);
	BOOST_CHECK_EQUAL(runtime.shader.Get_Src_Blend_Func(), Graphics::MaterialState::SRCBLEND_ONE);
	BOOST_CHECK_EQUAL(runtime.shader.Get_Primary_Gradient(), Graphics::MaterialState::GRADIENT_DISABLE);
	BOOST_CHECK_EQUAL(runtime.shader.Get_Texturing(), Graphics::MaterialState::TEXTURING_ENABLE);
	BOOST_REQUIRE(runtime.texture_map != nullptr);
	BOOST_CHECK(runtime.texture_map->filename == "last_si.tga");
	BOOST_CHECK(material.name == "alpha_material");
	BOOST_CHECK_SMALL(material.parameters.diffuse[0], kFloatTolerance);

	// A DC map after SI keeps the additive shader state while becoming the
	// selected map; this is the reverse transition used by exported materials.
	source.maps.push_back({Assets::W3D::W3DMaterial3MapKind::DiffuseColor, "last_dc.tga"});
	const auto reverse = Graphics::Apply_W3D_Material3(material, source);
	BOOST_REQUIRE(reverse.texture_map != nullptr);
	BOOST_CHECK(reverse.texture_map->filename == "last_dc.tga");
	BOOST_CHECK_EQUAL(reverse.shader.Get_Dst_Blend_Func(), Graphics::MaterialState::DSTBLEND_ONE);
	BOOST_CHECK_EQUAL(reverse.shader.Get_Src_Blend_Func(), Graphics::MaterialState::SRCBLEND_ONE);
	BOOST_CHECK(reverse.requires_sort);
}

BOOST_AUTO_TEST_CASE(material3_runtime_adapter_ignores_si_when_diffuse_is_nonzero)
{
	Assets::W3D::W3DMaterial3Data source;
	source.attributes = Assets::W3D::W3DMaterialUseAlpha;
	source.material.base_color = {.25f, 0, 0, 1};
	source.maps.push_back({Assets::W3D::W3DMaterial3MapKind::SpecularIllumination, "ignored_si.tga"});

	Graphics::MeshMaterial material;
	const auto runtime = Graphics::Apply_W3D_Material3(material, source);
	BOOST_CHECK(runtime.requires_sort);
	BOOST_CHECK(runtime.texture_map == nullptr);
	BOOST_CHECK_EQUAL(runtime.shader.Get_Depth_Mask(), Graphics::MaterialState::DEPTH_WRITE_DISABLE);
	BOOST_CHECK_EQUAL(runtime.shader.Get_Src_Blend_Func(), Graphics::MaterialState::SRCBLEND_SRC_ALPHA);
	BOOST_CHECK_EQUAL(runtime.shader.Get_Dst_Blend_Func(), Graphics::MaterialState::DSTBLEND_ONE_MINUS_SRC_ALPHA);
	BOOST_CHECK_EQUAL(runtime.shader.Get_Texturing(), Graphics::MaterialState::TEXTURING_DISABLE);
	BOOST_CHECK_SMALL(material.parameters.ambient[0] - .25f, kFloatTolerance);
	BOOST_CHECK_SMALL(material.parameters.diffuse[0], kFloatTolerance);
}

BOOST_AUTO_TEST_CASE(material3_runtime_adapter_reports_earlier_animated_map)
{
	Assets::W3D::W3DMaterial3Data source;
	source.material.base_color = {0, 0, 0, 1};
	source.maps = {
		{Assets::W3D::W3DMaterial3MapKind::SpecularIllumination, "animated_si.tga", 0, 2, 24.0f},
		{Assets::W3D::W3DMaterial3MapKind::DiffuseColor, "static_dc.tga", 0, 1, 0.0f}};
	Graphics::MeshMaterial material;
	const auto runtime = Graphics::Apply_W3D_Material3(material, source);
	BOOST_CHECK(runtime.has_animated_texture);
	BOOST_REQUIRE(runtime.texture_map != nullptr);
	BOOST_CHECK(runtime.texture_map->filename == "static_dc.tga");
}

BOOST_AUTO_TEST_CASE(material3_runtime_adapter_moves_untextured_diffuse_to_ambient)
{
	Assets::W3D::W3DMaterial3Data source;
	source.material.name = "solid_material";
	source.material.base_color = {.25f, .5f, .75f, 1};
	Graphics::MeshMaterial material;
	const auto runtime = Graphics::Apply_W3D_Material3(material, source);

	BOOST_CHECK(!runtime.requires_sort);
	BOOST_CHECK(runtime.texture_map == nullptr);
	BOOST_CHECK_EQUAL(runtime.shader.Get_Texturing(), Graphics::MaterialState::TEXTURING_DISABLE);
	BOOST_CHECK_SMALL(material.parameters.ambient[0] - .25f, kFloatTolerance);
	BOOST_CHECK_SMALL(material.parameters.ambient[1] - .5f, kFloatTolerance);
	BOOST_CHECK_SMALL(material.parameters.ambient[2] - .75f, kFloatTolerance);
	BOOST_CHECK_SMALL(material.parameters.diffuse[0], kFloatTolerance);
	BOOST_CHECK_SMALL(material.parameters.diffuse[1], kFloatTolerance);
	BOOST_CHECK_SMALL(material.parameters.diffuse[2], kFloatTolerance);
}

BOOST_AUTO_TEST_CASE(light_w3d_roundtrip_preserves_encoded_colors_and_authored_parameters)
{
	W3DLight source(W3DLight::SPOT);
	Configure_Test_Light(source);

	SerializedW3D saved;
	BOOST_REQUIRE(Serialize_Light(source, saved));

	W3DLightInfoFixture light_info;
	std::memset(&light_info, 0, sizeof(light_info));
	BOOST_REQUIRE(Read_Light_Info(saved, light_info));
	BOOST_CHECK_EQUAL(light_info.Attributes,
		static_cast<uint32>(kW3DLightAttributeSpot | kW3DLightAttributeCastShadows));
	BOOST_CHECK_EQUAL(static_cast<unsigned>(light_info.Ambient.R), 127u);
	BOOST_CHECK_EQUAL(static_cast<unsigned>(light_info.Ambient.G), 63u);
	BOOST_CHECK_EQUAL(static_cast<unsigned>(light_info.Ambient.B), 191u);
	BOOST_CHECK_EQUAL(static_cast<unsigned>(light_info.Ambient.pad), 0u);
	BOOST_CHECK_EQUAL(static_cast<unsigned>(light_info.Diffuse.R), 63u);
	BOOST_CHECK_EQUAL(static_cast<unsigned>(light_info.Diffuse.G), 127u);
	BOOST_CHECK_EQUAL(static_cast<unsigned>(light_info.Diffuse.B), 191u);
	BOOST_CHECK_EQUAL(static_cast<unsigned>(light_info.Diffuse.pad), 0u);
	BOOST_CHECK_EQUAL(static_cast<unsigned>(light_info.Specular.R), 191u);
	BOOST_CHECK_EQUAL(static_cast<unsigned>(light_info.Specular.G), 127u);
	BOOST_CHECK_EQUAL(static_cast<unsigned>(light_info.Specular.B), 63u);
	BOOST_CHECK_EQUAL(static_cast<unsigned>(light_info.Specular.pad), 0u);
	BOOST_CHECK_SMALL(light_info.Intensity - 1.0f, kFloatTolerance);

	W3DLight loaded(W3DLight::POINT);
	BOOST_REQUIRE(Load_Light(saved, loaded));
	BOOST_CHECK(loaded.Get_Type() == W3DLight::SPOT);
	BOOST_CHECK(loaded.Are_Shadows_Enabled());
	BOOST_CHECK_SMALL(loaded.Get_Intensity() - 1.0f, kFloatTolerance);
	Check_Light_Color(loaded,
		Vector3(Byte_To_Float(127), Byte_To_Float(63), Byte_To_Float(191)),
		Vector3(Byte_To_Float(63), Byte_To_Float(127), Byte_To_Float(191)),
		Vector3(Byte_To_Float(191), Byte_To_Float(127), Byte_To_Float(63)));
	Vector3 direction;
	loaded.Get_Spot_Direction(direction);
	Check_Vector(direction, Vector3(0.0f, 0.0f, -1.0f));
	BOOST_CHECK_SMALL(loaded.Get_Spot_Angle() - 0.75f, kFloatTolerance);
	BOOST_CHECK_SMALL(loaded.Get_Spot_Exponent() - 2.5f, kFloatTolerance);
	BOOST_CHECK(loaded.Get_Flag(W3DLight::FAR_ATTENUATION));
	double far_start = 0.0;
	double far_end = 0.0;
	loaded.Get_Far_Attenuation_Range(far_start, far_end);
	BOOST_CHECK_SMALL(static_cast<float>(far_start - 10.0), kFloatTolerance);
	BOOST_CHECK_SMALL(static_cast<float>(far_end - 20.0), kFloatTolerance);

	W3DSpotLightFixture spot_info;
	std::memset(&spot_info, 0, sizeof(spot_info));
	spot_info.SpotDirection.X = 0.0f;
	spot_info.SpotDirection.Y = 0.0f;
	spot_info.SpotDirection.Z = -1.0f;
	spot_info.SpotAngle = 0.75f;
	spot_info.SpotExponent = 2.5f;
	W3DLightAttenuationFixture near_attenuation{2.0f, 5.0f};
	W3DLightAttenuationFixture far_attenuation{10.0f, 20.0f};
	SerializedW3D fixture;
	BOOST_REQUIRE(Write_Light_Load_Fixture(
		light_info, spot_info, near_attenuation, far_attenuation, fixture));
	W3DLight loaded_fixture(W3DLight::POINT);
	BOOST_REQUIRE(Load_Light(fixture, loaded_fixture));
	double near_start = 0.0;
	double near_end = 0.0;
	loaded_fixture.Get_Near_Attenuation_Range(near_start, near_end);
	loaded_fixture.Get_Far_Attenuation_Range(far_start, far_end);
	BOOST_CHECK_SMALL(static_cast<float>(near_start - 2.0), kFloatTolerance);
	BOOST_CHECK_SMALL(static_cast<float>(near_end - 5.0), kFloatTolerance);
	BOOST_CHECK_SMALL(static_cast<float>(far_start - 10.0), kFloatTolerance);
	BOOST_CHECK_SMALL(static_cast<float>(far_end - 20.0), kFloatTolerance);
}

BOOST_AUTO_TEST_CASE(light_w3d_load_reports_success_for_a_fixture_and_failure_for_truncation)
{
	W3DLightInfoFixture light_info{};
	light_info.Attributes = kW3DLightAttributePoint;
	light_info.Intensity = 1.0f;
	W3DSpotLightFixture spot_info{};
	W3DLightAttenuationFixture near_attenuation{2.0f, 5.0f};
	W3DLightAttenuationFixture far_attenuation{10.0f, 20.0f};
	SerializedW3D fixture;
	BOOST_REQUIRE(Write_Light_Load_Fixture(
		light_info, spot_info, near_attenuation, far_attenuation, fixture));

	W3DLight loaded(W3DLight::POINT);
	BOOST_CHECK(Load_Light(fixture, loaded));

	SerializedW3D truncated = fixture;
	BOOST_REQUIRE(truncated.size > 0);
	--truncated.size;
	W3DLight rejected(W3DLight::POINT);
	BOOST_CHECK(!Load_Light(truncated, rejected));
}

BOOST_AUTO_TEST_CASE(w3d_light_copy_clone_and_assignment_preserve_independent_state)
{
	W3DLight source(W3DLight::SPOT);
	Configure_Test_Light(source);
	Matrix3D source_transform(1);
	source_transform.Rotate_Z(0.5f);
	source_transform.Set_Translation(Vector3(3.0f, 4.0f, 5.0f));
	source.Set_Transform(source_transform);
	const float copied_spot_cosine = source.Get_Spot_Angle_Cos();

	std::unique_ptr<W3DRenderObject> cloned_object(source.Clone());
	W3DLight *cloned = static_cast<W3DLight *>(cloned_object.get());
	BOOST_REQUIRE(cloned != nullptr);
	const Matrix3D identity_transform(1);
	BOOST_CHECK(cloned->Get_Transform() == identity_transform);
	BOOST_CHECK(source.Get_Transform() == source_transform);

	W3DLight assigned(W3DLight::POINT);
	const Matrix3D assigned_transform(Vector3(-6.0f, -7.0f, -8.0f));
	assigned.Set_Transform(assigned_transform);
	assigned = source;
	// W3DRenderObject assignment deliberately copies render flags but retains
	// the destination transform, matching the retired light behavior.
	BOOST_CHECK(assigned.Get_Transform() == assigned_transform);

	const auto check_copy = [](const W3DLight &light) {
		BOOST_CHECK(light.Get_Type() == W3DLight::SPOT);
		BOOST_CHECK(light.Are_Shadows_Enabled());
		BOOST_CHECK_EQUAL(light.Get_Flag(W3DLight::FAR_ATTENUATION), 1);
		BOOST_CHECK_SMALL(light.Get_Intensity() - 1.0f, kFloatTolerance);
		Check_Light_Color(light,
			Vector3(0.5f, 0.25f, 0.75f),
			Vector3(0.25f, 0.5f, 0.75f),
			Vector3(0.75f, 0.5f, 0.25f));
		BOOST_CHECK_SMALL(light.Get_Spot_Angle() - 0.75f, kFloatTolerance);
		BOOST_CHECK_SMALL(light.Get_Spot_Exponent() - 2.5f, kFloatTolerance);
	};
	check_copy(*cloned);
	check_copy(assigned);

	// NEAR_ATTENUATION retains its legacy zero-valued flag encoding. Both
	// operations are no-ops, including after the far flag has been enabled.
	BOOST_CHECK_EQUAL(source.Get_Flag(W3DLight::NEAR_ATTENUATION), 0);
	source.Set_Flag(W3DLight::NEAR_ATTENUATION, false);
	BOOST_CHECK_EQUAL(source.Get_Flag(W3DLight::FAR_ATTENUATION), 1);
	source.Set_Flag(W3DLight::NEAR_ATTENUATION, true);
	BOOST_CHECK_EQUAL(source.Get_Flag(W3DLight::FAR_ATTENUATION), 1);

	source.Set_Intensity(3.0f);
	source.Set_Ambient(Vector3(0.1f, 0.2f, 0.3f));
	source.Set_Spot_Angle(0.25f);
	source.Set_Flag(W3DLight::FAR_ATTENUATION, false);
	BOOST_CHECK_SMALL(cloned->Get_Intensity() - 1.0f, kFloatTolerance);
	BOOST_CHECK_SMALL(assigned.Get_Intensity() - 1.0f, kFloatTolerance);
	BOOST_CHECK_SMALL(cloned->Get_Spot_Angle_Cos() - copied_spot_cosine, kFloatTolerance);
	BOOST_CHECK_SMALL(assigned.Get_Spot_Angle_Cos() - copied_spot_cosine, kFloatTolerance);
	BOOST_CHECK_EQUAL(cloned->Get_Flag(W3DLight::FAR_ATTENUATION), 1);
	BOOST_CHECK_EQUAL(assigned.Get_Flag(W3DLight::FAR_ATTENUATION), 1);
	Check_Light_Color(*cloned,
		Vector3(0.5f, 0.25f, 0.75f),
		Vector3(0.25f, 0.5f, 0.75f),
		Vector3(0.75f, 0.5f, 0.25f));
}

BOOST_AUTO_TEST_CASE(w3d_light_registers_and_unregisters_with_scene)
{
	RecordingScene scene;
	W3DLight light(W3DLight::POINT);

	light.Notify_Added(&scene);
	BOOST_CHECK_EQUAL(scene.light_register_count, 1u);
	BOOST_CHECK_EQUAL(scene.light_unregister_count, 0u);
	BOOST_CHECK(light.Peek_Scene() == &scene);

	light.Notify_Removed(&scene);
	BOOST_CHECK_EQUAL(scene.light_register_count, 1u);
	BOOST_CHECK_EQUAL(scene.light_unregister_count, 1u);
	BOOST_CHECK(light.Peek_Scene() == nullptr);
}

BOOST_AUTO_TEST_CASE(w3d_light_save_marks_top_and_nested_chunk_flags)
{
	W3DLight source(W3DLight::SPOT);
	Configure_Test_Light(source);

	SerializedW3D top_level;
	BOOST_REQUIRE(Serialize_Light(source, top_level));
	RAMFileClass top_file(top_level.bytes.data(), static_cast<int>(top_level.size));
	BOOST_REQUIRE(top_file.Open(FileClass::READ));
	ChunkLoadClass top_load(&top_file);
	BOOST_REQUIRE(top_load.Open_Chunk());
	BOOST_CHECK_EQUAL(top_load.Cur_Chunk_ID(), kW3DChunkLight);
	BOOST_CHECK(top_load.Contains_Chunks() != 0);
	BOOST_REQUIRE(top_load.Open_Chunk());
	BOOST_CHECK_EQUAL(top_load.Cur_Chunk_ID(), kW3DChunkLightInfo);
	BOOST_CHECK_EQUAL(top_load.Contains_Chunks(), 0);
	BOOST_REQUIRE(top_load.Close_Chunk());
	BOOST_REQUIRE(top_load.Open_Chunk());
	BOOST_CHECK_EQUAL(top_load.Cur_Chunk_ID(), kW3DChunkSpotLightInfo);
	BOOST_CHECK_EQUAL(top_load.Contains_Chunks(), 0);
	BOOST_REQUIRE(top_load.Close_Chunk());
	BOOST_REQUIRE(top_load.Open_Chunk());
	BOOST_CHECK_EQUAL(top_load.Cur_Chunk_ID(), kW3DChunkFarAttenuation);
	BOOST_CHECK_EQUAL(top_load.Contains_Chunks(), 0);
	BOOST_REQUIRE(top_load.Close_Chunk());
	BOOST_CHECK(!top_load.Open_Chunk());
	BOOST_REQUIRE(top_load.Close_Chunk());
	top_file.Close();

	// Save() adds the persisted W3D payload below a separate state chunk. The
	// flag must be set on each container while leaf W3D records stay data-only.
	SerializedW3D persisted;
	RAMFileClass persisted_file(persisted.bytes.data(), static_cast<int>(persisted.bytes.size()));
	BOOST_REQUIRE(persisted_file.Open(FileClass::WRITE));
	ChunkSaveClass persisted_save(&persisted_file);
	BOOST_REQUIRE(persisted_save.Begin_Chunk(0x10203040u));
	BOOST_REQUIRE(source.Save(persisted_save));
	BOOST_REQUIRE(persisted_save.End_Chunk());
	persisted_file.Close();
	persisted.size = static_cast<std::size_t>(persisted_file.Size());

	RAMFileClass persisted_read_file(persisted.bytes.data(), static_cast<int>(persisted.size));
	BOOST_REQUIRE(persisted_read_file.Open(FileClass::READ));
	ChunkLoadClass persisted_load(&persisted_read_file);
	BOOST_REQUIRE(persisted_load.Open_Chunk());
	BOOST_CHECK_EQUAL(persisted_load.Cur_Chunk_ID(), 0x10203040u);
	BOOST_CHECK(persisted_load.Contains_Chunks() != 0);
	BOOST_REQUIRE(persisted_load.Open_Chunk());
	BOOST_CHECK_EQUAL(persisted_load.Cur_Chunk_ID(), 0x02157100u);
	BOOST_CHECK(persisted_load.Contains_Chunks() != 0);
	BOOST_REQUIRE(persisted_load.Open_Chunk());
	BOOST_CHECK_EQUAL(persisted_load.Cur_Chunk_ID(), kW3DChunkLight);
	BOOST_CHECK(persisted_load.Contains_Chunks() != 0);
	BOOST_REQUIRE(persisted_load.Open_Chunk());
	BOOST_CHECK_EQUAL(persisted_load.Cur_Chunk_ID(), kW3DChunkLightInfo);
	BOOST_CHECK_EQUAL(persisted_load.Contains_Chunks(), 0);
	BOOST_REQUIRE(persisted_load.Close_Chunk());
	BOOST_REQUIRE(persisted_load.Close_Chunk());
	BOOST_REQUIRE(persisted_load.Close_Chunk());
	BOOST_REQUIRE(persisted_load.Open_Chunk());
	BOOST_CHECK_EQUAL(persisted_load.Cur_Chunk_ID(), 0x02157101u);
	BOOST_CHECK_EQUAL(persisted_load.Contains_Chunks(), 0);
	BOOST_REQUIRE(persisted_load.Close_Chunk());
	BOOST_CHECK(!persisted_load.Open_Chunk());
	BOOST_REQUIRE(persisted_load.Close_Chunk());
	persisted_read_file.Close();
}

BOOST_AUTO_TEST_CASE(converted_w3d_material_and_light_values_reach_prop_pixels)
{
	Graphics::MeshMaterial source_material;
	BOOST_REQUIRE(Load_Test_Material(source_material));
	source_material.parameters.lighting = true;
	const Graphics::PropMaterial converted_material = source_material.parameters;

	W3DLight source(W3DLight::SPOT);
	Configure_Test_Light(source);
	SerializedW3D saved_light;
	BOOST_REQUIRE(Serialize_Light(source, saved_light));
	W3DLight loaded_light(W3DLight::POINT);
	BOOST_REQUIRE(Load_Light(saved_light, loaded_light));

	Vector3 ambient;
	Vector3 diffuse;
	Vector3 specular;
	Vector3 direction;
	loaded_light.Get_Ambient(&ambient);
	loaded_light.Get_Diffuse(&diffuse);
	loaded_light.Get_Specular(&specular);
	loaded_light.Get_Spot_Direction(direction);
	double attenuation_start = 0.0;
	double attenuation_end = 0.0;
	loaded_light.Get_Far_Attenuation_Range(attenuation_start, attenuation_end);

	Graphics::GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());
	Graphics::PropRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device,
		Graphics::Test_Shader_Directory(GRAPHICS_W3D_VALUE_CONVERSION_SHADER_DIRECTORY)));
	const auto target = device.Create_Texture({
		8, 8, 1, Graphics::RHITextureFormat::RGBA8_UNorm,
		static_cast<std::uint32_t>(Graphics::RHITextureUsage::RenderTarget)});
	const auto depth = device.Create_Texture({
		8, 8, 1, Graphics::RHITextureFormat::D32_Float,
		static_cast<std::uint32_t>(Graphics::RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid());
	BOOST_REQUIRE(depth.Is_Valid());

	std::array<Graphics::PropVertex, 4> vertices{};
	vertices[0].position = {-1.0f, -1.0f, 0.5f};
	vertices[1].position = {1.0f, -1.0f, 0.5f};
	vertices[2].position = {1.0f, 1.0f, 0.5f};
	vertices[3].position = {-1.0f, 1.0f, 0.5f};
	for (auto &vertex : vertices) {
		vertex.normal = {0.0f, 0.0f, 1.0f};
		Graphics::Apply_Prop_Material(vertex, converted_material);
	}
	const std::array<std::uint32_t, 6> indices{0, 1, 2, 0, 2, 3};
	const auto mesh = renderer.Create_Mesh(vertices, indices);
	BOOST_REQUIRE(mesh.Is_Valid());

	Graphics::PropParameters parameters;
	parameters.view_projection = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	parameters.camera_position = {0.0f, 0.0f, 5.0f, 1.0f};
	parameters.textured = 0.0f;
	parameters.secondary_gradient = 1.0f;
	parameters.light_direction[0] = {direction.X, direction.Y, direction.Z, 1.0f};
	parameters.light_diffuse[0] = {diffuse.X, diffuse.Y, diffuse.Z, 0.0f};
	parameters.light_specular[0] = {specular.X, specular.Y, specular.Z, 0.0f};
	parameters.light_ambient[0] = {ambient.X, ambient.Y, ambient.Z, 0.0f};
	parameters.light_position[0] = {0.0f, 0.0f, 10.0f,
		loaded_light.Get_Type() == W3DLight::SPOT ? 2.0f : 0.0f};
	parameters.light_attenuation[0] = {
		1.0f, 0.0f, 0.0f, static_cast<float>(attenuation_end)};
	parameters.light_spot[0] = {
		loaded_light.Get_Spot_Exponent(), 0.5f * loaded_light.Get_Spot_Angle(),
		loaded_light.Get_Spot_Angle(), 0.0f};

	Graphics::PropStyle style;
	style.blend = Graphics::RHIBlendMode::Disabled;
	style.depth_test = false;
	style.depth_write = false;
	style.cull = Graphics::RHICullMode::None;
	auto &commands = device.Immediate_Command_List();
	BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
	BOOST_REQUIRE(commands.Set_Viewport({0, 0, 8, 8}));
	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1.0f));
	BOOST_REQUIRE(renderer.Draw(commands, mesh, style, parameters, {}));

	std::array<std::byte, 8 * 8 * 4> pixels{};
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, 8 * 4));
	const std::size_t center = (4 * 8 + 4) * 4;
	const std::array<int, 4> expected{81, 32, 127, 159};
	for (unsigned channel = 0; channel < expected.size(); ++channel)
		BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + channel]) - expected[channel], 2);

	renderer.Destroy_Mesh(mesh);
	renderer.Shutdown();
	device.Destroy_Texture(target);
	device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(material3_container_converter_state_reaches_prop_pixels)
{
    const auto bytes=Make_Converter_Material3_Container();
    std::vector<Assets::W3D::W3DMaterial3Data> decoded;
    BOOST_REQUIRE(Assets::W3D::W3DRead_Material3_Container(bytes,decoded));
    BOOST_REQUIRE_EQUAL(decoded.size(),1u);
    BOOST_CHECK_EQUAL(decoded.front().attributes,Assets::W3D::W3DMaterialUseAlpha);
    BOOST_REQUIRE_EQUAL(decoded.front().maps.size(),2u);

    Graphics::MeshMaterial material;
    const auto runtime=Graphics::Apply_W3D_Material3(material,decoded.front());
    BOOST_REQUIRE(runtime.texture_map!=nullptr);
    BOOST_CHECK(runtime.texture_map->filename=="second.tga");
    BOOST_CHECK_EQUAL(runtime.shader.Get_Src_Blend_Func(),Graphics::MaterialState::SRCBLEND_SRC_ALPHA);
    BOOST_CHECK_EQUAL(runtime.shader.Get_Dst_Blend_Func(),Graphics::MaterialState::DSTBLEND_ONE_MINUS_SRC_ALPHA);
    BOOST_CHECK_EQUAL(runtime.shader.Get_Depth_Mask(),Graphics::MaterialState::DEPTH_WRITE_DISABLE);
    BOOST_CHECK_EQUAL(runtime.shader.Get_Texturing(),Graphics::MaterialState::TEXTURING_ENABLE);

    Graphics::GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    const auto resolve_texture=[&](std::string_view filename) {
        const std::array<std::uint8_t,4> texel = filename=="first.tga"
            ? std::array<std::uint8_t,4>{1,2,3,4}
            : filename=="second.tga"
                ? std::array<std::uint8_t,4>{128,255,64,128}
                : std::array<std::uint8_t,4>{0,0,0,0};
        return device.Create_Texture_Initialized({1,1},
            {std::as_bytes(std::span(texel)),4});
    };
    const auto first_texture=resolve_texture("first.tga");
    const auto selected_texture=resolve_texture(runtime.texture_map->filename);
    BOOST_REQUIRE(first_texture.Is_Valid());
    BOOST_REQUIRE(selected_texture.Is_Valid());
    BOOST_CHECK(selected_texture!=first_texture);
    Graphics::PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,
        Graphics::Test_Shader_Directory(GRAPHICS_W3D_VALUE_CONVERSION_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({
        8,8,1,Graphics::RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(Graphics::RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({
        8,8,1,Graphics::RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(Graphics::RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(target.Is_Valid());
    BOOST_REQUIRE(depth.Is_Valid());

    std::array<Graphics::PropVertex,4> vertices{};
    vertices[0].position={-1.0f,-1.0f,0.5f};
    vertices[1].position={1.0f,-1.0f,0.5f};
    vertices[2].position={1.0f,1.0f,0.5f};
    vertices[3].position={-1.0f,1.0f,0.5f};
    for (auto& vertex:vertices) {
        vertex.normal={0.0f,0.0f,1.0f};
        Graphics::Apply_Prop_Material(vertex,material.parameters);
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());

    Graphics::PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.camera_position={0.0f,0.0f,5.0f,1.0f};
    Graphics::SceneDrawParameters scene;
    auto style=Graphics::Resolve_Prop_Material_State(runtime.shader,scene,false,parameters);
    style.samplers[0].Set_Filter(Graphics::RHISamplerFilter::Point);
    style.cull=Graphics::RHICullMode::None;

    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1.0f));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{selected_texture}));

    std::array<std::byte,8*8*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,8*4));
    const std::array<int,4> expected{16,64,24,64};
    const auto check_interior=[&](std::size_t x,std::size_t y) {
        const std::size_t pixel=(y*8+x)*4;
        for (unsigned channel=0;channel<expected.size();++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[pixel+channel])-expected[channel],2);
    };
    // The two samples are strictly inside opposite triangles of the indexed
    // quad, so both triangle interiors exercise the selected map binding.
    check_interior(2,4);
    check_interior(5,4);

    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    device.Destroy_Texture(first_texture);
    device.Destroy_Texture(selected_texture);
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(segmented_line_cloning_resets_scene_transform_and_assignment_retains_destination)
{
    RefCountPtr<W3DSegmentedLineRenderObject> source =
        Create_No_Add_Ref(new W3DSegmentedLineRenderObject);
    const Vector3 points[]{Vector3(-1, 0, -5), Vector3(1, 0, -5)};
    source->Set_Points(2, points);
    source->Set_Width(2);
    source->Set_Subdivision_Levels(3);
    source->Set_LOD_Level(2);
    source->Set_Hidden(true);
    Matrix3D transform = Matrix3D::RotateZ90;
    transform.Set_Translation(Vector3(3, 4, 5));
    source->Set_Transform(transform);

    RefCountPtr<W3DSegmentedLineRenderObject> clone = Create_No_Add_Ref(
        static_cast<W3DSegmentedLineRenderObject *>(source->Clone()));
    const Matrix3D identity(1);
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < 4; ++column)
            BOOST_CHECK_EQUAL(clone->Get_Transform()[row][column], identity[row][column]);
    BOOST_CHECK(!clone->Is_Hidden());
    BOOST_CHECK_EQUAL(clone->Get_Num_Points(), 2);
    BOOST_CHECK_EQUAL(clone->Get_Width(), 2);
    BOOST_CHECK_EQUAL(clone->Get_Subdivision_Levels(), 3u);
    BOOST_CHECK_EQUAL(clone->Get_LOD_Level(), 2);
    BOOST_CHECK(clone->Peek_Scene() == nullptr);
    BOOST_CHECK(clone->Get_Container() == nullptr);

    clone->Set_Position(Vector3(8, 9, 10));
    *clone = *source;
    Check_Vector(clone->Get_Position(), Vector3(8, 9, 10));
    BOOST_CHECK(clone->Is_Hidden());
    clone->Set_Point_Location(0, Vector3(7, 8, 9));
    Vector3 original_point;
    source->Get_Point_Location(0, original_point);
    Check_Vector(original_point, points[0]);
}

BOOST_AUTO_TEST_CASE(segmented_line_frozen_noise_retains_three_rng_samples_and_chunk_restarts)
{
    struct CameraRestore {
        Graphics::CameraMatrices saved = Graphics::Get_Camera_Matrices();
        ~CameraRestore() { Graphics::Get_Camera_Matrices() = saved; }
    } restore_camera;
    Graphics::Get_Camera_Matrices() = {};
    W3DCamera camera;
    W3DRenderContext info(camera);
    RefCountPtr<W3DSegmentedLineRenderObject> line =
        Create_No_Add_Ref(new W3DSegmentedLineRenderObject);
    std::vector<Vector3> points;
    for (unsigned index = 0; index < 70; ++index)
        points.emplace_back(static_cast<float>(index) * 0.1f - 3.5f, 0, -5);
    line->Set_Points(static_cast<unsigned>(points.size()), points.data());
    line->Set_Width(0.25f);
    line->Set_Subdivision_Levels(1);
    line->Set_LOD_Level(1);
    line->Set_Noise_Amplitude(0.125f);
    line->Set_Merge_Intersections(0);
    line->Set_Freeze_Random(1);

    struct CapturedGeometry {
        std::vector<std::vector<Graphics::PropVertex>> vertices;
        std::vector<std::vector<std::uint32_t>> indices;
    } actual, expected;
    W3DSegmentedLineGeometrySink sink;
    sink.context = &actual;
    sink.submit = [](void *context, const Graphics::PropVertex *vertices, unsigned vertex_count,
        const std::uint32_t *indices, unsigned index_count) {
        auto &capture = *static_cast<CapturedGeometry *>(context);
        capture.vertices.emplace_back(vertices, vertices + vertex_count);
        capture.indices.emplace_back(indices, indices + index_count);
    };
    line->Extract_Geometry(info, sink);

    Graphics::RibbonPipeline pipeline;
    Graphics::RibbonPipelineSettings settings;
    settings.width = 0.25f;
    settings.subdivision_level = 1;
    settings.noise_amplitude = 0.125f;
    settings.merge_intersections = false;
    std::optional<Random3Class> random;
    std::vector<std::size_t> chunk_starts;
    unsigned samples = 0;
    BOOST_REQUIRE(pipeline.Build(points.size(), settings,
        [&](std::size_t index) {
            const auto &point = points[index];
            return Graphics::RibbonPoint{{point.X, point.Y, point.Z}, {1, 1, 1, 1}, 0};
        },
        [&] {
            Vector3 offset;
            const float inverse_max = 1.0f / static_cast<float>(INT_MAX);
            // This original expression is intentional: three conversions of
            // Random3Class, with the active compiler's argument evaluation order.
            offset.Set(*random * inverse_max, *random * inverse_max, *random * inverse_max);
            ++samples;
            return std::array<float, 3>{offset.X, offset.Y, offset.Z};
        },
        [&](std::size_t first_point) {
            chunk_starts.push_back(first_point);
            random.emplace();
        },
        [&](const Graphics::RibbonPipelineChunk &chunk) {
            expected.vertices.emplace_back(chunk.vertices.begin(), chunk.vertices.end());
            expected.indices.emplace_back(chunk.indices.begin(), chunk.indices.end());
        }));
    BOOST_REQUIRE(chunk_starts == (std::vector<std::size_t>{0, 64}));
    BOOST_CHECK_EQUAL(samples, 69u);
    BOOST_REQUIRE_EQUAL(actual.vertices.size(), expected.vertices.size());
    BOOST_CHECK(actual.indices == expected.indices);
    for (std::size_t chunk = 0; chunk < expected.vertices.size(); ++chunk) {
        BOOST_REQUIRE_EQUAL(actual.vertices[chunk].size(), expected.vertices[chunk].size());
        for (std::size_t vertex = 0; vertex < expected.vertices[chunk].size(); ++vertex) {
            const auto &value = actual.vertices[chunk][vertex];
            const auto &reference = expected.vertices[chunk][vertex];
            for (unsigned axis = 0; axis < 3; ++axis)
                BOOST_CHECK_SMALL(value.position[axis] - reference.position[axis], 0.00001f);
            BOOST_CHECK(value.color == reference.color);
            BOOST_CHECK(value.uv == reference.uv);
        }
    }
}

BOOST_AUTO_TEST_CASE(emitter_rotation_matches_game_cached_slerp_and_nonunit_vector_rotation)
{
    const std::array<Quaternion, 4> rotations{
        Quaternion(0, 0, 0, 1), Quaternion(.2f, .3f, .4f, .5f),
        Quaternion(0, 0, -.70710677f, -.70710677f), Quaternion(0, 0, .0001f, 1)};
    for (const auto &first : rotations) for (const auto &second : rotations) {
        SlerpInfoStruct setup;
        Slerp_Setup(first, second, &setup);
        const Graphics::EmitterRotationInterval interval(
            {first.X, first.Y, first.Z, first.W}, {second.X, second.Y, second.Z, second.W});
        for (const float fraction : {0.0f, .125f, .5f, .875f, 1.0f}) {
            const Quaternion expected = Cached_Slerp(first, second, fraction, &setup);
            const auto actual = interval.Sample(fraction);
            const std::array expected_values{expected.X, expected.Y, expected.Z, expected.W};
            for (unsigned axis = 0; axis < 4; ++axis)
                BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(actual[axis]),
                    std::bit_cast<std::uint32_t>(expected_values[axis]));
            const Vector3 vector(2, -3, 7);
            const Vector3 rotated = expected.Rotate_Vector(vector);
            const auto result = Graphics::Rotate_Emitter_Vector(actual, {2, -3, 7});
            const std::array expected_vector{rotated.X, rotated.Y, rotated.Z};
            for (unsigned axis = 0; axis < 3; ++axis)
                BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(result[axis]),
                    std::bit_cast<std::uint32_t>(expected_vector[axis]));
        }
    }
}

BOOST_AUTO_TEST_CASE(emitted_particles_keep_scene_membership_until_their_lifetime_ends)
{
    RecordingScene scene;
    Assets::EmitterAssetDesc description;
    description.emission_rate = 1000;
    description.max_emissions = 2;
    description.lifetime = .1f;
    description.geometry_mode = Assets::EmitterGeometryMode::LineGroupPrism;
    description.color.start = {1, 1, 1, 1};
    description.opacity.start = 1;
    description.size.start = .25f;
    auto source = RefCountPtr<W3DEmitterRenderObject>::Create_No_Add_Ref(
        new W3DEmitterRenderObject(description, nullptr, Graphics::MaterialState::AlphaSprite(), {}, {}));
    source->Add(&scene);
    BOOST_CHECK(source->Is_Stopped());
    source->Start();
    source->On_Frame_Update();
    RefCountPtr<W3DEmitterParticles> particles;
    auto *iterator = scene.Create_Iterator();
    for (iterator->First(); !iterator->Is_Done(); iterator->Next()) {
        auto *object = iterator->Current_Item();
        if (object->Class_ID() == W3DRenderObject::CLASSID_PARTICLEBUFFER)
            particles.Assign_Add_Ref(static_cast<W3DEmitterParticles *>(object));
    }
    scene.Destroy_Iterator(iterator);
    BOOST_REQUIRE(particles.Peek() != nullptr);
    BOOST_CHECK(particles->Is_Force_Visible());
    Graphics::Get_Render_Clock().Update_Logic_Frame_Time(10);
    Graphics::Get_Render_Clock().Sync(true);
    particles->On_Frame_Update();
    BOOST_CHECK(source->Is_Complete());
    AABoxClass bounds;
    particles->Get_Obj_Space_Bounding_Box(bounds);
    BOOST_CHECK_EQUAL(bounds.Extent.X, .25f);
    source->Remove();
    source.Clear();
    BOOST_CHECK(particles->Is_In_Scene());
    BOOST_CHECK(!particles->Is_Complete());
    Graphics::Get_Render_Clock().Update_Logic_Frame_Time(100);
    Graphics::Get_Render_Clock().Sync(true);
    particles->Get_Obj_Space_Bounding_Box(bounds);
    BOOST_CHECK_EQUAL(bounds.Extent.X, 0);
    BOOST_CHECK(particles->Is_Complete());
    particles->On_Frame_Update();
    BOOST_CHECK_EQUAL(scene.release_register_count, 1);
    scene.Remove_All_Render_Objects();
    BOOST_CHECK(!particles->Is_In_Scene());
}

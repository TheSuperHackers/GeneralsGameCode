module;
#define BOOST_TEST_MODULE SceneObjectListTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
export module Graphics.Scene.ObjectList.Tests;
import Graphics.Scene.ObjectList;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
using namespace Graphics;

namespace
{
struct Object final : SceneListMember
{
    explicit Object(unsigned value,unsigned* deaths=nullptr) : value(value),deaths(deaths) {}
    ~Object() override { if (deaths) ++*deaths; }
    void Add_Ref() { ++references; }
    void Release_Ref() { if (--references==0) delete this; }
    unsigned value,references=1;
    unsigned* deaths;
};
}

BOOST_AUTO_TEST_CASE(duplicate_order_and_transferred_references_preserve_borrowed_membership_lifetime)
{
    unsigned deaths=0;
    SceneObjectList<Object> owned;
    SceneObjectList<Object,false> observed;
    auto* object=new Object(7,&deaths);
    BOOST_REQUIRE(owned.Add(object));
    BOOST_CHECK(!owned.Add(object));
    BOOST_REQUIRE(owned.Add_Tail(object,false));
    BOOST_REQUIRE(observed.Add(object));
    BOOST_CHECK_EQUAL(object->references,3u);
    object->Release_Ref();
    auto* transferred=owned.Remove_Head();
    BOOST_REQUIRE(transferred==object);
    BOOST_CHECK_EQUAL(transferred->references,2u);
    BOOST_CHECK_EQUAL(owned.Count(),1u);
    transferred->Release_Ref();
    BOOST_CHECK_EQUAL(deaths,0u);
    BOOST_REQUIRE(owned.Release_Head());
    BOOST_CHECK_EQUAL(deaths,1u);
    BOOST_CHECK(observed.Is_Empty());
    BOOST_CHECK(owned.Is_Empty());
}

BOOST_AUTO_TEST_CASE(growth_and_callback_removal_keep_other_cursors_and_memberships_valid)
{
    std::vector<std::unique_ptr<Object>> objects;
    SceneObjectList<Object,false> list,secondary;
    objects.emplace_back(std::make_unique<Object>(0));
    BOOST_REQUIRE(list.Add_Tail(objects.back().get()));
    SceneObjectList<Object,false>::Cursor cursor(&list);
    for (unsigned i=1;i<4096;++i) {
        objects.emplace_back(std::make_unique<Object>(i));
        BOOST_REQUIRE(list.Add_Tail(objects.back().get()));
        if (i%2==0) BOOST_REQUIRE(secondary.Add_Tail(objects.back().get()));
    }
    unsigned visited=0;
    for (;!cursor.Is_Done();cursor.Next()) {
        auto* current=cursor.Peek_Obj();
        BOOST_REQUIRE_EQUAL(current->value,visited++);
        if (current->value%3==0) objects[current->value].reset();
    }
    BOOST_CHECK_EQUAL(visited,4096u);
    BOOST_CHECK_EQUAL(list.Count(),2730u);
    BOOST_CHECK_EQUAL(secondary.Count(),1365u);
    Object replacement(5000);
    BOOST_REQUIRE(list.Add(&replacement));
    cursor.First();
    BOOST_REQUIRE(cursor.Peek_Obj()==&replacement);
    SceneObjectList<Object,false>::Cursor other(&list);
    cursor.Remove_Current_Object();
    BOOST_CHECK_EQUAL(cursor.Peek_Obj()->value,1u);
    other.Next();
    BOOST_CHECK_EQUAL(other.Peek_Obj()->value,1u);
    BOOST_REQUIRE(list.Add(&replacement)); // Reuse cannot redirect the live cursors.
    BOOST_CHECK_EQUAL(cursor.Peek_Obj()->value,1u);
    cursor.Last();
    const unsigned last=cursor.Peek_Obj()->value;
    objects[last].reset();
    cursor.Prev();
    BOOST_CHECK_LT(cursor.Peek_Obj()->value,last);
}

BOOST_AUTO_TEST_CASE(member_copy_does_not_copy_links_and_collection_destruction_detaches_cursors)
{
    SceneListMember source;
    SceneObjectList<SceneListMember,false> members;
    BOOST_REQUIRE(members.Add(&source));
    SceneListMember copy(source);
    BOOST_CHECK(!members.Contains(&copy));
    BOOST_REQUIRE(members.Add(&copy));
    copy=source;
    BOOST_CHECK_EQUAL(members.Count(),2u);
    auto transient=std::make_unique<SceneObjectList<SceneListMember,false>>();
    BOOST_REQUIRE(transient->Add(&source));
    auto cursor=std::make_unique<SceneObjectList<SceneListMember,false>::Cursor>(transient.get());
    transient.reset();
    BOOST_CHECK(cursor->Is_Done());
    cursor->First(); cursor->Next();
    BOOST_CHECK(cursor->Peek_Obj()==nullptr);
    BOOST_CHECK(members.Contains(&source));
}

BOOST_AUTO_TEST_CASE(scene_list_order_and_reinsertion_preserve_drawn_rgb_and_destination_alpha)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        const auto target=device.Create_Texture({1,1,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth=device.Create_Texture({1,1,1,RHITextureFormat::D32_Float,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        auto& commands=device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,1,1}));
        SceneObjectList<Object> list;
        auto* red=new Object(0); auto* green=new Object(1);
        BOOST_REQUIRE(list.Add_Tail(red)); BOOST_REQUIRE(list.Add_Tail(green));
        std::array<PropVertex,3> vertices{};
        vertices[0].position={-1,-1,0.5f}; vertices[1].position={3,-1,0.5f}; vertices[2].position={-1,3,0.5f};
        PropStyle style; style.depth_test=false; style.depth_write=false;
        style.source_blend=RHIBlendFactor::SourceAlpha;
        style.destination_blend=RHIBlendFactor::InverseSourceAlpha;
        PropParameters parameters; parameters.textured=0;
        parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        for (unsigned pass=0;pass<2;++pass) {
            if (pass) { BOOST_REQUIRE(list.Remove(red)); BOOST_REQUIRE(list.Add_Tail(red)); }
            BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
            for (SceneObjectList<Object>::Cursor cursor(&list);!cursor.Is_Done();cursor.Next()) {
                for (auto& vertex : vertices) vertex.color=cursor.Peek_Obj()->value
                    ? std::array<float,4>{0,1,0,0.5f} : std::array<float,4>{1,0,0,0.5f};
                const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,3>{0,1,2});
                BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
                renderer.Destroy_Mesh(mesh);
            }
            std::array<std::byte,4> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(target,pixels,4));
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[0])-(pass ? 128 : 64),1);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[1])-(pass ? 64 : 128),1);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[2]),0u);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3])-96,1);
        }
        list.Reset_List(); red->Release_Ref(); green->Release_Ref();
        renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}

#define BOOST_TEST_MODULE GraphicsW3DSceneAdapterTests

#include <boost/test/included/unit_test.hpp>
import Graphics.Frame.RenderSettings;

#include "W3DDevice/GameClient/NullRenderObject.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DSceneClass.h"

#include "WWLib/ref_ptr.h"

namespace
{

class RenderContractScene final : public W3DSimpleScene
{
public:
	void Set_Change_Mode_On_Base(bool change_mode)
	{
		m_change_mode_on_base = change_mode;
	}

	int Customized_Call_Count() const
	{
		return m_customized_call_count;
	}

	bool Post_Saw_Texturing_Enabled() const
	{
		return m_post_saw_texturing_enabled;
	}

protected:
	void Customized_Render(W3DRenderContext &) override
	{
		++m_customized_call_count;
		Graphics::Get_Render_Settings().Set_Texturing_Enabled(false);
		if (m_change_mode_on_base && m_customized_call_count == 1)
			Set_Extra_Pass_Polygon_Mode(EXTRA_PASS_DISABLE);
	}

	void Post_Render_Processing(W3DRenderContext &) override
	{
		m_post_saw_texturing_enabled = Graphics::Get_Render_Settings().Is_Texturing_Enabled();
	}

private:
	bool m_change_mode_on_base = false;
	int m_customized_call_count = 0;
	bool m_post_saw_texturing_enabled = false;
};

void Render_Without_Objects(RenderContractScene &scene)
{
	W3DCamera camera;
	W3DRenderContext context(camera);
	scene.Render(context);
}

}

BOOST_AUTO_TEST_CASE(scene_adapter_keeps_native_membership_and_notifications)
{
	W3DSimpleScene scene;
	RefCountPtr<NullRenderObject> object =
		RefCountPtr<NullRenderObject>::Create_No_Add_Ref(new NullRenderObject);

	BOOST_CHECK_EQUAL(scene.Get_Scene_ID(), W3DScene::SCENE_ID_SIMPLE);
	scene.Add_Render_Object(object.Peek());
	BOOST_CHECK(object->Peek_Scene() == &scene);

	W3DSceneIterator *iterator = scene.Create_Iterator(true);
	BOOST_REQUIRE(iterator != nullptr);
	iterator->First();
	BOOST_REQUIRE(!iterator->Is_Done());
	BOOST_CHECK(iterator->Current_Item() == object.Peek());
	iterator->Next();
	BOOST_CHECK(iterator->Is_Done());
	scene.Destroy_Iterator(iterator);

	scene.Remove_Render_Object(object.Peek());
	BOOST_CHECK(object->Peek_Scene() == nullptr);
}

BOOST_AUTO_TEST_CASE(scene_render_restores_texturing_only_for_extra_passes)
{
	Graphics::Get_Render_Settings().Set_Texturing_Enabled(true);
	RenderContractScene disabled_scene;
	Render_Without_Objects(disabled_scene);
	BOOST_CHECK_EQUAL(disabled_scene.Customized_Call_Count(), 1);
	BOOST_CHECK(!disabled_scene.Post_Saw_Texturing_Enabled());
	BOOST_CHECK(!Graphics::Get_Render_Settings().Is_Texturing_Enabled());

	Graphics::Get_Render_Settings().Set_Texturing_Enabled(true);
	RenderContractScene extra_scene;
	extra_scene.Set_Extra_Pass_Polygon_Mode(W3DScene::EXTRA_PASS_LINE);
	Render_Without_Objects(extra_scene);
	BOOST_CHECK_EQUAL(extra_scene.Customized_Call_Count(), 2);
	BOOST_CHECK(extra_scene.Post_Saw_Texturing_Enabled());
	BOOST_CHECK(Graphics::Get_Render_Settings().Is_Texturing_Enabled());
}

BOOST_AUTO_TEST_CASE(scene_render_observes_base_mode_change_before_extra_stage)
{
	Graphics::Get_Render_Settings().Set_Texturing_Enabled(true);
	RenderContractScene scene;
	scene.Set_Extra_Pass_Polygon_Mode(W3DScene::EXTRA_PASS_LINE);
	scene.Set_Change_Mode_On_Base(true);
	Render_Without_Objects(scene);

	BOOST_CHECK_EQUAL(scene.Customized_Call_Count(), 1);
	BOOST_CHECK(scene.Post_Saw_Texturing_Enabled());
	BOOST_CHECK(Graphics::Get_Render_Settings().Is_Texturing_Enabled());
}

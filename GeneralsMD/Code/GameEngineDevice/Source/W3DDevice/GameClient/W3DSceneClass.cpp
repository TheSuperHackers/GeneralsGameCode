import Graphics.Frame.RenderSettings;
#include "W3DDevice/GameClient/W3DSceneClass.h"

#include <array>

#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "WWDebug/wwdebug.h"

#include "WWLib/chunkio.h"
#include "WWMath/lineseg.h"

import Graphics.Frame.AttachmentBindings;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.RenderObjectDrawing;

namespace
{

enum : unsigned
{
	W3DSceneVariablesChunk = 0x00042300,
	W3DSceneAmbientLight = 0,
	W3DScenePolygonMode,
	W3DSceneFogColor,
	W3DSceneFogEnabled,
	W3DSceneFogStart,
	W3DSceneFogEnd,
};

bool Is_Force_Visible(void *, const W3DRenderObject *object)
{
	return object->Is_Force_Visible() != 0;
}

bool Is_Culled(void *, const W3DRenderObject *object, const W3DCamera *camera)
{
	return camera->Cull_Sphere(object->Get_Bounding_Sphere());
}

void Set_Visible(void *, W3DRenderObject *object, bool visible)
{
	object->Set_Visible(visible ? 1 : 0);
}

bool Is_Really_Visible(void *, W3DRenderObject *object)
{
	return object->Is_Really_Visible() != 0;
}

bool Is_Ignoring_LOD_Cost(void *, W3DRenderObject *object)
{
	return object->Is_Ignoring_LOD_Cost();
}

void Prepare_LOD(void *, W3DRenderObject *object, W3DCamera *camera)
{
	object->Prepare_LOD(*camera);
}

void On_Frame_Update(void *, W3DRenderObject *object)
{
	object->On_Frame_Update();
}

Graphics::SceneDrawResult Draw_Object(void *, W3DRenderObject *object,
	W3DRenderContext *context)
{
	return Graphics::Extract_Render_Object_Draw(*object, *context)
		? Graphics::SceneDrawResult::Submitted
		: Graphics::SceneDrawResult::Skipped;
}

bool Describe_Light(void *, const W3DRenderObject *object,
	Graphics::MaterialLightSource &source)
{
	return object->Get_Light_Description(source);
}

void Remove_Object_From_Scene(void *raw, W3DRenderObject *object)
{
	auto *scene = static_cast<W3DSimpleScene *>(raw);
	object->Notify_Removed(scene);
}

void Detach_Release_Object(void *, W3DRenderObject *object)
{
	if (auto *container = object->Get_Container())
		container->Remove_Sub_Object(object);
	else
		object->Remove();
}

class W3DSceneIteratorImpl final : public W3DSceneIterator
{
public:
	explicit W3DSceneIteratorImpl(
		Graphics::SceneObjectList<W3DRenderObject> *list, bool only_visible)
		: m_iterator(list)
	{
		(void)only_visible;
	}

	void First() override
	{
		m_iterator.First();
	}

	void Next() override
	{
		m_iterator.Next();
	}

	bool Is_Done() override
	{
		return m_iterator.Is_Done();
	}

	W3DRenderObject *Current_Item() override
	{
		return m_iterator.Peek_Obj();
	}

private:
	Graphics::SceneObjectList<W3DRenderObject>::Cursor m_iterator;
};

} // namespace

W3DScene::W3DScene()
	: AmbientLight(0.5f, 0.5f, 0.5f),
	  PolyRenderMode(FILL),
	  ExtraPassPolyRenderMode(EXTRA_PASS_DISABLE),
	  FogEnabled(false),
	  FogColor(0.0f, 0.0f, 0.0f),
	  FogStart(0.0f),
	  FogEnd(1000.0f)
{
}

W3DScene::~W3DScene() = default;

void W3DScene::Add_Render_Object(W3DRenderObject *object)
{
	if (object != nullptr)
		object->Notify_Added(this);
}

void W3DScene::Remove_Render_Object(W3DRenderObject *object)
{
	if (object != nullptr)
		object->Notify_Removed(this);
}

void W3DScene::Apply_Pass_Settings() noexcept
{
	switch (PolyRenderMode) {
	case POINT:
		Pass.Set_Polygon_Mode(Graphics::ScenePolygonMode::Point);
		break;
	case LINE:
		Pass.Set_Polygon_Mode(Graphics::ScenePolygonMode::Line);
		break;
	case FILL:
	default:
		Pass.Set_Polygon_Mode(Graphics::ScenePolygonMode::Fill);
		break;
	}

	Pass.Apply_Base_State(Graphics::Get_Scene_Draw_Parameters());
}

void W3DScene::Apply_Extra_Pass_Settings() noexcept
{
	switch (ExtraPassPolyRenderMode) {
	case EXTRA_PASS_LINE:
		Pass.Set_Extra_Pass_Mode(Graphics::SceneExtraPassMode::Line);
		break;
	case EXTRA_PASS_CLEAR_LINE:
		Pass.Set_Extra_Pass_Mode(Graphics::SceneExtraPassMode::ClearLine);
		break;
	case EXTRA_PASS_DISABLE:
	default:
		Pass.Set_Extra_Pass_Mode(Graphics::SceneExtraPassMode::Disabled);
		break;
	}
}

void W3DScene::Apply_Fog_Settings() noexcept
{
	Pass.Set_Fog({FogEnabled, FogStart, FogEnd,
		{FogColor.X, FogColor.Y, FogColor.Z, 1.0f}});
}

void W3DScene::Render(W3DRenderContext &context)
{
	auto &parameters = Graphics::Get_Scene_Draw_Parameters();
	Apply_Pass_Settings();

	// Polygon state is visible to pre-processing. ScenePass applies fog when
	// the first stage executes, matching the original scene ordering.
	Pre_Render_Processing(context);
	Apply_Fog_Settings();
	const bool entered_extra_pass =
		Get_Extra_Pass_Polygon_Mode() != EXTRA_PASS_DISABLE;
	Apply_Extra_Pass_Settings();

	const bool old_texturing_enabled = Graphics::Get_Render_Settings().Is_Texturing_Enabled();
	Pass.Execute(parameters, old_texturing_enabled,
		[&](Graphics::ScenePassInvocation invocation,
			Graphics::SceneDrawParameters &) {
			Graphics::Get_Render_Settings().Set_Texturing_Enabled(invocation.texturing_enabled);
			Customized_Render(context);
			if (entered_extra_pass
				&& invocation.stage == Graphics::ScenePassStage::Base)
				Apply_Extra_Pass_Settings();
			return true;
		},
		[](std::array<float, 4> color) {
			Graphics::Get_Attachment_Bindings().Clear(true, false, color);
		});
	if (entered_extra_pass)
		Graphics::Get_Render_Settings().Set_Texturing_Enabled(old_texturing_enabled);

	Post_Render_Processing(context);
}

void W3DScene::Save(ChunkSaveClass &save)
{
	save.Begin_Chunk(W3DSceneVariablesChunk);
	WRITE_MICRO_CHUNK(save, W3DSceneAmbientLight, AmbientLight);
	WRITE_MICRO_CHUNK(save, W3DScenePolygonMode, PolyRenderMode);
	WRITE_MICRO_CHUNK(save, W3DSceneFogColor, FogColor);
	WRITE_MICRO_CHUNK(save, W3DSceneFogEnabled, FogEnabled);
	WRITE_MICRO_CHUNK(save, W3DSceneFogStart, FogStart);
	WRITE_MICRO_CHUNK(save, W3DSceneFogEnd, FogEnd);
	save.End_Chunk();
}

void W3DScene::Load(ChunkLoadClass &load)
{
	if (!load.Open_Chunk())
		return;

	if (load.Cur_Chunk_ID() == W3DSceneVariablesChunk) {
		while (load.Open_Micro_Chunk()) {
			switch (load.Cur_Micro_Chunk_ID()) {
				READ_MICRO_CHUNK(load, W3DSceneAmbientLight, AmbientLight);
				READ_MICRO_CHUNK(load, W3DScenePolygonMode, PolyRenderMode);
				READ_MICRO_CHUNK(load, W3DSceneFogColor, FogColor);
				READ_MICRO_CHUNK(load, W3DSceneFogEnabled, FogEnabled);
				READ_MICRO_CHUNK(load, W3DSceneFogStart, FogStart);
				READ_MICRO_CHUNK(load, W3DSceneFogEnd, FogEnd);
			}
			load.Close_Micro_Chunk();
		}
	} else {
		WWDEBUG_SAY(("Unhandled Chunk: 0x%X in file: %s line: %d",
			load.Cur_Chunk_ID(), __FILE__, __LINE__));
	}

	load.Close_Chunk();
}

W3DSimpleScene::W3DSimpleScene()
	: Traversal({this, nullptr, &Remove_Object_From_Scene}),
	  RenderList(Traversal.Render_Objects()),
	  UpdateList(Traversal.Update_Objects()),
	  LightList(Traversal.Light_Objects()),
	  ReleaseList(Traversal.Release_Objects())
{
}

W3DSimpleScene::~W3DSimpleScene()
{
	Remove_All_Render_Objects();
}

void W3DSimpleScene::Add_Render_Object(W3DRenderObject *object)
{
	if (object == nullptr)
		return;

	// The notification precedes insertion, as it did in the scene identity
	// layer. Traversal owns the retained collection reference.
	W3DScene::Add_Render_Object(object);
	Traversal.Add_Render_Object(object);
}

void W3DSimpleScene::Remove_Render_Object(W3DRenderObject *object)
{
	if (object == nullptr)
		return;

	// The traversal callback notifies before it releases the collection
	// reference, preserving the legacy removal ordering.
	Traversal.Remove_Render_Object(object);
}

void W3DSimpleScene::Remove_All_Render_Objects()
{
	Traversal.Remove_All_Render_Objects();
	Traversal.Clear_Visibility();
}

void W3DSimpleScene::Register(W3DRenderObject *object, RegType registration)
{
	if (object == nullptr)
		return;

	switch (registration) {
	case ON_FRAME_UPDATE:
		Traversal.Register(object, Graphics::SceneRegistration::On_Frame_Update);
		break;
	case LIGHT:
		Traversal.Register(object, Graphics::SceneRegistration::Light);
		break;
	case RELEASE:
		Traversal.Register(object, Graphics::SceneRegistration::Release);
		break;
	}
}

void W3DSimpleScene::Unregister(W3DRenderObject *object, RegType registration)
{
	if (object == nullptr)
		return;

	switch (registration) {
	case ON_FRAME_UPDATE:
		Traversal.Unregister(object, Graphics::SceneRegistration::On_Frame_Update);
		break;
	case LIGHT:
		Traversal.Unregister(object, Graphics::SceneRegistration::Light);
		break;
	case RELEASE:
		Traversal.Unregister(object, Graphics::SceneRegistration::Release);
		break;
	}
}

void W3DSimpleScene::Set_Ambient_Light(const Vector3 &color)
{
	W3DScene::Set_Ambient_Light(color);
	Traversal.Set_Ambient_Light({color.X, color.Y, color.Z});
}

void W3DSimpleScene::Visibility_Check(W3DCamera *camera)
{
	if (camera == nullptr)
		return;

	Graphics::SceneVisibilityCallbacks<W3DRenderObject, W3DCamera> visibility;
	visibility.is_force_visible = &Is_Force_Visible;
	visibility.is_culled = &Is_Culled;
	visibility.set_visible = &Set_Visible;
	visibility.is_really_visible = &Is_Really_Visible;
	visibility.is_ignoring_lod_cost = &Is_Ignoring_LOD_Cost;
	visibility.prepare_lod = &Prepare_LOD;
	// Check_Visibility publishes the native camera-tagged result.
	Traversal.Check_Visibility(*camera, visibility);
}

float W3DSimpleScene::Compute_Point_Visibility(
	W3DRenderContext &context, const Vector3 &point)
{
	CastResultStruct result;
	LineSegClass ray(context.Camera.Get_Position(), point);
	W3DRayCastQuery query(ray, &result, SCENE_QUERY_PROJECTILE, false, false);

	Graphics::SceneObjectList<W3DRenderObject>::Cursor iterator(&RenderList);
	for (iterator.First(); !iterator.Is_Done(); iterator.Next())
		iterator.Peek_Obj()->Cast_Ray(query);

	return result.Fraction == 1.0f ? 1.0f : 0.0f;
}

void W3DSimpleScene::Customized_Render(W3DRenderContext &context)
{
	if (!Traversal.Is_Visibility_Checked_For(context.Camera)) {
		Traversal.Clear_Visibility();
		Visibility_Check(&context.Camera);
	}

	Traversal.Set_Ambient_Light({AmbientLight.X, AmbientLight.Y, AmbientLight.Z});

	Graphics::SceneVisibilityCallbacks<W3DRenderObject, W3DCamera> visibility;
	visibility.is_force_visible = &Is_Force_Visible;
	visibility.is_culled = &Is_Culled;
	visibility.set_visible = &Set_Visible;
	visibility.is_really_visible = &Is_Really_Visible;
	visibility.is_ignoring_lod_cost = &Is_Ignoring_LOD_Cost;
	visibility.prepare_lod = &Prepare_LOD;

	Graphics::SceneRenderCallbacks<W3DRenderObject, W3DRenderContext> render;
	render.on_frame_update = &On_Frame_Update;
	render.is_really_visible = &Is_Really_Visible;
	render.draw = &Draw_Object;

	Graphics::SceneLightingCallbacks<W3DRenderObject> lighting;
	lighting.describe_light = &Describe_Light;

	Traversal.Render(context.Camera, context, visibility, render, lighting);
}

void W3DSimpleScene::Post_Render_Processing(W3DRenderContext &)
{
	Graphics::SceneReleaseCallbacks<W3DRenderObject> releases;
	releases.detach = &Detach_Release_Object;
	Traversal.Process_Releases(releases);
}

W3DSceneIterator *W3DSimpleScene::Create_Iterator(bool only_visible)
{
	return new W3DSceneIteratorImpl(&RenderList, only_visible);
}

void W3DSimpleScene::Destroy_Iterator(W3DSceneIterator *iterator)
{
	delete iterator;
}


#include <functional>
import Graphics.Frame.RenderClock;
import Graphics.Frame.Runtime;
import Graphics.Frame.RenderSettings;
#include "W3DDevice/GameClient/W3DRenderServices.h"

#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "W3DDevice/GameClient/W3DSceneClass.h"

import Graphics.Frame.AttachmentBindings;
import Graphics.Scene.DrawParameters;
import Graphics.Resources.Textures.CachePolicy;

namespace
{

void Release_Game_Assets()
{
	if (auto *catalog = W3DAssetCatalog::Get_Instance())
		catalog->Free_Assets();
}

void Evict_Unused_Game_Textures()
{
	const auto sync_time = Graphics::Get_Render_Clock().Sync_Time();
	if (auto *catalog = W3DAssetCatalog::Get_Instance())
	{
		catalog->Visit_Textures([sync_time](W3DTextureHandle *texture) {
			if (texture != nullptr)
				Graphics::TextureCachePolicy::Invalidate_Old_Unused_Texture(
					texture->Residency(), sync_time, 0);
		});
	}
}

void Invalidate_Game_Textures()
{
	if (auto *catalog = W3DAssetCatalog::Get_Instance())
		catalog->Visit_Textures([](W3DTextureHandle *texture) {
			if (texture != nullptr)
				texture->Invalidate();
		});
}

} // namespace

bool W3DRenderServices::Initialize()
{
	return Graphics::Get_Render_Services().Initialize();
}

bool W3DRenderServices::Shutdown(Graphics::RenderServiceCallback release_assets)
{
	if (release_assets == nullptr)
		release_assets = &Release_Game_Assets;
	return Graphics::Get_Render_Services().Shutdown(release_assets);
}

bool W3DRenderServices::Begin_Render(bool clear, bool clear_depth,
	const Vector3 &color, float destination_alpha,
	Graphics::RenderServiceCallback resource_progress,
	Graphics::RenderServiceCallback evict_unused_textures)
{
	Graphics::RenderBeginOptions options;
	options.clear = clear;
	options.clear_depth = clear_depth;
	options.clear_value = {color.X, color.Y, color.Z, destination_alpha};
	options.resource_progress = resource_progress;
	options.evict_unused_textures = evict_unused_textures != nullptr
		? evict_unused_textures : &Evict_Unused_Game_Textures;
	return Graphics::Get_Render_Services().Begin_Render(options);
}

bool W3DRenderServices::Render(W3DScene *scene, W3DCamera *camera,
	bool clear, bool clear_depth, const Vector3 &color)
{
	auto &services = Graphics::Get_Render_Services();
	if (!services.Is_Initialized())
		return true;
	if (scene == nullptr || camera == nullptr)
		return false;

	if (clear || clear_depth)
	{
		services.Clear_Current_Attachments(clear, clear_depth,
			{color.X, color.Y, color.Z, 0.0f});
	}

	return Render_Scene_Pass(scene, camera);
}

bool W3DRenderServices::Render_Scene_Pass(W3DScene *scene,
	W3DCamera *camera, const Graphics::RHIViewport *viewport_override)
{
	auto &services = Graphics::Get_Render_Services();
	if (!services.Is_Initialized())
		return true;
	if (scene == nullptr || camera == nullptr
		|| Graphics::Shared_Frame_Device() == nullptr)
		return false;

	camera->On_Frame_Update();
	W3DRenderContext context(*camera);
	camera->Apply();
	if (viewport_override != nullptr
		&& !Graphics::Get_Attachment_Bindings().Set_Viewport(*viewport_override))
		return false;

	// Keep the inherited scene parameters alive through scene traversal and
	// every deferred draw flushed by this pass.
	Graphics::SceneDrawScope draw_scope(Graphics::Get_Scene_Draw_Parameters());
	switch (scene->Get_Polygon_Mode())
	{
	case W3DScene::POINT:
	case W3DScene::FILL:
		Graphics::Get_Scene_Draw_Parameters().wireframe = false;
		break;
	case W3DScene::LINE:
		Graphics::Get_Scene_Draw_Parameters().wireframe = true;
		break;
	}

	scene->Render(context);
	return services.Flush(&context);
}

bool W3DRenderServices::Render(W3DRenderObject &object,
	W3DRenderContext &context)
{
	auto &services = Graphics::Get_Render_Services();
	if (!services.Is_Initialized())
		return true;
	context.Camera.On_Frame_Update();
	context.Camera.Apply();

	Graphics::SceneDrawScope draw_scope(Graphics::Get_Scene_Draw_Parameters());
	Graphics::Get_Scene_Draw_Parameters().wireframe = false;
	object.Render(context);
	return services.Flush(&context);
}

bool W3DRenderServices::Flush(W3DRenderContext &context)
{
	return Graphics::Get_Render_Services().Flush(&context);
}

bool W3DRenderServices::End_Render()
{
	return Graphics::Get_Render_Services().End_Render();
}

bool W3DRenderServices::Invalidate_Textures(
	Graphics::RenderServiceCallback invalidate)
{
	if (invalidate == nullptr)
		invalidate = &Invalidate_Game_Textures;
	return Graphics::Get_Render_Services().Invalidate_Textures(invalidate);
}

void W3DRenderServices::Set_Scene_Draw_Queue_Enabled(bool enabled) noexcept
{
	Graphics::Get_Render_Services().Set_Scene_Draw_Queue_Enabled(enabled);
}

bool W3DRenderServices::Is_Scene_Draw_Queue_Enabled() const noexcept
{
	return Graphics::Get_Render_Services().Is_Scene_Draw_Queue_Enabled();
}

void W3DRenderServices::Set_Sorting_Enabled(bool enabled) noexcept
{
	Graphics::Get_Render_Services().Set_Sorting_Enabled(enabled);
}

bool W3DRenderServices::Is_Initialized() const noexcept
{
	return Graphics::Get_Render_Services().Is_Initialized();
}

bool W3DRenderServices::Is_Rendering() const noexcept
{
	return Graphics::Get_Render_Services().Is_Rendering();
}

unsigned W3DRenderServices::Frame_Count() const noexcept
{
	return Graphics::Get_Render_Services().Frame_Count();
}

bool W3DRenderServices::Is_Reflection_Render_Pass() const noexcept
{
	return Reflection_Pass_Depth() != 0;
}

unsigned &W3DRenderServices::Reflection_Pass_Depth() noexcept
{
	static unsigned depth = 0;
	return depth;
}

W3DRenderServices::ReflectionRenderPassScope::ReflectionRenderPassScope() noexcept
{
	++W3DRenderServices::Reflection_Pass_Depth();
}

W3DRenderServices::ReflectionRenderPassScope::~ReflectionRenderPassScope()
{
	--W3DRenderServices::Reflection_Pass_Depth();
}

Graphics::RenderClock &W3DRenderServices::Clock() const noexcept
{
	return Graphics::Get_Render_Clock();
}

Graphics::RenderSettings &W3DRenderServices::Settings() const noexcept
{
	return Graphics::Get_Render_Settings();
}

W3DRenderServices &Get_W3D_Render_Services() noexcept
{
	static W3DRenderServices services;
	return services;
}

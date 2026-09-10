#pragma once

#include "WWMath/vector3.h"

import Graphics.Frame.RenderServices;
import Graphics.RHI;

class W3DScene;
class W3DCamera;
class W3DRenderContext;
class W3DRenderObject;

// Game-facing translation for the render-service calls that remain in the
// W3D device layer. Frame ownership stays with Graphics.Frame.Runtime;
// scene/object types stay at this adapter boundary.
class W3DRenderServices final
{
public:
	W3DRenderServices() noexcept = default;
	W3DRenderServices(const W3DRenderServices &) = delete;
	W3DRenderServices &operator=(const W3DRenderServices &) = delete;

	bool Initialize();
	bool Shutdown(Graphics::RenderServiceCallback release_assets = nullptr);

	bool Begin_Render(bool clear = false, bool clear_depth = true,
		const Vector3 &color = Vector3(0.0f, 0.0f, 0.0f),
		float destination_alpha = 0.0f,
		Graphics::RenderServiceCallback resource_progress = nullptr,
		Graphics::RenderServiceCallback evict_unused_textures = nullptr);
	bool Render(W3DScene *scene, W3DCamera *camera, bool clear = false,
		bool clear_depth = false, const Vector3 &color = Vector3(0.0f, 0.0f, 0.0f));
	bool Render_Scene_Pass(W3DScene *scene, W3DCamera *camera,
		const Graphics::RHIViewport *viewport_override = nullptr);
	bool Render(W3DRenderObject &object, W3DRenderContext &context);
	bool Flush(W3DRenderContext &context);
	bool End_Render();

	bool Invalidate_Textures(Graphics::RenderServiceCallback invalidate = nullptr);

	void Set_Scene_Draw_Queue_Enabled(bool enabled) noexcept;
	bool Is_Scene_Draw_Queue_Enabled() const noexcept;
	void Set_Sorting_Enabled(bool enabled) noexcept;

	bool Is_Initialized() const noexcept;
	bool Is_Rendering() const noexcept;
	unsigned Frame_Count() const noexcept;
	bool Is_Reflection_Render_Pass() const noexcept;

	class ReflectionRenderPassScope final
	{
	public:
		ReflectionRenderPassScope() noexcept;
		~ReflectionRenderPassScope();
		ReflectionRenderPassScope(const ReflectionRenderPassScope &) = delete;
		ReflectionRenderPassScope &operator=(const ReflectionRenderPassScope &) = delete;
	};

	Graphics::RenderClock &Clock() const noexcept;
	Graphics::RenderSettings &Settings() const noexcept;

private:
	static unsigned &Reflection_Pass_Depth() noexcept;
};

W3DRenderServices &Get_W3D_Render_Services() noexcept;

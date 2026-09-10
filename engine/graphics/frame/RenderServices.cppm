module;

#include <array>
#include <cstdint>

export module Graphics.Frame.RenderServices;

export import Graphics.Frame.RenderClock;
export import Graphics.Frame.RenderSettings;

import Graphics.Frame.AttachmentBindings;
import Graphics.Frame.Runtime;
import Graphics.Resources.Loading.Queue;
import Graphics.Scene.OrderedDraws;
import Graphics.Scene.Props.Submission;

namespace Graphics
{

export using RenderServiceCallback = void (*)();

export struct RenderBeginOptions final
{
	bool clear = false;
	bool clear_depth = true;
	std::array<float, 4> clear_value{0.0f, 0.0f, 0.0f, 0.0f};
	RenderServiceCallback resource_progress = nullptr;
	RenderServiceCallback evict_unused_textures = nullptr;
};

// Frame ownership remains in Graphics.Frame.Runtime. These services prepare
// and finish the render portion inside that already-active GPU frame, while
// keeping resource loading, deferred submission, and renderer state in their
// owning components.
export class RenderServices final
{
public:
	RenderServices() noexcept = default;
	RenderServices(const RenderServices &) = delete;
	RenderServices &operator=(const RenderServices &) = delete;

	bool Initialize()
	{
		if (m_initialized)
			return false;

		Get_Scene_Draw_Queue().Clear();
		if (!Get_Resource_Load_Queue().Start())
			return false;
		Get_Prop_Submission().Clear();
		m_initialized = true;
		return true;
	}

	bool Shutdown(RenderServiceCallback release_assets = nullptr)
	{
		if (!m_initialized)
			return true;

		// Asset owners release their CPU and GPU resources before the load
		// worker is stopped. The callback is supplied by the game adapter.
		if (release_assets != nullptr)
			release_assets();

		const bool queue_shutdown = Get_Resource_Load_Queue().Shutdown();
		Get_Prop_Submission().Clear();
		Get_Scene_Draw_Queue().Clear();
		Get_Scene_Draw_Queue().Set_Enabled(false);
		m_rendering = false;
		m_initialized = false;
		return queue_shutdown;
	}

	bool Is_Initialized() const noexcept
	{
		return m_initialized;
	}

	bool Is_Rendering() const noexcept
	{
		return m_rendering;
	}

	std::uint32_t Frame_Count() const noexcept
	{
		return m_frame_count;
	}

	bool Begin_Render(const RenderBeginOptions &options = {})
	{
		if (!m_initialized)
			return true;
		if (!Frame_Device_Ready())
			return false;

		Get_Resource_Load_Queue().Update(options.resource_progress);
		if (options.evict_unused_textures != nullptr)
			options.evict_unused_textures();

		m_rendering = true;

		if (options.clear || options.clear_depth)
		{
			const auto viewport = Get_Attachment_Bindings().Default().viewport;
			// Clear using the complete default viewport. Camera-specific
			// viewport state is selected only after this preparation step.
			Get_Attachment_Bindings().Set_Viewport(viewport);
			Get_Attachment_Bindings().Clear(options.clear, options.clear_depth,
				options.clear_value);
		}

		// The caller already bracketed this operation with Graphics_Begin_Frame
		// and Graphics_End_Frame. Rebind only the main target selection here.
		if (!Get_Attachment_Bindings().Offscreen()
			&& !Get_Attachment_Bindings().Rebind())
		{
			m_rendering = false;
			return false;
		}

		return true;
	}

	// Clear the currently selected attachments without changing the selection.
	void Clear_Current_Attachments(bool clear_color, bool clear_depth,
		const std::array<float, 4> &value) noexcept
	{
		Get_Attachment_Bindings().Clear(clear_color, clear_depth, value);
	}

	// Flushes in authored order: material submissions, ordered scene draws
	// with a material boundary after each layer, then transparent geometry.
	bool Flush(void *context)
	{
		const bool materials = Get_Prop_Submission().Flush_Materials();
		const bool ordered = Get_Scene_Draw_Queue().Drain(context,
			[] { Get_Prop_Submission().Flush_Materials(); });
		const bool transparent = Get_Prop_Submission().Flush_Transparent();
		return materials && ordered && transparent;
	}

	bool End_Render()
	{
		if (!m_initialized)
			return true;

		const bool flushed = Get_Prop_Submission().Flush_Transparent();
		m_rendering = false;
		++m_frame_count;
		return flushed;
	}

	bool Invalidate_Textures(RenderServiceCallback invalidate)
	{
		const bool drained = Get_Resource_Load_Queue().Drain();
		if (invalidate != nullptr)
			invalidate();
		return drained;
	}

	void Set_Scene_Draw_Queue_Enabled(bool enabled) noexcept
	{
		Get_Scene_Draw_Queue().Set_Enabled(enabled);
	}

	bool Is_Scene_Draw_Queue_Enabled() const noexcept
	{
		return Get_Scene_Draw_Queue().Is_Enabled();
	}

	void Set_Sorting_Enabled(bool enabled) noexcept
	{
		Get_Render_Settings().Set_Sorting_Enabled(enabled);
		// Sorting changes alter the extraction path for mesh submissions. Clear
		// pending material work even when the value did not change, matching
		// the explicit invalidation contract of the setting.
		Get_Prop_Submission().Clear();
	}

private:
	bool m_initialized = false;
	bool m_rendering = false;
	std::uint32_t m_frame_count = 0;
};

export RenderServices &Get_Render_Services() noexcept
{
	static RenderServices services;
	return services;
}

}

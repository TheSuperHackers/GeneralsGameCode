module;

#include <cstdint>

export module Graphics.Frame.RenderSettings;

namespace Graphics
{

export enum class RenderPrelitMode : std::uint8_t
{
	Vertex,
	LightmapMultiPass,
	LightmapMultiTexture,
};

export class RenderSettings final
{
public:
	RenderSettings() noexcept = default;

	bool Is_Texturing_Enabled() const noexcept
	{
		return m_texturing_enabled;
	}

	void Set_Texturing_Enabled(bool enabled) noexcept
	{
		m_texturing_enabled = enabled;
	}

	bool Is_Coloring_Enabled() const noexcept
	{
		return m_coloring_enabled;
	}

	void Set_Coloring_Enabled(bool enabled) noexcept
	{
		m_coloring_enabled = enabled;
	}

	bool Is_Sorting_Enabled() const noexcept
	{
		return m_sorting_enabled;
	}

	void Set_Sorting_Enabled(bool enabled) noexcept
	{
		m_sorting_enabled = enabled;
	}

	bool Is_Munge_Sort_On_Load_Enabled() const noexcept
	{
		return m_munge_sort_on_load;
	}

	void Set_Munge_Sort_On_Load_Enabled(bool enabled) noexcept
	{
		m_munge_sort_on_load = enabled;
	}

	bool Is_Overbright_Modify_On_Load_Enabled() const noexcept
	{
		return m_overbright_modify_on_load;
	}

	void Set_Overbright_Modify_On_Load_Enabled(bool enabled) noexcept
	{
		m_overbright_modify_on_load = enabled;
	}

	RenderPrelitMode Get_Prelit_Mode() const noexcept
	{
		return m_prelit_mode;
	}

	void Set_Prelit_Mode(RenderPrelitMode mode) noexcept
	{
		m_prelit_mode = mode;
	}

	bool Get_Preserve_FPU() const noexcept
	{
		return m_preserve_fpu;
	}

	void Set_Preserve_FPU(bool preserve) noexcept
	{
		m_preserve_fpu = preserve;
	}

private:
	bool m_texturing_enabled = true;
	bool m_coloring_enabled = false;
	bool m_sorting_enabled = true;
	bool m_munge_sort_on_load = false;
	bool m_overbright_modify_on_load = false;
	RenderPrelitMode m_prelit_mode = RenderPrelitMode::LightmapMultiPass;
	bool m_preserve_fpu = false;
};

// The game-facing adapter and scene traversal share this instance.
export RenderSettings &Get_Render_Settings() noexcept
{
	static RenderSettings settings;
	return settings;
}

}

module;

#include <array>
#include <cstdint>

export module Graphics.Scene.Pass;

export import Graphics.Materials.Fog;
export import Graphics.Scene.DrawParameters;

namespace Graphics
{

// A scene pass gives the adapter a stable point at which to translate scene
// state into material inputs. The extra stage deliberately reports whether
// texturing is enabled; the game adapter applies that generic material choice
// while the pass itself remains independent of a backend or texture class.
export enum class ScenePassStage : std::uint8_t
{
	Base,
	Extra
};

export struct ScenePassInvocation final
{
	ScenePassStage stage = ScenePassStage::Base;
	bool texturing_enabled = true;
};

export enum class ScenePolygonMode : std::uint8_t
{
	Point,
	Line,
	Fill
};

export enum class SceneExtraPassMode : std::uint8_t
{
	Disabled,
	Line,
	ClearLine
};

// Owns scene-level state and invokes the caller's draw operation for each
// authored stage. The caller supplies clearing because attachment ownership
// belongs to the frame subsystem.
export class ScenePass final
{
public:
	ScenePass() noexcept
		: m_fog{false, 0.0f, 1000.0f, {0.0f, 0.0f, 0.0f, 1.0f}}
	{
	}

	void Set_Fog(SceneFog fog) noexcept
	{
		m_fog = fog;
	}

	const SceneFog &Fog() const noexcept
	{
		return m_fog;
	}

	void Set_Fog_Enable(bool enabled) noexcept
	{
		m_fog.enabled = enabled;
	}

	bool Get_Fog_Enable() const noexcept
	{
		return m_fog.enabled;
	}

	void Set_Fog_Color(std::array<float, 3> color) noexcept
	{
		m_fog.color = {color[0], color[1], color[2], 1.0f};
	}

	std::array<float, 3> Fog_Color() const noexcept
	{
		return {m_fog.color[0], m_fog.color[1], m_fog.color[2]};
	}

	void Set_Fog_Range(float start, float end) noexcept
	{
		m_fog.start = start;
		m_fog.end = end;
	}

	void Get_Fog_Range(float &start, float &end) const noexcept
	{
		start = m_fog.start;
		end = m_fog.end;
	}

	void Set_Polygon_Mode(ScenePolygonMode mode) noexcept
	{
		m_polygon_mode = mode;
	}

	ScenePolygonMode Polygon_Mode() const noexcept
	{
		return m_polygon_mode;
	}

	void Set_Extra_Pass_Mode(SceneExtraPassMode mode) noexcept
	{
		m_extra_pass_mode = mode;
	}

	SceneExtraPassMode Extra_Pass_Mode() const noexcept
	{
		return m_extra_pass_mode;
	}

	// The outer render scope applies polygon mode before it invokes scene
	// pre-processing. Fog is applied by Execute after that callback, matching
	// the authored scene ordering.
	void Apply_Base_State(SceneDrawParameters &parameters) const noexcept
	{
		parameters.wireframe = m_polygon_mode == ScenePolygonMode::Line;
	}

	template<class Render, class Clear>
	bool Execute(SceneDrawParameters &parameters, bool texturing_enabled,
		Render &&render, Clear &&clear) const
	{
		parameters.fog = m_fog;

		const ScenePassInvocation base{ScenePassStage::Base, texturing_enabled};
		if (m_extra_pass_mode == SceneExtraPassMode::Disabled)
			return render(base, parameters);

		// The base stage is always submitted, even when the callback reports a
		// failed draw. This keeps the extra pass ordering observable to callers.
		parameters.depth_bias = 0;
		bool success = render(base, parameters);

		// The base callback can change the owning scene's extra-pass mode. The
		// caller refreshes this pass after that callback, so read the current
		// mode before deciding whether to submit another stage.
		if (m_extra_pass_mode == SceneExtraPassMode::Disabled)
			return success;

		if (m_extra_pass_mode == SceneExtraPassMode::ClearLine)
			clear({0.0f, 0.0f, 0.0f, 0.0f});

		parameters.wireframe = true;
		parameters.depth_bias = 7;
		const ScenePassInvocation extra{ScenePassStage::Extra, false};
		if (!render(extra, parameters))
			success = false;
		return success;
	}

	template<class Render>
	bool Execute(SceneDrawParameters &parameters, bool texturing_enabled,
		Render &&render) const
	{
		return Execute(parameters, texturing_enabled, render,
			[](std::array<float, 4>) {});
	}

private:
	SceneFog m_fog;
	ScenePolygonMode m_polygon_mode = ScenePolygonMode::Fill;
	SceneExtraPassMode m_extra_pass_mode = SceneExtraPassMode::Disabled;
};

}

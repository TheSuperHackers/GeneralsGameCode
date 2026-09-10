module;

#include <array>
#include <bit>
#include <cstdint>

export module Graphics.Scene.RenderObjectState;

export import Graphics.Scene.AffineTransform;

namespace Graphics
{

// These flags describe renderer-visible object state. Scene query masks stay
// in the game adapter because their meaning belongs to game collision code.
export enum class RenderObjectFlags : std::uint32_t
{
	None = 0,
	Visible = 1u << 0,
	NotHidden = 1u << 1,
	NotAnimationHidden = 1u << 2,
	ForceVisible = 1u << 3,
	Translucent = 1u << 4,
	IgnoreLodCost = 1u << 5,
	SubObjectsMatchLod = 1u << 6,
	SubObjectTransformsDirty = 1u << 7,
	Alpha = 1u << 8,
	Additive = 1u << 9,
	SelfShadowed = 1u << 10
};

export constexpr RenderObjectFlags operator|(
	RenderObjectFlags left, RenderObjectFlags right) noexcept
{
	return static_cast<RenderObjectFlags>(
		static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}

export constexpr RenderObjectFlags operator&(
	RenderObjectFlags left, RenderObjectFlags right) noexcept
{
	return static_cast<RenderObjectFlags>(
		static_cast<std::uint32_t>(left) & static_cast<std::uint32_t>(right));
}

export constexpr RenderObjectFlags operator~(RenderObjectFlags value) noexcept
{
	return static_cast<RenderObjectFlags>(~static_cast<std::uint32_t>(value));
}

export constexpr RenderObjectFlags &operator|=(
	RenderObjectFlags &left, RenderObjectFlags right) noexcept
{
	left = left | right;
	return left;
}

export constexpr bool Has_Render_Object_Flag(
	RenderObjectFlags flags, RenderObjectFlags flag) noexcept
{
	return (flags & flag) == flag;
}

// State shared by game-facing render objects and graphics extraction. Parent
// ownership, scene membership, and persistence deliberately live elsewhere.
export class RenderObjectState final
{
public:
	RenderObjectState() noexcept
		: m_flags(RenderObjectFlags::NotHidden | RenderObjectFlags::NotAnimationHidden)
	{
	}

	const RenderTransform &Transform() const noexcept
	{
		return m_transform;
	}

	bool Is_Transform_Identity() const noexcept
	{
		return m_transform_identity;
	}

	void Set_Transform(const RenderTransform &transform) noexcept
	{
		m_transform = transform;
		m_transform_identity = Is_Bitwise_Affine_Identity(transform);
	}

	void Set_Position(const std::array<float, 3> &position) noexcept
	{
		m_transform.matrix[3] = position[0];
		m_transform.matrix[7] = position[1];
		m_transform.matrix[11] = position[2];
		m_transform_identity = Is_Bitwise_Affine_Identity(m_transform);
	}

	std::array<float, 3> Position() const noexcept
	{
		return {
			m_transform.matrix[3],
			m_transform.matrix[7],
			m_transform.matrix[11]
		};
	}

	void Set_Transform_Identity(bool identity) noexcept
	{
		m_transform_identity = identity;
	}

	void Set_Flags(RenderObjectFlags flags) noexcept
	{
		m_flags = flags;
	}

	RenderObjectFlags Flags() const noexcept
	{
		return m_flags;
	}

	void Set_Flag(RenderObjectFlags flag, bool enabled) noexcept
	{
		if (enabled)
			m_flags |= flag;
		else
			m_flags = m_flags & ~flag;
	}

	bool Has_Flag(RenderObjectFlags flag) const noexcept
	{
		return Has_Render_Object_Flag(m_flags, flag);
	}

	float Object_Scale() const noexcept
	{
		return m_object_scale;
	}

	void Set_Object_Scale(float scale) noexcept
	{
		m_object_scale = scale;
	}

	float Native_Screen_Size() const noexcept
	{
		return m_native_screen_size;
	}

	void Set_Native_Screen_Size(float screen_size) noexcept
	{
		m_native_screen_size = screen_size;
	}

	private:
	static bool Is_Bitwise_Affine_Identity(const RenderTransform &transform) noexcept
	{
		constexpr float zero = 0.0f;
		constexpr float one = 1.0f;
		const std::array<float, 12> expected{
			one, zero, zero, zero,
			zero, one, zero, zero,
			zero, zero, one, zero
		};

		for (std::size_t index = 0; index < expected.size(); ++index) {
			if (std::bit_cast<std::uint32_t>(transform.matrix[index])
				!= std::bit_cast<std::uint32_t>(expected[index]))
				return false;
		}
		return true;
	}

	RenderTransform m_transform = Affine_Identity();
	RenderObjectFlags m_flags;
	float m_object_scale = 1.0f;
	float m_native_screen_size = 1.0f;
	// The historical base constructor initialized its identity cache lazily;
	// an untouched object therefore reports false until a transform setter or
	// a dirty-container refresh evaluates the matrix.
	bool m_transform_identity = false;
};

}

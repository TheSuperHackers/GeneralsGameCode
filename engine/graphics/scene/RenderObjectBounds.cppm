module;

#include <array>
#include <cmath>

export module Graphics.Scene.RenderObjectBounds;

export import Graphics.Scene.RenderScene;

namespace Graphics
{

// RenderBounds is the common sphere representation already consumed by the
// graphics scene. Reuse it here so object extraction and scene storage cannot
// drift in layout or transform behavior.
export using RenderObjectSphere = RenderBounds;

export struct RenderObjectBox final
{
	std::array<float, 3> center{};
	std::array<float, 3> extent{};
};

export struct RenderObjectBounds final
{
	RenderObjectSphere sphere{};
	RenderObjectBox box{};
};

export RenderObjectSphere Transform_Render_Object_Sphere(
	const RenderTransform &transform,
	const RenderObjectSphere &sphere,
	float object_scale) noexcept
{
	const auto &matrix = transform.matrix;
	RenderObjectSphere result;
	result.center = {
		matrix[0] * sphere.center[0]
			+ matrix[1] * sphere.center[1]
			+ matrix[2] * sphere.center[2]
			+ matrix[3],
		matrix[4] * sphere.center[0]
			+ matrix[5] * sphere.center[1]
			+ matrix[6] * sphere.center[2]
			+ matrix[7],
		matrix[8] * sphere.center[0]
			+ matrix[9] * sphere.center[1]
			+ matrix[10] * sphere.center[2]
			+ matrix[11]
	};
	// ObjectScale historically affects the sphere only. Preserve its signed
	// multiplication because callers may use a negative authored scale.
	result.radius = object_scale * sphere.radius;
	return result;
}

export RenderObjectBox Transform_Render_Object_Box(
	const RenderTransform &transform,
	const RenderObjectBox &box) noexcept
{
	const auto &matrix = transform.matrix;
	RenderObjectBox result;
	// Keep the legacy center/extent operation's accumulation order. Besides
	// matching floating-point rounding, fabs is applied to each product so a
	// signed authored extent cannot change the resulting extent's sign.
	for (std::size_t row = 0; row < 3; ++row) {
		result.center[row] = matrix[row * 4 + 3];
		result.extent[row] = 0.0f;
		for (std::size_t column = 0; column < 3; ++column) {
			result.center[row] += matrix[row * 4 + column] * box.center[column];
			result.extent[row] += std::fabs(matrix[row * 4 + column] * box.extent[column]);
		}
	}
	return result;
}

export RenderObjectBounds Transform_Render_Object_Bounds(
	const RenderTransform &transform,
	const RenderObjectBounds &bounds,
	float object_scale) noexcept
{
	return {
		Transform_Render_Object_Sphere(transform, bounds.sphere, object_scale),
		Transform_Render_Object_Box(transform, bounds.box)
	};
}

// The cache stores only world-space values. Local bounds remain owned by the
// object that knows how its asset is represented and can be invalidated after
// a geometry edit.
export class RenderObjectBoundsCache final
{
public:
	bool Is_Valid() const noexcept
	{
		return m_valid;
	}

	void Invalidate() noexcept
	{
		m_valid = false;
	}

	void Update(
		const RenderTransform &transform,
		const RenderObjectBounds &local_bounds,
		float object_scale) noexcept
	{
		m_world_bounds = Transform_Render_Object_Bounds(transform, local_bounds, object_scale);
		m_valid = true;
	}

	const RenderObjectBounds &World_Bounds() const noexcept
	{
		return m_world_bounds;
	}

	const RenderObjectSphere &World_Sphere() const noexcept
	{
		return m_world_bounds.sphere;
	}

	const RenderObjectBox &World_Box() const noexcept
	{
		return m_world_bounds.box;
	}

private:
	RenderObjectBounds m_world_bounds{};
	bool m_valid = false;
};

}

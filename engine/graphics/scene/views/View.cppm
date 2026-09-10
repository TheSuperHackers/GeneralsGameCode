module;

#include <array>
#include <cmath>
#include <cstddef>

export module Graphics.Scene.Views.View;

namespace Graphics
{

export struct Matrix4x4 final
{
	std::array<float, 16> values{};

	static constexpr Matrix4x4 Identity() noexcept
	{
		Matrix4x4 matrix{};
		matrix.values[0] = 1.0f;
		matrix.values[5] = 1.0f;
		matrix.values[10] = 1.0f;
		matrix.values[15] = 1.0f;
		return matrix;
	}

	constexpr float operator()(std::size_t row, std::size_t column) const noexcept
	{
		return values[row * 4 + column];
	}
};

export struct Vector3 final
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

export struct Viewport final
{
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	float min_depth = 0.0f;
	float max_depth = 1.0f;
};

export struct FrustumPlane final
{
	Vector3 normal{};
	float distance = 0.0f;
};

export struct Frustum final
{
	FrustumPlane left{};
	FrustumPlane right{};
	FrustumPlane bottom{};
	FrustumPlane top{};
	FrustumPlane near_plane{};
	FrustumPlane far_plane{};
};

export Matrix4x4 Compose_Matrices(const Matrix4x4 &left, const Matrix4x4 &right) noexcept
{
	Matrix4x4 result{};
	for (std::size_t row = 0; row < 4; ++row) {
		for (std::size_t column = 0; column < 4; ++column) {
			for (std::size_t element = 0; element < 4; ++element)
				result.values[row * 4 + column] += left(row, element) * right(element, column);
		}
	}
	return result;
}

export struct View final
{
	Matrix4x4 view_matrix{};
	Matrix4x4 projection_matrix{};
	Vector3 position{};
	Viewport viewport{};
	Frustum frustum{};

	View() noexcept = default;

	View(const Matrix4x4 &view, const Matrix4x4 &projection, Vector3 camera_position, Viewport view_port) noexcept
		: view_matrix(view),
		  projection_matrix(projection),
		  position(camera_position),
		  viewport(view_port)
	{
		Derive_Frustum();
	}

	void Derive_Frustum() noexcept
	{
		const Matrix4x4 view_projection = Compose_Matrices(projection_matrix, view_matrix);

		frustum.left = Make_Plane(
			view_projection(3, 0) + view_projection(0, 0),
			view_projection(3, 1) + view_projection(0, 1),
			view_projection(3, 2) + view_projection(0, 2),
			view_projection(3, 3) + view_projection(0, 3));
		frustum.right = Make_Plane(
			view_projection(3, 0) - view_projection(0, 0),
			view_projection(3, 1) - view_projection(0, 1),
			view_projection(3, 2) - view_projection(0, 2),
			view_projection(3, 3) - view_projection(0, 3));
		frustum.bottom = Make_Plane(
			view_projection(3, 0) + view_projection(1, 0),
			view_projection(3, 1) + view_projection(1, 1),
			view_projection(3, 2) + view_projection(1, 2),
			view_projection(3, 3) + view_projection(1, 3));
		frustum.top = Make_Plane(
			view_projection(3, 0) - view_projection(1, 0),
			view_projection(3, 1) - view_projection(1, 1),
			view_projection(3, 2) - view_projection(1, 2),
			view_projection(3, 3) - view_projection(1, 3));
		frustum.near_plane = Make_Plane(
			view_projection(3, 0) + view_projection(2, 0),
			view_projection(3, 1) + view_projection(2, 1),
			view_projection(3, 2) + view_projection(2, 2),
			view_projection(3, 3) + view_projection(2, 3));
		frustum.far_plane = Make_Plane(
			view_projection(3, 0) - view_projection(2, 0),
			view_projection(3, 1) - view_projection(2, 1),
			view_projection(3, 2) - view_projection(2, 2),
			view_projection(3, 3) - view_projection(2, 3));
	}

private:
	static FrustumPlane Make_Plane(float x, float y, float z, float distance) noexcept
	{
		const float length_squared = x * x + y * y + z * z;
		if (length_squared == 0.0f)
			return {};

		const float inverse_length = 1.0f / std::sqrt(length_squared);
		return {
			{x * inverse_length, y * inverse_length, z * inverse_length},
			distance * inverse_length
		};
	}
};

}

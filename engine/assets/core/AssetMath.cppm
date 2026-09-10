module;

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

export module Assets.Math;

namespace Assets
{

export struct Vector2f final
{
	float x = 0.0f;
	float y = 0.0f;
};

export struct Vector3f final
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

export struct Color4f final
{
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;

	constexpr std::array<float,4> To_Array() const noexcept { return {r,g,b,a}; }
};

// Hue is expressed in degrees; a negative hue marks a monochrome color.
export Vector3f RGB_To_HSV(Vector3f rgb) noexcept
{
	const float high = std::max({rgb.x, rgb.y, rgb.z});
	const float low = std::min({rgb.x, rgb.y, rgb.z});
	const float delta = high - low;
	Vector3f hsv{-1, high != 0 ? delta / high : 0, high};
	if (hsv.y == 0) return hsv;
	hsv.x = rgb.x == high ? (rgb.y - rgb.z) / delta
		: rgb.y == high ? 2 + (rgb.z - rgb.x) / delta : 4 + (rgb.x - rgb.y) / delta;
	hsv.x *= 60;
	if (hsv.x < 0) hsv.x += 360;
	return hsv;
}

export Vector3f HSV_To_RGB(Vector3f hsv) noexcept
{
	if (hsv.y == 0) return {hsv.z, hsv.z, hsv.z};
	float hue = std::fmod(hsv.x, 360.0f);
	if (hue < 0) hue += 360;
	if (!std::isfinite(hue)) return {};
	const float sector = hue / 60;
	const int index = int(sector);
	const float fraction = sector - float(index);
	const float p = hsv.z * (1 - hsv.y), q = hsv.z * (1 - hsv.y * fraction);
	const float t = hsv.z * (1 - hsv.y * (1 - fraction));
	switch (index) {
	case 0: return {hsv.z, t, p};
	case 1: return {q, hsv.z, p};
	case 2: return {p, hsv.z, t};
	case 3: return {p, q, hsv.z};
	case 4: return {t, p, hsv.z};
	default: return {hsv.z, p, q};
	}
}

export Vector3f Shift_Color_HSV(Vector3f rgb, Vector3f shift) noexcept
{
	Vector3f hsv = RGB_To_HSV(rgb);
	if (hsv.x >= 0) { hsv.x += shift.x; hsv.y += shift.y; }
	hsv.y = std::clamp(hsv.y, 0.0f, 1.0f);
	hsv.z = std::clamp(hsv.z + shift.z, 0.0f, 1.0f);
	return HSV_To_RGB(hsv);
}

export Color4f Shift_Color_HSV(Color4f color, Vector3f shift) noexcept
{
	const auto rgb = Shift_Color_HSV(Vector3f{color.r, color.g, color.b}, shift);
	return {rgb.x, rgb.y, rgb.z, color.a};
}

export Color4f Color_From_ARGB(std::uint32_t color) noexcept
{
	return {float((color >> 16) & 255) / 255, float((color >> 8) & 255) / 255,
		float(color & 255) / 255, float(color >> 24) / 255};
}

export std::uint32_t Color_To_ARGB(Color4f color) noexcept
{
	const auto byte = [](float channel) -> std::uint32_t {
		return std::isfinite(channel) ? std::uint32_t(std::clamp(channel, 0.0f, 1.0f) * 255 + 0.5f) : 0;
	};
	return (byte(color.a) << 24) | (byte(color.r) << 16) | (byte(color.g) << 8) | byte(color.b);
}

export std::uint32_t Shift_Color_ARGB(std::uint32_t color, Vector3f shift) noexcept
{
	return Color_To_ARGB(Shift_Color_HSV(Color_From_ARGB(color), shift));
}

export struct Bounds3f final
{
	Vector3f minimum{};
	Vector3f maximum{};

	bool Is_Valid() const noexcept;
};

}

namespace Assets
{

bool Bounds3f::Is_Valid() const noexcept
{
	return std::isfinite(minimum.x) && std::isfinite(minimum.y) && std::isfinite(minimum.z) &&
		std::isfinite(maximum.x) && std::isfinite(maximum.y) && std::isfinite(maximum.z) &&
		minimum.x <= maximum.x && minimum.y <= maximum.y && minimum.z <= maximum.z;
}

}

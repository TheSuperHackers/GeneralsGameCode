module;

#include <array>

export module Graphics.Scene.Lighting.State;

export import Assets.Lights;
export import Graphics.Scene.Lighting;
export import Graphics.Scene.Lighting.Local;
export import Graphics.Scene.RenderScene;

namespace Graphics
{

// Authored values remain in the asset representation so the game adapter and
// the W3D codec share one payload. The cosine is the runtime cache retained by
// the old light object after setting or loading its spot angle.
export struct LightState final
{
	Assets::LightAssetDesc authored{};
	float spot_angle_cosine = 0.707f;
};

// Convert authored light values and an affine scene transform into the
// material lighting input. Translation and orientation are extracted here so
// callers do not expose their native matrix type to graphics.
export MaterialLightSource Make_Material_Light(
	const LightState &light, const RenderTransform &transform) noexcept
{
	const auto &authored = light.authored;
	const auto &matrix = transform.matrix;

	MaterialLightSource result;
	switch (authored.type) {
	case Assets::LightType::Directional:
		result.type = RenderLightType::Directional;
		break;
	case Assets::LightType::Spot:
		result.type = RenderLightType::Spot;
		break;
	case Assets::LightType::Point:
	default:
		result.type = RenderLightType::Point;
		break;
	}

	result.position = {matrix[3], matrix[7], matrix[11]};
	if (result.type == RenderLightType::Spot) {
		const auto &direction = authored.spot_direction;
		result.direction = {
			matrix[0] * direction.x + matrix[1] * direction.y + matrix[2] * direction.z,
			matrix[4] * direction.x + matrix[5] * direction.y + matrix[6] * direction.z,
			matrix[8] * direction.x + matrix[9] * direction.y + matrix[10] * direction.z};
	} else {
		// WW3D2 lights point along negative local Z. Keep the affine basis
		// length intact; LocalLighting applies the same authored convention.
		result.direction = {-matrix[2], -matrix[6], -matrix[10]};
	}
	result.ambient = {authored.ambient.x, authored.ambient.y, authored.ambient.z};
	result.diffuse = {authored.diffuse.x, authored.diffuse.y, authored.diffuse.z};
	result.intensity = authored.intensity;
	result.attenuation_start = authored.far_attenuation_start;
	result.attenuation_end = authored.far_attenuation_end;
	result.attenuate = authored.far_attenuation_enabled;
	result.cone_cosine = light.spot_angle_cosine;
	return result;
}

}

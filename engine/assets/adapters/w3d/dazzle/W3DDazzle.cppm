module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <utility>
export module Assets.Adapters.W3D.Dazzle;
import Assets.Dazzles;
import Assets.Adapters.W3D.Chunks;

namespace Assets::W3D {
export inline constexpr std::uint32_t W3DChunkDazzle = 0x00000900u;

export struct W3DDazzleReference final {
    std::string name;
    std::string type;
};
export bool W3DRead_Dazzle(W3DByteSpan bytes, W3DDazzleReference& result) {
    W3DDazzleReference next;
    if (!W3DVisit_Chunks(bytes, [&](const W3DChunkView& chunk) {
        if (chunk.id != 0x901 && chunk.id != 0x902) return true;
        if (std::find(chunk.payload.begin(), chunk.payload.end(), std::byte{}) == chunk.payload.end()) return false;
        (chunk.id == 0x901 ? next.name : next.type) = W3DRead_String(chunk.payload);
        return true;
    })) return false;
    result = std::move(next);
    return true;
}

// The source provides ordered INI entries and typed values. Archive IO and the
// application's INI implementation stay outside the asset description.
export template<class Reader>
bool W3DRead_Dazzle_Definitions(const Reader& source, DazzleDefinitions& result) {
    DazzleDefinitions next;
    for (const auto& name : source.List("Lensflares_List")) {
        LensFlareDefinition definition;
        definition.name = name;
        definition.texture = source.String(name, "TextureName");
        const int count = source.Integer(name, "FlareCount", 0);
        if (count < 0) return false;
        definition.sprites.resize(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            const auto suffix = std::to_string(i + 1);
            auto& sprite = definition.sprites[i];
            sprite.location = source.Float(name, "FlareLocation" + suffix, sprite.location);
            sprite.size = source.Float(name, "FlareSize" + suffix, sprite.size);
            sprite.color = source.Vector3(name, "FlareColor" + suffix, sprite.color);
            sprite.uv = source.Vector4(name, "FlareUV" + suffix, sprite.uv);
            if (!std::isfinite(sprite.location) || !std::isfinite(sprite.size)) return false;
            for (const auto value : sprite.color) if (!std::isfinite(value)) return false;
            for (const auto value : sprite.uv) if (!std::isfinite(value)) return false;
        }
        next.lens_flares.push_back(std::move(definition));
    }
    for (const auto& name : source.List("Dazzles_List")) {
        DazzleDefinition definition;
        definition.name = name;
        definition.primary_texture = source.String(name, "DazzleTextureName");
        definition.halo_texture = source.String(name, "HaloTextureName");
        definition.lens_flare = source.String(name, "LensflareName");
        const auto scalar = [&](const char* key, float& value) { value = source.Float(name, key, value); };
        scalar("HaloIntensity", definition.halo_intensity);
        scalar("HaloIntensityPow", definition.halo_intensity_power);
        scalar("HaloScaleX", definition.halo_scale[0]);
        scalar("HaloScaleY", definition.halo_scale[1]);
        scalar("DazzleArea", definition.area);
        scalar("DazzleDirectionArea", definition.direction_area);
        scalar("DazzleIntensity", definition.intensity);
        scalar("DazzleIntensityPow", definition.intensity_power);
        scalar("DazzleSizePow", definition.size_power);
        scalar("DazzleScaleX", definition.scale[0]);
        scalar("DazzleScaleY", definition.scale[1]);
        scalar("FadeoutStart", definition.fade_start);
        scalar("FadeoutEnd", definition.fade_end);
        scalar("HistoryWeight", definition.history_weight);
        scalar("Radius", definition.radius);
        scalar("BlinkPeriod", definition.blink_period);
        scalar("BlinkOnTime", definition.blink_on_time);
        definition.use_camera_translation = source.Integer(name, "UseCameraTranslation", 1) != 0;
        definition.direction = source.Vector3(name, "DazzleDirection", definition.direction);
        definition.color = source.Vector3(name, "DazzleColor", definition.color);
        definition.halo_color = source.Vector3(name, "HaloColor", definition.halo_color);
        for (const float value : {definition.halo_intensity, definition.halo_intensity_power,
            definition.halo_scale[0], definition.halo_scale[1], definition.area, definition.direction_area,
            definition.intensity, definition.intensity_power, definition.size_power, definition.scale[0],
            definition.scale[1], definition.fade_start, definition.fade_end, definition.history_weight,
            definition.radius, definition.blink_period, definition.blink_on_time})
            if (!std::isfinite(value)) return false;
        for (const auto value : definition.direction) if (!std::isfinite(value)) return false;
        for (const auto value : definition.color) if (!std::isfinite(value)) return false;
        for (const auto value : definition.halo_color) if (!std::isfinite(value)) return false;
        if (definition.area <= 0 || definition.history_weight < 0) return false;
        // The former INI reader discarded Normalize()'s returned copy. Keep
        // authored direction magnitude because it affects the angular envelope.
        next.dazzles.push_back(std::move(definition));
    }
    result = std::move(next);
    return true;
}
}

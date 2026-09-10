module;

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

export module Assets.Adapters.W3D.RequestPolicy;

namespace Assets::W3D
{

// W3D names are intentionally kept in their authored spelling here.  The
// catalog uses these paths as source requests, while the asset caches apply
// their own canonical identity rules after decoding.
export enum class W3DRequestKind
{
	Model,
	Animation,
	Skeleton
};

export struct W3DRequestPaths final
{
	std::string primary;
	std::string parent_directory;
};

// The original W3D catalog used the first dot in a model identity to select
// its source file and the complete suffix after that dot for an animation.
// Keep that detail separate from generic asset-name canonicalization.
export std::optional<std::string> W3D_Request_Filename(
	W3DRequestKind kind, std::string_view name)
{
	if (name.empty())
		return std::nullopt;

	switch (kind) {
	case W3DRequestKind::Model: {
		const std::size_t separator = name.find('.');
		const std::string_view root = separator == std::string_view::npos
			? name
			: name.substr(0, separator);
		return std::string(root) + ".w3d";
	}
	case W3DRequestKind::Animation: {
		const std::size_t separator = name.find('.');
		if (separator == std::string_view::npos)
			return std::nullopt;
		return std::string(name.substr(separator + 1)) + ".w3d";
	}
	case W3DRequestKind::Skeleton:
		return std::string(name) + ".w3d";
	}

	return std::nullopt;
}

export std::optional<W3DRequestPaths> W3D_Make_Request_Paths(
	W3DRequestKind kind, std::string_view name)
{
	const auto filename = W3D_Request_Filename(kind, name);
	if (!filename)
		return std::nullopt;

	return W3DRequestPaths{*filename, "..\\" + *filename};
}

// Animation names are remembered after a failed load.  A remembered miss
// suppresses both another source request and another report until the cache
// is reset; model and skeleton lookup do not use this animation-only gate.
export constexpr bool W3D_Should_Attempt_Animation_Load(
	bool load_on_demand, bool missing_cached) noexcept
{
	return load_on_demand && !missing_cached;
}

}

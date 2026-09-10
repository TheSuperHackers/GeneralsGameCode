module;

#include <cstdint>
#include <cstddef>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

export module Assets.Identity;

namespace Assets
{

export enum class AssetType : std::uint8_t
{
	Model,
	Texture,
	Material,
	Mesh,
	Skeleton,
	Animation,
	Font
};

export struct AssetIdentity final
{
	AssetType type = AssetType::Model;
	std::string canonical_name;

	friend bool operator==(const AssetIdentity &left, const AssetIdentity &right) noexcept
	{
		return left.type == right.type && left.canonical_name == right.canonical_name;
	}
};

export std::string Canonicalize_Asset_Name(std::string_view name);

// These comparisons preserve the byte-oriented, current C character locale
// matching used by the legacy asset consumers without applying path
// canonicalization.
export bool Asset_Name_Equals_No_Case(const char *left, const char *right) noexcept;
export bool Asset_Name_Prefix_Equals_No_Case(
	const char *left, const char *right, std::size_t count) noexcept;

export struct AssetDependencyDesc final
{
	AssetType type = AssetType::Model;
	std::string name;
};

export struct AssetDependency final
{
	AssetType type = AssetType::Model;
	AssetIdentity identity;
};

}

namespace Assets
{

bool Asset_Name_Equals_No_Case(const char *left, const char *right) noexcept
{
	if (left == right)
		return true;

	if (left == nullptr || right == nullptr)
		return false;

	while (*left != '\0' && *right != '\0') {
		const int left_character = std::tolower(static_cast<unsigned char>(*left));
		const int right_character = std::tolower(static_cast<unsigned char>(*right));
		if (left_character != right_character)
			return false;

		++left;
		++right;
	}

	return *left == *right;
}

bool Asset_Name_Prefix_Equals_No_Case(
	const char *left, const char *right, std::size_t count) noexcept
{
	if (count == 0 || left == right)
		return true;

	if (left == nullptr || right == nullptr)
		return false;

	for (std::size_t index = 0; index < count; ++index) {
		const unsigned char left_value = static_cast<unsigned char>(left[index]);
		const unsigned char right_value = static_cast<unsigned char>(right[index]);
		const int left_character = std::tolower(left_value);
		const int right_character = std::tolower(right_value);
		if (left_character != right_character)
			return false;

		if (left_value == '\0' || right_value == '\0')
			return left_value == right_value;
	}

	return true;
}

std::string Canonicalize_Asset_Name(std::string_view name)
{
	std::string normalized;
	normalized.reserve(name.size());
	for (const char character : name) {
		if (character == '\\') {
			normalized.push_back('/');
		} else if (character >= 'A' && character <= 'Z') {
			normalized.push_back(static_cast<char>(character - 'A' + 'a'));
		} else {
			normalized.push_back(character);
		}
	}

	std::vector<std::string> components;
	std::size_t component_start = 0;
	while (component_start <= normalized.size()) {
		const std::size_t separator = normalized.find('/', component_start);
		const std::size_t component_end = separator == std::string::npos
			? normalized.size()
			: separator;
		const std::string_view component(normalized.data() + component_start, component_end - component_start);

		if (component.empty() || component == ".") {
			// Repeated separators and current-directory components do not affect
			// asset identity.
		} else if (component == "..") {
			if (!components.empty())
				components.pop_back();
		} else {
			components.emplace_back(component);
		}

		if (separator == std::string::npos)
			break;
		component_start = separator + 1;
	}

	std::string result;
	for (const std::string &component : components) {
		if (!result.empty())
			result.push_back('/');
		result += component;
	}
	return result;
}

}

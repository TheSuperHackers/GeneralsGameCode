#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

import Assets.Adapters.W3D;
import Assets.Identity;
import Assets.Importers.Models;
import Assets.Runtime;

#include "Common/FileSystem.h"
#include "Common/RuntimeConfig.h"
#include "W3DDevice/GameClient/W3DFileSystem.h"
#ifdef _WIN32
#include "Win32Device/FontSource.h"
#endif

namespace W3DAssetRuntime
{

namespace
{

std::vector<std::byte> Read_File_Source(const std::string &filename)
{
	if (TheW3DFileSystem == nullptr)
		return {};

	GameFileClass file(filename.c_str());
	if (!file.Is_Available(false) || !file.Open())
		return {};

	const int size = file.Size();
	if (size <= 0) {
		file.Close();
		return {};
	}

	std::vector<std::byte> bytes(static_cast<std::size_t>(size));
	if (file.Read(bytes.data(), size) != size)
		bytes.clear();
	file.Close();
	return bytes;
}

std::vector<std::byte> Read_Model_Source(const Assets::AssetIdentity &identity)
{
	std::string filename = identity.canonical_name;
	if (filename.find_last_of('.') == std::string::npos)
		filename += ".w3d";
	return Read_File_Source(filename);
}

std::vector<std::byte> Read_Texture_Source(const Assets::AssetIdentity &identity)
{
    auto bytes = Read_File_Source(identity.canonical_name);
    if (!bytes.empty()) return bytes;
    const auto extension = identity.canonical_name.find_last_of('.');
    if (extension == std::string::npos) return {};
    const std::string suffix = identity.canonical_name.substr(extension);
    if (suffix == ".tga") return Read_File_Source(identity.canonical_name.substr(0,extension)+".dds");
    if (suffix == ".dds") return Read_File_Source(identity.canonical_name.substr(0,extension)+".tga");
    return {};
}

std::vector<std::byte> Read_Font_Source(const Assets::AssetIdentity &identity)
{
	constexpr std::string_view prefix = "font/";
	if (!identity.canonical_name.starts_with(prefix))
		return {};

	const std::size_t family_end = identity.canonical_name.find('/', prefix.size());
	const std::size_t size_end = family_end == std::string::npos
		? std::string::npos
		: identity.canonical_name.find('/', family_end + 1);
	if (family_end == std::string::npos || size_end == std::string::npos)
		return {};

	const std::string family = identity.canonical_name.substr(
		prefix.size(), family_end - prefix.size());
	const std::size_t width_start = identity.canonical_name.find('/', size_end + 1);
	const bool bold = identity.canonical_name.substr(size_end + 1, width_start - size_end - 1) == "1";
	const std::string weight_suffix = bold ? " Bold" : "";
	const std::string language_directory = std::string("Data/") + GetGameLanguage().str() + "/Language/";
	const std::array<std::string, 8> candidates = {
		family + weight_suffix + ".ttf",
		family + weight_suffix + ".otf",
		family + ".ttf",
		family + ".otf",
		language_directory + family + weight_suffix + ".ttf",
		language_directory + family + weight_suffix + ".otf",
		language_directory + family + ".ttf",
		language_directory + family + ".otf"};
	for (const std::string &candidate : candidates) {
		std::vector<std::byte> bytes = Read_File_Source(candidate);
		if (!bytes.empty())
			return bytes;
	}
#ifdef _WIN32
	return Read_Installed_Font_Source(family,bold);
#else
	return {};
#endif
}

std::vector<std::byte> Read_Asset_Source(const Assets::AssetIdentity &identity)
{
	switch (identity.type) {
        case Assets::AssetType::Texture:
            return Read_Texture_Source(identity);
		case Assets::AssetType::Font:
			return Read_Font_Source(identity);
		default:
			return Read_Model_Source(identity);
	}
}


}

bool Initialize()
{
	if (Assets::Try_Get_Asset_Cache() != nullptr)
		return true;

	auto adapter = std::make_shared<Assets::W3DAdapter>();
	const std::array<std::shared_ptr<const Assets::IModelAdapter>, 1> adapters = {adapter};
	return Assets::Initialize_Asset_Runtime(Read_Asset_Source, adapters);
}

void Shutdown() noexcept
{
	Assets::Shutdown_Asset_Runtime();
}

}

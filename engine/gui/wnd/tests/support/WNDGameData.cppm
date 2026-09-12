module;

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

export module Engine.UI.WND.Tests.Support.GameData;

import Assets.Identity;
import Assets.Importers.Models;
import Assets.Runtime;
import Engine.UI.WND.Document;

namespace Engine::UI::WND::Tests
{

export struct GameData final
{
	std::filesystem::path root;
	std::unordered_map<std::string, std::filesystem::path> textures;
};

namespace
{

std::string Key(std::string_view value)
{
	std::string result;
	result.reserve(value.size());
	for (const char character : value)
		result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
	return result;
}

bool Is_INI(const std::filesystem::path &path)
{
	return Key(path.extension().string()) == ".ini";
}

bool Read_Bytes(const std::filesystem::path &path, std::vector<std::byte> &bytes)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
		return false;
	const std::streampos end = file.tellg();
	if (end <= 0)
		return false;
	bytes.resize(static_cast<std::size_t>(end));
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
	return file.good() || file.eof();
}

bool Read_Text(const std::filesystem::path &path, std::string &text)
{
	std::ifstream file(path);
	if (!file)
		return false;
	text.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	return !text.empty();
}

std::filesystem::path Font_Path(std::string_view family)
{
	const char *windows_directory = std::getenv("WINDIR");
	const std::filesystem::path root = windows_directory != nullptr && *windows_directory != '\0'
		? std::filesystem::path(windows_directory)
		: std::filesystem::path("C:/Windows");
	return root / "Fonts" / (Key(family).find("times") != std::string::npos ? "times.ttf" : "arial.ttf");
}

std::string Font_Family(std::string_view identity)
{
	if (!identity.starts_with("font/"))
		return {};
	const std::size_t end = identity.find('/', 5);
	return end == std::string_view::npos ? std::string(identity.substr(5))
		: std::string(identity.substr(5, end - 5));
}

}

export GameData Load_Game_Data(const std::filesystem::path &root)
{
	GameData data;
	data.root = root;
	std::error_code error;
	const std::filesystem::path texture_root = root / "Art" / "Textures";
	if (!std::filesystem::exists(texture_root, error) || error)
		return data;
	for (std::filesystem::recursive_directory_iterator iterator(texture_root, error), end;
		iterator != end && !error; iterator.increment(error)) {
		if (!iterator->is_regular_file(error) || error)
			continue;
		data.textures.emplace(Key(iterator->path().filename().string()), iterator->path());
	}
	return data;
}

export bool Load_WND_Source(const std::filesystem::path &path, std::string &source)
{
	return Read_Text(path, source);
}

export bool Load_Game_Image_Catalog(const GameData &data, ImageCatalog &catalog)
{
	std::error_code error;
	const std::filesystem::path mapped_images = data.root / "Data" / "INI" / "MappedImages";
	if (!std::filesystem::exists(mapped_images, error) || error)
		return false;

	std::vector<std::filesystem::path> files;
	for (std::filesystem::recursive_directory_iterator iterator(mapped_images, error), end;
		iterator != end && !error; iterator.increment(error)) {
		if (iterator->is_regular_file(error) && !error && Is_INI(iterator->path()))
			files.push_back(iterator->path());
	}
	// The hand-authored catalog is the runtime override for generated
	// TextureSize_512 entries.  Process generated files first so valid game
	// assets such as MainMenuBackdropuserinterface.tga win duplicate names.
	std::sort(files.rbegin(), files.rend());
	if (files.empty())
		return false;
	for (const std::filesystem::path &path : files) {
		std::string source;
		if (!Read_Text(path, source) || !Parse_Mapped_Image_INI(source, catalog))
			return false;
	}
	return catalog.Size() != 0;
}

export Assets::AssetSource Make_Game_Asset_Source(std::shared_ptr<const GameData> data)
{
	return [data = std::move(data)](const Assets::AssetIdentity &identity) {
		std::vector<std::byte> bytes;
		if (identity.type == Assets::AssetType::Font) {
			Read_Bytes(Font_Path(Font_Family(identity.canonical_name)), bytes);
			return bytes;
		}
		if (identity.type != Assets::AssetType::Texture || data == nullptr)
			return bytes;
		const std::string filename = Key(
			std::filesystem::path(identity.canonical_name).filename().string());
		auto found = data->textures.find(filename);
		// Older WND catalogs use a logical name such as MainMenuBackdrop.tga,
		// while the remastered run tree stores the matching atlas as
		// MainMenuBackdropuserinterface.tga.  Accept that canonical game alias
		// without copying or renaming the source texture in the fixture.
		if (found == data->textures.end()) {
			const std::filesystem::path path(filename);
			const std::string alias = path.stem().string() + "userinterface"
				+ path.extension().string();
			found = data->textures.find(alias);
			if (found == data->textures.end()) {
				// The remastered run tree stores several legacy UI atlases as DDS
				// while their mapped-image INIs retain the original TGA name.
				const std::string dds_alias = path.stem().string() + ".dds";
				found = data->textures.find(dds_alias);
			}
		}
		if (found != data->textures.end())
			Read_Bytes(found->second, bytes);
		return bytes;
	};
}

export bool Initialize_Game_Asset_Runtime(std::shared_ptr<const GameData> data)
{
	if (Assets::Try_Get_Asset_Cache() != nullptr)
		return true;
	return Assets::Initialize_Asset_Runtime(Make_Game_Asset_Source(std::move(data)), {});
}

}

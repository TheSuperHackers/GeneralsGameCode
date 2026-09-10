module;

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <exception>
#include <future>
#include <limits>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

export module Assets.Cache;

import Assets.Cache.ModelLoadTask;
import Assets.Cache.MaterialLoadTask;
import Assets.Cache.TextureLoadTask;
import Assets.Cache.FontLoadTask;
import Assets.Handles;
import Assets.Identity;
import Assets.Importers.Models;
import Assets.Fonts;
import Assets.Materials;
import Assets.Models;
import Assets.States;
import Assets.Textures;

namespace Assets
{

export class AssetCache final
{
public:
	explicit AssetCache(AssetSource source);
	~AssetCache();

	AssetCache(const AssetCache &) = delete;
	AssetCache &operator=(const AssetCache &) = delete;

	bool Register_Model_Adapter(std::shared_ptr<const IModelAdapter> adapter);

	ModelAssetHandle Request_Model(std::string_view name);
	MaterialAssetHandle Request_Material(MaterialAssetDesc description);
	TextureAssetHandle Request_Texture(std::string_view name);
	FontAssetHandle Request_Font(std::string_view family, std::uint32_t point_size, bool bold,
		std::uint32_t average_width = 0);

	AssetState Get_State(ModelAssetHandle handle) const noexcept;
	AssetState Get_State(MaterialAssetHandle handle) const noexcept;
	AssetState Get_State(TextureAssetHandle handle) const noexcept;
	AssetState Get_State(FontAssetHandle handle) const noexcept;

	// These are non-owning views. Consumers retain the typed handle and must
	// keep this cache alive while using a returned asset.
	const ModelAsset *Try_Get_Model(ModelAssetHandle handle) const noexcept;
	const MaterialAsset *Try_Get_Material(MaterialAssetHandle handle) const noexcept;
	const TextureAsset *Try_Get_Texture(TextureAssetHandle handle) const noexcept;
	const FontAsset *Try_Get_Font(FontAssetHandle handle) const noexcept;

	std::string Get_Error(ModelAssetHandle handle) const;
	std::string Get_Error(MaterialAssetHandle handle) const;
	std::string Get_Error(TextureAssetHandle handle) const;
	std::string Get_Error(FontAssetHandle handle) const;

	void Wait(ModelAssetHandle handle) const;
	void Wait(MaterialAssetHandle handle) const;
	void Wait(TextureAssetHandle handle) const;
	void Wait(FontAssetHandle handle) const;

	// Dependency views are published before the owning asset becomes Ready.
	// Callers should query them after observing Ready.
	std::span<const MaterialAssetHandle> Model_Material_Dependencies(ModelAssetHandle handle) const noexcept;
	std::span<const TextureAssetHandle> Model_Texture_Dependencies(ModelAssetHandle handle) const noexcept;
	std::span<const TextureAssetHandle> Material_Texture_Dependencies(MaterialAssetHandle handle) const noexcept;

	std::size_t Model_Count() const noexcept;
	std::size_t Material_Count() const noexcept;
	std::size_t Texture_Count() const noexcept;
	std::size_t Font_Count() const noexcept;

private:
	MaterialAssetHandle Request_Material(AssetIdentity identity, MaterialAssetDesc description);

	struct ModelEntry final
	{
		AssetIdentity identity;
		ModelAssetHandle handle;
		std::atomic<AssetState> state{AssetState::Unloaded};
		std::shared_ptr<const ModelAsset> asset;
		std::vector<MaterialAssetHandle> material_dependencies;
		std::vector<TextureAssetHandle> texture_dependencies;
		mutable std::mutex error_mutex;
		std::string error;
		std::shared_future<void> completion;
	};

	struct MaterialEntry final
	{
		AssetIdentity identity;
		MaterialAssetHandle handle;
		MaterialAssetDesc description;
		std::atomic<AssetState> state{AssetState::Unloaded};
		std::shared_ptr<const MaterialAsset> asset;
		std::vector<TextureAssetHandle> texture_dependencies;
		mutable std::mutex error_mutex;
		std::string error;
		std::shared_future<void> completion;
	};

	struct TextureEntry final
	{
		AssetIdentity identity;
		TextureAssetHandle handle;
		std::atomic<AssetState> state{AssetState::Unloaded};
		std::shared_ptr<const TextureAsset> asset;
		mutable std::mutex error_mutex;
		std::string error;
		std::shared_future<void> completion;
	};

	struct FontEntry final
	{
		AssetIdentity identity;
		FontAssetHandle handle;
		std::atomic<AssetState> state{AssetState::Unloaded};
		std::shared_ptr<const FontAsset> asset;
		mutable std::mutex error_mutex;
		std::string error;
		std::shared_future<void> completion;
	};

	struct ModelSnapshot final
	{
		std::vector<std::shared_ptr<ModelEntry>> entries;
		std::unordered_map<std::string, ModelAssetHandle> handles;
	};

	struct MaterialSnapshot final
	{
		std::vector<std::shared_ptr<MaterialEntry>> entries;
		std::unordered_map<std::string, MaterialAssetHandle> handles;
	};

	struct TextureSnapshot final
	{
		std::vector<std::shared_ptr<TextureEntry>> entries;
		std::unordered_map<std::string, TextureAssetHandle> handles;
	};

	struct FontSnapshot final
	{
		std::vector<std::shared_ptr<FontEntry>> entries;
		std::unordered_map<std::string, FontAssetHandle> handles;
	};

	static std::shared_ptr<ModelEntry> Find_Model_Entry(
		const std::shared_ptr<const ModelSnapshot> &snapshot,
		ModelAssetHandle handle) noexcept;
	static std::shared_ptr<MaterialEntry> Find_Material_Entry(
		const std::shared_ptr<const MaterialSnapshot> &snapshot,
		MaterialAssetHandle handle) noexcept;
	static std::shared_ptr<TextureEntry> Find_Texture_Entry(
		const std::shared_ptr<const TextureSnapshot> &snapshot,
		TextureAssetHandle handle) noexcept;
	static std::shared_ptr<FontEntry> Find_Font_Entry(
		const std::shared_ptr<const FontSnapshot> &snapshot,
		FontAssetHandle handle) noexcept;

	static void Set_Failure(const std::shared_ptr<ModelEntry> &entry, std::string error) noexcept;
	static void Set_Failure(const std::shared_ptr<MaterialEntry> &entry, std::string error) noexcept;
	static void Set_Failure(const std::shared_ptr<TextureEntry> &entry, std::string error) noexcept;
	static void Set_Failure(const std::shared_ptr<FontEntry> &entry, std::string error) noexcept;
	static std::string Dependency_Error(
		std::string_view owner_type,
		std::string_view dependency_name,
		std::string_view dependency_error);
	void Wait_All() const;

	AssetSource m_source;
	mutable std::mutex m_request_mutex;
	std::vector<std::shared_ptr<const IModelAdapter>> m_model_adapters;
	std::shared_ptr<const ModelSnapshot> m_model_snapshot;
	std::shared_ptr<const MaterialSnapshot> m_material_snapshot;
	std::shared_ptr<const TextureSnapshot> m_texture_snapshot;
	std::shared_ptr<const FontSnapshot> m_font_snapshot;
};

AssetCache::AssetCache(AssetSource source)
	: m_source(std::move(source)),
	  m_model_snapshot(std::make_shared<const ModelSnapshot>()),
	  m_material_snapshot(std::make_shared<const MaterialSnapshot>()),
	  m_texture_snapshot(std::make_shared<const TextureSnapshot>()),
	  m_font_snapshot(std::make_shared<const FontSnapshot>())
{
}

AssetCache::~AssetCache()
{
	Wait_All();
}

bool AssetCache::Register_Model_Adapter(std::shared_ptr<const IModelAdapter> adapter)
{
	if (!adapter)
		return false;

	std::lock_guard lock(m_request_mutex);
	m_model_adapters.push_back(std::move(adapter));
	return true;
}

ModelAssetHandle AssetCache::Request_Model(std::string_view name)
{
	const std::string canonical_name = Canonicalize_Asset_Name(name);
	if (canonical_name.empty())
		return ModelAssetHandle::Invalid();

	std::lock_guard lock(m_request_mutex);
	const std::shared_ptr<const ModelSnapshot> current =
		std::atomic_load_explicit(&m_model_snapshot, std::memory_order_acquire);
	const auto existing = current->handles.find(canonical_name);
	if (existing != current->handles.end())
		return existing->second;

	if (current->entries.size() >= std::numeric_limits<ModelAssetHandle::Index>::max())
		return ModelAssetHandle::Invalid();

	const ModelAssetHandle handle(
		static_cast<ModelAssetHandle::Index>(current->entries.size()),
		ModelAssetHandle::Generation{1});
	auto entry = std::make_shared<ModelEntry>();
	entry->identity = {AssetType::Model, canonical_name};
	entry->handle = handle;
	entry->state.store(AssetState::Loading, std::memory_order_release);

	const AssetSource source = m_source;
	const std::vector<std::shared_ptr<const IModelAdapter>> adapters = m_model_adapters;
	try {
		entry->completion = std::async(
			std::launch::async,
			[this, entry, source, adapters]() {
				try {
				ModelDescriptionLoadResult loaded =
					Load_Model_Description(entry->identity, source, adapters);
				if (!loaded.Succeeded()) {
					Set_Failure(entry, loaded.error);
					return;
				}

				std::unique_ptr<ModelAssetDesc> description = std::move(loaded.description);
				std::vector<std::string> material_names;
				std::vector<MaterialAssetHandle> material_handles;
				material_names.reserve(description->materials.size());
				material_handles.reserve(description->materials.size());
				entry->material_dependencies.reserve(description->materials.size());
				for (const ModelMaterialDesc &material : description->materials) {
					material_names.push_back(Canonicalize_Asset_Name(material.name));
					const MaterialAssetHandle material_handle = material.scope == MaterialScope::Model
						? Request_Material({AssetType::Material, entry->identity.canonical_name
							+ "#material/" + std::to_string(material_handles.size())}, material)
						: Request_Material(material);
					if (!material_handle.Is_Valid()) {
						Set_Failure(entry, "model contains a material with an empty identity");
						return;
					}
					material_handles.push_back(material_handle);
					if (std::find(
							entry->material_dependencies.begin(),
							entry->material_dependencies.end(),
							material_handle) == entry->material_dependencies.end())
						entry->material_dependencies.push_back(material_handle);
				}

				for (const AssetDependencyDesc &dependency : description->dependencies) {
					const std::string dependency_name = Canonicalize_Asset_Name(dependency.name);
					switch (dependency.type) {
						case AssetType::Material: {
							const auto material_name = std::find(material_names.begin(), material_names.end(), dependency_name);
							if (material_name == material_names.end()) {
								Set_Failure(entry, "model material dependency has no matching material description");
								return;
							}
							break;
						}
						case AssetType::Texture: {
							const TextureAssetHandle texture_handle = Request_Texture(dependency.name);
							if (!texture_handle.Is_Valid()) {
								Set_Failure(entry, "model contains a texture dependency with an empty identity");
								return;
							}
							if (std::find(
									entry->texture_dependencies.begin(),
									entry->texture_dependencies.end(),
									texture_handle) == entry->texture_dependencies.end())
								entry->texture_dependencies.push_back(texture_handle);
							break;
						}
						default:
							// Skeleton, animation, mesh, and future asset kinds remain
							// recorded in the generic description. This milestone only
							// schedules the model/material/texture chain.
							break;
					}
				}

				for (const MaterialAssetHandle material_handle : entry->material_dependencies) {
					Wait(material_handle);
					if (Get_State(material_handle) != AssetState::Ready) {
						Set_Failure(
							entry,
							Dependency_Error("model", "material", Get_Error(material_handle)));
						return;
					}
					for (const TextureAssetHandle texture_handle : Material_Texture_Dependencies(material_handle)) {
						if (std::find(entry->texture_dependencies.begin(), entry->texture_dependencies.end(),
								texture_handle) == entry->texture_dependencies.end())
							entry->texture_dependencies.push_back(texture_handle);
					}
				}
				for (const TextureAssetHandle texture_handle : entry->texture_dependencies) {
					Wait(texture_handle);
					if (Get_State(texture_handle) != AssetState::Ready) {
						Set_Failure(
							entry,
							Dependency_Error("model", "texture", Get_Error(texture_handle)));
						return;
					}
				}

				// Publish resolved identities for embedded material dependencies.
				// One authored name can refer to several distinct pass variants.
				std::vector<AssetDependencyDesc> resolved_dependencies;
				for (const AssetDependencyDesc &dependency : description->dependencies) {
					if (dependency.type != AssetType::Material) {
						resolved_dependencies.push_back(dependency);
						continue;
					}
					const std::string name = Canonicalize_Asset_Name(dependency.name);
					for (std::size_t index = 0; index != material_names.size(); ++index) {
						if (material_names[index] != name)
							continue;
						const auto &identity = Try_Get_Material(material_handles[index])->Identity();
						const bool duplicate = std::any_of(resolved_dependencies.begin(), resolved_dependencies.end(),
							[&identity](const AssetDependencyDesc &existing) {
								return existing.type == identity.type && existing.name == identity.canonical_name;
							});
						if (!duplicate)
							resolved_dependencies.push_back({identity.type, identity.canonical_name});
					}
				}
				for (const TextureAssetHandle texture_handle : entry->texture_dependencies) {
					const auto &identity = Try_Get_Texture(texture_handle)->Identity();
					const bool duplicate = std::any_of(resolved_dependencies.begin(), resolved_dependencies.end(),
						[&identity](const AssetDependencyDesc &existing) {
							return existing.type == identity.type
								&& Canonicalize_Asset_Name(existing.name) == identity.canonical_name;
						});
					if (!duplicate)
						resolved_dependencies.push_back({identity.type, identity.canonical_name});
				}
				description->dependencies = std::move(resolved_dependencies);

				const auto runtime = std::make_shared<const ModelAsset>(
					entry->identity,
					std::move(*description),
					material_handles);
				std::atomic_store_explicit(&entry->asset, runtime, std::memory_order_release);
				entry->state.store(AssetState::Ready, std::memory_order_release);
				} catch (const std::exception &exception) {
					Set_Failure(entry, exception.what());
				} catch (...) {
					Set_Failure(entry, "unknown exception while finalizing model");
				}
			});
	} catch (const std::exception &exception) {
		Set_Failure(entry, exception.what());
	} catch (...) {
		Set_Failure(entry, "could not start asynchronous model load");
	}

	auto next = std::make_shared<ModelSnapshot>(*current);
	next->entries.push_back(entry);
	next->handles.emplace(canonical_name, handle);
	std::atomic_store_explicit(
		&m_model_snapshot,
		std::shared_ptr<const ModelSnapshot>(std::move(next)),
		std::memory_order_release);
	return handle;
}

MaterialAssetHandle AssetCache::Request_Material(MaterialAssetDesc description)
{
	AssetIdentity identity{AssetType::Material, Canonicalize_Asset_Name(description.name)};
	return Request_Material(std::move(identity), std::move(description));
}

MaterialAssetHandle AssetCache::Request_Material(AssetIdentity identity, MaterialAssetDesc description)
{
	const std::string &canonical_name = identity.canonical_name;
	if (canonical_name.empty() || Canonicalize_Asset_Name(description.name).empty())
		return MaterialAssetHandle::Invalid();

	std::lock_guard lock(m_request_mutex);
	const std::shared_ptr<const MaterialSnapshot> current =
		std::atomic_load_explicit(&m_material_snapshot, std::memory_order_acquire);
	const auto existing = current->handles.find(canonical_name);
	if (existing != current->handles.end())
		return existing->second;

	if (current->entries.size() >= std::numeric_limits<MaterialAssetHandle::Index>::max())
		return MaterialAssetHandle::Invalid();

	const MaterialAssetHandle handle(
		static_cast<MaterialAssetHandle::Index>(current->entries.size()),
		MaterialAssetHandle::Generation{1});
	auto entry = std::make_shared<MaterialEntry>();
	entry->identity = {AssetType::Material, canonical_name};
	entry->handle = handle;
	entry->description = std::move(description);
	entry->state.store(AssetState::Loading, std::memory_order_release);
	try {
		entry->completion = std::async(
			std::launch::async,
			[this, entry]() {
			try {
				const MaterialAssetDesc description = entry->description;
				TextureAssetHandle primary_texture = TextureAssetHandle::Invalid();
				TextureAssetHandle secondary_texture = TextureAssetHandle::Invalid();
				MaterialSurfaceTextureHandles surface_textures{};
				if (!description.primary_texture.empty())
					primary_texture = Request_Texture(description.primary_texture);
				if (!description.secondary_texture.empty())
					secondary_texture = Request_Texture(description.secondary_texture);
				if ((!description.primary_texture.empty() && !primary_texture.Is_Valid()) ||
					(!description.secondary_texture.empty() && !secondary_texture.Is_Valid())) {
					Set_Failure(entry, "material contains a texture with an empty identity");
					return;
				}

				if (primary_texture.Is_Valid())
					entry->texture_dependencies.push_back(primary_texture);
				if (secondary_texture.Is_Valid() && secondary_texture != primary_texture)
					entry->texture_dependencies.push_back(secondary_texture);
				for (std::size_t index = 0; index < surface_textures.size(); ++index) {
					if (description.surface_textures[index].empty())
						continue;
					const auto texture = Request_Texture(description.surface_textures[index]);
					if (!texture.Is_Valid()) {
						Set_Failure(entry, "material surface contains a texture with an empty identity");
						return;
					}
					surface_textures[index] = texture;
					if (std::find(entry->texture_dependencies.begin(), entry->texture_dependencies.end(), texture)
						== entry->texture_dependencies.end())
						entry->texture_dependencies.push_back(texture);
				}
				for (const TextureAssetHandle texture_handle : entry->texture_dependencies) {
					Wait(texture_handle);
					if (Get_State(texture_handle) != AssetState::Ready) {
						Set_Failure(
							entry,
							Dependency_Error("material", "texture", Get_Error(texture_handle)));
						return;
					}
				}

				const MaterialLoadResult loaded = Finalize_Material_Asset(
					entry->identity,
					description,
					primary_texture,
					secondary_texture,
					surface_textures);
				if (!loaded.Succeeded()) {
					Set_Failure(entry, loaded.error);
					return;
				}
				std::atomic_store_explicit(&entry->asset, loaded.asset, std::memory_order_release);
				entry->state.store(AssetState::Ready, std::memory_order_release);
			} catch (const std::exception &exception) {
				Set_Failure(entry, exception.what());
			} catch (...) {
				Set_Failure(entry, "unknown exception while finalizing material");
			}
			});
	} catch (const std::exception &exception) {
		Set_Failure(entry, exception.what());
	} catch (...) {
		Set_Failure(entry, "could not start asynchronous material load");
	}

	auto next = std::make_shared<MaterialSnapshot>(*current);
	next->entries.push_back(entry);
	next->handles.emplace(canonical_name, handle);
	std::atomic_store_explicit(
		&m_material_snapshot,
		std::shared_ptr<const MaterialSnapshot>(std::move(next)),
		std::memory_order_release);
	return handle;
}

TextureAssetHandle AssetCache::Request_Texture(std::string_view name)
{
	const std::string canonical_name = Canonicalize_Asset_Name(name);
	if (canonical_name.empty())
		return TextureAssetHandle::Invalid();

	std::lock_guard lock(m_request_mutex);
	const std::shared_ptr<const TextureSnapshot> current =
		std::atomic_load_explicit(&m_texture_snapshot, std::memory_order_acquire);
	const auto existing = current->handles.find(canonical_name);
	if (existing != current->handles.end())
		return existing->second;

	if (current->entries.size() >= std::numeric_limits<TextureAssetHandle::Index>::max())
		return TextureAssetHandle::Invalid();

	const TextureAssetHandle handle(
		static_cast<TextureAssetHandle::Index>(current->entries.size()),
		TextureAssetHandle::Generation{1});
	auto entry = std::make_shared<TextureEntry>();
	entry->identity = {AssetType::Texture, canonical_name};
	entry->handle = handle;
	entry->state.store(AssetState::Loading, std::memory_order_release);
	const AssetSource source = m_source;
	try {
		entry->completion = std::async(
			std::launch::async,
			[entry, source]() {
				try {
					if (!source) {
						Set_Failure(entry, "no asset source is configured");
						return;
					}
					const TextureLoadResult loaded = Load_Texture_Asset(entry->identity, source);
					if (!loaded.Succeeded()) {
						Set_Failure(entry, loaded.error);
						return;
					}
					std::atomic_store_explicit(&entry->asset, loaded.asset, std::memory_order_release);
					entry->state.store(AssetState::Ready, std::memory_order_release);
				} catch (const std::exception &exception) {
					Set_Failure(entry, exception.what());
				} catch (...) {
					Set_Failure(entry, "unknown exception while loading texture");
				}
			});
	} catch (const std::exception &exception) {
		Set_Failure(entry, exception.what());
	} catch (...) {
		Set_Failure(entry, "could not start asynchronous texture load");
	}

	auto next = std::make_shared<TextureSnapshot>(*current);
	next->entries.push_back(entry);
	next->handles.emplace(canonical_name, handle);
	std::atomic_store_explicit(
		&m_texture_snapshot,
		std::shared_ptr<const TextureSnapshot>(std::move(next)),
		std::memory_order_release);
	return handle;
}

FontAssetHandle AssetCache::Request_Font(std::string_view family, std::uint32_t point_size, bool bold,
	std::uint32_t average_width)
{
	const std::string canonical_family = Canonicalize_Asset_Name(family);
	if (canonical_family.empty() || point_size == 0)
		return FontAssetHandle::Invalid();

	std::string canonical_name = "font/" + canonical_family + "/"
		+ std::to_string(point_size) + "/" + (bold ? "1" : "0");
	if (average_width != 0)
		canonical_name += "/" + std::to_string(average_width);
	std::lock_guard lock(m_request_mutex);
	const std::shared_ptr<const FontSnapshot> current =
		std::atomic_load_explicit(&m_font_snapshot, std::memory_order_acquire);
	const auto existing = current->handles.find(canonical_name);
	if (existing != current->handles.end())
		return existing->second;

	if (current->entries.size() >= std::numeric_limits<FontAssetHandle::Index>::max())
		return FontAssetHandle::Invalid();

	const FontAssetHandle handle(
		static_cast<FontAssetHandle::Index>(current->entries.size()),
		FontAssetHandle::Generation{1});
	auto entry = std::make_shared<FontEntry>();
	entry->identity = {AssetType::Font, canonical_name};
	entry->handle = handle;
	entry->state.store(AssetState::Loading, std::memory_order_release);
	const AssetSource source = m_source;
	try {
		entry->completion = std::async(
			std::launch::async,
			[entry, source]() {
				try {
					const FontLoadResult loaded = Load_Font_Asset(entry->identity, source);
					if (!loaded.Succeeded()) {
						Set_Failure(entry, loaded.error);
						return;
					}
					std::atomic_store_explicit(&entry->asset, loaded.asset, std::memory_order_release);
					entry->state.store(AssetState::Ready, std::memory_order_release);
				} catch (const std::exception &exception) {
					Set_Failure(entry, exception.what());
				} catch (...) {
					Set_Failure(entry, "unknown exception while loading font");
				}
			});
	} catch (const std::exception &exception) {
		Set_Failure(entry, exception.what());
	} catch (...) {
		Set_Failure(entry, "could not start asynchronous font load");
	}

	auto next = std::make_shared<FontSnapshot>(*current);
	next->entries.push_back(entry);
	next->handles.emplace(canonical_name, handle);
	std::atomic_store_explicit(
		&m_font_snapshot,
		std::shared_ptr<const FontSnapshot>(std::move(next)),
		std::memory_order_release);
	return handle;
}

AssetState AssetCache::Get_State(ModelAssetHandle handle) const noexcept
{
	const auto snapshot = std::atomic_load_explicit(&m_model_snapshot, std::memory_order_acquire);
	const auto entry = Find_Model_Entry(snapshot, handle);
	return entry ? entry->state.load(std::memory_order_acquire) : AssetState::Unloaded;
}

AssetState AssetCache::Get_State(MaterialAssetHandle handle) const noexcept
{
	const auto snapshot = std::atomic_load_explicit(&m_material_snapshot, std::memory_order_acquire);
	const auto entry = Find_Material_Entry(snapshot, handle);
	return entry ? entry->state.load(std::memory_order_acquire) : AssetState::Unloaded;
}

AssetState AssetCache::Get_State(TextureAssetHandle handle) const noexcept
{
	const auto snapshot = std::atomic_load_explicit(&m_texture_snapshot, std::memory_order_acquire);
	const auto entry = Find_Texture_Entry(snapshot, handle);
	return entry ? entry->state.load(std::memory_order_acquire) : AssetState::Unloaded;
}

AssetState AssetCache::Get_State(FontAssetHandle handle) const noexcept
{
	const auto snapshot = std::atomic_load_explicit(&m_font_snapshot, std::memory_order_acquire);
	const auto entry = Find_Font_Entry(snapshot, handle);
	return entry ? entry->state.load(std::memory_order_acquire) : AssetState::Unloaded;
}

const ModelAsset *AssetCache::Try_Get_Model(ModelAssetHandle handle) const noexcept
{
	const auto snapshot = std::atomic_load_explicit(&m_model_snapshot, std::memory_order_acquire);
	const auto entry = Find_Model_Entry(snapshot, handle);
	if (!entry || entry->state.load(std::memory_order_acquire) != AssetState::Ready)
		return nullptr;
	const auto asset = std::atomic_load_explicit(&entry->asset, std::memory_order_acquire);
	return asset.get();
}

const MaterialAsset *AssetCache::Try_Get_Material(MaterialAssetHandle handle) const noexcept
{
	const auto snapshot = std::atomic_load_explicit(&m_material_snapshot, std::memory_order_acquire);
	const auto entry = Find_Material_Entry(snapshot, handle);
	if (!entry || entry->state.load(std::memory_order_acquire) != AssetState::Ready)
		return nullptr;
	const auto asset = std::atomic_load_explicit(&entry->asset, std::memory_order_acquire);
	return asset.get();
}

const TextureAsset *AssetCache::Try_Get_Texture(TextureAssetHandle handle) const noexcept
{
	const auto snapshot = std::atomic_load_explicit(&m_texture_snapshot, std::memory_order_acquire);
	const auto entry = Find_Texture_Entry(snapshot, handle);
	if (!entry || entry->state.load(std::memory_order_acquire) != AssetState::Ready)
		return nullptr;
	const auto asset = std::atomic_load_explicit(&entry->asset, std::memory_order_acquire);
	return asset.get();
}

const FontAsset *AssetCache::Try_Get_Font(FontAssetHandle handle) const noexcept
{
	const auto snapshot = std::atomic_load_explicit(&m_font_snapshot, std::memory_order_acquire);
	const auto entry = Find_Font_Entry(snapshot, handle);
	if (!entry || entry->state.load(std::memory_order_acquire) != AssetState::Ready)
		return nullptr;
	const auto asset = std::atomic_load_explicit(&entry->asset, std::memory_order_acquire);
	return asset.get();
}

std::string AssetCache::Get_Error(ModelAssetHandle handle) const
{
	const auto snapshot = std::atomic_load_explicit(&m_model_snapshot, std::memory_order_acquire);
	const auto entry = Find_Model_Entry(snapshot, handle);
	if (!entry)
		return "invalid model asset handle";
	std::lock_guard lock(entry->error_mutex);
	return entry->error;
}

std::string AssetCache::Get_Error(MaterialAssetHandle handle) const
{
	const auto snapshot = std::atomic_load_explicit(&m_material_snapshot, std::memory_order_acquire);
	const auto entry = Find_Material_Entry(snapshot, handle);
	if (!entry)
		return "invalid material asset handle";
	std::lock_guard lock(entry->error_mutex);
	return entry->error;
}

std::string AssetCache::Get_Error(TextureAssetHandle handle) const
{
	const auto snapshot = std::atomic_load_explicit(&m_texture_snapshot, std::memory_order_acquire);
	const auto entry = Find_Texture_Entry(snapshot, handle);
	if (!entry)
		return "invalid texture asset handle";
	std::lock_guard lock(entry->error_mutex);
	return entry->error;
}

std::string AssetCache::Get_Error(FontAssetHandle handle) const
{
	const auto snapshot = std::atomic_load_explicit(&m_font_snapshot, std::memory_order_acquire);
	const auto entry = Find_Font_Entry(snapshot, handle);
	if (!entry)
		return "invalid font asset handle";
	std::lock_guard lock(entry->error_mutex);
	return entry->error;
}

void AssetCache::Wait(ModelAssetHandle handle) const
{
	const auto snapshot = std::atomic_load_explicit(&m_model_snapshot, std::memory_order_acquire);
	const auto entry = Find_Model_Entry(snapshot, handle);
	if (entry && entry->completion.valid())
		entry->completion.wait();
}

void AssetCache::Wait(MaterialAssetHandle handle) const
{
	const auto snapshot = std::atomic_load_explicit(&m_material_snapshot, std::memory_order_acquire);
	const auto entry = Find_Material_Entry(snapshot, handle);
	if (entry && entry->completion.valid())
		entry->completion.wait();
}

void AssetCache::Wait(TextureAssetHandle handle) const
{
	const auto snapshot = std::atomic_load_explicit(&m_texture_snapshot, std::memory_order_acquire);
	const auto entry = Find_Texture_Entry(snapshot, handle);
	if (entry && entry->completion.valid())
		entry->completion.wait();
}

void AssetCache::Wait(FontAssetHandle handle) const
{
	const auto snapshot = std::atomic_load_explicit(&m_font_snapshot, std::memory_order_acquire);
	const auto entry = Find_Font_Entry(snapshot, handle);
	if (entry && entry->completion.valid())
		entry->completion.wait();
}

std::span<const MaterialAssetHandle> AssetCache::Model_Material_Dependencies(ModelAssetHandle handle) const noexcept
{
	const auto snapshot = std::atomic_load_explicit(&m_model_snapshot, std::memory_order_acquire);
	const auto entry = Find_Model_Entry(snapshot, handle);
	if (!entry || entry->state.load(std::memory_order_acquire) != AssetState::Ready)
		return {};
	return entry->material_dependencies;
}

std::span<const TextureAssetHandle> AssetCache::Model_Texture_Dependencies(ModelAssetHandle handle) const noexcept
{
	const auto snapshot = std::atomic_load_explicit(&m_model_snapshot, std::memory_order_acquire);
	const auto entry = Find_Model_Entry(snapshot, handle);
	if (!entry || entry->state.load(std::memory_order_acquire) != AssetState::Ready)
		return {};
	return entry->texture_dependencies;
}

std::span<const TextureAssetHandle> AssetCache::Material_Texture_Dependencies(MaterialAssetHandle handle) const noexcept
{
	const auto snapshot = std::atomic_load_explicit(&m_material_snapshot, std::memory_order_acquire);
	const auto entry = Find_Material_Entry(snapshot, handle);
	if (!entry || entry->state.load(std::memory_order_acquire) != AssetState::Ready)
		return {};
	return entry->texture_dependencies;
}

std::size_t AssetCache::Model_Count() const noexcept
{
	return std::atomic_load_explicit(&m_model_snapshot, std::memory_order_acquire)->entries.size();
}

std::size_t AssetCache::Material_Count() const noexcept
{
	return std::atomic_load_explicit(&m_material_snapshot, std::memory_order_acquire)->entries.size();
}

std::size_t AssetCache::Texture_Count() const noexcept
{
	return std::atomic_load_explicit(&m_texture_snapshot, std::memory_order_acquire)->entries.size();
}

std::size_t AssetCache::Font_Count() const noexcept
{
	return std::atomic_load_explicit(&m_font_snapshot, std::memory_order_acquire)->entries.size();
}

std::shared_ptr<AssetCache::ModelEntry> AssetCache::Find_Model_Entry(
	const std::shared_ptr<const ModelSnapshot> &snapshot,
	ModelAssetHandle handle) noexcept
{
	if (!handle.Is_Valid() || handle.Get_Index() >= snapshot->entries.size())
		return {};
	const std::shared_ptr<ModelEntry> &entry = snapshot->entries[handle.Get_Index()];
	return entry && entry->handle == handle ? entry : std::shared_ptr<ModelEntry>{};
}

std::shared_ptr<AssetCache::MaterialEntry> AssetCache::Find_Material_Entry(
	const std::shared_ptr<const MaterialSnapshot> &snapshot,
	MaterialAssetHandle handle) noexcept
{
	if (!handle.Is_Valid() || handle.Get_Index() >= snapshot->entries.size())
		return {};
	const std::shared_ptr<MaterialEntry> &entry = snapshot->entries[handle.Get_Index()];
	return entry && entry->handle == handle ? entry : std::shared_ptr<MaterialEntry>{};
}

std::shared_ptr<AssetCache::TextureEntry> AssetCache::Find_Texture_Entry(
	const std::shared_ptr<const TextureSnapshot> &snapshot,
	TextureAssetHandle handle) noexcept
{
	if (!handle.Is_Valid() || handle.Get_Index() >= snapshot->entries.size())
		return {};
	const std::shared_ptr<TextureEntry> &entry = snapshot->entries[handle.Get_Index()];
	return entry && entry->handle == handle ? entry : std::shared_ptr<TextureEntry>{};
}

std::shared_ptr<AssetCache::FontEntry> AssetCache::Find_Font_Entry(
	const std::shared_ptr<const FontSnapshot> &snapshot,
	FontAssetHandle handle) noexcept
{
	if (!handle.Is_Valid() || handle.Get_Index() >= snapshot->entries.size())
		return {};
	const std::shared_ptr<FontEntry> &entry = snapshot->entries[handle.Get_Index()];
	return entry && entry->handle == handle ? entry : std::shared_ptr<FontEntry>{};
}

void AssetCache::Set_Failure(const std::shared_ptr<ModelEntry> &entry, std::string error) noexcept
{
	try {
		{
			std::lock_guard lock(entry->error_mutex);
			entry->error = std::move(error);
		}
		entry->state.store(AssetState::Failed, std::memory_order_release);
	} catch (...) {
		entry->state.store(AssetState::Failed, std::memory_order_release);
	}
}

void AssetCache::Set_Failure(const std::shared_ptr<MaterialEntry> &entry, std::string error) noexcept
{
	try {
		{
			std::lock_guard lock(entry->error_mutex);
			entry->error = std::move(error);
		}
		entry->state.store(AssetState::Failed, std::memory_order_release);
	} catch (...) {
		entry->state.store(AssetState::Failed, std::memory_order_release);
	}
}

void AssetCache::Set_Failure(const std::shared_ptr<TextureEntry> &entry, std::string error) noexcept
{
	try {
		{
			std::lock_guard lock(entry->error_mutex);
			entry->error = std::move(error);
		}
		entry->state.store(AssetState::Failed, std::memory_order_release);
	} catch (...) {
		entry->state.store(AssetState::Failed, std::memory_order_release);
	}
}

void AssetCache::Set_Failure(const std::shared_ptr<FontEntry> &entry, std::string error) noexcept
{
	try {
		{
			std::lock_guard lock(entry->error_mutex);
			entry->error = std::move(error);
		}
		entry->state.store(AssetState::Failed, std::memory_order_release);
	} catch (...) {
		entry->state.store(AssetState::Failed, std::memory_order_release);
	}
}

std::string AssetCache::Dependency_Error(
	std::string_view owner_type,
	std::string_view dependency_name,
	std::string_view dependency_error)
{
	std::string result;
	result.reserve(owner_type.size() + dependency_name.size() + dependency_error.size() + 32);
	result += owner_type;
	result += " dependency failed: ";
	result += dependency_name;
	result += ": ";
	result += dependency_error;
	return result;
}

void AssetCache::Wait_All() const
{
	const auto model_snapshot = std::atomic_load_explicit(&m_model_snapshot, std::memory_order_acquire);
	for (const auto &entry : model_snapshot->entries) {
		if (entry && entry->completion.valid())
			entry->completion.wait();
	}

	const auto material_snapshot = std::atomic_load_explicit(&m_material_snapshot, std::memory_order_acquire);
	for (const auto &entry : material_snapshot->entries) {
		if (entry && entry->completion.valid())
			entry->completion.wait();
	}

	const auto texture_snapshot = std::atomic_load_explicit(&m_texture_snapshot, std::memory_order_acquire);
	for (const auto &entry : texture_snapshot->entries) {
		if (entry && entry->completion.valid())
			entry->completion.wait();
	}

	const auto font_snapshot = std::atomic_load_explicit(&m_font_snapshot, std::memory_order_acquire);
	for (const auto &entry : font_snapshot->entries) {
		if (entry && entry->completion.valid())
			entry->completion.wait();
	}
}

}

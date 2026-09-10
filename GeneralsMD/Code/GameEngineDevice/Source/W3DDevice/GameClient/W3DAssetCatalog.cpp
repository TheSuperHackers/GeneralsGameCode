#include "W3DDevice/GameClient/W3DAssetCatalog.h"

#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "WWLib/ffactory.h"

import Assets.Adapters.W3D.Rig;
import Assets.Adapters.W3D.RequestPolicy;
import Assets.Adapters.W3D.Retention;
import Assets.Cache.Animations;

W3DAssetCatalog *W3DAssetCatalog::s_instance = nullptr;

namespace
{

std::string Lowercase_Name(std::string_view name)
{
	std::string result(name);
	for (char &character : result)
		character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
	return result;
}

bool Is_Null_Name(const char *name) noexcept
{
	return name == nullptr || *name == '\0';
}

bool Names_Equal_No_Case(std::string_view left, std::string_view right) noexcept
{
	if (left.size() != right.size())
		return false;
	for (std::size_t index = 0; index < left.size(); ++index) {
		const auto left_character = static_cast<unsigned char>(left[index]);
		const auto right_character = static_cast<unsigned char>(right[index]);
		if (std::tolower(left_character) != std::tolower(right_character))
			return false;
	}
	return true;
}

FileFactoryClass *Effective_File_Factory(FileFactoryClass *factory) noexcept
{
	return factory != nullptr ? factory : _TheFileFactory;
}

}

W3DAssetCatalog::W3DAssetCatalog(
	FileFactoryClass *file_factory,
	TextureRequestPolicy texture_policy,
	LoadedFileObserver loaded_file_observer)
	: m_file_factory(file_factory),
	  m_texture_request_policy(std::move(texture_policy)),
	  m_loaded_file_observer(std::move(loaded_file_observer))
{
	assert(s_instance == nullptr);
	s_instance = this;
}

W3DAssetCatalog::~W3DAssetCatalog()
{
	Free_Assets();
	Assets::Get_Animation_Cache().Reset_Missing();
	Write_Report();
	if (s_instance == this)
		s_instance = nullptr;
}

W3DAssetCatalog *W3DAssetCatalog::Get_Instance() noexcept
{
	return s_instance;
}

bool W3DAssetCatalog::Load_3D_Assets(const char *filename)
{
	if (Is_Null_Name(filename))
		return false;
	if (Has_Loaded_File_Prototype(filename))
		return true;

	FileFactoryClass *factory = Effective_File_Factory(m_file_factory);
	if (factory == nullptr)
		return false;

	FileClass *file = factory->Get_File(filename);
	if (file == nullptr)
		return false;

	bool loaded = false;
	if (file->Is_Available())
		loaded = Load_3D_Assets(*file);
	factory->Return_File(file);
	if (loaded && m_loaded_file_observer)
		m_loaded_file_observer(filename);
	return loaded;
}

bool W3DAssetCatalog::Load_3D_Assets(FileClass &file)
{
	if (!file.Open())
		return false;

	ChunkLoadClass chunks(&file);
	while (chunks.Open_Chunk()) {
		switch (chunks.Cur_Chunk_ID()) {
		case Assets::W3D::W3DChunkHierarchy:
			(void)Load_Skeleton(chunks);
			break;
		case Assets::W3D::W3DChunkAnimation:
		case Assets::W3D::W3DChunkCompressedAnimation:
		case Assets::W3D::W3DChunkMorphAnimation:
			Load_Animation_Chunk(chunks);
			break;
		default:
			(void)Load_Prototype(chunks);
			break;
		}
		chunks.Close_Chunk();
	}

	file.Close();
	return true;
}

W3DRenderObject *W3DAssetCatalog::Create_Render_Obj(const char *name)
{
	if (Is_Null_Name(name))
		return nullptr;

	Prototype *prototype = Find_Prototype(name);
	if (prototype == nullptr && m_load_on_demand) {
		m_report.Record_Load_On_Demand(Assets::AssetReportCategory::Model, name);
		const auto paths = Assets::W3D::W3D_Make_Request_Paths(
			Assets::W3D::W3DRequestKind::Model, name);
		if (paths) {
			if (!Load_3D_Assets(paths->primary.c_str()))
				(void)Load_3D_Assets(paths->parent_directory.c_str());
			prototype = Find_Prototype(name);
		}
	}

	if (prototype == nullptr) {
		if (name[0] != '#')
			m_report.Record_Missing(Assets::AssetReportCategory::Model, name);
		return nullptr;
	}

	return prototype->Instantiate();
}

bool W3DAssetCatalog::Render_Obj_Exists(const char *name) const
{
	return !Is_Null_Name(name) && Find_Prototype(name) != nullptr;
}

Assets::AnimationAssetHandle W3DAssetCatalog::Acquire_Animation(const char *name)
{
	if (Is_Null_Name(name))
		return {};

	auto &cache = Assets::Get_Animation_Cache();
	Assets::AnimationAssetHandle animation = cache.Acquire(name);
	if (animation.Is_Valid() || !Assets::W3D::W3D_Should_Attempt_Animation_Load(
			m_load_on_demand, cache.Is_Missing(name)))
		return animation;

	const auto paths = Assets::W3D::W3D_Make_Request_Paths(
		Assets::W3D::W3DRequestKind::Animation, name);
	if (!paths)
		return {};

	m_report.Record_Load_On_Demand(Assets::AssetReportCategory::Animation, name);
	if (!Load_3D_Assets(paths->primary.c_str()))
		(void)Load_3D_Assets(paths->parent_directory.c_str());

	animation = cache.Acquire(name);
	if (!animation.Is_Valid()) {
		cache.Register_Missing(name);
		m_report.Record_Missing(Assets::AssetReportCategory::Animation, name);
	}
	return animation;
}

Assets::SkeletonAssetHandle W3DAssetCatalog::Get_Skeleton(const char *name)
{
	if (Is_Null_Name(name))
		return {};

	Assets::SkeletonAssetHandle skeleton = m_skeletons.Find(name);
	if (skeleton.Is_Valid() || !m_load_on_demand)
		return skeleton;

	m_report.Record_Load_On_Demand(Assets::AssetReportCategory::Skeleton, name);
	const auto paths = Assets::W3D::W3D_Make_Request_Paths(
		Assets::W3D::W3DRequestKind::Skeleton, name);
	if (paths) {
		if (!Load_3D_Assets(paths->primary.c_str()))
			(void)Load_3D_Assets(paths->parent_directory.c_str());
		skeleton = m_skeletons.Find(name);
	}

	if (!skeleton.Is_Valid())
		m_report.Record_Missing(Assets::AssetReportCategory::Skeleton, name);
	return skeleton;
}

const Assets::ModelRigDesc *W3DAssetCatalog::Resolve_Skeleton(
	Assets::SkeletonAssetHandle handle) const noexcept
{
	return m_skeletons.Resolve(handle);
}

W3DTextureHandle *W3DAssetCatalog::Get_Texture(
	const char *filename,
	MipCountType mip_level_count,
	Assets::PixelEncoding texture_format,
	bool allow_compression,
	W3DTextureHandle::TexAssetType type,
	bool allow_reduction)
{
	if (m_texture_request_policy)
		m_texture_request_policy(filename == nullptr ? std::string_view{} : std::string_view(filename),
			allow_reduction);
	if (texture_format == Assets::PixelEncoding::RG8_SNorm)
		mip_level_count = MIP_LEVELS_1;
	if (Is_Null_Name(filename))
		return nullptr;

	const std::string key = Lowercase_Name(filename);
	const auto found = m_textures.find(key);
	if (found != m_textures.end()) {
		// The first request establishes the native handle's format, mip and
		// asset-type policy. Later requests for the same filename acquire that
		// handle and deliberately leave its original policy unchanged.
		found->second->Add_Ref();
		return found->second;
	}
	if (type != W3DTextureHandle::TEX_REGULAR
		&& type != W3DTextureHandle::TEX_CUBEMAP
		&& type != W3DTextureHandle::TEX_VOLUME)
		return nullptr;

	W3DTextureHandle *texture = new W3DTextureHandle(
		key.c_str(), nullptr, mip_level_count, texture_format,
		allow_compression, allow_reduction, type);
	try {
		m_textures.emplace(key, texture);
	} catch (...) {
		texture->Release_Ref();
		throw;
	}

	// The map owns the constructor's initial reference. The returned pointer is
	// a separate caller reference, matching the native handle contract.
	texture->Add_Ref();
	return texture;
}

W3DTextureHandle *W3DAssetCatalog::Adopt_Texture(
	RefCountPtr<W3DTextureHandle> &texture)
{
	if (!texture)
		return nullptr;

	RefCountPtr<W3DTextureHandle> owner =
		RefCountPtr<W3DTextureHandle>::Create_No_Add_Ref(texture.Release());
	const std::string key = Lowercase_Name(owner->Get_Texture_Name().str());
	if (key.empty())
		return nullptr;

	const auto found = m_textures.find(key);
	if (found != m_textures.end()) {
		found->second->Add_Ref();
		return found->second;
	}

	W3DTextureHandle *raw_texture = owner.Peek();
	m_textures.emplace(key, raw_texture);
	owner.Release();
	raw_texture->Add_Ref();
	return raw_texture;
}

W3DTextureHandle *W3DAssetCatalog::Find_Texture_Borrowed(
	std::string_view filename) const
{
	if (filename.empty())
		return nullptr;
	const std::string key = Lowercase_Name(filename);
	const auto found = m_textures.find(key);
	return found == m_textures.end() ? nullptr : found->second;
}

void W3DAssetCatalog::Visit_Textures(
	const std::function<void(W3DTextureHandle *)> &visit) const
{
	if (!visit)
		return;
	for (const auto &[key, texture] : m_textures) {
		(void)key;
		visit(texture);
	}
}

void W3DAssetCatalog::Release_All_Textures() noexcept
{
	for (const auto &[key, texture] : m_textures) {
		(void)key;
		if (texture != nullptr)
			texture->Release_Ref();
	}
	m_textures.clear();
}

void W3DAssetCatalog::Release_Unused_Textures()
{
	for (auto iterator = m_textures.begin(); iterator != m_textures.end();) {
		if (iterator->second == nullptr || iterator->second->Num_Refs() != 1) {
			++iterator;
			continue;
		}

		W3DTextureHandle *texture = iterator->second;
		iterator = m_textures.erase(iterator);
		texture->Release_Ref();
	}
}

void W3DAssetCatalog::Release_Texture(W3DTextureHandle *texture)
{
	if (texture == nullptr)
		return;

	const std::string key = Lowercase_Name(texture->Get_Texture_Name().str());
	const auto found = m_textures.find(key);
	if (found == m_textures.end() || found->second != texture)
		return;
	m_textures.erase(found);
	texture->Release_Ref();
}

void W3DAssetCatalog::Free_Assets() noexcept
{
	m_prototypes.Clear();
	Assets::Get_Animation_Cache().Clear_Named();
	m_skeletons.Clear();
	Release_All_Textures();
}

void W3DAssetCatalog::Release_Unused_Assets()
{
	Release_Unused_Textures();
}

void W3DAssetCatalog::Free_Assets_With_Exclusion_List(std::span<const std::string> names)
{
	std::unordered_set<std::string> retained;
	retained.reserve(names.size());
	for (const std::string &name : names)
		retained.insert(name);

	m_prototypes.Erase_If([&](const Prototype &prototype) {
		const auto key = Assets::W3D::W3D_Model_Retention_Key(prototype.name);
		return !key || !retained.contains(std::string(*key));
	});

	Assets::Get_Animation_Cache().Remove_Unused_If([&](const Assets::AnimationClip &animation) {
		const auto key = Assets::W3D::W3D_Animation_Retention_Key(animation.name);
		return !key || !retained.contains(std::string(*key));
	});
	m_skeletons.Remove_If([&](const Assets::ModelRigDesc &skeleton) {
		return !retained.contains(skeleton.skeleton_name);
	});
	Release_Unused_Textures();
}

void W3DAssetCatalog::Create_Asset_List(std::vector<std::string> &names) const
{
	for (std::size_t index = 0; index < m_prototypes.Size(); ++index) {
		const Prototype *prototype = m_prototypes.At(index);
		if (prototype == nullptr || prototype->name.find('#') != std::string::npos ||
			prototype->name.find('.') != std::string::npos)
			continue;
		names.push_back(prototype->name);
	}

	Assets::Get_Animation_Cache().Visit_Named([&](const Assets::AnimationClip &animation) {
		const std::size_t separator = animation.name.find('.');
		if (separator != std::string::npos)
			names.emplace_back(animation.name.substr(separator + 1));
	});
}

bool W3DAssetCatalog::Register_Model_Decoder(std::uint32_t chunk_id, ModelDecoder decoder)
{
	if (decoder == nullptr)
		return false;
	return m_model_decoders.emplace(chunk_id, decoder).second;
}

bool W3DAssetCatalog::Install_Reserved_Model_Factory(std::unique_ptr<Prototype> factory)
{
	if (!factory || m_reserved_model_factory || Find_Prototype(factory->name) != nullptr)
		return false;
	m_reserved_model_factory = std::move(factory);
	return true;
}

bool W3DAssetCatalog::Add_Prototype(std::unique_ptr<Prototype> factory)
{
	if (!factory)
		return false;
	const std::string identity = factory->name;
	m_prototypes.Insert(identity, std::move(factory));
	return true;
}

std::unique_ptr<W3DAssetCatalog::Prototype> W3DAssetCatalog::Release_Prototype(
	Prototype *factory) noexcept
{
	if (factory == nullptr || factory == m_reserved_model_factory.get())
		return {};
	return m_prototypes.Release(factory);
}

bool W3DAssetCatalog::Remove_Prototype(std::string_view name)
{
	auto owner = Release_Prototype(Find_Prototype(name));
	if (!owner)
		return false;
	owner.reset();
	return true;
}

W3DAssetCatalog::Prototype *W3DAssetCatalog::Find_Prototype(std::string_view name)
{
	if (m_reserved_model_factory && Names_Equal_No_Case(name, m_reserved_model_factory->name))
		return m_reserved_model_factory.get();
	return m_prototypes.Find(name);
}

const W3DAssetCatalog::Prototype *W3DAssetCatalog::Find_Prototype(
	std::string_view name) const
{
	if (m_reserved_model_factory && Names_Equal_No_Case(name, m_reserved_model_factory->name))
		return m_reserved_model_factory.get();
	return m_prototypes.Find(name);
}

bool W3DAssetCatalog::Has_Loaded_File_Prototype(std::string_view filename) const
{
	const std::size_t extension = filename.find_last_of('.');
	const std::string_view identity = extension == std::string_view::npos
		? filename
		: filename.substr(0, extension);
	return !identity.empty() && Find_Prototype(identity) != nullptr;
}

void W3DAssetCatalog::Write_Report() const noexcept
{
#ifdef WWDEBUG
	if (!m_report.Reporting_Enabled())
		return;

	const std::string report = m_report.Format_Report();
	RawFileClass raw_log_file("asset_report.txt");
	if (!raw_log_file.Create() || !raw_log_file.Open(RawFileClass::WRITE))
		return;
	raw_log_file.Write(report.data(), static_cast<int>(report.size()));
	raw_log_file.Close();
#endif
}

bool W3DAssetCatalog::Load_Prototype(ChunkLoadClass &chunks)
{
	const auto decoder = m_model_decoders.find(chunks.Cur_Chunk_ID());
	if (decoder == m_model_decoders.end() || decoder->second == nullptr)
		return false;

	Prototype *raw_factory = decoder->second(chunks);
	std::unique_ptr<Prototype> factory(raw_factory);
	if (!factory)
		return false;
	if (Find_Prototype(factory->name) != nullptr)
		return false;
	return Add_Prototype(std::move(factory));
}

bool W3DAssetCatalog::Load_Skeleton(ChunkLoadClass &chunks)
{
	const std::size_t length = chunks.Cur_Chunk_Length();
	std::vector<std::byte> bytes(length);
	if (chunks.Read(bytes.data(), static_cast<unsigned>(bytes.size())) != bytes.size())
		return false;

	Assets::ModelRigDesc description;
	std::string error;
	if (!Assets::W3D::W3DRead_Hierarchy(bytes, description, error))
		return false;
	return m_skeletons.Publish(std::move(description), error).Is_Valid();
}

void W3DAssetCatalog::Load_Animation_Chunk(ChunkLoadClass &chunks)
{
	const std::size_t length = chunks.Cur_Chunk_Length();
	std::vector<std::byte> bytes(length);
	if (chunks.Read(bytes.data(), static_cast<unsigned>(bytes.size())) != bytes.size())
		return;

	auto &cache = Assets::Get_Animation_Cache();
	std::string error;
	if (chunks.Cur_Chunk_ID() != Assets::W3D::W3DChunkMorphAnimation) {
		Assets::ModelAnimationDesc description;
		const bool compressed = chunks.Cur_Chunk_ID() == Assets::W3D::W3DChunkCompressedAnimation;
		if (!Assets::W3D::W3DRead_Animation(bytes, compressed, description, error))
			return;
		const auto skeleton = Get_Skeleton(description.skeleton_name.c_str());
		const auto *resolved = Resolve_Skeleton(skeleton);
		if (resolved == nullptr)
			return;
		cache.Publish(std::move(description), static_cast<std::uint32_t>(resolved->bones.size()),
			compressed ? Assets::AnimationSampling::Keyed : Assets::AnimationSampling::Consecutive,
			error);
		return;
	}

	Assets::ModelPoseAnimationDesc description;
	if (!Assets::W3D::W3DRead_Pose_Animation(bytes, description, error))
		return;
	const auto skeleton = Get_Skeleton(description.skeleton_name.c_str());
	const auto *resolved = Resolve_Skeleton(skeleton);
	if (resolved == nullptr)
		return;

	struct AcquiredSources final
	{
		std::vector<Assets::AnimationAssetHandle> handles;
		~AcquiredSources()
		{
			for (const auto handle : handles)
				Assets::Get_Animation_Cache().Release(handle);
		}
	} sources;
	sources.handles.reserve(description.channels.size());
	for (const auto &channel : description.channels) {
		const auto handle = Acquire_Animation(channel.animation_name.c_str());
		if (!handle.Is_Valid())
			return;
		sources.handles.push_back(handle);
	}
	cache.Publish_Pose(std::move(description), static_cast<std::uint32_t>(resolved->bones.size()),
		sources.handles, error);
}

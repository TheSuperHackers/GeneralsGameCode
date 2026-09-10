#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "WWLib/chunkio.h"
#include "WWLib/ffactory.h"
#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"

import Assets.Adapters.W3D.Chunks;
import Assets.Adapters.W3D.RequestPolicy;
import Assets.Adapters.W3D.Rig;
import Assets.Cache.Animations;
import Assets.Cache.AssetReport;
import Assets.Cache.Skeletons;
import Assets.Images.PixelEncoding;
import Graphics.Scene.Models.Factory;
import Graphics.Scene.Models.FactoryStore;

// The catalog is the small game-facing bridge around native W3D source files.
// Asset descriptions and generic caches remain in engine/assets; graphics owns
// the native renderable implementation and the catalog only retains handles
// and factories needed by current game callers.
class W3DAssetCatalog final
{
public:
	using Prototype = Graphics::ModelFactory<W3DRenderObject>;
	using PrototypeStore = Graphics::ModelFactoryStore<Prototype>;
	using ModelDecoder = Prototype * (*)(ChunkLoadClass &);
	using TextureRequestPolicy = std::function<void(std::string_view, bool &allow_reduction)>;
	using LoadedFileObserver = std::function<void(std::string_view)>;

	explicit W3DAssetCatalog(
		FileFactoryClass *file_factory = nullptr,
		TextureRequestPolicy texture_policy = {},
		LoadedFileObserver loaded_file_observer = {});
	~W3DAssetCatalog();

	W3DAssetCatalog(const W3DAssetCatalog &) = delete;
	W3DAssetCatalog &operator=(const W3DAssetCatalog &) = delete;

	// A display normally owns the catalog. The accessor is limited to native
	// persistence paths that have no display reference; callers with a display
	// should pass its catalog explicitly.
	static W3DAssetCatalog *Get_Instance() noexcept;

	bool Load_3D_Assets(const char *filename);
	bool Load_3D_Assets(FileClass &file);

	W3DRenderObject *Create_Render_Obj(const char *name);
	bool Render_Obj_Exists(const char *name) const;

	Assets::AnimationAssetHandle Acquire_Animation(const char *name);
	Assets::SkeletonAssetHandle Get_Skeleton(const char *name);
	const Assets::ModelRigDesc *Resolve_Skeleton(Assets::SkeletonAssetHandle handle) const noexcept;

	W3DTextureHandle *Get_Texture(
		const char *filename,
		MipCountType mip_level_count = MIP_LEVELS_ALL,
		Assets::PixelEncoding texture_format = Assets::PixelEncoding::Unknown,
		bool allow_compression = true,
		W3DTextureHandle::TexAssetType type = W3DTextureHandle::TEX_REGULAR,
		bool allow_reduction = true);
	// Consumes the supplied RefCountPtr's reference and returns a caller
	// reference. A name collision keeps the existing entry and releases the
	// incoming handle, matching the cache's first request policy.
	W3DTextureHandle *Adopt_Texture(RefCountPtr<W3DTextureHandle> &texture);
	void Set_Texture_Request_Policy(TextureRequestPolicy policy)
	{
		m_texture_request_policy = std::move(policy);
	}
	void Set_Loaded_File_Observer(LoadedFileObserver observer)
	{
		m_loaded_file_observer = std::move(observer);
	}

	void Release_All_Textures() noexcept;
	void Release_Unused_Textures();
	void Release_Texture(W3DTextureHandle *texture);
	std::size_t Texture_Count() const noexcept { return m_textures.size(); }
	W3DTextureHandle *Find_Texture_Borrowed(std::string_view filename) const;
	void Visit_Textures(const std::function<void(W3DTextureHandle *)> &visit) const;

	void Free_Assets() noexcept;
	void Release_Unused_Assets();
	void Free_Assets_With_Exclusion_List(std::span<const std::string> names);
	void Create_Asset_List(std::vector<std::string> &names) const;

	bool Register_Model_Decoder(std::uint32_t chunk_id, ModelDecoder decoder);
	bool Install_Reserved_Model_Factory(std::unique_ptr<Prototype> factory);
	bool Add_Prototype(std::unique_ptr<Prototype> factory);
	std::unique_ptr<Prototype> Release_Prototype(Prototype *factory) noexcept;
	bool Remove_Prototype(std::string_view name);
	Prototype *Find_Prototype(std::string_view name);
	const Prototype *Find_Prototype(std::string_view name) const;
	std::size_t Prototype_Count() const noexcept { return m_prototypes.Size(); }
	Prototype *Prototype_At(std::size_t index) noexcept { return m_prototypes.At(index); }
	const Prototype *Prototype_At(std::size_t index) const noexcept { return m_prototypes.At(index); }

	bool Get_Load_On_Demand() const noexcept { return m_load_on_demand; }
	void Set_Load_On_Demand(bool enabled) noexcept { m_load_on_demand = enabled; }
	bool Get_Fog_On_Load() const noexcept { return m_fog_on_load; }
	void Set_Fog_On_Load(bool enabled) noexcept { m_fog_on_load = enabled; }

	Assets::AssetReport &Report() noexcept { return m_report; }
	const Assets::AssetReport &Report() const noexcept { return m_report; }
	Assets::SkeletonCache &Skeletons() noexcept { return m_skeletons; }
	const Assets::SkeletonCache &Skeletons() const noexcept { return m_skeletons; }

private:
	bool Load_Prototype(ChunkLoadClass &chunks);
	void Load_Animation_Chunk(ChunkLoadClass &chunks);
	bool Load_Skeleton(ChunkLoadClass &chunks);
	bool Has_Loaded_File_Prototype(std::string_view filename) const;
	void Write_Report() const noexcept;

	FileFactoryClass *m_file_factory = nullptr;
	TextureRequestPolicy m_texture_request_policy;
	LoadedFileObserver m_loaded_file_observer;
	PrototypeStore m_prototypes;
	std::unordered_map<std::uint32_t, ModelDecoder> m_model_decoders;
	std::unique_ptr<Prototype> m_reserved_model_factory;
	Assets::SkeletonCache m_skeletons;
	Assets::AssetReport m_report;
	std::unordered_map<std::string, W3DTextureHandle *> m_textures;
	bool m_load_on_demand = false;
	bool m_fog_on_load = false;

	static W3DAssetCatalog *s_instance;
};

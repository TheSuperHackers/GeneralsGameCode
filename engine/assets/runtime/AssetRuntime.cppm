module;

#include <memory>
#include <span>
#include <utility>

export module Assets.Runtime;

import Assets.Cache;
import Assets.Importers.Models;

namespace Assets
{

export bool Initialize_Asset_Runtime(
	AssetSource source,
	std::span<const std::shared_ptr<const IModelAdapter>> model_adapters);
export void Shutdown_Asset_Runtime() noexcept;
export AssetCache *Try_Get_Asset_Cache() noexcept;

namespace
{
std::unique_ptr<AssetCache> g_asset_cache;
}

bool Initialize_Asset_Runtime(
	AssetSource source,
	std::span<const std::shared_ptr<const IModelAdapter>> model_adapters)
{
	if (g_asset_cache != nullptr)
		return true;

	auto cache = std::make_unique<AssetCache>(std::move(source));
	for (const std::shared_ptr<const IModelAdapter> &adapter : model_adapters) {
		if (adapter == nullptr || !cache->Register_Model_Adapter(adapter))
			return false;
	}
	g_asset_cache = std::move(cache);
	return true;
}

void Shutdown_Asset_Runtime() noexcept
{
	g_asset_cache.reset();
}

AssetCache *Try_Get_Asset_Cache() noexcept
{
	return g_asset_cache.get();
}

}

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace generalszh::legacy
{
struct UpgradeIdentityInput
{
	std::string_view name;
	std::uint32_t legacyBit;
	std::uint32_t version{1};
};

// A published catalog is scoped to one simulation/content lifetime. Legacy bit
// positions remain compatibility metadata, never modern schema identity.
class UpgradeCatalogBridge final
{
public:
	static constexpr std::uint32_t LegacyBitCount = 512;
	static constexpr std::uint32_t InvalidId = UINT32_MAX;
	UpgradeCatalogBridge();
	~UpgradeCatalogBridge() noexcept;
	UpgradeCatalogBridge(const UpgradeCatalogBridge &) = delete;
	UpgradeCatalogBridge &operator=(const UpgradeCatalogBridge &) = delete;
	UpgradeCatalogBridge(UpgradeCatalogBridge &&) = delete;
	UpgradeCatalogBridge &operator=(UpgradeCatalogBridge &&) = delete;

	void Publish(std::span<const UpgradeIdentityInput> definitions);
	void Reset() noexcept;
	bool IsPublished() const noexcept;
	void RequireMutable() const;
	std::size_t Count() const;
	std::size_t MaskWordCount() const;
	std::uint64_t SchemaHash() const;
	std::uint64_t Key(std::uint32_t id) const;
	std::string_view Name(std::uint32_t id) const;
	std::uint32_t DenseId(std::uint32_t legacyBit) const;
	std::uint32_t LegacyBit(std::uint32_t id) const;

	// Explicit word encoding, not a memcpy of std::bitset. Outputs are unchanged
	// on malformed masks; input/output overlap is supported. No allocation occurs.
	void ImportMask(std::span<const std::uint64_t> legacyWords,
		std::span<std::uint64_t> modernWords) const;
	void ExportMask(std::span<const std::uint64_t> modernWords,
		std::span<std::uint64_t> legacyWords) const;

private:
	struct Impl;
	Impl *m_impl;
};
}

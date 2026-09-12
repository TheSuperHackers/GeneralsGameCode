#pragma once

#include <cstddef>
#include <cstdint>

namespace generalszh { class GameplayState; }
namespace generalszh::legacy
{
class ProductionMarkerLease;
class ProductionMarkerStore
{
public:
	ProductionMarkerStore();
	explicit ProductionMarkerStore(GameplayState &state);
	~ProductionMarkerStore() noexcept;
	ProductionMarkerStore(const ProductionMarkerStore &) = delete;
	ProductionMarkerStore &operator=(const ProductionMarkerStore &) = delete;
	ProductionMarkerStore(ProductionMarkerStore &&) = delete;
	ProductionMarkerStore &operator=(ProductionMarkerStore &&) = delete;

	std::size_t Count() const noexcept;
	void Reset();

private:
	struct Impl;
	Impl *m_impl;
	friend class ProductionMarkerLease;
};

class ProductionMarkerLease
{
public:
	explicit ProductionMarkerLease(ProductionMarkerStore &store);
	~ProductionMarkerLease() noexcept;
	ProductionMarkerLease(const ProductionMarkerLease &) = delete;
	ProductionMarkerLease &operator=(const ProductionMarkerLease &) = delete;
	ProductionMarkerLease(ProductionMarkerLease &&) = delete;
	ProductionMarkerLease &operator=(ProductionMarkerLease &&) = delete;

	bool Advance(std::uint32_t now, std::uint32_t duration);
	bool Begin(std::uint32_t now);
	std::uint32_t ReadLegacy() const;
	void RestoreLegacy(std::uint32_t tick);

private:
	ProductionMarkerStore &m_store;
	std::uint32_t m_index;
	std::uint32_t m_generation;
};
}

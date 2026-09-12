#pragma once

#include <cstddef>
#include <cstdint>

namespace generalszh { class GameplayState; }
namespace generalszh::legacy
{
struct ProductionDoorChanges
{
	std::uint8_t clear{0};
	std::uint8_t set{0};
};

struct ProductionDoorState
{
	std::uint32_t opening{0};
	std::uint32_t waiting{0};
	std::uint32_t closing{0};
	bool held{false};
};

class ProductionDoorLease;
class ProductionDoorStore
{
public:
	ProductionDoorStore();
	explicit ProductionDoorStore(GameplayState &state);
	~ProductionDoorStore() noexcept;
	ProductionDoorStore(const ProductionDoorStore &) = delete;
	ProductionDoorStore &operator=(const ProductionDoorStore &) = delete;
	ProductionDoorStore(ProductionDoorStore &&) = delete;
	ProductionDoorStore &operator=(ProductionDoorStore &&) = delete;

	std::size_t Count() const noexcept;
	void Reset();

private:
	struct Impl;
	Impl *m_impl;
	friend class ProductionDoorLease;
};

class ProductionDoorLease
{
public:
	explicit ProductionDoorLease(ProductionDoorStore &store);
	~ProductionDoorLease() noexcept;
	ProductionDoorLease(const ProductionDoorLease &) = delete;
	ProductionDoorLease &operator=(const ProductionDoorLease &) = delete;
	ProductionDoorLease(ProductionDoorLease &&) = delete;
	ProductionDoorLease &operator=(ProductionDoorLease &&) = delete;

	ProductionDoorChanges Advance(std::uint32_t now,
		std::uint32_t openingTime,
		std::uint32_t waitingTime,
		std::uint32_t closingTime);
	ProductionDoorChanges RequestExit(std::uint32_t now);
	ProductionDoorChanges SetHeld(bool held, std::uint32_t now);
	bool Waiting() const;
	ProductionDoorState ReadLegacy() const;
	void RestoreLegacy(const ProductionDoorState &state);

private:
	ProductionDoorStore &m_store;
	std::uint32_t m_index;
	std::uint32_t m_generation;
};
}

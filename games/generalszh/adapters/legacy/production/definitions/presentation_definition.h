#pragma once

#include <cstdint>

namespace generalszh::legacy
{
class ProductionPresentationDefinition final
{
public:
	ProductionPresentationDefinition(std::int32_t numDoorAnimations,
		std::uint32_t doorOpeningTicks,
		std::uint32_t doorWaitingTicks,
		std::uint32_t doorClosingTicks,
		std::uint32_t constructionCompleteTicks,
		std::uint32_t ticksPerSecond);
	~ProductionPresentationDefinition() noexcept;
	ProductionPresentationDefinition(const ProductionPresentationDefinition &) = delete;
	ProductionPresentationDefinition &operator=(const ProductionPresentationDefinition &) = delete;
	ProductionPresentationDefinition(ProductionPresentationDefinition &&) = delete;
	ProductionPresentationDefinition &operator=(ProductionPresentationDefinition &&) = delete;

	std::int32_t NumDoorAnimations() const noexcept;
	std::uint32_t DoorOpeningTicks() const noexcept;
	std::uint32_t DoorWaitingTicks() const noexcept;
	std::uint32_t DoorClosingTicks() const noexcept;
	std::uint32_t ConstructionCompleteTicks() const noexcept;
	std::uint32_t TicksPerSecond() const noexcept;
	std::uint64_t Fingerprint() const noexcept;

private:
	struct Impl;
	Impl *m_impl;
};
}

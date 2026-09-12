module;

#include "presentation_definition.h"
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>

export module games.generalszh.adapters.legacy.production.definitions.presentation_definition;
import engine.time.simulation_time;
import games.generalszh.gameplay.production.definitions.presentation_definition;

extern "C++"
{
namespace generalszh::legacy
{
using ModernProductionPresentationDefinition =
	::generalszh::production::ProductionPresentationDefinition;

struct ProductionPresentationDefinition::Impl
{
	const ModernProductionPresentationDefinition runtime;
};

namespace
{
constexpr std::uint64_t NanosPerSecond = 1'000'000'000;
constexpr std::uint32_t MaximumAdapterRate = 1'000'000'000;
constexpr auto MaximumLegacyDurationProduct =
	static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()) *
	NanosPerSecond;

static_assert(MaximumLegacyDurationProduct <=
	static_cast<std::uint64_t>((std::numeric_limits<engine::time::Duration::rep>::max)()));

void ValidateRate(const std::uint32_t ticksPerSecond)
{
	if (ticksPerSecond == 0 || ticksPerSecond > MaximumAdapterRate)
		throw std::invalid_argument(
			"Production presentation tick rate must be between 1 and 1000000000");
}

engine::time::Duration ToRepresentativeDuration(const std::uint32_t ticks,
	const std::uint32_t ticksPerSecond) noexcept
{
	// The validated rate is at least one, and uint32_t ticks * 1e9 is below
	// int64_t's maximum. Integer division intentionally floors the authoring
	// duration; FixedStep::TicksFor then recovers the original tick count.
	const auto nanoseconds = (static_cast<std::uint64_t>(ticks) * NanosPerSecond) /
		ticksPerSecond;
	return engine::time::Duration{
		static_cast<engine::time::Duration::rep>(nanoseconds)};
}

ModernProductionPresentationDefinition MakeRuntime(
	const std::int32_t numDoorAnimations,
	const std::uint32_t doorOpeningTicks,
	const std::uint32_t doorWaitingTicks,
	const std::uint32_t doorClosingTicks,
	const std::uint32_t constructionCompleteTicks,
	const std::uint32_t ticksPerSecond)
{
	ValidateRate(ticksPerSecond);
	const engine::time::FixedStep step{ticksPerSecond};
	ModernProductionPresentationDefinition runtime{
		numDoorAnimations,
		ToRepresentativeDuration(doorOpeningTicks, ticksPerSecond),
		ToRepresentativeDuration(doorWaitingTicks, ticksPerSecond),
		ToRepresentativeDuration(doorClosingTicks, ticksPerSecond),
		ToRepresentativeDuration(constructionCompleteTicks, ticksPerSecond),
		step};
	assert(runtime.Doors().opening == doorOpeningTicks);
	assert(runtime.Doors().waiting == doorWaitingTicks);
	assert(runtime.Doors().closing == doorClosingTicks);
	assert(runtime.Marker().duration == constructionCompleteTicks);
	return runtime;
}
}

ProductionPresentationDefinition::ProductionPresentationDefinition(
	const std::int32_t numDoorAnimations,
	const std::uint32_t doorOpeningTicks,
	const std::uint32_t doorWaitingTicks,
	const std::uint32_t doorClosingTicks,
	const std::uint32_t constructionCompleteTicks,
	const std::uint32_t ticksPerSecond) :
	m_impl(new Impl{MakeRuntime(numDoorAnimations, doorOpeningTicks, doorWaitingTicks,
		doorClosingTicks, constructionCompleteTicks, ticksPerSecond)})
{
}

ProductionPresentationDefinition::~ProductionPresentationDefinition() noexcept
{
	delete m_impl;
}

std::int32_t ProductionPresentationDefinition::NumDoorAnimations() const noexcept
{
	return m_impl->runtime.NumDoorAnimations();
}

std::uint32_t ProductionPresentationDefinition::DoorOpeningTicks() const noexcept
{
	return static_cast<std::uint32_t>(m_impl->runtime.Doors().opening);
}

std::uint32_t ProductionPresentationDefinition::DoorWaitingTicks() const noexcept
{
	return static_cast<std::uint32_t>(m_impl->runtime.Doors().waiting);
}

std::uint32_t ProductionPresentationDefinition::DoorClosingTicks() const noexcept
{
	return static_cast<std::uint32_t>(m_impl->runtime.Doors().closing);
}

std::uint32_t ProductionPresentationDefinition::ConstructionCompleteTicks() const noexcept
{
	return static_cast<std::uint32_t>(m_impl->runtime.Marker().duration);
}

std::uint32_t ProductionPresentationDefinition::TicksPerSecond() const noexcept
{
	return m_impl->runtime.Step().TicksPerSecond();
}

std::uint64_t ProductionPresentationDefinition::Fingerprint() const noexcept
{
	return m_impl->runtime.Fingerprint();
}
}
}

module;

#include "build_definition.h"
#include <bit>
#include <cassert>
#include <cmath>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

export module games.generalszh.adapters.legacy.upgrades.definitions.build_definition;

import games.generalszh.gameplay.upgrades.definitions.build_definition;

extern "C++"
{
namespace generalszh::legacy
{
using ModernBuildDefinition = ::generalszh::upgrades::BuildDefinition;
using ModernSeconds = ::generalszh::upgrades::Seconds;

struct UpgradeBuildDefinition::Impl
{
	struct PendingOverride
	{
		float durationSeconds;
		std::int32_t cost;
	};

	struct Published
	{
		const ModernBuildDefinition definition;
		// This is derived compatibility data, not a second authoring value.
		const std::int32_t legacyFrames;

		Published(ModernBuildDefinition inDefinition,
			const std::int32_t inLegacyFrames)
			: definition(std::move(inDefinition)), legacyFrames(inLegacyFrames)
		{
		}
	};

	std::unique_ptr<const Published> runtime;
	std::optional<PendingOverride> pending;

	Impl()
		: runtime(std::make_unique<const Published>(
			ModernBuildDefinition{ModernSeconds{0.0f}, 0}, 0))
	{
	}

	static std::unique_ptr<const Published> CloneRuntime(const Impl &source)
	{
		if (source.pending.has_value())
			throw std::logic_error(
				"Cannot copy an upgrade build definition with an active override");
		return std::make_unique<const Published>(*source.runtime);
	}

	Impl(const Impl &source) : runtime(CloneRuntime(source)) {}

	PendingOverride &RequirePending()
	{
		if (!pending.has_value())
			throw std::logic_error("Upgrade build definition has no active override");
		return *pending;
	}
};

namespace
{
std::int32_t ToLegacyFrames(const float durationSeconds)
{
	// Reproduce binary32(seconds * 30), round-to-nearest-even, then truncate.
	// Configuration can be published outside GameLogic's setFPMode boundary;
	// integer arithmetic prevents ambient rounding/flush modes from deciding
	// this derived simulation value. This is cold external-input validation.
	const auto bits = std::bit_cast<std::uint32_t>(durationSeconds);
	const auto exponent = (bits >> 23) & 0xffu;
	const auto fraction = bits & 0x7fffffu;
	if (exponent == 0xffu || ((bits >> 31) != 0 && (bits & 0x7fffffffu) != 0))
		throw std::invalid_argument("Upgrade build duration must be finite and non-negative");

	std::uint64_t significand = (exponent == 0 ? fraction : fraction | 0x800000u);
	significand *= 30;
	const int droppedBits = static_cast<int>(std::bit_width(significand)) - 24;
	int scale = exponent == 0 ? -149 : static_cast<int>(exponent) - 150;
	if (droppedBits > 0)
	{
		const auto half = UINT64_C(1) << (droppedBits - 1);
		const auto remainder = significand & ((UINT64_C(1) << droppedBits) - 1);
		significand >>= droppedBits;
		significand += remainder > half || (remainder == half && (significand & 1) != 0);
		scale += droppedBits;
	}
	if (scale < 0)
		return scale <= -64 ? 0 : static_cast<std::int32_t>(significand >> -scale);
	constexpr std::uint64_t maximum = UINT64_C(2147483647);
	if (scale >= 31 || significand > (maximum >> scale))
		throw std::invalid_argument(
			"Upgrade build duration cannot be represented as legacy frames");
	return static_cast<std::int32_t>(significand << scale);
}
}

UpgradeBuildDefinition::UpgradeBuildDefinition() : m_impl(new Impl) {}

UpgradeBuildDefinition::~UpgradeBuildDefinition() noexcept
{
	delete m_impl;
}

UpgradeBuildDefinition::UpgradeBuildDefinition(const UpgradeBuildDefinition &source)
	: m_impl(new Impl{*source.m_impl})
{
}

UpgradeBuildDefinition &UpgradeBuildDefinition::operator=(
	const UpgradeBuildDefinition &source)
{
	if (this == &source)
		return *this;

	// Prepare the complete replacement first. Allocation or active-override
	// rejection therefore leaves this object's runtime and pending state intact.
	auto replacement = std::make_unique<Impl>(*source.m_impl);
	delete m_impl;
	m_impl = replacement.release();
	return *this;
}

void UpgradeBuildDefinition::BeginOverride()
{
	if (m_impl->pending.has_value())
		throw std::logic_error(
			"Upgrade build definition override is already active");

	m_impl->pending.emplace(Impl::PendingOverride{
		m_impl->runtime->definition.Duration().count(),
		m_impl->runtime->definition.Cost()});
}

void UpgradeBuildDefinition::SetDurationSeconds(const float seconds)
{
	m_impl->RequirePending().durationSeconds = seconds;
}

void UpgradeBuildDefinition::SetCost(const std::int32_t cost)
{
	m_impl->RequirePending().cost = cost;
}

void UpgradeBuildDefinition::Publish()
{
	auto &pending = m_impl->RequirePending();
	const auto legacyFrames = ToLegacyFrames(pending.durationSeconds);

	// The new definition and its compatibility projection are allocated as one
	// immutable record. The live owner is untouched until this construction
	// succeeds, preserving the previous runtime on every failure path.
	auto replacement = std::make_unique<const Impl::Published>(
		ModernBuildDefinition{ModernSeconds{pending.durationSeconds}, pending.cost},
		legacyFrames);
	m_impl->runtime = std::move(replacement);
	m_impl->pending.reset();
}

void UpgradeBuildDefinition::CancelOverride() noexcept
{
	m_impl->pending.reset();
}

float UpgradeBuildDefinition::DurationSeconds() const noexcept
{
	return m_impl->runtime->definition.Duration().count();
}

std::int32_t UpgradeBuildDefinition::Cost() const noexcept
{
	return m_impl->runtime->definition.Cost();
}

std::int32_t UpgradeBuildDefinition::LegacyFrames() const noexcept
{
	return m_impl->runtime->legacyFrames;
}

std::uint64_t UpgradeBuildDefinition::Fingerprint() const noexcept
{
	return m_impl->runtime->definition.Fingerprint();
}
}
}

module;

#include "disabled_processing_definition.h"
#include <cstdint>

export module games.generalszh.adapters.legacy.production.definitions.disabled_processing_definition;
import games.generalszh.gameplay.status.definitions.disabled_processing_definition;

extern "C++"
{
namespace generalszh::legacy
{
using ModernDisabledProcessingDefinition =
	::generalszh::status::DisabledProcessingDefinition;

struct DisabledProcessingDefinition::Impl
{
	const ModernDisabledProcessingDefinition runtime;
};

DisabledProcessingDefinition::DisabledProcessingDefinition(
	const std::uint16_t allowedMask) :
	m_impl(new Impl{ModernDisabledProcessingDefinition{allowedMask}})
{
}

DisabledProcessingDefinition::~DisabledProcessingDefinition() noexcept
{
	delete m_impl;
}

std::uint16_t DisabledProcessingDefinition::AllowedMask() const noexcept
{
	return m_impl->runtime.AllowedMask();
}

std::uint64_t DisabledProcessingDefinition::Fingerprint() const noexcept
{
	return m_impl->runtime.Fingerprint();
}
}
}

module;
#include "production_definition.h"
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

export module games.generalszh.adapters.legacy.production.definitions.production_definition;
import games.generalszh.gameplay.production.definitions.production_definition;

extern "C++"
{
namespace generalszh::legacy
{
using ModernProductionDefinition = ::generalszh::production::ProductionDefinition;
using QuantityRuleInput = ::generalszh::production::QuantityRuleInput;

struct ProductionDefinition::Impl
{
	struct PendingQuantity
	{
		std::string templateName;
		std::int32_t quantity;
	};

	std::optional<std::uint32_t> pendingQueueLimit{9};
	std::vector<PendingQuantity> pending;
	std::unique_ptr<ModernProductionDefinition> runtime;
	bool finalized{false};
};

namespace
{
void RequireMutable(const bool finalized)
{
	if (finalized)
		throw std::logic_error("Legacy production definition is already finalized");
}

void RequireFinalized(const bool finalized)
{
	if (!finalized)
		throw std::logic_error("Legacy production definition has not been finalized");
}
}

ProductionDefinition::ProductionDefinition() : m_impl(new Impl) {}

ProductionDefinition::~ProductionDefinition() noexcept
{
	delete m_impl;
}

void ProductionDefinition::SetQueueLimit(const std::uint32_t limit)
{
	RequireMutable(m_impl->finalized);
	m_impl->pendingQueueLimit = limit;
}

void ProductionDefinition::AddQuantity(const std::string_view templateName,
	const std::int32_t quantity)
{
	RequireMutable(m_impl->finalized);
	if (quantity < 0)
		throw std::invalid_argument("Production quantity cannot be negative");
	if (templateName.empty() || templateName.find('\0') != std::string_view::npos)
		throw std::invalid_argument("Production template name must be non-empty and NUL-free");

	// Copy the text at the legacy boundary. The caller may be passing a view into
	// transient configuration storage that will not survive startup finalization.
	m_impl->pending.push_back(Impl::PendingQuantity{std::string(templateName), quantity});
}

void ProductionDefinition::Finalize()
{
	if (m_impl->finalized)
		return;

	std::vector<QuantityRuleInput> inputs;
	inputs.reserve(m_impl->pending.size());
	for (const auto &pending : m_impl->pending)
		inputs.push_back(QuantityRuleInput{pending.templateName, pending.quantity});

	// Construct the immutable definition completely before publishing it. If the
	// modern constructor rejects the input or allocation fails, pending input and
	// the unfinalized state remain intact.
	auto runtime = std::make_unique<ModernProductionDefinition>(
		*m_impl->pendingQueueLimit,
		std::span<const QuantityRuleInput>(inputs.data(), inputs.size()));

	m_impl->runtime = std::move(runtime);
	m_impl->finalized = true;
	m_impl->pendingQueueLimit.reset();
	std::vector<Impl::PendingQuantity>().swap(m_impl->pending);
}

bool ProductionDefinition::IsFinalized() const noexcept
{
	return m_impl->finalized;
}

std::uint32_t ProductionDefinition::QueueLimit() const
{
	RequireFinalized(m_impl->finalized);
	return m_impl->runtime->QueueLimit();
}

std::span<const std::int32_t> ProductionDefinition::Quantities() const
{
	RequireFinalized(m_impl->finalized);
	return m_impl->runtime->Quantities();
}

std::string_view ProductionDefinition::Name(const std::size_t index) const
{
	RequireFinalized(m_impl->finalized);
	return m_impl->runtime->Name(index);
}

std::uint64_t ProductionDefinition::Fingerprint() const
{
	RequireFinalized(m_impl->finalized);
	return m_impl->runtime->Fingerprint();
}
}
}

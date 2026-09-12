#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace generalszh::legacy
{
class ProductionDefinition final
{
public:
	ProductionDefinition();
	~ProductionDefinition() noexcept;
	ProductionDefinition(const ProductionDefinition &) = delete;
	ProductionDefinition &operator=(const ProductionDefinition &) = delete;
	ProductionDefinition(ProductionDefinition &&) = delete;
	ProductionDefinition &operator=(ProductionDefinition &&) = delete;

	void SetQueueLimit(std::uint32_t limit);
	void AddQuantity(std::string_view templateName, std::int32_t quantity);
	void Finalize();
	bool IsFinalized() const noexcept;
	std::uint32_t QueueLimit() const;
	std::span<const std::int32_t> Quantities() const;
	std::string_view Name(std::size_t index) const;
	std::uint64_t Fingerprint() const;

private:
	struct Impl;
	Impl *m_impl;
};
}

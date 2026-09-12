#pragma once

#include <cstdint>

namespace generalszh::legacy
{
class DisabledProcessingDefinition final
{
public:
	explicit DisabledProcessingDefinition(std::uint16_t allowedMask);
	~DisabledProcessingDefinition() noexcept;
	DisabledProcessingDefinition(const DisabledProcessingDefinition &) = delete;
	DisabledProcessingDefinition &operator=(const DisabledProcessingDefinition &) = delete;
	DisabledProcessingDefinition(DisabledProcessingDefinition &&) = delete;
	DisabledProcessingDefinition &operator=(DisabledProcessingDefinition &&) = delete;

	std::uint16_t AllowedMask() const noexcept;
	std::uint64_t Fingerprint() const noexcept;

private:
	struct Impl;
	Impl *m_impl;
};
}

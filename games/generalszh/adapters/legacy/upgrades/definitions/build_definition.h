#pragma once

#include <cstdint>

namespace generalszh::legacy
{
class UpgradeBuildDefinition final
{
public:
	UpgradeBuildDefinition();
	~UpgradeBuildDefinition() noexcept;

	// Copying is a deep copy of the complete published runtime. A copy from an
	// object with an active override is rejected instead of copying staged data.
	UpgradeBuildDefinition(const UpgradeBuildDefinition &);
	UpgradeBuildDefinition &operator=(const UpgradeBuildDefinition &);
	UpgradeBuildDefinition(UpgradeBuildDefinition &&) = delete;
	UpgradeBuildDefinition &operator=(UpgradeBuildDefinition &&) = delete;

	void BeginOverride();
	void SetDurationSeconds(float seconds);
	void SetCost(std::int32_t cost);
	void Publish();
	void CancelOverride() noexcept;

	float DurationSeconds() const noexcept;
	std::int32_t Cost() const noexcept;
	std::int32_t LegacyFrames() const noexcept;
	std::uint64_t Fingerprint() const noexcept;

private:
	struct Impl;
	Impl *m_impl;
};
}

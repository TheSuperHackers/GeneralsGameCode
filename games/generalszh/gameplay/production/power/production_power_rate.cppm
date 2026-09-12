module;
#include <algorithm>
#include <cassert>
#include <cmath>

export module games.generalszh.gameplay.production.power.production_power_rate;

export namespace generalszh::production
{
struct PowerRateConfig
{
	float minimumSpeed{0};
	float maximumSpeed{0};
	float penaltyModifier{0};
};

inline float CalculatePowerRate(float supplyRatio, const PowerRateConfig &config) noexcept
{
	assert(std::isfinite(supplyRatio));
	assert(std::isfinite(config.minimumSpeed));
	assert(std::isfinite(config.maximumSpeed));
	assert(std::isfinite(config.penaltyModifier));

	if (supplyRatio > 1.0f)
		supplyRatio = 1.0f;

	float shortage = 1.0f - supplyRatio;
	shortage *= config.penaltyModifier;
	float rate = 1.0f - shortage;
	rate = std::max(rate, config.minimumSpeed);

	if (supplyRatio < 1.0f)
		rate = std::min(rate, config.maximumSpeed);

	if (rate <= 0.0f)
		rate = 0.01f;

	return rate;
}
}

module;
#include <cstdint>
#include <stdexcept>
export module games.generalszh.gameplay.economy.algorithms.income_history;
export import games.generalszh.gameplay.economy.components.income_history;

export namespace generalszh::economy
{
inline void RecordIncome(IncomeHistory &history, std::uint32_t amount)
{
	if (history.current >= history.buckets.size()) throw std::out_of_range("Invalid income bucket");
	history.buckets[history.current] += amount;
	history.total += amount; // Explicit unsigned compatibility arithmetic.
}
inline void AdvanceIncomeBucket(IncomeHistory &history, std::uint32_t next)
{
	if (next >= history.buckets.size()) throw std::out_of_range("Invalid next income bucket");
	if (next == history.current) return;
	history.total -= history.buckets[next];
	history.current = next;
	history.buckets[next] = 0;
}
inline void ValidateIncomeHistory(const IncomeHistory &history)
{
	if (history.current >= history.buckets.size()) throw std::out_of_range("Invalid income bucket");
	std::uint32_t total = 0;
	for (const auto amount : history.buckets) total += amount;
	if (total != history.total) throw std::invalid_argument("Income total disagrees with its buckets");
}
}

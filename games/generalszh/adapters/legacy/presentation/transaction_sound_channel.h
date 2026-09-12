#pragma once
#include <cstddef>
#include <cstdint>
#include <memory_resource>
#include <span>
namespace generalszh::presentation { struct TransactionSound; }
namespace generalszh::legacy
{
// Narrow, allocation-free batch ABI for legacy translation units. The injected
// consumer/context outlive the channel. This is not a per-event callback chain.
struct TransactionSoundConsumer
{
	void *context;
	void (*consume)(void *, std::span<const presentation::TransactionSound>);
};
class TransactionSoundChannel
{
public:
	TransactionSoundChannel(TransactionSoundConsumer consumer, std::size_t capacity, std::pmr::memory_resource &memory);
	~TransactionSoundChannel() noexcept;
	TransactionSoundChannel(const TransactionSoundChannel &) = delete;
	TransactionSoundChannel &operator=(const TransactionSoundChannel &) = delete;
	void Finalize();
	bool IsFinalized() const noexcept;
	bool IsFailed() const noexcept;
	void Begin(std::uint64_t tick);
	void Deposit(std::int32_t player);
	void Withdraw(std::int32_t player);
	void Commit();
private:
	struct Impl;
	Impl *m_impl;
};
}

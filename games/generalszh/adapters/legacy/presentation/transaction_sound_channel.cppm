module;
#include "transaction_sound_channel.h"
#include <stdexcept>
export module games.generalszh.adapters.legacy.presentation.transaction_sound_channel;
import games.generalszh.presentation.economy.transaction_sound;
import engine.events.storage.event_batch;

extern "C++"
{
namespace generalszh::legacy
{
using namespace engine::events;
using presentation::TransactionSound;
using presentation::TransactionSoundKind;
struct TransactionSoundChannel::Impl
{
	TransactionSoundConsumer consumer;
	MessageRegistry registry;
	RecordedBatch<TransactionSound> pending;
	PublishedBatch<TransactionSound> published;
	bool committing{false}, failed{false};
	Impl(TransactionSoundConsumer output, std::size_t capacity, std::pmr::memory_resource &memory) :
		consumer(output), pending(capacity, memory), published(capacity, 1, memory)
	{
		if (!consumer.context || !consumer.consume) throw std::invalid_argument("Transaction sound consumer is required");
		registry.Register<TransactionSound>();
	}
	void RequireAvailable() const
	{
		if (failed) throw std::logic_error("Failed transaction sound channel requires teardown");
		if (committing) throw std::logic_error("Transaction sound publication is not reentrant");
		if (!registry.IsFinalized()) throw std::logic_error("Finalize transaction sound schema before use");
	}
	void Record(std::int32_t player, TransactionSoundKind kind)
	{
		RequireAvailable();
		try { pending.Emplace(TransactionSound{player, kind}); }
		catch (...) { if (pending.State() == RecordingState::Failed) failed = true; throw; }
	}
};
TransactionSoundChannel::TransactionSoundChannel(TransactionSoundConsumer consumer, std::size_t capacity, std::pmr::memory_resource &memory) :
	m_impl(new Impl(consumer, capacity, memory)) {}
TransactionSoundChannel::~TransactionSoundChannel() noexcept { delete m_impl; }
void TransactionSoundChannel::Finalize() { m_impl->registry.Finalize(); }
bool TransactionSoundChannel::IsFinalized() const noexcept { return m_impl->registry.IsFinalized(); }
bool TransactionSoundChannel::IsFailed() const noexcept { return m_impl->failed; }
void TransactionSoundChannel::Begin(std::uint64_t tick)
{
	m_impl->RequireAvailable();
	// One legacy main-thread producer. This does not assign ECS worker/job IDs.
	m_impl->pending.Begin({{tick, 0}, 0, 0, 0});
}
void TransactionSoundChannel::Deposit(std::int32_t player) { m_impl->Record(player, TransactionSoundKind::Deposit); }
void TransactionSoundChannel::Withdraw(std::int32_t player) { m_impl->Record(player, TransactionSoundKind::Withdraw); }
void TransactionSoundChannel::Commit()
{
	auto &impl = *m_impl;
	impl.RequireAvailable();
	if (impl.pending.State() != RecordingState::Recording) throw std::logic_error("No transaction sound recording to commit");
	impl.committing = true;
	try
	{
		impl.pending.Seal();
		RecordedBatch<TransactionSound> *inputs[]{&impl.pending};
		impl.published.Publish(impl.pending.Order().boundary, inputs);
		impl.consumer.consume(impl.consumer.context, impl.published.Values());
		impl.published.Release();
		impl.committing = false;
	}
	catch (...)
	{
		// The consumer may already have submitted a prefix to audio. Never retry.
		impl.published.Release(); impl.pending.Discard();
		impl.committing = false; impl.failed = true;
		throw;
	}
}
}
}

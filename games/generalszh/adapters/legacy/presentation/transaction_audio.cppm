module;
#include <span>
#include <stdexcept>
export module games.generalszh.adapters.legacy.presentation.transaction_audio;
export import games.generalszh.presentation.economy.transaction_sound;

export namespace generalszh::legacy
{
// Audio is explicitly injected. Duck typing permits the actual legacy manager
// and a headless probe without importing legacy headers into the module graph.
// Instantiate only at the adapter boundary, never in a simulation chunk loop.
template<class Audio>
void ConsumeTransactionSounds(Audio &audio, std::span<const presentation::TransactionSound> requests)
{
	using presentation::TransactionSoundKind;
	for (const auto &request : requests)
	{
		if (request.kind != TransactionSoundKind::Deposit && request.kind != TransactionSoundKind::Withdraw)
			throw std::invalid_argument("Invalid transaction sound kind");
		const auto &definition = request.kind == TransactionSoundKind::Deposit ?
			audio.getMiscAudio()->m_moneyDepositSound : audio.getMiscAudio()->m_moneyWithdrawSound;
		auto volume = audio.getAudioSettings()->m_preferredMoneyTransactionVolume;
		volume *= definition.getVolume();
		if (volume <= 0.0f) continue;
		auto event = definition;
		event.setPlayerIndex(request.playerIndex);
		event.setVolume(volume);
		audio.addAudioEvent(&event);
	}
}
}

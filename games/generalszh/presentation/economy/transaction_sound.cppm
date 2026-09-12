module;
#include <cstdint>
#include <string_view>
export module games.generalszh.presentation.economy.transaction_sound;
export import engine.events.schema.message_registry;

export namespace generalszh::presentation
{
extern "C++"
{
enum class TransactionSoundKind : std::uint8_t { Deposit, Withdraw };
// A derived presentation request. It cannot debit/credit an account. No audio
// object, event-name string, resource handle, volume setting or process pointer.
struct TransactionSound
{
	std::int32_t playerIndex{0};
	TransactionSoundKind kind{TransactionSoundKind::Deposit};
};
}
}
export namespace engine::events
{
template<> struct MessageTraits<generalszh::presentation::TransactionSound>
{
	static constexpr std::string_view StableName = "games.generalszh.presentation.transaction_sound";
	static constexpr std::uint32_t Version = 1;
	static constexpr MessageKind Kind = MessageKind::Presentation;
	static constexpr RecordPolicy Recording = RecordPolicy::Transient;
};
}

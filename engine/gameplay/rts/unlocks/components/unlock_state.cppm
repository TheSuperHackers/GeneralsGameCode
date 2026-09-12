module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.unlocks.components.unlock_state;
export import engine.ecs.core.component_registry;
export import engine.gameplay.rts.unlocks.definitions.unlock_definition;

export namespace engine::gameplay::rts::unlocks
{
struct UnlockState final
{
	std::uint64_t credits{};
	std::array<std::uint64_t, UnlockWordCount> owned{};
	std::array<std::uint64_t, UnlockWordCount> hidden{};
	std::array<std::uint64_t, UnlockWordCount> disabled{};

	static constexpr bool IsAddressable(const UnlockId id) noexcept
	{
		return id.value < MaxUnlockDefinitions;
	}

	static constexpr std::size_t Word(const UnlockId id) noexcept
	{
		return static_cast<std::size_t>(id.value) / 64u;
	}

	static constexpr std::uint64_t Mask(const UnlockId id) noexcept
	{
		return UINT64_C(1) << (static_cast<std::size_t>(id.value) % 64u);
	}

	static bool Test(const std::array<std::uint64_t, UnlockWordCount> &bits,
		const UnlockId id) noexcept
	{
		return IsAddressable(id) && (bits[Word(id)] & Mask(id)) != 0;
	}

	static bool Set(std::array<std::uint64_t, UnlockWordCount> &bits,
		const UnlockId id, const bool value) noexcept
	{
		if (!IsAddressable(id))
			return false;
		const auto mask = Mask(id);
		if (value)
			bits[Word(id)] |= mask;
		else
			bits[Word(id)] &= ~mask;
		return true;
	}

	bool IsOwned(const UnlockId id) const noexcept { return Test(owned, id); }
	bool IsHidden(const UnlockId id) const noexcept { return Test(hidden, id); }
	bool IsDisabled(const UnlockId id) const noexcept { return Test(disabled, id); }
	bool SetOwned(const UnlockId id, const bool value = true) noexcept { return Set(owned, id, value); }
	bool SetHidden(const UnlockId id, const bool value = true) noexcept { return Set(hidden, id, value); }
	bool SetDisabled(const UnlockId id, const bool value = true) noexcept { return Set(disabled, id, value); }
};
} // namespace engine::gameplay::rts::unlocks

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::unlocks::UnlockState>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.unlocks.state";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
} // namespace ecs

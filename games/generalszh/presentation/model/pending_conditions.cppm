module;

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

export module games.generalszh.presentation.model.pending_conditions;

export import engine.ecs.core.component_registry;
export import games.generalszh.presentation.model.model_condition;

export namespace generalszh::presentation
{
struct PendingConditionClear
{
	ConditionMask mask;
};

struct PendingConditionSet
{
	ConditionMask mask;
};

struct PendingConditionDirty
{
	bool value{false};
};

struct PendingConditionView
{
	PendingConditionClear &clear;
	PendingConditionSet &set;
	PendingConditionDirty &dirty;
};

inline void QueueSet(PendingConditionView view, const ModelCondition condition) noexcept
{
	assert(static_cast<std::size_t>(condition) < ConditionCount);
	view.set.mask.Set(condition);
	view.dirty.value = true;
}

inline void QueueClear(PendingConditionView view, const ModelCondition condition) noexcept
{
	assert(static_cast<std::size_t>(condition) < ConditionCount);
	view.clear.mask.Set(condition);
	view.set.mask.Clear(condition);
	view.dirty.value = true;
}

inline void MarkDirty(PendingConditionView view) noexcept
{
	view.dirty.value = true;
}

inline void ResetPending(PendingConditionView view) noexcept
{
	view.clear.mask.Reset();
	view.set.mask.Reset();
	view.dirty.value = false;
}

static_assert(std::is_standard_layout_v<PendingConditionClear>);
static_assert(std::is_standard_layout_v<PendingConditionSet>);
static_assert(std::is_standard_layout_v<PendingConditionDirty>);
static_assert(std::is_trivially_copyable_v<PendingConditionClear>);
static_assert(std::is_trivially_copyable_v<PendingConditionSet>);
static_assert(std::is_trivially_copyable_v<PendingConditionDirty>);
static_assert(std::is_nothrow_default_constructible_v<PendingConditionClear>);
static_assert(std::is_nothrow_default_constructible_v<PendingConditionSet>);
static_assert(std::is_nothrow_default_constructible_v<PendingConditionDirty>);
static_assert(std::is_nothrow_copy_constructible_v<PendingConditionClear>);
static_assert(std::is_nothrow_copy_constructible_v<PendingConditionSet>);
static_assert(std::is_nothrow_copy_constructible_v<PendingConditionDirty>);
static_assert(noexcept(QueueSet(
	std::declval<PendingConditionView>(), ModelCondition::ACTIVELY_BEING_CONSTRUCTED)));
static_assert(noexcept(QueueClear(
	std::declval<PendingConditionView>(), ModelCondition::ACTIVELY_BEING_CONSTRUCTED)));
static_assert(noexcept(MarkDirty(std::declval<PendingConditionView>())));
static_assert(noexcept(ResetPending(std::declval<PendingConditionView>())));
}

export namespace ecs
{
template<>
struct ComponentTraits<generalszh::presentation::PendingConditionClear>
{
	static constexpr std::string_view StableName =
		"games.generalszh.presentation.model.pending_clear";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};

template<>
struct ComponentTraits<generalszh::presentation::PendingConditionSet>
{
	static constexpr std::string_view StableName =
		"games.generalszh.presentation.model.pending_set";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};

template<>
struct ComponentTraits<generalszh::presentation::PendingConditionDirty>
{
	static constexpr std::string_view StableName =
		"games.generalszh.presentation.model.pending_dirty";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}

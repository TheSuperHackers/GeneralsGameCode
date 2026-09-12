module;

#include <type_traits>

export module engine.ecs.query.access;

export import engine.ecs.core.component_registry;

export namespace ecs
{

enum class AccessMode
{
	Read,
	Write,
	None
};

struct AccessDescriptor
{
	ComponentId component{InvalidComponentId};
	AccessMode mode{AccessMode::None};
	bool optional{false};
	bool excluded{false};
};

template<typename T>
struct Read
{
	using ComponentType = std::remove_cv_t<T>;
	static constexpr AccessMode Mode = AccessMode::Read;
	static constexpr bool IsOptional = false;
	static constexpr bool IsExcluded = false;
};

template<typename T>
struct Write
{
	static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "ECS write queries require an unqualified component type");
	using ComponentType = std::remove_cv_t<T>;
	static constexpr AccessMode Mode = AccessMode::Write;
	static constexpr bool IsOptional = false;
	static constexpr bool IsExcluded = false;
};

template<typename T>
struct Optional
{
	using ComponentType = std::remove_cv_t<T>;
	static constexpr AccessMode Mode = AccessMode::Read;
	static constexpr bool IsOptional = true;
	static constexpr bool IsExcluded = false;
};

// Optional columns still declare their full write conflict, but do not require
// the component when matching an archetype. No per-entity lookup is introduced.
template<typename T>
struct OptionalWrite : Write<T>
{
	static constexpr bool IsOptional = true;
};

template<typename T>
struct Exclude
{
	using ComponentType = std::remove_cv_t<T>;
	static constexpr AccessMode Mode = AccessMode::None;
	static constexpr bool IsOptional = false;
	static constexpr bool IsExcluded = true;
};

} // namespace ecs

module;

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

export module engine.ecs.query.query;

export import engine.ecs.query.access;
export import engine.ecs.query.query_cache;

export namespace ecs
{

namespace detail
{

template<typename Wanted, typename... Terms>
struct ComponentIndex;

template<bool Match, typename Wanted, typename First, typename... Rest>
struct ComponentIndexStep;

template<typename Wanted, typename First, typename... Rest>
struct ComponentIndexStep<true, Wanted, First, Rest...>
{
	static constexpr std::size_t value = 0;
};

template<typename Wanted, typename First, typename... Rest>
struct ComponentIndexStep<false, Wanted, First, Rest...>
{
	static constexpr std::size_t value = 1 + ComponentIndex<Wanted, Rest...>::value;
};

template<typename Wanted, typename First, typename... Rest>
struct ComponentIndex<Wanted, First, Rest...> : ComponentIndexStep<
	std::is_same_v<std::remove_cv_t<Wanted>, typename First::ComponentType>,
	Wanted,
	First,
	Rest...>
{
};

template<typename Wanted>
struct ComponentIndex<Wanted>
{
	static_assert(!std::is_same_v<Wanted, Wanted>, "Requested component is not part of the ECS query");
};

template<typename... Terms>
struct UniqueComponents;

template<>
struct UniqueComponents<> : std::true_type
{
};

template<typename First, typename... Rest>
struct UniqueComponents<First, Rest...> : std::bool_constant<
	((!std::is_same_v<typename First::ComponentType, typename Rest::ComponentType>) && ...) &&
	UniqueComponents<Rest...>::value>
{
};

template<typename Term>
struct QueryView;

template<typename T>
struct QueryView<Read<T>>
{
	using Type = std::span<const typename Read<T>::ComponentType>;
};

template<typename T>
struct QueryView<Write<T>>
{
	using Type = std::span<typename Write<T>::ComponentType>;
};

template<typename T>
struct QueryView<Optional<T>>
{
	using Type = std::span<const typename Optional<T>::ComponentType>;
};

template<typename T>
struct QueryView<OptionalWrite<T>> : QueryView<Write<T>>
{
};

template<typename T>
struct QueryView<Exclude<T>>
{
	using Type = std::span<const typename Exclude<T>::ComponentType>;
};

} // namespace detail

template<typename... Terms>
class QueryChunk
{
public:
	using ViewTuple = std::tuple<typename detail::QueryView<Terms>::Type...>;

	QueryChunk(Chunk &chunk, const std::array<std::size_t, sizeof...(Terms)> &columns) :
		m_entities(chunk.Entities(), chunk.Size()),
		m_views(MakeViews(chunk, columns, std::index_sequence_for<Terms...>{}))
	{
	}

	std::size_t Count() const noexcept { return m_entities.size(); }
	std::span<const Entity> Entities() const noexcept { return m_entities; }

	template<typename T>
	auto Get() const
	{
		constexpr std::size_t index = detail::ComponentIndex<T, Terms...>::value;
		return std::get<index>(m_views);
	}

private:
	template<typename Term>
	static typename detail::QueryView<Term>::Type MakeView(Chunk &chunk, std::size_t column)
	{
		using Component = typename Term::ComponentType;
		if constexpr (Term::IsExcluded)
		{
			return typename detail::QueryView<Term>::Type{};
		}
		else if (column == std::numeric_limits<std::size_t>::max())
		{
			return typename detail::QueryView<Term>::Type{};
		}
		else if constexpr (Term::Mode == AccessMode::Write)
		{
			return std::span<Component>(static_cast<Component *>(chunk.ComponentData(column)), chunk.Size());
		}
		else
		{
			return std::span<const Component>(static_cast<const Component *>(chunk.ComponentData(column)), chunk.Size());
		}
	}

	template<std::size_t... Indices>
	static ViewTuple MakeViews(Chunk &chunk,
		const std::array<std::size_t, sizeof...(Terms)> &columns,
		std::index_sequence<Indices...>)
	{
		return ViewTuple{MakeView<Terms>(chunk, columns[Indices])...};
	}

	std::span<const Entity> m_entities;
	ViewTuple m_views;
};

template<typename... Terms>
class Query
{
public:
	using Chunk = QueryChunk<Terms...>;

	static_assert(sizeof...(Terms) > 0, "An ECS query must request at least one term");
	static_assert(detail::UniqueComponents<Terms...>::value,
		"An ECS query cannot contain the same component more than once");

	explicit Query(World &world) :
		m_world(&world),
		m_components{}
	{
		ResolveComponents();
	}

	Query(const Query &) = delete;
	Query &operator=(const Query &) = delete;

	// Prepares a deterministic snapshot of the currently matching non-empty
	// chunks. The snapshot is rebuilt on the calling thread before a scheduler
	// wave dispatches jobs; ExecutePreparedChunk() is read-only with respect to
	// the Query and is therefore safe for concurrent chunk jobs.
	std::size_t PrepareChunks()
	{
		if (m_cache.Refresh(*m_world, [this](const Archetype &archetype) {
			return Matches(archetype);
		}))
		{
			m_columnIndices.clear();
			m_columnIndices.reserve(m_cache.Matches().size());
			for (Archetype *archetype : m_cache.Matches())
				m_columnIndices.push_back(MakeColumnIndices(*archetype, std::index_sequence_for<Terms...>{}));
		}

		m_preparedChunks.clear();
		for (std::size_t archetypeIndex = 0; archetypeIndex < m_cache.Matches().size(); ++archetypeIndex)
		{
			Archetype *archetype = m_cache.Matches()[archetypeIndex];
			const std::array<std::size_t, sizeof...(Terms)> &columns = m_columnIndices[archetypeIndex];
			for (const std::unique_ptr<ecs::Chunk> &chunk : archetype->Chunks())
			{
				if (chunk->Size() != 0)
					m_preparedChunks.push_back(PreparedChunk{chunk.get(), columns});
			}
		}
		return m_preparedChunks.size();
	}

	std::size_t PreparedChunkCount() const noexcept { return m_preparedChunks.size(); }

	template<typename Function>
	void ForEachPreparedChunk(Function &&function) const
	{
		for (const PreparedChunk &prepared : m_preparedChunks)
			std::forward<Function>(function)(Chunk(*prepared.chunk, prepared.columns));
	}

	template<typename Function>
	void ExecutePreparedChunk(const std::size_t chunkIndex, Function &&function) const
	{
		if (chunkIndex >= m_preparedChunks.size())
			throw std::out_of_range("Invalid prepared ECS query chunk index");
		const PreparedChunk &prepared = m_preparedChunks[chunkIndex];
		std::forward<Function>(function)(Chunk(*prepared.chunk, prepared.columns));
	}

	template<typename Function>
	void ForEachChunk(Function &&function)
	{
		PrepareChunks();
		ForEachPreparedChunk(std::forward<Function>(function));
	}

	std::array<AccessDescriptor, sizeof...(Terms)> Accesses() const noexcept
	{
		return MakeAccesses(std::index_sequence_for<Terms...>{});
	}

	static std::array<AccessDescriptor, sizeof...(Terms)> ResolveAccesses(const ComponentRegistry &components)
	{
		return ResolveAccessesImpl(components, std::index_sequence_for<Terms...>{});
	}

	std::size_t CachedArchetypeCount() const noexcept { return m_cache.Matches().size(); }
	std::uint64_t CachedRevision() const noexcept { return m_cache.Revision(); }

private:
	struct PreparedChunk
	{
		ecs::Chunk *chunk{nullptr};
		std::array<std::size_t, sizeof...(Terms)> columns{};
	};

	void ResolveComponents()
	{
		if (!m_world->ComponentsFinalized())
			throw std::logic_error("ECS component registry must be finalized before constructing or executing a query");

		m_components = {m_world->Components().TryGet<typename Terms::ComponentType>()...};
		for (const ComponentId component : m_components)
		{
			if (component == InvalidComponentId)
				throw std::logic_error("ECS query component was not registered before finalization");
		}
	}

	template<typename Term>
	bool MatchesTerm(const Archetype &archetype, ComponentId component) const noexcept
	{
		if constexpr (Term::IsExcluded)
			return !archetype.Has(component);
		else if constexpr (Term::IsOptional)
			return true;
		else
			return archetype.Has(component);
	}

	template<std::size_t... Indices>
	bool MatchesImpl(const Archetype &archetype, std::index_sequence<Indices...>) const noexcept
	{
		return (MatchesTerm<Terms>(archetype, m_components[Indices]) && ...);
	}

	bool Matches(const Archetype &archetype) const noexcept
	{
		return MatchesImpl(archetype, std::index_sequence_for<Terms...>{});
	}

	template<typename Term>
	static AccessDescriptor ResolveAccess(const ComponentRegistry &components)
	{
		const ComponentId component = components.TryGet<typename Term::ComponentType>();
		if (component == InvalidComponentId)
			throw std::logic_error("ECS query component was not registered before finalization");
		return AccessDescriptor{component, Term::Mode, Term::IsOptional, Term::IsExcluded};
	}

	template<std::size_t... Indices>
	static std::array<AccessDescriptor, sizeof...(Terms)> ResolveAccessesImpl(
		const ComponentRegistry &components,
		std::index_sequence<Indices...>)
	{
		return {ResolveAccess<Terms>(components)...};
	}

	template<std::size_t... Indices>
	std::array<std::size_t, sizeof...(Terms)> MakeColumnIndices(const Archetype &archetype,
		std::index_sequence<Indices...>) const noexcept
	{
		return {archetype.ColumnIndex(m_components[Indices])...};
	}

	template<std::size_t... Indices>
	std::array<AccessDescriptor, sizeof...(Terms)> MakeAccesses(std::index_sequence<Indices...>) const noexcept
	{
		return {AccessDescriptor{m_components[Indices], Terms::Mode, Terms::IsOptional, Terms::IsExcluded}...};
	}

	World *m_world;
	std::array<ComponentId, sizeof...(Terms)> m_components;
	QueryCache m_cache;
	std::vector<std::array<std::size_t, sizeof...(Terms)>> m_columnIndices;
	std::vector<PreparedChunk> m_preparedChunks;
};

} // namespace ecs

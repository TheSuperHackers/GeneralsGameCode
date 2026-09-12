module;

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <memory>
#include <new>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

export module engine.ecs.system.system;

export import engine.ecs.commands.command_buffer;
export import engine.ecs.query.query;
export import engine.time.simulation_time;

export namespace ecs
{

using SystemId = std::uint32_t;
using SystemKey = std::uint64_t;

inline constexpr SystemId InvalidSystemId = (std::numeric_limits<SystemId>::max)();

enum class SystemPhase : std::uint8_t
{
	PreSimulation = 0,
	Simulation = 1,
	PostSimulation = 2
};

inline constexpr std::size_t SystemPhaseCount = 3;

constexpr std::size_t SystemPhaseIndex(const SystemPhase phase) noexcept
{
	return static_cast<std::size_t>(phase);
}

constexpr SystemKey HashSystemKey(const std::string_view stableName) noexcept
{
	SystemKey hash = 14695981039346656037ull;
	for (const char character : stableName)
	{
		hash ^= static_cast<SystemKey>(static_cast<std::uint8_t>(character));
		hash *= 1099511628211ull;
	}
	return hash;
}

template<typename... Systems>
struct SystemTypeList
{
};

template<typename T>
struct SystemTraits;

struct SystemDependency
{
	const std::type_info *type{&typeid(void)};
	SystemKey stableKey{};
	std::string_view stableName{};
};

class SystemContext;

struct SystemAccess
{
	const std::vector<ComponentId> &ReadComponents() const noexcept { return readComponents; }
	const std::vector<ComponentId> &WriteComponents() const noexcept { return writeComponents; }
	const std::vector<std::uint64_t> &ReadMask() const noexcept { return readMask; }
	const std::vector<std::uint64_t> &WriteMask() const noexcept { return writeMask; }

	bool ConflictsWith(const SystemAccess &other) const noexcept
	{
		const std::size_t words = (std::max)({readMask.size(), writeMask.size(), other.readMask.size(), other.writeMask.size()});
		for (std::size_t index = 0; index < words; ++index)
		{
			const std::uint64_t writes = index < writeMask.size() ? writeMask[index] : 0;
			const std::uint64_t reads = index < readMask.size() ? readMask[index] : 0;
			const std::uint64_t otherReads = index < other.readMask.size() ? other.readMask[index] : 0;
			const std::uint64_t otherWrites = index < other.writeMask.size() ? other.writeMask[index] : 0;
			const std::uint64_t otherWritesAndReads = otherWrites | otherReads;
			if ((writes & otherWritesAndReads) != 0 || (otherWrites & reads) != 0)
				return true;
		}
		return false;
	}

private:
	void Initialize(std::size_t componentCount)
	{
		readComponents.clear();
		writeComponents.clear();
		const std::size_t words = (componentCount + 63) / 64;
		readMask.assign(words, 0);
		writeMask.assign(words, 0);
	}

	void Add(const AccessDescriptor access)
	{
		if (access.component == InvalidComponentId || access.excluded)
			return;

		const std::size_t word = static_cast<std::size_t>(access.component) / 64;
		const std::uint64_t bit = std::uint64_t{1} << (access.component % 64);
		if (access.mode == AccessMode::Write)
		{
			writeComponents.push_back(access.component);
			writeMask[word] |= bit;
		}
		else if (access.mode == AccessMode::Read)
		{
			readComponents.push_back(access.component);
			readMask[word] |= bit;
		}
	}

	void SortComponents()
	{
		std::sort(readComponents.begin(), readComponents.end());
		std::sort(writeComponents.begin(), writeComponents.end());
		readComponents.erase(std::unique(readComponents.begin(), readComponents.end()), readComponents.end());
		writeComponents.erase(std::unique(writeComponents.begin(), writeComponents.end()), writeComponents.end());
		// Write access includes read access. Keep diagnostics and masks canonical
		// when a hook's auxiliary declaration overlaps its chunk query.
		std::erase_if(readComponents, [&](ComponentId id) { return std::binary_search(writeComponents.begin(), writeComponents.end(), id); });
		for (ComponentId id : writeComponents)
			readMask[id / 64] &= ~(std::uint64_t{1} << (id % 64));
	}

	std::vector<ComponentId> readComponents;
	std::vector<ComponentId> writeComponents;
	std::vector<std::uint64_t> readMask;
	std::vector<std::uint64_t> writeMask;

	friend class SystemRegistry;
};

struct SystemInfo
{
	using ResolveAccessFunction = void (*)(SystemAccess &, const ComponentRegistry &);
	using CreateQueryFunction = void *(*)(World &);
	using DestroyQueryFunction = void (*)(void *) noexcept;
	using ExecuteFunction = void (*)(void *, void *, SystemContext &);
	using PrepareQueryFunction = std::size_t (*)(void *);
	using ExecuteChunkFunction = void (*)(void *, void *, std::size_t, SystemContext &);

	SystemId id{InvalidSystemId};
	SystemKey stableKey{};
	std::string_view stableName{};
	SystemPhase phase{SystemPhase::Simulation};
	const std::type_info *type{&typeid(void)};
	void *instance{nullptr};
	SystemAccess access{};
	std::vector<SystemDependency> before;
	std::vector<SystemDependency> after;
	ResolveAccessFunction resolveAccess{nullptr};
	CreateQueryFunction createQuery{nullptr};
	DestroyQueryFunction destroyQuery{nullptr};
	ExecuteFunction execute{nullptr};
	PrepareQueryFunction prepareQuery{nullptr};
	ExecuteChunkFunction executeChunk{nullptr};
	// Optional lifecycle of ONE cohesive chunk system, not extra graph nodes.
	// Uses Query plus AuxiliaryAccess metadata and the same deferred wave commit.
	ExecuteFunction beforeChunks{nullptr};
	ExecuteFunction afterChunks{nullptr};
	// Explicit joined reduction/preparation node, not ordinary chunk iteration.
	// Runs once exclusively on the caller; shares its wave's final commit.
	// Query declares its full access.
	bool batch{false};

	const SystemAccess &Access() const noexcept { return access; }
	std::span<const SystemDependency> Before() const noexcept { return before; }
	std::span<const SystemDependency> After() const noexcept { return after; }
};

extern "C++" { class Scheduler; }

class SystemContext
{
public:
	World &GetWorld() const noexcept { return *m_world; }
	CommandBuffer &Commands() const noexcept { return *m_commands; }
	std::uint64_t Tick() const noexcept { return m_time.Tick(); }
	const engine::time::SimulationTime &Time() const noexcept { return m_time; }
	SystemId Id() const noexcept { return m_system; }
	SystemPhase Phase() const noexcept { return m_phase; }
	std::uint32_t JobOrder() const noexcept { return m_jobOrder; }
	std::uint32_t ChunkOrder() const noexcept { return m_chunkOrder; }

	// The scheduler supplies the command buffer and deterministic logical
	// ordering fields for each chunk job.
	SystemContext(World &world,
		CommandBuffer &commands,
		engine::time::SimulationTime time,
		SystemId system,
		SystemPhase phase,
		std::uint32_t jobOrder = 0,
		std::uint32_t chunkOrder = 0) noexcept :
		m_world(&world),
		m_commands(&commands),
		m_time(time),
		m_system(system),
		m_phase(phase),
		m_jobOrder(jobOrder),
		m_chunkOrder(chunkOrder)
	{
	}

private:
	World *m_world;
	CommandBuffer *m_commands;
	engine::time::SimulationTime m_time;
	SystemId m_system;
	SystemPhase m_phase;
	std::uint32_t m_jobOrder;
	std::uint32_t m_chunkOrder;

	friend class Scheduler;
};

namespace detail
{

template<typename T>
concept HasSystemTraits = requires
{
	typename T::Query;
	{ SystemTraits<T>::StableName } -> std::convertible_to<std::string_view>;
	{ SystemTraits<T>::Phase } -> std::convertible_to<SystemPhase>;
	typename SystemTraits<T>::Before;
	typename SystemTraits<T>::After;
};

template<typename T>
inline constexpr bool IsBatchSystem = [] {
	if constexpr (requires { SystemTraits<T>::Batch; })
		return bool(SystemTraits<T>::Batch);
	return false;
}();

template<typename T>
concept SystemDefinition = HasSystemTraits<T> && ((!IsBatchSystem<T> && requires(T &system,
	typename T::Query::Chunk chunk,
	SystemContext &context)
{
	system.Execute(chunk, context);
}) || (IsBatchSystem<T> && requires(T &system, SystemContext &context) {
	system.Execute(context);
}));

template<typename T>
SystemDependency MakeSystemDependency()
{
	static_assert(HasSystemTraits<T>,
		"System dependency types must provide ecs::SystemTraits and a Query");
	static_assert(SystemTraits<T>::StableName.size() != 0,
		"System dependency types must have a non-empty stable name");
	return SystemDependency{
		&typeid(T),
		HashSystemKey(SystemTraits<T>::StableName),
		SystemTraits<T>::StableName};
}

template<typename... Systems>
void AppendDependencies(std::vector<SystemDependency> &dependencies, SystemTypeList<Systems...>)
{
	(dependencies.push_back(MakeSystemDependency<Systems>()), ...);
}

} // namespace detail

class SystemRegistry
{
public:
	template<typename T>
	SystemId Register(T &system, SystemPhase phase = SystemTraits<T>::Phase);

	// Game composition may order reusable systems without making engine types
	// depend on game types. Like registration, this is startup-only metadata.
	template<typename Before, typename After>
	void OrderBefore();

	template<typename T>
	SystemId TryGet() const noexcept;

	SystemId TryGet(const std::type_info &type) const noexcept;
	const SystemInfo *TryGet(SystemId id) const noexcept;
	const SystemInfo &Get(SystemId id) const;

	void Finalize(const ComponentRegistry &components);

	bool IsFrozen() const noexcept { return m_frozen; }
	std::size_t Count() const noexcept { return m_infos.size(); }
	ComponentSchemaHash ComponentSchema() const noexcept { return m_componentSchemaHash; }
	const std::vector<const SystemInfo *> &OrderedSystems() const noexcept { return m_idToInfo; }

private:
	template<typename T>
	static void ResolveAccess(SystemAccess &access, const ComponentRegistry &components);

	template<typename T>
	static void *CreateQuery(World &world);

	template<typename T>
	static void DestroyQuery(void *query) noexcept;

	template<typename T>
	static void ExecuteSystem(void *instance, void *query, SystemContext &context);

	template<typename T>
	static std::size_t PrepareQuery(void *query);

	template<typename T>
	static void ExecuteSystemChunk(void *instance, void *query, std::size_t chunkIndex, SystemContext &context);

	std::deque<SystemInfo> m_infos;
	std::unordered_map<std::type_index, SystemInfo *> m_typeToInfo;
	std::unordered_map<SystemKey, SystemInfo *> m_keyToInfo;
	std::vector<const SystemInfo *> m_idToInfo;
	std::vector<std::pair<SystemDependency, SystemDependency>> m_ordering;
	bool m_frozen{false};
	ComponentSchemaHash m_componentSchemaHash{UnfinalizedSchemaHash};

	friend class Scheduler;
};

template<typename T>
SystemId SystemRegistry::Register(T &system, const SystemPhase phase)
{
	static_assert(std::is_object_v<T>, "ECS systems must be object types");
	static_assert(detail::SystemDefinition<T>,
		"ECS systems must provide Query, Execute, and ecs::SystemTraits");
	if (m_frozen)
		throw std::logic_error("Cannot register an ECS system after system finalization");

	const std::type_index type = std::type_index(typeid(T));
	if (const auto existing = m_typeToInfo.find(type); existing != m_typeToInfo.end())
	{
		if (existing->second->instance != &system || existing->second->phase != phase)
			throw std::logic_error("ECS system registered with a different instance or phase");
		return existing->second->id;
	}

	const std::string_view stableName = SystemTraits<T>::StableName;
	if (stableName.empty())
		throw std::invalid_argument("ECS system stable name cannot be empty");

	const SystemKey stableKey = HashSystemKey(stableName);
	if (m_keyToInfo.find(stableKey) != m_keyToInfo.end())
	{
		const SystemInfo &existing = *m_keyToInfo.at(stableKey);
		if (existing.stableName == stableName)
			throw std::logic_error("Duplicate ECS system stable key: " + std::string(stableName));
		throw std::logic_error("ECS system stable-key hash collision");
	}

	if (SystemPhaseIndex(phase) >= SystemPhaseCount)
		throw std::invalid_argument("ECS system phase is invalid");

	SystemInfo info;
	info.stableKey = stableKey;
	info.stableName = stableName;
	info.phase = phase;
	info.type = &typeid(T);
	info.instance = &system;
	info.resolveAccess = &ResolveAccess<T>;
	info.createQuery = &CreateQuery<T>;
	info.destroyQuery = &DestroyQuery<T>;
	info.execute = &ExecuteSystem<T>;
	info.prepareQuery = &PrepareQuery<T>;
	info.executeChunk = &ExecuteSystemChunk<T>;
	info.batch = detail::IsBatchSystem<T>;
	if constexpr (requires(T &value, typename T::Query &query, SystemContext &context) { value.BeforeChunks(query, context); })
	{
		static_assert(!detail::IsBatchSystem<T>, "Batch systems already execute once; chunk lifecycle hooks are unnecessary");
		info.beforeChunks = [](void *instance, void *query, SystemContext &context) {
			static_cast<T *>(instance)->BeforeChunks(*static_cast<typename T::Query *>(query), context);
		};
	}
	if constexpr (requires(T &value, typename T::Query &query, SystemContext &context) { value.AfterChunks(query, context); })
	{
		static_assert(!detail::IsBatchSystem<T>, "Batch systems already execute once; chunk lifecycle hooks are unnecessary");
		info.afterChunks = [](void *instance, void *query, SystemContext &context) {
			static_cast<T *>(instance)->AfterChunks(*static_cast<typename T::Query *>(query), context);
		};
	}
	detail::AppendDependencies(info.before, typename SystemTraits<T>::Before{});
	detail::AppendDependencies(info.after, typename SystemTraits<T>::After{});

	m_infos.push_back(std::move(info));
	try
	{
		m_typeToInfo.emplace(type, &m_infos.back());
		m_keyToInfo.emplace(stableKey, &m_infos.back());
	}
	catch (...)
	{
		m_keyToInfo.erase(stableKey);
		m_typeToInfo.erase(type);
		m_infos.pop_back();
		throw;
	}
	return InvalidSystemId;
}

template<typename Before, typename After>
void SystemRegistry::OrderBefore()
{
	if (m_frozen)
		throw std::logic_error("Cannot add ECS system ordering after finalization");
	m_ordering.emplace_back(detail::MakeSystemDependency<Before>(), detail::MakeSystemDependency<After>());
}

template<typename T>
SystemId SystemRegistry::TryGet() const noexcept
{
	return TryGet(typeid(T));
}

template<typename T>
void SystemRegistry::ResolveAccess(SystemAccess &access, const ComponentRegistry &components)
{
	access.Initialize(components.Count());
	for (const AccessDescriptor descriptor : T::Query::ResolveAccesses(components))
		access.Add(descriptor);
	// Additional typed component access for joined reductions/auxiliary queries.
	// Metadata only: this never changes chunk matching or builds another query.
	if constexpr (requires { typename T::AuxiliaryAccess; })
		for (const AccessDescriptor descriptor : T::AuxiliaryAccess::ResolveAccesses(components))
			access.Add(descriptor);
	access.SortComponents();
}

template<typename T>
void *SystemRegistry::CreateQuery(World &world)
{
	if constexpr (detail::IsBatchSystem<T>) return nullptr;
	using QueryType = typename T::Query;
	void *memory = ::operator new(sizeof(QueryType), std::align_val_t(alignof(QueryType)));
	try
	{
		std::construct_at(static_cast<QueryType *>(memory), world);
	}
	catch (...)
	{
		::operator delete(memory, std::align_val_t(alignof(QueryType)));
		throw;
	}
	return memory;
}

template<typename T>
void SystemRegistry::DestroyQuery(void *query) noexcept
{
	using QueryType = typename T::Query;
	std::destroy_at(static_cast<QueryType *>(query));
	::operator delete(query, std::align_val_t(alignof(QueryType)));
}

template<typename T>
void SystemRegistry::ExecuteSystem(void *instance, void *query, SystemContext &context)
{
	T &system = *static_cast<T *>(instance);
	if constexpr (detail::IsBatchSystem<T>)
		system.Execute(context);
	else
	{
	using QueryType = typename T::Query;
	QueryType &typedQuery = *static_cast<QueryType *>(query);
	if constexpr (requires { system.BeforeChunks(typedQuery, context); }) system.BeforeChunks(typedQuery, context);
	typedQuery.ForEachChunk([&](typename QueryType::Chunk chunk) {
		system.Execute(chunk, context);
	});
	if constexpr (requires { system.AfterChunks(typedQuery, context); }) system.AfterChunks(typedQuery, context);
	}
}

template<typename T>
std::size_t SystemRegistry::PrepareQuery(void *query)
{
	if constexpr (detail::IsBatchSystem<T>) return 1;
	else
	{
	using QueryType = typename T::Query;
	return static_cast<QueryType *>(query)->PrepareChunks();
	}
}

template<typename T>
void SystemRegistry::ExecuteSystemChunk(void *instance,
	void *query,
	const std::size_t chunkIndex,
	SystemContext &context)
{
	T &system = *static_cast<T *>(instance);
	if constexpr (detail::IsBatchSystem<T>)
		system.Execute(context);
	else
	{
	using QueryType = typename T::Query;
	QueryType &typedQuery = *static_cast<QueryType *>(query);
	typedQuery.ExecutePreparedChunk(chunkIndex, [&](typename QueryType::Chunk chunk) {
		system.Execute(chunk, context);
	});
	}
}

} // namespace ecs

namespace ecs
{

SystemId SystemRegistry::TryGet(const std::type_info &type) const noexcept
{
	const auto existing = m_typeToInfo.find(std::type_index(type));
	return existing == m_typeToInfo.end() ? InvalidSystemId : existing->second->id;
}

const SystemInfo *SystemRegistry::TryGet(const SystemId id) const noexcept
{
	return static_cast<std::size_t>(id) < m_idToInfo.size() ? m_idToInfo[id] : nullptr;
}

const SystemInfo &SystemRegistry::Get(const SystemId id) const
{
	const SystemInfo *info = TryGet(id);
	if (info == nullptr)
		throw std::out_of_range("Invalid ECS system ID");
	return *info;
}

void SystemRegistry::Finalize(const ComponentRegistry &components)
{
	if (m_frozen)
		return;
	if (!components.IsFrozen())
		throw std::logic_error("ECS component registry must be finalized before system finalization");

	std::vector<SystemInfo *> ordered;
	ordered.reserve(m_infos.size());
	for (SystemInfo &info : m_infos)
		ordered.push_back(&info);

	std::sort(ordered.begin(), ordered.end(), [](const SystemInfo *left, const SystemInfo *right) {
		if (left->stableKey != right->stableKey)
			return left->stableKey < right->stableKey;
		return left->stableName < right->stableName;
	});

	for (std::size_t index = 1; index < ordered.size(); ++index)
	{
		if (ordered[index - 1]->stableKey == ordered[index]->stableKey)
			throw std::logic_error("Duplicate or colliding ECS system stable key");
	}
	if (ordered.size() >= static_cast<std::size_t>(InvalidSystemId))
		throw std::length_error("Too many ECS systems for dense SystemId");

	std::vector<SystemAccess> access(ordered.size());
	for (std::size_t index = 0; index < ordered.size(); ++index)
		ordered[index]->resolveAccess(access[index], components);

	for (std::size_t index = 0; index < ordered.size(); ++index)
	{
		ordered[index]->id = static_cast<SystemId>(index);
		ordered[index]->access = std::move(access[index]);
	}
	m_idToInfo = std::vector<const SystemInfo *>(ordered.begin(), ordered.end());
	m_componentSchemaHash = components.SchemaHash();
	m_frozen = true;
}

} // namespace ecs

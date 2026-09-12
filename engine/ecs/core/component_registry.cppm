module;

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

export module engine.ecs.core.component_registry;

export namespace ecs
{
    using ComponentId = std::uint32_t;
    using ComponentKey = std::uint64_t;
    using ComponentSchemaHash = std::uint64_t;

    inline constexpr ComponentId InvalidComponentId = (std::numeric_limits<ComponentId>::max)();
    inline constexpr ComponentSchemaHash UnfinalizedSchemaHash = 0;
    inline constexpr std::uint32_t ComponentSchemaFormatVersion = 1;

    enum class PersistencePolicy : std::uint8_t
    {
        Transient,
        Serializable
    };

    // Every component must provide an explicit specialization. StableName is
    // the canonical, engine-level name used to derive StableKey.
    template<typename T>
    struct ComponentTraits;

    constexpr ComponentKey HashComponentKey(const std::string_view stableName) noexcept
    {
        // FNV-1a is used only as a deterministic key function. Collisions are
        // validated and rejected by ComponentRegistry during registration.
        ComponentKey hash = 14695981039346656037ull;
        for (const char character : stableName)
        {
            hash ^= static_cast<ComponentKey>(static_cast<std::uint8_t>(character));
            hash *= 1099511628211ull;
        }
        return hash;
    }

    struct ComponentInfo
    {
        ComponentId id{ InvalidComponentId };
        ComponentKey stableKey{};
        std::string_view stableName{};
        std::uint32_t version{};
        PersistencePolicy persistence{ PersistencePolicy::Transient };
        std::size_t size{};
        std::size_t alignment{};

        // Null when the component intentionally has no default constructor.
        void (*constructDefault)(void*){};
        void (*constructMove)(void*, void*) noexcept{};
        void (*destroy)(void*) noexcept{};
    };

    class ComponentRegistry
    {
    public:
        template<typename T>
        ComponentId Register();

        template<typename T>
        ComponentId TryGet() const noexcept;

        [[nodiscard]] ComponentId TryGet(const std::type_info& type) const noexcept;
        [[nodiscard]] const ComponentInfo* TryGet(ComponentId id) const noexcept;
        [[nodiscard]] ComponentId TryGet(ComponentKey key) const noexcept;
        [[nodiscard]] const ComponentInfo& Get(ComponentId id) const;

        // Finalization sorts all registered descriptors by stable key, assigns
        // dense runtime IDs, computes the schema hash, and freezes the set.
        void Finalize();

        [[nodiscard]] bool IsFrozen() const noexcept { return m_frozen; }
        [[nodiscard]] ComponentSchemaHash SchemaHash() const noexcept { return m_schemaHash; }
        [[nodiscard]] std::size_t Count() const noexcept { return m_infos.size(); }

    private:
        template<typename T>
        static void ConstructDefault(void* destination)
        {
            std::construct_at(static_cast<T*>(destination));
        }

        template<typename T>
        static void ConstructMove(void* destination, void* source) noexcept
        {
            std::construct_at(static_cast<T*>(destination), std::move(*static_cast<T*>(source)));
        }

        template<typename T>
        static void Destroy(void* object) noexcept
        {
            std::destroy_at(static_cast<T*>(object));
        }

        static ComponentSchemaHash ComputeSchemaHash(const std::vector<ComponentInfo*>& ordered) noexcept;

        std::deque<ComponentInfo> m_infos;
        // RTTI is used only to deduplicate repeated registration of the same
        // C++ type. It is never used for stable identity, dense IDs, or schema.
        std::unordered_map<std::type_index, ComponentInfo*> m_typeToInfo;
        std::unordered_map<ComponentKey, ComponentInfo*> m_keyToInfo;
        std::vector<ComponentInfo*> m_idToInfo;
        bool m_frozen{ false };
        ComponentSchemaHash m_schemaHash{ UnfinalizedSchemaHash };
    };

    namespace detail
    {
        template<typename T>
        concept HasComponentTraits = requires
        {
            { ComponentTraits<T>::StableName } -> std::convertible_to<std::string_view>;
            { ComponentTraits<T>::Version } -> std::convertible_to<std::uint32_t>;
            { ComponentTraits<T>::Persistence } -> std::convertible_to<PersistencePolicy>;
        };

        inline void AppendByte(ComponentSchemaHash& hash, const std::uint8_t value) noexcept
        {
            hash ^= static_cast<ComponentSchemaHash>(value);
            hash *= 1099511628211ull;
        }

        inline void AppendU32(ComponentSchemaHash& hash, const std::uint32_t value) noexcept
        {
            AppendByte(hash, static_cast<std::uint8_t>(value >> 24));
            AppendByte(hash, static_cast<std::uint8_t>(value >> 16));
            AppendByte(hash, static_cast<std::uint8_t>(value >> 8));
            AppendByte(hash, static_cast<std::uint8_t>(value));
        }

        inline void AppendU64(ComponentSchemaHash& hash, const std::uint64_t value) noexcept
        {
            AppendByte(hash, static_cast<std::uint8_t>(value >> 56));
            AppendByte(hash, static_cast<std::uint8_t>(value >> 48));
            AppendByte(hash, static_cast<std::uint8_t>(value >> 40));
            AppendByte(hash, static_cast<std::uint8_t>(value >> 32));
            AppendByte(hash, static_cast<std::uint8_t>(value >> 24));
            AppendByte(hash, static_cast<std::uint8_t>(value >> 16));
            AppendByte(hash, static_cast<std::uint8_t>(value >> 8));
            AppendByte(hash, static_cast<std::uint8_t>(value));
        }
    }

    template<typename T>
    ComponentId ComponentRegistry::Register()
    {
        static_assert(std::is_object_v<T>, "ECS components must be object types");
        static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "ECS component types must be unqualified");
        static_assert(std::is_move_constructible_v<T>, "ECS components must be move constructible");
        static_assert(std::is_nothrow_move_constructible_v<T>, "ECS component moves must be noexcept");
        static_assert(std::is_destructible_v<T>, "ECS components must be destructible");
        static_assert(std::is_nothrow_destructible_v<T>, "ECS component destruction must be noexcept");
        static_assert(detail::HasComponentTraits<T>,
            "Every ECS component must specialize ecs::ComponentTraits with StableName, Version, and Persistence");

        const std::type_index type = std::type_index(typeid(T));
        if (const auto existing = m_typeToInfo.find(type); existing != m_typeToInfo.end())
        {
            return existing->second->id;
        }

        if (m_frozen)
        {
            throw std::logic_error("Cannot register a new ECS component after registry finalization");
        }

        const std::string_view stableName = ComponentTraits<T>::StableName;
        if (stableName.empty())
        {
            throw std::invalid_argument("ECS component stable name cannot be empty");
        }

        const ComponentKey stableKey = HashComponentKey(stableName);
        for (const ComponentInfo& existing : m_infos)
        {
            if (existing.stableKey == stableKey)
            {
                if (existing.stableName == stableName)
                {
                    throw std::logic_error("Duplicate ECS component stable key: " + std::string(stableName));
                }
                throw std::logic_error("ECS component stable-key hash collision");
            }
        }

        ComponentInfo info;
        info.stableKey = stableKey;
        info.stableName = stableName;
        info.version = static_cast<std::uint32_t>(ComponentTraits<T>::Version);
        info.persistence = ComponentTraits<T>::Persistence;
        info.size = sizeof(T);
        info.alignment = alignof(T);
        info.constructMove = &ConstructMove<T>;
        info.destroy = &Destroy<T>;
        if constexpr (std::is_default_constructible_v<T>)
        {
            info.constructDefault = &ConstructDefault<T>;
        }

        m_infos.push_back(info);
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
        return InvalidComponentId;
    }

    template<typename T>
    ComponentId ComponentRegistry::TryGet() const noexcept
    {
        return TryGet(typeid(T));
    }
}

namespace ecs
{
    namespace
    {
        constexpr ComponentSchemaHash SchemaHashOffset = 14695981039346656037ull;

        void AppendSchemaTag(ComponentSchemaHash& hash) noexcept
        {
            constexpr std::string_view tag = "ECS_COMPONENT_SCHEMA";
            for (const char character : tag)
            {
                detail::AppendByte(hash, static_cast<std::uint8_t>(character));
            }
        }
    }

    ComponentSchemaHash ComponentRegistry::ComputeSchemaHash(const std::vector<ComponentInfo*>& ordered) noexcept
    {
        ComponentSchemaHash hash = SchemaHashOffset;
        AppendSchemaTag(hash);
        detail::AppendU32(hash, ComponentSchemaFormatVersion);
        detail::AppendU64(hash, static_cast<std::uint64_t>(ordered.size()));
        for (const ComponentInfo* info : ordered)
        {
            detail::AppendU64(hash, info->stableKey);
            detail::AppendU32(hash, info->version);
            detail::AppendByte(hash, static_cast<std::uint8_t>(info->persistence));
        }
        return hash;
    }

    const ComponentInfo* ComponentRegistry::TryGet(const ComponentId id) const noexcept
    {
        return static_cast<std::size_t>(id) < m_idToInfo.size() ? m_idToInfo[id] : nullptr;
    }

    ComponentId ComponentRegistry::TryGet(const std::type_info& type) const noexcept
    {
        const auto existing = m_typeToInfo.find(std::type_index(type));
        return existing == m_typeToInfo.end() ? InvalidComponentId : existing->second->id;
    }

    ComponentId ComponentRegistry::TryGet(const ComponentKey key) const noexcept
    {
        const auto found = m_keyToInfo.find(key);
        return found == m_keyToInfo.end() ? InvalidComponentId : found->second->id;
    }

    const ComponentInfo& ComponentRegistry::Get(const ComponentId id) const
    {
        const ComponentInfo* info = TryGet(id);
        if (info == nullptr)
        {
            throw std::out_of_range("Invalid ECS component ID");
        }
        return *info;
    }

    void ComponentRegistry::Finalize()
    {
        if (m_frozen)
        {
            return;
        }

        std::vector<ComponentInfo*> ordered;
        ordered.reserve(m_infos.size());
        for (ComponentInfo& info : m_infos)
        {
            ordered.push_back(&info);
        }

        std::sort(ordered.begin(), ordered.end(), [](const ComponentInfo* left, const ComponentInfo* right)
        {
            if (left->stableKey != right->stableKey)
            {
                return left->stableKey < right->stableKey;
            }
            return left->stableName < right->stableName;
        });

        for (std::size_t index = 0; index < ordered.size(); ++index)
        {
            ComponentInfo* info = ordered[index];
            if (info->stableName.empty())
            {
                throw std::invalid_argument("ECS component stable name cannot be empty");
            }
            if (index > 0 && ordered[index - 1]->stableKey == info->stableKey)
            {
                if (ordered[index - 1]->stableName == info->stableName)
                {
                    throw std::logic_error("Duplicate ECS component stable key");
                }
                throw std::logic_error("ECS component stable-key hash collision");
            }
        }

        if (ordered.size() >= static_cast<std::size_t>(InvalidComponentId))
        {
            throw std::length_error("Too many ECS components for dense ComponentId");
        }

        m_idToInfo = ordered;
        m_schemaHash = ComputeSchemaHash(ordered);

        for (ComponentId id = 0; id < ordered.size(); ++id)
        {
            ordered[id]->id = id;
        }

        m_frozen = true;
    }
}

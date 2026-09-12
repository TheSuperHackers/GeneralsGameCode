module;
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeinfo>
#include <type_traits>
#include <vector>
export module engine.events.schema.message_registry;

export namespace engine::events
{
using MessageKey = std::uint64_t;
using MessageId = std::uint32_t;
using MessageSchemaHash = std::uint64_t;
inline constexpr MessageId InvalidMessageId = (std::numeric_limits<MessageId>::max)();
inline constexpr std::uint32_t MessageSchemaFormatVersion = 1;
enum class MessageKind : std::uint8_t { Command = 0, Fact = 1, Presentation = 2 };
enum class RecordPolicy : std::uint8_t { Transient = 0, Recordable = 1 };
template<class T> struct MessageTraits;

constexpr MessageKey HashMessageKey(std::string_view name) noexcept
{
	MessageKey hash = 14695981039346656037ull;
	for (const char c : name) { hash ^= static_cast<std::uint8_t>(c); hash *= 1099511628211ull; }
	return hash;
}
template<class T>
concept DeclaredMessage = requires
{
	{ MessageTraits<T>::StableName } -> std::convertible_to<std::string_view>;
	{ MessageTraits<T>::Version } -> std::convertible_to<std::uint32_t>;
	{ MessageTraits<T>::Kind } -> std::convertible_to<MessageKind>;
	{ MessageTraits<T>::Recording } -> std::convertible_to<RecordPolicy>;
};
struct MessageInfo
{
	MessageId id{InvalidMessageId};
	MessageKey stableKey{0};
	std::string_view stableName{};
	std::uint32_t version{0};
	MessageKind kind{MessageKind::Fact};
	RecordPolicy recording{RecordPolicy::Transient};
};

// An explicitly owned cold-path registry. Cache the finalized dense ID before
// simulation; payload recording/iteration never calls RTTI or this registry.
class MessageRegistry
{
	struct Entry { MessageInfo info; const std::type_info *type; };
public:
	MessageRegistry() = default;
	MessageRegistry(const MessageRegistry &) = delete;
	MessageRegistry &operator=(const MessageRegistry &) = delete;
	MessageRegistry(MessageRegistry &&) = delete;
	MessageRegistry &operator=(MessageRegistry &&) = delete;
	template<DeclaredMessage T> void Register()
	{
		static_assert(std::is_object_v<T> && !std::is_const_v<T> && !std::is_volatile_v<T>,
			"Messages must be unqualified object types");
		if (m_frozen) throw std::logic_error("Cannot register messages after finalization");
		constexpr std::string_view name = MessageTraits<T>::StableName;
		constexpr std::uint32_t version = MessageTraits<T>::Version;
		constexpr MessageKind kind = MessageTraits<T>::Kind;
		constexpr RecordPolicy recording = MessageTraits<T>::Recording;
		Validate(name, version, kind, recording);
		const auto key = HashMessageKey(name);
		for (const auto &entry : m_entries)
		{
			if (*entry.type == typeid(T)) return; // Same exact local type only.
			if (entry.info.stableKey == key)
				throw std::logic_error("Message stable-key collision: '" + std::string(entry.info.stableName) + "' and '" + std::string(name) + "'");
		}
		if (m_entries.size() >= InvalidMessageId) throw std::length_error("Too many message types");
		m_entries.push_back({{InvalidMessageId, key, name, version, kind, recording}, &typeid(T)});
	}
	void Finalize()
	{
		if (m_frozen) return; // Explicitly idempotent; never invoked by lookup.
		auto ordered = m_entries; // Allocation failure leaves collection usable.
		std::sort(ordered.begin(), ordered.end(), [](const Entry &a, const Entry &b) {
			return a.info.stableKey < b.info.stableKey;
		});
		for (std::size_t i = 0; i < ordered.size(); ++i) ordered[i].info.id = static_cast<MessageId>(i);
		const auto hash = ComputeSchemaHash(ordered);
		m_entries.swap(ordered);
		m_hash = hash;
		m_frozen = true;
	}
	[[nodiscard]] bool IsFinalized() const noexcept { return m_frozen; }
	[[nodiscard]] std::size_t Count() const noexcept { return m_entries.size(); }
	[[nodiscard]] MessageSchemaHash SchemaHash() const { RequireFinalized(); return m_hash; }
	[[nodiscard]] const MessageInfo &Get(MessageId id) const
	{
		RequireFinalized();
		if (id >= m_entries.size()) throw std::out_of_range("Unknown dense message ID");
		return m_entries[id].info;
	}
	template<DeclaredMessage T> [[nodiscard]] MessageId Id() const
	{
		RequireFinalized();
		for (const auto &entry : m_entries)
			if (*entry.type == typeid(T)) return entry.info.id;
		throw std::out_of_range("Exact C++ message type is not registered");
	}
	[[nodiscard]] MessageId Find(MessageKey key) const
	{
		RequireFinalized();
		const auto found = std::lower_bound(m_entries.begin(), m_entries.end(), key,
			[](const Entry &entry, MessageKey value) { return entry.info.stableKey < value; });
		return found != m_entries.end() && found->info.stableKey == key ? found->info.id : InvalidMessageId;
	}
private:
	static void Validate(std::string_view name, std::uint32_t version, MessageKind kind, RecordPolicy recording)
	{
		if (name.empty()) throw std::invalid_argument("Message stable name must not be empty");
		for (const auto c : name)
			if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-' || c == '/'))
				throw std::invalid_argument("Message stable name requires canonical lowercase ASCII");
		if (version == 0) throw std::invalid_argument("Message schema version must be positive");
		if (kind != MessageKind::Command && kind != MessageKind::Fact && kind != MessageKind::Presentation)
			throw std::invalid_argument("Unknown message kind");
		if (recording != RecordPolicy::Transient && recording != RecordPolicy::Recordable)
			throw std::invalid_argument("Unknown message recording policy");
	}
	static void Byte(MessageSchemaHash &hash, std::uint8_t byte) noexcept { hash ^= byte; hash *= 1099511628211ull; }
	static void U32(MessageSchemaHash &hash, std::uint32_t value) noexcept
	{
		for (int shift = 24; shift >= 0; shift -= 8) Byte(hash, static_cast<std::uint8_t>(value >> shift));
	}
	static void U64(MessageSchemaHash &hash, std::uint64_t value) noexcept
	{
		for (int shift = 56; shift >= 0; shift -= 8) Byte(hash, static_cast<std::uint8_t>(value >> shift));
	}
	static MessageSchemaHash ComputeSchemaHash(const std::vector<Entry> &entries) noexcept
	{
		auto hash = HashMessageKey("ENGINE_MESSAGE_SCHEMA");
		U32(hash, MessageSchemaFormatVersion);
		U64(hash, entries.size());
		for (const auto &entry : entries)
		{
			const auto &info = entry.info;
			U64(hash, info.stableKey);
			U64(hash, info.stableName.size());
			for (const char c : info.stableName) Byte(hash, static_cast<std::uint8_t>(c));
			U32(hash, info.version);
			Byte(hash, static_cast<std::uint8_t>(info.kind));
			Byte(hash, static_cast<std::uint8_t>(info.recording));
		}
		return hash;
	}
	void RequireFinalized() const
	{
		if (!m_frozen) throw std::logic_error("Explicit message registry finalization is required");
	}
	std::vector<Entry> m_entries;
	MessageSchemaHash m_hash{0};
	bool m_frozen{false};
};
}

module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module games.generalszh.adapters.content.science.science_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import engine.gameplay.rts.unlocks.definitions.unlock_definition;

export namespace generalszh::content
{
inline constexpr std::size_t InspectedZeroHourScienceDefinitionCount = 95;
static_assert(engine::gameplay::rts::unlocks::MaxUnlockDefinitions >=
	InspectedZeroHourScienceDefinitionCount);

struct DecodedScienceDefinition final
{
	std::string name;
	std::uint32_t version{1};
	std::uint64_t creditCost{};
	bool grantable{true};
	std::vector<std::string> prerequisiteNames;
};

struct DecodedScienceCatalog final
{
	std::vector<DecodedScienceDefinition> definitions;
};

namespace science_detail
{
inline std::string_view StripComment(std::string_view line) noexcept
{
	bool quoted = false;
	for (std::size_t index = 0; index < line.size(); ++index)
	{
		if (line[index] == '"')
			quoted = !quoted;
		if (!quoted && (line[index] == ';' ||
			(line[index] == '/' && index + 1 < line.size() && line[index + 1] == '/')))
			return line.substr(0, index);
	}
	return line;
}

inline std::uint64_t ParseUnsigned(std::string_view value, std::string_view field)
{
	value = Trim(value);
	if (value.empty())
		throw std::invalid_argument("Missing numeric science field: " + std::string(field));
	std::uint64_t result = 0;
	for (const char character : value)
	{
		if (character < '0' || character > '9')
			throw std::invalid_argument("Invalid unsigned science field: " + std::string(field));
		const auto digit = static_cast<std::uint64_t>(character - '0');
		if (result > ((std::numeric_limits<std::uint64_t>::max)() - digit) / 10u)
			throw std::out_of_range("Science numeric field overflows: " + std::string(field));
		result = result * 10u + digit;
	}
	return result;
}

inline std::vector<std::string> FindScienceNames(std::string_view text)
{
	if (text.size() > 32u * 1024u * 1024u)
		throw std::length_error("Science.ini source exceeds 32 MiB boundary");
	std::vector<std::string> names;
	std::size_t depth = 0;
	while (!text.empty())
	{
		const auto lineEnd = text.find('\n');
		std::string_view line = StripComment(text.substr(0, lineEnd));
		line = Trim(line);
		if (!line.empty())
		{
			if (EqualToken(line, "End"))
			{
				if (depth == 0)
					throw std::invalid_argument("Science.ini contains an unmatched End");
				--depth;
			}
			else if (line.find('=') == std::string_view::npos)
			{
				const auto words = Tokens(line);
				if (depth == 0)
				{
					if (words.size() != 2 || !EqualToken(words[0], "Science"))
						throw std::invalid_argument("Unexpected top-level Science.ini block");
					names.emplace_back(words[1]);
					depth = 1;
				}
				else
				{
					// The shared named-block reader supports nested assignment blocks;
					// retain the same bounded nesting accounting while discovering names.
					++depth;
				}
			}
			else if (depth == 0)
			{
				throw std::invalid_argument("Science.ini contains a field outside a Science block");
			}
		}
		if (lineEnd == text.npos)
			break;
		text.remove_prefix(lineEnd + 1);
	}
	if (depth != 0)
		throw std::invalid_argument("Science.ini contains an unterminated block");
	return names;
}

inline bool DecodeGrantable(std::string_view value)
{
	if (value.empty())
		return true;
	if (EqualToken(value, "Yes"))
		return true;
	if (EqualToken(value, "No"))
		return false;
	throw std::invalid_argument("Invalid IsGrantable value in Science.ini");
}
} // namespace science_detail

inline DecodedScienceCatalog DecodeScienceDefinitions(std::string_view text)
{
	const auto names = science_detail::FindScienceNames(text);
	if (names.size() > engine::gameplay::rts::unlocks::MaxUnlockDefinitions)
		throw std::length_error("Science.ini exceeds the bounded unlock component capacity");

	DecodedScienceCatalog result;
	result.definitions.reserve(names.size());
	for (const std::string &name : names)
	{
		for (const auto &existing : result.definitions)
			if (existing.name == name)
				throw std::invalid_argument("Duplicate Science.ini definition: " + name);

		const auto block = ReadNamedBlock(text, "Science", name);
		DecodedScienceDefinition definition;
		definition.name = name;
		const auto cost = block.Value("SciencePurchasePointCost");
		definition.creditCost = cost.empty() ? 0 : science_detail::ParseUnsigned(cost, "SciencePurchasePointCost");
		definition.grantable = science_detail::DecodeGrantable(block.Value("IsGrantable"));
		const auto prerequisiteText = block.Value("PrerequisiteSciences");
		const auto prerequisiteTokens = Tokens(prerequisiteText);
		for (const auto token : prerequisiteTokens)
		{
			if (EqualToken(token, "None"))
			{
				if (prerequisiteTokens.size() != 1)
					throw std::invalid_argument("None cannot be combined with Science prerequisites");
				continue;
			}
			definition.prerequisiteNames.emplace_back(token);
		}
		result.definitions.push_back(std::move(definition));
	}
	return result;
}

inline DecodedScienceCatalog DecodeScienceFile(const std::filesystem::path &path)
{
	return DecodeScienceDefinitions(ReadIniFile(path));
}

inline std::vector<engine::gameplay::rts::unlocks::UnlockDefinition>
BindScienceDefinitions(const DecodedScienceCatalog &source)
{
	if (source.definitions.size() > engine::gameplay::rts::unlocks::MaxUnlockDefinitions)
		throw std::length_error("Decoded Science definitions exceed the bounded unlock component capacity");

	std::vector<engine::gameplay::rts::unlocks::UnlockDefinition> result;
	result.reserve(source.definitions.size());
	for (std::size_t index = 0; index < source.definitions.size(); ++index)
	{
		const auto &definition = source.definitions[index];
		if (!engine::gameplay::rts::unlocks::IsValidUnlockName(definition.name))
			throw std::invalid_argument("Invalid canonical Science definition name: " + definition.name);
		if (definition.version == 0)
			throw std::invalid_argument("Science definition version must be nonzero: " + definition.name);
		for (std::size_t previous = 0; previous < index; ++previous)
		{
			const auto &existing = source.definitions[previous];
			if (existing.name == definition.name ||
				engine::gameplay::rts::unlocks::MakeUnlockKey(existing.name) ==
				engine::gameplay::rts::unlocks::MakeUnlockKey(definition.name))
				throw std::invalid_argument("Duplicate or colliding Science definition: " + definition.name);
		}
		std::vector<engine::gameplay::rts::unlocks::UnlockKey> prerequisites;
		prerequisites.reserve(definition.prerequisiteNames.size());
		for (const std::string &name : definition.prerequisiteNames)
		{
			const auto found = std::find_if(source.definitions.begin(), source.definitions.end(),
				[&](const auto &candidate) { return candidate.name == name; });
			if (found == source.definitions.end())
				throw std::invalid_argument("Science prerequisite names an unknown definition: " + name);
			prerequisites.push_back(engine::gameplay::rts::unlocks::MakeUnlockKey(name));
		}
		result.emplace_back(definition.name, definition.version, definition.creditCost,
			definition.grantable, prerequisites);
	}
	return result;
}
} // namespace generalszh::content

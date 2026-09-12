module;

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module games.generalszh.adapters.content.rank.rank_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import engine.gameplay.rts.rank.definitions.rank_definition;

export namespace generalszh::content
{
inline constexpr std::size_t InspectedZeroHourRankDefinitionCount = 5;

struct DecodedRankDefinition final
{
	std::uint32_t level{};
	std::string rankName;
	std::uint64_t skillPointsNeeded{};
	std::uint64_t sciencePurchasePointsGranted{};
	std::vector<std::string> sciencesGranted;
};

struct DecodedRankCatalog final
{
	std::vector<DecodedRankDefinition> definitions;
};

namespace rank_detail
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

inline std::uint64_t ParseUnsigned(const std::string_view value, const std::string_view field)
{
	const auto trimmed = Trim(value);
	if (trimmed.empty())
		throw std::invalid_argument("Missing numeric Rank.ini field: " + std::string(field));
	std::uint64_t result = 0;
	for (const char character : trimmed)
	{
		if (character < '0' || character > '9')
			throw std::invalid_argument("Invalid unsigned Rank.ini field: " + std::string(field));
		const auto digit = static_cast<std::uint64_t>(character - '0');
		if (result > ((std::numeric_limits<std::uint64_t>::max)() - digit) / 10u)
			throw std::out_of_range("Rank.ini numeric field overflows: " + std::string(field));
		result = result * 10u + digit;
	}
	return result;
}

inline std::uint32_t ParseLevel(const std::string_view token)
{
	std::uint32_t level{};
	const auto parsed = std::from_chars(token.data(), token.data() + token.size(), level);
	if (parsed.ec != std::errc{} || parsed.ptr != token.data() + token.size() || level == 0)
		throw std::invalid_argument("Invalid Rank.ini rank number");
	return level;
}

inline std::vector<std::uint32_t> FindRankLevels(std::string_view text)
{
	if (text.size() > 32u * 1024u * 1024u)
		throw std::length_error("Rank.ini source exceeds 32 MiB boundary");
	std::vector<std::uint32_t> levels;
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
					throw std::invalid_argument("Rank.ini contains an unmatched End");
				--depth;
			}
			else if (line.find('=') == std::string_view::npos)
			{
				const auto words = Tokens(line);
				if (depth == 0)
				{
					if (words.size() != 2 || !EqualToken(words[0], "Rank"))
						throw std::invalid_argument("Unexpected top-level Rank.ini block");
					const auto level = ParseLevel(words[1]);
					if (level != levels.size() + 1u)
						throw std::invalid_argument("Rank.ini ranks must increase monotonically");
					levels.push_back(level);
					depth = 1;
				}
				else
					++depth;
			}
			else if (depth == 0)
				throw std::invalid_argument("Rank.ini contains a field outside a Rank block");
		}
		if (lineEnd == text.npos)
			break;
		text.remove_prefix(lineEnd + 1);
	}
	if (depth != 0)
		throw std::invalid_argument("Rank.ini contains an unterminated block");
	return levels;
}
} // namespace rank_detail

inline DecodedRankCatalog DecodeRankDefinitions(const std::string_view text)
{
	DecodedRankCatalog result;
	const auto levels = rank_detail::FindRankLevels(text);
	result.definitions.reserve(levels.size());
	for (const auto level : levels)
	{
		const auto block = ReadNamedBlock(text, "Rank", std::to_string(level));
		DecodedRankDefinition definition;
		definition.level = level;
		definition.rankName = std::string(block.Value("RankName"));
		definition.skillPointsNeeded = rank_detail::ParseUnsigned(
			block.Require("SkillPointsNeeded"), "SkillPointsNeeded");
		definition.sciencePurchasePointsGranted = rank_detail::ParseUnsigned(
			block.Require("SciencePurchasePointsGranted"), "SciencePurchasePointsGranted");
		const auto scienceTokens = Tokens(block.Require("SciencesGranted"));
		if (scienceTokens.empty())
			throw std::invalid_argument("Rank.ini SciencesGranted cannot be empty");
		for (const auto token : scienceTokens)
		{
			if (EqualToken(token, "None"))
			{
				if (scienceTokens.size() != 1)
					throw std::invalid_argument("None cannot be combined with rank sciences");
				continue;
			}
			definition.sciencesGranted.emplace_back(token);
		}
		result.definitions.push_back(std::move(definition));
	}
	return result;
}

inline DecodedRankCatalog DecodeRankFile(const std::filesystem::path &path)
{
	return DecodeRankDefinitions(ReadIniFile(path));
}

inline std::vector<engine::gameplay::rts::rank::RankDefinition>
BindRankDefinitions(const DecodedRankCatalog &source,
	const engine::gameplay::rts::unlocks::UnlockCatalog &unlockCatalog)
{
	if (source.definitions.empty())
		throw std::invalid_argument("Rank.ini did not define any ranks");
	std::vector<engine::gameplay::rts::rank::RankDefinition> result;
	result.reserve(source.definitions.size());
	for (std::size_t index = 0; index < source.definitions.size(); ++index)
	{
		const auto &sourceDefinition = source.definitions[index];
		if (sourceDefinition.level != index + 1u)
			throw std::invalid_argument("Rank definitions are not dense and monotonic");
		auto &definition = result.emplace_back();
		definition.skillPointsNeeded = sourceDefinition.skillPointsNeeded;
		definition.unlockCreditsGranted = sourceDefinition.sciencePurchasePointsGranted;
		definition.unlocksGranted.reserve(sourceDefinition.sciencesGranted.size());
		for (const std::string &name : sourceDefinition.sciencesGranted)
		{
			if (!engine::gameplay::rts::unlocks::IsValidUnlockName(name))
				throw std::invalid_argument("Invalid rank science name: " + name);
			const auto id = unlockCatalog.Find(
				engine::gameplay::rts::unlocks::MakeUnlockKey(name));
			if (id == engine::gameplay::rts::unlocks::InvalidUnlockId)
				throw std::invalid_argument("Rank references an unknown science: " + name);
			definition.unlocksGranted.push_back(id);
		}
	}
	return result;
}
} // namespace generalszh::content

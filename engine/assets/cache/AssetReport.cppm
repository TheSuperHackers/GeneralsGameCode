module;

#include <array>
#include <cctype>
#include <cstddef>
#include <map>
#include <string>
#include <string_view>

export module Assets.Cache.AssetReport;

namespace Assets
{

// The report keeps the three asset classes generic. The legacy file format
// names are emitted by Format_Report so existing diagnostics remain readable.
export enum class AssetReportCategory
{
	Model,
	Animation,
	Skeleton
};

export class AssetReport final
{
public:
	AssetReport() = default;

	void Enable_Reporting(bool enable) noexcept { m_reporting = enable; }
	bool Reporting_Enabled() const noexcept { return m_reporting; }

	void Enable_Load_On_Demand_Reporting(bool enable) noexcept
	{
		m_load_on_demand_reporting = enable;
	}
	bool Load_On_Demand_Reporting_Enabled() const noexcept
	{
		return m_load_on_demand_reporting;
	}

	// Load-on-demand records remain opt-in, matching the original status
	// collector. Missing records are always retained, even when output is
	// disabled, so enabling output later does not lose useful information.
	void Record_Load_On_Demand(AssetReportCategory category, std::string_view name)
	{
		if (m_load_on_demand_reporting)
			Add(m_records[Category_Index(category)].load_on_demand, name);
	}

	void Record_Missing(AssetReportCategory category, std::string_view name)
	{
		Add(m_records[Category_Index(category)].missing, name);
	}

	std::size_t Load_On_Demand_Count(AssetReportCategory category, std::string_view name) const
	{
		return Find_Count(m_records[Category_Index(category)].load_on_demand, name);
	}

	std::size_t Missing_Count(AssetReportCategory category, std::string_view name) const
	{
		return Find_Count(m_records[Category_Index(category)].missing, name);
	}

	std::size_t Load_On_Demand_Name_Count(AssetReportCategory category) const noexcept
	{
		return m_records[Category_Index(category)].load_on_demand.size();
	}

	std::size_t Missing_Name_Count(AssetReportCategory category) const noexcept
	{
		return m_records[Category_Index(category)].missing.size();
	}

	void Clear() noexcept
	{
		for (auto &records : m_records) {
			records.load_on_demand.clear();
			records.missing.clear();
		}
	}

	// HashTemplateClass did not define a useful stable iteration order. The
	// report deliberately uses case-folded lexical order so repeated runs are
	// comparable while retaining every record and count.
	std::string Format_Report() const
	{
		std::string report("Load-on-demand and missing assets report\n\n");
		for (std::size_t index = 0; index < m_records.size(); ++index)
			Append_Category(report, k_category_names[index].load_on_demand,
				m_records[index].load_on_demand);
		for (std::size_t index = 0; index < m_records.size(); ++index)
			Append_Category(report, k_category_names[index].missing,
				m_records[index].missing);
		return report;
	}

private:
	using Counts = std::map<std::string, std::size_t>;

	struct CategoryRecords
	{
		Counts load_on_demand;
		Counts missing;
	};

	struct CategoryNames
	{
		const char *load_on_demand;
		const char *missing;
	};

	static constexpr std::array<CategoryNames, 3> k_category_names{{
		{"LOAD_ON_DEMAND_ROBJ", "MISSING_ROBJ"},
		{"LOAD_ON_DEMAND_HANIM", "MISSING_HANIM"},
		{"LOAD_ON_DEMAND_HTREE", "MISSING_HTREE"}
	}};

	static std::size_t Category_Index(AssetReportCategory category) noexcept
	{
		switch (category) {
			case AssetReportCategory::Model:
				return 0;
			case AssetReportCategory::Animation:
				return 1;
			case AssetReportCategory::Skeleton:
				return 2;
		}
		return 0;
	}

	static std::string Fold_Name(std::string_view name)
	{
		std::string folded(name);
		for (char &character : folded)
			character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
		return folded;
	}

	static void Add(Counts &counts, std::string_view name)
	{
		auto [entry, inserted] = counts.try_emplace(Fold_Name(name), 0);
		(void)inserted;
		++entry->second;
	}

	static std::size_t Find_Count(const Counts &counts, std::string_view name)
	{
		const auto entry = counts.find(Fold_Name(name));
		return entry == counts.end() ? 0 : entry->second;
	}

	static void Append_Category(std::string &report, const char *category_name,
		const Counts &counts)
	{
		report += "Category: ";
		report += category_name;
		report += "\n\n";
		for (const auto &[name, count] : counts) {
			report += name;
			if (count > 1) {
				report += "\t(reported ";
				report += std::to_string(count);
				report += " times)";
			}
			report += '\n';
		}
		report += '\n';
	}

	std::array<CategoryRecords, 3> m_records;
	bool m_reporting = true;
	bool m_load_on_demand_reporting = false;
};

}

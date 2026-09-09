module;

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "Utility/CppMacros.h"
#include "PreRTS.h"
#include "GameLogic/PartitionManager.h"

// Keep access to the packed storage limited to this test probe.  The forward
// declaration and friend declaration live in PartitionManager.h so the game
// library and this module see the same class definition.
class PartitionManagerTestProbe
{
public:
	static void configure(PartitionManager& manager, Int cellCountX, Int cellCountY, Real cellSize)
	{
		manager.m_cellSize = cellSize;
		manager.m_cellSizeInv = (Real)(1.0 / cellSize);
		manager.m_cellCountX = cellCountX;
		manager.m_cellCountY = cellCountY;
		manager.m_totalCellCount = cellCountX * cellCountY;
		manager.calcRadiusVec();
	}

	static Int maxRadius(const PartitionManager& manager)
	{
		return manager.m_maxGcoRadius;
	}

	static Int radiusBegin(const PartitionManager& manager, Int radius)
	{
		return manager.m_radiusVec[radius].begin;
	}

	static Int radiusEnd(const PartitionManager& manager, Int radius)
	{
		return manager.m_radiusVec[radius].end;
	}

	static Int offsetCount(const PartitionManager& manager)
	{
		return static_cast<Int>(manager.m_radiusOffsets.size());
	}

	static ICoord2D offset(const PartitionManager& manager, Int index)
	{
		return manager.m_radiusOffsets[index];
	}

	static void prepareModule(PartitionData& module, Int coiArrayCount)
	{
		module.m_coiArrayCount = coiArrayCount;
		module.m_coiInUseCount = 0;
		module.m_coiArray = new CellAndObjectIntersection[coiArrayCount];
	}

	static void destroyModule(PartitionData& module)
	{
		module.~PartitionData();
	}

	static void addCoverage(PartitionData& module, PartitionCell& cell)
	{
		module.addSubPixToCoverage(&cell);
	}

	static void removeAllCoverage(PartitionData& module)
	{
		module.friend_removeAllTouchedCells();
	}

	static void setObject(PartitionData& module, Object* object)
	{
		module.friend_setObject(object);
	}

	static Int linkedMemberCount(PartitionCell& cell)
	{
		Int count = 0;
		for (CellAndObjectIntersection* coi = cell.getFirstCoiInCell(); coi; coi = coi->getNextCoi())
			++count;
		return count;
	}

	static PartitionData* linkedMemberModule(PartitionCell& cell, Int index)
	{
		CellAndObjectIntersection* coi = cell.getFirstCoiInCell();
		for (Int i = 0; coi && i < index; ++i)
			coi = coi->getNextCoi();
		return coi ? coi->getModule() : nullptr;
	}

	static Int compactMemberCount(PartitionCell& cell)
	{
		return static_cast<Int>(cell.getCompactMembers().size());
	}

	static PartitionData* compactMemberModule(PartitionCell& cell, Int index)
	{
		const auto& members = cell.getCompactMembers();
		return members[static_cast<std::size_t>(index)].module;
	}

	static Object* compactMemberObject(PartitionCell& cell, Int index)
	{
		const auto& members = cell.getCompactMembers();
		return members[static_cast<std::size_t>(index)].object;
	}

	static std::uintptr_t compactStorageAddress(PartitionCell& cell)
	{
		const auto& members = cell.getCompactMembers();
		return reinterpret_cast<std::uintptr_t>(members.data());
	}
};

export module engine.navigation.partition_fixture;

export namespace navigation::testing {

struct RadiusOffset
{
	int x = 0;
	int y = 0;
};

struct RadiusSpan
{
	int begin = 0;
	int end = 0;
};

struct RadiusGridSnapshot
{
	int maxRadius = 0;
	std::vector<RadiusSpan> spans;
	std::vector<RadiusOffset> offsets;
};

class PartitionFixture
{
	RadiusGridSnapshot m_radius;

public:
	PartitionFixture(int cellCountX, int cellCountY, float cellSize)
	{
		if (cellCountX <= 0 || cellCountY <= 0 || cellSize <= 0.0f)
			throw std::invalid_argument("Invalid partition grid");

		PartitionManager manager;
		PartitionManagerTestProbe::configure(manager, cellCountX, cellCountY, cellSize);

		m_radius.maxRadius = static_cast<int>(PartitionManagerTestProbe::maxRadius(manager));
		m_radius.spans.reserve(static_cast<std::size_t>(m_radius.maxRadius + 1));
		for (int radius = 0; radius <= m_radius.maxRadius; ++radius)
		{
			m_radius.spans.push_back({
				static_cast<int>(PartitionManagerTestProbe::radiusBegin(manager, radius)),
				static_cast<int>(PartitionManagerTestProbe::radiusEnd(manager, radius))});
		}

		const int offsetCount = PartitionManagerTestProbe::offsetCount(manager);
		m_radius.offsets.reserve(static_cast<std::size_t>(offsetCount));
		for (int index = 0; index < offsetCount; ++index)
		{
			const ICoord2D offset = PartitionManagerTestProbe::offset(manager, index);
			m_radius.offsets.push_back({static_cast<int>(offset.x), static_cast<int>(offset.y)});
		}
	}

	const RadiusGridSnapshot& radiusGrid() const
	{
		return m_radius;
	}
};

struct MembershipEntry
{
	std::uintptr_t module = 0;
	std::uintptr_t object = 0;
};

class PartitionMembershipFixture
{
	typedef std::aligned_storage_t<sizeof(PartitionData), alignof(PartitionData)> ModuleStorage;

	PartitionManager m_manager;
	PartitionManager* m_previousManager;
	PartitionCell m_cells[2];
	std::vector<ModuleStorage> m_moduleStorage;
	std::vector<PartitionData*> m_modules;
	std::vector<std::uintptr_t> m_objectTokens;
	std::vector<std::uint8_t> m_active;

	static std::uintptr_t pointerValue(const void* pointer)
	{
		return reinterpret_cast<std::uintptr_t>(pointer);
	}

	void checkIndex(std::size_t index) const
	{
		if (index >= m_modules.size())
			throw std::out_of_range("Invalid partition membership index");
	}

	void checkCellIndex(std::size_t cellIndex) const
	{
		if (cellIndex >= std::size(m_cells))
			throw std::out_of_range("Invalid partition cell index");
	}

	std::size_t activeIndex(std::size_t moduleIndex, std::size_t cellIndex) const
	{
		return moduleIndex * std::size(m_cells) + cellIndex;
	}

public:
	PartitionMembershipFixture(const PartitionMembershipFixture&) = delete;
	PartitionMembershipFixture& operator=(const PartitionMembershipFixture&) = delete;
	PartitionMembershipFixture(PartitionMembershipFixture&&) = delete;
	PartitionMembershipFixture& operator=(PartitionMembershipFixture&&) = delete;

	explicit PartitionMembershipFixture(std::size_t moduleCount)
		: m_previousManager(nullptr)
	{
		if (moduleCount == 0)
			throw std::invalid_argument("Partition membership fixture needs a module");

		m_moduleStorage.resize(moduleCount);
		m_modules.reserve(moduleCount);
		m_objectTokens.resize(moduleCount);
		m_active.assign(moduleCount * std::size(m_cells), 0);

		m_previousManager = ThePartitionManager;
		ThePartitionManager = &m_manager;

		for (std::size_t index = 0; index < moduleCount; ++index)
		{
			PartitionData* module = ::new (static_cast<void*>(&m_moduleStorage[index])) PartitionData();
			m_modules.push_back(module);
			PartitionManagerTestProbe::prepareModule(*module, static_cast<Int>(std::size(m_cells)));
			PartitionManagerTestProbe::setObject(
				*module,
				reinterpret_cast<Object*>(&m_objectTokens[index]));
		}
	}

	~PartitionMembershipFixture()
	{
		for (std::size_t index = 0; index < m_modules.size(); ++index)
		{
			// Clear the cached object before unlinking the COIs, including the
			// opaque test token that is never a real Object instance.
			PartitionManagerTestProbe::setObject(*m_modules[index], nullptr);
			PartitionManagerTestProbe::removeAllCoverage(*m_modules[index]);
		}

		for (PartitionData* module : m_modules)
			PartitionManagerTestProbe::destroyModule(*module);

		ThePartitionManager = m_previousManager;
	}

	void add(std::size_t index, std::size_t cellIndex)
	{
		checkIndex(index);
		checkCellIndex(cellIndex);
		const std::size_t active = activeIndex(index, cellIndex);
		if (m_active[active])
			return;

		PartitionManagerTestProbe::addCoverage(*m_modules[index], m_cells[cellIndex]);
		m_active[active] = 1;
	}

	void remove(std::size_t index)
	{
		checkIndex(index);
		PartitionManagerTestProbe::removeAllCoverage(*m_modules[index]);
		for (std::size_t cellIndex = 0; cellIndex < std::size(m_cells); ++cellIndex)
			m_active[activeIndex(index, cellIndex)] = 0;
	}

	void setObjectNull(std::size_t index)
	{
		checkIndex(index);
		PartitionManagerTestProbe::setObject(*m_modules[index], nullptr);
	}

	std::vector<MembershipEntry> linkedSequence(std::size_t cellIndex)
	{
		checkCellIndex(cellIndex);
		std::vector<MembershipEntry> result;
		const Int count = PartitionManagerTestProbe::linkedMemberCount(m_cells[cellIndex]);
		result.reserve(static_cast<std::size_t>(count));
		for (Int index = 0; index < count; ++index)
		{
			PartitionData* module = PartitionManagerTestProbe::linkedMemberModule(m_cells[cellIndex], index);
			result.push_back({ pointerValue(module), pointerValue(module ? module->getObject() : nullptr) });
		}
		return result;
	}

	std::vector<MembershipEntry> packedSequence(std::size_t cellIndex)
	{
		checkCellIndex(cellIndex);
		std::vector<MembershipEntry> result;
		const Int count = PartitionManagerTestProbe::compactMemberCount(m_cells[cellIndex]);
		result.reserve(static_cast<std::size_t>(count));
		for (Int index = 0; index < count; ++index)
		{
			PartitionData* module = PartitionManagerTestProbe::compactMemberModule(m_cells[cellIndex], index);
			Object* object = PartitionManagerTestProbe::compactMemberObject(m_cells[cellIndex], index);
			result.push_back({ pointerValue(module), pointerValue(object) });
		}
		return result;
	}

	std::uintptr_t packedStorageAddress(std::size_t cellIndex)
	{
		checkCellIndex(cellIndex);
		return PartitionManagerTestProbe::compactStorageAddress(m_cells[cellIndex]);
	}
};

enum class DistanceMode
{
	Center2D,
	Center3D,
	Boundary2D,
	Boundary3D,
};

struct Point3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

struct DistanceComparison
{
	float withoutVector = 0.0f;
	float withVector = 0.0f;
	Point3 vector;
};

DistanceComparison compareGoalDistance(DistanceMode mode, Point3 goal, Point3 other)
{
	DistanceCalculationType calculation;
	switch (mode)
	{
	case DistanceMode::Center2D:
		calculation = FROM_CENTER_2D;
		break;
	case DistanceMode::Center3D:
		calculation = FROM_CENTER_3D;
		break;
	case DistanceMode::Boundary2D:
		calculation = FROM_BOUNDINGSPHERE_2D;
		break;
	case DistanceMode::Boundary3D:
		calculation = FROM_BOUNDINGSPHERE_3D;
		break;
	default:
		throw std::invalid_argument("Invalid distance mode");
	}

	PartitionManager manager;
	const Coord3D goalPosition = {goal.x, goal.y, goal.z};
	const Coord3D otherPosition = {other.x, other.y, other.z};
	const Real withoutVector = manager.getGoalDistanceSquared(
		nullptr, &goalPosition, &otherPosition, calculation, nullptr);
	Coord3D vector = {-991.0f, -992.0f, -993.0f};
	const Real withVector = manager.getGoalDistanceSquared(
		nullptr, &goalPosition, &otherPosition, calculation, &vector);

	return {
		static_cast<float>(withoutVector),
		static_cast<float>(withVector),
		{vector.x, vector.y, vector.z}};
}

} // namespace navigation::testing

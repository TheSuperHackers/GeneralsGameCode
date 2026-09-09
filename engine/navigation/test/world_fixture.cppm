module;

#include <string>
#include <array>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <chrono>
#include <cstdint>
#include "Utility/CppMacros.h"
#include "PreRTS.h"
#include "engine/navigation/legacy/AIPathfind.h"
#include "Common/CriticalSection.h"
#include "Common/GlobalData.h"
#include "Common/ThingFactory.h"
#include "GameLogic/AI.h"
#include "GameLogic/TerrainLogic.h"

export module engine.navigation.world_fixture;
import engine.navigation.pathfinder;
import engine.navigation.querymemo;
import engine.navigation.search_workspace;
import engine.navigation.dynamic_request_queue;

// Supply executable-owned platform symbols without starting a renderer or game loop.
extern "C++" {
const Char* g_strFile = "data\\Generals.str";
const Char* g_csfFile = "data\\%s\\Generals.csf";
const char* gAppPrefix = "";
class GameEngine;
GameEngine* CreateGameEngine() { throw std::logic_error("Fixture must not create a game engine"); }
}

namespace navigation::testing::detail {
class FlatTerrain final : public TerrainLogic {
    Region3D extent_{};
public:
    void setExtent(int width, int height) { extent_ = {{0, 0, 0}, {width * 10.0f, height * 10.0f, 0}}; }
    void getExtent(Region3D* out) const override { *out = extent_; }
    void getMaximumPathfindExtent(Region3D* out) const override { *out = extent_; }
    void ownBridge(Bridge* bridge) { m_bridgeListHead = bridge; }
    Real getGroundHeight(Real, Real, Coord3D* normal = nullptr) const override {
        if (normal) *normal = {0, 0, 1};
        return 0;
    }
    Real getLayerHeight(Real, Real, PathfindLayerEnum, Coord3D* normal = nullptr, Bool = true) const override {
        if (normal) *normal = {0, 0, 1};
        return 0;
    }
};
}

export namespace navigation::testing {
struct Result {
    bool found = false;
    bool legal = false;
    bool macroEdges = false;
    bool optimizedEdges = false;
    std::vector<std::array<int, 2>> rawGroundCells;
    std::string raw;
    std::string optimized;
    int cells = 0;
    bool clean = false;
};
struct Timing {
    std::uint64_t nanoseconds = 0;
    std::uint64_t work = 0;
    int found = 0;
};
struct JpsStats {
    std::uint64_t attempted = 0;
    std::uint64_t accepted = 0;
    std::uint64_t validationRejected = 0;
    std::uint64_t unavailableFallback = 0;
    std::uint64_t topologyRebuilds = 0;
    std::uint64_t directAccepted = 0;
    std::uint64_t baseCellsExamined = 0;
    std::uint64_t clearanceCellsExamined = 0;
};
struct QueueTiming : Timing {
    int batches = 0;
    std::uint64_t maxBatchNanoseconds = 0;
};
struct GoalRayResult {
    int firstResult = 0;
    int secondResult = 0;
    int allocatedCells = 0;
    std::uint64_t hits = 0;
    std::uint64_t misses = 0;
    std::size_t entries = 0;
    std::string firstSearchSnapshot;
    std::string secondSearchSnapshot;
};
struct GoalRayTraversalResult {
    int legacyResult = 0;
    int specializedResult = 0;
    std::string legacySnapshot;
    std::string specializedSnapshot;
};

// Only classification and service setup are synthetic. Zone building, searching,
// clearance, costs, path construction and optimization are production code.
extern "C++" class World {
    Pathfinder* pf;
    static constexpr PathfindLayerEnum LAYER_BRIDGE_1 = static_cast<PathfindLayerEnum>(2);
public:
    explicit World(const std::vector<std::string>& rows) {
        if (rows.empty() || rows.front().empty()) throw std::invalid_argument("Empty map");
        for (const auto& row : rows) {
            if (row.size() != rows.front().size()) throw std::invalid_argument("Ragged map");
            for (char c : row) type(c);
        }
        if (TheAI || TheTerrainLogic || TheGameLogic || TheWritableGlobalData || TheThingFactory)
            throw std::logic_error("Fixtures must run serially in an isolated process");
        static CriticalSection locks[5];
        TheAsciiStringCriticalSection = &locks[0];
        TheUnicodeStringCriticalSection = &locks[1];
        TheDmaCriticalSection = &locks[2];
        TheMemoryPoolCriticalSection = &locks[3];
        TheDebugLogCriticalSection = &locks[4];
        initMemoryManager();
        TheWritableGlobalData = NEW GlobalData;
        TheGameLogic = NEW GameLogic;
        TheTerrainLogic = NEW detail::FlatTerrain;
        TheAI = NEW AI;
        TheThingFactory = NEW ThingFactory;
        pf = TheAI->pathfinder();
        const int width = static_cast<int>(rows.front().size());
        const int height = static_cast<int>(rows.size());
        static_cast<detail::FlatTerrain*>(TheTerrainLogic)->setExtent(width, height);
        pf->m_extent = {{0, 0}, {width - 1, height - 1}};
        pf->m_logicalExtent = pf->m_extent;
        pf->m_blockOfMapCells = NEW PathfindCell[width * height];
        pf->m_map = NEW PathfindCell*[width];
        for (int x = 0; x < width; ++x) {
            pf->m_map[x] = pf->m_blockOfMapCells + x * height;
            for (int y = 0; y < height; ++y) {
                auto& cell = pf->m_map[x][y];
                cell.setType(type(rows[y][x]));
                cell.setLayer(LAYER_GROUND);
            }
        }
        pf->m_goalRayMemo->configureGrid(
            pf->m_extent.lo.x, pf->m_extent.lo.y,
            static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height),
            static_cast<std::uint8_t>(LAYER_LAST + 1));
        pf->m_zoneManager.allocateBlocks(pf->m_extent);
        pf->m_zoneManager.calculateZones(pf->m_map, pf->m_layers, pf->m_extent);
        pf->m_isMapReady = true;
        // Existing fixture cases characterize the native pathfinder. New JPS
        // probes and benchmarks opt in explicitly through the test control.
        pf->setJpsAdapterEnabled(false);
    }
    ~World() {
        delete TheGameLogic; // GameLogic owns terrain and needs AI during teardown.
        TheGameLogic = nullptr;
        pf->reset();
        delete TheAI;
        TheAI = nullptr;
        delete TheThingFactory;
        TheThingFactory = nullptr;
        delete TheWritableGlobalData;
        TheWritableGlobalData = nullptr;
    }
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    // Test-only switch for comparing the production macro-ray path with its
    // legacy successor expansion.  Gameplay has no runtime setting for this.
    void setMacroEdgesEnabled(bool enabled) { pf->m_macroRayEnabled = enabled; }

    // Test-only controls for observing live-ground adapter dispatch without
    // exposing instrumentation through gameplay-facing APIs.
    void setJpsAdapterEnabled(bool enabled) { pf->setJpsAdapterEnabled(enabled); }
    void setDirectGroundPathEnabled(bool enabled) { pf->setDirectGroundPathEnabled(enabled); }
    JpsStats jpsStats() const {
        const auto stats = pf->getJpsAdapterStats();
        return {stats.attempted, stats.accepted, stats.validationRejected,
                stats.unavailableFallback, stats.topologyRebuilds, stats.directAccepted,
                stats.baseCellsExamined, stats.clearanceCellsExamined};
    }

    static PathfindCell::CellType type(char c) {
        switch (c) {
        case '.': return PathfindCell::CELL_CLEAR;
        case '#': return PathfindCell::CELL_IMPASSABLE;
        case '~': return PathfindCell::CELL_WATER;
        case '^': return PathfindCell::CELL_CLIFF;
        case 'r': return PathfindCell::CELL_RUBBLE;
        default: throw std::invalid_argument("Unknown map cell");
        }
    }
    void setCell(int x, int y, char c) {
        pf->m_map[x][y].setType(type(c));
        pf->markJpsStaticTopologyDirty(IRegion2D{{x, y}, {x, y}});
        pf->m_zoneManager.calculateZones(pf->m_map, pf->m_layers, pf->m_extent);
    }
    void setPinched(int x, int y, bool pinched) {
        auto* cell = pf->getCell(LAYER_GROUND, x, y);
        if (!cell) throw std::invalid_argument("Pinched cell outside fixture");
        cell->setPinched(pinched);
    }
    void movingUnit(int x, int y, bool present) {
        pf->m_map[x][y].setPosUnit(present ? static_cast<ObjectID>(42) : INVALID_ID, {x, y});
    }
    bool hasMovingUnit(int x, int y) const {
        const auto& cell = pf->m_map[x][y];
        return cell.getFlags() == PathfindCell::UNIT_PRESENT_MOVING && cell.getPosUnit() == static_cast<ObjectID>(42);
    }
    bool occupiedSearchMutation(bool parentFirst) {
        auto& occupied = pf->m_map[5][5];
        auto& parent = pf->m_map[4][5];
        occupied.setPosUnit(static_cast<ObjectID>(42), {5, 5});
        parent.allocateInfo({4, 5});
        parent.startPathfind(nullptr);
        pf->m_openList.reset();

        if (parentFirst) {
            occupied.setParentCell(&parent);
            occupied.setCostSoFar(10);
        } else {
            occupied.setCostSoFar(10);
            occupied.setParentCell(&parent);
        }
        occupied.setTotalCost(20);
        occupied.putOnSortedOpenList(pf->m_openList);

        const bool valid = occupied.getOpen() && occupied.getCostSoFar() == 10 &&
            occupied.getTotalCost() == 20 && occupied.getParentCell() == &parent &&
            occupied.getFlags() == PathfindCell::UNIT_PRESENT_MOVING &&
            occupied.getPosUnit() == static_cast<ObjectID>(42);

        pf->cleanOpenAndClosedLists();
        occupied.setPosUnit(INVALID_ID, {5, 5});
        parent.releaseInfo();
        return valid && !occupied.hasInfo() && !parent.hasInfo();
    }
    void populateMovingUnits(int count) {
        int placed = 0;
        for (int y = 0; y <= pf->m_extent.hi.y && placed < count; ++y)
            for (int x = 0; x <= pf->m_extent.hi.x && placed < count; ++x)
                pf->m_map[x][y].setPosUnit(static_cast<ObjectID>(++placed), {x, y});
        if (placed != count) throw std::invalid_argument("Map too small for occupancy fixture");
    }
    int movingUnitCount() const {
        int count = 0;
        for (int y = 0; y <= pf->m_extent.hi.y; ++y)
            for (int x = 0; x <= pf->m_extent.hi.x; ++x)
                if (pf->m_map[x][y].getFlags() == PathfindCell::UNIT_PRESENT_MOVING &&
                    pf->m_map[x][y].getPosUnit() != INVALID_ID) ++count;
        return count;
    }
    void enqueue(int count) {
        for (int i = 1; i <= count; ++i)
            if (!pf->queueForPath(static_cast<ObjectID>(i))) throw std::logic_error("Queue rejected a request");
    }
    std::size_t pendingRequests() const { return pf->m_pathRequests->size(); }
    void processMissingRequests() { pf->processPathfindQueue(); }
    std::size_t searchStorageBytes() const { return PathfindCell::s_search.storageBytes(); }
    std::size_t cellBytes() const { return sizeof(PathfindCell); }
    std::string goalRaySearchSnapshot() const {
        std::ostringstream snapshot;
        const auto appendCell = [&](const PathfindCell* cell) {
            snapshot << cell->getXIndex() << ',' << cell->getYIndex()
                << ':' << static_cast<int>(cell->getLayer())
                << ':' << (cell->getOpen() ? 'O' : cell->getClosed() ? 'C' : 'U')
                << ':' << cell->getCostSoFar() << ':' << cell->getTotalCost()
                << ':' << (cell->isBlockedByAlly() ? '1' : '0')
                << ':' << static_cast<int>(cell->getFlags())
                << ':' << cell->getGoalUnit() << ':' << cell->getPosUnit()
                << ':' << cell->getGoalAircraft() << ':';
            const auto* parent = cell->getParentCell();
            if (parent) {
                snapshot << parent->getXIndex() << ',' << parent->getYIndex()
                    << ':' << static_cast<int>(parent->getLayer());
            } else {
                snapshot << '-';
            }
            snapshot << ';';
        };

        snapshot << "open[";
        for (auto* cell = pf->m_openList.getHead(); cell; cell = pf->m_openList.next(cell))
            appendCell(cell);
        snapshot << "]closed[";
        for (auto* cell = pf->m_closedList.getHead(); cell; cell = pf->m_closedList.next(cell))
            appendCell(cell);
        snapshot << "]cells[";
        for (int y = pf->m_extent.lo.y; y <= pf->m_extent.hi.y; ++y) {
            for (int x = pf->m_extent.lo.x; x <= pf->m_extent.hi.x; ++x) {
                const auto& cell = pf->m_map[x][y];
                if (cell.hasInfo()) appendCell(&cell);
            }
        }
        snapshot << ']';
        return snapshot.str();
    }
    GoalRayResult goalRayProbe(int fromX, int fromY, int toX, int toY, bool useMemo) {
        if (fromX == toX && fromY == toY) throw std::invalid_argument("Goal ray needs distinct cells");
        pf->cleanOpenAndClosedLists();
        auto* parent = pf->getCell(LAYER_GROUND, fromX, fromY);
        auto* goal = pf->getCell(LAYER_GROUND, toX, toY);
        if (!parent || !goal) throw std::invalid_argument("Goal ray outside fixture");
        parent->releaseInfo();
        goal->releaseInfo();
        const ICoord2D parentIndex{fromX, fromY};
        const ICoord2D goalIndex{toX, toY};
        if (!parent->allocateInfo(parentIndex) || !goal->allocateInfo(goalIndex))
            throw std::logic_error("Goal ray search allocation failed");
        parent->startPathfind(goal);
        pf->m_cumulativeCellsAllocated = 0;
        pf->m_goalRayMemo->resetStats();
        const Bool oldMemoEnabled = pf->m_goalRayMemoEnabled;
        pf->m_goalRayMemoEnabled = useMemo;
        const int firstResult = pf->examineGoalRay(parent, goal,
            LOCOMOTORSURFACE_GROUND, false, false, false, 0, nullptr, INVALID_ID);
        const std::string firstSnapshot = goalRaySearchSnapshot();
        // Repeating the same production trace in one search makes the
        // second pass exercise cached movement verdicts while retaining the
        // exact callback ordering and frontier/cost work.
        const int secondResult = pf->examineGoalRay(parent, goal,
            LOCOMOTORSURFACE_GROUND, false, false, false, 0, nullptr, INVALID_ID);
        const std::string secondSnapshot = goalRaySearchSnapshot();
        const auto stats = pf->m_goalRayMemo->stats();
        const auto entries = pf->m_goalRayMemo->size();
        pf->m_goalRayMemoEnabled = oldMemoEnabled;
        pf->cleanOpenAndClosedLists();
        parent->releaseInfo();
        goal->releaseInfo();
        return {firstResult, secondResult,
                pf->m_cumulativeCellsAllocated, stats.hits, stats.misses, entries,
                firstSnapshot, secondSnapshot};
    }
    GoalRayTraversalResult goalRayTraversalProbe(int fromX, int fromY, int toX, int toY,
                                                 int seededX = -1, int seededY = -1) {
        if (fromX == toX && fromY == toY) throw std::invalid_argument("Goal ray needs distinct cells");

        int legacyResult = 0;
        int specializedResult = 0;
        std::string legacySnapshot;
        std::string specializedSnapshot;

        const auto run = [&](Bool useSpecializedIterator, int& result, std::string& snapshot) {
            pf->cleanOpenAndClosedLists();
            // Drop ordinary retained search records between the two oracle
            // runs. Occupancy records intentionally survive and exercise the
            // same inactive-record path as a production search.
            for (int x = pf->m_extent.lo.x; x <= pf->m_extent.hi.x; ++x)
                for (int y = pf->m_extent.lo.y; y <= pf->m_extent.hi.y; ++y) {
                    auto& cell = pf->m_map[x][y];
                    if (cell.hasInfo() && !cell.getOpen() && !cell.getClosed())
                        cell.releaseInfo();
                }

            auto* parent = pf->getCell(LAYER_GROUND, fromX, fromY);
            auto* goal = pf->getCell(LAYER_GROUND, toX, toY);
            if (!parent || !goal) throw std::invalid_argument("Goal ray outside fixture");
            const ICoord2D parentIndex{fromX, fromY};
            const ICoord2D goalIndex{toX, toY};
            if (!parent->allocateInfo(parentIndex) || !goal->allocateInfo(goalIndex))
                throw std::logic_error("Goal ray search allocation failed");
            parent->startPathfind(goal);

            if (seededX >= 0 && seededY >= 0) {
                auto* seeded = pf->getCell(LAYER_GROUND, seededX, seededY);
                if (!seeded) throw std::invalid_argument("Seed outside fixture");
                const ICoord2D seededIndex{seededX, seededY};
                if (!seeded->allocateInfo(seededIndex))
                    throw std::logic_error("Goal ray seed allocation failed");
                // This is an active, cheaper route that the ray will revisit.
                // The callback must clear the stale ally marker and preserve
                // the existing g/parent/frontier state.
                seeded->setCostSoFar(0);
                seeded->setTotalCost(0);
                seeded->setBlockedByAlly(true);
                seeded->putOnSortedOpenList(pf->m_openList);
            }

            const Bool oldMemoEnabled = pf->m_goalRayMemoEnabled;
            pf->m_goalRayMemoEnabled = false;
            result = pf->examineGoalRay(parent, goal,
                LOCOMOTORSURFACE_GROUND, false, false, false, 0, nullptr,
                INVALID_ID, useSpecializedIterator);
            snapshot = goalRaySearchSnapshot();
            pf->m_goalRayMemoEnabled = oldMemoEnabled;
            pf->cleanOpenAndClosedLists();
            parent->releaseInfo();
            goal->releaseInfo();
        };

        run(false, legacyResult, legacySnapshot);
        run(true, specializedResult, specializedSnapshot);
        return {legacyResult, specializedResult, legacySnapshot, specializedSnapshot};
    }
    void addBridge() {
        pf->markJpsStaticTopologyDirty();
        BridgeInfo info;
        info.from = {45, 65, 0};
        info.to = {105, 65, 0};
        info.fromLeft = {45, 45, 0};
        info.fromRight = {45, 85, 0};
        info.toLeft = {105, 45, 0};
        info.toRight = {105, 85, 0};
        info.bridgeWidth = 40;
        // The empty ThingFactory lets Bridge retain geometry without creating
        // a GenericBridge game object, towers or render resources.
        auto* bridge = newInstance(Bridge)(info, nullptr, AsciiString("FixtureBridge"));
        bridge->setLayer(LAYER_BRIDGE_1);
        bridge->setNext(nullptr);
        static_cast<detail::FlatTerrain*>(TheTerrainLogic)->ownBridge(bridge);
        auto& layer = pf->m_layers[LAYER_BRIDGE_1];
        layer.init(bridge, LAYER_BRIDGE_1);
        layer.allocateCells(&pf->m_extent);
        layer.classifyCells();
        pf->m_zoneManager.calculateZones(pf->m_map, pf->m_layers, pf->m_extent);
    }
    void bridgeDestroyed(bool destroyed) {
        pf->changeBridgeState(LAYER_BRIDGE_1, !destroyed);
        pf->m_zoneManager.calculateZones(pf->m_map, pf->m_layers, pf->m_extent);
    }

    Result ground(int fromX, int fromY, int toX, int toY, int diameter = 1, bool crusher = false) {
        Coord3D from = {fromX * 10.0f + 5, fromY * 10.0f + 5, 0};
        Coord3D to = {toX * 10.0f + 5, toY * 10.0f + 5, 0};
        pf->m_cumulativeCellsAllocated = 0;
        Path* path = pf->findGroundPath(&from, &to, diameter, crusher);
        return capture(path, diameter, crusher);
    }
    // Exercise the ordinary object-facing search with a null object.  The
    // fixture supplies the ground surface directly so this stays independent
    // of the game's INI locomotor store while still traversing findPath and
    // internalFindPath.
    Result normal(int fromX, int fromY, int toX, int toY) {
        Coord3D from = {fromX * 10.0f + 5, fromY * 10.0f + 5, 0};
        Coord3D to = {toX * 10.0f + 5, toY * 10.0f + 5, 0};
        LocomotorSet groundLoco;
        groundLoco.m_validLocomotorSurfaces = LOCOMOTORSURFACE_GROUND;
        pf->m_cumulativeCellsAllocated = 0;
        Path* path = pf->findPath(nullptr, groundLoco, &from, &to);
        return capture(path, 1, false);
    }
    Result jpsGround(int fromX, int fromY, int toX, int toY,
                     int radius, bool centerInCell) {
        Coord3D from = {fromX * 10.0f + 5, fromY * 10.0f + 5, 0};
        Coord3D to = {toX * 10.0f + 5, toY * 10.0f + 5, 0};
        Pathfinder::JpsGroundQuery query;
        query.acceptableSurfaces = LOCOMOTORSURFACE_GROUND;
        query.radius = radius;
        query.centerInCell = centerInCell;
        query.isHuman = true;
        pf->m_cumulativeCellsAllocated = 0;
        Path* path = pf->tryJpsGroundPath(query, &from, &to);
        return capture(path, 1, false);
    }
    // Time production search and path destruction, excluding string formatting
    // and the fixture's full-grid cleanup audit.
    Timing benchmarkGround(int fromX, int fromY, int toX, int toY, int repetitions) {
        Coord3D from = {fromX * 10.0f + 5, fromY * 10.0f + 5, 0};
        Coord3D to = {toX * 10.0f + 5, toY * 10.0f + 5, 0};
        Timing result;
        const auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < repetitions; ++i) {
            pf->m_cumulativeCellsAllocated = 0;
            auto* path = pf->findGroundPath(&from, &to, 1, false);
            result.work += pf->m_cumulativeCellsAllocated;
            if (path) { ++result.found; deleteInstance(path); }
        }
        result.nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - start).count();
        return result;
    }
    Timing benchmarkNormal(int fromX, int fromY, int toX, int toY, int repetitions) {
        Coord3D from = {fromX * 10.0f + 5, fromY * 10.0f + 5, 0};
        Coord3D to = {toX * 10.0f + 5, toY * 10.0f + 5, 0};
        LocomotorSet groundLoco;
        groundLoco.m_validLocomotorSurfaces = LOCOMOTORSURFACE_GROUND;
        Timing result;
        const auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < repetitions; ++i) {
            pf->m_cumulativeCellsAllocated = 0;
            auto* path = pf->findPath(nullptr, groundLoco, &from, &to);
            result.work += pf->m_cumulativeCellsAllocated;
            if (path) { ++result.found; deleteInstance(path); }
        }
        result.nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - start).count();
        return result;
    }
    std::string queuedGroundTrace(const std::vector<std::array<int, 4>>& queries,
                                  int workBudget, bool warm) {
        const auto find = [&](std::uint32_t id) {
            const auto& query = queries.at(id - 1);
            Coord3D from{query[0] * 10.0f + 5, query[1] * 10.0f + 5, 0};
            Coord3D to{query[2] * 10.0f + 5, query[3] * 10.0f + 5, 0};
            return pf->findGroundPath(&from, &to, 1, false);
        };
        if (warm) {
            for (std::uint32_t id = 1; id <= queries.size(); ++id)
                if (auto* path = find(id)) deleteInstance(path);
        }
        enqueue(static_cast<int>(queries.size()));
        std::ostringstream trace;
        trace.imbue(std::locale::classic());
        int batch = 0;
        while (!pf->m_pathRequests->empty()) {
            pf->m_cumulativeCellsAllocated = 0;
            navigation::processRequests(*pf->m_pathRequests, pf->m_cumulativeCellsAllocated,
                [&](std::uint32_t id) { return &queries.at(id - 1); },
                [&](const std::array<int, 4>* query) {
                    const auto id = static_cast<std::uint32_t>(query - queries.data()) + 1;
                    const int before = pf->m_cumulativeCellsAllocated;
                    auto result = capture(find(id), 1, false);
                    if (!result.found || !result.legal || !result.clean)
                        throw std::logic_error("Queued trace produced an invalid path for request " + std::to_string(id));
                    trace << batch << ':' << id << ':'
                          << pf->m_cumulativeCellsAllocated - before << ':'
                          << result.found << ':' << result.legal << ':' << result.clean
                          << ':' << result.raw << '|' << result.optimized << '\n';
                    return result.found;
                }, workBudget);
            trace << "batch=" << batch++ << ",work=" << pf->m_cumulativeCellsAllocated
                  << ",pending=" << pf->m_pathRequests->size() << '\n';
        }
        return trace.str();
    }
    QueueTiming benchmarkQueuedGround(int requestCount, int workBudget) {
        struct Query { Coord3D from, to; };
        std::vector<Query> queries(requestCount + 1);
        const auto width = pf->m_extent.hi.x - 3;
        const auto height = pf->m_extent.hi.y - 3;
        std::uint32_t random = 0x5a1793bd;
        auto next = [&] {
            random ^= random << 13; random ^= random >> 17; random ^= random << 5;
            return random;
        };
        auto clearPosition = [&] {
            for (;;) {
                const auto x = 2 + next() % width;
                const auto y = 2 + next() % height;
                if (pf->m_map[x][y].getType() == PathfindCell::CELL_CLEAR)
                    return Coord3D{x * 10.0f + 5, y * 10.0f + 5, 0};
            }
        };
        for (int i = 1; i <= requestCount; ++i) {
            queries[i].from = clearPosition();
            queries[i].to = clearPosition();
        }
        enqueue(requestCount);
        QueueTiming result;
        const auto start = std::chrono::steady_clock::now();
        while (!pf->m_pathRequests->empty()) {
            pf->m_cumulativeCellsAllocated = 0;
            const auto batchStart = std::chrono::steady_clock::now();
            navigation::processRequests(*pf->m_pathRequests, pf->m_cumulativeCellsAllocated,
                [&](std::uint32_t id) { return &queries[id]; },
                [&](Query* query) {
                    auto* path = pf->findGroundPath(&query->from, &query->to, 1, false);
                    if (path) { ++result.found; deleteInstance(path); return true; }
                    return false;
                }, workBudget);
            const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now() - batchStart).count();
            result.maxBatchNanoseconds = std::max(result.maxBatchNanoseconds, std::uint64_t(duration));
            result.work += pf->m_cumulativeCellsAllocated;
            ++result.batches;
        }
        result.nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - start).count();
        return result;
    }
private:
    struct SegmentContract {
        int pathDiameter;
        bool crusher;
        bool requireFullClearance;
        bool legal = true;
    };

    static Int validateGroundSegment(Pathfinder* pathfinder, PathfindCell*, PathfindCell* to,
                                     Int toX, Int toY, void* userData) {
        auto* contract = static_cast<SegmentContract*>(userData);
        if (!to) {
            contract->legal = false;
            return 1;
        }
        const Int clearDiameter = pathfinder->clearCellForDiameter(
            contract->crusher, toX, toY, to->getLayer(), contract->pathDiameter);
        if ((contract->requireFullClearance && clearDiameter != contract->pathDiameter) ||
            (!contract->requireFullClearance && clearDiameter <= 0) || to->getPinched()) {
            contract->legal = false;
            return 1;
        }
        return 0;
    }

    bool macroSegmentLegal(const Coord3D& from, PathfindLayerEnum layer,
                           const Coord3D& to, int pathDiameter, bool crusher) {
        SegmentContract contract{pathDiameter, crusher, true};
        const Int result = pf->iterateCellsAlongLine(
            from, to, layer, &World::validateGroundSegment, &contract);
        return result == 0 && contract.legal;
    }

    bool groundNeighborZoneLegal(int x, int y, PathfindLayerEnum layer) const {
        if (layer != LAYER_GROUND || pf->m_zoneManager.isPassable(x, y))
            return true;
        return pf->m_zoneManager.clipIsPassable(x + 3, y + 3) ||
            pf->m_zoneManager.clipIsPassable(x - 3, y + 3) ||
            pf->m_zoneManager.clipIsPassable(x + 3, y - 3) ||
            pf->m_zoneManager.clipIsPassable(x - 3, y - 3);
    }

    bool adjacentSegmentLegal(const ICoord2D& from, const ICoord2D& to,
                              PathfindLayerEnum layer, int pathDiameter, bool crusher) {
        auto* destination = pf->getCell(layer, to.x, to.y);
        if (!destination || destination->getType() != PathfindCell::CELL_CLEAR ||
            destination->getPinched() ||
            !groundNeighborZoneLegal(to.x, to.y, layer) ||
            pf->clearCellForDiameter(crusher, to.x, to.y, layer, pathDiameter) <= 0)
            return false;

        const int dx = to.x - from.x;
        const int dy = to.y - from.y;
        if (dx != 0 && dy != 0) {
            // Ground A* permits a diagonal when at least one of its two
            // cardinal neighbors is a valid movement cell.  Mirror that
            // neighborFlags contract instead of imposing a stricter corner
            // rule than the production search.
            const ICoord2D sideA{from.x + dx, from.y};
            const ICoord2D sideB{from.x, from.y + dy};
            const auto sideLegal = [&](const ICoord2D& side) {
                auto* cell = pf->getCell(layer, side.x, side.y);
                return cell && cell->getType() == PathfindCell::CELL_CLEAR &&
                    !cell->getPinched() &&
                    groundNeighborZoneLegal(side.x, side.y, layer);
            };
            if (!sideLegal(sideA) && !sideLegal(sideB))
                return false;
        }
        return true;
    }

    Result capture(Path* path, int pathDiameter, bool crusher) {
        Result result;
        result.found = path != nullptr;
        result.legal = path == nullptr;
        result.cells = pf->m_cumulativeCellsAllocated;
        if (path) {
            auto serialize = [](PathNode* node, bool optimized) {
                std::ostringstream out;
                out.imbue(std::locale::classic());
                out << std::setprecision(std::numeric_limits<Real>::max_digits10);
                int count = 0;
                while (node) {
                    if (++count > 10000) throw std::logic_error("Cyclic path");
                    if (count > 1) out << ' ';
                    const auto& p = *node->getPosition();
                    out << p.x << ',' << p.y << ',' << p.z << ':' << int(node->getLayer()) << ':' << node->getCanOptimize();
                    node = optimized ? node->getNextOptimized() : node->getNext();
                }
                return out.str();
            };
            result.raw = serialize(path->getFirstNode(), false);
            result.optimized = serialize(path->getFirstNode(), true);
            for (auto* node = path->getFirstNode(); node; node = node->getNext()) {
                if (node->getLayer() == LAYER_GROUND) {
                    ICoord2D cell;
                    pf->worldToCell(node->getPosition(), &cell);
                    result.rawGroundCells.push_back({cell.x, cell.y});
                }
            }
            for (auto* node = path->getFirstNode(); node && node->getNextOptimized();
                 node = node->getNextOptimized()) {
                ICoord2D start, end;
                pf->worldToCell(node->getPosition(), &start);
                pf->worldToCell(node->getNextOptimized()->getPosition(), &end);
                result.optimizedEdges = result.optimizedEdges ||
                    abs(end.x - start.x) > 1 || abs(end.y - start.y) > 1;
            }
            result.legal = true;
            for (auto* node = path->getFirstNode(); node && node->getNext(); node = node->getNext()) {
                auto* next = node->getNext();
                if (node->getLayer() != next->getLayer())
                    continue; // A same-coordinate bridge layer transition is legal by construction.
                ICoord2D startCell{};
                ICoord2D endCell{};
                pf->worldToCell(node->getPosition(), &startCell);
                pf->worldToCell(next->getPosition(), &endCell);
                if (!pf->getCell(node->getLayer(), startCell.x, startCell.y) ||
                    !pf->getCell(next->getLayer(), endCell.x, endCell.y))
                    continue; // The first node can intentionally be clipped outside the map.
                const int deltaX = endCell.x - startCell.x;
                const int deltaY = endCell.y - startCell.y;
                const int dx = deltaX < 0 ? -deltaX : deltaX;
                const int dy = deltaY < 0 ? -deltaY : deltaY;
                const bool macroEdge = dx > 1 || dy > 1;
                result.macroEdges = result.macroEdges || macroEdge;
                const bool legal = macroEdge
                    ? macroSegmentLegal(*node->getPosition(), node->getLayer(),
                                        *next->getPosition(), pathDiameter, crusher)
                    : adjacentSegmentLegal(startCell, endCell, node->getLayer(),
                                           pathDiameter, crusher);
                if (!legal) {
                    result.legal = false;
                    break;
                }
            }
            deleteInstance(path);
        }
        result.clean = pf->m_openList.empty() && pf->m_closedList.empty();
        for (int x = 0; x <= pf->m_extent.hi.x; ++x)
            for (int y = 0; y <= pf->m_extent.hi.y; ++y) {
                auto& cell = pf->m_map[x][y];
                result.clean = result.clean && (!cell.hasInfo() ||
                    (cell.getPosUnit() != INVALID_ID && !cell.getOpen() && !cell.getClosed()));
                auto* bridgeCell = pf->m_layers[LAYER_BRIDGE_1].getCell(x, y);
                if (bridgeCell) result.clean = result.clean && !bridgeCell->hasInfo();
            }
        return result;
    }
};
}

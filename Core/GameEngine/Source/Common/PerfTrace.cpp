#include "Common/PerfTrace.h"

#include "Common/GameEngine.h"
#include "Common/GameUtility.h"
#include "Common/Money.h"
#include "Common/Player.h"
#include "GameClient/Drawable.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <windows.h>

using namespace std::chrono;

namespace
{
Bool g_enabled = FALSE;
Int g_sampleFrames = 1;
Real g_spikeMS = 100.0f;
steady_clock::time_point g_sessionStart;
Bool g_sessionStarted = FALSE;
uint64_t g_engineFrame = 0;
PerfTrace::PathfindFrameStats g_pathCurrent;
PerfTrace::PathfindFrameStats g_pathLast;
PerfTrace::RenderFrameStats g_renderCurrent;
FILE *g_perfTraceFile = nullptr;
Bool g_perfTraceHeaderWritten = FALSE;
char g_perfTracePath[MAX_PATH] = {};

int64_t nowTicks()
{
	return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}

double elapsedMS(const int64_t startTicks)
{
	return static_cast<double>(nowTicks() - startTicks) / 1000.0;
}

uint64_t getUptimeMS()
{
	if (!g_sessionStarted)
	{
		return 0;
	}

	return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now() - g_sessionStart).count());
}

void appendTimestamp(char *buffer, size_t bufferLen)
{
	const auto now = system_clock::now();
	const auto nowMS = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
	const std::time_t tt = system_clock::to_time_t(now);
	std::tm tmValue{};
#if defined(_WIN32)
	localtime_s(&tmValue, &tt);
#else
	localtime_r(&tt, &tmValue);
#endif
	std::snprintf(buffer, bufferLen,
		"%04d-%02d-%02dT%02d:%02d:%02d.%03d",
		tmValue.tm_year + 1900,
		tmValue.tm_mon + 1,
		tmValue.tm_mday,
		tmValue.tm_hour,
		tmValue.tm_min,
		tmValue.tm_sec,
		static_cast<int>(nowMS.count()));
}

void closePerfTraceFile()
{
	if (g_perfTraceFile != nullptr)
	{
		std::fclose(g_perfTraceFile);
		g_perfTraceFile = nullptr;
	}
}

void openPerfTraceFileIfNeeded()
{
	if (!g_enabled || g_perfTraceFile != nullptr)
	{
		return;
	}

	if (g_perfTracePath[0] == '\0')
	{
		DWORD len = ::GetModuleFileNameA(nullptr, g_perfTracePath, MAX_PATH);
		if (len > 0 && len < MAX_PATH)
		{
			char *leaf = std::strrchr(g_perfTracePath, '\\');
			if (leaf != nullptr)
			{
				*(leaf + 1) = '\0';
				std::strncat(g_perfTracePath, "PerfTrace.txt", MAX_PATH - std::strlen(g_perfTracePath) - 1);
			}
			else
			{
				std::strncpy(g_perfTracePath, "PerfTrace.txt", MAX_PATH - 1);
				g_perfTracePath[MAX_PATH - 1] = '\0';
			}
		}
		else
		{
			std::strncpy(g_perfTracePath, "PerfTrace.txt", MAX_PATH - 1);
			g_perfTracePath[MAX_PATH - 1] = '\0';
		}
	}

	g_perfTraceFile = std::fopen(g_perfTracePath, "w");
	if (!g_perfTraceFile)
	{
		return;
	}

	if (!g_perfTraceHeaderWritten)
	{
		std::fputs(
			"timestamp,uptime_ms,frame,event,frame_ms,frame_fps,logic_updated,objects,structures,ground_units,air_units,infantry,vehicles,aircraft,"
			"path_queue,path_queue_max,path_queue_peak,path_cells,path_requests,path_enqueued,path_duplicate,path_failed,path_processed,"
			"explicit_req,explicit_enq,explicit_dup,explicit_fail,explicit_proc,"
			"attack_req,attack_enq,attack_dup,attack_fail,attack_proc,"
			"patch_req,patch_enq,patch_dup,patch_fail,patch_proc,"
			"safe_req,safe_enq,safe_dup,safe_fail,safe_proc,"
			"pathfind_update_ms,path_request_dispatch_ms,a_star_expand_ms,zone_update_ms,checkForMovement_ms,checkDestination_ms,move_allies_ms,"
			"render_submit_ms,draw_ms,nodes_popped,nodes_inserted,repaths_triggered,blocked_by_unit_checks,blocked_by_terrain_checks,zone_recomputes,bridge_special_terrain_checks,"
			"longest_single_request_cells,max_queue_wait_frames,units_waiting_path,units_active_path,ground_waiting_path,ground_active_path,air_waiting_path,air_active_path,"
			"local_player,local_side,local_money,selected_total,selected_structures,selected_ground,selected_air,selected_infantry,selected_vehicles,selected_aircraft,"
			"map_cells_x,map_cells_y,map_cells_total,logical_cells_x,logical_cells_y,logical_cells_total,zone_block_count_x,zone_block_count_y,zone_block_count_total,"
			"unit_footprint_cells_total_ground,unit_footprint_cells_total_air,unit_footprint_cells_total_all,avg_ground_unit_footprint_cells,avg_vehicle_footprint_cells,avg_infantry_footprint_cells,max_unit_footprint_cells,"
			"footprint_1x1_count,footprint_2x2_count,footprint_3x3_count,footprint_4x4_count,footprint_5x5_count,selected_avg_footprint_cells,selected_max_footprint_cells,"
			"path_queue_oldest_age,path_queue_explicit_oldest_age,queue_replaced,queue_suppressed_same_goal,queue_suppressed_weaker_than_existing,queue_service_pass1,queue_service_pass2,"
			"open_list_peak,closed_list_peak,cell_info_peak_live,cell_info_alloc_failures,cell_info_pool_capacity,request_cell_budget\n",
			g_perfTraceFile);
		g_perfTraceHeaderWritten = TRUE;
	}
}

Int requestClassIndex(const PerfTrace::PathRequestClass requestClass)
{
	const Int idx = static_cast<Int>(requestClass);
	return std::max(0, std::min(idx, PerfTrace::PATH_REQUEST_CLASS_COUNT - 1));
}

struct ObjectCounters
{
	Int totalObjects;
	Int structures;
	Int groundUnits;
	Int airUnits;
	Int infantry;
	Int vehicles;
	Int aircraft;
	Int unitsWaitingPath;
	Int unitsActivePath;
	Int groundWaitingPath;
	Int groundActivePath;
	Int airWaitingPath;
	Int airActivePath;
	Int footprintGroundTotal;
	Int footprintAirTotal;
	Int footprintAllTotal;
	Int vehicleFootprintTotal;
	Int infantryFootprintTotal;
	Int groundFootprintCount;
	Int vehicleFootprintCount;
	Int infantryFootprintCount;
	Int maxFootprintCells;
	Int footprintBuckets[5];

	ObjectCounters()
	{
		std::memset(this, 0, sizeof(*this));
	}
};

struct SelectionCounters
{
	Int total;
	Int structures;
	Int ground;
	Int air;
	Int infantry;
	Int vehicles;
	Int aircraft;
	Int footprintTotal;
	Int footprintMax;

	SelectionCounters()
	{
		std::memset(this, 0, sizeof(*this));
	}
};

void accumulateObjectCounts(ObjectCounters &counters)
{
	if (!TheGameLogic || !TheAI || !TheAI->pathfinder())
	{
		return;
	}

	Pathfinder *pathfinder = TheAI->pathfinder();
	for (Object *obj = TheGameLogic->getFirstObject(); obj; obj = obj->getNextObject())
	{
		++counters.totalObjects;

		const Bool isStructure = obj->isKindOf(KINDOF_STRUCTURE);
		const Bool isInfantry = obj->isKindOf(KINDOF_INFANTRY);
		const Bool isAircraft = obj->isKindOf(KINDOF_AIRCRAFT);
		const Bool isVehicle = obj->isKindOf(KINDOF_VEHICLE) && !isAircraft;

		if (isStructure)
		{
			++counters.structures;
		}
		if (isInfantry)
		{
			++counters.infantry;
			++counters.groundUnits;
		}
		if (isVehicle)
		{
			++counters.vehicles;
			++counters.groundUnits;
		}
		if (isAircraft)
		{
			++counters.aircraft;
			++counters.airUnits;
		}

		AIUpdateInterface *ai = obj->getAIUpdateInterface();
		if (ai != nullptr)
		{
			if (ai->isWaitingForPath())
			{
				++counters.unitsWaitingPath;
				if (isAircraft)
				{
					++counters.airWaitingPath;
				}
				else if (isInfantry || isVehicle)
				{
					++counters.groundWaitingPath;
				}
			}
			if (ai->getPath() != nullptr)
			{
				++counters.unitsActivePath;
				if (isAircraft)
				{
					++counters.airActivePath;
				}
				else if (isInfantry || isVehicle)
				{
					++counters.groundActivePath;
				}
			}
		}

		Int sideCells = 0;
		Int areaCells = 0;
		Int radius = 0;
		Bool center = false;
		pathfinder->computeFootprintMetrics(obj, sideCells, areaCells, radius, center);
		if (areaCells <= 0)
		{
			continue;
		}

		counters.maxFootprintCells = std::max(counters.maxFootprintCells, areaCells);
		counters.footprintAllTotal += areaCells;
		++counters.footprintBuckets[std::min(std::max(sideCells, 1), 5) - 1];

		if (isAircraft)
		{
			counters.footprintAirTotal += areaCells;
		}
		else if (isInfantry || isVehicle)
		{
			counters.footprintGroundTotal += areaCells;
			++counters.groundFootprintCount;
		}

		if (isVehicle)
		{
			counters.vehicleFootprintTotal += areaCells;
			++counters.vehicleFootprintCount;
		}
		if (isInfantry)
		{
			counters.infantryFootprintTotal += areaCells;
			++counters.infantryFootprintCount;
		}
	}
}

void accumulateSelectionCounts(SelectionCounters &counters)
{
	if (!TheInGameUI || !TheAI || !TheAI->pathfinder())
	{
		return;
	}

	const DrawableList *selected = TheInGameUI->getAllSelectedDrawables();
	if (!selected)
	{
		return;
	}

	Pathfinder *pathfinder = TheAI->pathfinder();
	for (DrawableListCIt it = selected->begin(); it != selected->end(); ++it)
	{
		Drawable *drawable = *it;
		if (!drawable || !drawable->getObject())
		{
			continue;
		}

		Object *obj = drawable->getObject();
		++counters.total;

		const Bool isStructure = obj->isKindOf(KINDOF_STRUCTURE);
		const Bool isInfantry = obj->isKindOf(KINDOF_INFANTRY);
		const Bool isAircraft = obj->isKindOf(KINDOF_AIRCRAFT);
		const Bool isVehicle = obj->isKindOf(KINDOF_VEHICLE) && !isAircraft;

		if (isStructure)
		{
			++counters.structures;
		}
		if (isInfantry || isVehicle)
		{
			++counters.ground;
		}
		if (isAircraft)
		{
			++counters.air;
		}
		if (isInfantry)
		{
			++counters.infantry;
		}
		if (isVehicle)
		{
			++counters.vehicles;
		}
		if (isAircraft)
		{
			++counters.aircraft;
		}

		Int sideCells = 0;
		Int areaCells = 0;
		Int radius = 0;
		Bool center = false;
		pathfinder->computeFootprintMetrics(obj, sideCells, areaCells, radius, center);
		counters.footprintTotal += areaCells;
		counters.footprintMax = std::max(counters.footprintMax, areaCells);
	}
}

void writeFrameSnapshot(const Bool logicUpdated, const double frameMS)
{
	openPerfTraceFileIfNeeded();
	if (!g_perfTraceFile || !TheGameLogic)
	{
		return;
	}

	const UnsignedInt logicFrame = TheGameLogic->getFrame();
	const UnsignedInt snapshotFrame = (logicUpdated && logicFrame > 0) ? (logicFrame - 1) : logicFrame;
	PerfTrace::PathfindFrameStats pathStats = PerfTrace::GetPathfindFrameStatsForFrame(snapshotFrame);
	if (logicUpdated && logicFrame > 0)
	{
		const PerfTrace::PathfindFrameStats nextFrameStats = PerfTrace::GetPathfindFrameStatsForFrame(logicFrame);
		if (nextFrameStats.requestAttempts > pathStats.requestAttempts ||
			nextFrameStats.requestEnqueued > pathStats.requestEnqueued ||
			nextFrameStats.requestProcessed > pathStats.requestProcessed ||
			nextFrameStats.pathQueuePeak > pathStats.pathQueuePeak ||
			nextFrameStats.pathCells > pathStats.pathCells)
		{
			pathStats = nextFrameStats;
		}
	}

	const Bool sampleFrame = logicUpdated && (snapshotFrame == 0 || snapshotFrame % static_cast<UnsignedInt>(std::max(g_sampleFrames, 1)) == 0);
	const Bool spikeFrame = frameMS >= static_cast<double>(g_spikeMS);
	if (!sampleFrame && !spikeFrame)
	{
		return;
	}

	ObjectCounters objectCounters;
	accumulateObjectCounts(objectCounters);

	SelectionCounters selectionCounters;
	accumulateSelectionCounts(selectionCounters);

	Int mapCellsX = 0;
	Int mapCellsY = 0;
	Int mapCellsTotal = 0;
	Int logicalCellsX = 0;
	Int logicalCellsY = 0;
	Int logicalCellsTotal = 0;
	Int zoneBlockCountX = 0;
	Int zoneBlockCountY = 0;
	Int zoneBlockCountTotal = 0;
	Int queueOldestAge = 0;
	Int queueExplicitOldestAge = 0;
	if (TheAI && TheAI->pathfinder())
	{
		TheAI->pathfinder()->getGridMetrics(
			mapCellsX, mapCellsY, mapCellsTotal,
			logicalCellsX, logicalCellsY, logicalCellsTotal,
			zoneBlockCountX, zoneBlockCountY, zoneBlockCountTotal);
		TheAI->pathfinder()->getQueueAgeMetrics(snapshotFrame, queueOldestAge, queueExplicitOldestAge);
	}

	Player *localPlayer = rts::getObservedOrLocalPlayer();
	const Int localPlayerIndex = localPlayer ? localPlayer->getPlayerIndex() : -1;
	const char *localSide = localPlayer ? localPlayer->getSide().str() : "";
	const UnsignedInt localMoney = localPlayer ? localPlayer->getMoney()->countMoney() : 0;

	const double frameFPS = frameMS > 0.0 ? 1000.0 / frameMS : 0.0;
	const double avgGroundFootprint = objectCounters.groundFootprintCount > 0
		? static_cast<double>(objectCounters.footprintGroundTotal) / static_cast<double>(objectCounters.groundFootprintCount)
		: 0.0;
	const double avgVehicleFootprint = objectCounters.vehicleFootprintCount > 0
		? static_cast<double>(objectCounters.vehicleFootprintTotal) / static_cast<double>(objectCounters.vehicleFootprintCount)
		: 0.0;
	const double avgInfantryFootprint = objectCounters.infantryFootprintCount > 0
		? static_cast<double>(objectCounters.infantryFootprintTotal) / static_cast<double>(objectCounters.infantryFootprintCount)
		: 0.0;
	const double selectedAvgFootprint = selectionCounters.total > 0
		? static_cast<double>(selectionCounters.footprintTotal) / static_cast<double>(selectionCounters.total)
		: 0.0;

	char timestamp[64] = {};
	appendTimestamp(timestamp, sizeof(timestamp));

	std::fprintf(
		g_perfTraceFile,
		"%s,%llu,%u,%s,%.3f,%.2f,%d,%d,%d,%d,%d,%d,%d,%d,"
		"%d,%d,%d,%d,%d,%d,%d,%d,%d,"
		"%d,%d,%d,%d,%d,"
		"%d,%d,%d,%d,%d,"
		"%d,%d,%d,%d,%d,"
		"%d,%d,%d,%d,%d,"
		"%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,"
		"%.3f,%.3f,%d,%d,%d,%d,%d,%d,%d,"
		"%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,%u,%d,%d,%d,%d,%d,%d,%d,"
		"%d,%d,%d,%d,%d,%d,%d,%d,%d,"
		"%d,%d,%d,%.3f,%.3f,%.3f,%d,"
		"%d,%d,%d,%d,%d,%.3f,%d,"
		"%d,%d,%d,%d,%d,%d,%d,"
		"%d,%d,%d,%d,%d,%d\n",
		timestamp,
		static_cast<unsigned long long>(getUptimeMS()),
		snapshotFrame,
		spikeFrame ? "SPIKE" : "SAMPLE",
		frameMS,
		frameFPS,
		logicUpdated ? 1 : 0,
		objectCounters.totalObjects,
		objectCounters.structures,
		objectCounters.groundUnits,
		objectCounters.airUnits,
		objectCounters.infantry,
		objectCounters.vehicles,
		objectCounters.aircraft,
		pathStats.pathQueue,
		PATHFIND_QUEUE_LEN,
		pathStats.pathQueuePeak,
		pathStats.pathCells,
		pathStats.requestAttempts,
		pathStats.requestEnqueued,
		pathStats.requestDuplicate,
		pathStats.requestFailed,
		pathStats.requestProcessed,
		pathStats.classAttempts[PerfTrace::PATH_REQUEST_EXPLICIT_MOVE],
		pathStats.classEnqueued[PerfTrace::PATH_REQUEST_EXPLICIT_MOVE],
		pathStats.classDuplicate[PerfTrace::PATH_REQUEST_EXPLICIT_MOVE],
		pathStats.classFailed[PerfTrace::PATH_REQUEST_EXPLICIT_MOVE],
		pathStats.classProcessed[PerfTrace::PATH_REQUEST_EXPLICIT_MOVE],
		pathStats.classAttempts[PerfTrace::PATH_REQUEST_ATTACK_MOVE_OR_ATTACK_PATH],
		pathStats.classEnqueued[PerfTrace::PATH_REQUEST_ATTACK_MOVE_OR_ATTACK_PATH],
		pathStats.classDuplicate[PerfTrace::PATH_REQUEST_ATTACK_MOVE_OR_ATTACK_PATH],
		pathStats.classFailed[PerfTrace::PATH_REQUEST_ATTACK_MOVE_OR_ATTACK_PATH],
		pathStats.classProcessed[PerfTrace::PATH_REQUEST_ATTACK_MOVE_OR_ATTACK_PATH],
		pathStats.classAttempts[PerfTrace::PATH_REQUEST_PATCH_OR_REPATH],
		pathStats.classEnqueued[PerfTrace::PATH_REQUEST_PATCH_OR_REPATH],
		pathStats.classDuplicate[PerfTrace::PATH_REQUEST_PATCH_OR_REPATH],
		pathStats.classFailed[PerfTrace::PATH_REQUEST_PATCH_OR_REPATH],
		pathStats.classProcessed[PerfTrace::PATH_REQUEST_PATCH_OR_REPATH],
		pathStats.classAttempts[PerfTrace::PATH_REQUEST_SAFE_PATH_OR_AUTONOMOUS],
		pathStats.classEnqueued[PerfTrace::PATH_REQUEST_SAFE_PATH_OR_AUTONOMOUS],
		pathStats.classDuplicate[PerfTrace::PATH_REQUEST_SAFE_PATH_OR_AUTONOMOUS],
		pathStats.classFailed[PerfTrace::PATH_REQUEST_SAFE_PATH_OR_AUTONOMOUS],
		pathStats.classProcessed[PerfTrace::PATH_REQUEST_SAFE_PATH_OR_AUTONOMOUS],
		pathStats.pathfindUpdateMS,
		pathStats.pathRequestDispatchMS,
		pathStats.aStarExpandMS,
		pathStats.zoneUpdateMS,
		pathStats.checkForMovementMS,
		pathStats.checkDestinationMS,
		pathStats.moveAlliesMS,
		g_renderCurrent.renderSubmitMS,
		g_renderCurrent.drawMS,
		pathStats.nodesPopped,
		pathStats.nodesInserted,
		pathStats.repathsTriggered,
		pathStats.blockedByUnitChecks,
		pathStats.blockedByTerrainChecks,
		pathStats.zoneRecomputes,
		pathStats.bridgeSpecialTerrainChecks,
		pathStats.longestSingleRequestCells,
		pathStats.maxQueueWaitFrames,
		objectCounters.unitsWaitingPath,
		objectCounters.unitsActivePath,
		objectCounters.groundWaitingPath,
		objectCounters.groundActivePath,
		objectCounters.airWaitingPath,
		objectCounters.airActivePath,
		localPlayerIndex,
		localSide,
		localMoney,
		selectionCounters.total,
		selectionCounters.structures,
		selectionCounters.ground,
		selectionCounters.air,
		selectionCounters.infantry,
		selectionCounters.vehicles,
		selectionCounters.aircraft,
		mapCellsX,
		mapCellsY,
		mapCellsTotal,
		logicalCellsX,
		logicalCellsY,
		logicalCellsTotal,
		zoneBlockCountX,
		zoneBlockCountY,
		zoneBlockCountTotal,
		objectCounters.footprintGroundTotal,
		objectCounters.footprintAirTotal,
		objectCounters.footprintAllTotal,
		avgGroundFootprint,
		avgVehicleFootprint,
		avgInfantryFootprint,
		objectCounters.maxFootprintCells,
		objectCounters.footprintBuckets[0],
		objectCounters.footprintBuckets[1],
		objectCounters.footprintBuckets[2],
		objectCounters.footprintBuckets[3],
		objectCounters.footprintBuckets[4],
		selectedAvgFootprint,
		selectionCounters.footprintMax,
		queueOldestAge,
		queueExplicitOldestAge,
		pathStats.queueReplaced,
		pathStats.queueSuppressedSameGoal,
		pathStats.queueSuppressedWeakerThanExisting,
		pathStats.queueServicePass1,
		pathStats.queueServicePass2,
		pathStats.openListPeak,
		pathStats.closedListPeak,
		pathStats.cellInfoPeakLive,
		pathStats.cellInfoAllocFailures,
		pathStats.cellInfoPoolCapacity,
		pathStats.requestCellBudget);
	std::fflush(g_perfTraceFile);
}
}

namespace PerfTrace
{
PathfindFrameStats::PathfindFrameStats()
{
	reset(0);
}

void PathfindFrameStats::reset(const UnsignedInt newFrame)
{
	frame = newFrame;
	pathQueue = 0;
	pathQueuePeak = 0;
	pathCells = 0;
	requestAttempts = 0;
	requestEnqueued = 0;
	requestDuplicate = 0;
	requestFailed = 0;
	requestProcessed = 0;
	std::memset(classAttempts, 0, sizeof(classAttempts));
	std::memset(classEnqueued, 0, sizeof(classEnqueued));
	std::memset(classDuplicate, 0, sizeof(classDuplicate));
	std::memset(classFailed, 0, sizeof(classFailed));
	std::memset(classProcessed, 0, sizeof(classProcessed));
	pathfindUpdateMS = 0.0;
	pathRequestDispatchMS = 0.0;
	aStarExpandMS = 0.0;
	zoneUpdateMS = 0.0;
	checkForMovementMS = 0.0;
	checkDestinationMS = 0.0;
	moveAlliesMS = 0.0;
	nodesPopped = 0;
	nodesInserted = 0;
	repathsTriggered = 0;
	blockedByUnitChecks = 0;
	blockedByTerrainChecks = 0;
	zoneRecomputes = 0;
	bridgeSpecialTerrainChecks = 0;
	longestSingleRequestCells = 0;
	maxQueueWaitFrames = 0;
	pathQueueOldestAge = 0;
	pathQueueExplicitOldestAge = 0;
	queueReplaced = 0;
	queueSuppressedSameGoal = 0;
	queueSuppressedWeakerThanExisting = 0;
	queueServicePass1 = 0;
	queueServicePass2 = 0;
	openListPeak = 0;
	closedListPeak = 0;
	cellInfoPeakLive = 0;
	cellInfoAllocFailures = 0;
	cellInfoPoolCapacity = 0;
	requestCellBudget = 0;
}

RenderFrameStats::RenderFrameStats()
{
	reset(0);
}

void RenderFrameStats::reset(const uint64_t newFrame)
{
	engineFrame = newFrame;
	renderSubmitMS = 0.0;
	drawMS = 0.0;
}

void SetEnabled(const Bool enabled)
{
	g_enabled = enabled;
	if (!g_enabled)
	{
		closePerfTraceFile();
	}
}

Bool IsEnabled()
{
	return g_enabled;
}

void SetSampleFrames(const Int sampleFrames)
{
	g_sampleFrames = std::max(sampleFrames, 1);
}

Int GetSampleFrames()
{
	return g_sampleFrames;
}

void SetSpikeMS(const Real spikeMS)
{
	g_spikeMS = std::max(spikeMS, 0.0f);
}

Real GetSpikeMS()
{
	return g_spikeMS;
}

void ResetSession()
{
	g_sessionStarted = TRUE;
	g_sessionStart = steady_clock::now();
	g_engineFrame = 0;
	g_pathCurrent.reset(0);
	g_pathLast.reset(0);
	g_renderCurrent.reset(0);
	closePerfTraceFile();
	g_perfTraceHeaderWritten = FALSE;
}

void Shutdown()
{
	closePerfTraceFile();
}

void BeginEngineFrame()
{
	if (!g_enabled)
	{
		return;
	}

	if (!g_sessionStarted)
	{
		ResetSession();
	}

	++g_engineFrame;
	g_renderCurrent.reset(g_engineFrame);
}

void EndEngineFrame(const Bool logicUpdated, const double frameMS)
{
	if (!g_enabled)
	{
		return;
	}

	writeFrameSnapshot(logicUpdated, frameMS);
}

uint64_t GetCurrentEngineFrame()
{
	return g_engineFrame;
}

void AdvancePathfindFrameStats(const UnsignedInt frame)
{
	if (!g_enabled)
	{
		return;
	}

	if (g_pathCurrent.frame != frame)
	{
		g_pathLast = g_pathCurrent;
		g_pathCurrent.reset(frame);
	}
}

PathfindFrameStats GetPathfindFrameStatsForFrame(const UnsignedInt frame)
{
	if (g_pathCurrent.frame == frame)
	{
		return g_pathCurrent;
	}
	if (g_pathLast.frame == frame)
	{
		return g_pathLast;
	}

	PathfindFrameStats emptyStats;
	emptyStats.reset(frame);
	return emptyStats;
}

#define PERFTRACE_NOTE(field, value) \
	AdvancePathfindFrameStats(frame); \
	g_pathCurrent.field += (value)

void NotePathQueueDepth(const UnsignedInt frame, const Int queueDepth)
{
	AdvancePathfindFrameStats(frame);
	g_pathCurrent.pathQueue = queueDepth;
	g_pathCurrent.pathQueuePeak = std::max(g_pathCurrent.pathQueuePeak, queueDepth);
}

void NotePathCells(const UnsignedInt frame, const Int cellCount)
{
	AdvancePathfindFrameStats(frame);
	g_pathCurrent.pathCells = cellCount;
}

void NotePathRequestAttempt(const UnsignedInt frame, const PathRequestClass requestClass)
{
	PERFTRACE_NOTE(requestAttempts, 1);
	++g_pathCurrent.classAttempts[requestClassIndex(requestClass)];
}

void NotePathRequestEnqueued(const UnsignedInt frame, const PathRequestClass requestClass)
{
	PERFTRACE_NOTE(requestEnqueued, 1);
	++g_pathCurrent.classEnqueued[requestClassIndex(requestClass)];
}

void NotePathRequestDuplicate(const UnsignedInt frame, const PathRequestClass requestClass)
{
	PERFTRACE_NOTE(requestDuplicate, 1);
	++g_pathCurrent.classDuplicate[requestClassIndex(requestClass)];
}

void NotePathRequestFailed(const UnsignedInt frame, const PathRequestClass requestClass)
{
	PERFTRACE_NOTE(requestFailed, 1);
	++g_pathCurrent.classFailed[requestClassIndex(requestClass)];
}

void NotePathRequestProcessed(const UnsignedInt frame, const PathRequestClass requestClass)
{
	PERFTRACE_NOTE(requestProcessed, 1);
	++g_pathCurrent.classProcessed[requestClassIndex(requestClass)];
}

void NoteQueueReplaced(const UnsignedInt frame) { PERFTRACE_NOTE(queueReplaced, 1); }
void NoteQueueSuppressedSameGoal(const UnsignedInt frame) { PERFTRACE_NOTE(queueSuppressedSameGoal, 1); }
void NoteQueueSuppressedWeakerThanExisting(const UnsignedInt frame) { PERFTRACE_NOTE(queueSuppressedWeakerThanExisting, 1); }

void NoteQueueServicePass(const UnsignedInt frame, const Int pass)
{
	AdvancePathfindFrameStats(frame);
	if (pass == 1)
	{
		++g_pathCurrent.queueServicePass1;
	}
	else if (pass == 2)
	{
		++g_pathCurrent.queueServicePass2;
	}
}

void NoteNodesPopped(const UnsignedInt frame, const Int count) { PERFTRACE_NOTE(nodesPopped, count); }
void NoteNodesInserted(const UnsignedInt frame, const Int count) { PERFTRACE_NOTE(nodesInserted, count); }
void NoteRepathTriggered(const UnsignedInt frame, const Int count) { PERFTRACE_NOTE(repathsTriggered, count); }
void NoteBlockedByUnitChecks(const UnsignedInt frame, const Int count) { PERFTRACE_NOTE(blockedByUnitChecks, count); }
void NoteBlockedByTerrainChecks(const UnsignedInt frame, const Int count) { PERFTRACE_NOTE(blockedByTerrainChecks, count); }
void NoteZoneRecomputes(const UnsignedInt frame, const Int count) { PERFTRACE_NOTE(zoneRecomputes, count); }
void NoteBridgeSpecialTerrainChecks(const UnsignedInt frame, const Int count) { PERFTRACE_NOTE(bridgeSpecialTerrainChecks, count); }

void NoteLongestSingleRequestCells(const UnsignedInt frame, const Int cellCount)
{
	AdvancePathfindFrameStats(frame);
	g_pathCurrent.longestSingleRequestCells = std::max(g_pathCurrent.longestSingleRequestCells, cellCount);
}

void NoteMaxQueueWaitFrames(const UnsignedInt frame, const Int frameCount)
{
	AdvancePathfindFrameStats(frame);
	g_pathCurrent.maxQueueWaitFrames = std::max(g_pathCurrent.maxQueueWaitFrames, frameCount);
}

void NoteQueueOldestAge(const UnsignedInt frame, const Int frameCount)
{
	AdvancePathfindFrameStats(frame);
	g_pathCurrent.pathQueueOldestAge = std::max(g_pathCurrent.pathQueueOldestAge, frameCount);
}

void NoteQueueExplicitOldestAge(const UnsignedInt frame, const Int frameCount)
{
	AdvancePathfindFrameStats(frame);
	g_pathCurrent.pathQueueExplicitOldestAge = std::max(g_pathCurrent.pathQueueExplicitOldestAge, frameCount);
}

void NoteOpenListPeak(const UnsignedInt frame, const Int count)
{
	AdvancePathfindFrameStats(frame);
	g_pathCurrent.openListPeak = std::max(g_pathCurrent.openListPeak, count);
}

void NoteClosedListPeak(const UnsignedInt frame, const Int count)
{
	AdvancePathfindFrameStats(frame);
	g_pathCurrent.closedListPeak = std::max(g_pathCurrent.closedListPeak, count);
}

void NoteCellInfoPeakLive(const UnsignedInt frame, const Int count)
{
	AdvancePathfindFrameStats(frame);
	g_pathCurrent.cellInfoPeakLive = std::max(g_pathCurrent.cellInfoPeakLive, count);
}

void NoteCellInfoAllocFailures(const UnsignedInt frame, const Int count)
{
	AdvancePathfindFrameStats(frame);
	g_pathCurrent.cellInfoAllocFailures += count;
}

void NoteCellInfoPoolCapacity(const UnsignedInt frame, const Int count)
{
	AdvancePathfindFrameStats(frame);
	g_pathCurrent.cellInfoPoolCapacity = count;
}

void NoteRequestCellBudget(const UnsignedInt frame, const Int budget)
{
	AdvancePathfindFrameStats(frame);
	g_pathCurrent.requestCellBudget = budget;
}

void NotePathTimer(const UnsignedInt frame, const PathTimerKind kind, const double elapsedMSValue)
{
	AdvancePathfindFrameStats(frame);
	switch (kind)
	{
		case PATH_TIMER_PATHFIND_UPDATE: g_pathCurrent.pathfindUpdateMS += elapsedMSValue; break;
		case PATH_TIMER_PATH_REQUEST_DISPATCH: g_pathCurrent.pathRequestDispatchMS += elapsedMSValue; break;
		case PATH_TIMER_ASTAR_EXPAND: g_pathCurrent.aStarExpandMS += elapsedMSValue; break;
		case PATH_TIMER_ZONE_UPDATE: g_pathCurrent.zoneUpdateMS += elapsedMSValue; break;
		case PATH_TIMER_CHECK_FOR_MOVEMENT: g_pathCurrent.checkForMovementMS += elapsedMSValue; break;
		case PATH_TIMER_CHECK_DESTINATION: g_pathCurrent.checkDestinationMS += elapsedMSValue; break;
		case PATH_TIMER_MOVE_ALLIES: g_pathCurrent.moveAlliesMS += elapsedMSValue; break;
	}
}

void NoteRenderTimer(const RenderTimerKind kind, const double elapsedMSValue)
{
	if (!g_enabled)
	{
		return;
	}

	switch (kind)
	{
		case RENDER_TIMER_SUBMIT: g_renderCurrent.renderSubmitMS += elapsedMSValue; break;
		case RENDER_TIMER_DRAW: g_renderCurrent.drawMS += elapsedMSValue; break;
	}
}

ScopedPathTimer::ScopedPathTimer(const UnsignedInt frame, const PathTimerKind kind)
	: m_frame(frame), m_kind(kind), m_startTicks(0)
{
	if (g_enabled)
	{
		m_startTicks = nowTicks();
	}
}

ScopedPathTimer::~ScopedPathTimer()
{
	if (!g_enabled || m_startTicks == 0)
	{
		return;
	}

	NotePathTimer(m_frame, m_kind, elapsedMS(m_startTicks));
}

ScopedRenderTimer::ScopedRenderTimer(const RenderTimerKind kind)
	: m_kind(kind), m_startTicks(0)
{
	if (g_enabled)
	{
		m_startTicks = nowTicks();
	}
}

ScopedRenderTimer::~ScopedRenderTimer()
{
	if (!g_enabled || m_startTicks == 0)
	{
		return;
	}

	NoteRenderTimer(m_kind, elapsedMS(m_startTicks));
}

#undef PERFTRACE_NOTE
}

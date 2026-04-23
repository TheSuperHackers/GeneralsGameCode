#pragma once

#include "Lib/BaseType.h"

#include <cstdint>

namespace PerfTrace
{
enum PathRequestClass
{
	PATH_REQUEST_EXPLICIT_MOVE = 0,
	PATH_REQUEST_ATTACK_MOVE_OR_ATTACK_PATH = 1,
	PATH_REQUEST_PATCH_OR_REPATH = 2,
	PATH_REQUEST_SAFE_PATH_OR_AUTONOMOUS = 3,
	PATH_REQUEST_CLASS_COUNT = 4,
};

enum PathTimerKind
{
	PATH_TIMER_PATHFIND_UPDATE = 0,
	PATH_TIMER_PATH_REQUEST_DISPATCH,
	PATH_TIMER_ASTAR_EXPAND,
	PATH_TIMER_ZONE_UPDATE,
	PATH_TIMER_CHECK_FOR_MOVEMENT,
	PATH_TIMER_CHECK_DESTINATION,
	PATH_TIMER_MOVE_ALLIES,
};

enum RenderTimerKind
{
	RENDER_TIMER_SUBMIT = 0,
	RENDER_TIMER_DRAW,
};

struct PathfindFrameStats
{
	UnsignedInt frame;
	Int pathQueue;
	Int pathQueuePeak;
	Int pathCells;
	Int requestAttempts;
	Int requestEnqueued;
	Int requestDuplicate;
	Int requestFailed;
	Int requestProcessed;
	Int classAttempts[PATH_REQUEST_CLASS_COUNT];
	Int classEnqueued[PATH_REQUEST_CLASS_COUNT];
	Int classDuplicate[PATH_REQUEST_CLASS_COUNT];
	Int classFailed[PATH_REQUEST_CLASS_COUNT];
	Int classProcessed[PATH_REQUEST_CLASS_COUNT];
	double pathfindUpdateMS;
	double pathRequestDispatchMS;
	double aStarExpandMS;
	double zoneUpdateMS;
	double checkForMovementMS;
	double checkDestinationMS;
	double moveAlliesMS;
	Int nodesPopped;
	Int nodesInserted;
	Int repathsTriggered;
	Int blockedByUnitChecks;
	Int blockedByTerrainChecks;
	Int zoneRecomputes;
	Int bridgeSpecialTerrainChecks;
	Int longestSingleRequestCells;
	Int maxQueueWaitFrames;
	Int pathQueueOldestAge;
	Int pathQueueExplicitOldestAge;
	Int queueReplaced;
	Int queueSuppressedSameGoal;
	Int queueSuppressedWeakerThanExisting;
	Int queueServicePass1;
	Int queueServicePass2;
	Int openListPeak;
	Int closedListPeak;
	Int cellInfoPeakLive;
	Int cellInfoAllocFailures;
	Int cellInfoPoolCapacity;
	Int requestCellBudget;

	PathfindFrameStats();
	void reset(UnsignedInt newFrame);
};

struct RenderFrameStats
{
	uint64_t engineFrame;
	double renderSubmitMS;
	double drawMS;

	RenderFrameStats();
	void reset(uint64_t newFrame);
};

void SetEnabled(Bool enabled);
Bool IsEnabled();
void SetSampleFrames(Int sampleFrames);
Int GetSampleFrames();
void SetSpikeMS(Real spikeMS);
Real GetSpikeMS();

void ResetSession();
void Shutdown();

void BeginEngineFrame();
void EndEngineFrame(Bool logicUpdated, double frameMS);
uint64_t GetCurrentEngineFrame();

void AdvancePathfindFrameStats(UnsignedInt frame);
PathfindFrameStats GetPathfindFrameStatsForFrame(UnsignedInt frame);

void NotePathQueueDepth(UnsignedInt frame, Int queueDepth);
void NotePathCells(UnsignedInt frame, Int cellCount);
void NotePathRequestAttempt(UnsignedInt frame, PathRequestClass requestClass);
void NotePathRequestEnqueued(UnsignedInt frame, PathRequestClass requestClass);
void NotePathRequestDuplicate(UnsignedInt frame, PathRequestClass requestClass);
void NotePathRequestFailed(UnsignedInt frame, PathRequestClass requestClass);
void NotePathRequestProcessed(UnsignedInt frame, PathRequestClass requestClass);
void NoteQueueReplaced(UnsignedInt frame);
void NoteQueueSuppressedSameGoal(UnsignedInt frame);
void NoteQueueSuppressedWeakerThanExisting(UnsignedInt frame);
void NoteQueueServicePass(UnsignedInt frame, Int pass);
void NoteNodesPopped(UnsignedInt frame, Int count = 1);
void NoteNodesInserted(UnsignedInt frame, Int count = 1);
void NoteRepathTriggered(UnsignedInt frame, Int count = 1);
void NoteBlockedByUnitChecks(UnsignedInt frame, Int count = 1);
void NoteBlockedByTerrainChecks(UnsignedInt frame, Int count = 1);
void NoteZoneRecomputes(UnsignedInt frame, Int count = 1);
void NoteBridgeSpecialTerrainChecks(UnsignedInt frame, Int count = 1);
void NoteLongestSingleRequestCells(UnsignedInt frame, Int cellCount);
void NoteMaxQueueWaitFrames(UnsignedInt frame, Int frameCount);
void NoteQueueOldestAge(UnsignedInt frame, Int frameCount);
void NoteQueueExplicitOldestAge(UnsignedInt frame, Int frameCount);

void NoteOpenListPeak(UnsignedInt frame, Int count);
void NoteClosedListPeak(UnsignedInt frame, Int count);
void NoteCellInfoPeakLive(UnsignedInt frame, Int count);
void NoteCellInfoAllocFailures(UnsignedInt frame, Int count);
void NoteCellInfoPoolCapacity(UnsignedInt frame, Int count);
void NoteRequestCellBudget(UnsignedInt frame, Int budget);

void NotePathTimer(UnsignedInt frame, PathTimerKind kind, double elapsedMS);
void NoteRenderTimer(RenderTimerKind kind, double elapsedMS);

class ScopedPathTimer
{
public:
	ScopedPathTimer(UnsignedInt frame, PathTimerKind kind);
	~ScopedPathTimer();

private:
	UnsignedInt m_frame;
	PathTimerKind m_kind;
	int64_t m_startTicks;
};

class ScopedRenderTimer
{
public:
	explicit ScopedRenderTimer(RenderTimerKind kind);
	~ScopedRenderTimer();

private:
	RenderTimerKind m_kind;
	int64_t m_startTicks;
};
}

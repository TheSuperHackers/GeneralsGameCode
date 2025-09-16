/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: GameLOD.cpp ///////////////////////////////////////////////////////////
//
// Used to set detail levels of various game systems.
//
// Author: Mark Wilczynski, Sept 2002
//
//
///////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameLOD.h"
#include "GameClient/TerrainVisual.h"
#include "GameClient/GameClient.h"
#include "Common/UserPreferences.h"

#define DEFINE_PARTICLE_SYSTEM_NAMES
#include "GameClient/ParticleSys.h"


#define PROFILE_ERROR_LIMIT	0.94f	//fraction of profiled result needed to get a match.  Allows some room for error/fluctuation.

//Hack to get access to a static method on the W3DDevice side. -MW
extern Bool testMinimumRequirements(ChipsetType *videoChipType, CpuType *cpuType, Int *cpuFreq, MemValueType *numRAM, Real *intBenchIndex, Real *floatBenchIndex, Real *memBenchIndex);

GameLODManager *TheGameLODManager=NULL;

static const FieldParse TheStaticGameLODFieldParseTable[] =
{
	{ "MinimumFPS",						INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_minFPS)},
	{ "MinimumProcessorFps",			INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_minProcessorFPS)},
	{ "SampleCount2D",					INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_sampleCount2D ) },
	{ "SampleCount3D",					INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_sampleCount3D ) },
	{ "StreamCount",					INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_streamCount ) },
	{ "MaxParticleCount",				INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_maxParticleCount ) },
	{ "UseShadowVolumes",				INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useShadowVolumes ) },
	{ "UseShadowDecals",				INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useShadowDecals ) },
	{ "UseCloudMap",					INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useCloudMap ) },
	{ "UseLightMap",					INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useLightMap ) },
	{ "ShowSoftWaterEdge",				INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_showSoftWaterEdge ) },
	{ "MaxTankTrackEdges",				INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_maxTankTrackEdges) },
	{ "MaxTankTrackOpaqueEdges",		INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_maxTankTrackOpaqueEdges) },
	{ "MaxTankTrackFadeDelay",			INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_maxTankTrackFadeDelay) },
	{ "UseBuildupScaffolds",			INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useBuildupScaffolds ) },
	{ "UseTreeSway",					INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useTreeSway ) },
	{ "UseEmissiveNightMaterials",		INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useEmissiveNightMaterials ) },
	{ "UseHeatEffects",					INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useHeatEffects ) },
	{ "TextureReductionFactor",		INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_textureReduction ) },
};

static const char *const StaticGameLODNames[]=
{
	"Low",
	"Medium",
	"High",
	"Custom"
};
static_assert(ARRAY_SIZE(StaticGameLODNames) == STATIC_GAME_LOD_COUNT, "Incorrect array size");

StaticGameLODInfo::StaticGameLODInfo(void)
{
	m_minFPS=0;
	m_minProcessorFPS=0;
	m_sampleCount2D=6;
	m_sampleCount3D=24;
	m_streamCount=2;
	m_maxParticleCount=2500;

	m_useShadowVolumes=TRUE;
	m_useShadowDecals=TRUE;
	m_useCloudMap=TRUE;
	m_useLightMap=TRUE;
	m_showSoftWaterEdge=TRUE;
	m_maxTankTrackEdges=100;
	m_maxTankTrackOpaqueEdges=25;
	m_maxTankTrackFadeDelay=300000;
	m_useBuildupScaffolds=TRUE;
	m_useTreeSway=TRUE;
	m_useEmissiveNightMaterials=TRUE;
	m_useHeatEffects=TRUE;
	m_textureReduction = 0;	//none
	m_useFpsLimit = TRUE;
	m_enableDynamicLOD = TRUE;
	m_useTrees = TRUE;
}

static const FieldParse TheDynamicGameLODFieldParseTable[] =
{
	{ "MinimumFPS",						INI::parseInt,					NULL,	offsetof( DynamicGameLODInfo, m_minFPS)},
	{ "ParticleSkipMask",				INI::parseInt,					NULL,	offsetof( DynamicGameLODInfo, m_dynamicParticleSkipMask)},
	{ "DebrisSkipMask",					INI::parseInt,					NULL,	offsetof( DynamicGameLODInfo, m_dynamicDebrisSkipMask)},
	{ "SlowDeathScale",					INI::parseReal,					NULL,	offsetof( DynamicGameLODInfo, m_slowDeathScale)},
	{ "MinParticlePriority",			INI::parseIndexList, ParticlePriorityNames,	offsetof( DynamicGameLODInfo, m_minDynamicParticlePriority)},
	{ "MinParticleSkipPriority",		INI::parseIndexList, ParticlePriorityNames,	offsetof( DynamicGameLODInfo, m_minDynamicParticleSkipPriority)},
};

static const char *const DynamicGameLODNames[]=
{
	"Low",
	"Medium",
	"High",
	"VeryHigh"
};
static_assert(ARRAY_SIZE(DynamicGameLODNames) == DYNAMIC_GAME_LOD_COUNT, "Incorrect array size");

DynamicGameLODInfo::DynamicGameLODInfo(void)
{
	m_minFPS=0;
	m_dynamicParticleSkipMask=0;
	m_dynamicDebrisSkipMask=0;
	m_slowDeathScale=1.0f;
	m_minDynamicParticlePriority = PARTICLE_PRIORITY_LOWEST;
	m_minDynamicParticleSkipPriority = PARTICLE_PRIORITY_LOWEST;
};

//Keep this in sync with enum in GameLOD.h
static const char *const CPUNames[] =
{
	"XX","P3", "P4","K7", NULL
};
static_assert(ARRAY_SIZE(CPUNames) == CPU_MAX + 1, "Incorrect array size");

//Keep this in sync with enum in GameLOD.h
static const char *const VideoNames[] =
{
	"XX","V2","V3","V4","V5","TNT","TNT2","GF2","R100","PS11","GF3","GF4","PS14","R200","PS20","R300", NULL
};
static_assert(ARRAY_SIZE(VideoNames) == DC_MAX + 1, "Incorrect array size");

void parseReallyLowMHz(INI* ini)
{
	Int mhz;
	INI::parseInt(ini,NULL,&mhz,NULL);
	if (TheGameLODManager)
	{
		TheGameLODManager->setReallyLowMHz(mhz);
	}
}

void INI::parseBenchProfile( INI* ini)
{
	if( TheGameLODManager )
	{
			BenchProfile *preset = TheGameLODManager->newBenchProfile();

			if (preset)
			{
				INI::parseIndexList(ini,NULL,&preset->m_cpuType,CPUNames);
				INI::parseInt(ini,NULL,&preset->m_mhz,NULL);
				INI::parseReal(ini,NULL,&preset->m_intBenchIndex,NULL);
				INI::parseReal(ini,NULL,&preset->m_floatBenchIndex,NULL);
				INI::parseReal(ini,NULL,&preset->m_memBenchIndex,NULL);
			}
	}
}

/**Parse a description of all the LOD settings for a given detail level*/
void INI::parseLODPreset( INI* ini )
{
	const char *c;
	AsciiString name;

	// read the name
	c = ini->getNextToken();
	name.set( c );	//name of detail level - low, medium, high

	if( TheGameLODManager )
	{
		StaticGameLODLevel index = (StaticGameLODLevel)TheGameLODManager->getStaticGameLODIndex(name);
		if (index != STATIC_GAME_LOD_UNKNOWN)
		{
			LODPresetInfo *preset = TheGameLODManager->newLODPreset(index);

			if (preset)
			{
				INI::parseIndexList(ini,NULL,&preset->m_cpuType,CPUNames);
				INI::parseInt(ini,NULL,&preset->m_mhz,NULL);
				INI::parseIndexList(ini,NULL,&preset->m_videoType,VideoNames);
				INI::parseInt(ini,NULL,&preset->m_memory,NULL);
			}
		}
	}
}

GameLODManager::GameLODManager(void)
{
	m_currentStaticLOD = STATIC_GAME_LOD_UNKNOWN;
	m_currentDynamicLOD = DYNAMIC_GAME_LOD_HIGH;
	m_numParticleGenerations=0;
	m_dynamicParticleSkipMask=0;
	m_numDebrisGenerations=0;
	m_dynamicDebrisSkipMask=0;
	m_videoPassed=false;
	m_cpuPassed=false;
	m_memPassed=false;
	m_slowDeathScale=1.0f;
	m_idealDetailLevel = STATIC_GAME_LOD_UNKNOWN;
	m_videoChipType = DC_MAX;
	m_cpuType = XX;
	m_numRAM=0;
	m_cpuFreq=0;
	m_intBenchIndex=0;
	m_floatBenchIndex=0;
	m_memBenchIndex=0;
	m_compositeBenchIndex=0;
	m_numBenchProfiles=0;
	m_currentTextureReduction=0;
	m_reallyLowMHz = 400;

	for (Int i=0; i<STATIC_GAME_LOD_CUSTOM; i++)
		m_numLevelPresets[i]=0;
};

GameLODManager::~GameLODManager()
{

}

BenchProfile *GameLODManager::newBenchProfile(void)
{
	if (m_numBenchProfiles < MAX_BENCH_PROFILES)
	{
		m_numBenchProfiles++;
		return &m_benchProfiles[m_numBenchProfiles-1];
	}

	DEBUG_CRASH(( "GameLODManager::newBenchProfile - Too many profiles defined"));
	return NULL;
}

LODPresetInfo *GameLODManager::newLODPreset(StaticGameLODLevel index)
{
	if (index >= 0 && index < STATIC_GAME_LOD_COUNT)
	{
		if (m_numLevelPresets[index] < MAX_LOD_PRESETS_PER_LEVEL)
		{
			m_numLevelPresets[index]++;
			return &m_lodPresets[index][m_numLevelPresets[index]-1];
		}

		DEBUG_CRASH(( "GameLODManager::newLODPreset - Too many presets defined for '%s'", TheGameLODManager->getStaticGameLODLevelName(index)));
	}

	return NULL;
}

void GameLODManager::init(void)
{
	INI ini;
	//Get Presets for each LOD level.
	ini.loadFileDirectory( AsciiString( "Data\\INI\\GameLOD" ), INI_LOAD_OVERWRITE, NULL );

	//Get presets for each known hardware configuration
	ini.loadFileDirectory( AsciiString( "Data\\INI\\GameLODPresets"), INI_LOAD_OVERWRITE, NULL);

	//Get Presets for custom LOD level by pulling them out of initial globaldata (which should
	//have all settings already applied).
	refreshCustomStaticLODLevel();

	//Override with user preferences
	OptionPreferences optionPref;

	StaticGameLODLevel userSetDetail=(StaticGameLODLevel)optionPref.getStaticGameDetail();

	m_idealDetailLevel=(StaticGameLODLevel)optionPref.getIdealStaticGameDetail();

	//always get this data in case we need it later.
	testMinimumRequirements(NULL,&m_cpuType,&m_cpuFreq,&m_numRAM,NULL,NULL,NULL);

	if ((Real)(m_numRAM)/(Real)(256*1024*1024) >= PROFILE_ERROR_LIMIT)
		m_memPassed=TRUE;	//check if they have at least 256 MB

	if (m_idealDetailLevel == STATIC_GAME_LOD_UNKNOWN || TheGlobalData->m_forceBenchmark)
	{
		if (m_cpuType == XX || TheGlobalData->m_forceBenchmark)
		{
			//need to run the benchmark
			testMinimumRequirements(NULL,NULL,NULL,NULL,&m_intBenchIndex,&m_floatBenchIndex,&m_memBenchIndex);

			if (TheGlobalData->m_forceBenchmark)
			{	//we want to see the numbers.  So dump them to a logfile.
				FILE *fp=fopen("Benchmark.txt","w");
				if (fp)
				{
					fprintf(fp,"BenchProfile = %s %d %f %f %f", CPUNames[m_cpuType], m_cpuFreq, m_intBenchIndex, m_floatBenchIndex, m_memBenchIndex);
					fclose(fp);
				}
			}

	 		m_compositeBenchIndex = m_intBenchIndex + m_floatBenchIndex;	///@todo: Need to scale these based on our apps usage of int/float/mem ops.

			StaticGameLODLevel currentLevel=STATIC_GAME_LOD_LOW;
			BenchProfile *prof=m_benchProfiles;
			m_cpuType = P3;	//assume lowest spec.
			m_cpuFreq = 1000;	//assume lowest spec.
			for (Int k=0; k<m_numBenchProfiles; k++)
			{
				//Check if we're within 5% of the performance of this cpu profile.
				if (m_intBenchIndex/prof->m_intBenchIndex >= PROFILE_ERROR_LIMIT && m_floatBenchIndex/prof->m_floatBenchIndex >= PROFILE_ERROR_LIMIT && m_memBenchIndex/prof->m_memBenchIndex >= PROFILE_ERROR_LIMIT)
				{
					for (Int i=STATIC_GAME_LOD_HIGH; i >= STATIC_GAME_LOD_LOW; i--)
					{
						LODPresetInfo *preset=&m_lodPresets[i][0];	//pointer to first preset at this LOD level.
						for (Int j=0; j<m_numLevelPresets[i]; j++)
						{
							if(	prof->m_cpuType == preset->m_cpuType &&	((Real)prof->m_mhz/(Real)preset->m_mhz >= PROFILE_ERROR_LIMIT))
							{	currentLevel = (StaticGameLODLevel)i;
								m_cpuType = prof->m_cpuType;
								m_cpuFreq = prof->m_mhz;
								break;
							}
							preset++;	//skip to next preset
						}
						if (currentLevel >= i)
							break;	//we already found a higher level than the remaining presets so no need to keep searching.
					}
				}
				prof++;
			}
		}
	}

	if (userSetDetail == STATIC_GAME_LOD_CUSTOM)
	{
		TheWritableGlobalData->m_textureReductionFactor = optionPref.getTextureReduction();
		TheWritableGlobalData->m_useShadowVolumes = optionPref.get3DShadowsEnabled();
		TheWritableGlobalData->m_useShadowDecals = optionPref.get2DShadowsEnabled();
		TheWritableGlobalData->m_enableBehindBuildingMarkers = optionPref.getBuildingOcclusionEnabled();
		TheWritableGlobalData->m_maxParticleCount = optionPref.getParticleCap();
		TheWritableGlobalData->m_enableDynamicLOD = optionPref.getDynamicLODEnabled();
		TheWritableGlobalData->m_useFpsLimit = optionPref.getFPSLimitEnabled();
		TheWritableGlobalData->m_useLightMap = optionPref.getLightmapEnabled();
		TheWritableGlobalData->m_useCloudMap = optionPref.getCloudShadowsEnabled();
		TheWritableGlobalData->m_showSoftWaterEdge = optionPref.getSmoothWaterEnabled();
		TheWritableGlobalData->m_useHeatEffects = optionPref.getUseHeatEffects();
		TheWritableGlobalData->m_useDrawModuleLOD = optionPref.getExtraAnimationsDisabled();
		TheWritableGlobalData->m_useTreeSway = !TheWritableGlobalData->m_useDrawModuleLOD;	//borrow same setting.
		TheWritableGlobalData->m_useTrees = optionPref.getTreesEnabled();
	}

	setStaticLODLevel(userSetDetail);
}

void GameLODManager::refreshCustomStaticLODLevel(void)
{
	StaticGameLODInfo *lodInfo=&m_staticGameLODInfo[STATIC_GAME_LOD_CUSTOM];

	lodInfo->m_maxParticleCount=TheGlobalData->m_maxParticleCount;
	lodInfo->m_useShadowVolumes=TheGlobalData->m_useShadowVolumes;
	lodInfo->m_useShadowDecals=TheGlobalData->m_useShadowDecals;
	lodInfo->m_useCloudMap=TheGlobalData->m_useCloudMap;
	lodInfo->m_useLightMap=TheGlobalData->m_useLightMap;
	lodInfo->m_showSoftWaterEdge=TheGlobalData->m_showSoftWaterEdge;
	lodInfo->m_maxTankTrackEdges=TheGlobalData->m_maxTankTrackEdges;
	lodInfo->m_maxTankTrackOpaqueEdges=TheGlobalData->m_maxTankTrackOpaqueEdges;
	lodInfo->m_maxTankTrackFadeDelay=TheGlobalData->m_maxTankTrackFadeDelay;
	lodInfo->m_useBuildupScaffolds=!TheGlobalData->m_useDrawModuleLOD;
	lodInfo->m_useHeatEffects = TheGlobalData->m_useHeatEffects;
	lodInfo->m_useTreeSway=lodInfo->m_useBuildupScaffolds;// Borrow same setting. //TheGlobalData->m_useTreeSway;
	lodInfo->m_textureReduction=TheGlobalData->m_textureReductionFactor;
	lodInfo->m_useFpsLimit = TheGlobalData->m_useFpsLimit;
	lodInfo->m_enableDynamicLOD=TheGlobalData->m_enableDynamicLOD;
	lodInfo->m_useTrees = TheGlobalData->m_useTrees;

}

/**Convert LOD name to an index*/
Int GameLODManager::getStaticGameLODIndex(AsciiString name)
{
	for (Int i=0; i<STATIC_GAME_LOD_COUNT; ++i)
	{
		if (name.compareNoCase(StaticGameLODNames[i]) == 0)
			return i;
	}

	DEBUG_CRASH(( "GameLODManager::getGameLODIndex - Invalid LOD name '%s'", name.str() ));
	return STATIC_GAME_LOD_UNKNOWN;
}

/**Parse a description of all the LOD settings for a given detail level*/
void INI::parseStaticGameLODDefinition( INI* ini )
{
	const char *c;
	AsciiString name;

	// read the name
	c = ini->getNextToken();
	name.set( c );

	if( TheGameLODManager )
	{
		Int index = TheGameLODManager->getStaticGameLODIndex(name);
		if (index != STATIC_GAME_LOD_UNKNOWN)
		{
			StaticGameLODInfo *lodInfo = &(TheGameLODManager->m_staticGameLODInfo[index]);

			// parse the ini definition
			ini->initFromINI( lodInfo, TheStaticGameLODFieldParseTable );
		}
	}
}

/**Parse an LOD level*/
void INI::parseStaticGameLODLevel( INI* ini, void * , void *store, const void*)
{
	const char *tok=ini->getNextToken();
	for (Int i=0; i<STATIC_GAME_LOD_COUNT; i++)
		if( stricmp(tok, StaticGameLODNames[i]) == 0 )
		{	*(StaticGameLODLevel*)store = (StaticGameLODLevel)i;
			return;
		}

	DEBUG_CRASH(("invalid GameLODLevel token %s -- expected LOW/MEDIUM/HIGH",tok));
	throw INI_INVALID_DATA;
}

const char *GameLODManager::getStaticGameLODLevelName(StaticGameLODLevel level)
{
	return StaticGameLODNames[level];
}

/**Function which calculates the recommended LOD level for current hardware
configuration.*/
StaticGameLODLevel GameLODManager::findStaticLODLevel(void)
{
	//Check if we have never done the test on current system
	if (m_idealDetailLevel == STATIC_GAME_LOD_UNKNOWN)
	{
		//search all our presets for matching hardware
		m_idealDetailLevel = STATIC_GAME_LOD_LOW;

		//get system configuration - only need vide chip type, got rest in ::init().
		testMinimumRequirements(&m_videoChipType,NULL,NULL,NULL,NULL,NULL,NULL);
		if (m_videoChipType == DC_UNKNOWN)
			m_videoChipType = DC_TNT2;	//presume it's at least TNT2 level

		Int numMBRam=m_numRAM/(1024*1024);

		for (Int i=STATIC_GAME_LOD_HIGH; i >= STATIC_GAME_LOD_LOW; i--)
		{
				LODPresetInfo *preset=&m_lodPresets[i][0];	//pointer to first preset at this LOD level.
				for (Int j=0; j<m_numLevelPresets[i]; j++)
				{

					if(	m_cpuType == preset->m_cpuType &&
							((Real)m_cpuFreq/(Real)preset->m_mhz >= PROFILE_ERROR_LIMIT) &&//make sure we're within 5% or higher
							m_videoChipType >= preset->m_videoType &&
							((Real)numMBRam/(Real)preset->m_memory >= PROFILE_ERROR_LIMIT)
						)
					{	m_idealDetailLevel = (StaticGameLODLevel)i;
						break;
					}

					preset++;	//skip to next preset

				}
				if (m_idealDetailLevel >= i)
					break;	//we already found a higher level than the remaining presets so no need to keep searching.
		}
		//Save ideal detail level for future usage
		OptionPreferences optionPref;
		optionPref["IdealStaticGameLOD"] = getStaticGameLODLevelName(m_idealDetailLevel);
		if (getStaticLODLevel() == STATIC_GAME_LOD_UNKNOWN)	//save for future usage.
			optionPref["StaticGameLOD"] = getStaticGameLODLevelName(m_idealDetailLevel);
		optionPref.write();
	}

	return m_idealDetailLevel;
}

/**Set all game systems to match the desired LOD level.*/
Bool GameLODManager::setStaticLODLevel(StaticGameLODLevel level)
{
	if (!TheGlobalData->m_enableStaticLOD)
	{	m_currentStaticLOD = STATIC_GAME_LOD_CUSTOM;
		return FALSE;
	}

	if (level == STATIC_GAME_LOD_UNKNOWN || (level != STATIC_GAME_LOD_CUSTOM && m_currentStaticLOD == level))
		return FALSE;	//level is already applied.  Custom levels are always applied since random options could change.

	applyStaticLODLevel(level);
	m_currentStaticLOD = level;

	return TRUE;
}

void GameLODManager::applyStaticLODLevel(StaticGameLODLevel level)
{
///@todo: Still need to implement these settings:
//	m_sampleCount2D=6;
//	m_sampleCount3D=24;
//	m_streamCount=2;
//	m_useEmissiveNightMaterials=TRUE;

	//save previous info for this level since it may be overwritten by refreshCustomStaticLODLevel().
	StaticGameLODInfo prevLodBackup;
	if (m_currentStaticLOD != STATIC_GAME_LOD_UNKNOWN)
		prevLodBackup=m_staticGameLODInfo[m_currentStaticLOD];

	if (level == STATIC_GAME_LOD_CUSTOM)
		refreshCustomStaticLODLevel();	//store current settings into custom preset

	StaticGameLODInfo *lodInfo=&m_staticGameLODInfo[level];
	StaticGameLODInfo *prevLodInfo=&prevLodBackup;

	Int requestedTextureReduction = 0;
	Bool requestedTrees = m_memPassed;	//only use trees if memory requirement passed.
	if (level == STATIC_GAME_LOD_CUSTOM)
	{	requestedTextureReduction = lodInfo->m_textureReduction;
		requestedTrees = lodInfo->m_useTrees;
	}
	else
	if (level >= STATIC_GAME_LOD_LOW)
	{	//normal non-custom level gets texture reduction based on recommendation
		requestedTextureReduction = getRecommendedTextureReduction();
	}

	if (TheGlobalData)
	{
		TheWritableGlobalData->m_maxParticleCount=lodInfo->m_maxParticleCount;
		TheWritableGlobalData->m_useShadowVolumes=lodInfo->m_useShadowVolumes;
		TheWritableGlobalData->m_useShadowDecals=lodInfo->m_useShadowDecals;

		//Check if texture resolution changed.  No need to apply when current is unknown because display will do it
		if (requestedTextureReduction != m_currentTextureReduction)
		{
				TheWritableGlobalData->m_textureReductionFactor = requestedTextureReduction;
				if (TheGameClient)
					TheGameClient->adjustLOD(0);	//apply the new setting stored in globaldata
		}

		//Check if shadow state changed
		if (m_currentStaticLOD == STATIC_GAME_LOD_UNKNOWN	||
			lodInfo->m_useShadowVolumes != prevLodInfo->m_useShadowVolumes ||
			lodInfo->m_useShadowDecals != prevLodInfo->m_useShadowDecals)
		{
			if (TheGameClient)
			{
				TheGameClient->releaseShadows();	//free all shadows
				TheGameClient->allocateShadows();	//allocate those shadows that are enabled.
			}
		}

		TheWritableGlobalData->m_useCloudMap=lodInfo->m_useCloudMap;
		TheWritableGlobalData->m_useLightMap=lodInfo->m_useLightMap;
		TheWritableGlobalData->m_showSoftWaterEdge=lodInfo->m_showSoftWaterEdge;
		//Check if shoreline blending mode has changed
		if (m_currentStaticLOD == STATIC_GAME_LOD_UNKNOWN || lodInfo->m_showSoftWaterEdge != prevLodInfo->m_showSoftWaterEdge)
		{
			if (TheTerrainVisual)
				TheTerrainVisual->setShoreLineDetail();
		}

		TheWritableGlobalData->m_maxTankTrackEdges=lodInfo->m_maxTankTrackEdges;
		TheWritableGlobalData->m_maxTankTrackOpaqueEdges=lodInfo->m_maxTankTrackOpaqueEdges;
		TheWritableGlobalData->m_maxTankTrackFadeDelay=lodInfo->m_maxTankTrackFadeDelay;
		TheWritableGlobalData->m_useTreeSway=lodInfo->m_useTreeSway;
		TheWritableGlobalData->m_useDrawModuleLOD=!lodInfo->m_useBuildupScaffolds;
		TheWritableGlobalData->m_useHeatEffects=lodInfo->m_useHeatEffects;
		TheWritableGlobalData->m_enableDynamicLOD = lodInfo->m_enableDynamicLOD;
		TheWritableGlobalData->m_useFpsLimit = lodInfo->m_useFpsLimit;
		TheWritableGlobalData->m_useTrees = requestedTrees;

		if (!m_memPassed || isReallyLowMHz()) {
			TheWritableGlobalData->m_shellMapOn = false;
		}
	}
	if (TheTerrainVisual)
		TheTerrainVisual->setTerrainTracksDetail();

}

/**Parse a description of all the LOD settings for a given detail level*/
void INI::parseDynamicGameLODDefinition( INI* ini )
{
	const char *c;
	AsciiString name;

	// read the name
	c = ini->getNextToken();
	name.set( c );

	if( TheGameLODManager )
	{
		Int index = TheGameLODManager->getDynamicGameLODIndex(name);
		if (index != DYNAMIC_GAME_LOD_UNKNOWN)
		{
			DynamicGameLODInfo *lodInfo = &(TheGameLODManager->m_dynamicGameLODInfo[index]);

			// parse the ini weapon definition
			ini->initFromINI( lodInfo, TheDynamicGameLODFieldParseTable );
		}
	}
}

/**Parse an LOD level*/
void INI::parseDynamicGameLODLevel( INI* ini, void * , void *store, const void*)
{
	const char *tok=ini->getNextToken();
	for (Int i=0; i<DYNAMIC_GAME_LOD_COUNT; i++)
		if( stricmp(tok, DynamicGameLODNames[i]) == 0 )
		{	*(DynamicGameLODLevel*)store = (DynamicGameLODLevel)i;
			return;
		}

	DEBUG_CRASH(("invalid GameLODLevel token %s -- expected LOW/MEDIUM/HIGH",tok));
	throw INI_INVALID_DATA;
}

/**Convert LOD name to an index*/
Int GameLODManager::getDynamicGameLODIndex(AsciiString name)
{
	for (Int i=0; i<DYNAMIC_GAME_LOD_COUNT; ++i)
	{
		if (name.compareNoCase(DynamicGameLODNames[i]) == 0)
			return i;
	}

	DEBUG_CRASH(( "GameLODManager::getGameLODIndex - Invalid LOD name '%s'", name.str() ));
	return STATIC_GAME_LOD_UNKNOWN;
}

const char *GameLODManager::getDynamicGameLODLevelName(DynamicGameLODLevel level)
{
	return DynamicGameLODNames[level];
}

/**Given an average fps, return the optimal dynamic LOD level that matches this fps.*/
DynamicGameLODLevel GameLODManager::findDynamicLODLevel(Real averageFPS)
{
	Int ifps=(Int)(averageFPS);	//convert to integer.

	for (Int i=DYNAMIC_GAME_LOD_VERY_HIGH; i>=DYNAMIC_GAME_LOD_LOW; i--)
	{	//check which of the LOD levels matches our fps
		if (m_dynamicGameLODInfo[i].m_minFPS < ifps)
			return (DynamicGameLODLevel)i;
	}
	return DYNAMIC_GAME_LOD_LOW;	//none of the low levels were slow enough so pick the lowest.
}

/**Set all game systems to match the desired LOD level.*/
Bool GameLODManager::setDynamicLODLevel(DynamicGameLODLevel level)
{
	if (level == DYNAMIC_GAME_LOD_UNKNOWN || m_currentDynamicLOD == level)
		return FALSE;

	m_currentDynamicLOD = level;

	applyDynamicLODLevel(level);

	return TRUE;
}

void GameLODManager::applyDynamicLODLevel(DynamicGameLODLevel level)
{
	m_numParticleGenerations=0;
	m_dynamicParticleSkipMask=m_dynamicGameLODInfo[level].m_dynamicParticleSkipMask;

	m_numDebrisGenerations=0;
	m_dynamicDebrisSkipMask=m_dynamicGameLODInfo[level].m_dynamicDebrisSkipMask;

	m_slowDeathScale=m_dynamicGameLODInfo[level].m_slowDeathScale;
	m_minDynamicParticlePriority=m_dynamicGameLODInfo[level].m_minDynamicParticlePriority;
	m_minDynamicParticleSkipPriority=m_dynamicGameLODInfo[level].m_minDynamicParticleSkipPriority;
}

Int GameLODManager::getRecommendedTextureReduction(void)
{
	if (m_idealDetailLevel == STATIC_GAME_LOD_UNKNOWN)
		findStaticLODLevel();	//it was never tested, so test now.

	if (!m_memPassed)	//if they have < 256 MB, force them to low res textures.
		return m_staticGameLODInfo[STATIC_GAME_LOD_LOW].m_textureReduction;

	if (m_idealDetailLevel < 0 || m_idealDetailLevel >= STATIC_GAME_LOD_COUNT)
		return m_staticGameLODInfo[STATIC_GAME_LOD_LOW].m_textureReduction;

	return m_staticGameLODInfo[m_idealDetailLevel].m_textureReduction;
}

Int GameLODManager::getLevelTextureReduction(StaticGameLODLevel level)
{
	return m_staticGameLODInfo[level].m_textureReduction;
}

Bool GameLODManager::didMemPass( void )
{
	return m_memPassed;
}

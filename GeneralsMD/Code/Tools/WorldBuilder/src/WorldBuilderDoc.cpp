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

// WorldBuilderDoc.cpp : implementation of the CWorldBuilderDoc class
//

#include "StdAfx.h"
#include "WorldBuilder.h"

#include <direct.h>
#include <windows.h>
#include <process.h>

#include "Common/Debug.h"
#include "Common/DataChunk.h"
#include "Common/PlayerTemplate.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/WellKnownKeys.h"

#include "GameClient/Line2D.h"
#include "GameClient/View.h"
#include "GameClient/GameText.h"

#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/ScriptEngine.h"


#include "Compression.h"
#include "CUndoable.h"
#include "LayersList.h"
#include "MainFrm.h"
#include "NewHeightMap.h"
#include "SaveMap.h"
#include "ScriptDialog.h"
#include "TerrainMaterial.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "wbview3d.h"
#include "wbview.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "MapPreview.h"


// Can't currently have multiple open... jba.
#define notONLY_ONE_AT_A_TIME

#ifdef ONLY_ONE_AT_A_TIME
static Bool gAlreadyOpen = false;
#endif

enum DIRECTION
{
	PREFER_CENTER,
	PREFER_LEFT,
	PREFER_TOP,
	PREFER_RIGHT,
	PREFER_BOTTOM,
};

static bool secondGreaterThan(const std::pair<AsciiString, Int>& __t1, const std::pair<AsciiString, Int>& __t2)
{
	return __t1.second > __t2.second;
}

static void FindIndexNearest(CWorldBuilderDoc* pDoc, const Coord3D* point, CPoint* outNdx, DIRECTION pref );
static Bool IndexInRect(CWorldBuilderDoc* pDoc, const Coord3D* bl, const Coord3D* tl, const Coord3D* br, const Coord3D* tr, CPoint* index);
static Bool AddUniqueAndNeighbors(CWorldBuilderDoc* pDoc, const Coord3D* bl, const Coord3D* tl, const Coord3D* br, const Coord3D* tr, CPoint ndx, VecHeightMapIndexes* allIndices);
/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderDoc

IMPLEMENT_DYNCREATE(CWorldBuilderDoc, CDocument)

BEGIN_MESSAGE_MAP(CWorldBuilderDoc, CDocument)
	//{{AFX_MSG_MAP(CWorldBuilderDoc)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_TS_INFO, OnTsInfo)
	ON_COMMAND(ID_TS_CANONICAL, OnTsCanonical)
	ON_UPDATE_COMMAND_UI(ID_TS_CANONICAL, OnUpdateTsCanonical)
	ON_COMMAND(ID_FILE_RESIZE, OnFileResize)
	ON_COMMAND(ID_FILE_JUMPTOGAME, OnJumpToGame)
	ON_COMMAND(ID_TS_REMAP, OnTsRemap)
	ON_COMMAND(ID_EDIT_LINK_CENTERS, OnEditLinkCenters)
	ON_UPDATE_COMMAND_UI(ID_EDIT_LINK_CENTERS, OnUpdateEditLinkCenters)
	ON_COMMAND(ID_VIEW_TIME_OF_DAY, OnViewTimeOfDay)
	ON_COMMAND(ID_WINDOW_2DWINDOW, OnWindow2dwindow)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_2DWINDOW, OnUpdateWindow2dwindow)
	ON_COMMAND(ID_VIEW_RELOADTEXTURES, OnViewReloadtextures)
	ON_COMMAND(ID_EDIT_SCRIPTS, OnEditScripts)
	ON_COMMAND(ID_SCRIPT_EDIT, OnEditScripts)
	ON_COMMAND(ID_VIEWHOME, OnViewHome)
	ON_COMMAND(ID_TEXTURESIZING_TILE4X4, OnTexturesizingTile4x4)
	ON_UPDATE_COMMAND_UI(ID_TEXTURESIZING_TILE4X4, OnUpdateTexturesizingTile4x4)
	ON_COMMAND(ID_TEXTURESIZING_TILE6X6, OnTexturesizingTile6x6)
	ON_UPDATE_COMMAND_UI(ID_TEXTURESIZING_TILE6X6, OnUpdateTexturesizingTile6x6)
	ON_COMMAND(ID_TEXTURESIZING_TILE8X8, OnTexturesizingTile8x8)
	ON_UPDATE_COMMAND_UI(ID_TEXTURESIZING_TILE8X8, OnUpdateTexturesizingTile8x8)
	ON_COMMAND(ID_FILE_DUMPTOFILE, OnDumpDocToText)
	ON_COMMAND(ID_TEXTURESIZING_REMOVECLIFFTEXMAPPING, OnRemoveclifftexmapping)
	ON_COMMAND(ID_TOGGLE_PITCH_AND_ROTATE, OnTogglePitchAndRotation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderDoc construction/destruction

CWorldBuilderDoc::CWorldBuilderDoc() :
	m_heightMap(nullptr),
	m_undoList(nullptr),
	m_maxUndos(MAX_UNDOS),		/// @todo: get from pref?
	m_curRedo(0),
	m_needAutosave(false),
	m_curWaypointID(0),
	m_numWaypointLinks(0),
	m_waypointTableNeedsUpdate(true),
	m_linkCenters(true)
{
}

CWorldBuilderDoc::~CWorldBuilderDoc()
{
#ifdef ONLY_ONE_AT_A_TIME
	if (m_heightMap != nullptr ) {
		gAlreadyOpen = false;
	}
#endif
	REF_PTR_RELEASE(m_heightMap);
	REF_PTR_RELEASE(m_undoList);
}


/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderDoc serialization

class MFCFileOutputStream : public OutputStream
{
protected:
	CFile *m_file;
public:
	MFCFileOutputStream(CFile *pFile):m_file(pFile) {};
	virtual Int write(const void *pData, Int numBytes) {
		Int numBytesWritten = 0;
		try {
			m_file->Write(pData, numBytes);
			numBytesWritten = numBytes;
		} catch(...) {}
		return(numBytesWritten);
	};
};

struct CachedChunk
{
	UnsignedByte *pData;
	Int size;
};

class CachedMFCFileOutputStream : public OutputStream
{
protected:
	CFile *m_file;
	std::list<CachedChunk> m_cachedChunks;
	Int m_totalBytes;
public:
	CachedMFCFileOutputStream(CFile *pFile):m_file(pFile), m_totalBytes(0) {};
	virtual Int write(const void *pData, Int numBytes) {
		UnsignedByte *tmp = new UnsignedByte[numBytes];
		memcpy(tmp, pData, numBytes);
		CachedChunk c;
		c.pData = tmp;
		c.size = numBytes;
		m_cachedChunks.push_back(c);
		DEBUG_LOG(("Caching %d bytes in chunk %d", numBytes, m_cachedChunks.size()));
		m_totalBytes += numBytes;
		return(numBytes);
	};
	virtual void flush(void) {
		while (!m_cachedChunks.empty())//!m_cachedChunks.empty())
		{
			CachedChunk c = m_cachedChunks.front();
			m_cachedChunks.pop_front();
			try {
				DEBUG_LOG(("Flushing %d bytes", c.size));
				m_file->Write(c.pData, c.size);
			} catch(...) {}
			delete[] c.pData;
			m_totalBytes -= c.size;
		}
	}
};

class CompressedCachedMFCFileOutputStream : public OutputStream
{
protected:
	CFile *m_file;
	std::list<CachedChunk> m_cachedChunks;
	Int m_totalBytes;
public:
	CompressedCachedMFCFileOutputStream(CFile *pFile):m_file(pFile), m_totalBytes(0) {};
	virtual Int write(const void *pData, Int numBytes) {
		UnsignedByte *tmp = new UnsignedByte[numBytes];
		memcpy(tmp, pData, numBytes);
		CachedChunk c;
		c.pData = tmp;
		c.size = numBytes;
		m_cachedChunks.push_back(c);
		//DEBUG_LOG(("Caching %d bytes in chunk %d", numBytes, m_cachedChunks.size()));
		m_totalBytes += numBytes;
		return(numBytes);
	};
	virtual void flush(void) {
		if (!m_totalBytes)
			return;
		UnsignedByte *srcBuffer = NEW UnsignedByte[m_totalBytes];
		UnsignedByte *insertPos = srcBuffer;
		while (!m_cachedChunks.empty())
		{
			CachedChunk c = m_cachedChunks.front();
			m_cachedChunks.pop_front();
			try {
				//DEBUG_LOG(("Flushing %d bytes", c.size));
				memcpy(insertPos, c.pData, c.size);
				insertPos += c.size;
			} catch(...) {}
			delete[] c.pData;
		}
		CompressionType compressionToUse = CompressionManager::getPreferredCompression();
		Dict *worldDict = MapObject::getWorldDict();
		if (worldDict)
		{
			Bool exists = FALSE;
			compressionToUse = (CompressionType)worldDict->getInt(TheKey_compression, &exists);
			if (!exists || compressionToUse > COMPRESSION_MAX || compressionToUse < COMPRESSION_MIN)
				compressionToUse = CompressionManager::getPreferredCompression();
		}

		Int compressedLen = CompressionManager::getMaxCompressedSize( m_totalBytes, compressionToUse );
		UnsignedByte *destBuffer = NEW UnsignedByte[compressedLen];
		compressedLen = CompressionManager::compressData( compressionToUse, srcBuffer, m_totalBytes, destBuffer, compressedLen );
		DEBUG_LOG(("Compressed %d bytes to %d bytes - compression of %g%%", m_totalBytes, compressedLen,
			compressedLen/(Real)m_totalBytes*100.0f));
		DEBUG_ASSERTCRASH(compressedLen, ("Failed to compress!"));
		if (compressedLen)
		{
			m_file->Write(destBuffer, compressedLen);
		}
		else
		{
			m_file->Write(srcBuffer, m_totalBytes);
		}
		delete[] srcBuffer;
		srcBuffer = nullptr;
		delete[] destBuffer;
		destBuffer = nullptr;
	}
};


void CWorldBuilderDoc::Serialize(CArchive& ar)
{
	ar.Flush();
	m_waypointTableNeedsUpdate = true;
	if (ar.IsStoring() && m_heightMap)
	{
		try {
			Int i;
			MapPreview mPreview;
			mPreview.save(ar.GetFile()->GetFilePath());

			CompressedCachedMFCFileOutputStream theStream(ar.GetFile());
			DataChunkOutput *chunkWriter = new DataChunkOutput(&theStream);


			m_heightMap->saveToFile(*chunkWriter);
 			/***************WAYPOINTS DATA ***************/
			chunkWriter->openDataChunk("WaypointsList", 	K_WAYPOINTS_VERSION_1);
			chunkWriter->writeInt(this->m_numWaypointLinks);
			for (i=0; i<m_numWaypointLinks; i++) {
				chunkWriter->writeInt(this->m_waypointLinks[i].waypoint1);
				chunkWriter->writeInt(this->m_waypointLinks[i].waypoint2);
			}
			chunkWriter->closeDataChunk();

			delete chunkWriter;
			chunkWriter = nullptr;
			theStream.flush();
		} catch(...) {
			const char *msg = "WorldHeightMapEdit::WorldHeightMapEdit  height map file write failed: ";
			AfxMessageBox(msg);
			return;
		}
	}
	else
	{
		WorldHeightMapEdit *pOldHeightMap = m_heightMap;
		CString pth = ar.GetFile()->GetFilePath();
		CachedFileInputStream theInputStream;
		if (theInputStream.open(AsciiString((const char *)pth)))
		try {

			WbApp()->selectPointerTool();
			PolygonTrigger::deleteTriggers();
			ChunkInputStream *pStrm = &theInputStream;

			// Read the logical data (map objects, waypoints, etc.)
			WorldHeightMap *terrainHeightMap = new WorldHeightMap(pStrm, true);
			REF_PTR_RELEASE(terrainHeightMap);
			pStrm->absoluteSeek(0);
			// Read & keep the graphical data.
			m_heightMap = NEW_REF(WorldHeightMapEdit, (pStrm));
			pStrm->absoluteSeek(0);
			try {
				DataChunkInput file( pStrm );
				if (file.isValidFileType()) {	// Backwards compatible files aren't valid data chunk files.
					// Read the waypoints.
					file.registerParser( "WaypointsList", AsciiString::TheEmptyString, ParseWaypointDataChunk );
					if (!file.parse(this)) {
						throw(ERROR_CORRUPT_FILE_FORMAT);
					}
				}
			} catch(...) {
				// just eat the error - legacy files aren't chunk format.
			}
			theInputStream.close();

			validate();

			compressWaypointIds(); // remove any unused waypoint ids.
			WbView3d * p3View = Get3DView();
			if (p3View) {
				p3View->resetRenderObjects();
			}
			m_heightMap->optimizeTiles(); // force to optimize tileset
			SetHeightMap(m_heightMap, true);
			Coord3D center;
			center.x = MAP_XY_FACTOR*m_heightMap->getXExtent()/2;
			center.y = MAP_XY_FACTOR*m_heightMap->getYExtent()/2;
			center.x -= m_heightMap->getBorderSize();
			center.y -= m_heightMap->getBorderSize();
			/* update objects. */
			AsciiString startingCamName = TheNameKeyGenerator->keyToName(TheKey_InitialCameraPosition);

			TheLayersList->resetLayers();
			AsciiString layerName;
			Bool exists;

			// always assign unique IDs. The things will still live in the correct layers, so this isn't
			// an especially big deal.
			MapObject::fastAssignAllUniqueIDs();

			TheLayersList->disableUpdates();
			MapObject *pMapObj = MapObject::getFirstMapObject();
			while (pMapObj) {

				// Then, add it to the Layers List
				layerName = pMapObj->getProperties()->getAsciiString(TheKey_objectLayer, &exists);
				if (exists) {
					TheLayersList->addMapObjectToLayersList(pMapObj, layerName);
				} else {
					TheLayersList->addMapObjectToLayersList(pMapObj);
				}

				MapObject *pTemplateObj = 	ObjectOptions::getObjectNamed(pMapObj->getName());
				if (pTemplateObj) {
					pMapObj->setColor(pTemplateObj->getColor());
				}
				if (pMapObj->isWaypoint()) {
					if (pMapObj->getWaypointID() >= m_curWaypointID) {
						m_curWaypointID = pMapObj->getWaypointID();
					}
					if (startingCamName == pMapObj->getWaypointName()) {
						center = *pMapObj->getLocation();
					}
				}
				pMapObj = pMapObj->getNext();
			}

			PolygonTrigger* polyTrigger = PolygonTrigger::getFirstPolygonTrigger();
			// Add the triggers to the layers list.
			while (polyTrigger) {
				layerName = polyTrigger->getLayerName();
				TheLayersList->addPolygonTriggerToLayersList(polyTrigger, layerName);

				polyTrigger = polyTrigger->getNext();
			}

			TheLayersList->enableUpdates();

			TerrainMaterial::updateTextures(m_heightMap);

			REF_PTR_RELEASE(m_undoList);
			m_curRedo = 0;
			POSITION pos = GetFirstViewPosition();
			while (pos != nullptr)
			{
				CView* pView = GetNextView(pos);
				WbView* pWView = (WbView *)pView;
				ASSERT_VALID(pWView);
				pWView->setCenterInView(center.x/MAP_XY_FACTOR, center.y/MAP_XY_FACTOR);
			}
			REF_PTR_RELEASE(pOldHeightMap);
			if (p3View) {
				p3View->setDefaultCamera();
			}

		} catch(...) {
			m_heightMap = pOldHeightMap;
		}

		// note - mHeight map has ref count of 1.
	}
}

AsciiString ConvertToNonGCName(AsciiString name, Bool checkTemplate=true)
{
	const char* replacePrefix = "GC_";
	const size_t offset = name.startsWith(replacePrefix) ? strlen(replacePrefix) : 0u;
	char newName[256];
	strlcpy(newName, name.str() + offset, ARRAY_SIZE(newName));
	AsciiString swapName;
	swapName.set(newName);
	if (checkTemplate)
	{
		const ThingTemplate *tt = TheThingFactory->findTemplate(swapName);
		if (tt) {
			return swapName;
		}
		return AsciiString::TheEmptyString;
	}
	return swapName;
}

AsciiString ConvertName(AsciiString name)
{
	const char* replacePrefix = "Fundamentalist";
	const size_t offset = name.startsWith(replacePrefix) ? strlen(replacePrefix) : 0u;
	char newName[256];
	strcpy(newName, "GLA");
	strlcat(newName, name.str() + offset, ARRAY_SIZE(newName));
	AsciiString swapName;
	swapName.set(newName);
	const ThingTemplate *tt = TheThingFactory->findTemplate(swapName);
	if (tt) {
		return swapName;
	}
	return AsciiString::TheEmptyString;
}

AsciiString ConvertFaction(AsciiString name)
{
	const char* replacePrefix = "FactionFundamentalist";
	const size_t offset = name.startsWith(replacePrefix) ? strlen(replacePrefix) : 0u;
	char newName[256];
	strcpy(newName, "FactionGLA");
	strlcat(newName, name.str() + offset, ARRAY_SIZE(newName));
	AsciiString swapName;
	swapName.set(newName);
	const PlayerTemplate* pt = ThePlayerTemplateStore->findPlayerTemplate(NAMEKEY(swapName));
	if (pt) {
		return swapName;
	}
	return AsciiString::TheEmptyString;
}

void CWorldBuilderDoc::validate(void)
{
	DEBUG_LOG(("Validating"));

	Dict swapDict;
	Bool changed = false;
	AsciiString swapName;

	Bool needToFixTeams = false;

	// verify/fix the build lists
	for (int side=0; side<TheSidesList->getNumSides(); side++) {
		SidesInfo *pSide = TheSidesList->getSideInfo(side);

		AsciiString tmplname = pSide->getDict()->getAsciiString(TheKey_playerFaction);
		AsciiString playername = pSide->getDict()->getAsciiString(TheKey_playerName);
		if (tmplname.isEmpty()) {
			continue; // Neutral player has empty template. jba. [8/8/2003]
		}
		const PlayerTemplate* pt = ThePlayerTemplateStore->findPlayerTemplate(NAMEKEY(tmplname));
		if (!pt) {
			DEBUG_LOG(("Player '%s' Faction '%s' could not be found in sides list!", playername.str(), tmplname.str()));
			if (tmplname.startsWith("FactionFundamentalist")) {
				swapName = ConvertFaction(tmplname);
				if (swapName != AsciiString::TheEmptyString) {
					DEBUG_LOG(("Changing Faction from %s to %s", tmplname.str(), swapName.str()));
					pSide->getDict()->setAsciiString(TheKey_playerFaction, swapName);
				}
			}
		}

		BuildListInfo *pBuild = pSide->getBuildList();
		while (pBuild) {
			AsciiString name = pBuild->getTemplateName();
			if (name.startsWith("Fundamentalist")) {
				swapName = ConvertName(name);
				if (swapName != AsciiString::TheEmptyString) {
					DEBUG_LOG(("Changing BuildList from %s to %s", name.str(), swapName.str()));
					pBuild->setTemplateName(swapName);
				}
			}
			pBuild = pBuild->getNext();
		}
	}


#define FIX_TEAM(key)																	\
	type = teamDict->getAsciiString(key, &exists);			\
	if (exists) {																				\
		if (type.startsWith("Fundamentalist")) {					\
			swapName = ConvertName(type);										\
			if (swapName != AsciiString::TheEmptyString) {	\
				DEBUG_LOG(("Changing Team Ref from %s to %s", type.str(), swapName.str())); \
				teamDict->setAsciiString(key, swapName);			\
			}																								\
		}																									\
	}																										\

	// verify/fix the team definitions
	Int numTeams = TheSidesList->getNumTeams();
	for (Int team=0; team<numTeams; team++)
	{
		TeamsInfo *ti = TheSidesList->getTeamInfo(team);
		Dict* teamDict = ti->getDict();
		AsciiString type;
		Bool exists;
		FIX_TEAM(TheKey_teamUnitType1)
		FIX_TEAM(TheKey_teamUnitType2)
		FIX_TEAM(TheKey_teamUnitType3)
		FIX_TEAM(TheKey_teamUnitType4)
		FIX_TEAM(TheKey_teamUnitType5)
		FIX_TEAM(TheKey_teamUnitType6)
		FIX_TEAM(TheKey_teamUnitType7)
	}

	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext())
	{
		// there is no validation code for these items as of yet.
		if (pMapObj->isScorch() || pMapObj->isWaypoint() || pMapObj->isLight() || pMapObj->getFlag(FLAG_ROAD_FLAGS) || pMapObj->getFlag(FLAG_BRIDGE_FLAGS))
		{
			continue;
		}

		// at this point, only objects with models and teams should be left to process

		// start by verifying the ThingTemplate for the object.
		// swapDict contains a 'history' of missing model swaps done this load, so all objects with a
		// particular name are replaced with the exact same model.
		AsciiString name = pMapObj->getName();
		if (pMapObj->getThingTemplate() == nullptr)
		{
			Bool exists = false;
			swapName = swapDict.getAsciiString(NAMEKEY(name), &exists);

			// quick hack to make loading models with "Fundamentalist" switch to "GLA"
			if (name.startsWith("Fundamentalist")) {
				swapName = ConvertName(name);
				if (swapName != AsciiString::TheEmptyString) {
					swapDict.setAsciiString(NAMEKEY(name), swapName);
					exists = true;
				}
			}

			// quick hack to remove "GC_" objects from Generals mission disk maps.
			if (name.startsWith("GC_")) {
				swapName = ConvertToNonGCName(name);
				if (swapName != AsciiString::TheEmptyString) {
					swapDict.setAsciiString(NAMEKEY(name), swapName);
					exists = true;
				}
			}

			if (!exists) {
				ReplaceUnitDialog dlg;
				dlg.setMissing(name);
				for (int i = ES_FIRST; i<ES_NUM_SORTING_TYPES; i++)	{
					dlg.SetAllowableType((EditorSortingType)i);
				}
				dlg.SetFactionOnly(false);
				if (dlg.DoModal() == IDOK) {
					const ThingTemplate* thing = dlg.getPickedThing();
					if (thing) {
						swapName = thing->getName();
						swapDict.setAsciiString(NAMEKEY(name), swapName);
					}
				}
			}
			swapName = swapDict.getAsciiString(NAMEKEY(name), &exists);
			if (exists)
			{
				const ThingTemplate *tt = TheThingFactory->findTemplate(swapName);
				if (tt) {
					changed = true;
					pMapObj->setName(swapName);
					pMapObj->setThingTemplate(tt);
					DEBUG_LOG(("Changing Map Object from %s to %s", name.str(), swapName.str()));
				}
			}
		}


		// the following code verifies and fixes the team name, player name, and faction linkages
		Bool exists;
		AsciiString teamName = pMapObj->getProperties()->getAsciiString(TheKey_originalOwner, &exists);
		if (exists) {
			TeamsInfo *teamInfo = TheSidesList->findTeamInfo(teamName);
			if (teamInfo) {
				AsciiString teamOwner = teamInfo->getDict()->getAsciiString(TheKey_teamOwner);
				SidesInfo* pSide = TheSidesList->findSideInfo(teamOwner);
				if (pSide) {
//					Bool hasColor = false;
					AsciiString tmplname = pSide->getDict()->getAsciiString(TheKey_playerFaction);
					AsciiString playername = pSide->getDict()->getAsciiString(TheKey_playerName);
					if (tmplname.isEmpty()) {
						continue; // Neutral player has empty template. jba. [8/8/2003]
					}
					const PlayerTemplate* pt = ThePlayerTemplateStore->findPlayerTemplate(NAMEKEY(tmplname));
					if (!pt) {
						DEBUG_LOG(("Player '%s' Faction '%s' could not be found in sides list!", playername.str(), tmplname.str()));
						if (tmplname.startsWith("FactionFundamentalist")) {
							swapName = ConvertFaction(tmplname);
							if (swapName != AsciiString::TheEmptyString) {
								DEBUG_LOG(("Changing Faction from %s to %s", tmplname.str(), swapName.str()));
								pSide->getDict()->setAsciiString(TheKey_playerFaction, swapName);
							}
						}
					}
				} else {
					needToFixTeams = true;
					DEBUG_LOG(("Side '%s' could not be found in sides list!", teamOwner.str()));
				}
			} else {
				needToFixTeams = true;
				DEBUG_LOG(("Team '%s' could not be found in sides list!", teamName.str()));
			}
		} else {
			needToFixTeams = true;
			DEBUG_LOG(("Object '%s' does not have a team at all!", name.str()));
		}
	}
	if (needToFixTeams) {
		AfxMessageBox(IDS_NEED_TO_FIX_TEAMS, MB_OK|MB_ICONERROR);
	}
}

void CWorldBuilderDoc::OnJumpToGame()
{
	try {
		DoFileSave();
		CString filename;
		DEBUG_LOG(("strTitle=%s strPathName=%s", m_strTitle, m_strPathName));
		if (strstr(m_strPathName, TheGlobalData->getPath_UserData().str()) != nullptr)
			filename.Format("%sMaps\\%s", TheGlobalData->getPath_UserData().str(), static_cast<const char*>(m_strTitle));
		else
			filename.Format("Maps\\%s", static_cast<const char*>(m_strTitle));

		/*int retval =*/ _spawnl(_P_NOWAIT, "\\projects\\rts\\run\\rtsi.exe", "ignored", "-scriptDebug", "-win", "-file", static_cast<const char*>(filename), nullptr);
	} catch (...) {
	}
}

BOOL CWorldBuilderDoc::DoFileSave()
{
	DWORD dwAttrib = GetFileAttributes(m_strPathName);
	if (dwAttrib & FILE_ATTRIBUTE_READONLY)
	{
		if (dwAttrib != 0xFFFFFFFF) {
			::AfxMessageBox(IDS_FILE_IS_READONLY);
			return FALSE;
		}
		// File does not exist, dwAttrib==0xffffffff
		// we do not have read-write access or the file does not (now) exist
		if (!DoSave(nullptr))
		{
			TRACE0("Warning: File save with new name failed.\n");
			return FALSE;
		}
	}
	else
	{
		if (!DoSave(m_strPathName))
		{
			TRACE0("Warning: File save failed.\n");
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CWorldBuilderDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
	// Save the document data to a file
	// lpszPathName = path name where to save document file
	// if lpszPathName is null then the user will be prompted (SaveAs)
	// note: lpszPathName can be different than 'm_strPathName'
	// if 'bReplace' is TRUE will change file name if successful (SaveAs)
	// if 'bReplace' is FALSE will not change path name (SaveCopyAs)
{
	CString newName = lpszPathName;
	if (newName.IsEmpty())
	{
		CDocTemplate* pTemplate = GetDocTemplate();
		ASSERT(pTemplate != nullptr);

		newName = m_strPathName;
		if (bReplace && newName.IsEmpty())
		{
			newName = m_strTitle;
			// check for dubious filename
			int iBad = newName.FindOneOf(_T(" #%;/\\"));
			if (iBad != -1)
				newName.ReleaseBuffer(iBad);

			// append the default suffix if there is one
			CString strExt;
			if (pTemplate->GetDocString(strExt, CDocTemplate::filterExt) &&
			  !strExt.IsEmpty())
			{
				ASSERT(strExt[0] == '.');
				newName += strExt;
			}
		}

		TSaveMapInfo info;
		info.filename = newName;
		SaveMap saveDlg(&info);
		if (saveDlg.DoModal() == IDCANCEL) {
			return FALSE;
		}
		if (info.browse) {
			if (!AfxGetApp()->DoPromptFileName(newName,
				bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
				OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, pTemplate))
				return FALSE;       // don't even attempt to save
		} else {
			// Construct file name of .\Maps\mapname\mapname.map
			if (info.usingSystemDir)
				newName = ".\\Maps\\";
			else
			{
				newName = TheGlobalData->getPath_UserData().str();
				newName = newName + "Maps\\";
			}
			newName += info.filename;
			// Create directory.
			CFileStatus status;
			if (CFile::GetStatus(newName, status)) {
				if (!(status.m_attribute&CFile::directory)) {
					CString error = "Error: file '" + newName + "' exists, and is not a directory.";
					::AfxMessageBox(error);
					return FALSE;
				}
			} else {
				Int status = ::_mkdir(newName);
				if (status != 0) {
					CString error = "Error: could not create directory '" + newName + "'.";
					::AfxMessageBox(error);
					return FALSE;
				}
			}
			newName += "\\";
			newName += info.filename;
			newName += ".map";
		}
	}

	CWaitCursor wait;

	if (!OnSaveDocument(newName))
	{
		if (lpszPathName == nullptr)
		{
			// be sure to delete the file
			try
			{
				CFile::Remove(newName);
			}
			catch(...)
			{
				TRACE0("Warning: failed to delete file after failed SaveAs.\n");
			}
		}
		return FALSE;
	}

	// reset the title and change the document name
	if (bReplace)
		SetPathName(newName);

	return TRUE;        // success
}


/**
* CWorldBuilderDoc::ParseWaypointDataChunk - read a waypoint chunk.
* Format is the newer CHUNKY format.
*	See WHeightMapEdit.cpp for the writer.
*	Input: DataChunkInput
*
*/
Bool CWorldBuilderDoc::ParseWaypointDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	CWorldBuilderDoc *pThis = (CWorldBuilderDoc *)userData;
	return pThis->ParseWaypointData(file, info, userData);
}

/**
* CWorldBuilderDoc::ParseWaypointData - read waypoint data chunk.
* Format is the newer CHUNKY format.
*	See WorldBuilderDoc.cpp for the writer.
*	Input: DataChunkInput
*
*/
Bool CWorldBuilderDoc::ParseWaypointData(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	m_numWaypointLinks = file.readInt();
	Int i;
	for (i=0; i<m_numWaypointLinks; i++) {
		this->m_waypointLinks[i].waypoint1 = file.readInt();
		this->m_waypointLinks[i].waypoint2 = file.readInt();
		//DEBUG_LOG(("Waypoint link from %d to %d", m_waypointLinks[i].waypoint1, m_waypointLinks[i].waypoint2));
	}
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));
	return true;
}

void CWorldBuilderDoc::autoSave(void)
{
	// srj sez: put autosave into our user data folder, not the ap dir
	AsciiString autosave1 = TheGlobalData->getPath_UserData();
	AsciiString autosave2 = TheGlobalData->getPath_UserData();
	AsciiString autosave3 = TheGlobalData->getPath_UserData();
	autosave1.concat("WorldBuilderAutoSave1.map");
	autosave2.concat("WorldBuilderAutoSave2.map");
	autosave3.concat("WorldBuilderAutoSave3.map");

	if (m_heightMap) try {
		CFileStatus status;
		try {
			if (CFile::GetStatus(autosave3.str(), status)) {
				CFile::Remove(autosave3.str());
			}
		} catch(...) {}
		try {
			if (CFile::GetStatus(autosave2.str(), status)) {
				CFile::Rename(autosave2.str(), autosave3.str());
			}
		} catch(...){}
		try {
			if (CFile::GetStatus(autosave1.str(), status)) {
				CFile::Rename(autosave1.str(), autosave2.str());
			}
		} catch(...){}

		CFile theFile(autosave1.str(), CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite|CFile::typeBinary);
		try {
			Int i;
			MFCFileOutputStream theStream(&theFile);
			DataChunkOutput chunkWriter(&theStream);

			m_heightMap->saveToFile(chunkWriter);
 			/***************WAYPOINTS DATA ***************/
			chunkWriter.openDataChunk("WaypointsList", 	K_WAYPOINTS_VERSION_1);
			chunkWriter.writeInt(this->m_numWaypointLinks);
			for (i=0; i<m_numWaypointLinks; i++) {
				chunkWriter.writeInt(this->m_waypointLinks[i].waypoint1);
				chunkWriter.writeInt(this->m_waypointLinks[i].waypoint2);
			}
			chunkWriter.closeDataChunk();

		} catch(...) {
		}
		theFile.Close();
		m_needAutosave = false;
	}	catch(...) {
		::AfxMessageBox(IDS_NO_AUTOSAVE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderDoc diagnostics

#ifdef RTS_DEBUG
void CWorldBuilderDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWorldBuilderDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //RTS_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderDoc commands

void CWorldBuilderDoc::SetHeightMap(WorldHeightMapEdit *pMap, Bool doUpdate)
{
	REF_PTR_SET(m_heightMap, pMap);
	if (doUpdate) {
		POSITION pos = GetFirstViewPosition();
		while (pos != nullptr)
		{
			CView* pView = GetNextView(pos);
			WbView* pWView = (WbView *)pView;
			IRegion2D partialRange = {0,0,0,0};
			ASSERT_VALID(pWView);
			pWView->updateHeightMapInView(m_heightMap, false, partialRange);
			pWView->Invalidate(false);
		}
	}
}

void CWorldBuilderDoc::AddAndDoUndoable(Undoable *pUndo)
{
	Undoable *pCurUndo = m_undoList;
	Int count = m_curRedo;
	while(count>0 && pCurUndo != nullptr) {
		count--;
		pCurUndo = pCurUndo->GetNext();
	}
	m_needAutosave = true;
	m_waypointTableNeedsUpdate=true;
	m_curRedo = 0;
	pUndo->LinkNext(pCurUndo);
	REF_PTR_SET(m_undoList, pUndo);
	pUndo->Do();
	SetModifiedFlag();
	pCurUndo = m_undoList;
	count = 0;
	while (pCurUndo) {
		count++;
		if (count >= MAX_UNDOS) {
			pCurUndo->LinkNext(nullptr);
			break;
		}
		pCurUndo = pCurUndo->GetNext();
	}
}

void CWorldBuilderDoc::OnEditRedo()
{
	Undoable *pUndo = m_undoList;
	m_needAutosave = true;
	m_waypointTableNeedsUpdate=true;
	if (m_curRedo>0) {
		Int count = m_curRedo-1;
		while(count>0) {
			count--;
			pUndo = pUndo->GetNext();
		}
		DEBUG_ASSERTCRASH((pUndo != nullptr),("oops"));
		if (pUndo) {
			pUndo->Redo();
			SetModifiedFlag();
			m_curRedo--;
		}
	}
}

void CWorldBuilderDoc::OnUpdateEditRedo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_undoList!=nullptr && m_curRedo>0);
}

void CWorldBuilderDoc::OnEditUndo()
{
	Undoable *pUndo = m_undoList;
	m_needAutosave = true;
	m_waypointTableNeedsUpdate=true;
	Int count = m_curRedo;
	while(count>0 && pUndo != nullptr) {
		count--;
		pUndo = pUndo->GetNext();
	}
	if (pUndo != nullptr) {
		pUndo->Undo();
		SetModifiedFlag();
		m_curRedo++;
	}
}

void CWorldBuilderDoc::OnTogglePitchAndRotation( void )
{
	WbView3d * p3View = Get3DView();
	if (p3View)
	{
		p3View->togglePitchAndRotation();
	}
}

void CWorldBuilderDoc::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	Bool canUndo=false;
	if (m_undoList!=nullptr) {
		if (m_curRedo == 0) {
			canUndo = true; // haven't undone any yet.
		} else {
			Undoable *pUndo = m_undoList;
			Int count = m_curRedo;
			while(count>0 && pUndo != nullptr) {
				count--;
				pUndo = pUndo->GetNext();
			}
			canUndo = pUndo != nullptr;
		}
	}
	pCmdUI->Enable(canUndo);
}


void CWorldBuilderDoc::OnTsInfo()
{
	if (m_heightMap) {
		m_heightMap->showTileStatusInfo();
	}
}


void CWorldBuilderDoc::OnTsCanonical()
{
	if (m_heightMap) {

		WorldHeightMapEdit *htMapEditCopy = GetHeightMap()->duplicate();
		if (htMapEditCopy == nullptr) return;
		if (htMapEditCopy->optimizeTiles()) {  // does all the work.
			IRegion2D partialRange = {0,0,0,0};
			updateHeightMap(htMapEditCopy, false, partialRange);
			WBDocUndoable *pUndo = new WBDocUndoable(this, htMapEditCopy);
			this->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo); // belongs to this now.
		} else {
			::Beep(1000,500);
		}
		REF_PTR_RELEASE(htMapEditCopy);
	}
}

void CWorldBuilderDoc::OnUpdateTsCanonical(CCmdUI* pCmdUI)
{
}

void CWorldBuilderDoc::OnFileResize()
{
	TNewHeightInfo hi;
	hi.initialHeight = 8;
	hi.xExtent = m_heightMap->getXExtent()-2*m_heightMap->getBorderSize();
	hi.yExtent = m_heightMap->getYExtent()-2*m_heightMap->getBorderSize();
	hi.borderWidth = m_heightMap->getBorderSize();
	hi.forResize = true;
	CString label;
	label.LoadString(IDS_RESIZE);
	CNewHeightMap htDialog(&hi, label);
	if (IDOK == htDialog.DoModal()) {
		htDialog.GetHeightInfo(&hi);
	} else {
		return;
	}

	WorldHeightMapEdit *htMapEditCopy = GetHeightMap()->duplicate();
	if (htMapEditCopy == nullptr) return;
	Coord3D objOffset;
	if (htMapEditCopy->resize(hi.xExtent, hi.yExtent, hi.initialHeight, hi.borderWidth,
		hi.anchorTop, hi.anchorBottom, hi.anchorLeft, hi.anchorRight, &objOffset)) {  // does all the work.
		WBDocUndoable *pUndo = new WBDocUndoable(this, htMapEditCopy, &objOffset);
		this->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to this now.
		POSITION pos = GetFirstViewPosition();
		IRegion2D partialRange = {0,0,0,0};
		Get3DView()->updateHeightMapInView(m_heightMap, false, partialRange);
		while (pos != nullptr)
		{
			CView* pView = GetNextView(pos);
			WbView* pWView = (WbView *)pView;
			ASSERT_VALID(pWView);
			pWView->adjustDocSize();
			pWView->Invalidate();
		}
	} else {
		::Beep(1000,500);
	}
	REF_PTR_RELEASE(htMapEditCopy);

}


void CWorldBuilderDoc::OnTsRemap()
{
	if (m_heightMap) {
		WorldHeightMapEdit *htMapEditCopy = GetHeightMap()->duplicate();
		if (htMapEditCopy == nullptr) return;
		if (htMapEditCopy->remapTextures()) {  // does all the work.
			IRegion2D partialRange = {0,0,0,0};
			updateHeightMap(htMapEditCopy, false, partialRange);
			WBDocUndoable *pUndo = new WBDocUndoable(this, htMapEditCopy);
			this->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo); // belongs to this now.
		} else {
			::Beep(1000,500);
		}
		REF_PTR_RELEASE(htMapEditCopy);
	}
}

/* static */ CWorldBuilderDoc *CWorldBuilderDoc::GetActiveDoc()
{
#ifdef MDI
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	if (pFrame) {
		DEBUG_ASSERTCRASH((pFrame == CMainFrame::GetMainFrame()),("oops"));
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
		if (pChild) {
			// Get the active view attached to the active MDI child
			// window.
			WbView *pView = (WbView *) pChild->GetActiveView();
			if (pView) {
				return pView->WbDoc();
			}
		}
	}

#else
// only works for SDI, not MDI
	return (CWorldBuilderDoc*)CMainFrame::GetMainFrame()->GetActiveDocument();
#endif
	return nullptr;
}

/* static */ CWorldBuilderView *CWorldBuilderDoc::GetActive2DView()
{
	CWorldBuilderDoc* pDoc = GetActiveDoc();
	if (pDoc) {
		return pDoc->Get2DView();
	}
	return nullptr;
}

/* static */ WbView3d *CWorldBuilderDoc::GetActive3DView()
{
	CWorldBuilderDoc* pDoc = GetActiveDoc();
	if (pDoc) {
		return pDoc->Get3DView();
	}
	return nullptr;
}

CWorldBuilderView *CWorldBuilderDoc::Get2DView()
{
	POSITION pos = GetFirstViewPosition();
	while (pos != nullptr)
	{
		CView* pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS(CWorldBuilderView)))
			return (CWorldBuilderView*)pView;
	}

	return nullptr;
}

WbView3d *CWorldBuilderDoc::Get3DView()
{
	POSITION pos = GetFirstViewPosition();
	while (pos != nullptr)
	{
		CView* pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS(WbView3d)))
			return (WbView3d*)pView;
	}

	return nullptr;
}

void CWorldBuilderDoc::Create2DView()
{
}

void CWorldBuilderDoc::Create3DView()
{
	if (Get3DView())
		return;
#ifdef ONLY_ONE_AT_A_TIME
	gAlreadyOpen = true;
#endif
#if 1
	CDocTemplate* pTemplate = WbApp()->Get3dTemplate();
	IRegion2D partialRange = {0,0,0,0};
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame = pTemplate->CreateNewFrame(this, nullptr);
	if (pFrame == nullptr)
	{
		TRACE0("Warning: failed to create new frame.\n");
		return;     // command failed
	}
	pTemplate->InitialUpdateFrame(pFrame, this);
	Get3DView()->updateHeightMapInView(m_heightMap, false, partialRange);
#endif
}


BOOL CWorldBuilderDoc::OnNewDocument()
{
#ifdef ONLY_ONE_AT_A_TIME
	if (gAlreadyOpen) {
		::AfxMessageBox(IDS_ONLY_ONE_FILE);
		return FALSE;
	}
#endif
	if (!CDocument::OnNewDocument())
		return FALSE;
	static Bool firstTime = true;

	// clear out map-specific text
	TheGameText->reset();

	TNewHeightInfo hi;
	hi.initialHeight = AfxGetApp()->GetProfileInt("GameOptions", "Default Map Height", 16);
	hi.xExtent = AfxGetApp()->GetProfileInt("GameOptions", "Default Map X-size", 100);
	hi.yExtent = AfxGetApp()->GetProfileInt("GameOptions", "Default Map Y-size", 100);
	hi.borderWidth = AfxGetApp()->GetProfileInt("GameOptions", "Default Map Border", 30);
	hi.forResize = false;
	if (!firstTime) {
		CString label;
		label.LoadString(IDS_NEW);
		CNewHeightMap htDialog(&hi, label);
		if (IDOK == htDialog.DoModal()) {
			htDialog.GetHeightInfo(&hi);
			AfxGetApp()->WriteProfileInt("GameOptions", "Default Map Height", hi.initialHeight);
			AfxGetApp()->WriteProfileInt("GameOptions", "Default Map X-size", hi.xExtent);
			AfxGetApp()->WriteProfileInt("GameOptions", "Default Map Y-size", hi.yExtent);
			AfxGetApp()->WriteProfileInt("GameOptions", "Default Map Border", hi.borderWidth);
		} else {
			return(false);
		}
	}
	REF_PTR_RELEASE(m_heightMap);
	REF_PTR_RELEASE(m_undoList);
	m_curRedo = 0;
	m_numWaypointLinks = 0;
	m_waypointTableNeedsUpdate = true;
	m_curWaypointID = 0;
	WbApp()->selectPointerTool();

	// Make sure that all the old units are removed from the list.
	// Bug fix by MLL 1/14/03
	TheLayersList->enableUpdates();
	TheLayersList->resetLayers();
	TheLayersList->disableUpdates();

	// TheSuperHackers @bugfix Caball009 20/06/2025 Must not delete polygon triggers before calling enableUpdates.
	PolygonTrigger::deleteTriggers();

	TheSidesList->clear();
	TheSidesList->validateSides();

	WbView3d * p3View = Get3DView();
	if (p3View) {
		p3View->resetRenderObjects();
	}
	firstTime = false;
	m_heightMap = NEW_REF(WorldHeightMapEdit,(hi.xExtent,hi.yExtent,hi.initialHeight, hi.borderWidth));
	// note - mHeight map has ref count of 1.

	// Create a default water area.
	PolygonTrigger *pTrig = newInstance(PolygonTrigger)(4);
	ICoord3D loc;
	pTrig->setWaterArea(true);
	pTrig->setTriggerName("Default Water");
	loc.x = -hi.borderWidth*MAP_XY_FACTOR;
	loc.y = -hi.borderWidth*MAP_XY_FACTOR;
	loc.z = TheGlobalData->m_waterPositionZ;
	pTrig->addPoint(loc);
	loc.x = (hi.xExtent+hi.borderWidth)*MAP_XY_FACTOR;
	pTrig->addPoint(loc);
	loc.y = (hi.yExtent+hi.borderWidth)*MAP_XY_FACTOR;
	pTrig->addPoint(loc);
	loc.x = -hi.borderWidth*MAP_XY_FACTOR;
	pTrig->addPoint(loc);
	PolygonTrigger::addPolygonTrigger(pTrig);
	TheLayersList->addPolygonTriggerToLayersList(pTrig, pTrig->getLayerName());
	SetHeightMap(m_heightMap, true);
	TerrainMaterial::updateTextures(m_heightMap);

	Create3DView();

	POSITION pos = GetFirstViewPosition();
	while (pos != nullptr)
	{
		CView* pView = GetNextView(pos);
		WbView* pWView = (WbView *)pView;
		ASSERT_VALID(pWView);
		pWView->setCenterInView(m_heightMap->getXExtent()/2-m_heightMap->getBorderSize(), m_heightMap->getYExtent()/2-m_heightMap->getBorderSize());
	}
	if (p3View) {
		p3View->setDefaultCamera();
	}
	return TRUE;
}

void CWorldBuilderDoc::invalObject(MapObject *pMapObj)
{
	POSITION pos = GetFirstViewPosition();
	while (pos != nullptr)
	{
		CView* pView = GetNextView(pos);
		WbView* pWView = (WbView *)pView;
		ASSERT_VALID(pWView);
		pWView->invalObjectInView(pMapObj);
	}
}

void CWorldBuilderDoc::invalCell(int xIndex, int yIndex)
{
	POSITION pos = GetFirstViewPosition();
	while (pos != nullptr)
	{
		CView* pView = GetNextView(pos);
		WbView* pWView = (WbView *)pView;
		ASSERT_VALID(pWView);
		pWView->invalidateCellInView(xIndex, yIndex);
	}
}

void CWorldBuilderDoc::syncViewCenters(Real x, Real y)
{
	if (!m_linkCenters)
		return;

	POSITION pos = GetFirstViewPosition();
	while (pos != nullptr)
	{
		CView* pView = GetNextView(pos);
		WbView* pWView = (WbView *)pView;
		ASSERT_VALID(pWView);
		pWView->setCenterInView(x, y);
	}
}


void CWorldBuilderDoc::updateAllViews()
{
	POSITION pos = GetFirstViewPosition();
	while (pos != nullptr)
	{
		CView* pView = GetNextView(pos);
		WbView* pWView = (WbView *)pView;
		ASSERT_VALID(pWView);
		pWView->UpdateWindow();
	}
}

void CWorldBuilderDoc::updateHeightMap(WorldHeightMap *htMap, Bool partial, const IRegion2D &partialRange)
{
	POSITION pos = GetFirstViewPosition();
	while (pos != nullptr)
	{
		CView* pView = GetNextView(pos);
		WbView* pWView = (WbView *)pView;
		ASSERT_VALID(pWView);
		pWView->updateHeightMapInView(htMap, partial, partialRange);
		pWView->Invalidate();
	}
}

BOOL CWorldBuilderDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
#ifdef ONLY_ONE_AT_A_TIME
	if (gAlreadyOpen) {
		::AfxMessageBox(IDS_ONLY_ONE_FILE);
		return FALSE;
	}
#endif

	// Open document dialog may change working directory,
	// let the app know what it was for future opens, and change it back.
	char buf[_MAX_PATH];
	::GetCurrentDirectory(_MAX_PATH, buf);

	// clear out map-specific text
	TheGameText->reset();
	AsciiString s = lpszPathName;
	const char* lastSep = s.reverseFind('\\');
	if (lastSep != nullptr)
	{
		s.truncateTo(lastSep - s.str() + 1);
	}
	s.concat("map.str");
	DEBUG_LOG(("Looking for map-specific text in [%s]", s.str()));
	TheGameText->initMapStringFile(s);

	WbApp()->setCurrentDirectory(AsciiString(buf));
	::GetModuleFileName(nullptr, buf, sizeof(buf));
	if (char *pEnd = strrchr(buf, '\\')) {
		*pEnd = 0;
	}
	::SetCurrentDirectory(buf);

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	Create3DView();

	return TRUE;
}

//=============================================================================
// CWorldBuilderView::getCellIndexFromPoint
//=============================================================================
/** Given a cursor location, return the x and y index into the height map.
If the location is outside the height map, returns false. */
//=============================================================================
Bool CWorldBuilderDoc::getCellIndexFromCoord(Coord3D cpt, CPoint *ndxP)
{
	// Set up default return value.
	ndxP->x = -1;
	ndxP->y = -1;
	Bool inMap = true;

	WorldHeightMapEdit *pMap = GetHeightMap();
	if (pMap == nullptr) return false;

	Int xIndex = floor(cpt.x/MAP_XY_FACTOR);
	xIndex += pMap->getBorderSize();

	// If negative, outside of map so return false.
	if (xIndex<0) {
		inMap = false;
		xIndex = 0;
	}
	// If larger than the map, return default.
	if (xIndex >= pMap->getXExtent()) {
		inMap = false;
		xIndex = pMap->getXExtent()-1;
	}
	Int yIndex = floor(cpt.y/MAP_XY_FACTOR);

	yIndex += pMap->getBorderSize();


	// If negative, outside of map so return default.
	if (yIndex<0) {
		inMap = false;
		yIndex = 0;
	}

	// If larger than the map, return default.
	if (yIndex >= pMap->getYExtent())  {
		inMap = false;
		yIndex = pMap->getYExtent()-1;
	}


	ndxP->x = xIndex;
	ndxP->y = yIndex;

	return inMap;
}

void CWorldBuilderDoc::getCoordFromCellIndex(CPoint ndx, Coord3D* pt)
{
	if (!pt) {
		return;
	}
	WorldHeightMap* hm = GetHeightMap();
	if (!hm) {
		return;
	}

	(*pt).x = (ndx.x - hm->getBorderSize()) * MAP_XY_FACTOR;
	(*pt).y = (ndx.y - hm->getBorderSize()) * MAP_XY_FACTOR;
}

//=============================================================================
// CWorldBuilderView::getAllIndexesInRect
//=============================================================================
//=============================================================================
Bool CWorldBuilderDoc::getAllIndexesInRect(const Coord3D* bl, const Coord3D* br,
																					 const Coord3D* tl, const Coord3D* tr,
																					 Int widthOutside, VecHeightMapIndexes* allIndices)
{
	// given the four corners of this rectangle, find all indices that are within
	// widthOutside of the rect and place them into allIndices.
	if (!(bl && br && tl && tr && allIndices)) {
		return false;
	}

	Coord3D center = { (bl->x + tr->x) / 2, (bl->y + tr->y) / 2, (bl->z + tr->z) / 2 };

	allIndices->clear();

	CPoint ndx;

	FindIndexNearest(this, &center, &ndx, PREFER_CENTER);
	AddUniqueAndNeighbors(this, bl, br, tl, tr, ndx, allIndices);

	FindIndexNearest(this, &center, &ndx, PREFER_LEFT);
	AddUniqueAndNeighbors(this, bl, br, tl, tr, ndx, allIndices);

	FindIndexNearest(this, &center, &ndx, PREFER_TOP);
	AddUniqueAndNeighbors(this, bl, br, tl, tr, ndx, allIndices);

	FindIndexNearest(this, &center, &ndx, PREFER_RIGHT);
	AddUniqueAndNeighbors(this, bl, br, tl, tr, ndx, allIndices);

	FindIndexNearest(this, &center, &ndx, PREFER_BOTTOM);
	AddUniqueAndNeighbors(this, bl, br, tl, tr, ndx, allIndices);

	return (!allIndices->empty());
}


//=============================================================================
// CWorldBuilderView::getCellPositionFromPoint
//=============================================================================
/** Given a pixel position, returns the x/y location in the height map.  This
will return real values, so a position can be 1.7, 2.4 or such.  If the position
is not over the height map, return -1, -1. */
//=============================================================================
Bool CWorldBuilderDoc::getCellPositionFromCoord(Coord3D cpt,  Coord3D *locP)
{
	// Set up default values.
	locP->x = -1;
	locP->y = -1;
	WorldHeightMapEdit *pMap = GetHeightMap();
	if (pMap == nullptr) return(false);
//	yLocation = pMap->getYExtent() - yLocation;
	CPoint curNdx;
	if (getCellIndexFromCoord(cpt, &curNdx)) {
		locP->x = cpt.x;
		locP->y = cpt.y;
		locP->z = pMap->getHeight(curNdx.x, curNdx.y)*MAP_HEIGHT_SCALE;
		return(true);
	}
	return false;
}

//=============================================================================
// CWorldBuilderDoc::getObjArrowPoint
//=============================================================================
/** Gets the location in pixels of the arrowhead point for an object. */
//=============================================================================
void CWorldBuilderDoc::getObjArrowPoint(MapObject *pObj, Coord3D *location)
{
	// Get the center location, and the angle.
	Coord3D loc = *pObj->getLocation();
 	float angle = pObj->getAngle();
	// The arrow starts in the +x direction.
	Vector3 arrow(1.2f*MAP_XY_FACTOR, 0, 0);
	// Rotate
	arrow.Rotate_Z(angle);
	// Rotated.
	location->x = arrow.X;
	location->y = arrow.Y;
	// Add the rotated offset to the center.
	location->x += loc.x;
	location->y += loc.y;
	//location->z += loc.z;
}

void CWorldBuilderDoc::OnEditLinkCenters()
{
	m_linkCenters = !m_linkCenters;
}

void CWorldBuilderDoc::OnUpdateEditLinkCenters(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_linkCenters?1:0);
}

BOOL CWorldBuilderDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	CView *pView = this->Get2DView();
	if (pView && pView->GetParentFrame() == pFrame) {
		return true; // can always close the 2d window.
	}
	return SaveModified();
}

void CWorldBuilderDoc::OnViewTimeOfDay()
{
	WbView3d * pView = Get3DView();
	if (pView) {
		pView->stepTimeOfDay();
	}
}

void CWorldBuilderDoc::OnWindow2dwindow()
{
/*
	CView *pView = this->Get2DView();
	if (pView) {
		CFrameWnd *pFrame = pView->GetParentFrame();
		if (pFrame->IsIconic()) {
			pFrame->ShowWindow(SW_RESTORE);
		} else {
			pFrame->DestroyWindow();
		}
	} else {
		Create2DView();
	}
*/
}

void CWorldBuilderDoc::OnUpdateWindow2dwindow(CCmdUI* pCmdUI)
{
/*
	CView *pView = this->Get2DView();
	pCmdUI->SetCheck(pView?1:0);
*/
}

//=============================================================================
// CWorldBuilderDoc::compressWaypointIds
//=============================================================================
/** Renumbers the waypoints and the links that reference them, removing any
unused ids. */
//=============================================================================
void CWorldBuilderDoc::compressWaypointIds(void)
{
	updateWaypointTable();
	m_curWaypointID = 0;
	MapObject *pMapObj = nullptr;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj->isWaypoint()) {
			Int nwpid = getNextWaypointID();
			pMapObj->setWaypointID(nwpid);
		}
	}
	Int i, j;
	for (i=0; i<m_numWaypointLinks; i++) {
		MapObject *pWay1 = getWaypointByID(m_waypointLinks[i].waypoint1);
		MapObject *pWay2 = getWaypointByID(m_waypointLinks[i].waypoint2);
		if (pWay1 &&	pWay2) {
			m_waypointLinks[i].waypoint1 = pWay1->getWaypointID();
			m_waypointLinks[i].waypoint2 = pWay2->getWaypointID();
		} else {
			// Delete the link.
			for (j=i; j<m_numWaypointLinks-1; j++) {
				m_waypointLinks[j] = m_waypointLinks[j+1];
			}
			m_numWaypointLinks--;
			i--;
		}
	}
	m_waypointTableNeedsUpdate = true;
	updateWaypointTable();
#ifdef DEBUG_CRASHING
	for (i=0; i<m_numWaypointLinks; i++) {
		MapObject *pWay1 = getWaypointByID(m_waypointLinks[i].waypoint1);
		MapObject *pWay2 = getWaypointByID(m_waypointLinks[i].waypoint2);
		DEBUG_ASSERTCRASH(pWay1 && pWay1->getWaypointID() == m_waypointLinks[i].waypoint1, ("Bad waypoint."));
		DEBUG_ASSERTCRASH(pWay2 && pWay2->getWaypointID() == m_waypointLinks[i].waypoint2, ("Bad waypoint."));
	}
	int count = 1;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj->isWaypoint()) {
			DEBUG_ASSERTCRASH(pMapObj->getWaypointID()==count, ("Bad waypoint"));
			DEBUG_ASSERTCRASH(pMapObj==getWaypointByID(count), ("Bad waypoint"));
			count++;
		}
	}
#endif
}

//=============================================================================
// CWorldBuilderDoc::updateWaypointTable
//=============================================================================
/** If any waypoints have changed (m_waypointTableNeedsUpdate) updates the waypoint
table.  The waypoint table is used to locate waypoints by id, without searching
the objects list. (See getWaypointByID()) */
//=============================================================================
void CWorldBuilderDoc::updateWaypointTable(void)
{
	if (m_waypointTableNeedsUpdate) {
		m_waypointTableNeedsUpdate=false;
		Int i;
		for (i=0; i<MAX_WAYPOINTS; i++) {
			m_waypointTable[i] = nullptr;
		}

		MapObject *pMapObj = nullptr;
		for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
			if (pMapObj->isWaypoint()) {
				Int id = pMapObj->getWaypointID();
				DEBUG_ASSERTCRASH(id>0 && id<MAX_WAYPOINTS, ("Bad waypoint id."));
				if (id>0 && id<MAX_WAYPOINTS) {
					if (m_waypointTable[id] != nullptr) DEBUG_LOG(("Duplicate waypoint id."));
					if (m_waypointTable[id] != nullptr) {
						pMapObj->setWaypointID(getNextWaypointID());
						m_waypointTableNeedsUpdate=true;
					} else {
						m_waypointTable[id] = pMapObj;
					}
				}
			}
		}
	}
}

//=============================================================================
// CWorldBuilderDoc::addWaypointLink
//=============================================================================
/** Adds a waypoint link between two waypoints, referenced by waypoint id. */
//=============================================================================
void CWorldBuilderDoc::addWaypointLink(Int waypointID1, Int waypointID2)
{
	Int i;
	for (i=0; i<m_numWaypointLinks; i++) {
		if (m_waypointLinks[i].waypoint1 == waypointID1 &&
			m_waypointLinks[i].waypoint2 == waypointID2) {
			return; // already linked.
		}
	}
	DEBUG_ASSERTCRASH(m_numWaypointLinks<MAX_WAYPOINTS-1, ("Too many links."));
	if (m_numWaypointLinks<MAX_WAYPOINTS) {
		m_waypointLinks[m_numWaypointLinks].waypoint1 = waypointID1;
		m_waypointLinks[m_numWaypointLinks].waypoint2 = waypointID2;
		m_numWaypointLinks++;
	}
}

//=============================================================================
// CWorldBuilderDoc::removeWaypointLink
//=============================================================================
/** Removes a waypoint link between two waypoints, referenced by waypoint id. */
//=============================================================================
void CWorldBuilderDoc::removeWaypointLink(Int waypointID1, Int waypointID2)
{
	Int i;
	for (i=0; i<m_numWaypointLinks; i++) {
		if (m_waypointLinks[i].waypoint1 == waypointID1 &&
			m_waypointLinks[i].waypoint2 == waypointID2) {
			for (i; i<m_numWaypointLinks-1; i++) {
				m_waypointLinks[i] = m_waypointLinks[i+1];
			}
			m_numWaypointLinks--;
		}
	}
}


//=============================================================================
// CWorldBuilderDoc::getWaypointByID
//=============================================================================
/** Returns a pointer to the waypoint map object referenced by waypointID. */
//=============================================================================
MapObject *CWorldBuilderDoc::getWaypointByID(Int waypointID)
{
	updateWaypointTable();
	DEBUG_ASSERTCRASH(waypointID>=0 && waypointID<MAX_WAYPOINTS, ("Invalid id."));
	if (waypointID>0 && waypointID<MAX_WAYPOINTS) {
		MapObject *pObj = m_waypointTable[waypointID];
		if (pObj && pObj->isWaypoint()) {
			return pObj;
		}
		DEBUG_ASSERTCRASH(pObj==nullptr, ("Waypoint links to an obj that isn't a waypoint."));
	}
	return nullptr;
}

//=============================================================================
// CWorldBuilderDoc::isWaypointLinked
//=============================================================================
/** Returns true if a waypoint is part of a linked waypoint path. */
//=============================================================================
Bool CWorldBuilderDoc::isWaypointLinked(MapObject *pWay)
{
	updateWaypointTable();
	Int i;
	for (i=0; i<m_numWaypointLinks; i++) {
		Int waypointID = m_waypointLinks[i].waypoint1;
		DEBUG_ASSERTCRASH(waypointID>=0 && waypointID<MAX_WAYPOINTS, ("Invalid id."));
		if (waypointID>0 && waypointID<MAX_WAYPOINTS) {
			MapObject *pObj = m_waypointTable[waypointID];
			if (pObj == pWay) return true;
		}
		waypointID = m_waypointLinks[i].waypoint2;
		DEBUG_ASSERTCRASH(waypointID>=0 && waypointID<MAX_WAYPOINTS, ("Invalid id."));
		if (waypointID>0 && waypointID<MAX_WAYPOINTS) {
			MapObject *pObj = m_waypointTable[waypointID];
			if (pObj == pWay) return true;
		}
	}
	return false;
}

//=============================================================================
// CWorldBuilderDoc::updateLinkedWaypointLabels
//=============================================================================
/** Updates the waypoint labels for a linked waypoint path. */
//=============================================================================
void CWorldBuilderDoc::updateLinkedWaypointLabels(MapObject *pWay)
{
	updateWaypointTable();
	Int i;
	for (i=0; i<m_numWaypointLinks; i++) {
		m_waypointLinks[i].processedFlag = false;
	}
	updateLWL(pWay, pWay);
}

//=============================================================================
// CWorldBuilderDoc::updateLWL
//=============================================================================
/** Updates the waypoint labels for a linked waypoint path. */
//=============================================================================
void CWorldBuilderDoc::updateLWL(MapObject *pWay, MapObject *pSrcWay)
{
	while (pWay) {

		Bool exists;
		AsciiString label;
		label = pSrcWay->getProperties()->getAsciiString(TheKey_waypointPathLabel1, &exists);
		if (exists) {
			pWay->getProperties()->setAsciiString(TheKey_waypointPathLabel1, label);
		} else if (pWay->getProperties()->known(TheKey_waypointPathLabel1, Dict::DICT_ASCIISTRING)) {
			pWay->getProperties()->remove(TheKey_waypointPathLabel1);
		}
		label = pSrcWay->getProperties()->getAsciiString(TheKey_waypointPathLabel2, &exists);
		if (exists) {
			pWay->getProperties()->setAsciiString(TheKey_waypointPathLabel2, label);
		} else if (pWay->getProperties()->known(TheKey_waypointPathLabel2, Dict::DICT_ASCIISTRING)){
			pWay->getProperties()->remove(TheKey_waypointPathLabel2);
		}
		label = pSrcWay->getProperties()->getAsciiString(TheKey_waypointPathLabel3, &exists);
		if (exists) {
			pWay->getProperties()->setAsciiString(TheKey_waypointPathLabel3, label);
		} else if (pWay->getProperties()->known(TheKey_waypointPathLabel3, Dict::DICT_ASCIISTRING)) {
			pWay->getProperties()->remove(TheKey_waypointPathLabel3);
		}

		Bool biDirectional;
		biDirectional = pSrcWay->getProperties()->getBool(TheKey_waypointPathBiDirectional, &exists);
		if (exists) {
			pWay->getProperties()->setBool(TheKey_waypointPathBiDirectional, biDirectional);
		}

		MapObject *pCurWay = pWay;
		pWay = nullptr;
		Int i;

		for (i=0; i<m_numWaypointLinks; i++) {
			if (m_waypointLinks[i].processedFlag) continue;
			Bool process = false;
			MapObject *pNewWay = nullptr;
			Int waypointID1 = m_waypointLinks[i].waypoint1;
			Int waypointID2 = m_waypointLinks[i].waypoint2;
			DEBUG_ASSERTCRASH(waypointID1>=0 && waypointID1<MAX_WAYPOINTS, ("Invalid id."));
			DEBUG_ASSERTCRASH(waypointID2>=0 && waypointID2<MAX_WAYPOINTS, ("Invalid id."));
			if (waypointID1>0 && waypointID1<MAX_WAYPOINTS && waypointID2>0 && waypointID2<MAX_WAYPOINTS ) {
				MapObject *pObj = m_waypointTable[waypointID1];
				if (pObj == pCurWay) {
					process = true;
					pNewWay = m_waypointTable[waypointID2];
				}
				pObj = m_waypointTable[waypointID2];
				if (pObj == pCurWay) {
					process = true;
					pNewWay = m_waypointTable[waypointID1];
				}
			}
			if (process) {
				m_waypointLinks[i].processedFlag = true;
				if (pWay == nullptr) {
					pWay = pNewWay;
				} else {
					updateLWL(pNewWay, pSrcWay);
				}
			}
		}
	}
}

//=============================================================================
// CWorldBuilderDoc::getWaypointLink
//=============================================================================
/** Returns the two waypoint ID's that are linked.  Note that due to edits, one
or both waypoints may have been deleted. */
//=============================================================================
void CWorldBuilderDoc::getWaypointLink(Int ndx, Int *waypointID1, Int *waypointID2)
{
	*waypointID1 = 0;
	*waypointID2 = 0;
	if (ndx >=0 && ndx <= m_numWaypointLinks) {
		*waypointID1 = m_waypointLinks[ndx].waypoint1;
		*waypointID2 = m_waypointLinks[ndx].waypoint2;
	}
}

//=============================================================================
// CWorldBuilderDoc::waypointLinkExists
//=============================================================================
/** Returns true if the two waypoint ID's are linked.  Note that due to edits, one
or both waypoints may have been deleted. */
//=============================================================================
Bool CWorldBuilderDoc::waypointLinkExists(Int waypointID1, Int waypointID2)
{
	Int i;
	for (i=0; i<m_numWaypointLinks; i++) {
		if (m_waypointLinks[i].waypoint1 == waypointID1 &&
			m_waypointLinks[i].waypoint2 == waypointID2) {
			return true; // already linked.
		}
	}
	return false;
}


void CWorldBuilderDoc::OnViewReloadtextures()
{
	WW3D::_Invalidate_Textures();
	WorldHeightMapEdit *pMap = GetHeightMap();
	pMap->reloadTextures();
	IRegion2D range = {0,0,0,0};
	updateHeightMap(pMap, false, range);
}

void CWorldBuilderDoc::OnEditScripts()
{
	ASSERT(CMainFrame::GetMainFrame());
	CMainFrame::GetMainFrame()->onEditScripts();
}

/* when "home" key is pressed, goes to the initial camera waypoint or if
 no such waypoint exists, goes to the center of the map */
void CWorldBuilderDoc::OnViewHome()
{
	// !!! needs to be updated if/when camera stuff for worldbuilder changes !!!
	Coord3D pos;
	AsciiString startingCamName = TheNameKeyGenerator->keyToName(TheKey_InitialCameraPosition);
	MapObject *pMapObj = MapObject::getFirstMapObject();

	// set pos to be the coordinates of the center of the map
	pos.x = MAP_XY_FACTOR*m_heightMap->getXExtent()/2;
	pos.y = MAP_XY_FACTOR*m_heightMap->getYExtent()/2;
	pos.x -= MAP_XY_FACTOR*m_heightMap->getBorderSize();
	pos.y -= MAP_XY_FACTOR*m_heightMap->getBorderSize();

	// if waypoint "InitialCameraPosition" exists, replace pos with the appropriate coordinates
	while (pMapObj) {
		if (pMapObj->isWaypoint()) {
			if (startingCamName == pMapObj->getWaypointName()) {
				pos = *pMapObj->getLocation();
			}
		}
		pMapObj = pMapObj->getNext();
	}

	// set camera position to pos
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc) {
		WbView3d *p3View = pDoc->GetActive3DView();
		if (p3View) {
			p3View->setCenterInView(pos.x/MAP_XY_FACTOR, pos.y/MAP_XY_FACTOR);
		}
	}
}

void CWorldBuilderDoc::OnTexturesizingTile4x4()
{
#ifdef EVAL_TILING_MODES
	WorldHeightMapEdit *pMap = GetHeightMap();
	pMap->m_tileMode = WorldHeightMap::TILE_4x4;
	IRegion2D range = {0,0,0,0};
	updateHeightMap(pMap, false, range);
#else
	::AfxMessageBox("Feature not currently enabled.", MB_OK);
#endif
}

void CWorldBuilderDoc::OnUpdateTexturesizingTile4x4(CCmdUI* pCmdUI)
{
#ifdef EVAL_TILING_MODES
	WorldHeightMapEdit *pMap = GetHeightMap();
	pCmdUI->SetCheck(pMap->m_tileMode == WorldHeightMap::TILE_4x4?1:0);
#endif
}

void CWorldBuilderDoc::OnTexturesizingTile6x6()
{
#ifdef EVAL_TILING_MODES
	WorldHeightMapEdit *pMap = GetHeightMap();
	pMap->m_tileMode = WorldHeightMap::TILE_6x6;
	IRegion2D range = {0,0,0,0};
	updateHeightMap(pMap, false, range);
#else
	::AfxMessageBox("Feature not currently enabled.", MB_OK);
#endif
}

void CWorldBuilderDoc::OnUpdateTexturesizingTile6x6(CCmdUI* pCmdUI)
{
#ifdef EVAL_TILING_MODES
	WorldHeightMapEdit *pMap = GetHeightMap();
	pCmdUI->SetCheck(pMap->m_tileMode == WorldHeightMap::TILE_6x6?1:0);
#endif
}

void CWorldBuilderDoc::OnTexturesizingTile8x8()
{
#ifdef EVAL_TILING_MODES
	WorldHeightMapEdit *pMap = GetHeightMap();
	pMap->m_tileMode = WorldHeightMap::TILE_8x8;
	IRegion2D range = {0,0,0,0};
	updateHeightMap(pMap, false, range);
#else
	::AfxMessageBox("Feature not currently enabled.", MB_OK);
#endif
}

void CWorldBuilderDoc::OnUpdateTexturesizingTile8x8(CCmdUI* pCmdUI)
{
#ifdef EVAL_TILING_MODES
	WorldHeightMapEdit *pMap = GetHeightMap();
	pCmdUI->SetCheck(pMap->m_tileMode == WorldHeightMap::TILE_8x8?1:0);
#endif
}

static AsciiString formatScriptLabel(Script *pScr) {
	AsciiString fmt;
	if (pScr->isSubroutine()) {
		fmt.concat("[S ");
	} else {
		fmt.concat("[ns ");
	}
	if (pScr->isActive()) {
		fmt.concat("A ");
	} else {
		fmt.concat("na ");
	}
	if (pScr->isOneShot()) {
		fmt.concat("D] [");
	} else {
		fmt.concat("nd] [");
	}
	if (pScr->isEasy()) {
		fmt.concat("E ");
	}
	if (pScr->isNormal()) {
		fmt.concat("N ");
	}
	if (pScr->isHard()) {
		fmt.concat("H]");
	} else {
		fmt.concat("]");
	}
	fmt.concat(pScr->getName().str());
	return fmt;
}


static void writeScript(FILE *theLogFile, const char * str)
{
	while (*str) {
		if (*str != '\r') {
			fputc(*str, theLogFile);
		}
		str++;
	}
}

#define DUMP_RAW_DICTS
#ifdef DUMP_RAW_DICTS
static void writeRawDict( FILE *theLogFile, const char* nm, const Dict* d )
{
	if (!d)
	{
		fprintf(theLogFile, "Dict %s is null!\n", nm);
		return;
	}

	UnsignedShort len = d->getPairCount();
	fprintf(theLogFile, "Dict %s has %d entries\n", nm, len);
	for (int i = 0; i < len; i++)
	{
		NameKeyType k = d->getNthKey(i);
		AsciiString kname = TheNameKeyGenerator->keyToName(k);

		Dict::DataType t = d->getNthType(i);

		const char* typenames[] = { "Bool", "int", "float", "ascii", "unicode" };
		fprintf(theLogFile, "Entry %d is %s: %s = ",i,typenames[t], kname.str());

		switch(t)
		{
			case Dict::DICT_BOOL:
				fprintf(theLogFile,"%s\n",d->getNthBool(i)?"true":"false");
				break;
			case Dict::DICT_INT:
				fprintf(theLogFile,"%d\n",d->getNthInt(i));
				break;
			case Dict::DICT_REAL:
				fprintf(theLogFile,"%f\n",d->getNthReal(i));
				break;
			case Dict::DICT_ASCIISTRING:
				fprintf(theLogFile,"%s\n",d->getNthAsciiString(i).str());
				break;
			case Dict::DICT_UNICODESTRING:
				fprintf(theLogFile,"%ls\n",d->getNthUnicodeString(i).str());
				break;
			default:
				DEBUG_CRASH(("impossible"));
				break;
		}
	}
}
#endif

static void fprintUnit(FILE *theLogFile, Dict *teamDict, NameKeyType keyMinUnit, NameKeyType keyMaxUnit,
									NameKeyType keyUnitType)
{
	Bool exists;
	Int minCount = teamDict->getInt(keyMinUnit, &exists);
	Int maxCount = teamDict->getInt(keyMaxUnit, &exists);
	AsciiString type = teamDict->getAsciiString(keyUnitType, &exists);
	if (type.isEmpty()) type = "<none>";
	if (minCount || maxCount) {
		fprintf(theLogFile, " %d-%d %s", minCount, maxCount, type.str());
	}

}

void CWorldBuilderDoc::OnDumpDocToText(void)
{
	MapObject *pMapObj = nullptr;
	const char* vetStrings[] = {"Green", "Regular", "Veteran", "Elite"};
	const char* aggroStrings[] = {"Passive", "Normal", "Guard", "Hunt", "Agressive", "Sleep"};
	AsciiString noOwner = "No Owner";
	static FILE *theLogFile = nullptr;
	Bool open = false;
	try {
		char curbuf[_MAX_PATH];
		GetModuleFileName(nullptr, curbuf, sizeof(curbuf));
		if (char *pEnd = strrchr(curbuf, '\\'))
		{
			*(pEnd + 1) = 0;
		}

		strlcat(curbuf, m_strTitle, ARRAY_SIZE(curbuf));
		strlcat(curbuf, ".txt", ARRAY_SIZE(curbuf));

		theLogFile = fopen(curbuf, "w");
		if (theLogFile == nullptr)
			throw;

		open = true;

		fprintf(theLogFile,"\n\n\nDump of Doc Contents\n");

#ifdef DUMP_RAW_DICTS

		writeRawDict(theLogFile, "WorldDict", MapObject::getWorldDict());

		fprintf(theLogFile,"Raw Map Object\n");
		for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext())
		{
			Dict *d = pMapObj->getProperties();
			TeamsInfo *teamInfo = TheSidesList->findTeamInfo(d->getAsciiString(TheKey_originalOwner));
			Dict *teamDict = (teamInfo)?teamInfo->getDict():nullptr;
			writeRawDict( theLogFile, "MapObject",d );
			writeRawDict( theLogFile, "MapObjectTeam",teamDict );
		}
#endif

		// dump the Buildings
		fprintf(theLogFile,"\nBuildings\n");
		for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
			const ThingTemplate* tt = pMapObj->getThingTemplate();
			if (tt)	{
				if (tt->getEditorSorting() == ES_STRUCTURE) {
					Dict *d = pMapObj->getProperties();
					TeamsInfo *teamInfo = TheSidesList->findTeamInfo(d->getAsciiString(TheKey_originalOwner));
					Dict *teamDict = (teamInfo)?teamInfo->getDict():nullptr;
					AsciiString objectOwnerName = (teamDict)?teamDict->getAsciiString(TheKey_teamOwner):noOwner;

					Bool showScript = false;
					AsciiString script = d->getAsciiString(TheKey_objectScriptAttachment, &showScript);

					Bool showName = false;
					AsciiString name = d->getAsciiString(TheKey_objectName, &showName);

					fprintf(theLogFile,"  %s", tt->getName().str());
					fprintf(theLogFile,", %s", objectOwnerName.str());
					fprintf(theLogFile,", @ (%0.0f,%0.0f)", pMapObj->getLocation()->x, pMapObj->getLocation()->y);
					fprintf(theLogFile,", Angle %0.0f", pMapObj->getAngle() * 180 / PI);
					fprintf(theLogFile,", %d%%", d->getInt(TheKey_objectInitialHealth));
					if (showName) {
						fprintf(theLogFile,", Name %s", name.str());
					} else {
						fprintf(theLogFile,", Unnamed");
					}
					if (showScript) {
					fprintf(theLogFile,", Script %s", script.str());
					} else {
					fprintf(theLogFile,", No Script");
					}

					fprintf(theLogFile,"\n");
				}
			}
		}
		fprintf(theLogFile,"End of Buildings\n");

		// dump the units
		fprintf(theLogFile,"\nUnits\n");
		for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
			const ThingTemplate* tt = pMapObj->getThingTemplate();
			if (tt)	{
				if (tt->getEditorSorting() == ES_VEHICLE || tt->getEditorSorting() == ES_INFANTRY) {
					Bool exists;
					Dict *d = pMapObj->getProperties();
					TeamsInfo *teamInfo = TheSidesList->findTeamInfo(d->getAsciiString(TheKey_originalOwner));
					Dict *teamDict = (teamInfo)?teamInfo->getDict():nullptr;

					AsciiString objectOwnerName = (teamDict)?teamDict->getAsciiString(TheKey_teamOwner):noOwner;
					Int veterancy = d->getInt(TheKey_objectVeterancy, &exists);
					if (!exists) {
						veterancy = 0;
					}
					Int aggro = d->getInt(TheKey_objectAggressiveness, &exists);
					if (!exists) {
						aggro = 0;
					}
					aggro++;

					Bool showScript = false;
					AsciiString script = d->getAsciiString(TheKey_objectScriptAttachment, &showScript);

					Bool showName = false;
					AsciiString name = d->getAsciiString(TheKey_objectName, &showName);

					fprintf(theLogFile,"  %s", tt->getName().str());
					fprintf(theLogFile,", %s", objectOwnerName.str());
					fprintf(theLogFile,", @ %0.0f.%0.0f", pMapObj->getLocation()->x/10, pMapObj->getLocation()->y/10);
					fprintf(theLogFile,", Angle %0.0f", pMapObj->getAngle() * 180 / PI);
					fprintf(theLogFile,", %d%%", d->getInt(TheKey_objectInitialHealth));
					if (showName) {
						fprintf(theLogFile,", Name %s", name.str());
					} else {
						fprintf(theLogFile,", Unnamed");
					}
					if (showScript) {
					fprintf(theLogFile,", Script %s", script.str());
					} else {
					fprintf(theLogFile,", No Script");
					}
					fprintf(theLogFile,", Team %s", d->getAsciiString(TheKey_originalOwner).str());
					fprintf(theLogFile,", %s", d->getBool(TheKey_objectRecruitableAI, &exists)?"AIRecruitable":"Not AIRecruitable");
					fprintf(theLogFile,", %s", d->getBool(TheKey_objectSelectable, &exists)?"Selectable":"Not Selectable");
					fprintf(theLogFile,", %s", aggroStrings[aggro]);
					fprintf(theLogFile,", %s", vetStrings[veterancy]);

					fprintf(theLogFile,"\n");
				}
			}
		}
		fprintf(theLogFile,"End of Units\n");

		fprintf(theLogFile,"\nObject Types summary\n");
		{
			Int totalObjectCount = 0;
			std::map<AsciiString, Int> mapOfTemplates;
			std::map<AsciiString, Int>::iterator it;
			for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
				if (pMapObj->getThingTemplate()) {
					++totalObjectCount;

					it = mapOfTemplates.find(pMapObj->getThingTemplate()->getName());
					if (it == mapOfTemplates.end()) {
						mapOfTemplates.insert(std::make_pair(pMapObj->getThingTemplate()->getName(), 1));
					} else {
						++(it->second);
					}
				}
			}

			fprintf(theLogFile, "Total Map Objects (with ThingTemplates): %d\n", totalObjectCount);

			while (!mapOfTemplates.empty()) {
				std::map<AsciiString, Int>::iterator storedIt = mapOfTemplates.begin();

				for (it = mapOfTemplates.begin(); it != mapOfTemplates.end(); ++it) {
					if (storedIt->second < it->second) {
						storedIt = it;
					}
				}

				fprintf(theLogFile, "Map Object: %s, Instances: %d\n", storedIt->first.str(), storedIt->second);

				// Now, erase it.
				mapOfTemplates.erase(storedIt);
			}
		}
		fprintf(theLogFile,"\nEnd of Object Types summary\n");

		// dump the waypoints
		fprintf(theLogFile,"\nWaypoints\n");
		for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
			if (pMapObj->isWaypoint()) {
				fprintf(theLogFile,"  %s, @ %0.0f.%0.0f\n", pMapObj->getWaypointName().str(), pMapObj->getLocation()->x/10, pMapObj->getLocation()->y/10);
			}
		}
		fprintf(theLogFile,"End of Waypoints\n");

		fprintf(theLogFile,"\nProps\n");
		for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
			const ThingTemplate* tt = pMapObj->getThingTemplate();
			if (tt)	{
				if (tt->getEditorSorting() == ES_MISC_MAN_MADE) {
					Dict *d = pMapObj->getProperties();
					TeamsInfo *teamInfo = TheSidesList->findTeamInfo(d->getAsciiString(TheKey_originalOwner));
					Dict *teamDict = (teamInfo)?teamInfo->getDict():nullptr;

					AsciiString objectOwnerName = (teamDict)?teamDict->getAsciiString(TheKey_teamOwner):noOwner;

					Bool showName = false;
					AsciiString name = d->getAsciiString(TheKey_objectName, &showName);

					fprintf(theLogFile,"  %s", tt->getName().str());
					fprintf(theLogFile,", %s", objectOwnerName.str());
					fprintf(theLogFile,", @ %0.0f.%0.0f", pMapObj->getLocation()->x/10, pMapObj->getLocation()->y/10);
					fprintf(theLogFile,", Angle %0.0f", pMapObj->getAngle() * 180 / PI);
					fprintf(theLogFile,", %d%%", d->getInt(TheKey_objectInitialHealth));
					if (showName) {
						fprintf(theLogFile,", Name %s", name.str());
					} else {
						fprintf(theLogFile,", Unnamed");
					}

					fprintf(theLogFile,"\n");
				}
			}
		}
		fprintf(theLogFile,"End of Props\n");

		fprintf(theLogFile,"\nAudio\n");
		for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
			const ThingTemplate* tt = pMapObj->getThingTemplate();
			if (tt)	{
				if (tt->getEditorSorting() == ES_AUDIO) {
					Dict *d = pMapObj->getProperties();

					Bool showName = false;
					AsciiString name = d->getAsciiString(TheKey_objectName, &showName);

					fprintf(theLogFile,"  %s", tt->getName().str());
					fprintf(theLogFile,", @ %0.0f.%0.0f", pMapObj->getLocation()->x/10, pMapObj->getLocation()->y/10);
					if (showName) {
						fprintf(theLogFile,", Name %s", name.str());
					} else {
						fprintf(theLogFile,", Unnamed");
					}

					fprintf(theLogFile,"\n");
				}
			}
		}
		fprintf(theLogFile,"End of Audio\n");

		fprintf(theLogFile,"\nTeams\n");
		Int j;
		for (j=0; j<TheSidesList->getNumSides(); j++) {
			Dict *d = TheSidesList->getSideInfo(j)->getDict();
#ifdef DUMP_RAW_DICTS
writeRawDict( theLogFile, "TeamDict",d );
#endif
			AsciiString name = d->getAsciiString(TheKey_playerName);
			AsciiString fmt;
			if (name.isEmpty())
				fmt.format("%s", "(neutral)");
			else
				fmt.format("%s",name.str());
			fprintf(theLogFile, "PLAYER %s\n", fmt.str());
			Int numTeams = TheSidesList->getNumTeams();
			for (Int i=0; i<numTeams; i++)
			{
				TeamsInfo *ti = TheSidesList->getTeamInfo(i);
#ifdef DUMP_RAW_DICTS
writeRawDict( theLogFile, "TeamInfo",ti->getDict() );
#endif
				if (ti->getDict()->getAsciiString(TheKey_teamOwner) == name)
				{
					Bool exists;
					AsciiString teamName = ti->getDict()->getAsciiString(TheKey_teamName);
					AsciiString waypoint = ti->getDict()->getAsciiString(TheKey_teamHome, &exists);
					CString pri;
					pri.Format(TEXT("%d"), ti->getDict()->getInt(TheKey_teamProductionPriority, &exists));
					AsciiString trigger = ti->getDict()->getAsciiString(TheKey_teamProductionCondition, &exists);

					fprintf(theLogFile, "TEAM %s home '%s', priority %s, condition '%s',\n", teamName.str(),
						waypoint.str(), static_cast<LPCSTR>(pri), trigger.str());
					fprintf(theLogFile, "  UNITS:");
					fprintUnit(theLogFile, ti->getDict(), TheKey_teamUnitMinCount1, TheKey_teamUnitMaxCount1, TheKey_teamUnitType1);
					fprintUnit(theLogFile, ti->getDict(), TheKey_teamUnitMinCount2, TheKey_teamUnitMaxCount2, TheKey_teamUnitType2);
					fprintUnit(theLogFile, ti->getDict(), TheKey_teamUnitMinCount3, TheKey_teamUnitMaxCount3, TheKey_teamUnitType3);
					fprintUnit(theLogFile, ti->getDict(), TheKey_teamUnitMinCount4, TheKey_teamUnitMaxCount4, TheKey_teamUnitType4);
					fprintUnit(theLogFile, ti->getDict(), TheKey_teamUnitMinCount5, TheKey_teamUnitMaxCount5, TheKey_teamUnitType5);
					fprintUnit(theLogFile, ti->getDict(), TheKey_teamUnitMinCount6, TheKey_teamUnitMaxCount6, TheKey_teamUnitType6);
					fprintUnit(theLogFile, ti->getDict(), TheKey_teamUnitMinCount7, TheKey_teamUnitMaxCount7, TheKey_teamUnitType7);
					fprintf(theLogFile, "\n  SCRIPTS: ");
					AsciiString script = ti->getDict()->getAsciiString(TheKey_teamOnCreateScript, &exists);
					if (script.isEmpty()) script="<none>";
					fprintf(theLogFile, "OnCreate='%s'", script.str());
					script = ti->getDict()->getAsciiString(TheKey_teamOnIdleScript, &exists);
					if (script.isEmpty()) script="<none>";
					fprintf(theLogFile, " OnIdle='%s'", script.str());
					script = ti->getDict()->getAsciiString(TheKey_teamOnDestroyedScript, &exists);
					if (script.isEmpty()) script="<none>";
					fprintf(theLogFile, " OnDestroyed='%s'", script.str());
					script = ti->getDict()->getAsciiString(TheKey_teamEnemySightedScript, &exists);
					if (script.isEmpty()) script="<none>";
					fprintf(theLogFile, " OnEnemySighted='%s'", script.str());
					script = ti->getDict()->getAsciiString(TheKey_teamAllClearScript, &exists);
					if (script.isEmpty()) script="<none>";
					fprintf(theLogFile, " OnAllClear='%s'\n", script.str());
				}
			}
		}
		fprintf(theLogFile,"End of Teams\n");

		fprintf(theLogFile,"\nScripts\n");
		Int i, groupNdx;
		for (i=0; i<TheSidesList->getNumSides(); i++) {
			Dict *d = TheSidesList->getSideInfo(i)->getDict();
#ifdef DUMP_RAW_DICTS
writeRawDict( theLogFile, "Scripts",d );
#endif
			AsciiString name = d->getAsciiString(TheKey_playerName);
			UnicodeString uni = d->getUnicodeString(TheKey_playerDisplayName);
			AsciiString fmt;
			if (name.isEmpty())
				fmt.format("%s", "(neutral)");
			else
				fmt.format("%s",name.str());
			fprintf(theLogFile, "PLAYER %s\n", fmt.str());
			ScriptList *pSL = TheSidesList->getSideInfo(i)->getScriptList();
			if (pSL) {
				ScriptGroup *pGroup = pSL->getScriptGroup();
				for (groupNdx = 0; pGroup; groupNdx++,pGroup=pGroup->getNext()) {
					AsciiString fmt;
					if (pGroup->getName().isEmpty())
						continue;
					else
						fmt.format("%s",pGroup->getName().str());
					fprintf(theLogFile, "GROUP %s\n", fmt.str());
					Script *pScr = pGroup->getScript();
					if (pScr) {
						Int scriptNdx;
						for (scriptNdx = 0; pScr; scriptNdx++,pScr=pScr->getNext()) {
							AsciiString fmt;
							if (pScr->getName().isEmpty())
								continue;
							fmt = formatScriptLabel(pScr);
							fprintf(theLogFile, "%s:\n", fmt.str());
							AsciiString scriptComment;
							AsciiString scriptText;
							if (pScr) {
								scriptComment = pScr->getComment();
								scriptText = pScr->getUiText();
							}
							if (scriptComment.isNotEmpty()) {
								fprintf(theLogFile, "//:%s:\n", scriptComment.str());
							}
							writeScript(theLogFile, scriptText.str());
							fprintf(theLogFile, "\n");

						}
					}
				}
				Script *pScr = pSL->getScript();
				if (pScr) {
					Int scriptNdx;
					for (scriptNdx = 0; pScr; scriptNdx++,pScr=pScr->getNext()) {
						AsciiString fmt;
						if (pScr->getName().isEmpty())
							continue;
						fmt = formatScriptLabel(pScr);
						fprintf(theLogFile, "%s:\n", fmt.str());
						AsciiString scriptComment;
						AsciiString scriptText;
						if (pScr) {
							scriptComment = pScr->getComment();
							scriptText = pScr->getUiText();
						}
						if (scriptComment.isNotEmpty()) {
							fprintf(theLogFile, "//:%s:\n", scriptComment.str());
						}
						writeScript(theLogFile, scriptText.str());
						fprintf(theLogFile, "\n");
					}
				}
			}
		}
		fprintf(theLogFile,"End of Scripts\n");
		fclose(theLogFile);
		open = false;
	} catch (...) {
		if (open) {
			fclose(theLogFile);
		}
	}
}

// Find the index nearest the point in the preferred direction
void FindIndexNearest(CWorldBuilderDoc* pDoc, const Coord3D* point, CPoint* outNdx, DIRECTION pref )
{
	Coord3D testPoint = *point;
	switch(pref)
	{
		case PREFER_CENTER:
		{
			break;
		}
		case PREFER_LEFT:
		{
			testPoint.x -= MAP_XY_FACTOR / 2;
			break;
		}
		case PREFER_TOP:
		{
			testPoint.y += MAP_XY_FACTOR / 2;
			break;
		}
		case PREFER_RIGHT:
		{
			testPoint.x += MAP_XY_FACTOR / 2;
			break;
		}
		case PREFER_BOTTOM:
		{
			testPoint.y -= MAP_XY_FACTOR / 2;
			break;
		}
	};

	pDoc->getCellIndexFromCoord(testPoint, outNdx);
}

Bool IndexInRect(CWorldBuilderDoc* pDoc, const Coord3D* bl, const Coord3D* tl, const Coord3D* br, const Coord3D* tr, CPoint* index)
{
	Coord3D testPoint;
	pDoc->getCoordFromCellIndex(*index, &testPoint);
	return PointInsideRect3D(bl, tl, br, tr, &testPoint);
}

Bool AddUniqueAndNeighbors(CWorldBuilderDoc* pDoc, const Coord3D* bl, const Coord3D* tl, const Coord3D* br, const Coord3D* tr, CPoint ndx, VecHeightMapIndexes* allIndices)
{
	if (!allIndices) {
		return false;
	}

	if (!IndexInRect(pDoc, bl, tl, br, tr, &ndx)) {
		return false;
	}

	if (std::find(allIndices->begin(), allIndices->end(), ndx) != allIndices->end()) {
		return false;
	}

	// we have a winner. This index is both inside the rectangle and not already in the index list.
	allIndices->push_back(ndx);

	// now attempt to add the neighbors.
	// first left
	ndx.x += 1;
	AddUniqueAndNeighbors(pDoc, bl, tl, br, tr, ndx, allIndices);

	// then right
	ndx.x -= 2;
	AddUniqueAndNeighbors(pDoc,bl, tl, br, tr, ndx, allIndices);

	// then top
	ndx.x += 1;
	ndx.y += 1;
	AddUniqueAndNeighbors(pDoc,bl, tl, br, tr, ndx, allIndices);

	// then bottom
	ndx.y -= 2;
	AddUniqueAndNeighbors(pDoc, bl, tl, br, tr, ndx, allIndices);

	return true;
}


void CWorldBuilderDoc::OnRemoveclifftexmapping()
{
	if (::AfxMessageBox(IDS_CONFIRM_REMOVE_CLIFF_MAPPING, MB_YESNO) == IDYES) {
		if (m_heightMap) {

			WorldHeightMapEdit *htMapEditCopy = GetHeightMap()->duplicate();
			if (htMapEditCopy == nullptr) return;
			if (htMapEditCopy->removeCliffMapping()) {  // does all the work.
				IRegion2D partialRange = {0,0,0,0};
				updateHeightMap(htMapEditCopy, false, partialRange);
				WBDocUndoable *pUndo = new WBDocUndoable(this, htMapEditCopy);
				this->AddAndDoUndoable(pUndo);
				REF_PTR_RELEASE(pUndo); // belongs to this now.
			}
			REF_PTR_RELEASE(htMapEditCopy);
		}
	}
}

Int CWorldBuilderDoc::getNumBoundaries(void) const
{
	return m_heightMap->getNumBoundaries();
}

void CWorldBuilderDoc::getBoundary(Int ndx, ICoord2D* border) const
{
	m_heightMap->getBoundary(ndx, border);
}

void CWorldBuilderDoc::addBoundary(ICoord2D* boundaryToAdd)
{
	m_heightMap->addBoundary(boundaryToAdd);
}

void CWorldBuilderDoc::changeBoundary(Int ndx, ICoord2D *border)
{
	m_heightMap->changeBoundary(ndx, border);
}

void CWorldBuilderDoc::removeLastBoundary(void)
{
	m_heightMap->removeLastBoundary();
}

void CWorldBuilderDoc::findBoundaryNear(Coord3D *pt, float okDistance, Int *outNdx, Int *outHandle)
{
	m_heightMap->findBoundaryNear(pt, okDistance, outNdx, outHandle);
}


/*
**	Command & Conquer Generals(tm)
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

// FILE: ReplayMenu.cpp /////////////////////////////////////////////////////////////////////
// Author: Chris The masta Huybregts, December 2001
// Description: Replay Menus
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine


#include "Lib/BaseType.h"
#include "Common/FileSystem.h"
#include "Common/GameEngine.h"
#include "Common/GameState.h"
#include "Common/Recorder.h"
#include "Common/version.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/MessageBox.h"
#include "GameClient/MapUtil.h"
#include "GameClient/Mouse.h"
#include "GameClient/GameText.h"
#include "GameClient/GameWindowTransitions.h"

typedef UnicodeString ReplayName;
typedef UnicodeString TooltipString;
typedef std::map<ReplayName, TooltipString> ReplayTooltipMap;

static ReplayTooltipMap replayTooltipCache;

// window ids -------------------------------------------------------------------------------------
static NameKeyType parentReplayMenuID = NAMEKEY_INVALID;
static NameKeyType buttonLoadID = NAMEKEY_INVALID;
static NameKeyType buttonBackID = NAMEKEY_INVALID;
static NameKeyType listboxReplayFilesID = NAMEKEY_INVALID;
static NameKeyType buttonDeleteID = NAMEKEY_INVALID;
static NameKeyType buttonCopyID = NAMEKEY_INVALID;

static Bool isShuttingDown = false;

// window pointers --------------------------------------------------------------------------------
static GameWindow *parentReplayMenu = NULL;
static GameWindow *buttonLoad = NULL;
static GameWindow *buttonBack = NULL;
static GameWindow *listboxReplayFiles = NULL;
static GameWindow *buttonDelete = NULL;
static GameWindow *buttonCopy = NULL;
static Int	initialGadgetDelay = 2;
static Bool justEntered = FALSE;


#if defined(RTS_DEBUG)
static GameWindow *buttonAnalyzeReplay = NULL;
#endif

void deleteReplay( void );
void copyReplay( void );
static Bool callCopy = FALSE;
static Bool callDelete = FALSE;
void deleteReplayFlag( void ) { callDelete = TRUE;}
void copyReplayFlag( void ) { callCopy = TRUE;}

UnicodeString GetReplayFilenameFromListbox(GameWindow *listbox, Int index)
{
	UnicodeString fname = GadgetListBoxGetText(listbox, index);

	if (fname == TheGameText->fetch("GUI:LastReplay"))
	{
		fname.translate(TheRecorder->getLastReplayFileName());
	}

	UnicodeString ext;
	ext.translate(TheRecorder->getReplayExtention());
	fname.concat(ext);

	return fname;
}

//-------------------------------------------------------------------------------------------------

static Bool readReplayMapInfo(const AsciiString& filename, RecorderClass::ReplayHeader &header, ReplayGameInfo &info, const MapMetaData *&mapData)
{
	header.forPlayback = FALSE;
	header.filename = filename;

	if (TheRecorder != NULL && TheRecorder->readReplayHeader(header))
	{
		if (ParseAsciiStringToGameInfo(&info, header.gameOptions))
		{
			if (TheMapCache != NULL)
				mapData = TheMapCache->findMap(info.getMap());
			else
				mapData = NULL;

			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------

static void removeReplayExtension(UnicodeString& replayName)
{
	const Int extensionLength = TheRecorder->getReplayExtention().getLength();
	replayName.truncateBy(extensionLength);
}

//-------------------------------------------------------------------------------------------------

static UnicodeString createReplayName(const AsciiString& filename)
{
	AsciiString lastReplayFName = TheRecorder->getLastReplayFileName();
	lastReplayFName.concat(TheRecorder->getReplayExtention());
	UnicodeString replayName;

	if (lastReplayFName.compareNoCase(filename) == 0)
	{
		replayName = TheGameText->fetch("GUI:LastReplay");
	}
	else
	{
		replayName.translate(filename);
		removeReplayExtension(replayName);
	}
	return replayName;
}

//-------------------------------------------------------------------------------------------------

static UnicodeString createMapName(const AsciiString& filename, const ReplayGameInfo& info, const MapMetaData *mapData)
{
	UnicodeString mapName;
	if (!mapData)
	{
		// TheSuperHackers @bugfix helmutbuhler 08/03/2025 Just use the filename.
		// Displaying a long map path string would break the map list gui.
		const char* filename = info.getMap().reverseFind('\\');
		mapName.translate(filename ? filename + 1 : info.getMap());
	}
	else
	{
		mapName = mapData->m_displayName;
	}
	return mapName;
}

//-------------------------------------------------------------------------------------------------
// TheSuperHackers @feature Stubbjax 21/10/2025 Show extra info tooltip when hovering over a replay.

static void showReplayTooltip(GameWindow* window, WinInstanceData* instData, UnsignedInt mouse)
{
	Int x, y, row, col;
	x = LOLONGTOSHORT(mouse);
	y = HILONGTOSHORT(mouse);

	GadgetListBoxGetEntryBasedOnXY(window, x, y, row, col);

	if (row == -1 || col == -1)
	{
		TheMouse->setCursorTooltip(UnicodeString::TheEmptyString);
		return;
	}

	UnicodeString replayFileName = GetReplayFilenameFromListbox(window, row);

	ReplayTooltipMap::const_iterator it = replayTooltipCache.find(replayFileName);
	if (it != replayTooltipCache.end())
		TheMouse->setCursorTooltip(it->second, -1, NULL, 1.5f);
	else
		TheMouse->setCursorTooltip(UnicodeString::TheEmptyString);
}

static UnicodeString buildReplayTooltip(RecorderClass::ReplayHeader header, ReplayGameInfo info)
{
	UnicodeString tooltipStr;

	if (header.endTime < header.startTime)
		header.startTime = header.endTime;

	time_t totalSeconds = header.endTime - header.startTime;
	UnsignedInt hours = totalSeconds / 3600;
	UnsignedInt mins = (totalSeconds % 3600) / 60;
	UnsignedInt secs = totalSeconds % 60;
	Real fps = totalSeconds > 0 ? header.frameCount / totalSeconds : 0;
	tooltipStr.format(L"%02u:%02u:%02u (%g fps)", hours, mins, secs, fps);

	if (header.localPlayerIndex >= 0)
	{
		// MP game
		for (Int i = 0; i < MAX_SLOTS; ++i)
		{
			const GameSlot* slot = info.getConstSlot(i);
			if (slot && slot->isHuman())
			{
				tooltipStr.concat(L"\n");
				tooltipStr.concat(info.getConstSlot(i)->getName());
			}
		}
	}

	return tooltipStr;
}

//-------------------------------------------------------------------------------------------------
/** Populate the listbox with the names of the available replay files */
//-------------------------------------------------------------------------------------------------
void PopulateReplayFileListbox(GameWindow *listbox)
{
	replayTooltipCache.clear();

	if (!TheMapCache)
		return;

	GadgetListBoxReset(listbox);
	const Int listboxLength = GadgetListBoxGetListLength(listbox);

	// TheSuperHackers @tweak xezon 08/06/2025 Now shows missing maps in red color.
	enum {
		COLOR_SP = 0,
		COLOR_SP_CRC_MISMATCH,
		COLOR_MP,
		COLOR_MP_CRC_MISMATCH,
		COLOR_MISSING_MAP,
		COLOR_MISSING_MAP_CRC_MISMATCH,
		COLOR_MAX
	};
	Color colors[] = {
		GameMakeColor( 255, 255, 255, 255 ),
		GameMakeColor( 128, 128, 128, 255 ),
		GameMakeColor( 255, 255, 255, 255 ),
		GameMakeColor( 128, 128, 128, 255 ),
		GameMakeColor( 243,  24,  24, 255 ),
		GameMakeColor( 128,  32,  32, 255 )
	};
	static_assert(ARRAY_SIZE(colors) == COLOR_MAX, "Mismatch between colors array size and COLOR_MAX");

	AsciiString asciistr;
	AsciiString asciisearch;
	asciisearch = "*";
	asciisearch.concat(TheRecorder->getReplayExtention());

	FilenameList replayFilenames;
	FilenameListIter it;

	TheFileSystem->getFileListInDirectory(TheRecorder->getReplayDir(), asciisearch, replayFilenames, FALSE);

	TheMapCache->updateCache();

	for (it = replayFilenames.begin(); it != replayFilenames.end(); ++it)
	{
		// just want the filename
		asciistr.set((*it).reverseFind('\\') + 1);

		RecorderClass::ReplayHeader header;
		ReplayGameInfo info;
		const MapMetaData *mapData;

		if (readReplayMapInfo(asciistr, header, info, mapData))
		{
			// columns are: name, date, version, map, extra

			// name
			UnicodeString replayNameToShow = createReplayName(asciistr);

			UnicodeString displayTimeBuffer = getUnicodeTimeBuffer(header.timeVal);

			//displayTimeBuffer.format( L"%ls", timeBuffer);

			// version (no-op)

			// map
			UnicodeString mapStr = createMapName(asciistr, info, mapData);

			// tooltip
			UnicodeString tooltipStr = buildReplayTooltip(header, info);

			UnicodeString key;
			key.translate(asciistr);
			replayTooltipCache[key] = tooltipStr;

			// pick a color
			Color color;
			Color mapColor;

			const Bool hasMap = mapData != NULL;

			const Bool isCrcCompatible = RecorderClass::replayMatchesGameVersion(header);

			if (isCrcCompatible)
			{
				if (header.localPlayerIndex >= 0)
				{
					// MP
					color = colors[COLOR_MP];
				}
				else
				{
					// SP
					color = colors[COLOR_SP];
				}

				if (hasMap)
					mapColor = color;
				else
					mapColor = colors[COLOR_MISSING_MAP];
			}
			else
			{
				if (header.localPlayerIndex >= 0)
				{
					// MP
					color = colors[COLOR_MP_CRC_MISMATCH];
				}
				else
				{
					// SP
					color = colors[COLOR_SP_CRC_MISMATCH];
				}

				if (hasMap)
					mapColor = color;
				else
					mapColor = colors[COLOR_MISSING_MAP_CRC_MISMATCH];
			}

			const Int insertionIndex = GadgetListBoxAddEntryText(listbox, replayNameToShow, color, -1, 0);
			DEBUG_ASSERTCRASH(insertionIndex >= 0, ("Expects valid index"));
			GadgetListBoxAddEntryText(listbox, displayTimeBuffer, color, insertionIndex, 1);
			GadgetListBoxAddEntryText(listbox, header.versionString, color, insertionIndex, 2);
			GadgetListBoxAddEntryText(listbox, mapStr, mapColor, insertionIndex, 3);

			// TheSuperHackers @performance Now stops processing when the list is full.
			if (insertionIndex == listboxLength - 1)
				break;
		}
	}
	GadgetListBoxSetSelected(listbox, 0);
}

//-------------------------------------------------------------------------------------------------
/** Initialize the single player menu */
//-------------------------------------------------------------------------------------------------
void ReplayMenuInit( WindowLayout *layout, void *userData )
{
	TheShell->showShellMap(TRUE);

	// get ids for our children controls
	parentReplayMenuID = TheNameKeyGenerator->nameToKey( AsciiString("ReplayMenu.wnd:ParentReplayMenu") );
	buttonLoadID = TheNameKeyGenerator->nameToKey( AsciiString("ReplayMenu.wnd:ButtonLoadReplay") );
	buttonBackID = TheNameKeyGenerator->nameToKey( AsciiString("ReplayMenu.wnd:ButtonBack") );
	listboxReplayFilesID = TheNameKeyGenerator->nameToKey( AsciiString("ReplayMenu.wnd:ListboxReplayFiles") );
	buttonDeleteID = TheNameKeyGenerator->nameToKey( AsciiString("ReplayMenu.wnd:ButtonDeleteReplay") );
	buttonCopyID = TheNameKeyGenerator->nameToKey( AsciiString("ReplayMenu.wnd:ButtonCopyReplay") );

	parentReplayMenu = TheWindowManager->winGetWindowFromId( NULL, parentReplayMenuID );
	buttonLoad = TheWindowManager->winGetWindowFromId( parentReplayMenu, buttonLoadID );
	buttonBack = TheWindowManager->winGetWindowFromId( parentReplayMenu, buttonBackID );
	listboxReplayFiles = TheWindowManager->winGetWindowFromId( parentReplayMenu, listboxReplayFilesID );
	listboxReplayFiles->winSetTooltipFunc(showReplayTooltip);
	buttonDelete = TheWindowManager->winGetWindowFromId( parentReplayMenu, buttonDeleteID );
	buttonCopy = TheWindowManager->winGetWindowFromId( parentReplayMenu, buttonCopyID );

	//Load the listbox shiznit
	GadgetListBoxReset(listboxReplayFiles);
	PopulateReplayFileListbox(listboxReplayFiles);

#if defined(RTS_DEBUG)
	WinInstanceData instData;
	instData.init();
	BitSet( instData.m_style, GWS_PUSH_BUTTON | GWS_MOUSE_TRACK );
	instData.m_textLabelString = "Debug: Analyze Replay";
	instData.setTooltipText(UnicodeString(L"Only Used in Debug and Internal!"));
	buttonAnalyzeReplay = TheWindowManager->gogoGadgetPushButton( parentReplayMenu,
																									 WIN_STATUS_ENABLED | WIN_STATUS_IMAGE,
																									 4, 4,
																									 180, 26,
																									 &instData, NULL, TRUE );
#endif

	// show menu
	layout->hide( FALSE );

	// set keyboard focus to main parent
	TheWindowManager->winSetFocus( parentReplayMenu );
	justEntered = TRUE;
	initialGadgetDelay = 2;
	GameWindow *win = TheWindowManager->winGetWindowFromId(NULL, TheNameKeyGenerator->nameToKey("ReplayMenu.wnd:GadgetParent"));
	if(win)
		win->winHide(TRUE);
	isShuttingDown = FALSE;

}

//-------------------------------------------------------------------------------------------------
/** single player menu shutdown method */
//-------------------------------------------------------------------------------------------------
void ReplayMenuShutdown( WindowLayout *layout, void *userData )
{

	Bool popImmediate = *(Bool *)userData;
	if( popImmediate )
	{

		layout->hide( TRUE );
		TheShell->shutdownComplete( layout );
		return;

	}

	// our shutdown is complete
	TheTransitionHandler->reverse("ReplayMenuFade");
	isShuttingDown = TRUE;
}

//-------------------------------------------------------------------------------------------------
/** single player menu update method */
//-------------------------------------------------------------------------------------------------
void ReplayMenuUpdate( WindowLayout *layout, void *userData )
{
	if(justEntered)
	{
		if(initialGadgetDelay == 1)
		{
			TheTransitionHandler->remove("MainMenuDefaultMenuLogoFade");
			TheTransitionHandler->setGroup("ReplayMenuFade");
			initialGadgetDelay = 2;
			justEntered = FALSE;
		}
		else
			initialGadgetDelay--;
	}

	if(callCopy)
		copyReplay();
	if(callDelete)
		deleteReplay();
		// We'll only be successful if we've requested to
	if(isShuttingDown && TheShell->isAnimFinished()&& TheTransitionHandler->isFinished())
		TheShell->shutdownComplete( layout );

}

//-------------------------------------------------------------------------------------------------
/** Replay menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType ReplayMenuInput( GameWindow *window, UnsignedInt msg,
																						WindowMsgData mData1, WindowMsgData mData2 )
{

	switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CHAR:
		{
			UnsignedByte key = mData1;
			UnsignedByte state = mData2;

			switch( key )
			{

				// ----------------------------------------------------------------------------------------
				case KEY_ESC:
				{

					//
					// send a simulated selected event to the parent window of the
					// back/exit button
					//
					if( BitIsSet( state, KEY_STATE_UP ) )
					{

						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED,
																								(WindowMsgData)buttonBack, buttonBackID );

					}

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}

			}

		}

	}

	return MSG_IGNORED;

}

void reallyLoadReplay(void)
{
	UnicodeString filename;
	Int selected;
	GadgetListBoxGetSelected( listboxReplayFiles,  &selected );
	if(selected < 0)
	{
		MessageBoxOk(TheGameText->fetch("GUI:NoFileSelected"),TheGameText->fetch("GUI:PleaseSelectAFile"), NULL);
		return;
	}

	filename = GetReplayFilenameFromListbox(listboxReplayFiles, selected);

	AsciiString asciiFilename;
	asciiFilename.translate(filename);

	TheRecorder->playbackFile(asciiFilename);

	if(parentReplayMenu != NULL)
	{
		parentReplayMenu->winHide(TRUE);
	}
}

static void loadReplay(UnicodeString filename)
{
	AsciiString asciiFilename;
	asciiFilename.translate(filename);

	RecorderClass::ReplayHeader header;
	ReplayGameInfo info;
	const MapMetaData *mapData;

	if(!readReplayMapInfo(asciiFilename, header, info, mapData))
	{
		// TheSuperHackers @bugfix Prompts a message box when the replay was deleted by the user while the Replay Menu was opened.

		UnicodeString title = TheGameText->FETCH_OR_SUBSTITUTE("GUI:ReplayFileNotFoundTitle", L"REPLAY NOT FOUND");
		UnicodeString body = TheGameText->FETCH_OR_SUBSTITUTE("GUI:ReplayFileNotFound", L"This replay cannot be loaded because the file no longer exists on this device.");

		MessageBoxOk(title, body, NULL);
	}
	else if(mapData == NULL)
	{
		// TheSuperHackers @bugfix Prompts a message box when the map used by the replay was not found.

		UnicodeString title = TheGameText->FETCH_OR_SUBSTITUTE("GUI:ReplayMapNotFoundTitle", L"MAP NOT FOUND");
		UnicodeString body = TheGameText->FETCH_OR_SUBSTITUTE("GUI:ReplayMapNotFound", L"This replay cannot be loaded because the map was not found on this device.");

		MessageBoxOk(title, body, NULL);
	}
	else if(!TheRecorder->replayMatchesGameVersion(header))
	{
		// Pressing OK loads the replay.

		MessageBoxOkCancel(TheGameText->fetch("GUI:OlderReplayVersionTitle"), TheGameText->fetch("GUI:OlderReplayVersion"), reallyLoadReplay, NULL);
	}
	else
	{
		TheRecorder->playbackFile(asciiFilename);

		if(parentReplayMenu != NULL)
		{
			parentReplayMenu->winHide(TRUE);
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** single player menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType ReplayMenuSystem( GameWindow *window, UnsignedInt msg,
														 WindowMsgData mData1, WindowMsgData mData2 )
{

	switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CREATE:
		{


			break;

		}

		//---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			// if we're given the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			return MSG_HANDLED;

		}
		//---------------------------------------------------------------------------------------------
		case GLM_DOUBLE_CLICKED:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				if( controlID == listboxReplayFilesID )
				{
					int rowSelected = mData2;

					if (rowSelected >= 0)
					{
						UnicodeString filename = GetReplayFilenameFromListbox(listboxReplayFiles, rowSelected);
						loadReplay(filename);
					}
				}
				break;
			}
		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			UnicodeString filename;
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

#if defined(RTS_DEBUG)
			if( controlID == buttonAnalyzeReplay->winGetWindowId() )
			{
				if(listboxReplayFiles)
				{
					Int selected;
					GadgetListBoxGetSelected( listboxReplayFiles,  &selected );
					if(selected < 0)
					{
						MessageBoxOk(UnicodeString(L"Blah Blah"),UnicodeString(L"Please select something munkee boy"), NULL);
						break;
					}

					filename = GetReplayFilenameFromListbox(listboxReplayFiles, selected);

					AsciiString asciiFilename;
					asciiFilename.translate(filename);
					if (TheRecorder->analyzeReplay(asciiFilename))
					{
						do
						{
							TheRecorder->update();
						} while (TheRecorder->isPlaybackInProgress());
					}
				}
			}
			else
#endif
			if( controlID == buttonLoadID )
			{
				if(listboxReplayFiles)
				{
					Int selected;
					GadgetListBoxGetSelected( listboxReplayFiles,  &selected );
					if(selected < 0)
					{
						MessageBoxOk(TheGameText->fetch("GUI:NoFileSelected"),TheGameText->fetch("GUI:PleaseSelectAFile"), NULL);
						break;
					}

					filename = GetReplayFilenameFromListbox(listboxReplayFiles, selected);
					loadReplay(filename);
				}
			}
			else if( controlID == buttonBackID )
			{

				// thou art directed to return to thy known solar system immediately!
				TheShell->pop();

			}
			else if( controlID == buttonDeleteID )
			{
				Int selected;
				GadgetListBoxGetSelected( listboxReplayFiles,  &selected );
				if(selected < 0)
				{
					MessageBoxOk(TheGameText->fetch("GUI:NoFileSelected"),TheGameText->fetch("GUI:PleaseSelectAFile"), NULL);
					break;
				}
				filename = GetReplayFilenameFromListbox(listboxReplayFiles, selected);
				MessageBoxYesNo(TheGameText->fetch("GUI:DeleteFile"), TheGameText->fetch("GUI:AreYouSureDelete"), deleteReplayFlag, NULL);
			}
			else if( controlID == buttonCopyID )
			{
				Int selected;
				GadgetListBoxGetSelected( listboxReplayFiles,  &selected );
				if(selected < 0)
				{
					MessageBoxOk(TheGameText->fetch("GUI:NoFileSelected"),TheGameText->fetch("GUI:PleaseSelectAFile"), NULL);
					break;
				}
				filename = GetReplayFilenameFromListbox(listboxReplayFiles, selected);
				MessageBoxYesNo(TheGameText->fetch("GUI:CopyReplay"), TheGameText->fetch("GUI:AreYouSureCopy"), copyReplayFlag, NULL);
			}
			break;
		}

		default:
			return MSG_IGNORED;
	}

	return MSG_HANDLED;
}

void deleteReplay( void )
{
	callDelete = FALSE;
	Int selected;
	GadgetListBoxGetSelected( listboxReplayFiles,  &selected );
	if(selected < 0)
	{
		MessageBoxOk(TheGameText->fetch("GUI:NoFileSelected"),TheGameText->fetch("GUI:PleaseSelectAFile"), NULL);
		return;
	}
	AsciiString filename, translate;
	filename = TheRecorder->getReplayDir();
	translate.translate(GetReplayFilenameFromListbox(listboxReplayFiles, selected));
	filename.concat(translate);
	if(DeleteFile(filename.str()) == 0)
	{
		char buffer[1024];
		FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buffer, sizeof(buffer), NULL);
		UnicodeString errorStr;
		translate.set(buffer);
		errorStr.translate(translate);
		MessageBoxOk(TheGameText->fetch("GUI:Error"),errorStr, NULL);
	}
	//Load the listbox shiznit
	GadgetListBoxReset(listboxReplayFiles);
	PopulateReplayFileListbox(listboxReplayFiles);
}


void copyReplay( void )
{
	callCopy = FALSE;
	Int selected;
	GadgetListBoxGetSelected( listboxReplayFiles,  &selected );
	if(selected < 0)
	{
		MessageBoxOk(TheGameText->fetch("GUI:NoFileSelected"),TheGameText->fetch("GUI:PleaseSelectAFile"), NULL);
		return;
	}
	AsciiString filename, translate;
	filename = TheRecorder->getReplayDir();
	translate.translate(GetReplayFilenameFromListbox(listboxReplayFiles, selected));
	filename.concat(translate);

	char path[1024];
	LPITEMIDLIST pidl;
	SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &pidl);
	SHGetPathFromIDList(pidl,path);
	AsciiString newFilename;
	newFilename.set(path);
	newFilename.concat("\\");
	newFilename.concat(translate);
	if(CopyFile(filename.str(),newFilename.str(), FALSE) == 0)
	{
		wchar_t buffer[1024];
		FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buffer, ARRAY_SIZE(buffer), NULL);
		UnicodeString errorStr;
		errorStr.set(buffer);
		errorStr.trim();
		MessageBoxOk(TheGameText->fetch("GUI:Error"),errorStr, NULL);
	}

}


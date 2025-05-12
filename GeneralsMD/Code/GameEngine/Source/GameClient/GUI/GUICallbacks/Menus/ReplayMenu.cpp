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

// FILE: ReplayMenu.cpp /////////////////////////////////////////////////////////////////////
// Author: Chris The masta Huybregts, December 2001
// Description: Replay Menus
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine


#include "Lib/BaseType.h"
#include "Common/FileSystem.h"
#include "Common/GameEngine.h"
#include "Common/GameState.h"
#include "Common/Recorder.h"
#include "Common/version.h"
#include "GameClient/GameClient.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/MessageBox.h"
#include "GameClient/MapUtil.h"
#include "GameClient/GameText.h"
#include "GameClient/GameWindowTransitions.h"
#include "GameLogic/GameLogic.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

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


#if 1
static GameWindow *buttonAnalyzeReplay = NULL;

// TheSuperHackers @feature helmutbuhler 13/04/2025
// Button to simulate replay without graphics
static GameWindow *buttonSimulateReplay = NULL;
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



void WriteOutReplayList()
{
	AsciiString fname;
	fname.format("%sreplay_list.csv", TheRecorder->getReplayDir().str());
	FILE *fp = fopen(fname.str(), "wt");
	if (!fp)
		return;

	// Get list of replay filenames
	AsciiString asciisearch;
	asciisearch = "*";
	asciisearch.concat(TheRecorder->getReplayExtention());
	FilenameList replayFilenamesSet;
	TheFileSystem->getFileListInDirectory(TheRecorder->getReplayDir(), asciisearch, replayFilenamesSet, FALSE);
	std::vector<AsciiString> replayFilenames;
	for (FilenameListIter it = replayFilenamesSet.begin(); it != replayFilenamesSet.end(); ++it)
	{
		replayFilenames.push_back(*it);
	}

	std::set<int> foundSeeds;

	// Print out a line per filename. i = -1 is csv header.
	for (int i = -1; i < (int)replayFilenames.size(); i++)
	{
		AsciiString filename;
		RecorderClass::ReplayHeader header;
		ReplayGameInfo info;
		const MapMetaData *md = NULL;
		if (i != -1)
		{
			filename.set(replayFilenames[i].reverseFind('\\') + 1);
			Bool success = GetMapInfo(filename, &header, &info, &md);
			if (!success)
				continue;
		}
		int numHumans = 0, numAIs = 0;
		for (int slot = 0; slot < MAX_SLOTS; slot++)
		{
			SlotState state = info.getSlot(slot)->getState();
			numHumans += state == SLOT_PLAYER ? 1 : 0;
			numAIs += state >= SLOT_EASY_AI && state <= SLOT_BRUTAL_AI ? 1 : 0;
		}

		bool compatibleVersion =
				header.versionString == UnicodeString(L"Version 1.4") ||
				header.versionString == UnicodeString(L"Version 1.04") ||
				header.versionString == UnicodeString(L"Version 1.05") ||
				header.versionString == UnicodeString(L"\x0412\x0435\x0440\x0441\x0438\x044f 1.04") ||
				header.versionString == UnicodeString(L"\x0412\x0435\x0440\x0441\x0438\x044f 1.05") ||
				header.versionString == UnicodeString(L"\x0412\x0435\x0440\x0441\x0438\x044f 1.4") ||
				header.versionString == UnicodeString(L"Versi\x00F3n 1.04") ||
				header.versionString == UnicodeString(L"Versi\x00F3n 1.05");

		// Some versions, e.g. "Zero Hour 1.04 The Ultimate Collection" have a different ini crc and are
		// actually incompatible. Mark them as incompatible
		compatibleVersion = compatibleVersion && header.iniCRC == 0xfeaae3f3;

		// Check whether random seed appears multiple times. This can be used to check only one replay
		// per game in case multiple replays by different players of the same game are in the list.
		int seed = info.getSeed();
		bool uniqueSeed = foundSeeds.find(seed) == foundSeeds.end();
		if (uniqueSeed)
			foundSeeds.insert(seed);

		// When a csv file is loaded with -simReplayList, check indicates whether the replay should be simulated.
		// If you want to check replays with certain properties, you can change this expression
		// or change the csv file manually.
		bool check = md && !header.desyncGame && header.endTime != 0 &&
			compatibleVersion && numHumans > 1 && numAIs > 0 && uniqueSeed;
		fprintf(fp, "%s", i == -1 ? "check" : check ? "1" : "0");

		if (i == -1)
			fprintf(fp, ",filename");
		else
			fprintf(fp, ",\"%s\"", filename.str());

		fprintf(fp, ",%s", i == -1 ? "map_exists" : md ? "1" : "0");
		fprintf(fp, ",%s", i == -1 ? "mismatch"   : header.desyncGame ? "1" : "0");
		fprintf(fp, ",%s", i == -1 ? "crash"      : header.endTime == 0 ? "1" : "0");
		//fprintf(fp, i == -1 ? ",frames" : ",%d",    header.frameDuration);

		UnsignedInt gameTime = header.frameDuration / LOGICFRAMES_PER_SECOND;
		fprintf(fp, i == -1 ? ",time" : ",%02d:%02d", gameTime/60, gameTime%60);

		fprintf(fp, i == -1 ? ",numHumans" : ",%d", numHumans);
		fprintf(fp, i == -1 ? ",numAIs" : ",%d",    numAIs);

		AsciiString tmp;
		tmp.translate(header.versionString);
		if (i == -1)
			fprintf(fp, ",version");
		else
			fprintf(fp, ",\"%s\"", tmp.str());
		//fprintf(fp, i == -1 ? ",exeCRC" : ",0x%08x",    header.exeCRC);
		//fprintf(fp, i == -1 ? ",iniCRC" : ",0x%08x",    header.iniCRC);

		fprintf(fp, ",%s", i == -1 ? "compatibleVersion" : compatibleVersion ? "1" : "0");

		//fprintf(fp, i == -1 ? ",crcInterval" : ",%d", info.getCRCInterval());
		//fprintf(fp, i == -1 ? ",seed" : ",0x%08x", seed);

		fprintf(fp, "\n");


#if 0
		if (i != -1 && check)
		{
			AsciiString sourceFilename = replayFilenames[i];
			
			AsciiString targetFilename;
			targetFilename = TheRecorder->getReplayDir();
			targetFilename.concat("filter/");
			targetFilename.concat(filename);

			CopyFile(sourceFilename.str(), targetFilename.str(), FALSE);
		}
#endif
	}
	fclose(fp);
}

bool ReadLineFromFile(FILE *fp, AsciiString *str)
{
	const int bufferSize = 128;
	char buffer[bufferSize];
	str->clear();
	while (true)
	{
		if (fgets(buffer, bufferSize, fp) == NULL)
		{
			str->clear();
			return false;
		}
		buffer[bufferSize-1] = 0; // Should be already nul-terminated, just to be sure
		str->concat(buffer);
		if (strlen(buffer) != bufferSize-1 || buffer[bufferSize-2] == '\n')
			break;
	}
	return true;
}

void NextToken(AsciiString *string, AsciiString *token, char separator)
{
	const char *tokenStart = string->str();

	const char *str = tokenStart;
	bool inQuotationMarks = false;
	while (*str)
	{
		if (*str == separator && !inQuotationMarks)
			break;
		if (*str == '\"')
			inQuotationMarks = !inQuotationMarks;
		str++;
	}
	const char *tokenEnd = str;

	Int len = tokenEnd - tokenStart;
	char *tmp = token->getBufferForRead(len + 1);
	memcpy(tmp, tokenStart, len);
	tmp[len] = 0;
	token->trim();

	string->set(*tokenEnd == 0 ? tokenEnd : tokenEnd+1);
}

void ReadReplayListFromCsv(AsciiString filename, std::vector<AsciiString>* replayList)
{
	// Get path of csv file relative to replay folder.
	// Later we will search for replays in that path.
	AsciiString relativeFolder = filename;
	{
		int len = relativeFolder.getLength();
		while (len)
		{
			char c = relativeFolder.getCharAt(len-1);
			if (c == '/' || c == '\\')
				break;
			relativeFolder.removeLastChar();
			len--;
		}
	}

	AsciiString fname;
	fname.format("%s%s", TheRecorder->getReplayDir().str(), filename.str());
	FILE *fp = fopen(fname.str(), "rt");
	if (!fp)
		return;

	// Parse header
	AsciiString line, token;
	ReadLineFromFile(fp, &line);
	char separator = line.find(';') == NULL ? ',' : ';';

	while (feof(fp) == 0)
	{
		ReadLineFromFile(fp, &line);

		// Parse check
		NextToken(&line, &token, separator);
		if (token != "1")
			continue;

		// Parse filename
		NextToken(&line, &token, separator);
		if (token.isEmpty())
			continue;
		if (token.getCharAt(0) == '\"' && token.getCharAt(token.getLength()-1) == '\"')
		{
			token.set(token.str()+1);
			token.removeLastChar();
		}
		if (!token.isEmpty())
		{
			AsciiString path;
			path.format("%s%s", relativeFolder.str(), token.str());
			replayList->push_back(path);
		}

		// Ignore remaining columns
	}
	fclose(fp);
}

//-------------------------------------------------------------------------------------------------
/** Populate the listbox with the names of the available replay files */
//-------------------------------------------------------------------------------------------------
void PopulateReplayFileListbox(GameWindow *listbox)
{
	if (!TheMapCache)
		return;
	
	GadgetListBoxReset(listbox);

	enum {
		COLOR_SP = 0,
		COLOR_SP_CRC_MISMATCH,
		COLOR_MP,
		COLOR_MP_CRC_MISMATCH,
		COLOR_MAX
	};
	Color colors[COLOR_MAX] = {
		GameMakeColor( 255, 255, 255, 255 ),
		GameMakeColor( 128, 128, 128, 255 ),
		GameMakeColor( 255, 255, 255, 255 ),
		GameMakeColor( 128, 128, 128, 255 )
	};

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

		// lets get some info about the replay
		RecorderClass::ReplayHeader header;
		header.forPlayback = FALSE;
		header.filename = asciistr;
		Bool success = TheRecorder && TheMapCache && TheRecorder->readReplayHeader( header );
		if (success)
		{
			ReplayGameInfo info;
			if (ParseAsciiStringToGameInfo( &info, header.gameOptions ))
			{

				// columns are: name, date, version, map, extra

				// name
				header.replayName.translate(asciistr);
				for (Int tmp=0; tmp < TheRecorder->getReplayExtention().getLength(); ++tmp)
					header.replayName.removeLastChar();

				UnicodeString replayNameToShow = header.replayName;

				AsciiString lastReplayFName = TheRecorder->getLastReplayFileName();
				lastReplayFName.concat(TheRecorder->getReplayExtention());
				if (lastReplayFName.compareNoCase(asciistr) == 0)
					replayNameToShow = TheGameText->fetch("GUI:LastReplay");

				UnicodeString displayTimeBuffer = getUnicodeTimeBuffer(header.timeVal);

				//displayTimeBuffer.format( L"%ls", timeBuffer);

				// version (no-op)

				// map
				UnicodeString mapStr;
				const MapMetaData *md = TheMapCache->findMap(info.getMap());
				if (!md)
				{
					// TheSuperHackers @bugfix helmutbuhler 08/03/2025 Just use the filename.
					// Displaying a long map path string would break the map list gui.
					const char* filename = info.getMap().reverseFind('\\');
					mapStr.translate(filename ? filename + 1 : info.getMap());
				}
				else
				{
					mapStr = md->m_displayName;
				}

//				// extra
//				UnicodeString extraStr;
//				if (header.localPlayerIndex >= 0)
//				{
//					// MP game
//					time_t totalSeconds = header.endTime - header.startTime;
//					Int mins = totalSeconds/60;
//					Int secs = totalSeconds%60;
//					Real fps = header.frameDuration/totalSeconds;
//					extraStr.format(L"%d:%d (%g fps) %hs", mins, secs, fps, header.desyncGame?"OOS ":"");
//
//					for (Int i=0; i<MAX_SLOTS; ++i)
//					{
//						const GameSlot *slot = info.getConstSlot(i);
//						if (slot && slot->isHuman())
//						{
//							if (i)
//								extraStr.concat(L", ");
//							if (header.playerDiscons[i])
//								extraStr.concat(L'*');
//							extraStr.concat(info.getConstSlot(i)->getName());
//						}
//					}
//				}
//				else
//				{
//					// solo game
//					time_t totalSeconds = header.endTime - header.startTime;
//					Int mins = totalSeconds/60;
//					Int secs = totalSeconds%60;
//					Real fps = header.frameDuration/totalSeconds;
//					extraStr.format(L"%d:%d (%g fps)", mins, secs, fps);
//				}

				// pick a color
				Color color;
				if (header.versionString == TheVersion->getUnicodeVersion() && header.versionNumber == TheVersion->getVersionNumber() &&
					header.exeCRC == TheGlobalData->m_exeCRC && header.iniCRC == TheGlobalData->m_iniCRC)
				{
					// good version
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
				}
				else
				{
					// bad version
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
				}
				
				Int insertionIndex = GadgetListBoxAddEntryText(listbox, replayNameToShow, color, -1, 0);
				GadgetListBoxAddEntryText(listbox, displayTimeBuffer, color, insertionIndex, 1);
				GadgetListBoxAddEntryText(listbox, header.versionString, color, insertionIndex, 2);
				GadgetListBoxAddEntryText(listbox, mapStr, color, insertionIndex, 3);
				//GadgetListBoxAddEntryText(listbox, extraStr, color, insertionIndex, 4);
			}
		}
	}
	GadgetListBoxSetSelected(listbox, 0);
	WriteOutReplayList();
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
	buttonDelete = TheWindowManager->winGetWindowFromId( parentReplayMenu, buttonDeleteID );
	buttonCopy = TheWindowManager->winGetWindowFromId( parentReplayMenu, buttonCopyID );

	//Load the listbox shiznit
	GadgetListBoxReset(listboxReplayFiles);
	PopulateReplayFileListbox(listboxReplayFiles);

#if 1
	WinInstanceData instData;
	instData.init();
	BitSet( instData.m_style, GWS_PUSH_BUTTON | GWS_MOUSE_TRACK );
	instData.m_textLabelString = "Debug: Analyze Replay";
	instData.setTooltipText(UnicodeString(L"Dump commands stored in selected replay into log"));
	buttonAnalyzeReplay = TheWindowManager->gogoGadgetPushButton( parentReplayMenu, 
																									 WIN_STATUS_ENABLED | WIN_STATUS_IMAGE, 
																									 4, 4, 
																									 180, 26, 
																									 &instData, NULL, TRUE );

	instData.m_id = 1;
	instData.m_textLabelString = "Debug: Simulate Replay";
	instData.setTooltipText(UnicodeString(L"Playback selected replay without graphics. Will block game until replay simulation is done!"));
	buttonSimulateReplay = TheWindowManager->gogoGadgetPushButton( parentReplayMenu, 
																									 WIN_STATUS_ENABLED | WIN_STATUS_IMAGE, 
																									 4, 40, 
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

}  // end ReplayMenuInit

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

	}  //end if

	// our shutdown is complete
	TheTransitionHandler->reverse("ReplayMenuFade");
	isShuttingDown = TRUE;
}  // end ReplayMenuShutdown

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

}  // end ReplayMenuUpdate

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

					}  // end if

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}  // end escape

			}  // end switch( key )

		}  // end char

	}  // end switch( msg )

	return MSG_IGNORED;

}  // end ReplayMenuInput

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

		}  // end create

		//---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{

			break;

		}  // end case

		// --------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			return MSG_HANDLED;

		}  // end input
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
						UnicodeString filename;
						filename = GetReplayFilenameFromListbox(listboxReplayFiles, rowSelected);

						AsciiString asciiFilename;
						asciiFilename.translate(filename);
						TheRecorder->playbackFile(asciiFilename);

						if(parentReplayMenu != NULL)
						{
							parentReplayMenu->winHide(TRUE);
						}
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

#if 1
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
						TheRecorder->stopAnalysis();
					}
				}
			}
			else if( controlID == buttonSimulateReplay->winGetWindowId() )
			{
				if(listboxReplayFiles)
				{
					Int selected;
					GadgetListBoxGetSelected( listboxReplayFiles,  &selected );
					if(selected < 0)
					{
						MessageBoxOk(UnicodeString(L"Blah Blah"),UnicodeString(L"Please select something munkee girl"), NULL);
						break;
					}

					filename = GetReplayFilenameFromListbox(listboxReplayFiles, selected);

					AsciiString asciiFilename;
					asciiFilename.translate(filename);
					if (TheRecorder->simulateReplay(asciiFilename))
					{
						do
						{
							TheGameClient->updateHeadless();
							TheGameLogic->UPDATE();
							if (TheRecorder->sawCRCMismatch())
								break;
						} while (TheRecorder->isPlaybackInProgress());
						if (TheGameLogic->isInGame())
							TheGameLogic->clearGameData();
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

					AsciiString asciiFilename;
					asciiFilename.translate(filename);

					if(TheRecorder->testVersionPlayback(asciiFilename))
					{
						MessageBoxOkCancel(TheGameText->fetch("GUI:OlderReplayVersionTitle"), TheGameText->fetch("GUI:OlderReplayVersion"),reallyLoadReplay ,NULL);
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
			}  // end else if
			else if( controlID == buttonBackID )
			{

				// thou art directed to return to thy known solar system immediately!
				TheShell->pop();

			}  // end else if
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
		}  // end selected

		default:
			return MSG_IGNORED;
	}  // end switch

	return MSG_HANDLED;
}  // end ReplayMenuSystem

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
		FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buffer, sizeof(buffer), NULL);
		UnicodeString errorStr;
		errorStr.set(buffer);
		errorStr.trim();
		MessageBoxOk(TheGameText->fetch("GUI:Error"),errorStr, NULL);
	}

}


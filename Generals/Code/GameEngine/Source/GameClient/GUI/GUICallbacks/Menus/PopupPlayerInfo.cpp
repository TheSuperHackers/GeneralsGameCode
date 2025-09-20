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

///////////////////////////////////////////////////////////////////////////////////////
// FILE: PopupPlayerInfo.cpp
// Author: Matthew D. Campbell, July 2002
// Description: Player info right-click popup screen
///////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/PlayerTemplate.h"
#include "Common/BattleHonors.h"
#include "Common/CustomMatchPreferences.h"
#include "Common/GameSpyMiscPreferences.h"
#include "Common/FileSystem.h"
#include "GameClient/Mouse.h"
#include "GameClient/GameText.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GadgetProgressBar.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/Display.h"
#include "GameClient/MessageBox.h"
#include "GameNetwork/GameSpyOverlay.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PersistentStorageDefs.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include "GameNetwork/RankPointValue.h"
#include "GameNetwork/GameSpy/BuddyThread.h"
#include "GameNetwork/GameSpy/GSConfig.h"
#include "GameNetwork/GameSpy/LobbyUtils.h"

#include "WWDownload/Registry.h"


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
// window ids ------------------------------------------------------------------------------
static NameKeyType parentID = NAMEKEY_INVALID;
static NameKeyType listboxInfoID = NAMEKEY_INVALID;
static NameKeyType buttonCloseID = NAMEKEY_INVALID;
static NameKeyType buttonBuddiesID = NAMEKEY_INVALID;
//static NameKeyType buttonOptionsID = NAMEKEY_INVALID;
static NameKeyType buttonSetLocaleID = NAMEKEY_INVALID;
static NameKeyType buttonDeleteAccountID = NAMEKEY_INVALID;
static NameKeyType checkBoxAsianFontID = NAMEKEY_INVALID;
static NameKeyType checkBoxNonAsianFontID = NAMEKEY_INVALID;

// Window Pointers ------------------------------------------------------------------------
static GameWindow *parent = NULL;
static GameWindow *listboxInfo = NULL;
static GameWindow *buttonClose = NULL;
static GameWindow *buttonBuddies = NULL;
//static GameWindow *buttonbuttonOptions = NULL;
static GameWindow *buttonSetLocale = NULL;
static GameWindow *buttonDeleteAccount = NULL;
static GameWindow *checkBoxAsianFont = NULL;
static GameWindow *checkBoxNonAsianFont = NULL;

static Bool isOverlayActive = false;
static Bool raiseMessageBox = false;
static Int lookAtPlayerID = 0;
static std::string lookAtPlayerName;


static const char *const rankNames[] = {
	"Private",
	"Corporal",
	"Sergeant",
	"Lieutenant",
	"Captain",
	"Major",
	"Colonel",
	"General",
	"Brigadier",
	"Commander",
};
static_assert(ARRAY_SIZE(rankNames) == MAX_RANKS, "Incorrect array size");


static const Image* lookupRankImage(AsciiString side, Int rank)
{
	if (side.isEmpty())
		return TheMappedImageCollection->findImageByName("NewPlayer");

	if (rank < 0 || rank >= MAX_RANKS)
		return NULL;

	// dirty hack rather than try to get artists to follow a naming convention
	if (side == "America")
		side = "_USA";
	else if (side == "China")
		side = "_China";
	else if (side == "GLA")
		side = "_GLA";
	else if (side == "Random")
		side = "Elite";

	AsciiString fullImageName;
	fullImageName.format("Rank_%s%s", rankNames[rank], side.str());
	if(strcmp(fullImageName.str(),"Rank_PrivateElite") == 0)
		fullImageName = "Rank";//_Private_Elite";
	const Image *img = TheMappedImageCollection->findImageByName(fullImageName);
	if (img)
	{
		DEBUG_LOG(("*** Loaded rank image '%s' from TheMappedImageCollection!", fullImageName.str()));
	}
	else
	{
		DEBUG_LOG(("*** Could not load rank image '%s' from TheMappedImageCollection!", fullImageName.str()));
	}
	return img;
}

static Int getTotalDisconnectsFromFile(Int playerID)
{
	Int retval = 0;
	if (playerID == 0)
	{
		return 0;
	}

	UserPreferences pref;
	AsciiString userPrefFilename;
	userPrefFilename.format("GeneralsOnline\\MiscPref%d.ini", playerID);
	DEBUG_LOG(("setPersistentDataCallback - reading stats from file %s", userPrefFilename.str()));
	pref.load(userPrefFilename);

	// if there is a file override, use that data instead.
	if (pref.find("0") != pref.end()) {
		retval = atoi(pref.find("0")->second.str());
	}
	if (pref.find("1") != pref.end()) {
		retval += atoi(pref.find("1")->second.str());
	}
	if (pref.find("2") != pref.end()) {
		retval += atoi(pref.find("2")->second.str());
	}
	if (pref.find("3") != pref.end()) {
		retval += atoi(pref.find("3")->second.str());
	}
	if (pref.find("4") != pref.end()) {
		retval += atoi(pref.find("4")->second.str());
	}
	if (pref.find("5") != pref.end()) {
		retval += atoi(pref.find("5")->second.str());
	}

	return retval;
}

Int GetAdditionalDisconnectsFromUserFile(Int playerID)
{
	Int retval = getTotalDisconnectsFromFile(playerID);

	if (playerID == 0) {
		return 0;
	}

	if (TheGameSpyInfo->getAdditionalDisconnects() > 0 && !retval)
	{
		DEBUG_LOG(("Clearing additional disconnects"));
		TheGameSpyInfo->clearAdditionalDisconnects();
	}

	if (TheGameSpyInfo->getAdditionalDisconnects() != -1)
	{
		return TheGameSpyInfo->getAdditionalDisconnects();
	}

	return retval;
}

void GetAdditionalDisconnectsFromUserFile(PSPlayerStats *stats)
{
	if (!stats || stats->id == 0) {
		return;
	}

	if (TheGameSpyInfo->getAdditionalDisconnects() > 0 && !getTotalDisconnectsFromFile(stats->id))
	{
		DEBUG_LOG(("Clearing additional disconnects"));
		TheGameSpyInfo->clearAdditionalDisconnects();
	}

	if (TheGameSpyInfo->getAdditionalDisconnects() < 1)
	{
		return;
	}

	UserPreferences pref;
	AsciiString userPrefFilename;
	userPrefFilename.format("GeneralsOnline\\MiscPref%d.ini", stats->id);
	DEBUG_LOG(("setPersistentDataCallback - reading stats from file %s", userPrefFilename.str()));
	pref.load(userPrefFilename);

	// if there is a file override, use that data instead.
	if (pref.find("0") != pref.end()) {
		stats->desyncs[2] += abs(atoi(pref.find("0")->second.str()));
	}
	if (pref.find("1") != pref.end()) {
		stats->desyncs[3] += abs(atoi(pref.find("1")->second.str()));
	}
	if (pref.find("2") != pref.end()) {
		stats->desyncs[4] += abs(atoi(pref.find("2")->second.str()));
	}
	if (pref.find("3") != pref.end()) {
		stats->discons[2] += abs(atoi(pref.find("3")->second.str()));
	}
	if (pref.find("4") != pref.end()) {
		stats->discons[3] += abs(atoi(pref.find("4")->second.str()));
	}
	if (pref.find("5") != pref.end()) {
		stats->discons[4] += abs(atoi(pref.find("5")->second.str()));
	}
}

// default values
RankPoints::RankPoints(void)
{
	m_ranks[RANK_PRIVATE]							= 0;
	m_ranks[RANK_CORPORAL]						= TheGameSpyConfig->getPointsForRank(RANK_CORPORAL); // 5
	m_ranks[RANK_SERGEANT]						= TheGameSpyConfig->getPointsForRank(RANK_SERGEANT); // 10
	m_ranks[RANK_LIEUTENANT]					= TheGameSpyConfig->getPointsForRank(RANK_LIEUTENANT); // 20
	m_ranks[RANK_CAPTAIN]							= TheGameSpyConfig->getPointsForRank(RANK_CAPTAIN); // 50
	m_ranks[RANK_MAJOR]								= TheGameSpyConfig->getPointsForRank(RANK_MAJOR); // 100
	m_ranks[RANK_COLONEL]							= TheGameSpyConfig->getPointsForRank(RANK_COLONEL); // 200
	m_ranks[RANK_BRIGADIER_GENERAL]		= TheGameSpyConfig->getPointsForRank(RANK_BRIGADIER_GENERAL); // 500
	m_ranks[RANK_GENERAL]							= TheGameSpyConfig->getPointsForRank(RANK_GENERAL); // 1000
	m_ranks[RANK_COMMANDER_IN_CHIEF]	= TheGameSpyConfig->getPointsForRank(RANK_COMMANDER_IN_CHIEF); // 2000

	m_winMultiplier = 3.0f;
	m_lostMultiplier = 0.0f;
	m_hourSpentOnlineMultiplier = 1.0f;
	m_completedSoloCampaigns = 5.0f;
	m_disconnectMultiplier = -1.0f;

#ifdef DEBUG_LOGGING
	AsciiStringList sidesList;
	ThePlayerTemplateStore->getAllSideStrings(&sidesList);
	for (AsciiStringList::iterator sit = sidesList.begin(); sit != sidesList.end(); ++sit)
	{
		for (Int i=0; i<MAX_RANKS; ++i)
			lookupRankImage(*sit, i);
	}
	for (Int i=0; i<MAX_RANKS; ++i)
		lookupRankImage("Random", i);
#endif
}

RankPoints *TheRankPointValues = NULL;

void SetLookAtPlayer( Int id, AsciiString nick)
{
	lookAtPlayerID = id;
	lookAtPlayerName = nick.str();
}

//	BATTLE_HONOR_LADDER_CHAMP		= 0x0000001,
//	BATTLE_HONOR_STREAK_3				= 0x0000002,
//	BATTLE_HONOR_STREAK_5				= 0x0000004,
//	BATTLE_HONOR_STREAK_10			= 0x0000008,
//	BATTLE_HONOR_STREAK_20			= 0x0000010,
//	BATTLE_HONOR_LOYALTY_USA		= 0x0000020,
//	BATTLE_HONOR_LOYALTY_CHINA	= 0x0000040,
//	BATTLE_HONOR_LOYALTY_GLA		= 0x0000060,
//	BATTLE_HONOR_BATTLE_TANK		= 0x0000080,
//	BATTLE_HONOR_AIR_WING				= 0x0000100,
//	BATTLE_HONOR_SPECIAL_FORCES	= 0x0000200,
//	BATTLE_HONOR_ENDURANCE			= 0x0000400,
//	BATTLE_HONOR_CAMPAIGN_USA		= 0x0000800,
//	BATTLE_HONOR_CAMPAIGN_CHINA	= 0x0001000,
//	BATTLE_HONOR_CAMPAIGN_GLA  	= 0x0002000,
//	BATTLE_HONOR_BLITZ5					= 0x0004000,
//	BATTLE_HONOR_BLITZ10				= 0x0008000,
//	BATTLE_HONOR_SOLO_USA_B			= 0x0010000,
//	BATTLE_HONOR_SOLO_USA_S			= 0x0020000,
//	BATTLE_HONOR_SOLO_USA_G			= 0x0040000,
//	BATTLE_HONOR_SOLO_CHINA_B		= 0x0080000,
//	BATTLE_HONOR_SOLO_CHINA_S		= 0x0100000,
//	BATTLE_HONOR_SOLO_CHINA_G		= 0x0200000,
//	BATTLE_HONOR_SOLO_GLA_B			= 0x0400000,
//	BATTLE_HONOR_SOLO_GLA_S			= 0x0800000,
//	BATTLE_HONOR_SOLO_GLA_G			= 0x1000000,

void BattleHonorTooltip(GameWindow *window,
												WinInstanceData *instData,
												UnsignedInt mouse)
{
	Int x, y, row, col;
	x = LOLONGTOSHORT(mouse);
	y = HILONGTOSHORT(mouse);

	GadgetListBoxGetEntryBasedOnXY(window, x, y, row, col);

	if (row == -1 || col == -1)
	{
		TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonors") );
		return;
	}

	Int battleHonor = (Int)GadgetListBoxGetItemData( window, row, col );
	if (battleHonor == 0)
	{
		//DEBUG_CRASH(("No Battle Honor in listbox row %d, col %d!", row, col));
		TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonors") );
		return;
	}
	Real tooltipWidth = 1.5f;
	if (BitIsSet(battleHonor, BATTLE_HONOR_NOT_GAINED))
	{
		if(BitIsSet(battleHonor, BATTLE_HONOR_STREAK_3))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorStreak3Disabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_LOYALTY_USA))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorLoyaltyUSADisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_LOYALTY_CHINA))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorLoyaltyChinaDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_LOYALTY_GLA))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorLoyaltyGLADisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_BATTLE_TANK))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorBattleTankDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_AIR_WING))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorAirWingDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_ENDURANCE))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorEnduranceDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_CAMPAIGN_USA))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorCampaignUSADisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_CAMPAIGN_CHINA))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorCampaignChinaDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_CAMPAIGN_GLA))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorCampaignGLADisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_BLITZ10))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorBlitz10Disabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_FAIR_PLAY))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorFairPlayDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_APOCALYPSE))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorApocalypseDisabled"), -1, NULL, tooltipWidth );
		/*
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_USA_B))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloUSABDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_USA_S))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloUSASDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_USA_G))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloUSAGDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_CHINA_B))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloChinaBDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_CHINA_S))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloChinaSDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_CHINA_G))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloChinaGDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_GLA_B))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloGLABDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_GLA_S))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloGLASDisabled"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_GLA_G))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloGLAGDisabled"), -1, NULL, tooltipWidth );
			*/
		else if(BitIsSet(battleHonor, BATTLE_HONOR_CHALLENGE))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorChallengeDisabled"), -1, NULL, tooltipWidth );
	}
	else
	{
		if(BitIsSet(battleHonor, BATTLE_HONOR_LADDER_CHAMP))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorLadderChamp"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_STREAK_3))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorStreak3"), -1, NULL, tooltipWidth );
		//else if(BitIsSet(battleHonor, BATTLE_HONOR_STREAK_5))
			//TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorStreak5"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_STREAK_10))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorStreak10"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_STREAK_25))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorStreak25"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_LOYALTY_USA))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorLoyaltyUSA"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_LOYALTY_CHINA))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorLoyaltyChina"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_LOYALTY_GLA))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorLoyaltyGLA"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_BATTLE_TANK))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorBattleTank"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_AIR_WING))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorAirWing"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_ENDURANCE))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorEndurance"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_CAMPAIGN_USA))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorCampaignUSA"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_CAMPAIGN_CHINA))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorCampaignChina"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_CAMPAIGN_GLA))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorCampaignGLA"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_BLITZ5))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorBlitz5"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_BLITZ10))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorBlitz10"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_FAIR_PLAY))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorFairPlay"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_APOCALYPSE))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorApocalypse"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_OFFICERSCLUB))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorOfficersClub"), -1, NULL, tooltipWidth );
		/*
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_USA_B))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloUSAB"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_USA_S))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloUSAS"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_USA_G))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloUSAG"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_CHINA_B))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloChinaB"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_CHINA_S))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloChinaS"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_CHINA_G))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloChinaG"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_GLA_B))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloGLAB"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_GLA_S))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloGLAS"), -1, NULL, tooltipWidth );
		else if(BitIsSet(battleHonor, BATTLE_HONOR_SOLO_GLA_G))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorSoloGLAG"), -1, NULL, tooltipWidth );
			*/
		else if(BitIsSet(battleHonor, BATTLE_HONOR_CHALLENGE))
			TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:BattleHonorChallenge"), -1, NULL, tooltipWidth );
	}

}

static Int rowsToSkip = 0;
void ResetBattleHonorInsertion(void)
{
	rowsToSkip = 0;
}
void InsertBattleHonor(GameWindow *list, const Image *image, Bool enabled, Int itemData, Int& row, Int& column, UnicodeString text = UnicodeString::TheEmptyString)
{
	Int width = MAX_BATTLE_HONOR_IMAGE_WIDTH * (TheDisplay->getWidth() / (Real)DEFAULT_DISPLAY_WIDTH);
	Int height = MAX_BATTLE_HONOR_IMAGE_HEIGHT * (TheDisplay->getHeight() / (Real)DEFAULT_DISPLAY_HEIGHT);

	static Int enabledColor = 0xFFFFFFFF;
	static Int disabledColor = GameMakeColor(80, 80, 80, 255);
	Int color;
	if (enabled)
		color = enabledColor;
	else
		color = disabledColor;

	if (!enabled)
		itemData |= BATTLE_HONOR_NOT_GAINED;

	GadgetListBoxAddEntryImage(list, image, row, column, height, width, TRUE, color);
	GadgetListBoxSetItemData(list, (void *)itemData, row, column );

	/*
	** removing text, since every place that adds text has alternate displays of the same thing
	if (!text.isEmpty())
	{
		GadgetListBoxAddEntryText(list, text, GameSpyColor[GSCOLOR_DEFAULT], row+1, column, TRUE );
		GadgetListBoxSetItemData(list, (void *)itemData, row+1, column );
		rowsToSkip++;
	}
	*/

	if(++column >= GadgetListBoxGetNumColumns(list))
	{
		column = 0;
		row = row + 1 + rowsToSkip;
		rowsToSkip = max(rowsToSkip-1, 0);
	}
}

static void populateBattleHonors(const PSPlayerStats& stats, Int battleHonors, Int gamesInRow, Int lastGen, Int challenge, GameWindow *list)
{
	if( !list )
		return;

	list->winSetTooltipFunc(BattleHonorTooltip);
	GadgetListBoxReset( list );
	Int column = 0;
	Int row = 0;

	Bool isFairPlayer = FALSE;
	Int numGames = 0;
	Int numDiscons = 0;
	PerGeneralMap::const_iterator it;
	for(it = stats.games.begin(); it != stats.games.end(); ++it)
	{
		numGames += it->second;
	}
	for(it = stats.discons.begin(); it != stats.discons.end(); ++it)
	{
		numDiscons += it->second;
	}
	for(it = stats.desyncs.begin(); it != stats.desyncs.end(); ++it)
	{
		numDiscons += it->second;
	}
	if (numGames >= 10 && numDiscons * 10 < numGames)
	{
		isFairPlayer = TRUE;
	}

	ResetBattleHonorInsertion();
	GadgetListBoxAddEntryImage(list, NULL, 0, 0, 10, 10, TRUE, GameMakeColor(255,255,255,255));
	row = 1;

	InsertBattleHonor(list, TheMappedImageCollection->findImageByName("FairPlay"), isFairPlayer,
		BATTLE_HONOR_FAIR_PLAY, row, column);

	InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorAirWing"), BitIsSet(battleHonors, BATTLE_HONOR_AIR_WING),
		BATTLE_HONOR_AIR_WING, row, column);
	InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorBattleTank"), BitIsSet(battleHonors, BATTLE_HONOR_BATTLE_TANK),
		BATTLE_HONOR_BATTLE_TANK, row, column);
	InsertBattleHonor(list, TheMappedImageCollection->findImageByName("Apocalypse"), BitIsSet(battleHonors, BATTLE_HONOR_APOCALYPSE),
		BATTLE_HONOR_APOCALYPSE, row, column);

	if (BitIsSet(battleHonors, BATTLE_HONOR_BLITZ5))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorBlitz5"), TRUE,
			BATTLE_HONOR_BLITZ5, row, column);
	}
	else if (BitIsSet(battleHonors, BATTLE_HONOR_BLITZ10))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorBlitz10"), TRUE,
			BATTLE_HONOR_BLITZ10, row, column);
	}
	else
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorBlitz10"), FALSE,
			BATTLE_HONOR_BLITZ10, row, column);
	}
	GadgetListBoxAddEntryImage(list, NULL, 2, 0, 10, 10, TRUE, GameMakeColor(255,255,255,255));
	row = 3;

	UnicodeString uStr;
	uStr.format(L"%10d", stats.maxWinsInARow);
	if(BitIsSet(battleHonors, BATTLE_HONOR_STREAK_25))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorStreak_G"), TRUE,
			BATTLE_HONOR_STREAK_25, row, column, uStr);
	}
	else if(BitIsSet(battleHonors, BATTLE_HONOR_STREAK_10))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorStreak_S"), TRUE,
			BATTLE_HONOR_STREAK_10, row, column, uStr);
	}
	else if(BitIsSet(battleHonors, BATTLE_HONOR_STREAK_3))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorStreak_B"), TRUE,
			BATTLE_HONOR_STREAK_3, row, column, uStr);
	}
	else
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorStreak_B"), FALSE,
			BATTLE_HONOR_STREAK_3, row, column);
	}

	/*
	Bool isLoyal;
	isLoyal = ThePlayerTemplateStore->getNthPlayerTemplate(lastGen)->getSide().compareNoCase( "america") == 0 && gamesInRow >= 20;
	InsertBattleHonor(list, TheMappedImageCollection->findImageByName("Loyalty_USA"), isLoyal,
		BATTLE_HONOR_LOYALTY_USA, row, column);
	isLoyal = ThePlayerTemplateStore->getNthPlayerTemplate(lastGen)->getSide().compareNoCase( "china") == 0 && gamesInRow >= 20;
	InsertBattleHonor(list, TheMappedImageCollection->findImageByName("Loyalty_China"), isLoyal,
		BATTLE_HONOR_LOYALTY_CHINA, row, column);
	isLoyal = ThePlayerTemplateStore->getNthPlayerTemplate(lastGen)->getSide().compareNoCase( "gla") == 0 && gamesInRow >= 20;
	InsertBattleHonor(list, TheMappedImageCollection->findImageByName("Loyalty_GLA"), isLoyal,
		BATTLE_HONOR_LOYALTY_GLA, row, column);
		*/


	//insertBattleHonor(list, TheMappedImageCollection->findImageByName("Endurance"), BitIsSet(battleHonors, BATTLE_HONOR_ENDURANCE),
		//BATTLE_HONOR_ENDURANCE, row, column);

	/*
	InsertBattleHonor(list, TheMappedImageCollection->findImageByName("Campaign_USA"), BitIsSet(battleHonors, BATTLE_HONOR_CAMPAIGN_USA),
		BATTLE_HONOR_CAMPAIGN_USA, row, column);
	InsertBattleHonor(list, TheMappedImageCollection->findImageByName("Campaign_China"), BitIsSet(battleHonors, BATTLE_HONOR_CAMPAIGN_CHINA),
		BATTLE_HONOR_CAMPAIGN_CHINA, row, column);
	InsertBattleHonor(list, TheMappedImageCollection->findImageByName("Campaign_GLA"), BitIsSet(battleHonors, BATTLE_HONOR_CAMPAIGN_GLA),
		BATTLE_HONOR_CAMPAIGN_GLA, row, column);
	*/

	/*
	if(BitIsSet(challenge, BH_CHALLENGE_MASK_7))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorChallenge7"), TRUE,
			BATTLE_HONOR_CHALLENGE, row, column);
	}
	else if (BitIsSet(challenge, BH_CHALLENGE_MASK_6))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorChallenge6"), TRUE,
			BATTLE_HONOR_CHALLENGE, row, column);
	}
	else if (BitIsSet(challenge, BH_CHALLENGE_MASK_5))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorChallenge5"), TRUE,
			BATTLE_HONOR_CHALLENGE, row, column);
	}
	else if (BitIsSet(challenge, BH_CHALLENGE_MASK_4))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorChallenge4"), TRUE,
			BATTLE_HONOR_CHALLENGE, row, column);
	}
	else if (BitIsSet(challenge, BH_CHALLENGE_MASK_3))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorChallenge3"), TRUE,
			BATTLE_HONOR_CHALLENGE, row, column);
	}
	else if (BitIsSet(challenge, BH_CHALLENGE_MASK_2))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorChallenge2"), TRUE,
			BATTLE_HONOR_CHALLENGE, row, column);
	}
	else if (BitIsSet(challenge, BH_CHALLENGE_MASK_1))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorChallenge1"), TRUE,
			BATTLE_HONOR_CHALLENGE, row, column);
	}
	else
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("HonorChallenge1"), FALSE,
			BATTLE_HONOR_CHALLENGE, row, column);
	}
	*/

	if (TheGameSpyInfo->didPlayerPreorder(stats.id))
	{
		InsertBattleHonor(list, TheMappedImageCollection->findImageByName("OfficersClub"), TRUE,
			BATTLE_HONOR_OFFICERSCLUB, row, column);
	}
}

Int GetFavoriteSide( const PSPlayerStats& stats )
{
	PerGeneralMap::const_iterator it;
	Int numGames = 0;
	Int favorite = 0;
	for(it =stats.games.begin(); it != stats.games.end(); ++it)
	{
		if(it->second >= numGames)
		{
			numGames = it->second;
			favorite = it->first;
		}
	}
	if(numGames == 0)
		return -1;
	else if( stats.gamesAsRandom >= numGames )
		return 0;

	return favorite;
}

Int CalculateRank( const PSPlayerStats& stats )
{
	if(stats.id == 0 || !TheRankPointValues)
		return 0;
	PerGeneralMap::const_iterator it;
	Int rankPoints = 0;
	Int numGames = 0;

	for(it =stats.wins.begin(); it != stats.wins.end(); ++it)
	{
		numGames += it->second;
	}
	rankPoints += (numGames * TheRankPointValues->m_winMultiplier);

	numGames = 0;
	for(it =stats.losses.begin(); it != stats.losses.end(); ++it)
	{
		numGames += it->second;
	}
	rankPoints += (numGames * TheRankPointValues->m_lostMultiplier);

	numGames = 0;
	for(it =stats.duration.begin(); it != stats.duration.end(); ++it)
	{
		numGames += it->second;
	}
	rankPoints += (numGames / 60) * TheRankPointValues->m_hourSpentOnlineMultiplier;

	numGames = 0;
	for(it =stats.discons.begin(); it != stats.discons.end(); ++it)
	{
		numGames += it->second;
	}
	for(it =stats.desyncs.begin(); it != stats.desyncs.end(); ++it)
	{
		numGames += it->second;
	}
	rankPoints += numGames * TheRankPointValues->m_disconnectMultiplier;

	if(BitIsSet(stats.battleHonors, BATTLE_HONOR_CAMPAIGN_USA | BATTLE_HONOR_CAMPAIGN_CHINA |BATTLE_HONOR_CAMPAIGN_GLA))
	{
		rankPoints += 1 * TheRankPointValues->m_completedSoloCampaigns;
	}

	rankPoints = max(0, rankPoints); // clip off negative values, since discons can push us below 0.

	return rankPoints;


}

static GameWindow* findWindow(GameWindow *parent, AsciiString baseWindow, AsciiString gadgetName)
{
	AsciiString fullPath;
	fullPath.format("%s:%s", baseWindow.str(), gadgetName.str());
	GameWindow *res = TheWindowManager->winGetWindowFromId(parent, NAMEKEY(fullPath));
	DEBUG_ASSERTLOG(res, ("Cannot find window %s", fullPath.str()));
	return res;
}

void PopulatePlayerInfoWindows( AsciiString parentWindowName )
{
	Int lookupID = TheGameSpyInfo->getLocalProfileID();
	if(parentWindowName == "PopupPlayerInfo.wnd")
	{
		lookupID = lookAtPlayerID;
		if (lookAtPlayerID <= 0 || !parent)
			return;
	}

	PSPlayerStats stats = TheGameSpyPSMessageQueue->findPlayerStatsByID(lookupID);

	Bool weHaveStats = (stats.id != 0);

	// if we don't have the stats from the server, see if we have cached stats
	if( !weHaveStats && lookupID == TheGameSpyInfo->getLocalProfileID() )
	{
		stats = TheGameSpyInfo->getCachedLocalPlayerStats();

		weHaveStats = TRUE;
	}

	Int currentRank = 0;
	Int rankPoints = CalculateRank(stats);
	Int i = 0;
	while( rankPoints >= TheRankPointValues->m_ranks[i + 1])
		++i;
	currentRank = i;

	PerGeneralMap::iterator it;
	Int numWins = 0;
	Int numLosses = 0;
	Int numDiscons = 0;
	Int numGames = 0;
	for(it =stats.wins.begin(); it != stats.wins.end(); ++it)
	{
		numWins += it->second;
	}
	for(it =stats.losses.begin(); it != stats.losses.end(); ++it)
	{
		numLosses += it->second;
	}
	for(it =stats.discons.begin(); it != stats.discons.end(); ++it)
	{
		numDiscons += it->second;
	}
	for(it =stats.desyncs.begin(); it != stats.desyncs.end(); ++it)
	{
		numDiscons += it->second;
	}

	numDiscons += GetAdditionalDisconnectsFromUserFile(lookupID);

	numGames = numWins + numLosses + numDiscons;

	GameWindow *win = NULL;
	UnicodeString uStr;
	win = findWindow(NULL, parentWindowName, "StaticTextPlayerStatisticsLabel");
	if(win)
	{
		AsciiString localeID = "WOL:Locale00";
		if (stats.locale >= LOC_MIN && stats.locale <= LOC_MAX)
			localeID.format("WOL:Locale%2.2d", stats.locale);
		uStr.format(TheGameText->fetch("GUI:PlayerStatistics"), lookAtPlayerName.c_str(), TheGameText->fetch(localeID).str());
		GadgetStaticTextSetText(win, uStr);
	}
	win = findWindow(NULL, parentWindowName, "StaticTextGamesPlayedValue");
	if(win)
	{
		uStr.format(L"%d", numGames);
		GadgetStaticTextSetText(win, uStr);
	}
	win = findWindow(NULL, parentWindowName, "StaticTextWinsValue");
	if(win)
	{
		uStr.format(L"%d", numWins);
		GadgetStaticTextSetText(win, uStr);
	}
	win = findWindow(NULL, parentWindowName, "StaticTextLossesValue");
	if(win)
	{
		uStr.format(L"%d", numLosses);
		GadgetStaticTextSetText(win, uStr);
	}
	win = findWindow(NULL, parentWindowName, "StaticTextDisconnectsValue");
	if(win)
	{
		uStr.format(L"%d", numDiscons);
		GadgetStaticTextSetText(win, uStr);
	}

	win = findWindow(NULL, parentWindowName, "StaticTextBestStreakValue");
	if (win)
	{
		uStr.format(L"%d", stats.maxWinsInARow);
		GadgetStaticTextSetText(win, uStr);
	}

	win = findWindow(NULL, parentWindowName, "StaticTextStreak");
	if (win)
	{
		if (stats.lossesInARow > 0)
		{
			GadgetStaticTextSetText(win, TheGameText->fetch("GUI:CurrentLossStreak"));
		}
		else
		{
			GadgetStaticTextSetText(win, TheGameText->fetch("GUI:CurrentWinStreak"));
		}
	}
	win = findWindow(NULL, parentWindowName, "StaticTextStreakValue");
	if(win)
	{
		Int streak = max(stats.lossesInARow, stats.winsInARow);
		uStr.format(L"%d", streak);
		GadgetStaticTextSetText(win, uStr);
	}

	AsciiString favoriteSide = "Random";
	win = findWindow(NULL, parentWindowName, "StaticTextFavoriteSideValue");
	{
		Int numGames = 0;
		Int favorite = 0;
		for(it =stats.games.begin(); it != stats.games.end(); ++it)
		{
			if(it->second >= numGames)
			{
				numGames = it->second;
				favorite = it->first;
			}
		}
		if(numGames == 0)
			GadgetStaticTextSetText(win, TheGameText->fetch("GUI:None"));
		else if( stats.gamesAsRandom >= numGames )
			GadgetStaticTextSetText(win, TheGameText->fetch("GUI:Random"));
		else
		{
			const PlayerTemplate *fac = ThePlayerTemplateStore->getNthPlayerTemplate(favorite);
			if (fac)
			{
				AsciiString side;
				side.format("SIDE:%s", fac->getSide().str());

				GadgetStaticTextSetText(win, TheGameText->fetch(side));

				favoriteSide = fac->getSide();
			}
		}
	}

	win = findWindow(NULL, parentWindowName, "StaticTextTotalKillsValue");
	if(win)
	{
		Int numGames = 0;
		for(it =stats.unitsKilled.begin(); it != stats.unitsKilled.end(); ++it)
		{
			numGames += it->second;
		}
		uStr.format(L"%d", numGames);
		GadgetStaticTextSetText(win, uStr);
	}
	win = findWindow(NULL, parentWindowName, "StaticTextTotalDeathsValue");
	if(win)
	{
		Int numGames = 0;
		for(it =stats.unitsLost.begin(); it != stats.unitsLost.end(); ++it)
		{
			numGames += it->second;
		}
		uStr.format(L"%d", numGames);
		GadgetStaticTextSetText(win, uStr);
	}
	win = findWindow(NULL, parentWindowName, "StaticTextTotalBuiltValue");
	if(win)
	{
		Int numGames = 0;
		for(it =stats.unitsBuilt.begin(); it != stats.unitsBuilt.end(); ++it)
		{
			numGames += it->second;
		}
		uStr.format(L"%d", numGames);
		GadgetStaticTextSetText(win, uStr);
	}
	win = findWindow(NULL, parentWindowName, "StaticTextBuildingsKilledValue");
	if(win)
	{
		Int numGames = 0;
		for(it =stats.buildingsKilled.begin(); it != stats.buildingsKilled.end(); ++it)
		{
			numGames += it->second;
		}
		uStr.format(L"%d", numGames);
		GadgetStaticTextSetText(win, uStr);
	}
	win = findWindow(NULL, parentWindowName, "StaticTextBuildingsLostValue");
	if(win)
	{
		Int numGames = 0;
		for(it =stats.buildingsLost.begin(); it != stats.buildingsLost.end(); ++it)
		{
			numGames += it->second;
		}
		uStr.format(L"%d", numGames);
		GadgetStaticTextSetText(win, uStr);
	}
	win = findWindow(NULL, parentWindowName, "StaticTextBuildingsBuiltValue");
	if(win)
	{
		Int numGames = 0;
		for(it =stats.buildingsBuilt.begin(); it != stats.buildingsBuilt.end(); ++it)
		{
			numGames += it->second;
		}
		uStr.format(L"%d", numGames);
		GadgetStaticTextSetText(win, uStr);
	}

	win = findWindow(NULL, parentWindowName, "StaticTextWinPercentValue");
	if(win)
	{
		uStr.format(TheGameText->fetch("GUI:WinPercent"), REAL_TO_INT(numWins/(Real)numGames*100.0f));
		GadgetStaticTextSetText(win, uStr);
	}

	win = findWindow(NULL, parentWindowName, "ProgressBarRank");
	if(win && TheRankPointValues)
	{
		if( currentRank == MAX_RANKS - 1)
		{
			// we've reached the max rank
			win->winHide(TRUE);
		}
		else
		{
			GadgetProgressBarSetProgress(win, 100 * INT_TO_REAL(rankPoints - TheRankPointValues->m_ranks[currentRank])/( TheRankPointValues->m_ranks[currentRank + 1] - TheRankPointValues->m_ranks[currentRank]));
		}
	}
	win = findWindow(NULL, parentWindowName, "WinRank");
	if(win && TheRankPointValues)
	{
		if (rankPoints == 0)
			win->winSetEnabledImage(0, lookupRankImage(AsciiString::TheEmptyString, 0));
		else
			win->winSetEnabledImage(0, lookupRankImage(favoriteSide, currentRank));
	}
	win = findWindow(NULL, parentWindowName, "StaticTextRank");
	if(win)
	{
		AsciiString rankStr;
		rankStr.format("GUI:GSRank%d", currentRank);
		GadgetStaticTextSetText(win, TheGameText->fetch(rankStr));
	}

	win = findWindow(NULL, parentWindowName, "StaticTextInProgress");
	if (win)
	{
		if (weHaveStats)
		{
			win->winHide(TRUE);
		}
		else
		{
			win->winHide(FALSE);
			GadgetStaticTextSetText(win, TheGameText->fetch("GUI:FetchingPlayerInfo"));
		}
	}

	win = findWindow(NULL, parentWindowName, "ListboxInfo");
	if(win)
	{
		populateBattleHonors(stats, stats.battleHonors,stats.gamesInRowWithLastGeneral,stats.lastGeneral,stats.challengeMedals, win);
	}
}



void HandlePersistentStorageResponses( void )
{
	if (TheGameSpyPSMessageQueue)
	{
		PSResponse resp;
		if (TheGameSpyPSMessageQueue->getResponse( resp ))
		{
			switch (resp.responseType)
			{
			case PSResponse::PSRESPONSE_COULDNOTCONNECT:
				{
					// message box & hide the window
					GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:PSCannotConnect"), NULL);
					GameSpyCloseOverlay(GSOVERLAY_PLAYERINFO);
				}
				break;
			case PSResponse::PSRESPONSE_PREORDER:
				{
					if (resp.preorder)
					{
						SetUnsignedIntInRegistry("", "Preorder", 1);
						TheGameSpyInfo->markPlayerAsPreorder( TheGameSpyInfo->getLocalProfileID() );

						// force an update of our shtuff
						PSResponse newResp;
						newResp.responseType = PSResponse::PSRESPONSE_PLAYERSTATS;
						newResp.player = TheGameSpyPSMessageQueue->findPlayerStatsByID(TheGameSpyInfo->getLocalProfileID());
						TheGameSpyPSMessageQueue->addResponse(newResp);
					}
				}
				break;
			case PSResponse::PSRESPONSE_PLAYERSTATS:
				{
					DEBUG_LOG(("LocalProfileID %d, resp.player.id %d, resp.player.locale %d", TheGameSpyInfo->getLocalProfileID(), resp.player.id, resp.player.locale));
					/*
					if(resp.player.id == TheGameSpyInfo->getLocalProfileID() && resp.player.locale < LOC_MIN)
					{
						if (!GameSpyIsOverlayOpen(GSOVERLAY_LOCALESELECT))
							GameSpyOpenOverlay(GSOVERLAY_LOCALESELECT);
					}
					else
					*/
					if (resp.player.id == TheGameSpyInfo->getLocalProfileID())
					{
						PeerRequest req;
						req.peerRequestType = PeerRequest::PEERREQUEST_PUSHSTATS;
						GameSpyMiscPreferences cPref;
						req.statsToPush.locale = cPref.getLocale();
						Int wins = 0, losses = 0;
						PerGeneralMap::const_iterator it;
						for (it = resp.player.wins.begin(); it != resp.player.wins.end(); ++it)
						{
							wins += it->second;
						}
						for (it = resp.player.losses.begin(); it != resp.player.losses.end(); ++it)
						{
							losses += it->second;
						}
						req.statsToPush.wins = wins;
						req.statsToPush.losses = losses;
						req.statsToPush.rankPoints = CalculateRank( resp.player );

						Int numGames = 0;
						Int favorite = 0;
						for(it =resp.player.games.begin(); it != resp.player.games.end(); ++it)
						{
							if(it->second >= numGames)
							{
								numGames = it->second;
								favorite = it->first;
							}
						}
						if(numGames == 0)
							req.statsToPush.side = 0;
						else if( resp.player.gamesAsRandom >= numGames )
							req.statsToPush.side = 0;
						else
							req.statsToPush.side = favorite;

						Bool isPreorder = TheGameSpyInfo->didPlayerPreorder( TheGameSpyInfo->getLocalProfileID() );
						req.statsToPush.preorder = isPreorder;

						DEBUG_LOG(("PEERREQUEST_PUSHSTATS: stats will be %d,%d,%d,%d,%d,%d",
							req.statsToPush.locale, req.statsToPush.wins, req.statsToPush.losses, req.statsToPush.rankPoints, req.statsToPush.side, req.statsToPush.preorder));
						TheGameSpyPeerMessageQueue->addRequest(req);
					}
					TheGameSpyPSMessageQueue->trackPlayerStats(resp.player);
					if (resp.player.id == TheGameSpyInfo->getLocalProfileID())
					{
						UpdateLocalPlayerStats();
					}
					DEBUG_LOG(("PopulatePlayerInfoWindows() - lookAtPlayerID is %d, got %d", lookAtPlayerID, resp.player.id));
					PopulatePlayerInfoWindows("PopupPlayerInfo.wnd");
					//GadgetListBoxAddEntryText(listboxInfo, UnicodeString(L"Got info!"), GameSpyColor[GSCOLOR_DEFAULT], -1);

					// also update info for player list in lobby
					PlayerInfoMap::iterator it = TheGameSpyInfo->getPlayerInfoMap()->begin();
					while (it != TheGameSpyInfo->getPlayerInfoMap()->end())
					{
						PlayerInfo *info = &(it->second);
						if (info && info->m_profileID == resp.player.id)
						{
							// update m_wins, m_losses, m_rankPoints
							Int wins = 0, losses = 0;
							PerGeneralMap::const_iterator it;
							for (it = resp.player.wins.begin(); it != resp.player.wins.end(); ++it)
							{
								wins += it->second;
							}
							for (it = resp.player.losses.begin(); it != resp.player.losses.end(); ++it)
							{
								losses += it->second;
							}
							info->m_wins = wins;
							info->m_losses = losses;
							info->m_rankPoints = CalculateRank( resp.player );
							Int numGames = 0;
							Int favorite = 0;
							for(it = resp.player.games.begin(); it != resp.player.games.end(); ++it)
							{
								if(it->second >= numGames)
								{
									numGames = it->second;
									favorite = it->first;
								}
							}
							if(numGames == 0)
								info->m_side = 0;
							else if( resp.player.gamesAsRandom >= numGames )
								info->m_side = 0;
							else
								info->m_side = favorite;

							PeerResponse r;
							r.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERINFO;
							r.nick = info->m_name.str();
							r.player.profileID = info->m_profileID;
							r.player.flags = info->m_flags;
							r.player.wins = info->m_wins;
							r.player.losses = info->m_losses;
							r.locale = info->m_locale.str();
							r.player.rankPoints = info->m_rankPoints;
							r.player.side = info->m_side;
							r.player.preorder = info->m_preorder;
							TheGameSpyPeerMessageQueue->addResponse(r);
							break;
						}
						++it;
					}

				}
				break;
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** Initialize the Overlay */
//-------------------------------------------------------------------------------------------------
void GameSpyPlayerInfoOverlayInit( WindowLayout *layout, void *userData )
{
	parentID = TheNameKeyGenerator->nameToKey( "PopupPlayerInfo.wnd:PopupParent" );
	buttonCloseID = TheNameKeyGenerator->nameToKey( "PopupPlayerInfo.wnd:ButtonClose" );
	buttonBuddiesID = TheNameKeyGenerator->nameToKey( "PopupPlayerInfo.wnd:ButtonCommunicator" );
	listboxInfoID = TheNameKeyGenerator->nameToKey( "PopupPlayerInfo.wnd:ListboxInfo" );
	//buttonOptionsID = TheNameKeyGenerator->nameToKey( "PopupPlayerInfo.wnd:ButtonOptions" );
	buttonSetLocaleID = TheNameKeyGenerator->nameToKey( "PopupPlayerInfo.wnd:ButtonSetLocale" );
	buttonDeleteAccountID = TheNameKeyGenerator->nameToKey( "PopupPlayerInfo.wnd:ButtonDeleteAccount" );
	checkBoxAsianFontID = TheNameKeyGenerator->nameToKey( "PopupPlayerInfo.wnd:CheckBoxAsianText" );
	checkBoxNonAsianFontID = TheNameKeyGenerator->nameToKey( "PopupPlayerInfo.wnd:CheckBoxNonAsianText" );

	parent = TheWindowManager->winGetWindowFromId( NULL, parentID );
	buttonClose = TheWindowManager->winGetWindowFromId( parent,  buttonCloseID);
	buttonBuddies = TheWindowManager->winGetWindowFromId( parent,  buttonBuddiesID);
	listboxInfo = TheWindowManager->winGetWindowFromId( parent,  listboxInfoID);
	//buttonbuttonOptions = TheWindowManager->winGetWindowFromId( parent,  buttonOptionsID);
	buttonSetLocale = TheWindowManager->winGetWindowFromId( parent,  buttonSetLocaleID);
	buttonDeleteAccount = TheWindowManager->winGetWindowFromId( parent,  buttonDeleteAccountID);
	checkBoxAsianFont = TheWindowManager->winGetWindowFromId( parent,  checkBoxAsianFontID);
	checkBoxNonAsianFont = TheWindowManager->winGetWindowFromId( parent,  checkBoxNonAsianFontID);

	// Show Menu
	layout->hide( FALSE );

	// Set Keyboard to Main Parent
	TheWindowManager->winSetFocus( parent );

	isOverlayActive = true;

	//GadgetListBoxAddEntryText(listboxInfo, UnicodeString(L"Working"), GameSpyColor[GSCOLOR_DEFAULT], -1);

	GameSpyCloseOverlay(GSOVERLAY_BUDDY);
	raiseMessageBox = true;
	PopulatePlayerInfoWindows("PopupPlayerInfo.wnd");

	// we're on the myinfo screen
	if(lookAtPlayerID == TheGameSpyInfo->getLocalProfileID())
	{
		//buttonbuttonOptions->winHide(FALSE);
		buttonSetLocale->winHide(FALSE);
		buttonDeleteAccount->winHide(TRUE); // set back to false when we have this worked out.
		checkBoxAsianFont->winHide(FALSE);
		checkBoxNonAsianFont->winHide(FALSE);
	}
	else
	{
		//buttonbuttonOptions->winHide(TRUE);
		buttonSetLocale->winHide(TRUE);
		buttonDeleteAccount->winHide(TRUE);
		checkBoxAsianFont->winHide(TRUE);
		checkBoxNonAsianFont->winHide(TRUE);
	}

	// set the asian check boxes
	CustomMatchPreferences pref;
	GadgetCheckBoxSetChecked(checkBoxAsianFont,!pref.getDisallowAsianText());
	GadgetCheckBoxSetChecked(checkBoxNonAsianFont,!pref.getDisallowNonAsianText());

	OSVERSIONINFO	osvi;
	osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osvi))
	{	//check if we're running Win9x variant since they may need different fonts
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			if (checkBoxAsianFont)
				checkBoxAsianFont->winEnable(FALSE);
			if (checkBoxNonAsianFont)
				checkBoxNonAsianFont->winEnable(FALSE);
		}
	}

	//TheWindowManager->winSetModal(parent);
}

//-------------------------------------------------------------------------------------------------
/** Overlay shutdown method */
//-------------------------------------------------------------------------------------------------
void GameSpyPlayerInfoOverlayShutdown( WindowLayout *layout, void *userData )
{
	// hide menu
	layout->hide( TRUE );

	parent = NULL;

	// our shutdown is complete
	isOverlayActive = false;
}


//-------------------------------------------------------------------------------------------------
/** Overlay update method */
//-------------------------------------------------------------------------------------------------
void GameSpyPlayerInfoOverlayUpdate( WindowLayout * layout, void *userData)
{
	if (raiseMessageBox)
		RaiseGSMessageBox();
	raiseMessageBox = false;
}

//-------------------------------------------------------------------------------------------------
/** Overlay input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType GameSpyPlayerInfoOverlayInput( GameWindow *window, UnsignedInt msg,
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
																							(WindowMsgData)buttonClose, buttonCloseID );

					}

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}

			}

		}

	}

	return MSG_IGNORED;
}
void messageBoxYes( void );
//-------------------------------------------------------------------------------------------------
/** Overlay window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType GameSpyPlayerInfoOverlaySystem( GameWindow *window, UnsignedInt msg,
														 WindowMsgData mData1, WindowMsgData mData2 )
{
	UnicodeString txtInput;

	switch( msg )
	{


		case GWM_CREATE:
			{

				break;
			}

		case GWM_DESTROY:
			{
				break;
			}

		case GWM_INPUT_FOCUS:
			{
				// if we're givin the opportunity to take the keyboard focus we must say we want it
				if( mData1 == TRUE )
					*(Bool *)mData2 = TRUE;

				return MSG_HANDLED;
			}
		case GBM_SELECTED:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if (controlID == buttonCloseID)
				{
					RefreshGameListBoxes();
					GameSpyCloseOverlay( GSOVERLAY_PLAYERINFO );
				}
				else if (controlID == buttonBuddiesID)
				{
					RefreshGameListBoxes();
					//GameSpyCloseOverlay( GSOVERLAY_PLAYERINFO );
					GameSpyOpenOverlay( GSOVERLAY_BUDDY );
				}
//				else if (controlID == buttonOptionsID)
//				{
//					RefreshGameListBoxes();
//					GameSpyCloseOverlay( GSOVERLAY_PLAYERINFO );
//					GameSpyOpenOverlay( GSOVERLAY_OPTIONS );
//				}
				else if (controlID == buttonSetLocaleID)
				{
					RefreshGameListBoxes();
					GameSpyCloseOverlay( GSOVERLAY_PLAYERINFO );
					if (!GameSpyIsOverlayOpen(GSOVERLAY_LOCALESELECT))
						GameSpyOpenOverlay( GSOVERLAY_LOCALESELECT );
					ReOpenPlayerInfo();
				}
				else if (controlID == buttonDeleteAccountID)
				{
					RefreshGameListBoxes();
					GameSpyCloseOverlay( GSOVERLAY_PLAYERINFO );
					MessageBoxYesNo(TheGameText->fetch("GUI:DeleteAccount"), TheGameText->fetch("GUI:AreYouSureDeleteAccount"),messageBoxYes, NULL);
				}
				else if (controlID == checkBoxAsianFontID)
				{
					Bool isChecked = !GadgetCheckBoxIsChecked(control);
					CustomMatchPreferences pref;
					pref.setDisallowAsianText(isChecked);
					pref.write();
					if(TheGameSpyInfo)
						TheGameSpyInfo->setDisallowAsianText(isChecked);
					if(isChecked && !GadgetCheckBoxIsChecked(checkBoxNonAsianFont))
					{
						GadgetCheckBoxSetChecked(checkBoxNonAsianFont, TRUE);
						CustomMatchPreferences pref;
						pref.setDisallowNonAsianText(FALSE);
						pref.write();
						if(TheGameSpyInfo)
							TheGameSpyInfo->setDisallowNonAsianText(FALSE);
					}

				}
				else if (controlID == checkBoxNonAsianFontID)
				{
					Bool isChecked = !GadgetCheckBoxIsChecked(control);
					CustomMatchPreferences pref;
					pref.setDisallowNonAsianText(isChecked);
					pref.write();
					if(TheGameSpyInfo)
						TheGameSpyInfo->setDisallowNonAsianText(isChecked);
					if(isChecked && !GadgetCheckBoxIsChecked(checkBoxAsianFont))
					{
						GadgetCheckBoxSetChecked(checkBoxAsianFont, TRUE);
						CustomMatchPreferences pref;
						pref.setDisallowAsianText(FALSE);
						pref.write();
						if(TheGameSpyInfo)
							TheGameSpyInfo->setDisallowAsianText(FALSE);
					}
				}

				break;
			}

		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;
}

static void messageBoxYes( void )
{
	BuddyRequest breq;
	breq.buddyRequestType = BuddyRequest::BUDDYREQUEST_DELETEACCT;
	TheGameSpyBuddyMessageQueue->addRequest( breq );
	TheGameSpyInfo->setLocalProfileID(0);

}

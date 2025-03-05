// TheSuperHackers @refactor @ShizCalev 04/05/2025 Switching a lot of strings to defines, first up: namekeys.

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

// This file contains all the header files that shouldn't change frequently.
// Be careful what you stick in here, because putting files that change often in here will 
// tend to cheese people's goats.

	// Factions
#define NAMEKEY_FactionAmerica	"FactionAmerica"
#define NAMEKEY_FactionAmericaAirForce	"FactionAmericaAirForce"
#define NAMEKEY_FactionAmericaChooseAGeneral	"FactionAmericaChooseAGeneral"
#define NAMEKEY_FactionAmericaSpecialForces	"FactionAmericaSpecialForces"
#define NAMEKEY_FactionAmericaTankCommand	"FactionAmericaTankCommand"
#define NAMEKEY_FactionChina	"FactionChina"
#define NAMEKEY_FactionChinaChooseAGeneral	"FactionChinaChooseAGeneral"
#define NAMEKEY_FactionChinaRedArmy	"FactionChinaRedArmy"
#define NAMEKEY_FactionChinaSecretPolice	"FactionChinaSecretPolice"
#define NAMEKEY_FactionChinaSpecialWeapons	"FactionChinaSpecialWeapons"
#define NAMEKEY_FactionCivilian	"FactionCivilian"
#define NAMEKEY_FactionFundamentalist "FactionFundamentalist" //GLA's placeholder name during initial development.
#define NAMEKEY_FactionGLA	"FactionGLA"
#define NAMEKEY_FactionGLABiowarCommand	"FactionGLABiowarCommand"
#define NAMEKEY_FactionGLAChooseAGeneral	"FactionGLAChooseAGeneral"
#define NAMEKEY_FactionGLATerrorCell	"FactionGLATerrorCell"
#define NAMEKEY_FactionGLAWarlordCommand	"FactionGLAWarlordCommand"
#define NAMEKEY_FactionObserver	"FactionObserver"
#define NAMEKEY_ThePlayer "ThePlayer"


	// Misc
#define NAMEKEY_Bogus "Bogus"
#define NAMEKEY_BoneFXDamage "BoneFXDamage"
#define NAMEKEY_ControlBar.wnd:ButtonPlaceBeacon "ControlBar.wnd:ButtonPlaceBeacon"
#define NAMEKEY_OverchargeBehavior "OverchargeBehavior"
#define NAMEKEY_PlyrCivilian "PlyrCivilian"
#define NAMEKEY_RailroadBehavior "RailroadBehavior"
#define NAMEKEY_SupplyCenterCreate "SupplyCenterCreate"


	// Upgrades
#define NAMEKEY_AutoDepositUpdate "AutoDepositUpdate"
#define NAMEKEY_BeaconClientUpdate "BeaconClientUpdate"
#define NAMEKEY_DeliverPayloadAIUpdate "DeliverPayloadAIUpdate"
#define NAMEKEY_DemoTrapUpdate "DemoTrapUpdate"
#define NAMEKEY_FireSpreadUpdate "FireSpreadUpdate"
#define NAMEKEY_FlammableUpdate "FlammableUpdate"
#define NAMEKEY_HackInternetAIUpdate "HackInternetAIUpdate"
#define NAMEKEY_PowerPlantUpdate "PowerPlantUpdate"
#define NAMEKEY_PowerPlantUpgrade "PowerPlantUpgrade"
#define NAMEKEY_RadarUpgrade "RadarUpgrade"
#define NAMEKEY_StealthUpdate "StealthUpdate"
#define NAMEKEY_SupplyCenterDockUpdate "SupplyCenterDockUpdate"
#define NAMEKEY_SupplyWarehouseDockUpdate "SupplyWarehouseDockUpdate"
#define NAMEKEY_SwayClientUpdate "SwayClientUpdate"

// Menus
#define NAMEKEY_PopupLadderSelect.wnd:StaticTextLadderName	"PopupLadderSelect.wnd:StaticTextLadderName"
#define NAMEKEY_PopupLadderSelect.wnd:PasswordParent	"PopupLadderSelect.wnd:PasswordParent"
#define NAMEKEY_PopupLadderSelect.wnd:PasswordEntry	"PopupLadderSelect.wnd:PasswordEntry"
#define NAMEKEY_PopupLadderSelect.wnd:Parent	"PopupLadderSelect.wnd:Parent"
#define NAMEKEY_PopupLadderSelect.wnd:ListBoxLadderSelect	"PopupLadderSelect.wnd:ListBoxLadderSelect"
#define NAMEKEY_PopupLadderSelect.wnd:ListBoxLadderDetails	"PopupLadderSelect.wnd:ListBoxLadderDetails"
#define NAMEKEY_PopupLadderSelect.wnd:ButtonPasswordOk	"PopupLadderSelect.wnd:ButtonPasswordOk"
#define NAMEKEY_PopupLadderSelect.wnd:ButtonPasswordCancel	"PopupLadderSelect.wnd:ButtonPasswordCancel"
#define NAMEKEY_PopupLadderSelect.wnd:ButtonOk	"PopupLadderSelect.wnd:ButtonOk"
#define NAMEKEY_PopupLadderSelect.wnd:ButtonCancel	"PopupLadderSelect.wnd:ButtonCancel"
#define NAMEKEY_PopupLadderSelect.wnd:ButtonBadPasswordOk	"PopupLadderSelect.wnd:ButtonBadPasswordOk"
#define NAMEKEY_PopupLadderSelect.wnd:BadPasswordParent	"PopupLadderSelect.wnd:BadPasswordParent"

#define NAMEKEY_SkirmishGameOptionsMenu.wnd:StaticTextWinsValue	"SkirmishGameOptionsMenu.wnd:StaticTextWinsValue"
#define NAMEKEY_SkirmishGameOptionsMenu.wnd:StaticTextStreakValue	"SkirmishGameOptionsMenu.wnd:StaticTextStreakValue"
#define NAMEKEY_SkirmishGameOptionsMenu.wnd:StaticTextLossesValue	"SkirmishGameOptionsMenu.wnd:StaticTextLossesValue"
#define NAMEKEY_SkirmishGameOptionsMenu.wnd:StaticTextBestStreakValue	"SkirmishGameOptionsMenu.wnd:StaticTextBestStreakValue"
#define NAMEKEY_SkirmishGameOptionsMenu.wnd:ListboxInfo	"SkirmishGameOptionsMenu.wnd:ListboxInfo"

#define NAMEKEY_WOLWelcomeMenu.wnd:WOLWelcomeMenuParent	"WOLWelcomeMenu.wnd:WOLWelcomeMenuParent"
#define NAMEKEY_WOLWelcomeMenu.wnd:StaticTextUSAToday	"WOLWelcomeMenu.wnd:StaticTextUSAToday"
#define NAMEKEY_WOLWelcomeMenu.wnd:StaticTextUSALastWeek	"WOLWelcomeMenu.wnd:StaticTextUSALastWeek"
#define NAMEKEY_WOLWelcomeMenu.wnd:StaticTextGLAToday	"WOLWelcomeMenu.wnd:StaticTextGLAToday"
#define NAMEKEY_WOLWelcomeMenu.wnd:StaticTextGLALastWeek	"WOLWelcomeMenu.wnd:StaticTextGLALastWeek"
#define NAMEKEY_WOLWelcomeMenu.wnd:StaticTextChinaToday	"WOLWelcomeMenu.wnd:StaticTextChinaToday"
#define NAMEKEY_WOLWelcomeMenu.wnd:StaticTextChinaLastWeek	"WOLWelcomeMenu.wnd:StaticTextChinaLastWeek"

#define NAMEKEY_ControlBar.wnd:BackgroundMarker	"ControlBar.wnd:BackgroundMarker"
#define NAMEKEY_ControlBar.wnd:ButtonGeneral	"ControlBar.wnd:ButtonGeneral"
#define NAMEKEY_ControlBar.wnd:ButtonIdleWorker	"ControlBar.wnd:ButtonIdleWorker"
#define NAMEKEY_ControlBar.wnd:ButtonLarge	"ControlBar.wnd:ButtonLarge"
#define NAMEKEY_ControlBar.wnd:ButtonOptions	"ControlBar.wnd:ButtonOptions"
#define NAMEKEY_ControlBar.wnd:ButtonPlaceBeacon "ControlBar.wnd:ButtonPlaceBeacon"
#define NAMEKEY_ControlBar.wnd:GeneralsExp	"ControlBar.wnd:GeneralsExp"
#define NAMEKEY_ControlBar.wnd:MoneyDisplay	"ControlBar.wnd:MoneyDisplay"
#define NAMEKEY_ControlBar.wnd:PowerWindow	"ControlBar.wnd:PowerWindow"
#define NAMEKEY_ControlBar.wnd:WinUAttack	"ControlBar.wnd:WinUAttack"

#define NAMEKEY_NonCommand_UpDown	"NonCommand_UpDown"
#define NAMEKEY_NonCommand_Options	"NonCommand_Options"
#define NAMEKEY_NonCommand_IdleWorker	"NonCommand_IdleWorker"
#define NAMEKEY_NonCommand_GeneralsExperience	"NonCommand_GeneralsExperience"
#define NAMEKEY_NonCommand_Beacon	"NonCommand_Beacon"
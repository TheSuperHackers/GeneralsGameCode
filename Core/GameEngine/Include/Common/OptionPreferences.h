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

///////////////////////////////////////////////////////////////////////////////////////
// FILE: OptionPreferences.h
// Author: Matthew D. Campbell, April 2002
// Description: Saving/Loading of option preferences
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/UserPreferences.h"

typedef UnsignedInt CursorCaptureMode;
typedef UnsignedInt ScreenEdgeScrollMode;

//-----------------------------------------------------------------------------
// OptionsPreferences options menu class
//-----------------------------------------------------------------------------
class OptionPreferences : public UserPreferences
{
public:
	OptionPreferences();
	virtual ~OptionPreferences();

	Bool loadFromIniFile();

	UnsignedInt getLANIPAddress(void);
	UnsignedInt getOnlineIPAddress(void);
	void setLANIPAddress(AsciiString IP);
	void setOnlineIPAddress(AsciiString IP);
	void setLANIPAddress(UnsignedInt IP);
	void setOnlineIPAddress(UnsignedInt IP);
	Bool getArchiveReplaysEnabled() const;
	Bool getAlternateMouseModeEnabled(void);
	Bool getRetaliationModeEnabled();
	Bool getDoubleClickAttackMoveEnabled(void);
	Real getScrollFactor(void);
	Bool getDrawScrollAnchor(void);
	Bool getMoveScrollAnchor(void);
	Bool getCursorCaptureEnabledInWindowedGame() const;
	Bool getCursorCaptureEnabledInWindowedMenu() const;
	Bool getCursorCaptureEnabledInFullscreenGame() const;
	Bool getCursorCaptureEnabledInFullscreenMenu() const;
	CursorCaptureMode getCursorCaptureMode() const;
	Bool getScreenEdgeScrollEnabledInWindowedApp() const;
	Bool getScreenEdgeScrollEnabledInFullscreenApp() const;
	ScreenEdgeScrollMode getScreenEdgeScrollMode() const;
	Bool getSendDelay(void);
	Int getFirewallBehavior(void);
	Short getFirewallPortAllocationDelta(void);
	UnsignedShort getFirewallPortOverride(void);
	Bool getFirewallNeedToRefresh(void);
	Bool usesSystemMapDir(void);
	AsciiString getPreferred3DProvider(void);
	AsciiString getSpeakerType(void);
	Real getSoundVolume(void);
	Real get3DSoundVolume(void);
	Real getSpeechVolume(void);
	Real getMusicVolume(void);
	Real getMoneyTransactionVolume(void) const;
	Bool saveCameraInReplays(void);
	Bool useCameraInReplays(void);
	Bool getPlayerObserverEnabled() const;
	Int getStaticGameDetail(void);
	Int getIdealStaticGameDetail(void);
	Real getGammaValue(void);
	Int getTextureReduction(void);
	void getResolution(Int *xres, Int *yres);
	Bool get3DShadowsEnabled(void);
	Bool get2DShadowsEnabled(void);
	Bool getCloudShadowsEnabled(void);
	Bool getLightmapEnabled(void);
	Bool getSmoothWaterEnabled(void);
	Bool getTreesEnabled(void);
	Bool getExtraAnimationsDisabled(void);
	Bool getUseHeatEffects(void);
	Bool getDynamicLODEnabled(void);
	Bool getFPSLimitEnabled(void);
	Bool getBuildingOcclusionEnabled(void);
	Int getParticleCap(void);

	Int getCampaignDifficulty(void);
	void setCampaignDifficulty(Int diff);

	Int getNetworkLatencyFontSize(void);
	Int getRenderFpsFontSize(void);
	Int getSystemTimeFontSize(void);
	Int getGameTimeFontSize(void);
	Int getPlayerInfoListFontSize(void);

	Real getResolutionFontAdjustment(void);

	Bool getShowMoneyPerMinute(void) const;
};

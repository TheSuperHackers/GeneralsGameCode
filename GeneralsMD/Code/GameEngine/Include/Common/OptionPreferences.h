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
// Description: Options menu preferences class
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/UserPreferences.h"

class Money;
typedef UnsignedInt CursorCaptureMode;
typedef UnsignedInt ScreenEdgeScrollMode;

class OptionPreferences : public UserPreferences
{
public:
	OptionPreferences(  );
	virtual ~OptionPreferences();

	Bool loadFromIniFile();

	UnsignedInt getLANIPAddress(void);				// convenience function
	UnsignedInt getOnlineIPAddress(void);			// convenience function
	void setLANIPAddress(AsciiString IP);			// convenience function
	void setOnlineIPAddress(AsciiString IP);	// convenience function
	void setLANIPAddress(UnsignedInt IP);			// convenience function
	void setOnlineIPAddress(UnsignedInt IP);	// convenience function
	Bool getArchiveReplaysEnabled() const;		// convenience function
	Bool getAlternateMouseModeEnabled(void);	// convenience function
	Bool getRetaliationModeEnabled();					// convenience function
	Bool getDoubleClickAttackMoveEnabled(void);	// convenience function
	Real getScrollFactor(void);								// convenience function
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
	Bool getSendDelay(void);									// convenience function
	Int getFirewallBehavior(void);						// convenience function
	Short getFirewallPortAllocationDelta(void);	// convenience function
	UnsignedShort getFirewallPortOverride(void); // convenience function
	Bool getFirewallNeedToRefresh(void);			// convenience function
	Bool usesSystemMapDir(void);							// convenience function
	AsciiString getPreferred3DProvider(void);	// convenience function
	AsciiString getSpeakerType(void);					// convenience function
	Real getSoundVolume(void);								// convenience function
	Real get3DSoundVolume(void);							// convenience function
	Real getSpeechVolume(void);								// convenience function
	Real getMusicVolume(void);								// convenience function
	Real getMoneyTransactionVolume(void) const;
	Bool saveCameraInReplays(void);
	Bool useCameraInReplays(void);
	Bool getPlayerObserverEnabled() const;
	Int	 getStaticGameDetail(void);	// detail level selected by the user.
	Int	 getIdealStaticGameDetail(void);	// detail level detected for user.
	Real getGammaValue(void);
	Int	 getTextureReduction(void);
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
	Bool getNoDynamicLODEnabled(void);
	Bool getBuildingOcclusionEnabled(void);
	Int getParticleCap(void);

	Int	 getCampaignDifficulty(void);
	void setCampaignDifficulty( Int diff );

	Int getNetworkLatencyFontSize(void);
	Int getRenderFpsFontSize(void);
	Int getSystemTimeFontSize(void);
	Int getGameTimeFontSize(void);

	Real getResolutionFontAdjustment(void);

	Bool getShowMoneyPerMinute(void) const;
};


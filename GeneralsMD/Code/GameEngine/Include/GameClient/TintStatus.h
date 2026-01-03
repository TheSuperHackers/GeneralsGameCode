
#pragma once
#ifndef __TINTSTATUS_H__
#define __TINTSTATUS_H__

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"
#include "Common/BitFlags.h"
#include "Common/BitFlagsIO.h"

// Tint status types can now be used via ini;
// Sync with TintStatusFlags::s_bitNameList[] in Drawable.cpp
enum TintStatus CPP_11(: Int)
{
	TINT_STATUS_INVALID = 0,

	TINT_STATUS_DISABLED = 1,///< drawable tint color is deathly dark grey
	TINT_STATUS_IRRADIATED,  ///< drawable tint color is sickly green
	TINT_STATUS_POISONED,  ///< drawable tint color is open-sore red
	TINT_STATUS_GAINING_SUBDUAL_DAMAGE,  ///< When gaining subdual damage, we tint SUBDUAL_DAMAGE_COLOR
	TINT_STATUS_FRENZY,  ///< When frenzied, we tint FRENZY_COLOR
	// New generic entries:
	TINT_STATUS_SHIELDED,  ///<  When shielded, we tint SHIELDED_COLOR
	TINT_STATUS_DEMORALIZED,
	TINT_STATUS_BOOST,
	TINT_STATUS_TELEPORT_RECOVER, ///< (Chrono Legionnaire -> recover from teleport)
	TINT_STATUS_DISABLED_CHRONO,  ///< Unit disabled by chrono gun
	TINT_STATUS_GAINING_CHRONO_DAMAGE,  ///< Unit getting damaged from chrono gun
	TINT_STATUS_FORCE_FIELD,
	TINT_STATUS_IRON_CURTAIN,
	TINT_STATUS_EXTRA1,
	TINT_STATUS_EXTRA2,
	TINT_STATUS_EXTRA3,
	TINT_STATUS_EXTRA4,
	TINT_STATUS_EXTRA5,
	TINT_STATUS_EXTRA6,
	TINT_STATUS_EXTRA7,
	TINT_STATUS_EXTRA8,
	TINT_STATUS_EXTRA9,
	TINT_STATUS_EXTRA10,

	TINT_STATUS_COUNT    // Keep this last
};

//-------------------
struct DrawableColorTint
{
	RGBColor color;
	RGBColor colorInfantry;
	UnsignedInt attackFrames;
	UnsignedInt decayFrames;
};

typedef BitFlags<TINT_STATUS_COUNT> TintStatusFlags;

// --------
#endif /* __TINTSTATUS_H__ */

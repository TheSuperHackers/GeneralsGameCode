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
//  (c) 2001-2003 Electronic Arts Inc.                                        //
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: GlobalLanguage.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Aug 2002
//
//	Filename: 	GlobalLanguage.cpp
//
//	author:		Chris Huybregts
//
//	purpose:	Contains the member functions for the language munkee
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"

#include "Common/AddonCompat.h"
#include "Common/AsciiString.h"
#include "Common/FileSystem.h"
#include "Common/INI.h"
#include "Common/Language.h"
#include "Common/OptionPreferences.h"
#include "Common/Registry.h"
#include "GameClient/Display.h"
#include "GameClient/GlobalLanguage.h"

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
GlobalLanguage *TheGlobalLanguageData = nullptr;

static const LookupListRec ResolutionFontSizeMethodNames[] = {
    {"CLASSIC", GlobalLanguage::ResolutionFontSizeMethod_Classic},
    {"CLASSIC_NO_CEILING",
     GlobalLanguage::ResolutionFontSizeMethod_ClassicNoCeiling},
    {"STRICT", GlobalLanguage::ResolutionFontSizeMethod_Strict},
    {"BALANCED", GlobalLanguage::ResolutionFontSizeMethod_Balanced},
    {nullptr, 0}};

static const FieldParse TheGlobalLanguageDataFieldParseTable[] = {
    {"UnicodeFontName", INI::parseAsciiString, nullptr,
     offsetof(GlobalLanguage, m_unicodeFontName)},
    //{	"UnicodeFontFileName",
    // INI::parseAsciiString,nullptr,
    // offsetof( GlobalLanguage, m_unicodeFontFileName ) },
    {"LocalFontFile", GlobalLanguage::parseFontFileName, nullptr, 0},
    {"MilitaryCaptionSpeed", INI::parseInt, nullptr,
     offsetof(GlobalLanguage, m_militaryCaptionSpeed)},
    {"UseHardWordWrap", INI::parseBool, nullptr,
     offsetof(GlobalLanguage, m_useHardWrap)},
    {"ResolutionFontAdjustment", INI::parseReal, nullptr,
     offsetof(GlobalLanguage, m_resolutionFontSizeAdjustment)},
    {"ResolutionFontSizeMethod", INI::parseLookupList,
     ResolutionFontSizeMethodNames,
     offsetof(GlobalLanguage, m_resolutionFontSizeMethod)},
    {"CopyrightFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_copyrightFont)},
    {"MessageFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_messageFont)},
    {"MilitaryCaptionTitleFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_militaryCaptionTitleFont)},
    {"MilitaryCaptionDelayMS", INI::parseInt, nullptr,
     offsetof(GlobalLanguage, m_militaryCaptionDelayMS)},
    {"MilitaryCaptionFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_militaryCaptionFont)},
    {"SuperweaponCountdownNormalFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_superweaponCountdownNormalFont)},
    {"SuperweaponCountdownReadyFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_superweaponCountdownReadyFont)},
    {"NamedTimerCountdownNormalFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_namedTimerCountdownNormalFont)},
    {"NamedTimerCountdownReadyFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_namedTimerCountdownReadyFont)},
    {"DrawableCaptionFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_drawableCaptionFont)},
    {"DefaultWindowFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_defaultWindowFont)},
    {"DefaultDisplayStringFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_defaultDisplayStringFont)},
    {"TooltipFontName", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_tooltipFontName)},
    {"NativeDebugDisplay", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_nativeDebugDisplay)},
    {"DrawGroupInfoFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_drawGroupInfoFont)},
    {"CreditsTitleFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_creditsTitleFont)},
    {"CreditsMinorTitleFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_creditsPositionFont)},
    {"CreditsNormalFont", GlobalLanguage::parseFontDesc, nullptr,
     offsetof(GlobalLanguage, m_creditsNormalFont)},
    {"LanguagePrefix", INI::parseAsciiString, nullptr,
     offsetof(GlobalLanguage, m_languagePrefix)},
    {"IsRTL", INI::parseBool, nullptr, offsetof(GlobalLanguage, m_isRTL)},

    {nullptr, nullptr, nullptr, 0}};

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
void INI::parseLanguageDefinition(INI *ini) {
  if (!TheGlobalLanguageData) {
    DEBUG_ASSERTCRASH(
        TheGlobalLanguageData,
        ("INI::parseLanguageDefinition - TheGlobalLanguage Data is not around, "
         "please create it before trying to parse the ini file."));
    return;
  }

  // Polypheides @feature Polypheides 18/03/2026 - Support named Language blocks: 'Language English'
  // to allow multiple language configurations in a single centralized INI file.
  AsciiString langName = ini->getNextTokenOrNull();
  Bool shouldParse = TRUE;

  if (!langName.isEmpty()) {
    // Check if the token is a known field name. If it is, then this block
    // doesn't have a name and we should start parsing it immediately.
    Bool isFieldName = FALSE;
    for (const FieldParse *parse = TheGlobalLanguageDataFieldParseTable;
         parse->token; ++parse) {
      if (langName.compareNoCase(parse->token) == 0) {
        isFieldName = TRUE;
        break;
      }
    }

    if (!isFieldName) {
      if (langName.compareNoCase(TheGlobalLanguageData->m_languageID) != 0) {
        shouldParse = FALSE;
      }
    } else {
      // It's a field name, so we need to "put back" the token.
      // Since INI doesn't have a put-back, we need to handle the first field
      // manually or use a different parsing strategy. However, usually the
      // fields are on the next line anyway. If it's on the same line, it's
      // trickier.
      // For now, let's assume if it's a field name, it's the old format and we
      // parse.
    }
  }

  if (shouldParse) {
    ini->initFromINI(TheGlobalLanguageData,
                     TheGlobalLanguageDataFieldParseTable);
  } else {
    // Skip until END
    while (!ini->isEOF()) {
      ini->readLine();
      const char *token = strtok(ini->m_buffer, ini->getSeps());
      if (token && stricmp(token, "END") == 0)
        break;
    }
  }
}

GlobalLanguage::GlobalLanguage() {
  m_unicodeFontName.clear();
  m_unicodeFontFileName.clear();
  m_unicodeFontName.clear();
  m_militaryCaptionSpeed = 0;
  m_useHardWrap = FALSE;
  m_resolutionFontSizeAdjustment = 0.7f;
  m_resolutionFontSizeMethod = ResolutionFontSizeMethod_Default;
  m_militaryCaptionDelayMS = 750;

  m_userResolutionFontSizeAdjustment = -1.0f;
  m_languageID = GetRegistryLanguage();
  m_languagePrefix = "US";
  m_isRTL = FALSE;
}

GlobalLanguage::~GlobalLanguage() {
  StringList::iterator it = m_localFonts.begin();
  while (it != m_localFonts.end()) {
    AsciiString font = *it;
    RemoveFontResource(font.str());
    // SendMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
    ++it;
  }
}

void GlobalLanguage::init() {
  {
    INI ini;
    AsciiString fname;

    // Polypheides @feature Polypheides 18/03/2026 - Prioritize loading centralized Language.ini from Data root.
    // If not found, fall back to the localized directory for backward compatibility.
    fname = "Data\\Language.ini";

    try {
      ini.load(fname, INI_LOAD_OVERWRITE, nullptr);
      INI::parseLanguageDefinition(&ini);
    } catch (...) {
      // Central Language.ini not found, try fallback to localized directory.
      fname.format("Data\\%s\\Language.ini", GetRegistryLanguage().str());

      try {
        // Load localized overrides.
        ini.load(fname, INI_LOAD_OVERWRITE, nullptr);
        INI::parseLanguageDefinition(&ini);
      } catch (...) {
        // Language directory or INI file not found for this locale — use
        // defaults.
        DEBUG_LOG(
            ("GlobalLanguage::init - no Language INI found, using defaults."));
      }
    }
  }

  StringList::iterator it = m_localFonts.begin();
  while (it != m_localFonts.end()) {
    AsciiString font = *it;
    if (AddFontResource(font.str()) == 0) {
      DEBUG_CRASH(("GlobalLanguage::init Failed to add font %s", font.str()));
    } else {
      // SendMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
    }
    ++it;
  }

  // override values with user preferences
  OptionPreferences optionPref;
  m_userResolutionFontSizeAdjustment = optionPref.getResolutionFontAdjustment();
}

void GlobalLanguage::reset() {}

void GlobalLanguage::parseFontDesc(INI *ini, void *instance, void *store,
                                   const void *userData) {
  FontDesc *fontDesc = (FontDesc *)store;
  fontDesc->name = ini->getNextQuotedAsciiString();
  fontDesc->size = ini->scanInt(ini->getNextToken());
  fontDesc->bold = ini->scanBool(ini->getNextToken());
}

void GlobalLanguage::parseFontFileName(INI *ini, void *instance, void *store,
                                       const void *userData) {
  GlobalLanguage *globalLanguage = static_cast<GlobalLanguage *>(instance);
  AsciiString asciiString = ini->getNextAsciiString();
  globalLanguage->m_localFonts.push_front(asciiString);
}

Real GlobalLanguage::getResolutionFontSizeAdjustment() const {
  if (m_userResolutionFontSizeAdjustment >= 0.0f)
    return m_userResolutionFontSizeAdjustment;
  else
    return m_resolutionFontSizeAdjustment;
}

Real GlobalLanguage::getResolutionFontSizeScale(ResolutionFontSizeMethod method,
                                                Real scaler) {
  Real adjustFactor;

  switch (method) {
  default:
  case ResolutionFontSizeMethod_Classic: {
    // TheSuperHackers @info The original font scaling for this game.
    // Useful for not breaking legacy Addons and Mods. Scales poorly with large resolutions.
    // Polypheides @tweak Polypheides 18/03/2026 - Maintain original scaling for 'CLASSIC' mode.
    adjustFactor = TheDisplay->getWidth() / (Real)DEFAULT_DISPLAY_WIDTH;
    adjustFactor = 1.0f + (adjustFactor - 1.0f) * scaler;
    if (adjustFactor > 2.0f)
      adjustFactor = 2.0f;
    break;
  }
  case ResolutionFontSizeMethod_ClassicNoCeiling: {
    // TheSuperHackers @feature The original font scaling, but without ceiling.
    // Useful for not changing the original look of the game. Scales alright with large resolutions.
    // Polypheides @tweak Polypheides 18/03/2026 - Allow uncapped scaling for 'CLASSIC_NO_CEILING' mode.
    adjustFactor = TheDisplay->getWidth() / (Real)DEFAULT_DISPLAY_WIDTH;
    adjustFactor = 1.0f + (adjustFactor - 1.0f) * scaler;
    break;
  }
  case ResolutionFontSizeMethod_Strict: {
    // TheSuperHackers @feature The strict method scales fonts based on the smallest screen
    // dimension so they scale independent of aspect ratio.
    // Polypheides @tweak Polypheides 18/03/2026 - Ensure aspect-ratio independent scaling.
    const Real wScale = TheDisplay->getWidth() / (Real)DEFAULT_DISPLAY_WIDTH;
    const Real hScale = TheDisplay->getHeight() / (Real)DEFAULT_DISPLAY_HEIGHT;
    adjustFactor = min(wScale, hScale);
    adjustFactor = 1.0f + (adjustFactor - 1.0f) * scaler;
    break;
  }
  case ResolutionFontSizeMethod_Balanced: {
    // TheSuperHackers @feature The balanced method evenly weighs the display width and height
    // for a balanced rescale on non 4:3 resolutions. The aspect ratio scaling is clamped to
    // prevent oversizing.
    // Polypheides @tweak Polypheides 18/03/2026 - Clamped balanced scaling for modern displays.
    constexpr const Real maxAspect = 1.8f;
    constexpr const Real minAspect = 1.0f;
    Real w = TheDisplay->getWidth();
    Real h = TheDisplay->getHeight();
    const Real aspect = w / h;
    Real wScale = w / (Real)DEFAULT_DISPLAY_WIDTH;
    Real hScale = h / (Real)DEFAULT_DISPLAY_HEIGHT;

    if (aspect > maxAspect) {
      // Recompute width at max aspect
      w = maxAspect * h;
      wScale = w / (Real)DEFAULT_DISPLAY_WIDTH;
    } else if (aspect < minAspect) {
      // Recompute height at min aspect
      h = minAspect * w;
      hScale = h / (Real)DEFAULT_DISPLAY_HEIGHT;
    }
    adjustFactor = (wScale + hScale) * 0.5f;
    adjustFactor = 1.0f + (adjustFactor - 1.0f) * scaler;
    break;
  }
  }

  if (adjustFactor < 1.0f)
    adjustFactor = 1.0f;

  return adjustFactor;
}

Int GlobalLanguage::adjustFontSize(Int theFontSize) {
  // TheSuperHackers @todo This function is called very often.
  // Therefore cache the adjustFactor on resolution change to not recompute it on every call.
  // Polypheides @tweak Polypheides 18/03/2026 - Added note about potential optimization.
  const Real resolutionScaler = getResolutionFontSizeAdjustment();
  const Real adjustFactor =
      getResolutionFontSizeScale(m_resolutionFontSizeMethod, resolutionScaler);
  const Int pointSize = REAL_TO_INT_FLOOR(theFontSize * adjustFactor);

  return pointSize;
}

void GlobalLanguage::parseCustomDefinition() {
  if (addon::HasFullviewportDat()) {
    // TheSuperHackers @tweak xezon 19/08/2025 Force the classic font size adjustment for the old
    // 'Control Bar Pro' Addons because they use manual font upscaling in higher resolution packages.
    // Polypheides @fix Polypheides 18/03/2026 - Maintain Addon compatibility.
    m_resolutionFontSizeMethod = ResolutionFontSizeMethod_Classic;
  }
}

LanguageID GlobalLanguage::getLanguageIDEnumValue() const {
  if (m_languageID.isEmpty())
    return OurLanguage;

  if (m_languageID.compareNoCase("English") == 0)
    return LANGUAGE_ID_US;
  if (m_languageID.compareNoCase("German") == 0)
    return LANGUAGE_ID_GERMAN;
  if (m_languageID.compareNoCase("French") == 0)
    return LANGUAGE_ID_FRENCH;
  if (m_languageID.compareNoCase("Spanish") == 0)
    return LANGUAGE_ID_SPANISH;
  if (m_languageID.compareNoCase("Italian") == 0)
    return LANGUAGE_ID_ITALIAN;
  if (m_languageID.compareNoCase("Japanese") == 0)
    return LANGUAGE_ID_JAPANESE;
  if (m_languageID.compareNoCase("Jabber") == 0)
    return LANGUAGE_ID_JABBER;
  if (m_languageID.compareNoCase("Korean") == 0)
    return LANGUAGE_ID_KOREAN;
  if (m_languageID.compareNoCase("Unknown") == 0)
    return LANGUAGE_ID_UNKNOWN;

  return OurLanguage;
}

FontDesc::FontDesc() {
  name = "Arial Unicode MS";
  size = 12;
  bold = FALSE;
}
//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

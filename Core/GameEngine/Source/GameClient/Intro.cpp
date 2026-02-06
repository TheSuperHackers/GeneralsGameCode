/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#include "PreRTS.h"
#include "GameClient/Intro.h"

#include "Common/FramePacer.h"
#include "Common/GameLOD.h"

#include "GameClient/Display.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/GameText.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GlobalLanguage.h"
#include "GameClient/Image.h"


Intro::DisplayEntity::~DisplayEntity()
{
	TheDisplayStringManager->freeDisplayString(displayString);
}

Intro::Intro()
	: m_currentState(IntroState_Start)
	, m_allowedStateFlags(0)
	, m_waitUntilMs(0)
	, m_fadeValue(0.0f)
{
	if (TheGlobalData->m_playIntro)
	{
		// Please be kind and do not remove the EA logo movie. This game was built by EA. It lasts just 3 seconds.
		m_allowedStateFlags |= 1u << IntroState_EALogoMovie;
		// Please be kind and do not remove the custom logo screen when building on top of the work of The Super Hackers.
		m_allowedStateFlags |= 1u << IntroState_TheSuperHackersWait;
		m_allowedStateFlags |= 1u << IntroState_TheSuperHackers;
	}

	if (TheGlobalData->m_playSizzle)
	{
		if (TheGlobalData->m_playIntro)
			m_allowedStateFlags |= 1u << IntroState_SizzleMovieWait;
		m_allowedStateFlags |= 1u << IntroState_SizzleMovie;
	}
}

void Intro::enterNextState()
{
	Int currentState = m_currentState;
	while (currentState < IntroState_Done)
	{
		++currentState;
		if (m_allowedStateFlags & (1u << currentState))
			break;
	}
	m_currentState = static_cast<IntroState>(currentState);
}

void Intro::update()
{
	if (!TheDisplay->isMoviePlaying() && m_waitUntilMs < timeGetTime())
	{
		enterNextState();

		switch (m_currentState)
		{
		case IntroState_EALogoMovie: doEALogoMovie(); break;
		case IntroState_TheSuperHackersWait: doAsyncWait(800); break;
		case IntroState_TheSuperHackers: doTheSuperHackers(); break;
		case IntroState_SizzleMovieWait: doAsyncWait(1000); break;
		case IntroState_SizzleMovie: doSizzleMovie(); break;
		case IntroState_Done: doPostIntro(); break;
		}
	}
}

void Intro::draw()
{
	switch (m_currentState)
	{
		case IntroState_TheSuperHackers: drawDisplayEntities(); break;
	}
}

void Intro::doEALogoMovie()
{
	TheWritableGlobalData->m_allowExitOutOfMovies = FALSE;
	if (TheGameLODManager && TheGameLODManager->didMemPass())
		TheDisplay->playMovie("EALogoMovie");
	else
		TheDisplay->playMovie("EALogoMovie640");
}

struct DisplaySetting
{
	DisplaySetting()
		: imageName(nullptr)
		, text(nullptr)
		, screenHeightFactor(0.5f)
		, sizeX(10)
		, sizeY(10)
		, bold(false)
		, centered(false)
	{}

	const Char* imageName;
	const WideChar* text;
	Real screenHeightFactor;
	Int sizeX;
	Int sizeY;
	Bool bold;
	Bool centered;
};

void Intro::doTheSuperHackers()
{
	std::vector<DisplaySetting> settings;

	constexpr const Real startY = 0.35f;

	{
		// Pretext
		m_unicodeStrings[0] = TheGameText->FETCH_OR_SUBSTITUTE("CREDITS:CustomIntroPretext", L"Game improved by");
		DisplaySetting setting;
		setting.text = m_unicodeStrings[0].str();
		setting.screenHeightFactor = startY;
		setting.sizeY = 16;
		setting.bold = false;
		setting.centered = true;
		settings.push_back(setting);
	}
	{
		// Team name
		DisplaySetting setting;
		setting.text = L"The Super Hackers";
		setting.screenHeightFactor = startY + 0.05f;
		setting.sizeY = 32;
		setting.bold = true;
		setting.centered = true;
		settings.push_back(setting);
	}
	{
		// Website
		DisplaySetting setting;
		setting.text = L"www.thesuperhackers.org";
		setting.screenHeightFactor = startY + 0.20f;
		setting.sizeY = 16;
		setting.bold = false;
		setting.centered = true;
		settings.push_back(setting);
	}
	{
		// China Hacker image
		DisplaySetting setting;
		setting.imageName = "SNHacker2_L";
		setting.screenHeightFactor = startY + 0.27f;
		setting.sizeX = (Int)(122 * 0.50f);
		setting.sizeY = (Int)(98 * 0.50f);
		settings.push_back(setting);
	}
	if constexpr (false)
	{
		// Code contributors with 10 or more commits to main branch.
		// Sort by alphabet or commit count.
		DisplaySetting setting;
		setting.text =
			L"bobtista "
			L"Caball009 "
			L"feliwir "
			L"helmutbuhler\n"
			L"JohnsterID "
			L"Mauller "
			L"OmniBlade "
			L"SkyAero\n"
			L"slurmlord "
			L"Stubbjax "
			L"tomsons26 "
			L"xezon\n";
		setting.screenHeightFactor = 0.55f;
		setting.sizeY = 12;
		setting.bold = false;
		setting.centered = true;
		settings.push_back(setting);
	}

	const Real fontScale = GlobalLanguage::getResolutionFontSizeScale(GlobalLanguage::ResolutionFontSizeMethod_Strict);

	m_displayEntities.resize(settings.size());

	for (size_t i = 0; i < m_displayEntities.size(); ++i)
	{
		const DisplaySetting& s = settings[i];
		DisplayEntity& e = m_displayEntities[i];
		e.sizeX = Int(s.sizeX * fontScale);
		e.sizeY = Int(s.sizeY * fontScale);
		if (s.text != nullptr)
		{
			e.displayString = TheDisplayStringManager->newDisplayString();
			e.displayString->setText(s.text);
			// Consolas is Windows Vista native, can be installed in 2000, XP
			e.displayString->setFont(TheFontLibrary->getFont("Consolas", e.sizeY, s.bold));
			e.displayString->setWordWrapCentered(s.centered);
		}
		if (s.imageName != nullptr)
		{
			e.image = TheMappedImageCollection->findImageByName(s.imageName);
		}
		e.screenHeightFactor = s.screenHeightFactor;
	}

	doAsyncWait(3000);
	m_fadeValue = 0.0f;
}

void Intro::doSizzleMovie()
{
	TheWritableGlobalData->m_allowExitOutOfMovies = TRUE;
	if (TheGameLODManager && TheGameLODManager->didMemPass())
		TheDisplay->playMovie("Sizzle");
	else
		TheDisplay->playMovie("Sizzle640");
}

void Intro::doPostIntro()
{
	TheWritableGlobalData->m_breakTheMovie = TRUE;
	TheWritableGlobalData->m_allowExitOutOfMovies = TRUE;
}

void Intro::doAsyncWait(UnsignedInt milliseconds)
{
	m_waitUntilMs = timeGetTime() + milliseconds;
}

void Intro::drawDisplayEntities()
{
	const Real alpha = 255.0f * clamp(0.0f, m_fadeValue, 1.0f);
	const Color color = GameMakeColor(255, 255, 255, (Int)alpha);
	const Color dropColor = GameMakeColor(255, 255, 255, 0);
	const Int screenWidth = TheDisplay->getWidth();
	const Int screenHeight = TheDisplay->getHeight();

	for (size_t i = 0; i < m_displayEntities.size(); ++i)
	{
		DisplayEntity& e = m_displayEntities[i];
		if (e.displayString != nullptr)
		{
			ICoord2D pos;
			e.displayString->getSize(&pos.x, &pos.y);
			pos.x = (screenWidth / 2) - (pos.x / 2);
			pos.y = (Int)(e.screenHeightFactor * screenHeight);
			e.displayString->setWordWrap(screenWidth);
			e.displayString->draw(pos.x, pos.y, color, dropColor);
		}
		if (e.image != nullptr)
		{
			IRegion2D region;
			region.lo.x = (screenWidth / 2) - (e.sizeX / 2);
			region.hi.x = (screenWidth / 2) + (e.sizeX / 2);
			region.lo.y = (Int)(e.screenHeightFactor * screenHeight);
			region.hi.y = region.lo.y + e.sizeY;
			TheDisplay->drawImage(e.image, region.lo.x, region.lo.y, region.hi.x, region.hi.y, color);
		}
	}

	// Note: Applies fade after drawing the text once because originally the text takes one draw update to 'settle in' first.

	constexpr const Real fadeInMs = 500.f;
	constexpr const Real fadeOutMs = 125.f;

	if (timeGetTime() + fadeOutMs > m_waitUntilMs)
	{
		if (m_fadeValue > 0.0f)
		{
			// Fade out
			m_fadeValue -= TheFramePacer->getUpdateTime() * (1000.f / fadeOutMs);
			if (m_fadeValue < 0.0f)
				m_fadeValue = 0.0f;
		}
	}
	else if (m_fadeValue < 1.0f)
	{
		// Fade in
		m_fadeValue += TheFramePacer->getUpdateTime() * (1000.f / fadeInMs);
		if (m_fadeValue > 1.0f)
			m_fadeValue = 1.0f;
	}
}

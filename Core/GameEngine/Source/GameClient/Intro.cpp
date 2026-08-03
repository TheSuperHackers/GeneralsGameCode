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

Bool Intro::skipCurrentIntroStage()
{
	// Aborts waits, stops movies and skips waiting states

	Bool isSkipping = !isDone();

	m_waitUntilMs = 0;

	switch (m_currentState)
	{
	case IntroState_EALogoMovie:
	case IntroState_SizzleMovie:
		TheDisplay->stopMovie();
		enterNextState();
		break;

	case IntroState_TheSuperHackers:
		enterNextState();
		break;
	}

	return isSkipping;
}

void Intro::doEALogoMovie()
{
	if (TheGameLODManager && TheGameLODManager->didMemPass())
		TheDisplay->playMovie("EALogoMovie");
	else
		TheDisplay->playMovie("EALogoMovie640");
}

struct DisplaySetting
{
	DisplaySetting()
		: imageName(nullptr)
		, font("Arial")
		, text(nullptr)
		, centerOffsetY(0)
		, sizeX(10)
		, sizeY(10)
		, bold(false)
		, centered(false)
	{}

	const Char* imageName;
	const Char* font;
	const WideChar* text;
	Int centerOffsetY;
	Int sizeX;
	Int sizeY;
	Bool bold;
	Bool centered;
};

void Intro::doTheSuperHackers()
{
	std::vector<DisplaySetting> settings;

	const Real resolutionScale = GlobalLanguage::getResolutionFontSizeScale(GlobalLanguage::ResolutionFontSizeMethod_Strict);
	const Int screenWidth = TheDisplay->getWidth();
	const Int screenHeight = TheDisplay->getHeight();
	Int centerOffsetY = -(screenHeight * 0.05f);

	// "Consolas" is Windows Vista native, can be installed in 2000, XP

	{
		// Pretext
		m_unicodeStrings[0] = TheGameText->FETCH_OR_SUBSTITUTE("CREDITS:CustomIntroPretext", L"Game improved by");
		DisplaySetting setting;
		setting.font = "Consolas";
		setting.text = m_unicodeStrings[0].str();
		setting.sizeY = 16;
		centerOffsetY -= setting.sizeY * resolutionScale * 2;
		setting.centerOffsetY = centerOffsetY;
		centerOffsetY += setting.sizeY * resolutionScale * 2;
		setting.bold = false;
		setting.centered = true;
		settings.push_back(setting);
	}
	{
		// Team name
		DisplaySetting setting;
		setting.font = "Consolas";
		setting.text = L"THE SUPER HACKERS";
		setting.sizeY = 32;
		setting.centerOffsetY = centerOffsetY;
		centerOffsetY += setting.sizeY * resolutionScale * 2;
		setting.bold = true;
		setting.centered = true;
		settings.push_back(setting);
	}
	if constexpr (false)
	{
		// Website
		DisplaySetting setting;
		setting.font = "Consolas";
		setting.text = L"thesuperhackers.org";
		setting.sizeY = 16;
		setting.centerOffsetY = centerOffsetY;
		centerOffsetY += setting.sizeY * resolutionScale * 2;
		setting.bold = false;
		setting.centered = true;
		settings.push_back(setting);
	}
	{
		// China Hacker image
		DisplaySetting setting;
		setting.imageName = "SNHacker2_L";
		setting.centerOffsetY = centerOffsetY;
		setting.sizeX = (Int)(122 * 0.50f);
		setting.sizeY = (Int)(98 * 0.50f);
		settings.push_back(setting);
	}

	m_displayEntities.resize(settings.size());

	for (size_t i = 0; i < m_displayEntities.size(); ++i)
	{
		const DisplaySetting& s = settings[i];
		DisplayEntity& e = m_displayEntities[i];
		e.sizeX = Int(s.sizeX * resolutionScale);
		e.sizeY = Int(s.sizeY * resolutionScale);

		if (s.text != nullptr)
		{
			e.displayString = TheDisplayStringManager->newDisplayString();
			e.displayString->setText(s.text);
			e.displayString->setFont(TheFontLibrary->getFont(s.font, e.sizeY, s.bold));
			e.displayString->setWordWrap(screenWidth);
		}

		if (s.imageName != nullptr)
		{
			e.image = TheMappedImageCollection->findImageByName(s.imageName);
		}

		e.centerOffsetY = s.centerOffsetY;
	}

	doAsyncWait(3000);
	m_fadeValue = 0.0f;
}

void Intro::doSizzleMovie()
{
	if (TheGameLODManager && TheGameLODManager->didMemPass())
		TheDisplay->playMovie("Sizzle");
	else
		TheDisplay->playMovie("Sizzle640");
}

void Intro::doPostIntro()
{
	TheWritableGlobalData->m_breakTheMovie = TRUE;
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
			pos.y = (screenHeight / 2) + e.centerOffsetY;
			e.displayString->draw(pos.x, pos.y, color, dropColor);
		}

		if (e.image != nullptr)
		{
			IRegion2D region;
			region.lo.x = (screenWidth / 2) - (e.sizeX / 2);
			region.hi.x = (screenWidth / 2) + (e.sizeX / 2);
			region.lo.y = (screenHeight / 2) + e.centerOffsetY;
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

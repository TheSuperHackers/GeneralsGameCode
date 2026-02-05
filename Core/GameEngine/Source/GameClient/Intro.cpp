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

#include "Common/GameLOD.h"

#include "GameClient/Display.h"
#include "GameClient/GameWindowManager.h"

#include "GameLogic/FPUControl.h"


Intro::Intro()
	: m_currentState(IntroState_Start)
	, m_allowedStateFlags(0)
{
	if (TheGlobalData->m_playIntro)
	{
		m_allowedStateFlags |= 1u << IntroState_EALogoMovie;
	}

	if (TheGlobalData->m_playSizzle)
	{
		m_allowedStateFlags |= 1u << IntroState_SizzleMovie;
	}

	if (TheGameLODManager && !TheGameLODManager->didMemPass())
	{
		m_allowedStateFlags |= 1u << IntroState_LegalPage;
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
	if (!TheDisplay->isMoviePlaying())
	{
		enterNextState();

		switch (m_currentState)
		{
		case IntroState_EALogoMovie: doEALogoMovie(); break;
		case IntroState_SizzleMovie: doSizzleMovie(); break;
		case IntroState_LegalPage: doLegalPage(); break;
		case IntroState_Done: doPostIntro(); break;
		}
	}
}

void Intro::doEALogoMovie()
{
	TheWritableGlobalData->m_allowExitOutOfMovies = FALSE;
	if (TheGameLODManager && TheGameLODManager->didMemPass())
		TheDisplay->playLogoMovie("EALogoMovie", 5000, 3000);
	else
		TheDisplay->playLogoMovie("EALogoMovie640", 5000, 3000);
}

void Intro::doSizzleMovie()
{
	TheWritableGlobalData->m_allowExitOutOfMovies = TRUE;
	if (TheGameLODManager && TheGameLODManager->didMemPass())
		TheDisplay->playMovie("Sizzle");
	else
		TheDisplay->playMovie("Sizzle640");
}


void Intro::doLegalPage()
{
	TheWritableGlobalData->m_breakTheMovie = FALSE;
	WindowLayout *legal = TheWindowManager->winCreateLayout("Menus/LegalPage.wnd");
	if (legal)
	{
		legal->hide(FALSE);
		legal->bringForward();
		Int beginTime = timeGetTime();
		while (beginTime + 4000 > timeGetTime() )
		{
			TheWindowManager->update();
			// redraw all views, update the GUI
			TheDisplay->draw();
			Sleep(100);
		}
		setFPMode();
		legal->destroyWindows();
		deleteInstance(legal);
	}
	TheWritableGlobalData->m_breakTheMovie = TRUE;
}

void Intro::doPostIntro()
{
	TheWritableGlobalData->m_breakTheMovie = TRUE;
	TheWritableGlobalData->m_allowExitOutOfMovies = TRUE;
}

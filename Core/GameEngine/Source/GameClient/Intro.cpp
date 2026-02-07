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
	: m_playSizzle(false)
	, m_done(false)
{
}

void Intro::update()
{
	// We need to show the movie first.
	if(TheGlobalData->m_playIntro && !TheDisplay->isMoviePlaying())
	{
		if(TheGameLODManager && TheGameLODManager->didMemPass())
			TheDisplay->playLogoMovie("EALogoMovie", 5000, 3000);
		else
			TheDisplay->playLogoMovie("EALogoMovie640", 5000, 3000);
		TheWritableGlobalData->m_playIntro = FALSE;
		TheWritableGlobalData->m_afterIntro = TRUE;
		m_playSizzle = TRUE;
	}

	// We must show the movie first and then we can display the shell.
	if(TheGlobalData->m_afterIntro && !TheDisplay->isMoviePlaying())
	{
		if( m_playSizzle && TheGlobalData->m_playSizzle )
		{
			TheWritableGlobalData->m_allowExitOutOfMovies = TRUE;
			if(TheGameLODManager && TheGameLODManager->didMemPass())
				TheDisplay->playMovie("Sizzle");
			else
				TheDisplay->playMovie("Sizzle640");
			m_playSizzle = FALSE;
		}
		else
		{
			TheWritableGlobalData->m_breakTheMovie = TRUE;
			TheWritableGlobalData->m_allowExitOutOfMovies = TRUE;

			if(TheGameLODManager && !TheGameLODManager->didMemPass())
			{
				TheWritableGlobalData->m_breakTheMovie = FALSE;

				WindowLayout *legal = TheWindowManager->winCreateLayout("Menus/LegalPage.wnd");
				if(legal)
				{
					legal->hide(FALSE);
					legal->bringForward();
					Int beginTime = timeGetTime();
					while(beginTime + 4000 > timeGetTime() )
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

			TheWritableGlobalData->m_afterIntro = FALSE;
			m_done = TRUE;
		}
	}
}

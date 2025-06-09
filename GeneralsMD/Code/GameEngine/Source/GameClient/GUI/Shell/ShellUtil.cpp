/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

#include "GameClient/ShellUtil.h"

#include "GameClient/GameWindowTransitions.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Shell.h"
#include "GameClient/WindowLayout.h"

namespace shell
{

namespace
{
	struct ScreenInfo
	{
		ScreenInfo() : isHidden(false) {}
		AsciiString filename;
		bool isHidden;
	};
}

void recreateShell()
{
	// collect state of the current shell
	const Int screenCount = TheShell->getScreenCount();
	std::vector<ScreenInfo> screenStackInfos;
	Bool showOptions = false;

	{
		screenStackInfos.resize(screenCount);
		Int screenIndex = 0;
		for (; screenIndex < screenCount; ++screenIndex)
		{
			const WindowLayout* layout = TheShell->getScreenLayout(screenIndex);
			ScreenInfo& screenInfo = screenStackInfos[screenIndex];
			screenInfo.filename = layout->getFilename();
			screenInfo.isHidden = layout->isHidden();
		}

		const WindowLayout* optionsLayout = TheShell->getOptionsLayout(false);
		if (optionsLayout != NULL)
		{
			DEBUG_ASSERTCRASH(!optionsLayout->isHidden(), ("options menu layout is hidden\n"));
			showOptions = true;
		}
	}

	// recreate the shell
	delete TheShell;
	TheShell = MSGNEW("GameClientSubsystem") Shell;
	TheShell->init();

	// restore the screen stack
	Int screenIndex = 0;
	for (; screenIndex < screenCount; ++screenIndex)
	{
		const ScreenInfo& screenInfo = screenStackInfos[screenIndex];
		TheShell->push(screenInfo.filename);

		WindowLayout* layout = TheShell->getScreenLayout(screenIndex);
		layout->hide(screenInfo.isHidden);
	}

	if (showOptions)
	{
		// restore the options menu
		WindowLayout* layout = TheShell->getOptionsLayout(true);
		DEBUG_ASSERTCRASH(layout != NULL, ("options menu layout is NULL\n"));
		layout->runInit();
		layout->hide(false);
		layout->bringForward();
	}

	// Show the main menu logo and buttons right away.
	if (TransitionGroup* transitionGroup = TheTransitionHandler->setGroup("MainMenuDefaultMenuLogoFade", true))
	{
		transitionGroup->skip();
	}

	TheInGameUI->recreateControlBar();
}

} // namespace shell

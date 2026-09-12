#include "PreRTS.h"
#include "GameClient/Shell.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/GameWindowManager.h"

#include <algorithm>


WindowLayout::WindowLayout()
{
	m_filenameString.set("EmptyLayout");
	m_hidden = FALSE;
	m_init = nullptr;
	m_update = nullptr;
	m_shutdown = nullptr;
}

WindowLayout::~WindowLayout()
{
	DEBUG_ASSERTCRASH(m_windows.empty(), ("Window layout being destroyed still has window references"));
}

void WindowLayout::hide(Bool hide)
{
	for (GameWindow *window : m_windows)
	{
		window->winHide(hide);
	}

	m_hidden = hide;
}

void WindowLayout::addWindow(GameWindow *window)
{
	if (window == nullptr || findWindow(window) != nullptr)
	{
		return;
	}

	window->winSetLayout(this);
	// The legacy layout exposed the newest window at the head of its list.
	m_windows.insert(m_windows.begin(), window);
}

void WindowLayout::removeWindow(GameWindow *window)
{
	GameWindow *found = findWindow(window);
	if (found == nullptr)
	{
		return;
	}

	found->winSetLayout(nullptr);
	found->winSetNextInLayout(nullptr);
	found->winSetPrevInLayout(nullptr);
	std::erase(m_windows, found);
}

void WindowLayout::destroyWindows()
{
	while (GameWindow *window = getFirstWindow())
	{
		removeWindow(window);
		TheWindowManager->winDestroy(window);
	}
}

Bool WindowLayout::load(AsciiString filename)
{
	if (filename.isEmpty())
	{
		return FALSE;
	}

	WindowLayoutInfo info;
	GameWindow *target = TheWindowManager->winCreateFromScript(filename, &info);
	if (target == nullptr)
	{
		DEBUG_ASSERTCRASH(target, ("WindowLayout::load - Failed to load layout"));
		DEBUG_LOG(("WindowLayout::load - Unable to load layout file '%s'", filename.str()));
		return FALSE;
	}

	for (GameWindow *window : info.windows)
	{
		addWindow(window);
	}

	m_filenameString = filename;
	setInit(info.init);
	setUpdate(info.update);
	setShutdown(info.shutdown);
	return TRUE;
}

void WindowLayout::bringForward()
{
	// Walk from the oldest entry so the original stacking order is preserved.
	for (auto it = m_windows.rbegin(); it != m_windows.rend(); ++it)
	{
		(*it)->winBringToTop();
	}
}

GameWindow *WindowLayout::findWindow(GameWindow *window)
{
	const auto it = std::find(m_windows.begin(), m_windows.end(), window);
	return it == m_windows.end() ? nullptr : *it;
}

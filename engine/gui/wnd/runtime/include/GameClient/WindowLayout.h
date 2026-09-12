/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
*/

#pragma once

#include <vector>

#include "Common/GameMemory.h"
#include "GameClient/GameWindow.h"

class GameWindow;
class WindowLayout;

using WindowLayoutInitFunc = void (*)(WindowLayout *layout, void *userData);
using WindowLayoutUpdateFunc = void (*)(WindowLayout *layout, void *userData);
using WindowLayoutShutdownFunc = void (*)(WindowLayout *layout, void *userData);

/** Non-owning collection of windows loaded from one .wnd layout. */
class WindowLayout : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(WindowLayout, "WindowLayoutPool");

public:
	WindowLayout();

	AsciiString getFilename() const;
	Bool load(AsciiString filename);
	void hide(Bool hide);
	Bool isHidden() const;
	void bringForward();

	void addWindow(GameWindow *window);
	void removeWindow(GameWindow *window);
	void destroyWindows();
	GameWindow *getFirstWindow() const;

	void runInit(void *userData = nullptr);
	void runUpdate(void *userData = nullptr);
	void runShutdown(void *userData = nullptr);
	void setInit(WindowLayoutInitFunc init);
	void setUpdate(WindowLayoutUpdateFunc update);
	void setShutdown(WindowLayoutShutdownFunc shutdown);

protected:
	GameWindow *findWindow(GameWindow *window);

	AsciiString m_filenameString;
	std::vector<GameWindow *> m_windows;
	Bool m_hidden;

	WindowLayoutInitFunc m_init;
	WindowLayoutUpdateFunc m_update;
	WindowLayoutShutdownFunc m_shutdown;
};

inline AsciiString WindowLayout::getFilename() const { return m_filenameString; }
inline GameWindow *WindowLayout::getFirstWindow() const
{
	return m_windows.empty() ? nullptr : m_windows.front();
}
inline Bool WindowLayout::isHidden() const { return m_hidden; }

inline void WindowLayout::runInit(void *userData) { if (m_init) m_init(this, userData); }
inline void WindowLayout::runUpdate(void *userData) { if (m_update) m_update(this, userData); }
inline void WindowLayout::runShutdown(void *userData) { if (m_shutdown) m_shutdown(this, userData); }

inline void WindowLayout::setInit(WindowLayoutInitFunc init) { m_init = init; }
inline void WindowLayout::setUpdate(WindowLayoutUpdateFunc update) { m_update = update; }
inline void WindowLayout::setShutdown(WindowLayoutShutdownFunc shutdown) { m_shutdown = shutdown; }

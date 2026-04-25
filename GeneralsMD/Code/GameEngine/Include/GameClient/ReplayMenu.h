/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#pragma once

#include "Common/AsciiString.h"
#include "Common/Recorder.h"

class GameWindow;
class MapMetaData;

Bool readReplayMapInfo(const AsciiString& filename,
                       RecorderClass::ReplayHeader& header,
                       ReplayGameInfo& info,
                       const MapMetaData*& mapData);

void PopulateReplayFileListbox(GameWindow* listbox);

UnicodeString GetReplayFilenameFromListbox(GameWindow* listbox, Int index);

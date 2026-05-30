/*
**	Command & Conquer Generals Zero Hour(tm)
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
*/

#include "PreRTS.h"

#include "GameNetwork/MapDownloadHook.h"

// Default-null; GeneralsMD installs the real implementation at engine
// init. See MapDownloadHook.h for the contract.
MapDownloadHookFn TheMapDownloadHook = nullptr;

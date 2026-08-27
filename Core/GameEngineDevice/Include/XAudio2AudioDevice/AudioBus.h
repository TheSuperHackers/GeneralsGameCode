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

#pragma once

// Audio bus categories — maps to existing AudioAffect flags.
// Each bus represents a distinct volume/mute domain.
enum class AudioBus
{
	Music,
	Sound,
	Sound3D,
	Speech,

	Count
};

// Volume domain — which layer of volume control is being adjusted.
// Final volume = System * Script * UserSetting.
enum class AudioVolumeDomain
{
	System,     // Engine-level volume (e.g. zoom adjustment)
	Script,     // Script-controlled volume override
	Final       // User-facing volume setting
};



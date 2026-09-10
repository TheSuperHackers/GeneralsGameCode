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
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include "W3DDevice/GameClient/W3DTextureHandle.h"
import Assets.Dazzles;
import Graphics.Scene.Dazzles.Resources;

Graphics::DazzleResources<RefCountPtr<W3DTextureHandle>>& Get_Dazzle_Resources();
bool Initialize_Dazzle_Resources();
void Shutdown_Dazzle_Resources();
RefCountPtr<W3DTextureHandle> Acquire_Dazzle_Texture(const std::string& name);

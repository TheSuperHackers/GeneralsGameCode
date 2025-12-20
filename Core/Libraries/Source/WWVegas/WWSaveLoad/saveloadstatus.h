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

#include "always.h"
#include "wwstring.h"

namespace SaveLoadStatus
{
	void Set_Status_Text(const char* text,int id);

	void	Reset_Status_Count( void );
	void	Inc_Status_Count( void );
	int	Get_Status_Count( void );
	void Get_Status_Text(StringClass& text, int id);
};

#define INIT_STATUS(t) SaveLoadStatus::Set_Status_Text(t,0)
#define INIT_SUB_STATUS(t) SaveLoadStatus::Set_Status_Text(t,1)

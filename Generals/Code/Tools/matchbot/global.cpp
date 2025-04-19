/*
**	Command & Conquer Generals(tm)
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

#include <cstdlib> // for FILE ops
#include "global.h"

GlobalClass Global;

GlobalClass::GlobalClass(void)
{}

bool GlobalClass::ReadFile(const char *fname)
{
	FILE *fp;
	if ((fp = fopen(fname, "r")) == NULL)
		return false;
	config.readFile(fp);
	fclose(fp);

	return true;
}

bool GlobalClass::GetString(const Wstring& key, Wstring& val)
{
	val = "";
	config.getString(key, val, "STRINGS");
	if (val == "")
	{
		val.setFormatted("MISSING:%s", key.get());
		return false;
	}

	return true;

}


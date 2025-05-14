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

#include <string>
#include <Utility/stdio_adapter.h>
#include "Registry.h"

void FormatURLFromRegistry( std::string& gamePatchURL, std::string& mapPatchURL,
													 std::string& configURL, std::string& motdURL )
{
#if RTS_GENERALS
	std::string sku = "generals";
#elif RTS_ZEROHOUR
	std::string sku = "GeneralsZH";
#endif
	std::string language = "english";
	unsigned int version = 0; // invalid version - can't get on with a corrupt reg.
	unsigned int mapVersion = 0; // invalid version - can't get on with a corrupt reg.
	std::string baseURL = "http://servserv.generals.ea.com/servserv/";
	baseURL.append(sku);
	baseURL.append("/");

	GetStringFromRegistry("", "BaseURL", baseURL);
	GetStringFromRegistry("", "Language", language);
	GetUnsignedIntFromRegistry("", "Version", version);
	GetUnsignedIntFromRegistry("", "MapPackVersion", mapVersion);

	char buf[256];
	snprintf(buf, 256, "%s%s-%d.txt", baseURL.c_str(), language.c_str(), version);
	gamePatchURL = buf;
	snprintf(buf, 256, "%smaps-%d.txt", baseURL.c_str(), mapVersion);
	mapPatchURL = buf;
	snprintf(buf, 256, "%sconfig.txt", baseURL.c_str());
	configURL = buf;
	snprintf(buf, 256, "%sMOTD-%s.txt", baseURL.c_str(), language.c_str());
	motdURL = buf;
}


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

/*
** Network order is big-endian.
**
** Packet router and mangler order is big or little endian depending on server platform.
**
** Game client order is little endian.
*/
extern bool BigEndian;

template<class T> inline T Endian(T val)
{
	if (!BigEndian) {
		return(val);
	}

/*
	char temp[sizeof(T)];
	*((T*)(&temp[0])) = val;
*/

	T retval = 0;

/*
	for (int i=0 ; i<sizeof(T) ; i++) {
		retval <<= 8;
		retval |= temp[i];
	}
*/

	int len = sizeof(T);
	unsigned char *c = (unsigned char *)(&val);
	for (int i=0; i<len; i++)
	{
		retval |= ( (*c++) << (8*i) );
	}

	return (retval);
}

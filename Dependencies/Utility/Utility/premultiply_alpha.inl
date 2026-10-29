/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

// Portable implementation of premultiply_alpha.
inline void premultiply_alpha(unsigned char* pixels, int pitch, int width, int height)
{
	for (int y = 0; y < height; ++y, pixels += pitch)
	{
		unsigned int* line = reinterpret_cast<unsigned int*>(pixels);
		for (int x = 0; x < width; ++x)
		{
			const unsigned int alpha = line[x] >> 24;
			unsigned int result = line[x] & 0xFF000000u;
			result |= ((line[x] & 255) * alpha + 127) / 255;
			result |= ((((line[x] >> 8) & 255) * alpha + 127) / 255) << 8;
			result |= ((((line[x] >> 16) & 255) * alpha + 127) / 255) << 16;
			line[x] = result;
		}
	}
}

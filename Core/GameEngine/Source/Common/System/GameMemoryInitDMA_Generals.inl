/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 TheSuperHackers
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

static const PoolInitRec DefaultDMA[] =
{
	//          name, allocSize, initialCount, overflowCount
	{   "dmaPool_16",        16,        65536,          1024 },
	{   "dmaPool_32",        32,       150000,          1024 },
	{   "dmaPool_64",        64,        60000,          1024 },
	{  "dmaPool_128",       128,        32768,          1024 },
	{  "dmaPool_256",       256,         8192,          1024 },
	{  "dmaPool_512",       512,         8192,          1024 },
	{ "dmaPool_1024",      1024,        24000,          1024 }
};

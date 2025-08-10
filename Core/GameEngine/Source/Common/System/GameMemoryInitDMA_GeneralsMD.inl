/*
**	Command & Conquer Generals Zero Hour(tm)
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
	{   "dmaPool_16",        16,       130000,         10000 },
	{   "dmaPool_32",        32,       250000,         10000 },
	{   "dmaPool_64",        64,       100000,         10000 },
	{  "dmaPool_128",       128,        80000,         10000 },
	{  "dmaPool_256",       256,        20000,          5000 },
	{  "dmaPool_512",       512,        16000,          5000 },
	{ "dmaPool_1024",      1024,         6000,          1024 }
};

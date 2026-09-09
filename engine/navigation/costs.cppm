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

export module engine.navigation.costs;

export namespace navigation {
inline int estimateGoalCost(int dx, int dy)
{
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    if (dx > dy) return 10 * dx + (10 * dy) / 2;
    return 10 * dy + (10 * dx) / 2;
}

inline int stepCost(unsigned int parentCost, int dx, int dy, bool pinched)
{
    int cost;
    if (dx == 0 || dy == 0) cost = parentCost + 10;
    else cost = parentCost + 14;
    if (pinched) cost += 14;
    return cost;
}

inline int turnCost(int previousDx, int previousDy, int dx, int dy)
{
    if (previousDx != dx || previousDy != dy) {
        int dot = previousDx * dx + previousDy * dy;
        if (dot > 0) return 4;
        if (dot == 0) return 8;
        return 16;
    }
    return 0;
}
} // namespace navigation

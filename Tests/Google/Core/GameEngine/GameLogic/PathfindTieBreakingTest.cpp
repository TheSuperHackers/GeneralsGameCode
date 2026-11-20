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

#include <gtest/gtest.h>

#include "Common/GameDefines.h"
#include "Common/GameType.h"
#include "Lib/ICoord2D.h"
#include "GameLogic/Pathfinder/PathfindCell.h"
#include "GameLogic/Pathfinder/PathfindCellList.h"
#include "GameLogic/Pathfinder/PathfindConstants.h"

class PathfindTieBreakingTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
#if RETAIL_COMPATIBLE_PATHFINDING
		m_previousMode = s_useFixedPathfinding;
#endif
		PathfindCellInfo::allocateCellInfos();
	}

	void TearDown() override
	{
		PathfindCellInfo::releaseCellInfos();
#if RETAIL_COMPATIBLE_PATHFINDING
		s_useFixedPathfinding = m_previousMode;
#endif
	}

#if RETAIL_COMPATIBLE_PATHFINDING
	Bool m_previousMode;
#endif
};

TEST_F(PathfindTieBreakingTest, SharedTieBreakingIsStableAndPreservesRetailOrdering)
{
	for (int fixed = 0; fixed != 2; ++fixed) {
#if RETAIL_COMPATIBLE_PATHFINDING
		s_useFixedPathfinding = fixed != 0;
#endif
		PathfindCell cells[3];
		PathfindCellList open;
		const UnsignedInt progress[] = {20, 50, 50};
		for (int i = 0; i != 3; ++i) {
			const ICoord2D pos = {i, 0};
			ASSERT_TRUE(cells[i].allocateInfo(pos));
			cells[i].setTotalCost(100);
			cells[i].setCostSoFar(progress[i]);
			cells[i].putOnSortedOpenList(open);
		}
		const int order[] = {1, 2, 0};
		for (int i = 0; i != 3; ++i) {
#if RETAIL_COMPATIBLE_PATHFINDING
			PathfindCell* expected = &cells[fixed ? order[i] : i];
#else
			PathfindCell* expected = &cells[order[i]];
#endif
			ASSERT_EQ(open.getHead(), expected);
			expected->removeFromOpenList(open);
			expected->releaseInfo();
		}
		EXPECT_TRUE(open.empty());
	}
}

TEST_F(PathfindTieBreakingTest, ReverseInsertionOrdersCostsAndPreservesStableTies)
{
#if RETAIL_COMPATIBLE_PATHFINDING
	s_useFixedPathfinding = true;
#endif
	PathfindCell cells[7];
	PathfindCellList open;
	const UnsignedInt total[] = {10, 100, 100, 100, 90, 100, 110};
	const UnsignedInt progress[] = {0, 20, 50, 50, 5, 35, 0};
	for (int i = 0; i != 7; ++i) {
		SCOPED_TRACE(i);
		const ICoord2D pos = {i, 0};
		ASSERT_TRUE(cells[i].allocateInfo(pos));
		cells[i].setTotalCost(total[i]);
		cells[i].setCostSoFar(progress[i]);

		// The low-cost head makes each subsequent insertion closer to the tail.
		if (i >= 2) {
			EXPECT_TRUE(open.canReverseSort(cells[i]));
		}
		cells[i].putOnSortedOpenList(open);
	}

	// Higher progress wins equal-total ties; cells 2 and 3 retain insertion order.
	const int order[] = {0, 4, 2, 3, 5, 1, 6};
	for (int i = 0; i != 7; ++i) {
		SCOPED_TRACE(i);
		PathfindCell* expected = &cells[order[i]];
		ASSERT_EQ(open.getHead(), expected);
		expected->removeFromOpenList(open);
		expected->releaseInfo();
	}
	EXPECT_TRUE(open.empty());
}

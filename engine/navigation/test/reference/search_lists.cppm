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

export module engine.navigation.reference.search_lists;

// Global linkage keeps the legacy friend declaration source-compatible.
export extern "C++" {
namespace navigation::reference {
// Intrusive operations extracted from GeneralsMD's PathfindCell. Cell and List
// provide the existing links/flags; no allocation or layout changes are needed.
// Preconditions (checked by the game adapter): a cell has info and is detached
// before insertion, or belongs to the appropriate list before removal.
struct SearchLists
{
	template<class Cell, class List>
	static bool canReverseSort(Cell& cell, const List& list)
	{
		if (list.m_head && list.m_tail)
			return list.m_head->getTotalCostDifference(cell) > list.m_tail->getTotalCostDifference(cell);
		return false;
	}

	template<class Cell, class List>
	static void forwardInsertionSortRetailCompatible(Cell* self, List& list, unsigned int scanLimit)
	{

		// mark the newCell as being on the open list
		self->m_info->m_open = true;
		self->m_info->m_closed = false;

		if (list.m_head == nullptr)
		{
			list.m_head = self;
			self->m_info->m_prevOpen = nullptr;
			self->m_info->m_nextOpen = nullptr;
			return;
		}

		// insertion sort
		Cell* currentCell = list.m_head;
		Cell* previousCell = nullptr;
		unsigned int cellCount = 0;
		while (currentCell && cellCount < scanLimit && currentCell->m_info->m_totalCost <= self->m_info->m_totalCost)
		{
			// Prevent a retail crash where a pathfindCell has an m_info with a dangling nextOpen pointer
			if (currentCell->m_info->m_nextOpen && !currentCell->m_info->m_nextOpen->m_cell->m_info)
			{
				currentCell->m_info->m_nextOpen->m_cell = nullptr;
				currentCell->m_info->m_nextOpen = nullptr;
			}

			cellCount++;
			previousCell = currentCell;
			currentCell = currentCell->getNextOpen();
		}

		if (currentCell)
		{
			// insert just before "currentCell"
			if (currentCell->m_info->m_prevOpen)
				currentCell->m_info->m_prevOpen->m_nextOpen = self->m_info;
			else
				list.m_head = self;

			self->m_info->m_prevOpen = currentCell->m_info->m_prevOpen;
			currentCell->m_info->m_prevOpen = self->m_info;

			self->m_info->m_nextOpen = currentCell->m_info;

		}
		else
		{
			// append after "previousCell" - we are at the end of the list
			previousCell->m_info->m_nextOpen = self->m_info;
			self->m_info->m_prevOpen = previousCell->m_info;
			self->m_info->m_nextOpen = nullptr;
		}
	}

	template<class Cell, class List>
	static void forwardInsertionSort(Cell* self, List& list)
	{

		// mark the new cell as being on the open list
		self->m_info->m_open = true;
		self->m_info->m_closed = false;

		if (list.m_head == nullptr) {
			self->m_info->m_prevOpen = nullptr;
			self->m_info->m_nextOpen = nullptr;
			list.m_head = self;
			list.m_tail = self;
			return;
		}

		// If the node needs inserting before the current list head
		if (self->m_info->m_totalCost < list.m_head->m_info->m_totalCost) {
			self->m_info->m_prevOpen = nullptr;
			list.m_head->m_info->m_prevOpen = self->m_info;
			self->m_info->m_nextOpen = list.m_head->m_info;
			list.m_head = self;
			return;
		}

		// Traverse the list to find correct position
		Cell* current = list.m_head;
		while (current->m_info->m_nextOpen && current->m_info->m_nextOpen->m_totalCost <= self->m_info->m_totalCost) {
			current = current->getNextOpen();
		}

		// Insert the new node in the correct position
		self->m_info->m_nextOpen = current->m_info->m_nextOpen;
		if (current->m_info->m_nextOpen != nullptr) {
			current->m_info->m_nextOpen->m_prevOpen = self->m_info;
		}
		else {
			list.m_tail = self;
		}

		current->m_info->m_nextOpen = self->m_info;
		self->m_info->m_prevOpen = current->m_info;
	}

	template<class Cell, class List>
	static void reverseInsertionSort(Cell* self, List& list)
	{

		// mark the new cell as being on the open list
		self->m_info->m_open = true;
		self->m_info->m_closed = false;

		if (list.m_tail == nullptr) {
			self->m_info->m_prevOpen = nullptr;
			self->m_info->m_nextOpen = nullptr;
			list.m_tail = self;
			list.m_head = self;
			return;
		}

		// If the node needs inserting after the current list tail
		if (self->m_info->m_totalCost >= list.m_tail->m_info->m_totalCost) {
			self->m_info->m_prevOpen = list.m_tail->m_info;
			list.m_tail->m_info->m_nextOpen = self->m_info;
			self->m_info->m_nextOpen = nullptr;
			list.m_tail = self;
			return;
		}

		// Traverse the list to find correct position
		Cell* current = list.m_tail;
		while (current->m_info->m_prevOpen && current->m_info->m_prevOpen->m_totalCost > self->m_info->m_totalCost) {
			current = current->getPrevOpen();
		}

		// Insert the new node in the correct position
		self->m_info->m_prevOpen = current->m_info->m_prevOpen;
		if (current->m_info->m_prevOpen != nullptr) {
			current->m_info->m_prevOpen->m_nextOpen = self->m_info;
		}
		else {
			list.m_head = self;
		}

		current->m_info->m_prevOpen = self->m_info;
		self->m_info->m_nextOpen = current->m_info;
	}

	template<class Cell, class List>
	static void removeFromOpenList(Cell* self, List& list)
	{
		if (self->m_info->m_nextOpen)
			self->m_info->m_nextOpen->m_prevOpen = self->m_info->m_prevOpen;
		else {
			list.m_tail = self->getPrevOpen();
		}

		if (self->m_info->m_prevOpen)
			self->m_info->m_prevOpen->m_nextOpen = self->m_info->m_nextOpen;
		else
			list.m_head = self->getNextOpen();

		self->m_info->m_open = false;
		self->m_info->m_nextOpen = nullptr;
		self->m_info->m_prevOpen = nullptr;

	}

	template<class Cell, class List>
	static void removeFromClosedList(Cell* self, List& list)
	{
		if (self->m_info->m_nextOpen)
			self->m_info->m_nextOpen->m_prevOpen = self->m_info->m_prevOpen;

		if (self->m_info->m_prevOpen)
			self->m_info->m_prevOpen->m_nextOpen = self->m_info->m_nextOpen;
		else
			list.m_head = self->getNextOpen();

		self->m_info->m_closed = false;
		self->m_info->m_nextOpen = nullptr;
		self->m_info->m_prevOpen = nullptr;

	}
};
} // namespace navigation

} // extern "C++"

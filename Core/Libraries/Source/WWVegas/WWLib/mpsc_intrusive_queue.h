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

#pragma once

#include <atomic>

#include "Utility/CppMacros.h"

	// Lock-free intrusive queue for multiple producer threads and a single consumer thread.
	// The node type T must have a T* next member, which the queue owns while the node is queued.
template <typename T>
class MPSCIntrusiveQueue
{
public:
	MPSCIntrusiveQueue()
		: m_head(nullptr)
	{
	}

	// Pushes a node. Safe to call from multiple threads concurrently. The node is only
	// published to the consumer when the compare exchange succeeds against an unchanged
	// head, so its next pointer is never visible while stale.
	void Push(T* node)
	{
		T* head;
		do
		{
			head = m_head.load(std::memory_order_relaxed);
			node->next = head;
		}
		while (!m_head.compare_exchange_weak(head, node,
			std::memory_order_release, std::memory_order_relaxed));
	}

	// Detaches all nodes and returns them linked in push order, or null if the queue is
	// empty. Must only be called by the single consumer thread. The whole chain is taken
	// with one exchange, so nodes are never popped individually and the push loop cannot
	// suffer ABA.
	T* Flush()
	{
		T* list = m_head.exchange(nullptr, std::memory_order_acquire);

		// The detached chain is in last-in-first-out order, so reverse it.
		T* reversed = nullptr;
		while (list != nullptr)
		{
			T* next = list->next;
			list->next = reversed;
			reversed = list;
			list = next;
		}
		return reversed;
	}

private:
	MPSCIntrusiveQueue(const MPSCIntrusiveQueue&) CPP_11(= delete);
	MPSCIntrusiveQueue& operator=(const MPSCIntrusiveQueue&) CPP_11(= delete);

	std::atomic<T*> m_head;
};

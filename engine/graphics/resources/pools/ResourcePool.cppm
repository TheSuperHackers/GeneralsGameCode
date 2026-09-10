module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

export module Graphics.Resources.Pools.ResourcePool;

import Graphics.Resources.Handles.ResourceHandle;

namespace Graphics
{

export template <typename Resource, typename Handle>
class ResourcePool final
{
public:
	using Index = typename Handle::Index;
	using Generation = typename Handle::Generation;

	static_assert(std::is_nothrow_move_constructible_v<Resource>);
	static_assert(std::is_nothrow_move_assignable_v<Resource>);
	static_assert(std::is_nothrow_destructible_v<Resource>);

	void Reserve(std::size_t capacity)
	{
		m_resources.reserve(capacity);
		m_slots.reserve(capacity);
		m_dense_handles.reserve(capacity);
	}

	template <typename... Arguments>
	Handle Create(Arguments &&...arguments)
	{
		Ensure_Capacity();

		const bool reuses_slot = m_free_head != InvalidIndex;
		const Index slot_index = reuses_slot
			? m_free_head
			: static_cast<Index>(m_slots.size());
		const Index dense_index = static_cast<Index>(m_resources.size());

		m_resources.emplace_back(std::forward<Arguments>(arguments)...);

		if (reuses_slot) {
			Slot &slot = m_slots[slot_index];
			m_free_head = slot.next_free;
			slot.next_free = InvalidIndex;
			slot.dense_index = dense_index;
		} else {
			m_slots.push_back({dense_index, InvalidIndex, 1});
		}
		m_dense_handles.emplace_back(slot_index, m_slots[slot_index].generation);

		return Handle(slot_index, m_slots[slot_index].generation);
	}

	bool Destroy(Handle handle) noexcept
	{
		if (!Is_Valid_Handle(handle))
			return false;

		const Index slot_index = handle.Get_Index();
		const Index dense_index = m_slots[slot_index].dense_index;
		const Index last_dense_index = static_cast<Index>(m_resources.size() - 1);

		if (dense_index != last_dense_index) {
			m_resources[dense_index] = std::move(m_resources[last_dense_index]);

			const Index moved_slot_index = m_dense_handles[last_dense_index].Get_Index();
			m_dense_handles[dense_index] = m_dense_handles[last_dense_index];
			m_slots[moved_slot_index].dense_index = dense_index;
		}

		m_resources.pop_back();
		m_dense_handles.pop_back();

		Slot &slot = m_slots[slot_index];
		slot.dense_index = InvalidIndex;
		slot.generation = Next_Generation(slot.generation);
		slot.next_free = m_free_head;
		m_free_head = slot_index;

		return true;
	}

	void Clear() noexcept
	{
		while (!m_dense_handles.empty())
			Destroy(m_dense_handles.back());
	}

	Resource *Resolve(Handle handle) noexcept
	{
		if (!Is_Valid_Handle(handle))
			return nullptr;

		return &m_resources[m_slots[handle.Get_Index()].dense_index];
	}

	const Resource *Resolve(Handle handle) const noexcept
	{
		if (!Is_Valid_Handle(handle))
			return nullptr;

		return &m_resources[m_slots[handle.Get_Index()].dense_index];
	}

	std::size_t Size() const noexcept
	{
		return m_resources.size();
	}

	template <typename Function>
	void For_Each(Function &&function) const noexcept
	{
		for (std::size_t dense_index = 0; dense_index < m_resources.size(); ++dense_index) {
			function(m_dense_handles[dense_index], m_resources[dense_index]);
		}
	}

	private:
	void Ensure_Capacity()
	{
		if (m_resources.size() < m_resources.capacity())
			return;

		const std::size_t current_capacity = m_resources.size();
		const std::size_t next_capacity = current_capacity == 0
			? 1
			: current_capacity > std::numeric_limits<std::size_t>::max() / 2
				? std::numeric_limits<std::size_t>::max()
				: current_capacity * 2;
		m_resources.reserve(next_capacity);
		m_slots.reserve(next_capacity);
		m_dense_handles.reserve(next_capacity);
	}

	struct Slot
	{
		Index dense_index;
		Index next_free;
		Generation generation;
	};

	static constexpr Index InvalidIndex = std::numeric_limits<Index>::max();

	static constexpr Generation Next_Generation(Generation generation) noexcept
	{
		const Generation next_generation = generation + 1;
		return next_generation == 0 ? 1 : next_generation;
	}

	bool Is_Valid_Handle(Handle handle) const noexcept
	{
		if (!handle.Is_Valid())
			return false;

		const Index slot_index = handle.Get_Index();
		if (slot_index >= m_slots.size())
			return false;

		const Slot &slot = m_slots[slot_index];
		return slot.dense_index != InvalidIndex && slot.generation == handle.Get_Generation();
	}

	std::vector<Resource> m_resources;
	std::vector<Slot> m_slots;
	std::vector<Handle> m_dense_handles;
	Index m_free_head = InvalidIndex;
};

}

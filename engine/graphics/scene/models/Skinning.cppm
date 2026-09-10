module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <vector>

export module Graphics.Scene.Models.Skinning;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Scene.RenderScene;

namespace Graphics
{

export struct alignas(16) GPUBoneMatrixData final
{
	std::array<float, 16> matrix{};
};

static_assert(sizeof(GPUBoneMatrixData) == 64);
static_assert(std::is_trivially_copyable_v<GPUBoneMatrixData>);

export inline constexpr std::uint32_t Invalid_Bone_Matrix_Index = std::numeric_limits<std::uint32_t>::max();

export struct BoneMatrixRange final
{
	std::uint32_t first_matrix = Invalid_Bone_Matrix_Index;
	std::uint32_t count = 0;

	bool Is_Valid() const noexcept
	{
		return first_matrix != Invalid_Bone_Matrix_Index && count != 0;
	}
};

export struct BoneMatrixDirtyRange final
{
	std::uint32_t first_matrix = Invalid_Bone_Matrix_Index;
	std::uint32_t count = 0;
};

export class BoneMatrixTable final
{
public:
	void Reserve(std::size_t pose_capacity, std::size_t matrix_capacity)
	{
		m_slots.reserve(pose_capacity);
		m_free_ranges.reserve(pose_capacity);
		m_dirty_ranges.reserve(pose_capacity);
		m_matrices.reserve(matrix_capacity);
	}

	PoseHandle Create(std::span<const RenderTransform> matrices)
	{
		if (matrices.empty() || matrices.size() > std::numeric_limits<std::uint32_t>::max())
			return {};

		const BoneMatrixRange range = Allocate_Range(static_cast<std::uint32_t>(matrices.size()));
		if (!range.Is_Valid())
			return {};

		std::uint32_t slot_index = Invalid_Bone_Matrix_Index;
		if (m_free_head != Invalid_Bone_Matrix_Index) {
			slot_index = m_free_head;
			Slot &slot = m_slots[slot_index];
			m_free_head = slot.next_free;
			slot.next_free = Invalid_Bone_Matrix_Index;
			slot.generation = Next_Generation(slot.last_generation);
			slot.range = range;
		} else {
			if (m_slots.size() >= std::numeric_limits<std::uint32_t>::max())
				return {};
			slot_index = static_cast<std::uint32_t>(m_slots.size());
			m_slots.push_back({range, 1, 0, Invalid_Bone_Matrix_Index});
		}

		for (std::uint32_t index = 0; index < range.count; ++index)
			m_matrices[range.first_matrix + index].matrix = matrices[index].matrix;
		Mark_Dirty(range);
		return PoseHandle(slot_index, m_slots[slot_index].generation);
	}

	bool Update(PoseHandle handle, std::span<const RenderTransform> matrices) noexcept
	{
		const BoneMatrixRange range = Range(handle);
		if (!range.Is_Valid() || matrices.size() != range.count || !Can_Mark(range))
			return false;

		for (std::uint32_t index = 0; index < range.count; ++index)
			m_matrices[range.first_matrix + index].matrix = matrices[index].matrix;
		Mark_Dirty(range);
		return true;
	}

	bool Destroy(PoseHandle handle) noexcept
	{
		if (!Is_Valid(handle))
			return false;

		Slot &slot = m_slots[handle.Get_Index()];
		m_free_ranges.push_back(slot.range);
		slot.last_generation = slot.generation;
		slot.generation = 0;
		slot.range = {};
		slot.next_free = m_free_head;
		m_free_head = handle.Get_Index();
		return true;
	}

	bool Is_Valid(PoseHandle handle) const noexcept
	{
		if (!handle.Is_Valid() || handle.Get_Index() >= m_slots.size())
			return false;

		const Slot &slot = m_slots[handle.Get_Index()];
		return slot.generation == handle.Get_Generation() && slot.range.Is_Valid();
	}

	BoneMatrixRange Range(PoseHandle handle) const noexcept
	{
		return Is_Valid(handle) ? m_slots[handle.Get_Index()].range : BoneMatrixRange{};
	}

	bool Transform(PoseHandle handle, std::uint32_t bone, RenderTransform &transform) const noexcept
	{
		const BoneMatrixRange range = Range(handle);
		if (!range.Is_Valid() || bone >= range.count)
			return false;
		transform.matrix = m_matrices[range.first_matrix + bone].matrix;
		return true;
	}

	std::span<const GPUBoneMatrixData> Matrices() const noexcept
	{
		return m_matrices;
	}

	std::span<const BoneMatrixDirtyRange> Dirty_Ranges() const noexcept
	{
		return m_dirty_ranges;
	}

	void Clear_Dirty() noexcept
	{
		m_dirty_ranges.clear();
	}

	void Clear() noexcept
	{
		m_slots.clear();
		m_free_ranges.clear();
		m_matrices.clear();
		m_dirty_ranges.clear();
		m_free_head = Invalid_Bone_Matrix_Index;
	}

private:
	struct Slot final
	{
		BoneMatrixRange range{};
		std::uint32_t generation = 0;
		std::uint32_t last_generation = 0;
		std::uint32_t next_free = Invalid_Bone_Matrix_Index;
	};

	static std::uint32_t Next_Generation(std::uint32_t generation) noexcept
	{
		const std::uint32_t next = generation + 1;
		return next == 0 ? 1 : next;
	}

	BoneMatrixRange Allocate_Range(std::uint32_t count)
	{
		for (std::size_t index = 0; index < m_free_ranges.size(); ++index) {
			BoneMatrixRange &free_range = m_free_ranges[index];
			if (free_range.count < count)
				continue;

			const BoneMatrixRange result{free_range.first_matrix, count};
			if (free_range.count == count) {
				free_range = m_free_ranges.back();
				m_free_ranges.pop_back();
			} else {
				free_range.first_matrix += count;
				free_range.count -= count;
			}
			return result;
		}

		if (m_matrices.size() > std::numeric_limits<std::uint32_t>::max() - count)
			return {};
		const std::uint32_t first_matrix = static_cast<std::uint32_t>(m_matrices.size());
		m_matrices.resize(m_matrices.size() + count);
		return {first_matrix, count};
	}

	bool Can_Mark(BoneMatrixRange range) const noexcept
	{
		if (!m_dirty_ranges.empty()) {
			const BoneMatrixDirtyRange &last = m_dirty_ranges.back();
			const std::uint64_t last_end = static_cast<std::uint64_t>(last.first_matrix) + last.count;
			if (last_end == range.first_matrix || (range.first_matrix <= last.first_matrix
				&& static_cast<std::uint64_t>(range.first_matrix) + range.count >= last_end))
				return true;
		}
		return m_dirty_ranges.size() < m_dirty_ranges.capacity();
	}

	void Mark_Dirty(BoneMatrixRange range) noexcept
	{
		if (!m_dirty_ranges.empty()) {
			BoneMatrixDirtyRange &last = m_dirty_ranges.back();
			const std::uint64_t last_end = static_cast<std::uint64_t>(last.first_matrix) + last.count;
			const std::uint64_t range_end = static_cast<std::uint64_t>(range.first_matrix) + range.count;
			if (last_end == range.first_matrix) {
				last.count += range.count;
				return;
			}
			if (range.first_matrix <= last.first_matrix && range_end >= last_end) {
				last.first_matrix = range.first_matrix;
				last.count = static_cast<std::uint32_t>(range_end - range.first_matrix);
				return;
			}
		}
		m_dirty_ranges.push_back({range.first_matrix, range.count});
	}

	std::vector<Slot> m_slots;
	std::vector<BoneMatrixRange> m_free_ranges;
	std::vector<GPUBoneMatrixData> m_matrices;
	std::vector<BoneMatrixDirtyRange> m_dirty_ranges;
	std::uint32_t m_free_head = Invalid_Bone_Matrix_Index;
};

static_assert(std::is_nothrow_move_constructible_v<BoneMatrixTable>);
static_assert(std::is_nothrow_move_assignable_v<BoneMatrixTable>);

}

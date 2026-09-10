module;

#include <cmath>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

export module Graphics.Scene.Models.Animation;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Resources.Pools.ResourcePool;
export import Graphics.Scene.Models.Skeleton;
import Graphics.Memory.AlignedAllocator;

namespace Graphics
{

export enum class AnimationPlaybackMode : std::uint8_t
{
	Once,
	Loop,
	Once_Backwards,
	Loop_Backwards
};

export struct AnimationClipDescription final
{
	std::uint32_t bone_count = 0;
	std::uint32_t frame_count = 0;
	float frame_rate = 0.0f;
	std::span<const RenderTransform> local_transforms{};
};

export bool Validate_Animation_Clip_Description(const AnimationClipDescription &description) noexcept
{
	if (description.bone_count == 0 || description.frame_count == 0
		|| !std::isfinite(description.frame_rate) || description.frame_rate <= 0.0f)
		return false;

	if (description.frame_count > std::numeric_limits<std::size_t>::max() / description.bone_count)
		return false;
	return description.local_transforms.size()
		== static_cast<std::size_t>(description.bone_count) * description.frame_count;
}

export class AnimationClip final
{
public:
	AnimationClip() = default;

	AnimationClip(std::uint32_t bone_count, std::uint32_t frame_count, float frame_rate,
		std::span<const RenderTransform> local_transforms)
		: m_bone_count(bone_count),
		  m_frame_count(frame_count),
		  m_frame_rate(frame_rate)
	{
		if (!Validate_Animation_Clip_Description({bone_count, frame_count, frame_rate, local_transforms})) {
			*this = {};
			return;
		}
		m_local_transforms.assign(local_transforms.begin(), local_transforms.end());
	}

	AnimationClip(const AnimationClip &) = default;
	AnimationClip &operator=(const AnimationClip &) = default;
	AnimationClip(AnimationClip &&) noexcept = default;
	AnimationClip &operator=(AnimationClip &&) noexcept = default;

	bool Is_Valid() const noexcept
	{
		return m_bone_count != 0 && m_frame_count != 0 && std::isfinite(m_frame_rate) && m_frame_rate > 0.0f
			&& m_local_transforms.size() == static_cast<std::size_t>(m_bone_count) * m_frame_count;
	}

	std::uint32_t Bone_Count() const noexcept
	{
		return m_bone_count;
	}

	std::uint32_t Frame_Count() const noexcept
	{
		return m_frame_count;
	}

	float Frame_Rate() const noexcept
	{
		return m_frame_rate;
	}

	float Duration() const noexcept
	{
		return Is_Valid() ? static_cast<float>(m_frame_count) / m_frame_rate : 0.0f;
	}

	std::span<const RenderTransform> Local_Transforms() const noexcept
	{
		return m_local_transforms;
	}

	bool Sample(float time_seconds, AnimationPlaybackMode mode,
		std::span<RenderTransform> output) const noexcept
	{
		if (!Is_Valid() || output.size() < m_bone_count || !std::isfinite(time_seconds))
			return false;

		float frame = time_seconds * m_frame_rate;
		const bool backwards = mode == AnimationPlaybackMode::Once_Backwards
			|| mode == AnimationPlaybackMode::Loop_Backwards;
		if (backwards)
			frame = static_cast<float>(m_frame_count - 1) - frame;

		const bool looping = mode == AnimationPlaybackMode::Loop
			|| mode == AnimationPlaybackMode::Loop_Backwards;
		if (looping) {
			const float frame_count = static_cast<float>(m_frame_count);
			frame = std::fmod(frame, frame_count);
			if (frame < 0.0f)
				frame += frame_count;
		} else {
			if (frame < 0.0f)
				frame = 0.0f;
			const float last_frame = static_cast<float>(m_frame_count - 1);
			if (frame > last_frame)
				frame = last_frame;
		}

		const std::uint32_t first_frame = static_cast<std::uint32_t>(std::floor(frame));
		const float fraction = frame - static_cast<float>(first_frame);
		const std::uint32_t second_frame = looping
			? (first_frame + 1u) % m_frame_count
			: first_frame + (first_frame + 1u < m_frame_count ? 1u : 0u);
		const std::size_t first_offset = static_cast<std::size_t>(first_frame) * m_bone_count;
		const std::size_t second_offset = static_cast<std::size_t>(second_frame) * m_bone_count;
		for (std::uint32_t bone = 0; bone < m_bone_count; ++bone) {
			const RenderTransform &first = m_local_transforms[first_offset + bone];
			const RenderTransform &second = m_local_transforms[second_offset + bone];
			RenderTransform &sample = output[bone];
			for (std::size_t element = 0; element < sample.matrix.size(); ++element)
				sample.matrix[element] = first.matrix[element]
					+ (second.matrix[element] - first.matrix[element]) * fraction;
		}
		return true;
	}

private:
	std::uint32_t m_bone_count = 0;
	std::uint32_t m_frame_count = 0;
	float m_frame_rate = 0.0f;
	std::vector<RenderTransform> m_local_transforms;
};

static_assert(std::is_nothrow_move_constructible_v<AnimationClip>);
static_assert(std::is_nothrow_move_assignable_v<AnimationClip>);

export using AnimationClipPool = ResourcePool<AnimationClip, AnimationClipHandle>;

export AnimationClipHandle Create_Animation_Clip(AnimationClipPool &pool,
	const AnimationClipDescription &description)
{
	if (!Validate_Animation_Clip_Description(description))
		return {};
	return pool.Create(description.bone_count, description.frame_count, description.frame_rate,
		description.local_transforms);
}

export class Pose final
{
public:
	Pose() = default;

	bool Initialize(std::size_t bone_count)
	{
		if (bone_count == 0)
			return false;
		if (m_local_transforms.size() != bone_count)
			m_local_transforms.resize(bone_count);
		if (m_world_transforms.size() != bone_count)
			m_world_transforms.resize(bone_count);
		return true;
	}

	void Reset() noexcept
	{
		m_local_transforms.clear();
		m_world_transforms.clear();
	}

	bool Evaluate(const Skeleton &skeleton, std::span<const RenderTransform> local_transforms) noexcept
	{
		if (local_transforms.size() != skeleton.Bone_Count()
			|| m_world_transforms.size() != skeleton.Bone_Count())
			return false;
		if (local_transforms.data() != m_local_transforms.data())
			std::copy(local_transforms.begin(), local_transforms.end(), m_local_transforms.begin());
		return skeleton.Evaluate_Pose(m_local_transforms, m_world_transforms);
	}

	std::span<const RenderTransform> Local_Transforms() const noexcept
	{
		return m_local_transforms;
	}

	std::span<RenderTransform> Local_Transforms() noexcept
	{
		return m_local_transforms;
	}

	std::span<const RenderTransform> World_Transforms() const noexcept
	{
		return m_world_transforms;
	}

	std::span<RenderTransform> World_Transforms() noexcept
	{
		return m_world_transforms;
	}

	bool Is_Valid() const noexcept
	{
		return !m_world_transforms.empty() && m_local_transforms.size() == m_world_transforms.size();
	}

private:
	AlignedVector<RenderTransform> m_local_transforms;
	AlignedVector<RenderTransform> m_world_transforms;
};

static_assert(std::is_nothrow_move_constructible_v<Pose>);
static_assert(std::is_nothrow_move_assignable_v<Pose>);

}

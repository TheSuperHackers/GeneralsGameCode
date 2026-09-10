module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <type_traits>
#include <vector>

export module Graphics.Scene.Models.ModelInstance;

export import Graphics.Scene.Models.Skeleton;
export import Graphics.Scene.Models.Animation;
export import Graphics.Scene.Models.AnimationBlend;

namespace Graphics
{

export struct ModelInstance final
{
	SkeletonHandle skeleton{};
	RenderTransform transform{};
	AnimationClipHandle animation{};
	float animation_time = 0.0f;
	AnimationPlaybackMode animation_mode = AnimationPlaybackMode::Loop;
	AnimationClipHandle secondary_animation{};
	float secondary_animation_time = 0.0f;
	AnimationPlaybackMode secondary_animation_mode = AnimationPlaybackMode::Loop;
	float blend_weight = 0.0f;
	Pose pose{};

	bool Set_Animation(const SkeletonPool &skeletons, const AnimationClipPool &animations,
		AnimationClipHandle animation_handle, AnimationPlaybackMode mode, float time_seconds = 0.0f)
	{
		const Skeleton *skeleton_resource = skeletons.Resolve(skeleton);
		const AnimationClip *clip = animations.Resolve(animation_handle);
		if (skeleton_resource == nullptr || clip == nullptr
			|| clip->Bone_Count() != skeleton_resource->Bone_Count()
			|| !std::isfinite(time_seconds) || !pose.Initialize(skeleton_resource->Bone_Count()))
			return false;

		animation = animation_handle;
		animation_mode = mode;
		animation_time = time_seconds;
		secondary_animation = {};
		secondary_animation_time = 0.0f;
		secondary_animation_mode = AnimationPlaybackMode::Loop;
		blend_weight = 0.0f;
		m_secondary_local_transforms.clear();
		return Evaluate_Animation(skeletons, animations);
	}

	bool Set_Animation_Blend(const SkeletonPool &skeletons, const AnimationClipPool &animations,
		AnimationClipHandle first_animation, AnimationPlaybackMode first_mode,
		float first_time, AnimationClipHandle second_animation_handle,
		AnimationPlaybackMode second_mode, float second_time, float weight)
	{
		const Skeleton *skeleton_resource = skeletons.Resolve(skeleton);
		const AnimationClip *first_clip = animations.Resolve(first_animation);
		const AnimationClip *second_clip = animations.Resolve(second_animation_handle);
		if (skeleton_resource == nullptr || first_clip == nullptr || second_clip == nullptr
			|| first_clip->Bone_Count() != skeleton_resource->Bone_Count()
			|| second_clip->Bone_Count() != skeleton_resource->Bone_Count()
			|| !std::isfinite(first_time) || !std::isfinite(second_time)
			|| !std::isfinite(weight) || !pose.Initialize(skeleton_resource->Bone_Count()))
			return false;

		if (m_secondary_local_transforms.size() != skeleton_resource->Bone_Count())
			m_secondary_local_transforms.resize(skeleton_resource->Bone_Count());
		animation = first_animation;
		animation_mode = first_mode;
		animation_time = first_time;
		secondary_animation = second_animation_handle;
		secondary_animation_mode = second_mode;
		secondary_animation_time = second_time;
		blend_weight = std::clamp(weight, 0.0f, 1.0f);
		return Evaluate_Animation(skeletons, animations);
	}

	void Clear_Animation() noexcept
	{
		animation = {};
		animation_time = 0.0f;
		animation_mode = AnimationPlaybackMode::Loop;
		secondary_animation = {};
		secondary_animation_time = 0.0f;
		secondary_animation_mode = AnimationPlaybackMode::Loop;
		blend_weight = 0.0f;
		m_secondary_local_transforms.clear();
	}

	bool Set_Animation_Time(const SkeletonPool &skeletons, const AnimationClipPool &animations,
		float time_seconds) noexcept
	{
		if (!std::isfinite(time_seconds) || !animation.Is_Valid() || !pose.Is_Valid())
			return false;
		const float previous_time = animation_time;
		animation_time = time_seconds;
		if (Evaluate_Animation(skeletons, animations))
			return true;
		animation_time = previous_time;
		return false;
	}

	bool Set_Animation_Blend_State(const SkeletonPool &skeletons, const AnimationClipPool &animations,
		float first_time, float second_time, float weight) noexcept
	{
		if (!std::isfinite(first_time) || !std::isfinite(second_time) || !std::isfinite(weight)
			|| !animation.Is_Valid() || !secondary_animation.Is_Valid() || !pose.Is_Valid())
			return false;
		const float previous_time = animation_time;
		const float previous_secondary_time = secondary_animation_time;
		const float previous_weight = blend_weight;
		animation_time = first_time;
		secondary_animation_time = second_time;
		blend_weight = std::clamp(weight, 0.0f, 1.0f);
		if (Evaluate_Animation(skeletons, animations))
			return true;
		animation_time = previous_time;
		secondary_animation_time = previous_secondary_time;
		blend_weight = previous_weight;
		return false;
	}

	bool Advance_Animation(const SkeletonPool &skeletons, const AnimationClipPool &animations,
		float delta_seconds) noexcept
	{
		if (!std::isfinite(delta_seconds))
			return false;
		const float next_time = animation_time + delta_seconds;
		if (!std::isfinite(next_time) || !animation.Is_Valid() || !pose.Is_Valid())
			return false;
		float next_secondary_time = secondary_animation_time;
		if (secondary_animation.Is_Valid()) {
			next_secondary_time += delta_seconds;
			if (!std::isfinite(next_secondary_time))
				return false;
		}
		animation_time = next_time;
		secondary_animation_time = next_secondary_time;
		if (Evaluate_Animation(skeletons, animations))
			return true;
		animation_time -= delta_seconds;
		secondary_animation_time -= delta_seconds;
		return false;
	}

	bool Set_Animation_Mode(const SkeletonPool &skeletons, const AnimationClipPool &animations,
		AnimationPlaybackMode mode) noexcept
	{
		if (!animation.Is_Valid() || !pose.Is_Valid())
			return false;
		const AnimationPlaybackMode previous_mode = animation_mode;
		animation_mode = mode;
		if (Evaluate_Animation(skeletons, animations))
			return true;
		animation_mode = previous_mode;
		return false;
	}

	bool Set_Bone_Local_Transform(const SkeletonPool &skeletons, BoneHandle bone,
		const RenderTransform &local_transform) noexcept
	{
		const Skeleton *skeleton_resource = skeletons.Resolve(skeleton);
		if (skeleton_resource == nullptr || !skeleton_resource->Is_Valid_Bone(bone))
			return false;

		if (!pose.Is_Valid()) {
			if (!pose.Initialize(skeleton_resource->Bone_Count()))
				return false;
			for (BoneIndex index = 0; index < skeleton_resource->Bone_Count(); ++index)
				pose.Local_Transforms()[index] = skeleton_resource->Bones()[index].rest_transform;
		}

		pose.Local_Transforms()[bone.Get_Index()] = local_transform;
		return pose.Evaluate(*skeleton_resource, pose.Local_Transforms());
	}

	bool Get_Bone_Transform(const SkeletonPool &skeletons, BoneHandle bone, RenderTransform &result) const noexcept
	{
		const Skeleton *resource = skeletons.Resolve(skeleton);
		if (resource == nullptr || !resource->Is_Valid_Bone(bone))
			return false;
		if (animation.Is_Valid() && pose.Is_Valid())
			result = pose.World_Transforms()[bone.Get_Index()];
		else if (!resource->Rest_Transform(bone, result))
			return false;
		result = Multiply(transform, result);
		return true;
	}

	bool Get_Attachment_Transform(const SkeletonPool &skeletons, AttachmentHandle attachment, RenderTransform &result) const noexcept
	{
		const Skeleton *resource = skeletons.Resolve(skeleton);
		if (resource == nullptr || !resource->Is_Valid_Attachment(attachment))
			return false;
		const SkeletonAttachment &point = resource->Attachments()[attachment.Get_Index()];
		if (animation.Is_Valid() && pose.Is_Valid()) {
			result = Multiply(pose.World_Transforms()[point.bone.Get_Index()], point.local_transform);
		} else if (!resource->Attachment_Transform(attachment, result)) {
			return false;
		}
		result = Multiply(transform, result);
		return true;
	}

private:
	bool Evaluate_Animation(const SkeletonPool &skeletons, const AnimationClipPool &animations) noexcept
	{
		const Skeleton *skeleton_resource = skeletons.Resolve(skeleton);
		const AnimationClip *clip = animations.Resolve(animation);
		if (skeleton_resource == nullptr || clip == nullptr || !pose.Is_Valid()
			|| clip->Bone_Count() != skeleton_resource->Bone_Count())
			return false;
		if (!clip->Sample(animation_time, animation_mode, pose.Local_Transforms()))
			return false;
		if (secondary_animation.Is_Valid()) {
			const AnimationClip *secondary_clip = animations.Resolve(secondary_animation);
			if (secondary_clip == nullptr || secondary_clip->Bone_Count() != skeleton_resource->Bone_Count()
				|| m_secondary_local_transforms.size() != skeleton_resource->Bone_Count()
				|| !secondary_clip->Sample(secondary_animation_time, secondary_animation_mode,
					m_secondary_local_transforms))
				return false;
			if (!Blend_Local_Poses(pose.Local_Transforms(), m_secondary_local_transforms,
				blend_weight, pose.Local_Transforms()))
				return false;
		}
		return pose.Evaluate(*skeleton_resource, pose.Local_Transforms());
	}

	static RenderTransform Multiply(const RenderTransform &left, const RenderTransform &right) noexcept
	{
		RenderTransform result;
		for (std::size_t row = 0; row < 4; ++row) {
			for (std::size_t column = 0; column < 4; ++column) {
				float value = 0.0f;
				for (std::size_t element = 0; element < 4; ++element)
					value += left.matrix[row * 4 + element] * right.matrix[element * 4 + column];
				result.matrix[row * 4 + column] = value;
			}
		}
		return result;
	}

	std::vector<RenderTransform> m_secondary_local_transforms;
};

static_assert(std::is_nothrow_move_constructible_v<ModelInstance>);
static_assert(std::is_nothrow_move_assignable_v<ModelInstance>);

}

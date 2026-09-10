module;

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

export module Graphics.Scene.Models.Skeleton;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Resources.Pools.ResourcePool;
export import Graphics.Scene.RenderScene;

namespace Graphics
{

export using BoneIndex = std::uint32_t;
export using AttachmentIndex = std::uint32_t;

export inline constexpr BoneIndex Invalid_Bone_Index = std::numeric_limits<BoneIndex>::max();
export inline constexpr AttachmentIndex Invalid_Attachment_Index = std::numeric_limits<AttachmentIndex>::max();

export struct SkeletonBone final
{
	BoneIndex parent = Invalid_Bone_Index;
	RenderTransform rest_transform{};
};

export struct SkeletonAttachment final
{
	BoneHandle bone{};
	RenderTransform local_transform{};
};

export struct SkeletonDescription final
{
	std::span<const SkeletonBone> bones{};
	std::span<const SkeletonAttachment> attachments{};
};

namespace
{
RenderTransform Identity_Transform() noexcept
{
	RenderTransform transform;
	transform.matrix[0] = 1.0f;
	transform.matrix[5] = 1.0f;
	transform.matrix[10] = 1.0f;
	transform.matrix[15] = 1.0f;
	return transform;
}

RenderTransform Multiply_Transform(const RenderTransform &left, const RenderTransform &right) noexcept
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
}

export bool Validate_Skeleton_Description(const SkeletonDescription &description) noexcept
{
	if (description.bones.empty() || description.bones.size() > std::numeric_limits<BoneIndex>::max())
		return false;

	for (BoneIndex index = 0; index < description.bones.size(); ++index) {
		const BoneIndex parent = description.bones[index].parent;
		if (parent != Invalid_Bone_Index && parent >= index)
			return false;
	}

	if (description.attachments.size() > std::numeric_limits<AttachmentIndex>::max())
		return false;
	for (const SkeletonAttachment &attachment : description.attachments) {
		if (!attachment.bone.Is_Valid()
			|| attachment.bone.Get_Generation() != 1
			|| attachment.bone.Get_Index() >= description.bones.size())
			return false;
	}
	return true;
}

export class Skeleton final
{
public:
	Skeleton() = default;

	Skeleton(std::span<const SkeletonBone> bones, std::span<const SkeletonAttachment> attachments)
	{
		const SkeletonDescription description{bones, attachments};
		if (!Validate_Skeleton_Description(description))
			return;

		m_bones.assign(bones.begin(), bones.end());
		m_world_rest_transforms.resize(m_bones.size(), Identity_Transform());
		for (BoneIndex index = 0; index < m_bones.size(); ++index) {
			const SkeletonBone &bone = m_bones[index];
			m_world_rest_transforms[index] = bone.parent == Invalid_Bone_Index
				? bone.rest_transform
				: Multiply_Transform(m_world_rest_transforms[bone.parent], bone.rest_transform);
		}
		m_attachments.assign(attachments.begin(), attachments.end());
	}

	Skeleton(const Skeleton &) = default;
	Skeleton &operator=(const Skeleton &) = default;
	Skeleton(Skeleton &&) noexcept = default;
	Skeleton &operator=(Skeleton &&) noexcept = default;

	bool Is_Valid() const noexcept
	{
		return !m_bones.empty() && m_bones.size() == m_world_rest_transforms.size();
	}

	std::size_t Bone_Count() const noexcept
	{
		return m_bones.size();
	}

	std::size_t Attachment_Count() const noexcept
	{
		return m_attachments.size();
	}

	BoneHandle Bone(BoneIndex index) const noexcept
	{
		return index < m_bones.size() ? BoneHandle(index, 1) : BoneHandle{};
	}

	AttachmentHandle Attachment(AttachmentIndex index) const noexcept
	{
		return index < m_attachments.size() ? AttachmentHandle(index, 1) : AttachmentHandle{};
	}

	bool Is_Valid_Bone(BoneHandle bone) const noexcept
	{
		return bone.Is_Valid() && bone.Get_Generation() == 1 && bone.Get_Index() < m_bones.size();
	}

	bool Is_Valid_Attachment(AttachmentHandle attachment) const noexcept
	{
		return attachment.Is_Valid() && attachment.Get_Generation() == 1
			&& attachment.Get_Index() < m_attachments.size();
	}

	BoneIndex Parent(BoneHandle bone) const noexcept
	{
		return Is_Valid_Bone(bone) ? m_bones[bone.Get_Index()].parent : Invalid_Bone_Index;
	}

	bool Rest_Transform(BoneHandle bone, RenderTransform &transform) const noexcept
	{
		if (!Is_Valid_Bone(bone))
			return false;
		transform = m_world_rest_transforms[bone.Get_Index()];
		return true;
	}

	bool Evaluate_Pose(std::span<const RenderTransform> local_transforms,
		std::span<RenderTransform> world_transforms) const noexcept
	{
		if (!Is_Valid() || local_transforms.size() != m_bones.size()
			|| world_transforms.size() != m_bones.size())
			return false;

		for (BoneIndex index = 0; index < m_bones.size(); ++index) {
			const BoneIndex parent = m_bones[index].parent;
			world_transforms[index] = parent == Invalid_Bone_Index
				? local_transforms[index]
				: Multiply_Transform(world_transforms[parent], local_transforms[index]);
		}
		return true;
	}

	bool Attachment_Transform(AttachmentHandle attachment, RenderTransform &transform) const noexcept
	{
		if (!Is_Valid_Attachment(attachment))
			return false;
		const SkeletonAttachment &point = m_attachments[attachment.Get_Index()];
		RenderTransform bone_transform;
		if (!Rest_Transform(point.bone, bone_transform))
			return false;
		transform = Multiply_Transform(bone_transform, point.local_transform);
		return true;
	}

	std::span<const SkeletonBone> Bones() const noexcept
	{
		return m_bones;
	}

	std::span<const SkeletonAttachment> Attachments() const noexcept
	{
		return m_attachments;
	}

private:
	std::vector<SkeletonBone> m_bones;
	std::vector<RenderTransform> m_world_rest_transforms;
	std::vector<SkeletonAttachment> m_attachments;
};

static_assert(std::is_nothrow_move_constructible_v<Skeleton>);
static_assert(std::is_nothrow_move_assignable_v<Skeleton>);

export using SkeletonPool = ResourcePool<Skeleton, SkeletonHandle>;

export SkeletonHandle Create_Skeleton(SkeletonPool &pool, const SkeletonDescription &description)
{
	if (!Validate_Skeleton_Description(description))
		return {};
	return pool.Create(description.bones, description.attachments);
}

}

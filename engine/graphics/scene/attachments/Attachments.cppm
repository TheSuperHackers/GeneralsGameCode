module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <vector>

export module Graphics.Scene.Attachments;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Resources.Pools.ResourcePool;
export import Graphics.Scene.RenderScene;
export import Graphics.Scene.Models.Skeleton;
export import Graphics.Scene.Models.Skinning;

namespace Graphics
{

export RenderTransform Identity_Render_Transform() noexcept
{
	RenderTransform transform;
	transform.matrix[0] = 1.0f;
	transform.matrix[5] = 1.0f;
	transform.matrix[10] = 1.0f;
	transform.matrix[15] = 1.0f;
	return transform;
}

export enum class AttachmentTargetKind : std::uint8_t
{
	Origin,
	Bone,
	Socket
};

export struct AttachmentTarget final
{
	AttachmentTargetKind kind = AttachmentTargetKind::Origin;
	BoneHandle bone{};
	AttachmentHandle socket{};
	RenderTransform local_transform = Identity_Render_Transform();
};

export struct AttachmentLinkDesc final
{
	InstanceHandle child{};
	InstanceHandle parent{};
	AttachmentTarget target{};
	RenderTransform child_local_transform = Identity_Render_Transform();
};

namespace
{
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

bool Same_Transform(const RenderTransform &left, const RenderTransform &right) noexcept
{
	for (std::size_t index = 0; index < left.matrix.size(); ++index) {
		if (left.matrix[index] != right.matrix[index])
			return false;
	}
	return true;
}

bool Valid_Target(const AttachmentTarget &target) noexcept
{
	switch (target.kind) {
		case AttachmentTargetKind::Origin:
			return true;
		case AttachmentTargetKind::Bone:
			return target.bone.Is_Valid();
		case AttachmentTargetKind::Socket:
			return target.socket.Is_Valid();
		default:
			return false;
	}
}
}

struct AttachmentLink final
{
	AttachmentLinkDesc description{};
};

static_assert(std::is_nothrow_move_constructible_v<AttachmentLink>);
static_assert(std::is_nothrow_move_assignable_v<AttachmentLink>);

export class AttachmentGraph final
{
public:
	void Reserve(std::size_t capacity)
	{
		m_links.Reserve(capacity);
		m_update_order.reserve(capacity);
		m_changed_children.reserve(capacity);
	}

	AttachmentLinkHandle Attach(RenderScene &scene, InstanceHandle child, InstanceHandle parent,
		const AttachmentTarget &target, const RenderTransform &child_local_transform = Identity_Render_Transform())
	{
		if (scene.Dense_Index(child) == Invalid_Render_Scene_Index || scene.Dense_Index(parent) == Invalid_Render_Scene_Index
			|| child == parent || !Valid_Target(target) || Has_Child_Link(child) || Would_Create_Cycle(child, parent))
			return {};

		const std::uint32_t child_slot = child.Get_Index();
		if (child_slot >= m_child_links.size())
			m_child_links.resize(static_cast<std::size_t>(child_slot) + 1);

		const AttachmentLinkDesc description{child, parent, target, child_local_transform};
		const AttachmentLinkHandle handle = m_links.Create(description);
		if (!handle.Is_Valid())
			return {};

		m_child_links[child_slot] = handle;
		m_update_order_dirty = true;
		return handle;
	}

	bool Detach(AttachmentLinkHandle handle) noexcept
	{
		const AttachmentLink *link = m_links.Resolve(handle);
		if (link == nullptr)
			return false;

		const InstanceHandle child = link->description.child;
		if (child.Get_Index() < m_child_links.size() && m_child_links[child.Get_Index()] == handle)
			m_child_links[child.Get_Index()] = {};
		m_update_order_dirty = true;
		return m_links.Destroy(handle);
	}

	bool Remove_Instance(InstanceHandle instance) noexcept
	{
		bool removed = false;
		for (;;) {
			AttachmentLinkHandle found;
			m_links.For_Each([&](AttachmentLinkHandle handle, const AttachmentLink &link) noexcept {
				if (!found.Is_Valid() && (link.description.child == instance || link.description.parent == instance))
					found = handle;
			});
			if (!found.Is_Valid())
				break;
			removed = Detach(found) || removed;
		}
		return removed;
	}

	void Clear() noexcept
	{
		for (;;) {
			AttachmentLinkHandle found;
			m_links.For_Each([&](AttachmentLinkHandle handle, const AttachmentLink &) noexcept {
				if (!found.Is_Valid())
					found = handle;
			});
			if (!found.Is_Valid())
				break;
			Detach(found);
		}
		m_child_links.clear();
		m_update_order.clear();
		m_changed_children.clear();
		m_update_order_dirty = false;
	}

	bool Update_Target(AttachmentLinkHandle handle, const AttachmentTarget &target) noexcept
	{
		if (!Valid_Target(target))
			return false;
		AttachmentLink *link = m_links.Resolve(handle);
		if (link == nullptr)
			return false;
		link->description.target = target;
		return true;
	}

	bool Update_Child_Local_Transform(AttachmentLinkHandle handle, const RenderTransform &transform) noexcept
	{
		AttachmentLink *link = m_links.Resolve(handle);
		if (link == nullptr)
			return false;
		link->description.child_local_transform = transform;
		return true;
	}

	bool Get(AttachmentLinkHandle handle, AttachmentLinkDesc &description) const noexcept
	{
		const AttachmentLink *link = m_links.Resolve(handle);
		if (link == nullptr)
			return false;
		description = link->description;
		return true;
	}

	bool Update(RenderScene &scene) noexcept
	{
		return Update_Implementation(scene, nullptr, nullptr);
	}

	bool Update(RenderScene &scene, const SkeletonPool &skeletons, const BoneMatrixTable &poses) noexcept
	{
		return Update_Implementation(scene, &skeletons, &poses);
	}

	std::size_t Size() const noexcept
	{
		return m_links.Size();
	}

	std::span<const InstanceHandle> Changed_Children() const noexcept
	{
		return m_changed_children;
	}

	void Clear_Changed() noexcept
	{
		m_changed_children.clear();
	}

	private:
	using LinkPool = ResourcePool<AttachmentLink, AttachmentLinkHandle>;

	bool Update_Implementation(RenderScene &scene, const SkeletonPool *skeletons, const BoneMatrixTable *poses) noexcept
	{
		m_changed_children.clear();
		if (m_links.Size() == 0)
			return true;
		if (m_changed_children.capacity() < m_links.Size() || !Build_Update_Order())
			return false;

		for (const AttachmentLinkHandle handle : m_update_order) {
			const AttachmentLink *link = m_links.Resolve(handle);
			if (link == nullptr || scene.Dense_Index(link->description.parent) == Invalid_Render_Scene_Index
				|| scene.Dense_Index(link->description.child) == Invalid_Render_Scene_Index)
				return false;
		}

		for (const AttachmentLinkHandle handle : m_update_order) {
			const AttachmentLink *link = m_links.Resolve(handle);
			RenderTransform parent_transform;
			RenderTransform child_transform;
			if (link == nullptr || !scene.Get_Transform(link->description.parent, parent_transform)
				|| !scene.Get_Transform(link->description.child, child_transform))
				return false;

			RenderTransform target_local_transform;
			if (!Build_Target_Transform(scene, link->description, skeletons, poses, target_local_transform))
				return false;
			const RenderTransform target_transform = Multiply_Transform(parent_transform, target_local_transform);
			const RenderTransform world_transform = Multiply_Transform(target_transform, link->description.child_local_transform);
			if (Same_Transform(child_transform, world_transform))
				continue;
			if (!scene.Update_Transform(link->description.child, world_transform)
				|| m_changed_children.size() >= m_changed_children.capacity())
				return false;
			m_changed_children.push_back(link->description.child);
		}
		return true;
	}

	bool Build_Target_Transform(RenderScene &scene, const AttachmentLinkDesc &description,
		const SkeletonPool *skeletons, const BoneMatrixTable *poses, RenderTransform &transform) const noexcept
	{
		if (skeletons == nullptr || poses == nullptr || description.target.kind == AttachmentTargetKind::Origin) {
			transform = description.target.local_transform;
			return true;
		}

		SkeletonHandle skeleton_handle;
		PoseHandle pose_handle;
		bool found = false;
		scene.Visit(description.parent, [&](InstanceHandle, const RenderInstanceView &view) noexcept {
			skeleton_handle = view.skeleton;
			pose_handle = view.pose;
			found = true;
		});
		if (!found)
			return false;
		const Skeleton *skeleton = skeletons->Resolve(skeleton_handle);
		if (skeleton == nullptr)
			return false;

		RenderTransform bone_transform;
		if (description.target.kind == AttachmentTargetKind::Bone) {
			if (!skeleton->Is_Valid_Bone(description.target.bone))
				return false;
			if (!poses->Transform(pose_handle, description.target.bone.Get_Index(), bone_transform)
				&& !skeleton->Rest_Transform(description.target.bone, bone_transform))
				return false;
		} else if (description.target.kind == AttachmentTargetKind::Socket) {
			if (!skeleton->Is_Valid_Attachment(description.target.socket))
				return false;
			const SkeletonAttachment &socket = skeleton->Attachments()[description.target.socket.Get_Index()];
			if (!poses->Transform(pose_handle, socket.bone.Get_Index(), bone_transform)
				&& !skeleton->Rest_Transform(socket.bone, bone_transform))
				return false;
			bone_transform = Multiply_Transform(bone_transform, socket.local_transform);
		} else {
			return false;
		}

		transform = Multiply_Transform(bone_transform, description.target.local_transform);
		return true;
	}

	AttachmentLinkHandle Link_For_Child(InstanceHandle child) const noexcept
	{
		if (!child.Is_Valid() || child.Get_Index() >= m_child_links.size())
			return {};
		const AttachmentLinkHandle handle = m_child_links[child.Get_Index()];
		return m_links.Resolve(handle) == nullptr ? AttachmentLinkHandle{} : handle;
	}

	bool Has_Child_Link(InstanceHandle child) const noexcept
	{
		return Link_For_Child(child).Is_Valid();
	}

	bool Would_Create_Cycle(InstanceHandle child, InstanceHandle parent) const noexcept
	{
		InstanceHandle current = parent;
		for (std::size_t count = 0; count <= m_links.Size(); ++count) {
			if (current == child)
				return true;
			const AttachmentLinkHandle parent_link = Link_For_Child(current);
			if (!parent_link.Is_Valid())
				return false;
			const AttachmentLink *link = m_links.Resolve(parent_link);
			if (link == nullptr)
				return false;
			current = link->description.parent;
		}
		return true;
	}

	bool Build_Update_Order() noexcept
	{
		if (!m_update_order_dirty)
			return true;
		m_update_order.clear();
		m_links.For_Each([&](AttachmentLinkHandle handle, const AttachmentLink &) noexcept {
			if (m_update_order.size() < m_update_order.capacity())
				m_update_order.push_back(handle);
		});
		if (m_update_order.size() != m_links.Size())
			return false;

		for (const AttachmentLinkHandle handle : m_update_order) {
			std::size_t depth = 0;
			if (!Link_Depth(handle, depth))
				return false;
		}

		std::sort(m_update_order.begin(), m_update_order.end(), [&](AttachmentLinkHandle left, AttachmentLinkHandle right) noexcept {
			std::size_t left_depth = 0;
			std::size_t right_depth = 0;
			Link_Depth(left, left_depth);
			Link_Depth(right, right_depth);
			if (left_depth != right_depth)
				return left_depth < right_depth;
			return left.Get_Index() < right.Get_Index();
		});
		m_update_order_dirty = false;
		return true;
	}

	bool Link_Depth(AttachmentLinkHandle handle, std::size_t &depth) const noexcept
	{
		depth = 0;
		AttachmentLinkHandle current = handle;
		for (;;) {
			const AttachmentLink *link = m_links.Resolve(current);
			if (link == nullptr)
				return false;
			const AttachmentLinkHandle parent_link = Link_For_Child(link->description.parent);
			if (!parent_link.Is_Valid())
				return true;
			if (++depth > m_links.Size())
				return false;
			current = parent_link;
		}
	}

	LinkPool m_links;
	std::vector<AttachmentLinkHandle> m_child_links;
	std::vector<AttachmentLinkHandle> m_update_order;
	std::vector<InstanceHandle> m_changed_children;
	bool m_update_order_dirty = true;
};

static_assert(std::is_nothrow_move_constructible_v<AttachmentGraph>);
static_assert(std::is_nothrow_move_assignable_v<AttachmentGraph>);

}

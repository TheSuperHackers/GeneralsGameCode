module;

#include <cstddef>
#include <string_view>

export module Graphics.Scene.RenderObjectHierarchy;

export import Graphics.Scene.RenderObjectState;

namespace Graphics
{

namespace RenderObjectHierarchyDetail
{

inline char Lower_Ascii(char value) noexcept
{
	return value >= 'A' && value <= 'Z' ? static_cast<char>(value - 'A' + 'a') : value;
}

inline bool Equal_No_Case(std::string_view left, std::string_view right) noexcept
{
	if (left.size() != right.size())
		return false;
	for (std::size_t index = 0; index < left.size(); ++index) {
		if (Lower_Ascii(left[index]) != Lower_Ascii(right[index]))
			return false;
	}
	return true;
}

inline std::string_view Suffix(std::string_view name) noexcept
{
	const std::size_t separator = name.find('.');
	return separator == std::string_view::npos ? name : name.substr(separator + 1);
}

}

// Validate a child's transform using the same root update ordering as the
// original scene graph: inspect the immediate parent, climb all ancestors,
// then update exactly the root container once. Identity is refreshed only
// when an ancestor marked its children dirty.
export template<class Object, class IsDirty, class ParentOf, class UpdateRoot, class RefreshIdentity>
bool Validate_Render_Object_Transform(
	Object *container,
	IsDirty &&is_dirty,
	ParentOf &&parent_of,
	UpdateRoot &&update_root,
	RefreshIdentity &&refresh_identity)
{
	bool dirty = false;
	Object *root = container;
	if (root != nullptr) {
		dirty = is_dirty(root);
		while (parent_of(root) != nullptr) {
			dirty = is_dirty(root) || dirty;
			root = parent_of(root);
		}
		if (dirty)
			update_root(root);
	}
	if (dirty)
		refresh_identity();
	return dirty;
}

// Get_Child must return an owned/retained pointer. Non-matching children are
// released before the next lookup, while the matching pointer is returned to
// the caller. Full names are checked before suffix names.
export template<class GetChild, class ReleaseChild, class NameOf>
auto Find_Render_Object_Child_By_Name(
	std::size_t count,
	std::string_view requested_name,
	GetChild &&get_child,
	ReleaseChild &&release_child,
	NameOf &&name_of,
	int *index = nullptr)
{
	using ChildPointer = decltype(get_child(std::size_t{}));
	for (int pass = 0; pass < 2; ++pass) {
		for (std::size_t child_index = 0; child_index < count; ++child_index) {
			ChildPointer child = get_child(child_index);
			if (child == nullptr)
				continue;
			const std::string_view child_name = name_of(*child);
			const std::string_view candidate = pass == 0
				? child_name
				: RenderObjectHierarchyDetail::Suffix(child_name);
			if (RenderObjectHierarchyDetail::Equal_No_Case(candidate, requested_name)) {
				if (index != nullptr)
					*index = static_cast<int>(child_index);
				return child;
			}
			release_child(child);
		}
	}
	return ChildPointer{};
}

// Aggregate renderer-visible child flags. Collision/query masks are left to
// the game adapter; this function only propagates graphics state.
export template<class GetChild, class ReleaseChild, class FlagsOf>
RenderObjectFlags Aggregate_Render_Object_Child_Flags(
	std::size_t count,
	GetChild &&get_child,
	ReleaseChild &&release_child,
	FlagsOf &&flags_of)
{
	constexpr RenderObjectFlags propagated =
		RenderObjectFlags::Translucent
		| RenderObjectFlags::Alpha
		| RenderObjectFlags::Additive;
	RenderObjectFlags result = RenderObjectFlags::None;
	for (std::size_t child_index = 0; child_index < count; ++child_index) {
		auto child = get_child(child_index);
		if (child == nullptr)
			continue;
		result |= flags_of(*child) & propagated;
		release_child(child);
	}
	return result;
}

}

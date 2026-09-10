module;

#include <array>
#include <cstddef>
#include <cstdint>

export module Graphics.Scene.Traversal;

export import Graphics.Scene.DrawContext;
export import Graphics.Scene.Lighting.Local;
export import Graphics.Scene.ObjectList;

namespace Graphics
{

// Registration lists retain the ordering contract of the original scene
// manager. Frame and release registrations are inserted at the head, while
// lights are appended so equal-priority lights keep their registration order.
export enum class SceneRegistration : std::uint8_t
{
	On_Frame_Update,
	Light,
	Release
};

export enum class SceneDrawResult : std::uint8_t
{
	Skipped,
	Submitted,
	Failed
};

export template<class Object>
struct SceneLifecycleCallbacks final
{
	void *context = nullptr;
	void (*added)(void *, Object *) = nullptr;
	void (*removed)(void *, Object *) = nullptr;
};

export template<class Object, class Camera>
struct SceneVisibilityCallbacks final
{
	void *context = nullptr;
	bool (*is_force_visible)(void *, const Object *) = nullptr;
	bool (*is_culled)(void *, const Object *, const Camera *) = nullptr;
	void (*set_visible)(void *, Object *, bool) = nullptr;
	bool (*is_really_visible)(void *, Object *) = nullptr;
	bool (*is_ignoring_lod_cost)(void *, Object *) = nullptr;
	void (*prepare_lod)(void *, Object *, Camera *) = nullptr;
};

export template<class Object, class Context>
struct SceneRenderCallbacks final
{
	void *context = nullptr;
	void (*on_frame_update)(void *, Object *) = nullptr;
	bool (*is_really_visible)(void *, Object *) = nullptr;
	// The draw callback owns the object render-hook policy. Keeping the hook
	// extraction in one callback avoids a second policy observing a hook that a
	// pre-render callback replaced or removed.
	SceneDrawResult (*draw)(void *, Object *, Context *) = nullptr;
};

export template<class Object>
struct SceneLightingCallbacks final
{
	void *context = nullptr;
	bool (*describe_light)(void *, const Object *, MaterialLightSource &) = nullptr;
};

export template<class Object>
struct SceneReleaseCallbacks final
{
	void *context = nullptr;
	void (*detach)(void *, Object *) = nullptr;
};

// The traversal component owns only scene membership and traversal policy. It
// receives object and camera operations through callbacks so rendering types
// remain in their owning game adapter.
export template<class Object>
class SceneTraversal final
{
public:
	using ObjectList = SceneObjectList<Object>;
	using Cursor = typename ObjectList::Cursor;

	explicit SceneTraversal(SceneLifecycleCallbacks<Object> lifecycle = {}) noexcept
		: m_lifecycle(lifecycle)
	{
	}

	SceneTraversal(const SceneTraversal &) = delete;
	SceneTraversal &operator=(const SceneTraversal &) = delete;

	~SceneTraversal()
	{
		Remove_All_Render_Objects();
	}

	void Set_Lifecycle_Callbacks(SceneLifecycleCallbacks<Object> lifecycle) noexcept
	{
		m_lifecycle = lifecycle;
	}

	bool Add_Render_Object(Object *object)
	{
		if (object == nullptr)
			return false;

		// Notify before insertion. Objects can register for updates or lighting
		// from their added callback, matching the legacy notification order.
		if (m_lifecycle.added != nullptr)
			m_lifecycle.added(m_lifecycle.context, object);
		if (!m_render_list.Add(object))
			return false;
		return true;
	}

	bool Remove_Render_Object(Object *object)
	{
		if (object == nullptr)
			return false;

		// Keep notification ahead of the final list release. The legacy base
		// notified even when the derived list no longer contained the object;
		// preserve that callback contract for unregistering object state.
		if (m_lifecycle.removed != nullptr)
			m_lifecycle.removed(m_lifecycle.context, object);
		return m_render_list.Remove(object);
	}

	void Remove_All_Render_Objects()
	{
		// Remove_Head transfers the retained collection reference. This is
		// intentionally separate from Remove_Render_Object, as the old scene
		// destruction path unlinked the render list before notifying objects.
		while (Object *object = m_render_list.Remove_Head()) {
			if (m_lifecycle.removed != nullptr)
				m_lifecycle.removed(m_lifecycle.context, object);
			object->Release_Ref();
		}
	}

	bool Register(Object *object, SceneRegistration registration)
	{
		if (object == nullptr)
			return false;

		switch (registration) {
		case SceneRegistration::On_Frame_Update:
			return m_update_list.Add(object);
		case SceneRegistration::Light:
			return m_light_list.Add_Tail(object);
		case SceneRegistration::Release:
			return m_release_list.Add(object);
		}
		return false;
	}

	bool Unregister(Object *object, SceneRegistration registration)
	{
		if (object == nullptr)
			return false;

		switch (registration) {
		case SceneRegistration::On_Frame_Update:
			return m_update_list.Remove(object);
		case SceneRegistration::Light:
			return m_light_list.Remove(object);
		case SceneRegistration::Release:
			return m_release_list.Remove(object);
		}
		return false;
	}

	ObjectList &Render_Objects() noexcept
	{
		return m_render_list;
	}

	const ObjectList &Render_Objects() const noexcept
	{
		return m_render_list;
	}

	ObjectList &Update_Objects() noexcept
	{
		return m_update_list;
	}

	const ObjectList &Update_Objects() const noexcept
	{
		return m_update_list;
	}

	ObjectList &Light_Objects() noexcept
	{
		return m_light_list;
	}

	const ObjectList &Light_Objects() const noexcept
	{
		return m_light_list;
	}

	ObjectList &Release_Objects() noexcept
	{
		return m_release_list;
	}

	const ObjectList &Release_Objects() const noexcept
	{
		return m_release_list;
	}

	void Set_Ambient_Light(std::array<float, 3> color) noexcept
	{
		m_ambient_light = color;
	}

	const std::array<float, 3> &Ambient_Light() const noexcept
	{
		return m_ambient_light;
	}

	bool Visibility_Checked() const noexcept
	{
		return m_visibility_checked;
	}

	template<class Camera>
	bool Is_Visibility_Checked_For(const Camera &camera) const noexcept
	{
		return m_visibility_checked && m_visibility_camera == &camera;
	}

	void Clear_Visibility() noexcept
	{
		m_visibility_checked = false;
		m_visibility_camera = nullptr;
	}

	// A game-specific visibility pass may do more than frustum culling (for
	// example, reflection eligibility or shroud state).  It can publish that
	// completed result here without asking the generic traversal to overwrite
	// the object's visibility flags.  The camera identity is still recorded so
	// a result cannot be reused by another view.
	template<class Camera>
	void Adopt_Visibility(Camera &camera) noexcept
	{
		m_visibility_checked = true;
		m_visibility_camera = &camera;
	}

	template<class Camera>
	bool Check_Visibility(Camera &camera, const SceneVisibilityCallbacks<Object, Camera> &callbacks)
	{
		if (callbacks.is_force_visible == nullptr || callbacks.is_culled == nullptr
			|| callbacks.set_visible == nullptr)
			return false;

		Cursor cursor(&m_render_list);
		for (cursor.First(); !cursor.Is_Done(); cursor.Next()) {
			Object *object = cursor.Peek_Obj();
			const bool visible = callbacks.is_force_visible(callbacks.context, object)
				? true
				: !callbacks.is_culled(callbacks.context, object, &camera);
			callbacks.set_visible(callbacks.context, object, visible);

			// LOD preparation follows visibility assignment. Objects which opt
			// out of the cost calculation must not receive the preparation call.
			const bool really_visible = callbacks.is_really_visible == nullptr
				? visible
				: callbacks.is_really_visible(callbacks.context, object);
			const bool ignore_lod = callbacks.is_ignoring_lod_cost != nullptr
				&& callbacks.is_ignoring_lod_cost(callbacks.context, object);
			if (really_visible && !ignore_lod && callbacks.prepare_lod != nullptr)
				callbacks.prepare_lod(callbacks.context, object, &camera);
		}
		m_visibility_checked = true;
		m_visibility_camera = &camera;
		return true;
	}

	template<class Context, class Camera>
	bool Render(Camera &camera, Context &context,
		const SceneVisibilityCallbacks<Object, Camera> &visibility,
		const SceneRenderCallbacks<Object, Context> &render,
		const SceneLightingCallbacks<Object> &lighting = {})
	{
		// A visibility result is valid only for the camera that produced it.
		// The result is consumed and invalidated below, so this identity cache
		// covers an explicit check immediately before a render while preventing
		// a reflection view from reusing the main-camera result.
		if ((!m_visibility_checked || m_visibility_camera != &camera)
			&& !Check_Visibility(camera, visibility))
			return false;
		m_visibility_checked = false;
		m_visibility_camera = nullptr;

		Cursor update_cursor(&m_update_list);
		for (update_cursor.First(); !update_cursor.Is_Done(); update_cursor.Next()) {
			if (render.on_frame_update != nullptr)
				render.on_frame_update(render.context, update_cursor.Peek_Obj());
		}

		if (context.light_environment == nullptr) {
			// The original game scene supplied a function-static lighting
			// environment and left the borrowed pointer installed in the draw
			// context through the complete scene pass. Keep that lifetime and
			// sharing rule here; the graphics boundary still owns no game state.
			static LocalLighting lighting_environment;
			lighting_environment.Reset({0.0f, 0.0f, 0.0f}, m_ambient_light);
			Cursor light_cursor(&m_light_list);
			for (light_cursor.First(); !light_cursor.Is_Done(); light_cursor.Next()) {
				if (lighting.describe_light == nullptr)
					continue;
				MaterialLightSource source;
				if (lighting.describe_light(lighting.context, light_cursor.Peek_Obj(), source))
					lighting_environment.Add(source);
			}
			lighting_environment.Finalize();
			context.light_environment = &lighting_environment;
		}

		bool success = true;
		Cursor render_cursor(&m_render_list);
		for (render_cursor.First(); !render_cursor.Is_Done(); render_cursor.Next()) {
			Object *object = render_cursor.Peek_Obj();
			if (render.is_really_visible != nullptr
				&& !render.is_really_visible(render.context, object))
				continue;

			if (render.draw != nullptr
				&& render.draw(render.context, object, &context) == SceneDrawResult::Failed)
				success = false;
		}

		// Keep the scene-owned environment installed until the caller discards
		// this render context. The original scene left this borrowed pointer in
		// place across its extra pass and post-render processing; the scene owns
		// the replacement for at least that complete render lifetime.
		return success;
	}

	void Process_Releases(const SceneReleaseCallbacks<Object> &callbacks)
	{
		// First let every registered object detach from its scene or container.
		// Only after this pass do we release the retained release-list entries.
		Cursor cursor(&m_release_list);
		for (cursor.First(); !cursor.Is_Done(); cursor.Next()) {
			if (callbacks.detach != nullptr)
				callbacks.detach(callbacks.context, cursor.Peek_Obj());
		}
		while (!m_release_list.Is_Empty())
			m_release_list.Release_Head();
	}

private:
	SceneLifecycleCallbacks<Object> m_lifecycle{};
	ObjectList m_render_list;
	ObjectList m_update_list;
	ObjectList m_light_list;
	ObjectList m_release_list;
	std::array<float, 3> m_ambient_light{0.5f, 0.5f, 0.5f};
	bool m_visibility_checked = false;
	const void *m_visibility_camera = nullptr;
};

}

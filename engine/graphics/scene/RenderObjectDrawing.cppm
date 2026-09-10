module;

export module Graphics.Scene.RenderObjectDrawing;

namespace Graphics
{

// Rendering policy stays generic: the object supplies its own hook and draw
// operation while this helper guarantees pre/post ordering and reports whether
// the object submitted work.
export template<class Object, class Context>
bool Extract_Render_Object_Draw(Object &object, Context &context)
{
	bool submitted = false;
	if (object.Get_Render_Hook() != nullptr) {
		// Keep the lookup immediately before each callback. A hook is allowed to
		// replace the object's hook while rendering, and the legacy contract
		// dispatches Post_Render through the hook currently installed then.
		if (object.Get_Render_Hook()->Pre_Render(&object, context)) {
			object.Render(context);
			submitted = true;
		}
		if (auto *post_hook = object.Get_Render_Hook(); post_hook != nullptr)
			post_hook->Post_Render(&object, context);
	} else {
		object.Render(context);
		submitted = true;
	}
	return submitted;
}

}

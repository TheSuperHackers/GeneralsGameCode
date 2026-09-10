module;
#include <cstdint>
#include <utility>

export module Graphics.Scene.Shadows.ProjectedCapture;
import Graphics.Frame.AttachmentBindings;
import Graphics.RHI;
import Graphics.Resources.Textures.Resource;

namespace Graphics
{
// Camera adapters store a normalized viewport, while a generated texture needs
// its one-pixel border expressed in the active target's pixel dimensions.
export RHIViewport Projected_Texture_Viewport(std::uint32_t width,
	std::uint32_t height) noexcept
{
	return {
		width > 2 ? 1u : 0u, height > 2 ? 1u : 0u,
		width > 2 ? width - 2 : width, height > 2 ? height - 2 : height,
		0.0f, 1.0f};
}

// Record one offscreen caster pass inside the caller's current frame. The
// context and callbacks are deliberately generic so game-side render objects
// remain outside engine/graphics.
export template<class Context, class Draw, class Flush>
bool Capture_Projected_Texture(AttachmentBindings &attachments, TextureResource *target,
	Context &context, Draw &&draw, Flush &&flush)
{
	if (target == nullptr) return false;
	AttachmentScope scope(attachments, target);
	if (!scope.Active()) return false;
	attachments.Clear(true, attachments.Current().depth.Is_Valid(), {1.0f, 1.0f, 1.0f, 0.0f});
	if (!std::forward<Draw>(draw)(context)) return false;
	return std::forward<Flush>(flush)(context);
}
}

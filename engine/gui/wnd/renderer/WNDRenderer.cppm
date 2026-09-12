module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <span>
#include <string_view>
#include <vector>

export module Engine.UI.WND;

export import Assets.Runtime;
export import Assets.Handles;
import Assets.Cache;
export import Assets.Fonts;
export import Assets.Textures;
export import Graphics.Renderer2D;
import Graphics.Text.GlyphAtlas;

namespace Engine::UI::WND
{

export struct Rect final
{
	std::int32_t left = 0;
	std::int32_t top = 0;
	std::int32_t right = 0;
	std::int32_t bottom = 0;
};

export struct ImageRef final
{
	Assets::TextureAssetHandle texture{};
	Graphics::Rect2D uv{0.0f, 0.0f, 1.0f, 1.0f};
	// Caller-owned generated images remain alive until the UI frame is submitted.
	Graphics::Renderer2DTexture generated{};
};

// WND draw data is shared by the document parser and every control renderer.
// Keeping it in the foundational module means controls do not need to depend on
// the parser (and lets tools construct controls without loading a .wnd file).
export constexpr std::size_t WND_Draw_Cell_Count = 9;

export struct WNDDrawCell final
{
	std::string image_name;
	ImageRef image{};
	std::uint32_t image_width = 0;
	std::uint32_t image_height = 0;
	Graphics::Color2D color{1.0f, 1.0f, 1.0f, 0.0f};
	Graphics::Color2D border_color{1.0f, 1.0f, 1.0f, 0.0f};
};

export struct WNDDrawState final
{
	std::array<WNDDrawCell, WND_Draw_Cell_Count> cells{};
};

export class ClipScope final
{
public:
	ClipScope(Graphics::Renderer2D &renderer, bool enabled, Graphics::Rect2D rectangle) noexcept
		: m_renderer(renderer),
		  m_previous(renderer.Get_Clip())
	{
		if (!enabled) {
			m_renderer.Set_Clip(m_previous.enabled, m_previous.rectangle);
			return;
		}

		Graphics::Rect2D clip = rectangle;
		if (m_previous.enabled) {
			clip.left = std::max(clip.left, m_previous.rectangle.left);
			clip.top = std::max(clip.top, m_previous.rectangle.top);
			clip.right = std::min(clip.right, m_previous.rectangle.right);
			clip.bottom = std::min(clip.bottom, m_previous.rectangle.bottom);
		}
		m_renderer.Set_Clip(true, clip);
	}

	~ClipScope() { m_renderer.Set_Clip(m_previous.enabled, m_previous.rectangle); }
	ClipScope(const ClipScope &) = delete;
	ClipScope &operator=(const ClipScope &) = delete;

private:
	Graphics::Renderer2D &m_renderer;
	Graphics::Renderer2D::ClipState m_previous;
};

export enum class BorderPiece : std::uint8_t
{
	CornerUpperLeft,
	CornerUpperRight,
	CornerLowerLeft,
	CornerLowerRight,
	VerticalLeft,
	VerticalLeftShort,
	HorizontalTop,
	HorizontalTopShort,
	VerticalRight,
	VerticalRightShort,
	HorizontalBottom,
	HorizontalBottomShort,
	Count
};

export struct BorderAtlas final
{
	std::array<ImageRef, static_cast<std::size_t>(BorderPiece::Count)> pieces{};
};

export enum class Layer : std::uint8_t
{
	Below,
	Normal,
	Above
};

export enum class VisualState : std::uint8_t
{
	Normal,
	Disabled,
	Highlighted,
	Selected
};

export constexpr VisualState Resolve_Visual_State(
	bool enabled, bool highlighted, bool selected) noexcept
{
	if (!enabled)
		return VisualState::Disabled;
	if (selected)
		return VisualState::Selected;
	if (highlighted)
		return VisualState::Highlighted;
	return VisualState::Normal;
}

export enum class WindowFlag : std::uint32_t
{
	None = 0,
	Hidden = 1u << 0,
	SeeThrough = 1u << 1,
	Border = 1u << 2,
	BorderBeforeChildren = 1u << 3,
	ClipChildren = 1u << 4
};

export constexpr std::uint32_t operator|(WindowFlag left, WindowFlag right) noexcept
{
	return static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right);
}

export constexpr bool Has_Flag(std::uint32_t flags, WindowFlag flag) noexcept
{
	return (flags & static_cast<std::uint32_t>(flag)) != 0;
}

export using NodeIndex = std::uint32_t;
export constexpr NodeIndex Invalid_Node = std::numeric_limits<NodeIndex>::max();

export class DrawList;

// The callbacks are deliberately opaque. The WND adapter supplies the legacy
// GameWindow and WinInstanceData pointers; this module owns traversal and
// ordering without importing the game UI or a graphics backend.
export using DrawDataCallback = bool (*)(
	void *context, void *window, void *instance_data, DrawList &draw_list) noexcept;

export struct RenderNode final
{
	void *window = nullptr;
	void *instance_data = nullptr;
	DrawDataCallback extract = nullptr;
	void *extract_context = nullptr;
	DrawDataCallback extract_border = nullptr;
	Layer layer = Layer::Normal;
	VisualState visual_state = VisualState::Normal;
	std::uint32_t flags = static_cast<std::uint32_t>(WindowFlag::None);
	Rect screen_region{};
	NodeIndex first_child = Invalid_Node;
	NodeIndex last_child = Invalid_Node;
	NodeIndex next_sibling = Invalid_Node;
	NodeIndex previous_sibling = Invalid_Node;
};

export using WindowLinkCallback = void *(*)(
	void *context, void *window) noexcept;
export using DescribeWindowCallback = bool (*)(
	void *context, void *window, RenderNode &node) noexcept;

export struct WindowTreeSource final
{
	void *context = nullptr;
	WindowLinkCallback next = nullptr;
	WindowLinkCallback child = nullptr;
	DescribeWindowCallback describe = nullptr;
};

export struct TextMetrics final
{
	const void *context = nullptr;
	int (*spacing)(const void *context, std::uint16_t character) noexcept = nullptr;
	int (*height)(const void *context) noexcept = nullptr;
	int (*extra_overlap)(const void *context) noexcept = nullptr;
};

export struct TextLayoutOptions final
{
	int wrapping_width = 0;
	bool centered = false;
	bool parse_hotkey = false;
	std::uint16_t hotkey = 0;
	bool hard_wrap = false;
};

export struct TextPlacement final
{
	std::uint16_t character = 0;
	float x = 0.0f;
	float y = 0.0f;
	std::uint32_t line = 0;
	bool hotkey = false;
};

export struct TextLine final
{
	std::uint32_t first = 0;
	std::uint32_t count = 0;
	float width = 0.0f;
};

export class TextLayout final
{
public:
	explicit TextLayout(std::size_t placement_capacity = 8192, std::size_t line_capacity = 1024)
		: m_placement_capacity(placement_capacity), m_line_capacity(line_capacity)
	{
		m_placements.reserve(placement_capacity);
		m_lines.reserve(line_capacity);
	}

	bool Build(TextMetrics metrics, const std::uint16_t *text, TextLayoutOptions options) noexcept
	{
		m_placements.clear();
		m_lines.clear();
		m_width = 0.0f;
		m_height = 0;
		if (text == nullptr || metrics.context == nullptr || metrics.spacing == nullptr
			|| metrics.height == nullptr || metrics.extra_overlap == nullptr)
			return false;

		const int line_height = std::max(1, metrics.height(metrics.context));
		float cursor_x = 0.0f;
		std::uint32_t line = 0;
		std::uint32_t line_start = 0;
		bool hotkey_seen = false;

		for (std::size_t index = 0; text[index] != 0;) {
			std::uint16_t character = text[index++];
			if (character == static_cast<std::uint16_t>('\n')) {
				if (!Finish_Line(cursor_x, line, line_start))
					return false;
				continue;
			}

			bool hotkey = false;
			if (options.parse_hotkey && character == static_cast<std::uint16_t>('&')
				&& text[index] != 0 && text[index] > static_cast<std::uint16_t>(' ')
				&& text[index] != static_cast<std::uint16_t>('\n')) {
				character = text[index++];
				hotkey = !hotkey_seen && (options.hotkey == 0 || character == options.hotkey);
				if (hotkey)
					hotkey_seen = true;
			}

			const float spacing = static_cast<float>(metrics.spacing(metrics.context, character));
			if (character == static_cast<std::uint16_t>(' ')) {
				if (options.wrapping_width > 0) {
					float word_width = spacing;
					for (std::size_t word_index = index; text[word_index] != 0
						&& text[word_index] > static_cast<std::uint16_t>(' '); ++word_index) {
						if (options.parse_hotkey && text[word_index] == static_cast<std::uint16_t>('&')
							&& text[word_index + 1] != 0
							&& text[word_index + 1] > static_cast<std::uint16_t>(' ')
							&& text[word_index + 1] != static_cast<std::uint16_t>('\n'))
							++word_index;
						if (text[word_index] != 0)
							word_width += static_cast<float>(metrics.spacing(metrics.context, text[word_index]));
					}
					if (cursor_x > 0.0f && cursor_x + word_width >= static_cast<float>(options.wrapping_width)) {
						if (!Finish_Line(cursor_x, line, line_start))
							return false;
						continue;
					}
				}
				cursor_x += spacing;
				continue;
			}

			if (options.hard_wrap && options.wrapping_width > 0 && cursor_x > 0.0f
				&& cursor_x + spacing >= static_cast<float>(options.wrapping_width)) {
				if (!Finish_Line(cursor_x, line, line_start))
					return false;
			}

			if (m_placements.size() >= m_placement_capacity)
				return false;
			m_placements.push_back({character, cursor_x, static_cast<float>(line * line_height), line, hotkey});
			cursor_x += spacing;
			if (hotkey)
				cursor_x += static_cast<float>(metrics.extra_overlap(metrics.context));
		}

		if (!Finish_Line(cursor_x, line, line_start))
			return false;

		for (const TextLine &current_line : m_lines)
			m_width = std::max(m_width, current_line.width);
		if (options.centered) {
			for (TextPlacement &placement : m_placements)
				placement.x += (m_width - m_lines[placement.line].width) * 0.5f;
		}
		m_height = static_cast<std::uint32_t>(m_lines.size()) * static_cast<std::uint32_t>(line_height);
		return true;
	}

	std::span<const TextPlacement> Placements() const noexcept { return m_placements; }
	std::span<const TextLine> Lines() const noexcept { return m_lines; }
	float Width() const noexcept { return m_width; }
	std::uint32_t Height() const noexcept { return m_height; }

private:
	bool Finish_Line(float &cursor_x, std::uint32_t &line, std::uint32_t &line_start) noexcept
	{
		if (m_lines.size() >= m_line_capacity || m_placements.size() > std::numeric_limits<std::uint32_t>::max())
			return false;
		m_lines.push_back({line_start, static_cast<std::uint32_t>(m_placements.size()) - line_start, cursor_x});
		line_start = static_cast<std::uint32_t>(m_placements.size());
		cursor_x = 0.0f;
		++line;
		return true;
	}

	std::size_t m_placement_capacity = 0;
	std::size_t m_line_capacity = 0;
	std::vector<TextPlacement> m_placements;
	std::vector<TextLine> m_lines;
	float m_width = 0.0f;
	std::uint32_t m_height = 0;
};

export struct FontGlyph final
{
	std::uint16_t character = 0;
	std::uint16_t width = 0;
	std::int16_t spacing = 0;
	Graphics::Rect2D uv{};
	bool available = false;
	std::uint32_t page = 0;
};

export class FontFace final
{
public:
	FontFace() = default;
	FontFace(const FontFace &) = delete;
	FontFace &operator=(const FontFace &) = delete;

    bool Build(const Assets::FontAsset &source)
    {
        if (source.Height()==0) return false;
        m_height=source.Height();
        m_extra_overlap=source.Extra_Overlap();
        m_atlas.Clear(); m_resource_ids.clear(); m_glyphs.clear();
        m_glyphs.reserve(source.Glyphs().size());
        for (const auto& input : source.Glyphs()) {
            FontGlyph glyph;
            glyph.character=input.character; glyph.width=input.width; glyph.spacing=input.spacing;
            if (input.width) {
                const auto region=m_atlas.Add(input.width,source.Height(),input.alpha);
                if (!region) return false;
                const float size=static_cast<float>(m_atlas.Page_Size());
                glyph.uv={region->x/size,region->y/size,(region->x+region->width)/size,(region->y+region->height)/size};
                glyph.page=region->page;
                glyph.available=true;
            }
            m_glyphs.push_back(glyph);
        }
        for (std::size_t page=0; page<m_atlas.Page_Count(); ++page) {
            const auto id=Allocate_Resource_Id();
            if (!id) return false;
            m_resource_ids.push_back(id);
        }
        return true;
    }

	int Height() const noexcept { return m_height; }
	int Extra_Overlap() const noexcept { return m_extra_overlap; }
	int Get_Char_Spacing(std::uint16_t character) const noexcept
	{
		const FontGlyph *glyph = Find_Glyph(character);
		return glyph != nullptr ? glyph->spacing : 0;
	}
	const FontGlyph *Get_Glyph(std::uint16_t character) const noexcept
	{
		const FontGlyph *glyph = Find_Glyph(character);
		if (glyph == nullptr || !glyph->available)
			return nullptr;
		return glyph;
	}

    Graphics::Renderer2DTexture Ensure_Texture(Graphics::Renderer2D &renderer, std::uint32_t page=0) const
    {
        if (page>=m_resource_ids.size()) return {};
        const auto size=m_atlas.Page_Size();
        return renderer.Register_Texture({Graphics::TextureHandle(m_resource_ids[page],1),
            size,size,size*4,1,std::as_bytes(m_atlas.Pixels(page))});
    }

private:
	const FontGlyph *Find_Glyph(std::uint16_t character) const noexcept
	{
		const auto found = std::lower_bound(m_glyphs.begin(), m_glyphs.end(), character,
			[](const FontGlyph &glyph, std::uint16_t value) { return glyph.character < value; });
		return found != m_glyphs.end() && found->character == character ? &*found : nullptr;
	}

	static std::uint32_t Allocate_Resource_Id() noexcept
	{
		static std::uint32_t next_id = 0x20000000u;
		if (next_id == 0x3fffffffu)
			return 0;
		return next_id++;
	}

	Graphics::GlyphAtlas m_atlas;
	std::vector<FontGlyph> m_glyphs;
	std::vector<std::uint32_t> m_resource_ids;
	int m_height = 0;
	int m_extra_overlap = 0;
};

export struct TextStyle final
{
	Graphics::Color2D color{};
	Graphics::Color2D drop_color{};
	Graphics::Color2D hotkey_color{};
	int x_drop = 1;
	int y_drop = 1;
};

export struct StaticTextContent final
{
	const FontFace *font = nullptr;
	const FontFace *hotkey_font = nullptr;
	const std::uint16_t *text = nullptr;
	TextLayoutOptions options{};
	TextStyle style{};
	bool clip = false;
	Graphics::Rect2D clip_rectangle{};
};

export enum class DrawCommandKind : std::uint8_t
{
	Rectangle,
	Outline,
	Line,
	Image,
	Clock,
	Text
};

export struct DrawCommand final
{
	DrawCommandKind kind = DrawCommandKind::Rectangle;
	Graphics::Rect2D rectangle{};
	Graphics::Point2D line_end{};
	ImageRef image{};
	Graphics::Color2D color{};
	Graphics::Color2D secondary_color{};
	float width = 1.0f;
	Graphics::Renderer2DBlendMode blend = Graphics::Renderer2DBlendMode::Alpha;
	bool remaining = false;
	bool grayscale = false;
	int percent = 0;
	const FontFace *font = nullptr;
	const FontFace *hotkey_font = nullptr;
	const std::uint16_t *text = nullptr;
	float text_x = 0.0f;
	float text_y = 0.0f;
	TextLayoutOptions text_options{};
	TextStyle text_style{};
	bool text_clip = false;
	Graphics::Rect2D text_clip_rectangle{};
};

export class DrawList final
{
public:
	explicit DrawList(std::size_t capacity = 4096)
		: m_capacity(capacity)
	{
		m_commands.reserve(capacity);
	}

	void Clear() noexcept { m_commands.clear(); }
	std::span<const DrawCommand> Commands() const noexcept { return m_commands; }
	std::size_t Size() const noexcept { return m_commands.size(); }
	std::size_t Capacity() const noexcept { return m_capacity; }

	bool Add_Rect(
		Graphics::Rect2D rectangle,
		Graphics::Color2D color,
		Graphics::Renderer2DBlendMode blend = Graphics::Renderer2DBlendMode::Alpha) noexcept
	{
		return Add({DrawCommandKind::Rectangle, rectangle, {}, {}, color, {}, 1.0f, blend});
	}

	bool Add_Outline(
		Graphics::Rect2D rectangle,
		float width,
		Graphics::Color2D color,
		Graphics::Renderer2DBlendMode blend = Graphics::Renderer2DBlendMode::Alpha) noexcept
	{
		DrawCommand command;
		command.kind = DrawCommandKind::Outline;
		command.rectangle = rectangle;
		command.color = color;
		command.secondary_color = color;
		command.width = width;
		command.blend = blend;
		return Add(command);
	}

	bool Add_Line(
		Graphics::Point2D start,
		Graphics::Point2D end,
		float width,
		Graphics::Color2D color,
		Graphics::Renderer2DBlendMode blend = Graphics::Renderer2DBlendMode::Alpha) noexcept
	{
		DrawCommand command;
		command.kind = DrawCommandKind::Line;
		command.rectangle = {start.x, start.y, start.x, start.y};
		command.line_end = end;
		command.color = color;
		command.secondary_color = color;
		command.width = width;
		command.blend = blend;
		return Add(command);
	}

	bool Add_Gradient_Line(
		Graphics::Point2D start, Graphics::Point2D end, float width,
		Graphics::Color2D start_color, Graphics::Color2D end_color) noexcept
	{
		DrawCommand command;
		command.kind = DrawCommandKind::Line;
		command.rectangle = {start.x, start.y, start.x, start.y};
		command.line_end = end;
		command.width = width;
		command.color = start_color;
		command.secondary_color = end_color;
		return Add(command);
	}

	bool Add_Image(
		ImageRef image,
		Graphics::Rect2D rectangle,
		Graphics::Color2D color = {},
		Graphics::Renderer2DBlendMode blend = Graphics::Renderer2DBlendMode::Alpha,
		bool grayscale = false) noexcept
	{
		DrawCommand command;
		command.kind = DrawCommandKind::Image;
		command.rectangle = rectangle;
		command.image = image;
		command.color = color;
		command.blend = blend;
		command.grayscale = grayscale;
		return Add(command);
	}

	bool Add_Window_Background(
		Graphics::Rect2D rectangle,
		bool has_image,
		ImageRef image,
		bool has_border,
		Graphics::Color2D border_color,
		bool has_fill,
		Graphics::Color2D fill_color,
		float border_width = 1.0f) noexcept
	{
		if (has_image && !Add_Image(image, rectangle))
			return false;
		if (has_border && !Add_Outline(rectangle, border_width, border_color))
			return false;
		if (has_fill) {
			const Graphics::Rect2D inside{
				rectangle.left + border_width,
				rectangle.top + border_width,
				rectangle.right - border_width,
				rectangle.bottom - border_width};
			if (!Add_Rect(inside, fill_color))
				return false;
		}
		return true;
	}

	bool Add_Clock(
		Graphics::Rect2D rectangle,
		int percent,
		Graphics::Color2D color,
		bool remaining = false,
		Graphics::Renderer2DBlendMode blend = Graphics::Renderer2DBlendMode::Alpha) noexcept
	{
		DrawCommand command;
		command.kind = DrawCommandKind::Clock;
		command.rectangle = rectangle;
		command.color = color;
		command.percent = percent;
		command.remaining = remaining;
		command.blend = blend;
		return Add(command);
	}

	bool Add_Text(
		const FontFace *font,
		const FontFace *hotkey_font,
		const std::uint16_t *text,
		float x,
		float y,
		TextLayoutOptions options,
		TextStyle style,
		bool clip = false,
		Graphics::Rect2D clip_rectangle = {}) noexcept
	{
		if (font == nullptr || text == nullptr)
			return false;
		DrawCommand command;
		command.kind = DrawCommandKind::Text;
		command.font = font;
		command.hotkey_font = hotkey_font;
		command.text = text;
		command.text_x = x;
		command.text_y = y;
		command.text_options = options;
		command.text_style = style;
		command.text_clip = clip;
		command.text_clip_rectangle = clip_rectangle;
		return Add(command);
	}

private:
	bool Add(const DrawCommand &command) noexcept
	{
		if (m_commands.size() >= m_capacity)
			return false;
		m_commands.push_back(command);
		return true;
	}

	std::size_t m_capacity = 0;
	std::vector<DrawCommand> m_commands;
};

export struct PushButtonVisual final
{
	Graphics::Rect2D rectangle{};

	bool has_image = false;
	ImageRef image{};
	Graphics::Color2D image_color{};
	bool grayscale = false;

	bool segmented = false;
	ImageRef left_image{};
	ImageRef middle_image{};
	ImageRef right_image{};
	float left_width = 0.0f;
	float middle_width = 0.0f;
	float right_width = 0.0f;

	bool has_fill = false;
	Graphics::Color2D fill_color{};
	bool has_border = false;
	Graphics::Color2D border_color{};
	float border_width = 1.0f;

	bool has_overlay = false;
	ImageRef overlay_image{};
	Graphics::Rect2D overlay_rectangle{};
	bool has_clock = false;
	Graphics::Rect2D clock_rectangle{};
	int clock_percent = 0;
	bool remaining_clock = false;
	Graphics::Color2D clock_color{};
	bool has_extra_border = false;
	Graphics::Rect2D extra_border{};
	Graphics::Color2D extra_border_color{};
	float extra_border_width = 1.0f;

	bool flashing = false;
	ImageRef flashing_image{};
	Graphics::Rect2D flashing_rectangle{};
	bool use_overlay_states = false;
	bool enabled = true;
	bool highlighted = false;
	bool selected = false;
	bool has_pushed_overlay = false;
	ImageRef pushed_overlay{};
	Graphics::Rect2D state_rectangle{};
	bool has_highlighted_overlay = false;
	ImageRef highlighted_overlay{};
};

export struct StaticTextVisual final
{
	Graphics::Rect2D rectangle{};
	bool has_image = false;
	ImageRef image{};
	Graphics::Rect2D image_rectangle{};
	bool has_fill = false;
	Graphics::Color2D fill_color{};
	bool has_border = false;
	Graphics::Color2D border_color{};
	float border_width = 1.0f;
	bool centered = false;
	bool centered_vertically = false;
	float left_margin = 0.0f;
	float top_margin = 0.0f;
	std::uint32_t text_width = 0;
	std::uint32_t text_height = 0;
};

export Graphics::Point2D Get_Static_Text_Position(const StaticTextVisual &visual) noexcept
{
	return {
		visual.centered
			? visual.rectangle.left
				+ (visual.rectangle.right - visual.rectangle.left) * 0.5f
				- static_cast<float>(visual.text_width) * 0.5f
			: visual.rectangle.left + visual.left_margin,
		visual.centered_vertically
			? visual.rectangle.top
				+ (visual.rectangle.bottom - visual.rectangle.top) * 0.5f
				- static_cast<float>(visual.text_height) * 0.5f
			: visual.rectangle.top + visual.top_margin};
}

export bool Add_Static_Text_Background(
	DrawList &draw_list,
	const StaticTextVisual &visual) noexcept
{
	if (visual.has_image && !draw_list.Add_Image(visual.image, visual.image_rectangle))
		return false;
	if (visual.has_border && !draw_list.Add_Outline(
		visual.rectangle, visual.border_width, visual.border_color))
		return false;
	if (visual.has_fill) {
		const Graphics::Rect2D inside{
			visual.rectangle.left + visual.border_width,
			visual.rectangle.top + visual.border_width,
			visual.rectangle.right - visual.border_width,
			visual.rectangle.bottom - visual.border_width};
		if (!draw_list.Add_Rect(inside, visual.fill_color))
			return false;
	}
	return true;
}

export struct CheckBoxVisual final
{
	Graphics::Rect2D rectangle{};
	Graphics::Rect2D box_rectangle{};
	bool has_background_image = false;
	ImageRef background_image{};
	Graphics::Rect2D background_image_rectangle{};
	bool has_background_fill = false;
	Graphics::Color2D background_fill{};
	bool has_background_border = false;
	Graphics::Color2D background_border{};
	bool has_box_image = false;
	ImageRef box_image{};
	Graphics::Rect2D box_image_rectangle{};
	bool has_box_fill = false;
	Graphics::Color2D box_fill{};
	bool has_box_border = false;
	Graphics::Color2D box_border{};
	bool checked = false;
	Graphics::Color2D check_color{};
	float line_width = 1.0f;
};

export Graphics::Point2D Get_Check_Box_Text_Position(
	const CheckBoxVisual &visual,
	std::uint32_t text_height) noexcept
{
	return {
		visual.rectangle.left + (visual.rectangle.bottom - visual.rectangle.top),
		visual.rectangle.top
			+ (visual.rectangle.bottom - visual.rectangle.top) * 0.5f
			- static_cast<float>(text_height) * 0.5f};
}

export bool Add_Check_Box_Visual(
	DrawList &draw_list,
	const CheckBoxVisual &visual) noexcept
{
	if (visual.has_background_image && !draw_list.Add_Image(
		visual.background_image, visual.background_image_rectangle))
		return false;
	if (visual.has_background_fill) {
		const Graphics::Rect2D inside{
			visual.rectangle.left + visual.line_width,
			visual.rectangle.top + visual.line_width,
			visual.rectangle.right - visual.line_width,
			visual.rectangle.bottom - visual.line_width};
		if (!draw_list.Add_Rect(inside, visual.background_fill))
			return false;
	}
	if (visual.has_background_border && !draw_list.Add_Outline(
		visual.rectangle, visual.line_width, visual.background_border))
		return false;
	if (visual.has_box_image && !draw_list.Add_Image(
		visual.box_image, visual.box_image_rectangle))
		return false;
	if (visual.has_box_fill && !draw_list.Add_Rect(visual.box_rectangle, visual.box_fill))
		return false;
	if (visual.has_box_border && !draw_list.Add_Outline(
		visual.box_rectangle, visual.line_width, visual.box_border))
		return false;
	if (visual.checked) {
		const Graphics::Point2D upper_left{
			visual.box_rectangle.left, visual.box_rectangle.top};
		const Graphics::Point2D lower_right{
			visual.box_rectangle.right, visual.box_rectangle.bottom};
		if (!draw_list.Add_Line(
				upper_left, lower_right, visual.line_width, visual.check_color)
			|| !draw_list.Add_Line(
				{visual.box_rectangle.left, visual.box_rectangle.bottom},
				{visual.box_rectangle.right, visual.box_rectangle.top},
				visual.line_width,
				visual.check_color))
			return false;
	}
	return true;
}

export struct RadioButtonVisual final
{
	Graphics::Rect2D rectangle{};
	bool segmented_images = false;
	ImageRef left_image{};
	ImageRef middle_image{};
	ImageRef right_image{};
	Graphics::Rect2D left_image_rectangle{};
	Graphics::Rect2D middle_image_rectangle{};
	Graphics::Rect2D right_image_rectangle{};
	float middle_width = 0.0f;
	bool has_background_fill = false;
	Graphics::Color2D background_fill{};
	bool has_background_border = false;
	Graphics::Color2D background_border{};
	float line_width = 1.0f;
	Graphics::Rect2D left_box_rectangle{};
	Graphics::Rect2D right_box_rectangle{};
	bool has_box_fill = false;
	Graphics::Color2D box_fill{};
};

export Graphics::Point2D Get_Radio_Button_Text_Position(
	const RadioButtonVisual &visual,
	std::uint32_t text_width,
	std::uint32_t text_height) noexcept
{
	return {
		visual.rectangle.left
			+ (visual.rectangle.right - visual.rectangle.left) * 0.5f
			- static_cast<float>(text_width) * 0.5f,
		visual.rectangle.top
			+ (visual.rectangle.bottom - visual.rectangle.top) * 0.5f
			- static_cast<float>(text_height) * 0.5f};
}

export bool Add_Radio_Button_Visual(
	DrawList &draw_list,
	const RadioButtonVisual &visual) noexcept
{
	if (visual.segmented_images) {
		const float middle_width = visual.middle_width;
		float cursor = visual.middle_image_rectangle.left;
		const float middle_right = visual.middle_image_rectangle.right;
		while (middle_width > 0.0f && cursor < middle_right) {
			const float width = std::min(middle_width, middle_right - cursor);
			ImageRef image = visual.middle_image;
			if (width < middle_width) {
				const float uv_width = image.uv.right - image.uv.left;
				image.uv.right = image.uv.left + uv_width * width / middle_width;
			}
			if (!draw_list.Add_Image(image, {
					cursor,
					visual.middle_image_rectangle.top,
					cursor + width,
					visual.middle_image_rectangle.bottom}))
				return false;
			cursor += width;
		}

		if (!draw_list.Add_Image(visual.left_image, visual.left_image_rectangle)
			|| !draw_list.Add_Image(visual.right_image, visual.right_image_rectangle))
			return false;
		return true;
	}
	else {
		if (visual.has_background_border && !draw_list.Add_Outline(
			visual.rectangle, visual.line_width, visual.background_border))
			return false;
		if (visual.has_background_fill) {
			const Graphics::Rect2D inside{
				visual.rectangle.left + visual.line_width,
				visual.rectangle.top + visual.line_width,
				visual.rectangle.right - visual.line_width,
				visual.rectangle.bottom - visual.line_width};
			if (!draw_list.Add_Rect(inside, visual.background_fill))
				return false;
		}
		const float left_separator = visual.left_box_rectangle.right + visual.line_width;
		const float right_separator = visual.right_box_rectangle.left;
		if (visual.has_background_border && !draw_list.Add_Line(
				{left_separator, visual.rectangle.top},
				{left_separator, visual.rectangle.bottom},
				visual.line_width,
				visual.background_border))
			return false;
		if (visual.has_box_fill && !draw_list.Add_Rect(visual.left_box_rectangle, visual.box_fill))
			return false;
		if (visual.has_background_border && !draw_list.Add_Line(
				{right_separator, visual.rectangle.top},
				{right_separator, visual.rectangle.bottom},
				visual.line_width,
				visual.background_border))
			return false;
		if (visual.has_box_fill && !draw_list.Add_Rect(visual.right_box_rectangle, visual.box_fill))
			return false;
		return true;
	}
}

export struct ProgressBarVisual final
{
	Graphics::Rect2D rectangle{};
	int progress = 0;
	float line_width = 1.0f;
	bool has_background_border = false;
	Graphics::Color2D background_border{};
	bool has_background_fill = false;
	Graphics::Color2D background_fill{};
	bool has_bar_border = false;
	Graphics::Color2D bar_border{};
	bool has_bar_fill = false;
	Graphics::Color2D bar_fill{};
};

export bool Add_Progress_Bar_Visual(
	DrawList &draw_list,
	const ProgressBarVisual &visual) noexcept
{
	if (visual.has_background_border && !draw_list.Add_Outline(
		visual.rectangle, visual.line_width, visual.background_border))
		return false;
	if (visual.has_background_fill) {
		const Graphics::Rect2D inside{
			visual.rectangle.left + visual.line_width,
			visual.rectangle.top + visual.line_width,
			visual.rectangle.right - visual.line_width,
			visual.rectangle.bottom - visual.line_width};
		if (!draw_list.Add_Rect(inside, visual.background_fill))
			return false;
	}
	if (visual.progress == 0)
		return true;

	const float width = (visual.rectangle.right - visual.rectangle.left)
		* static_cast<float>(visual.progress) / 100.0f;
	const Graphics::Rect2D bar{
		visual.rectangle.left,
		visual.rectangle.top,
		visual.rectangle.left + width,
		visual.rectangle.bottom};
	if (visual.has_bar_border && width > visual.line_width + 1.0f
		&& !draw_list.Add_Outline(bar, visual.line_width, visual.bar_border))
		return false;
	if (visual.has_bar_fill) {
		const Graphics::Rect2D inside{
			visual.rectangle.left + visual.line_width,
			visual.rectangle.top + visual.line_width,
			visual.rectangle.left + width - visual.line_width - 1.0f,
			visual.rectangle.bottom - visual.line_width};
		if (width > visual.line_width + 2.0f
			&& (!draw_list.Add_Rect(inside, visual.bar_fill)
				|| !draw_list.Add_Line(
					{inside.left, inside.top}, {inside.right, inside.top}, 1.0f,
					{1.0f, 1.0f, 1.0f, 1.0f})
				|| !draw_list.Add_Line(
					{inside.left, inside.top}, {inside.left, inside.bottom}, 1.0f,
					{200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 1.0f})))
			return false;
	}
	return true;
}

export bool Add_Tiled_Image(
	DrawList &draw_list,
	ImageRef image,
	Graphics::Rect2D rectangle,
	float tile_width,
	float &cursor) noexcept
{
	cursor = rectangle.left;
	if (tile_width <= 0.0f || rectangle.right <= rectangle.left)
		return true;
	while (cursor < rectangle.right) {
		const float width = std::min(tile_width, rectangle.right - cursor);
		if (width < tile_width) {
			const float uv_width = image.uv.right - image.uv.left;
			image.uv.right = image.uv.left + uv_width * width / tile_width;
		}
		if (!draw_list.Add_Image(image, {
				cursor, rectangle.top, cursor + width, rectangle.bottom}))
			return false;
		cursor += width;
	}
	return true;
}

export bool Add_Clipped_Image(
	DrawList &draw_list,
	ImageRef image,
	Graphics::Rect2D source_rectangle,
	Graphics::Rect2D clip_rectangle,
	Graphics::Color2D color = {}) noexcept
{
	const Graphics::Rect2D visible{
		std::max(source_rectangle.left, clip_rectangle.left),
		std::max(source_rectangle.top, clip_rectangle.top),
		std::min(source_rectangle.right, clip_rectangle.right),
		std::min(source_rectangle.bottom, clip_rectangle.bottom)};
	if (visible.right <= visible.left || visible.bottom <= visible.top)
		return true;

	const float width = source_rectangle.right - source_rectangle.left;
	const float height = source_rectangle.bottom - source_rectangle.top;
	if (width > 0.0f) {
		const float uv_width = image.uv.right - image.uv.left;
		image.uv.left += uv_width * (visible.left - source_rectangle.left) / width;
		image.uv.right = image.uv.left + uv_width * (visible.right - visible.left) / width;
	}
	if (height > 0.0f) {
		const float uv_height = image.uv.bottom - image.uv.top;
		image.uv.top += uv_height * (visible.top - source_rectangle.top) / height;
		image.uv.bottom = image.uv.top + uv_height * (visible.bottom - visible.top) / height;
	}
	return draw_list.Add_Image(image, visible, color);
}

bool Add_Clipped_Tiled_Image(
	DrawList &draw_list,
	ImageRef image,
	Graphics::Rect2D source_rectangle,
	Graphics::Rect2D clip_rectangle,
	float tile_width,
	float &cursor) noexcept
{
	cursor = source_rectangle.left;
	if (tile_width <= 0.0f || source_rectangle.right <= source_rectangle.left)
		return true;
	while (cursor < source_rectangle.right) {
		const float width = std::min(tile_width, source_rectangle.right - cursor);
		ImageRef piece = image;
		if (width < tile_width) {
			const float uv_width = image.uv.right - image.uv.left;
			piece.uv.right = piece.uv.left + uv_width * width / tile_width;
		}
		if (!Add_Clipped_Image(draw_list, piece,
			{cursor, source_rectangle.top, cursor + width, source_rectangle.bottom},
			clip_rectangle))
			return false;
		cursor += width;
	}
	return true;
}

export struct TextEntryVisual final
{
	Graphics::Rect2D rectangle{};
	bool segmented_image = false;
	ImageRef left_image{};
	ImageRef center_image{};
	ImageRef small_center_image{};
	ImageRef right_image{};
	float image_offset_x = 0.0f;
	float image_offset_y = 0.0f;
	float left_width = 0.0f;
	float right_width = 0.0f;
	float center_width = 0.0f;
	float small_center_width = 0.0f;
	bool has_fill = false;
	Graphics::Color2D fill{};
	bool has_border = false;
	Graphics::Color2D border{};
	float border_width = 1.0f;
};

export bool Add_Text_Entry_Background(
	DrawList &draw_list,
	const TextEntryVisual &visual) noexcept
{
	if (!visual.segmented_image) {
		if (visual.has_border && !draw_list.Add_Outline(
			visual.rectangle, visual.border_width, visual.border))
			return false;
		if (visual.has_fill) {
			const Graphics::Rect2D inside{
				visual.rectangle.left + visual.border_width,
				visual.rectangle.top + visual.border_width,
				visual.rectangle.right - visual.border_width,
				visual.rectangle.bottom - visual.border_width};
			if (!draw_list.Add_Rect(inside, visual.fill))
				return false;
		}
		return true;
	}

	const float left = visual.rectangle.left + visual.image_offset_x;
	const float top = visual.rectangle.top + visual.image_offset_y;
	const float right = visual.rectangle.right + visual.image_offset_x;
	const float bottom = visual.rectangle.bottom + visual.image_offset_y;
	const float left_end = left + visual.left_width;
	const float right_start = right - visual.right_width;
	float cursor = left_end;
	if (!Add_Tiled_Image(draw_list, visual.center_image,
		{left_end, top, right_start, bottom}, visual.center_width, cursor))
		return false;
	if (cursor < right_start && !Add_Tiled_Image(draw_list, visual.small_center_image,
		{cursor, top, right_start, bottom}, visual.small_center_width, cursor))
		return false;
	if (!draw_list.Add_Image(visual.left_image, {left, top, left_end, bottom}))
		return false;
	return draw_list.Add_Image(visual.right_image, {right_start, top, right, bottom});
}

export struct TextEntryTextVisual final
{
	const FontFace *font = nullptr;
	const std::uint16_t *text = nullptr;
	const FontFace *composite_font = nullptr;
	const std::uint16_t *composite_text = nullptr;
	Graphics::Color2D text_color{};
	Graphics::Color2D text_drop_color{};
	Graphics::Color2D composite_color{};
	Graphics::Color2D composite_drop_color{};
	Graphics::Rect2D clip_rectangle{};
	Graphics::Rect2D cursor_rectangle{};
	float x = 0.0f;
	float y = 0.0f;
	float visible_width = 0.0f;
	float font_height = 0.0f;
	int text_width = 0;
	int composite_width = 0;
	bool draw_from_start = false;
	int composite_cursor_width = 0;
	bool has_composite = false;
	bool show_cursor = false;
	Graphics::Color2D cursor_color{};
};

export bool Add_Text_Entry_Text(
	DrawList &draw_list,
	const TextEntryTextVisual &visual) noexcept
{
	if (visual.font == nullptr || visual.text == nullptr)
		return true;

	float draw_x = visual.x;
	if (visual.draw_from_start) {
		draw_x += 5.0f;
	}
	else {
		const int half_width = static_cast<int>(visual.visible_width) / 2;
		if (visual.text_width < visual.visible_width) {
			draw_x += 2.0f;
		}
		else if (half_width > 0) {
			const int divisor = visual.text_width / half_width - 1;
			draw_x += 2.0f - static_cast<float>(divisor * half_width);
		}
	}

	const bool clip_text = visual.clip_rectangle.right > visual.clip_rectangle.left
		&& visual.clip_rectangle.bottom > visual.clip_rectangle.top;
	if (!draw_list.Add_Text(
		visual.font, nullptr, visual.text, draw_x, visual.y,
		{}, {visual.text_color, visual.text_drop_color, {}, 1, 1},
		clip_text, visual.clip_rectangle))
		return false;

	if (visual.has_composite && visual.composite_font != nullptr && visual.composite_text != nullptr) {
		if (!draw_list.Add_Text(
			visual.composite_font, nullptr, visual.composite_text,
			draw_x + static_cast<float>(visual.text_width), visual.y,
			{}, {visual.composite_color, visual.composite_drop_color, {}, 1, 1},
			clip_text, visual.clip_rectangle))
			return false;
	}

	if (visual.show_cursor && !draw_list.Add_Rect(
		visual.cursor_rectangle, visual.cursor_color))
		return false;
	return true;
}

export struct ProgressBarImageVisual final
{
	Graphics::Rect2D rectangle{};
	ImageRef background_left{};
	ImageRef background_center{};
	ImageRef background_right{};
	Graphics::Rect2D background_left_rectangle{};
	Graphics::Rect2D background_center_rectangle{};
	Graphics::Rect2D background_right_rectangle{};
	float background_center_width = 0.0f;
	ImageRef bar_center{};
	ImageRef bar_right{};
	Graphics::Rect2D bar_rectangle{};
	float bar_center_width = 0.0f;
	float bar_right_width = 0.0f;
};

export bool Add_Progress_Bar_Image_Visual(
	DrawList &draw_list,
	const ProgressBarImageVisual &visual) noexcept
{
	if (!draw_list.Add_Image(visual.background_left, visual.background_left_rectangle))
		return false;
	float cursor = visual.background_center_rectangle.left;
	if (!Add_Tiled_Image(
			draw_list,
			visual.background_center,
			visual.background_center_rectangle,
			visual.background_center_width,
			cursor)
		|| !draw_list.Add_Image(visual.background_right, visual.background_right_rectangle))
		return false;

	if (visual.bar_rectangle.right <= visual.bar_rectangle.left)
		return true;
	if (!Add_Tiled_Image(
			draw_list,
			visual.bar_center,
			visual.bar_rectangle,
			visual.bar_center_width,
			cursor))
		return false;
	if (cursor < visual.bar_rectangle.right) {
		const Graphics::Rect2D remainder{
			cursor,
			visual.bar_rectangle.top,
			visual.bar_rectangle.right,
			visual.bar_rectangle.bottom};
		if (!Add_Tiled_Image(
				draw_list,
				visual.bar_right,
				remainder,
				visual.bar_right_width,
				cursor))
			return false;
	}
	return true;
}

export struct SliderVisual final
{
	Graphics::Rect2D rectangle{};
	float line_width = 1.0f;
	bool has_background_border = false;
	Graphics::Color2D background_border{};
	bool has_background_fill = false;
	Graphics::Color2D background_fill{};
};

export bool Add_Slider_Visual(
	DrawList &draw_list,
	const SliderVisual &visual) noexcept
{
	if (visual.has_background_border && !draw_list.Add_Outline(
		visual.rectangle, visual.line_width, visual.background_border))
		return false;
	if (!visual.has_background_fill)
		return true;
	const Graphics::Rect2D inside{
		visual.rectangle.left + visual.line_width,
		visual.rectangle.top + visual.line_width,
		visual.rectangle.right - visual.line_width,
		visual.rectangle.bottom - visual.line_width};
	return draw_list.Add_Rect(inside, visual.background_fill);
}

export struct HorizontalSliderImageVisual final
{
	ImageRef highlighted_image{};
	ImageRef selected_image{};
	ImageRef unselected_image{};
	Graphics::Point2D origin{};
	float box_width = 0.0f;
	float box_padding = 2.0f;
	int box_count = 0;
	int selected_box_count = 0;
	bool highlighted = false;
};

export void Layout_Horizontal_Slider_Images(HorizontalSliderImageVisual &visual,
	Graphics::Rect2D rectangle, float image_width, float scale,
	int minimum, int maximum, int position) noexcept
{
	visual.box_width = (std::max)(1.0f, image_width * scale);
	visual.box_padding = (std::max)(1.0f, 2.0f * scale);
	visual.box_count = 0;
	visual.selected_box_count = 0;
	const float selected = maximum != minimum ? float(position - minimum) / (maximum - minimum) : 0;
	const float selected_end = rectangle.left + selected * (rectangle.right - rectangle.left);
	float start = rectangle.left;
	float end = start + visual.box_width;
	while (end < rectangle.right) {
		if (start <= selected_end && position != minimum)
			++visual.selected_box_count;
		++visual.box_count;
		start = end + visual.box_padding;
		end = start + visual.box_width;
	}
	const float covered = end - visual.box_width - rectangle.left;
	visual.origin = {rectangle.left + (rectangle.right - rectangle.left - covered) * 0.5f, rectangle.top};
}

export bool Add_Horizontal_Slider_Image_Visual(
	DrawList &draw_list,
	const HorizontalSliderImageVisual &visual) noexcept
{
	if (visual.highlighted) {
		const float offset_x = -(visual.box_width + visual.box_padding) * 0.5f;
		const float offset_y = visual.box_width / 3.0f;
		for (int index = 0; index <= visual.box_count; ++index) {
			const float left = visual.origin.x + offset_x
				+ static_cast<float>(index) * (visual.box_width + visual.box_padding);
			const Graphics::Rect2D rectangle{
				left,
				visual.origin.y + offset_y,
				left + visual.box_width + visual.box_padding,
				visual.origin.y + offset_y + visual.box_width + visual.box_padding};
			if (!draw_list.Add_Image(visual.highlighted_image, rectangle))
				return false;
		}
	}

	for (int index = 0; index < visual.box_count; ++index) {
		const float left = visual.origin.x
			+ static_cast<float>(index) * (visual.box_width + visual.box_padding);
		const Graphics::Rect2D rectangle{
			left, visual.origin.y, left + visual.box_width,
			visual.origin.y + visual.box_width};
		if (!draw_list.Add_Image(
				index < visual.selected_box_count
					? visual.selected_image : visual.unselected_image,
				rectangle))
			return false;
	}
	return true;
}

export bool Add_Vertical_Tiled_Image(
	DrawList &draw_list,
	ImageRef image,
	Graphics::Rect2D rectangle,
	float tile_height,
	float &cursor) noexcept
{
	cursor = rectangle.top;
	if (tile_height <= 0.0f || rectangle.bottom <= rectangle.top)
		return true;
	while (cursor < rectangle.bottom) {
		const float height = std::min(tile_height, rectangle.bottom - cursor);
		if (height < tile_height) {
			const float uv_height = image.uv.bottom - image.uv.top;
			image.uv.bottom = image.uv.top + uv_height * height / tile_height;
		}
		if (!draw_list.Add_Image(image, {
				rectangle.left, cursor, rectangle.right, cursor + height}))
			return false;
		cursor += height;
	}
	return true;
}

export struct VerticalSliderImageVisual final
{
	ImageRef top_image{};
	ImageRef bottom_image{};
	ImageRef center_image{};
	ImageRef small_center_image{};
	Graphics::Rect2D top_rectangle{};
	Graphics::Rect2D bottom_rectangle{};
	Graphics::Rect2D center_rectangle{};
	Graphics::Rect2D small_center_rectangle{};
	float center_height = 0.0f;
	float small_center_height = 0.0f;
	bool compact = false;
};

export bool Add_Vertical_Slider_Image_Visual(
	DrawList &draw_list,
	const VerticalSliderImageVisual &visual) noexcept
{
	if (visual.compact) {
		return draw_list.Add_Image(visual.top_image, visual.top_rectangle)
			&& draw_list.Add_Image(visual.bottom_image, visual.bottom_rectangle);
	}

	float cursor = visual.center_rectangle.top;
	if (!Add_Vertical_Tiled_Image(
			draw_list, visual.center_image, visual.center_rectangle,
			visual.center_height, cursor))
		return false;
	if (cursor < visual.small_center_rectangle.bottom) {
		Graphics::Rect2D remainder = visual.small_center_rectangle;
		remainder.top = cursor;
		if (!Add_Vertical_Tiled_Image(
				draw_list, visual.small_center_image, remainder,
				visual.small_center_height, cursor))
			return false;
	}
	return draw_list.Add_Image(visual.top_image, visual.top_rectangle)
		&& draw_list.Add_Image(visual.bottom_image, visual.bottom_rectangle);
}

export constexpr std::size_t Maximum_Tab_Count = 8;

export struct TabControlVisual final
{
	bool has_background_image = false;
	ImageRef background_image{};
	Graphics::Rect2D background_image_rectangle{};
	bool has_background_fill = false;
	Graphics::Color2D background_fill{};
	bool has_background_border = false;
	Graphics::Color2D background_border{};
	std::size_t count = 0;
	std::array<Graphics::Rect2D, Maximum_Tab_Count> rectangles{};
	std::array<ImageRef, Maximum_Tab_Count> images{};
	std::array<Graphics::Color2D, Maximum_Tab_Count> fills{};
	std::array<Graphics::Color2D, Maximum_Tab_Count> borders{};
	std::array<bool, Maximum_Tab_Count> has_images{};
	std::array<bool, Maximum_Tab_Count> has_fills{};
	std::array<bool, Maximum_Tab_Count> has_borders{};
};

export bool Add_Tab_Control_Visual(
	DrawList &draw_list,
	const TabControlVisual &visual) noexcept
{
	if (visual.has_background_image && !draw_list.Add_Image(
		visual.background_image, visual.background_image_rectangle))
		return false;
	if (visual.has_background_fill) {
		if (!draw_list.Add_Rect(visual.background_image_rectangle, visual.background_fill))
			return false;
	}
	if (visual.has_background_border && !draw_list.Add_Outline(
		visual.background_image_rectangle, 1.0f, visual.background_border))
		return false;
	const std::size_t count = std::min(visual.count, Maximum_Tab_Count);
	for (std::size_t index = 0; index < count; ++index) {
		if (visual.has_images[index]) {
			if (!draw_list.Add_Image(visual.images[index], visual.rectangles[index]))
				return false;
			continue;
		}
		if (visual.has_borders[index] && !draw_list.Add_Outline(
			visual.rectangles[index], 1.0f, visual.borders[index]))
			return false;
		if (visual.has_fills[index]) {
			const Graphics::Rect2D inside{
				visual.rectangles[index].left + 1.0f,
				visual.rectangles[index].top + 1.0f,
				visual.rectangles[index].right - 1.0f,
				visual.rectangles[index].bottom - 1.0f};
			if (!draw_list.Add_Rect(inside, visual.fills[index]))
				return false;
		}
	}
	return true;
}

export struct ListBoxSelectionVisual final
{
	bool segmented_image = false;
	Graphics::Rect2D rectangle{};
	bool has_fill = false;
	Graphics::Color2D fill{};
	bool has_border = false;
	Graphics::Color2D border{};
	ImageRef left_image{};
	ImageRef center_image{};
	ImageRef small_center_image{};
	ImageRef right_image{};
	Graphics::Rect2D left_image_rectangle{};
	Graphics::Rect2D center_image_rectangle{};
	Graphics::Rect2D small_center_image_rectangle{};
	Graphics::Rect2D right_image_rectangle{};
	Graphics::Rect2D source_rectangle{};
	Graphics::Rect2D clip_rectangle{};
	float center_width = 0.0f;
	float small_center_width = 0.0f;
	float left_width = 0.0f;
	float right_width = 0.0f;
	float piece_height = 0.0f;
};

export bool Add_List_Box_Selection_Visual(
	DrawList &draw_list,
	const ListBoxSelectionVisual &visual) noexcept
{
	if (!visual.segmented_image) {
		if (visual.has_border && !draw_list.Add_Outline(visual.rectangle, 1.0f, visual.border))
			return false;
		if (!visual.has_fill)
			return true;
		const Graphics::Rect2D inside{
			visual.rectangle.left + 1.0f,
			visual.rectangle.top + 1.0f,
			visual.rectangle.right - 1.0f,
			visual.rectangle.bottom - 1.0f};
		return draw_list.Add_Rect(inside, visual.fill);
	}

	const Graphics::Rect2D source = visual.source_rectangle.right > visual.source_rectangle.left
		? visual.source_rectangle : visual.rectangle;
	const Graphics::Rect2D clip = visual.clip_rectangle.right > visual.clip_rectangle.left
		? visual.clip_rectangle : source;
	const float left_end = std::min(source.left + visual.left_width, source.right);
	const float right_start = std::max(left_end, source.right - visual.right_width);
	if (!Add_Clipped_Image(draw_list, visual.left_image,
		{source.left, source.top, left_end, source.bottom}, clip))
		return false;
	float cursor = left_end;
	if (!Add_Clipped_Tiled_Image(draw_list, visual.center_image,
		{left_end, source.top, right_start, source.bottom}, clip,
		visual.center_width, cursor))
		return false;
	if (cursor < right_start && !Add_Clipped_Tiled_Image(draw_list,
		visual.small_center_image,
		{cursor, source.top, right_start, source.bottom}, clip,
		visual.small_center_width, cursor))
			return false;
	return Add_Clipped_Image(draw_list, visual.right_image,
		{right_start, source.top, source.right, source.bottom}, clip);
}

export struct ListBoxRowVisual final
{
	Graphics::Rect2D rectangle{};
	bool selected = false;
};

export using ListBoxRowQuery = bool (*)(
	void *context, std::uint32_t row, ListBoxRowVisual &visual) noexcept;
export using ListBoxCellEmitter = bool (*)(
	void *context,
	DrawList &draw_list,
	std::uint32_t row,
	std::uint32_t column,
	Graphics::Rect2D row_rectangle,
	Graphics::Rect2D cell_clip) noexcept;

export struct ListBoxVisual final
{
	Graphics::Rect2D clip_rectangle{};
	std::uint32_t row_count = 0;
	std::uint32_t column_count = 0;
	ListBoxSelectionVisual selection{};
	void *context = nullptr;
	ListBoxRowQuery query_row = nullptr;
	ListBoxCellEmitter emit_cell = nullptr;
};

export bool Add_List_Box_Visual(
	DrawList &draw_list,
	const ListBoxVisual &visual) noexcept
{
	if (visual.query_row == nullptr || visual.emit_cell == nullptr)
		return false;

	for (std::uint32_t row_index = 0; row_index < visual.row_count; ++row_index) {
		ListBoxRowVisual row;
		if (!visual.query_row(visual.context, row_index, row))
			return false;
		const Graphics::Rect2D clipped_row{
			std::max(row.rectangle.left, visual.clip_rectangle.left),
			std::max(row.rectangle.top, visual.clip_rectangle.top),
			std::min(row.rectangle.right, visual.clip_rectangle.right),
			std::min(row.rectangle.bottom, visual.clip_rectangle.bottom)};
		if (clipped_row.right <= clipped_row.left || clipped_row.bottom <= clipped_row.top)
			continue;

		if (row.selected) {
			ListBoxSelectionVisual selection = visual.selection;
			selection.rectangle = clipped_row;
			if (selection.segmented_image) {
				const float left = clipped_row.left;
				const float right = clipped_row.right;
				const float top = clipped_row.top;
				const float bottom = clipped_row.bottom;
				const float left_end = std::min(left + selection.left_width, right);
				const float right_start = std::max(left_end,
					right - selection.right_width);
				selection.source_rectangle = row.rectangle;
				selection.clip_rectangle = clipped_row;
				selection.left_image_rectangle = {left, top, left_end, bottom};
				selection.right_image_rectangle = {right_start, top, right, bottom};
				selection.center_image_rectangle = {left_end, top, right_start, bottom};
				selection.small_center_image_rectangle = selection.center_image_rectangle;
				selection.piece_height = row.rectangle.bottom - row.rectangle.top;
			}
			if (!Add_List_Box_Selection_Visual(draw_list, selection))
				return false;
		}

		for (std::uint32_t column = 0; column < visual.column_count; ++column) {
			if (!visual.emit_cell(
				visual.context, draw_list, row_index, column, row.rectangle, clipped_row))
				return false;
		}
	}
	return true;
}

export bool Add_Push_Button_Background(
	DrawList &draw_list,
	const PushButtonVisual &button) noexcept
{
	if (button.segmented) {
		const float left = button.rectangle.left;
		const float top = button.rectangle.top;
		const float right = button.rectangle.right;
		const float bottom = button.rectangle.bottom;
		const float left_end = left + button.left_width;
		const float right_start = right - button.right_width;
		const float center_width = right_start - left_end;

		if (center_width <= 0.0f) {
			const float middle = left + (right - left) * 0.5f;
			if (!draw_list.Add_Image(button.left_image, {left, top, middle, bottom},
					button.image_color, Graphics::Renderer2DBlendMode::Alpha, button.grayscale)
				|| !draw_list.Add_Image(button.right_image, {middle, top, right, bottom},
					button.image_color, Graphics::Renderer2DBlendMode::Alpha, button.grayscale))
				return false;
		}
		else {
			// Segmented atlas images are sampled with linear filtering.  Adjacent
			// quads that only touch at an integer edge can therefore expose the
			// transparent/filter border of the neighboring atlas cell, especially
			// after the authored coordinates have been scaled for high-DPI output.
			// Keep the authored layout, but overlap the tiled destination quads by
			// half a physical pixel so their coverage remains continuous.
			constexpr float seam_overlap = 0.5f;
			auto seam_free_rectangle = [&](float segment_left, float segment_right) {
				return Graphics::Rect2D{
					std::max(left, segment_left - seam_overlap),
					top,
					std::min(right, segment_right + seam_overlap),
					bottom};
			};

			float cursor = left_end;
			if (button.middle_width > 0.0f) {
				const int pieces = static_cast<int>(center_width / button.middle_width);
				for (int piece = 0; piece < pieces; ++piece) {
					const float segment_right = cursor + button.middle_width;
					if (!draw_list.Add_Image(button.middle_image,
							seam_free_rectangle(cursor, segment_right),
							button.image_color,
							Graphics::Renderer2DBlendMode::Alpha,
							button.grayscale))
						return false;
					cursor = segment_right;
				}

				const float remainder = right_start - cursor;
				if (remainder > 0.0f) {
					ImageRef partial = button.middle_image;
					const float uv_width = partial.uv.right - partial.uv.left;
					partial.uv.right = partial.uv.left + uv_width * remainder / button.middle_width;
					if (!draw_list.Add_Image(partial,
							seam_free_rectangle(cursor, right_start),
							button.image_color,
							Graphics::Renderer2DBlendMode::Alpha,
							button.grayscale))
						return false;
				}
			}

			if (!draw_list.Add_Image(button.left_image, {left, top, left_end, bottom},
					button.image_color, Graphics::Renderer2DBlendMode::Alpha, button.grayscale)
				|| !draw_list.Add_Image(button.right_image, {right_start, top, right, bottom},
					button.image_color, Graphics::Renderer2DBlendMode::Alpha, button.grayscale))
				return false;
		}
	}
	else if (button.has_image) {
		if (!draw_list.Add_Image(button.image, button.rectangle, button.image_color,
				Graphics::Renderer2DBlendMode::Alpha, button.grayscale))
			return false;
	}
	else {
		if (button.has_border && !draw_list.Add_Outline(
				button.rectangle, button.border_width, button.border_color))
			return false;
		if (button.has_fill) {
			const Graphics::Rect2D inside{
				button.rectangle.left + button.border_width,
				button.rectangle.top + button.border_width,
				button.rectangle.right - button.border_width,
				button.rectangle.bottom - button.border_width};
			if (!draw_list.Add_Rect(inside, button.fill_color))
				return false;
		}
	}
	return true;
}

export bool Add_Push_Button_Overlays(
	DrawList &draw_list,
	const PushButtonVisual &button) noexcept
{
	const Graphics::Rect2D overlay_rectangle =
		button.overlay_rectangle.right != button.overlay_rectangle.left
			|| button.overlay_rectangle.bottom != button.overlay_rectangle.top
		? button.overlay_rectangle : button.rectangle;
	const Graphics::Rect2D clock_rectangle =
		button.clock_rectangle.right != button.clock_rectangle.left
			|| button.clock_rectangle.bottom != button.clock_rectangle.top
		? button.clock_rectangle : button.rectangle;
	const Graphics::Rect2D flashing_rectangle =
		button.flashing_rectangle.right != button.flashing_rectangle.left
			|| button.flashing_rectangle.bottom != button.flashing_rectangle.top
		? button.flashing_rectangle : button.rectangle;
	const Graphics::Rect2D state_rectangle =
		button.state_rectangle.right != button.state_rectangle.left
			|| button.state_rectangle.bottom != button.state_rectangle.top
		? button.state_rectangle : button.rectangle;

	if (button.has_overlay && !draw_list.Add_Image(button.overlay_image, overlay_rectangle))
		return false;
	if (button.has_clock && !draw_list.Add_Clock(
			clock_rectangle, button.clock_percent, button.clock_color, button.remaining_clock))
		return false;
	if (button.has_extra_border && !draw_list.Add_Outline(
			button.extra_border, button.extra_border_width, button.extra_border_color))
		return false;
	if (button.flashing && !draw_list.Add_Image(button.flashing_image, flashing_rectangle))
		return false;
	if (button.use_overlay_states && button.enabled) {
		if (button.selected && button.has_pushed_overlay
			&& !draw_list.Add_Image(button.pushed_overlay, state_rectangle))
			return false;
		if (button.highlighted && !button.selected && button.has_highlighted_overlay
			&& !draw_list.Add_Image(button.highlighted_overlay, state_rectangle))
			return false;
	}
	return true;
}

export bool Draw_Image(
	Graphics::Renderer2D &renderer,
	ImageRef image,
	Graphics::Rect2D screen,
	Graphics::Color2D color = {},
	Graphics::Renderer2DBlendMode blend = Graphics::Renderer2DBlendMode::Alpha,
	bool grayscale = false);

export class TextRenderer final
{
public:
	TextRenderer()
		: m_layout(8192, 1024)
	{
		m_glyphs.reserve(8192);
	}

	bool Measure(
		const FontFace &font,
		const std::uint16_t *text,
		TextLayoutOptions options,
		std::uint32_t &width,
		std::uint32_t &height)
	{
		if (!Build_Layout(font, text, options))
			return false;
		width = static_cast<std::uint32_t>(m_layout.Width());
		height = m_layout.Height();
		return true;
	}

	bool Draw(
		Graphics::Renderer2D &renderer,
		const FontFace &font,
		const FontFace *hotkey_font,
		const std::uint16_t *text,
		float x,
		float y,
		TextLayoutOptions options,
		TextStyle style)
	{
		if (!Build_Layout(font, text, options))
			return false;
        if (!Draw_Glyphs(renderer,font,x+style.x_drop,y+style.y_drop,style.drop_color,false)
            || !Draw_Glyphs(renderer,font,x,y,style.color,false)) return false;
        if (!options.parse_hotkey) return true;
        const FontFace& highlight=hotkey_font ? *hotkey_font : font;
        return Draw_Glyphs(renderer,highlight,x+style.x_drop,y+style.y_drop,style.drop_color,true)
            && Draw_Glyphs(renderer,highlight,x,y,style.hotkey_color,true);
	}

private:
	static int Spacing(const void *context, std::uint16_t character) noexcept
	{
		return static_cast<const FontFace *>(context)->Get_Char_Spacing(character);
	}
	static int Height(const void *context) noexcept
	{
		return static_cast<const FontFace *>(context)->Height();
	}
	static int Overlap(const void *context) noexcept
	{
		return static_cast<const FontFace *>(context)->Extra_Overlap();
	}

	bool Build_Layout(const FontFace &font, const std::uint16_t *text, TextLayoutOptions options)
	{
		return m_layout.Build({&font, &Spacing, &Height, &Overlap}, text, options);
	}

    bool Draw_Glyphs(Graphics::Renderer2D& renderer,const FontFace& font,
        float x,float y,Graphics::Color2D color,bool hotkey_only)
    {
        m_glyphs.clear();
        std::uint32_t page=0;
        const auto flush=[&]() {
            if (m_glyphs.empty()) return true;
            const auto texture=font.Ensure_Texture(renderer,page);
            const bool drawn=texture.index.Is_Valid() && renderer.Add_Text_Glyphs(m_glyphs,texture);
            m_glyphs.clear();
            return drawn;
        };
        for (const auto& placement : m_layout.Placements()) {
            if (placement.hotkey!=hotkey_only) continue;
            const auto* glyph=font.Get_Glyph(placement.character);
            // Spaces and other advance-only glyphs affect layout without ink.
            if (!glyph) continue;
            if (glyph->page!=page && !flush()) return false;
            page=glyph->page;
            m_glyphs.push_back({{x+placement.x,y+placement.y,x+placement.x+glyph->width,
                y+placement.y+font.Height()},glyph->uv,color});
        }
        return flush();
    }

	TextLayout m_layout;
	std::vector<Graphics::Renderer2DGlyph> m_glyphs;
};

export TextRenderer &Get_Text_Renderer() noexcept;

export bool Add_Static_Text(
	DrawList &draw_list,
	StaticTextVisual visual,
	const StaticTextContent &content)
{
	if (content.font == nullptr || content.text == nullptr)
		return false;

	std::uint32_t text_width = 0;
	std::uint32_t text_height = 0;
	if (!Get_Text_Renderer().Measure(
			*content.font, content.text, content.options, text_width, text_height))
		return false;
	visual.text_width = text_width;
	visual.text_height = text_height;
	if (!Add_Static_Text_Background(draw_list, visual))
		return false;

	const Graphics::Point2D position = Get_Static_Text_Position(visual);
	return draw_list.Add_Text(
		content.font,
		content.hotkey_font,
		content.text,
		position.x,
		position.y,
		content.options,
		content.style,
		content.clip,
		content.clip_rectangle);
}

export class RenderList final
{
public:
	explicit RenderList(std::size_t capacity = 576)
	{
		m_nodes.reserve(capacity);
	}

	void Clear() noexcept
	{
		m_nodes.clear();
		m_root_head = Invalid_Node;
		m_root_tail = Invalid_Node;
	}

	bool Add_Node(const RenderNode &node, NodeIndex &index) noexcept
	{
		if (m_nodes.size() >= Invalid_Node)
			return false;
		index = static_cast<NodeIndex>(m_nodes.size());
		m_nodes.push_back(node);
		return true;
	}

	RenderNode *Get_Node(NodeIndex index) noexcept
	{
		return index < m_nodes.size() ? &m_nodes[index] : nullptr;
	}

	const RenderNode *Get_Node(NodeIndex index) const noexcept
	{
		return index < m_nodes.size() ? &m_nodes[index] : nullptr;
	}

	void Set_Roots(NodeIndex head, NodeIndex tail) noexcept
	{
		m_root_head = head;
		m_root_tail = tail;
	}

	NodeIndex Root_Head() const noexcept { return m_root_head; }
	NodeIndex Root_Tail() const noexcept { return m_root_tail; }
	std::size_t Size() const noexcept { return m_nodes.size(); }
	std::size_t Capacity() const noexcept { return m_nodes.capacity(); }
	std::span<const RenderNode> Nodes() const noexcept { return m_nodes; }

private:
	std::vector<RenderNode> m_nodes;
	NodeIndex m_root_head = Invalid_Node;
	NodeIndex m_root_tail = Invalid_Node;
};

namespace
{

Graphics::Rect2D To_Graphics_Rect(Rect rectangle) noexcept
{
	return {
		static_cast<float>(rectangle.left),
		static_cast<float>(rectangle.top),
		static_cast<float>(rectangle.right),
		static_cast<float>(rectangle.bottom)};
}

class RendererClipScope final
{
public:
	explicit RendererClipScope(Graphics::Renderer2D &renderer) noexcept
		: m_renderer(renderer),
		  m_previous(renderer.Get_Clip())
	{
		m_renderer.Set_Clip(false, {});
	}

	~RendererClipScope()
	{
		m_renderer.Set_Clip(m_previous.enabled, m_previous.rectangle);
	}
	RendererClipScope(const RendererClipScope &) = delete;
	RendererClipScope &operator=(const RendererClipScope &) = delete;

private:
	Graphics::Renderer2D &m_renderer;
	Graphics::Renderer2D::ClipState m_previous;
};

bool Has_Sibling_Cycle(void *first, const WindowTreeSource &source) noexcept
{
	if (source.next == nullptr) return false;
	void *slow = first;
	void *fast = first;
	while (fast != nullptr) {
		fast = source.next(source.context, fast);
		if (fast == nullptr) return false;
		fast = source.next(source.context, fast);
		slow = source.next(source.context, slow);
		if (fast == slow) return true;
	}
	return false;
}

bool Append_Window(
	RenderList &list,
	void *window,
	const WindowTreeSource &source,
	NodeIndex &index,
	std::uint32_t depth) noexcept
{
	if (window == nullptr || source.describe == nullptr || depth > 512)
		return false;

	RenderNode node;
	if (!source.describe(source.context, window, node))
		return false;
	node.window = window;
	if (!list.Add_Node(node, index))
		return false;

	RenderNode *stored_node = list.Get_Node(index);
	if (stored_node == nullptr)
		return false;

	NodeIndex previous_child = Invalid_Node;
	void *first_child = source.child != nullptr ? source.child(source.context, window) : nullptr;
	if (Has_Sibling_Cycle(first_child, source)) return false;
	for (void *child = first_child;
		child != nullptr;
		child = source.next != nullptr ? source.next(source.context, child) : nullptr) {
		NodeIndex child_index = Invalid_Node;
		if (!Append_Window(list, child, source, child_index, depth + 1))
			return false;

		// Recursive insertion can grow the node vector. Reacquire the parent.
		stored_node = list.Get_Node(index);
		RenderNode *stored_child = list.Get_Node(child_index);
		if (stored_child == nullptr)
			return false;
		if (stored_node->first_child == Invalid_Node)
			stored_node->first_child = child_index;
		if (previous_child != Invalid_Node) {
			RenderNode *previous = list.Get_Node(previous_child);
			if (previous == nullptr)
				return false;
			previous->next_sibling = child_index;
		}
		stored_child->previous_sibling = previous_child;
		previous_child = child_index;
	}
	stored_node->last_child = previous_child;
	return true;
}

}

export bool Build_Render_List(
	RenderList &list,
	void *window_head,
	WindowTreeSource source) noexcept
{
	if (source.next == nullptr || source.child == nullptr || source.describe == nullptr)
		return false;

	list.Clear();
	NodeIndex root_head = Invalid_Node;
	NodeIndex previous_root = Invalid_Node;
	if (Has_Sibling_Cycle(window_head, source)) return false;
	for (void *window = window_head; window != nullptr;
		window = source.next(source.context, window)) {
		NodeIndex root_index = Invalid_Node;
		if (!Append_Window(list, window, source, root_index, 0))
			return false;

		RenderNode *root = list.Get_Node(root_index);
		if (root == nullptr)
			return false;
		if (root_head == Invalid_Node)
			root_head = root_index;
		if (previous_root != Invalid_Node) {
			RenderNode *previous = list.Get_Node(previous_root);
			if (previous == nullptr)
				return false;
			previous->next_sibling = root_index;
		}
		root->previous_sibling = previous_root;
		previous_root = root_index;
	}
	list.Set_Roots(root_head, previous_root);
	return true;
}

export class Renderer final
{
public:
	Renderer()
		: m_draw_data(4096)
	{
	}

	bool Render(
		const RenderList &list,
		Graphics::Renderer2D &renderer) noexcept
	{
		RendererClipScope clip_scope(renderer);
		for (const Layer layer : {Layer::Below, Layer::Normal, Layer::Above}) {
			NodeIndex node_index = list.Root_Tail();
			std::size_t visited = 0;
			while (node_index != Invalid_Node) {
				if (++visited > list.Size())
					return false;
				const RenderNode *node = list.Get_Node(node_index);
				if (node == nullptr)
					return false;
				const NodeIndex previous = node->previous_sibling;
				if (node->layer == layer && !Render_Node(
					list, node_index, renderer, false, {}, 0, m_draw_data))
					return false;
				node_index = previous;
			}
		}
		return true;
	}

	private:
	static bool Emit_Draw_Data(
		const DrawList &draw_list,
		Graphics::Renderer2D &renderer) noexcept
	{
		for (const DrawCommand &command : draw_list.Commands()) {
			bool emitted = false;
			switch (command.kind) {
				case DrawCommandKind::Rectangle:
					emitted = renderer.Add_Rect(command.rectangle, command.color, command.blend);
					break;
				case DrawCommandKind::Outline:
					emitted = renderer.Add_Outline(command.rectangle, command.width, command.color, command.blend);
					break;
				case DrawCommandKind::Line:
					emitted = renderer.Add_Line(
						{command.rectangle.left, command.rectangle.top}, command.line_end,
						command.width,
						std::array{command.color, command.color, command.secondary_color, command.secondary_color},
						command.blend);
					break;
				case DrawCommandKind::Image:
					emitted = Draw_Image(renderer, command.image, command.rectangle,
						command.color, command.blend, command.grayscale);
					break;
				case DrawCommandKind::Clock:
					emitted = renderer.Add_Rect_Clock(command.rectangle, command.percent,
						command.color, command.remaining, command.blend);
					break;
				case DrawCommandKind::Text:
				{
					const ClipScope clip_scope(
						renderer, command.text_clip, command.text_clip_rectangle);
					emitted = Get_Text_Renderer().Draw(
						renderer, *command.font, command.hotkey_font, command.text,
						command.text_x, command.text_y, command.text_options, command.text_style);
					break;
				}
					break;
			}
			if (!emitted)
				return false;
		}
		return true;
	}

	static Rect Intersect(Rect first, Rect second) noexcept
	{
		return {
			std::max(first.left, second.left),
			std::max(first.top, second.top),
			std::min(first.right, second.right),
			std::min(first.bottom, second.bottom)};
	}

	static bool Render_Node(
		const RenderList &list,
		NodeIndex node_index,
		Graphics::Renderer2D &renderer,
		bool has_clip,
		Rect clip,
		std::uint32_t depth,
		DrawList &draw_data) noexcept
	{
		if (depth > 512)
			return false;

		const RenderNode *node = list.Get_Node(node_index);
		if (node == nullptr)
			return false;

		const bool hidden = Has_Flag(node->flags, WindowFlag::Hidden);
		const bool see_through = Has_Flag(node->flags, WindowFlag::SeeThrough);
		if (hidden)
			return true;

		if (!see_through) {
			if (node->extract != nullptr) {
				draw_data.Clear();
				if (!node->extract(node->extract_context, node->window, node->instance_data, draw_data)
					|| !Emit_Draw_Data(draw_data, renderer))
					return false;
			}
		}

		const bool has_border = Has_Flag(node->flags, WindowFlag::Border);
		const bool border_before_children = Has_Flag(node->flags, WindowFlag::BorderBeforeChildren);
		if (!see_through && has_border && border_before_children) {
			if (node->extract_border != nullptr) {
				draw_data.Clear();
				if (!node->extract_border(node->extract_context, node->window, node->instance_data, draw_data)
					|| !Emit_Draw_Data(draw_data, renderer))
					return false;
			}
		}

		bool child_has_clip = has_clip;
		Rect child_clip = clip;
		const bool clip_children = Has_Flag(node->flags, WindowFlag::ClipChildren);
		Graphics::Renderer2D::ClipState previous_renderer_clip{};
		if (clip_children) {
			child_clip = has_clip ? Intersect(clip, node->screen_region) : node->screen_region;
			child_has_clip = true;
			previous_renderer_clip = renderer.Get_Clip();
			renderer.Set_Clip(true, To_Graphics_Rect(child_clip));
		}

		NodeIndex child_index = node->last_child;
		while (child_index != Invalid_Node) {
			const RenderNode *child = list.Get_Node(child_index);
			if (child == nullptr)
				return false;
			const NodeIndex previous = child->previous_sibling;
			if (!Render_Node(
				list, child_index, renderer, child_has_clip, child_clip, depth + 1,
				draw_data))
				return false;
			child_index = previous;
		}

		if (clip_children) {
			renderer.Set_Clip(
				previous_renderer_clip.enabled, previous_renderer_clip.rectangle);
		}

		if (!see_through && has_border && !border_before_children) {
			if (node->extract_border != nullptr) {
				draw_data.Clear();
				if (!node->extract_border(node->extract_context, node->window, node->instance_data, draw_data)
					|| !Emit_Draw_Data(draw_data, renderer))
					return false;
			}
		}

		return true;
	}

	DrawList m_draw_data;
};

namespace
{
TextRenderer g_text_renderer;
}

TextRenderer &Get_Text_Renderer() noexcept
{
	return g_text_renderer;
}

export ImageRef Resolve_Image_Reference(std::string_view name)
{
	ImageRef image;
	Assets::AssetCache *cache = Assets::Try_Get_Asset_Cache();
	if (cache == nullptr || name.empty())
		return image;
	image.texture = cache->Request_Texture(name);
	return image;
}

export Graphics::Renderer2DTexture Resolve_Image_Texture(
	Assets::TextureAssetHandle asset_handle,
	Graphics::Renderer2D &renderer)
{
	if (!asset_handle.Is_Valid())
		return {};
	Assets::AssetCache *cache = Assets::Try_Get_Asset_Cache();
	if (cache == nullptr)
		return {};

	cache->Wait(asset_handle);
	const Assets::TextureAsset *asset = cache->Try_Get_Texture(asset_handle);
	if (asset == nullptr || !asset->Has_Pixels())
		return {};

	const Graphics::TextureHandle owner(
		static_cast<Graphics::TextureHandle::Index>(0x40000000u | asset_handle.Get_Index()),
		static_cast<Graphics::TextureHandle::Generation>(asset_handle.Get_Generation()));
	return renderer.Register_Texture({
		owner,
		asset->Width(),
		asset->Height(),
		asset->Row_Pitch(),
		1,
		asset->Pixels()});
}

export bool Draw_Image(
	Graphics::Renderer2D &renderer,
	ImageRef image,
	Graphics::Rect2D screen,
	Graphics::Color2D color,
	Graphics::Renderer2DBlendMode blend,
	bool grayscale)
{
	if (!image.texture.Is_Valid() && !image.generated.index.Is_Valid())
		return true;
	const Graphics::Renderer2DTexture texture = image.generated.index.Is_Valid()
		? image.generated : Resolve_Image_Texture(image.texture, renderer);
	if (!texture.index.Is_Valid())
		return false;
	return renderer.Add_Quad(screen, image.uv, texture, color, blend, grayscale);
}

export bool Draw_Window_Background(
	Graphics::Renderer2D &renderer,
	Graphics::Rect2D rectangle,
	bool has_image,
	ImageRef image,
	bool has_border,
	Graphics::Color2D border_color,
	bool has_fill,
	Graphics::Color2D fill_color,
	float border_width = 1.0f)
{
	if (has_image && !Draw_Image(renderer, image, rectangle))
		return false;
	if (has_border && !renderer.Add_Outline(rectangle, border_width, border_color))
		return false;
	if (has_fill) {
		const Graphics::Rect2D inside{
			rectangle.left + border_width,
			rectangle.top + border_width,
			rectangle.right - border_width,
			rectangle.bottom - border_width};
		if (!renderer.Add_Rect(inside, fill_color))
			return false;
	}
	return true;
}

export bool Draw_Border(
	DrawList &draw_list,
	const BorderAtlas &atlas,
	std::int32_t original_x,
	std::int32_t original_y,
	std::int32_t width,
	std::int32_t height)
{
	constexpr std::int32_t corner_size = 15;
	constexpr std::int32_t line_size = 20;
	constexpr std::int32_t half_line_size = line_size / 2;
	constexpr std::int32_t short_offset = 5;
	const auto draw_piece = [&](BorderPiece piece, std::int32_t left, std::int32_t top,
		std::int32_t right, std::int32_t bottom) {
		return draw_list.Add_Image(atlas.pieces[static_cast<std::size_t>(piece)],
			{static_cast<float>(left), static_cast<float>(top),
				static_cast<float>(right), static_cast<float>(bottom)});
	};

	const std::int32_t maximum_x = original_x + width;
	const std::int32_t maximum_y = original_y + height;
	const std::int32_t top_y = original_y - corner_size;
	const std::int32_t bottom_y = maximum_y - short_offset;
	const std::int32_t left_x = original_x - corner_size;
	const std::int32_t right_x = maximum_x - short_offset;

	std::int32_t x = original_x + short_offset;
	const std::int32_t horizontal_end = maximum_x - (short_offset + line_size);
	for (; x <= horizontal_end; x += line_size) {
		if (!draw_piece(BorderPiece::HorizontalTop, x, top_y, x + line_size, top_y + line_size)
			|| !draw_piece(BorderPiece::HorizontalBottom, x, bottom_y, x + line_size, bottom_y + line_size))
			return false;
	}

	const std::int32_t horizontal_corner = maximum_x - short_offset;
	if (horizontal_corner - x >= half_line_size) {
		if (!draw_piece(BorderPiece::HorizontalTopShort, x, top_y, x + half_line_size, top_y + line_size)
			|| !draw_piece(BorderPiece::HorizontalBottomShort, x, bottom_y, x + half_line_size, bottom_y + line_size))
			return false;
		x += half_line_size;
	}
	if (x < horizontal_corner) {
		x -= half_line_size - (((horizontal_corner - x) + 1) & ~1);
		if (!draw_piece(BorderPiece::HorizontalTopShort, x, top_y, x + half_line_size, top_y + line_size)
			|| !draw_piece(BorderPiece::HorizontalBottomShort, x, bottom_y, x + half_line_size, bottom_y + line_size))
			return false;
	}

	std::int32_t y = original_y + short_offset;
	const std::int32_t vertical_end = maximum_y - (short_offset + line_size);
	for (; y <= vertical_end; y += line_size) {
		if (!draw_piece(BorderPiece::VerticalLeft, left_x, y, left_x + line_size, y + line_size)
			|| !draw_piece(BorderPiece::VerticalRight, right_x, y, right_x + line_size, y + line_size))
			return false;
	}

	const std::int32_t vertical_corner = maximum_y - short_offset;
	if (vertical_corner - y >= half_line_size) {
		if (!draw_piece(BorderPiece::VerticalLeftShort, left_x, y, left_x + line_size, y + half_line_size)
			|| !draw_piece(BorderPiece::VerticalRightShort, right_x, y, right_x + line_size, y + half_line_size))
			return false;
		y += half_line_size;
	}
	if (y < vertical_corner) {
		y -= half_line_size - (((vertical_corner - y) + 1) & ~1);
		if (!draw_piece(BorderPiece::VerticalLeftShort, left_x, y, left_x + line_size, y + half_line_size)
			|| !draw_piece(BorderPiece::VerticalRightShort, right_x, y, right_x + line_size, y + half_line_size))
			return false;
	}

	return draw_piece(BorderPiece::CornerUpperLeft, original_x - corner_size, original_y - corner_size,
		original_x - corner_size + line_size, original_y - corner_size + line_size)
		&& draw_piece(BorderPiece::CornerUpperRight, maximum_x - short_offset, original_y - corner_size,
			maximum_x - short_offset + line_size, original_y - corner_size + line_size)
		&& draw_piece(BorderPiece::CornerLowerLeft, original_x - corner_size, maximum_y - short_offset,
			original_x - corner_size + line_size, maximum_y - short_offset + line_size)
		&& draw_piece(BorderPiece::CornerLowerRight, maximum_x - short_offset, maximum_y - short_offset,
			maximum_x - short_offset + line_size, maximum_y - short_offset + line_size);
}

export bool Draw_Border(
	Graphics::Renderer2D &renderer,
	const BorderAtlas &atlas,
	std::int32_t original_x,
	std::int32_t original_y,
	std::int32_t width,
	std::int32_t height)
{
	constexpr std::int32_t corner_size = 15;
	constexpr std::int32_t line_size = 20;
	constexpr std::int32_t half_line_size = line_size / 2;
	constexpr std::int32_t short_offset = 5;

	const auto Draw_Piece = [&](BorderPiece piece, std::int32_t left, std::int32_t top,
		std::int32_t right, std::int32_t bottom) {
		return Draw_Image(renderer, atlas.pieces[static_cast<std::size_t>(piece)],
			{static_cast<float>(left), static_cast<float>(top), static_cast<float>(right), static_cast<float>(bottom)});
	};

	const std::int32_t maximum_x = original_x + width;
	const std::int32_t maximum_y = original_y + height;
	const std::int32_t top_y = original_y - corner_size;
	const std::int32_t bottom_y = maximum_y - short_offset;
	const std::int32_t left_x = original_x - corner_size;
	const std::int32_t right_x = maximum_x - short_offset;

	std::int32_t x = original_x + short_offset;
	const std::int32_t horizontal_end = maximum_x - (short_offset + line_size);
	for (; x <= horizontal_end; x += line_size) {
		if (!Draw_Piece(BorderPiece::HorizontalTop, x, top_y, x + line_size, top_y + line_size)
			|| !Draw_Piece(BorderPiece::HorizontalBottom, x, bottom_y, x + line_size, bottom_y + line_size))
			return false;
	}

	const std::int32_t horizontal_corner = maximum_x - short_offset;
	if (horizontal_corner - x >= half_line_size) {
		if (!Draw_Piece(BorderPiece::HorizontalTopShort, x, top_y, x + half_line_size, top_y + line_size)
			|| !Draw_Piece(BorderPiece::HorizontalBottomShort, x, bottom_y, x + half_line_size, bottom_y + line_size))
			return false;
		x += half_line_size;
	}
	if (x < horizontal_corner) {
		x -= half_line_size - (((horizontal_corner - x) + 1) & ~1);
		if (!Draw_Piece(BorderPiece::HorizontalTopShort, x, top_y, x + half_line_size, top_y + line_size)
			|| !Draw_Piece(BorderPiece::HorizontalBottomShort, x, bottom_y, x + half_line_size, bottom_y + line_size))
			return false;
	}

	std::int32_t y = original_y + short_offset;
	const std::int32_t vertical_end = maximum_y - (short_offset + line_size);
	for (; y <= vertical_end; y += line_size) {
		if (!Draw_Piece(BorderPiece::VerticalLeft, left_x, y, left_x + line_size, y + line_size)
			|| !Draw_Piece(BorderPiece::VerticalRight, right_x, y, right_x + line_size, y + line_size))
			return false;
	}

	const std::int32_t vertical_corner = maximum_y - short_offset;
	if (vertical_corner - y >= half_line_size) {
		if (!Draw_Piece(BorderPiece::VerticalLeftShort, left_x, y, left_x + line_size, y + half_line_size)
			|| !Draw_Piece(BorderPiece::VerticalRightShort, right_x, y, right_x + line_size, y + half_line_size))
			return false;
		y += half_line_size;
	}
	if (y < vertical_corner) {
		y -= half_line_size - (((vertical_corner - y) + 1) & ~1);
		if (!Draw_Piece(BorderPiece::VerticalLeftShort, left_x, y, left_x + line_size, y + half_line_size)
			|| !Draw_Piece(BorderPiece::VerticalRightShort, right_x, y, right_x + line_size, y + half_line_size))
			return false;
	}

	return Draw_Piece(BorderPiece::CornerUpperLeft, original_x - corner_size, original_y - corner_size,
		original_x - corner_size + line_size, original_y - corner_size + line_size)
		&& Draw_Piece(BorderPiece::CornerUpperRight, maximum_x - short_offset, original_y - corner_size,
			maximum_x - short_offset + line_size, original_y - corner_size + line_size)
		&& Draw_Piece(BorderPiece::CornerLowerLeft, original_x - corner_size, maximum_y - short_offset,
			original_x - corner_size + line_size, maximum_y - short_offset + line_size)
		&& Draw_Piece(BorderPiece::CornerLowerRight, maximum_x - short_offset, maximum_y - short_offset,
			maximum_x - short_offset + line_size, maximum_y - short_offset + line_size);
}

}

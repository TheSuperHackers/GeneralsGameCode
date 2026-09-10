module;

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

export module Engine.UI.WND.Layout;

namespace Engine::UI::WND
{

export struct LayoutBounds final
{
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
};

export enum class LayoutAnchor { Automatic, Start, Center, End };

export struct ViewportTransform final
{
	double scale = 1;
	double x = 0;
	double y = 0;
};

export struct SliderThumbLayout final
{
	LayoutBounds bounds{};
	float pixels_per_step = 0;
};

export struct ComboBoxChildLayout final
{
	LayoutBounds button{};
	LayoutBounds entry{};
};

export ComboBoxChildLayout Layout_Combo_Box_Children(int width, int height, float scale) noexcept
{
	const int button_width = std::clamp(int(std::lround(21.0f * scale)), 1, (std::max)(1, width));
	return {{width - button_width, 0, button_width, height},
		{0, 0, (std::max)(0, width - button_width), height}};
}

export SliderThumbLayout Layout_Slider_Thumb(int width, int height, float scale,
	int authored_thumb_width, int authored_y, int minimum, int maximum, int position) noexcept
{
	const int thumb_width = (std::max)(1, int(std::lround(authored_thumb_width * scale)));
	const int travel = (std::max)(0, width - thumb_width);
	const float step = maximum > minimum ? float(travel) / (maximum - minimum) : 0;
	const float x = std::clamp((position - minimum) * step, 0.0f, float(travel));
	return {{int(x), int(std::lround(authored_y * scale)), thumb_width, height}, step};
}

export ViewportTransform Fit_Viewport(int authored_width, int authored_height,
	int width, int height, LayoutAnchor horizontal = LayoutAnchor::Center,
	LayoutAnchor vertical = LayoutAnchor::Center) noexcept
{
	if (authored_width <= 0 || authored_height <= 0 || width <= 0 || height <= 0)
		return {};
	const double scale = (std::min)(double(width) / authored_width, double(height) / authored_height);
	const auto offset = [](double remaining, LayoutAnchor anchor) {
		return anchor == LayoutAnchor::Start ? 0.0 : anchor == LayoutAnchor::End ? remaining : remaining * 0.5;
	};
	return {scale, offset(width - authored_width * scale, horizontal),
		offset(height - authored_height * scale, vertical)};
}

export struct LayoutSource final
{
	void *context = nullptr;
	void *(*next)(void *, void *) noexcept = nullptr;
	void *(*child)(void *, void *) noexcept = nullptr;
	LayoutBounds (*read)(void *, void *) noexcept = nullptr;
	bool (*preserve_aspect)(void *, void *) noexcept = nullptr;
	void (*apply)(void *, void *, LayoutBounds) noexcept = nullptr;
	bool (*managed_by_parent)(void *, void *) noexcept = nullptr;
};

// Owns fractional layout coordinates separately from the integer hit rectangles.
// Decoration stretches with the viewport; sibling controls retain their authored
// proportions and spacing around the group's existing center.
export class Layout final
{
public:
	bool Register(void *window, int authored_width, int authored_height, std::string_view anchors = {})
	{
		LayoutAnchor horizontal = LayoutAnchor::Automatic, vertical = LayoutAnchor::Automatic;
		if (!anchors.empty())
		{
			std::istringstream fields{std::string(anchors)};
			std::string h, v, extra;
			if (!(fields >> h >> v) || (fields >> extra))
				return false;
			const auto parse = [](const std::string &value, const char *start, const char *end, LayoutAnchor &out) {
				if (value == start) out = LayoutAnchor::Start;
				else if (value == "CENTER") out = LayoutAnchor::Center;
				else if (value == end) out = LayoutAnchor::End;
				else return false;
				return true;
			};
			if (!parse(h, "LEFT", "RIGHT", horizontal) || !parse(v, "TOP", "BOTTOM", vertical))
				return false;
		}
		auto &entry = m_entries[window];
		entry.authored_width = authored_width;
		entry.authored_height = authored_height;
		entry.horizontal = horizontal;
		entry.vertical = vertical;
		return true;
	}

	void Forget(void *window) { m_entries.erase(window); }

	double Scale(void *window, int width, int height) const
	{
		const auto found = m_entries.find(window);
		if (found == m_entries.end() || !found->second.captured)
			return Fit_Viewport(800, 600, width, height).scale;
		const Entry &entry = found->second;
		return (std::min)(double(width) / entry.authored_width * entry.factor_x,
			double(height) / entry.authored_height * entry.factor_y);
	}

	bool Default_Position(void *window, int width, int height, int &x, int &y) const
	{
		const auto found = m_entries.find(window);
		if (found == m_entries.end() || !found->second.captured)
			return false;
		const Entry &entry = found->second;
		const auto transform = Fit_Viewport(entry.authored_width, entry.authored_height,
			width, height, Resolve_Anchor(entry.horizontal, entry.initial_x, entry.initial_width),
				Resolve_Anchor(entry.vertical, entry.initial_y, entry.initial_height));
		x = int(std::lround(entry.initial_x * entry.authored_width * transform.scale + transform.x));
		y = int(std::lround(entry.initial_y * entry.authored_height * transform.scale + transform.y));
		return true;
	}

	void Apply(void *root, bool siblings, int old_width, int old_height,
		int width, int height, const LayoutSource &source)
	{
		if (!root || old_width <= 0 || old_height <= 0 || width <= 0 || height <= 0
			|| !source.next || !source.child || !source.read || !source.apply)
			return;
		std::vector<Node> nodes;
		Capture(root, siblings, -1, old_width, old_height, source, nodes);
		// Capture every rectangle before delivering any size messages, since a
		// gadget's size callback can reposition its children.
		Arrange(-1, 1.0, 1.0, width, height, nodes);
		for (const Node &node : nodes)
		{
			auto &entry = m_entries.at(node.window);
			entry.last = node.bounds;
			entry.factor_x = node.factor_x;
			entry.factor_y = node.factor_y;
			source.apply(source.context, node.window, node.bounds);
		}
	}

private:
	struct Entry
	{
		double x = 0, y = 0, width = 0, height = 0;
		double initial_x = 0, initial_y = 0, initial_width = 0, initial_height = 0;
		double factor_x = 1, factor_y = 1;
		int authored_width = 0, authored_height = 0;
		LayoutBounds last{};
		LayoutAnchor horizontal = LayoutAnchor::Automatic, vertical = LayoutAnchor::Automatic;
		bool captured = false;
		bool fullscreen = false;
	};

	static LayoutAnchor Resolve_Anchor(LayoutAnchor requested, double position, double size)
	{
		return requested == LayoutAnchor::Automatic ? Anchor(position, size) : requested;
	}

	static LayoutAnchor Anchor(double position, double size)
	{
		if (position <= 0.005 && position + size < 0.995)
			return LayoutAnchor::Start;
		if (position > 0.005 && position + size >= 0.995)
			return LayoutAnchor::End;
		return LayoutAnchor::Center;
	}
	struct Node
	{
		void *window;
		int parent;
		bool preserve;
		LayoutBounds bounds{};
		double factor_x = 1, factor_y = 1;
	};

	void Capture(void *window, bool siblings, int parent, int width, int height,
		const LayoutSource &source, std::vector<Node> &nodes)
	{
		for (; window; window = siblings ? source.next(source.context, window) : nullptr)
		{
			Entry &entry = m_entries[window];
			if (entry.authored_width <= 0 && source.managed_by_parent
				&& source.managed_by_parent(source.context, window))
				continue;
			const LayoutBounds current = source.read(source.context, window);
			if (!entry.captured)
			{
				entry.x = double(current.x) / width;
				entry.y = double(current.y) / height;
				entry.width = double(current.width) / width;
				entry.height = double(current.height) / height;
				entry.initial_x = entry.x;
				entry.initial_y = entry.y;
				entry.initial_width = entry.width;
				entry.initial_height = entry.height;
				entry.fullscreen = parent == -1 && entry.width >= 0.995 && entry.height >= 0.995;
				entry.captured = true;
				if (entry.authored_width <= 0 || entry.authored_height <= 0)
				{
					// Generated slider and combo-box children share the authored
					// coordinate system of their owning gadget.
					entry.authored_width = parent < 0 ? width : m_entries.at(nodes[parent].window).authored_width;
					entry.authored_height = parent < 0 ? height : m_entries.at(nodes[parent].window).authored_height;
				}
			}
			else
			{
				// Incorporate deliberate game-side moves without accumulating the
				// rounding introduced by previous resize operations.
				entry.x += double(current.x - entry.last.x) / (width * entry.factor_x);
				entry.y += double(current.y - entry.last.y) / (height * entry.factor_y);
				entry.width += double(current.width - entry.last.width) / (width * entry.factor_x);
				entry.height += double(current.height - entry.last.height) / (height * entry.factor_y);
			}
			const int index = int(nodes.size());
			nodes.push_back({window, parent,
				entry.horizontal != LayoutAnchor::Automatic ||
				(!entry.fullscreen && source.preserve_aspect && source.preserve_aspect(source.context, window))});
			Capture(source.child(source.context, window), true, index, width, height, source, nodes);
		}
	}

	void Arrange(int parent, double inherited_x, double inherited_y,
		int width, int height, std::vector<Node> &nodes)
	{
		double left = 0, top = 0, right = 0, bottom = 0;
		bool first = true;
		for (const Node &node : nodes)
		{
			if (node.parent != parent || !node.preserve)
				continue;
			const Entry &entry = m_entries.at(node.window);
			const double x = entry.x * width * inherited_x;
			const double y = entry.y * height * inherited_y;
			const double r = x + entry.width * width * inherited_x;
			const double b = y + entry.height * height * inherited_y;
			left = first ? x : (std::min)(left, x);
			top = first ? y : (std::min)(top, y);
			right = first ? r : (std::max)(right, r);
			bottom = first ? b : (std::max)(bottom, b);
			first = false;
		}
		for (int index = 0; index < int(nodes.size()); ++index)
		{
			Node &node = nodes[index];
			if (node.parent != parent)
				continue;
			const Entry &entry = m_entries.at(node.window);
			double correction_x = 1, correction_y = 1;
			if (node.preserve)
			{
				const double sx = double(width) / entry.authored_width * inherited_x;
				const double sy = double(height) / entry.authored_height * inherited_y;
				const double uniform = (std::min)(sx, sy);
				correction_x = uniform / sx;
				correction_y = uniform / sy;
			}
			node.factor_x = inherited_x * correction_x;
			node.factor_y = inherited_y * correction_y;
			double offset_x = (left + right) * 0.5 * (1 - correction_x);
			double offset_y = (top + bottom) * 0.5 * (1 - correction_y);
			if (parent == -1 && node.preserve)
			{
				const auto transform = Fit_Viewport(entry.authored_width, entry.authored_height,
					width, height, Resolve_Anchor(entry.horizontal, entry.initial_x, entry.initial_width),
				Resolve_Anchor(entry.vertical, entry.initial_y, entry.initial_height));
				offset_x = transform.x;
				offset_y = transform.y;
			}
			node.bounds = {
				int(std::lround(entry.x * width * node.factor_x + offset_x)),
				int(std::lround(entry.y * height * node.factor_y + offset_y)),
				(std::max)(1, int(std::lround(entry.width * width * node.factor_x))),
				(std::max)(1, int(std::lround(entry.height * height * node.factor_y)))};
			Arrange(index, node.factor_x, node.factor_y, width, height, nodes);
		}
	}

	std::unordered_map<void *, Entry> m_entries;
};

}

module;

#include <cstdint>

export module Video.Presentation;

export import Video.Frame;

namespace Engine::Video
{

export using PresentationId = std::uint64_t;
export constexpr PresentationId Invalid_Presentation = 0;

export struct PresentationRect final
{
	std::int32_t x = 0;
	std::int32_t y = 0;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

export struct PresentationExtent final
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

export enum class PresentationLayout : std::uint8_t
{
	Fixed,
	Fit_Output
};

using PresentationRectProvider = bool (*)(void *context, PresentationRect &rect) noexcept;

export struct PresentationTarget final
{
	PresentationLayout layout = PresentationLayout::Fixed;
	PresentationRect rect{};
	PresentationRectProvider rect_provider = nullptr;
	void *context = nullptr;
	bool visible = true;
};

export class FramePresenter
{
public:
	virtual ~FramePresenter() noexcept = default;
	virtual bool Submit(PresentationId source_id, const DecodedVideoFrame &frame, PresentationRect destination) = 0;
};

}

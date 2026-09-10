module;

#define BOOST_TEST_MODULE VideoPresentationTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>

export module Video.Presentation.Tests;

import Video.Presentation;

using namespace Engine::Video;

class TestPresenter final : public FramePresenter
{
public:
	bool Submit(PresentationId source_id, const DecodedVideoFrame &frame, PresentationRect destination) override
	{
		last_source_id = source_id;
		last_frame = &frame;
		last_destination = destination;
		return true;
	}

	PresentationId last_source_id = Invalid_Presentation;
	const DecodedVideoFrame *last_frame = nullptr;
	PresentationRect last_destination{};
};

BOOST_AUTO_TEST_CASE(presentation_target_is_generic_and_provider_driven)
{
	PresentationTarget target;
	target.layout = PresentationLayout::Fixed;
	target.rect = {4, 8, 320, 180};
	BOOST_CHECK(target.visible);
	BOOST_CHECK(target.rect_provider == nullptr);
	BOOST_CHECK(target.rect.width == 320);
	BOOST_CHECK(target.rect.height == 180);
}

BOOST_AUTO_TEST_CASE(frame_presenter_receives_decoded_frame_and_destination)
{
	TestPresenter presenter;
	const std::array<std::byte, 3> pixels = {std::byte{0xff}, std::byte{0x80}, std::byte{0x20}};
	const DecodedVideoFrame frame{1, 1, 3, PixelFormat::RGB8, 7, 1000, pixels};

	BOOST_REQUIRE(presenter.Submit(42, frame, {10, 20, 30, 40}));
	BOOST_CHECK(presenter.last_source_id == 42);
	BOOST_CHECK(presenter.last_frame == &frame);
	BOOST_CHECK(presenter.last_destination.x == 10);
	BOOST_CHECK(presenter.last_destination.y == 20);
	BOOST_CHECK(presenter.last_destination.width == 30);
	BOOST_CHECK(presenter.last_destination.height == 40);
}

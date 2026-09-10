module;

#define BOOST_TEST_MODULE VideoRuntimeTests

#include <boost/test/included/unit_test.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>

export module Video.Runtime.Tests;

import Video.Runtime;

using namespace Engine::Video;

namespace
{
class EmptySource final : public Source
{
public:
	~EmptySource() noexcept override { ++destruction_count; }

	std::size_t Read(std::span<std::byte>) noexcept override { return 0; }
	bool Seek(std::int64_t, SourceSeekOrigin) noexcept override { return false; }
	std::int64_t Tell() const noexcept override { return 0; }
	std::int64_t Size() const noexcept override { return 0; }

	static std::size_t destruction_count;
};

std::size_t EmptySource::destruction_count = 0;
}

BOOST_AUTO_TEST_CASE(runtime_rejects_invalid_sources_and_releases_failed_sessions)
{
	Runtime runtime;
	BOOST_CHECK(runtime.State(PlaybackSlot::Fullscreen) == PlaybackState::Closed);
	BOOST_CHECK(!runtime.Configure(PlaybackSlot::Fullscreen, {}));

	EmptySource::destruction_count = 0;
	std::unique_ptr<Source> source(new EmptySource());
	BOOST_CHECK(!runtime.Open(PlaybackSlot::Fullscreen, std::move(source)));
	BOOST_CHECK(runtime.State(PlaybackSlot::Fullscreen) == PlaybackState::Closed);
	BOOST_CHECK_EQUAL(EmptySource::destruction_count, 1);

	BOOST_CHECK(!runtime.Open(PlaybackSlot::Fullscreen, "missing-video-file.bik"));
	BOOST_CHECK(runtime.State(PlaybackSlot::Fullscreen) == PlaybackState::Closed);

	runtime.Close_All();
	BOOST_CHECK(runtime.State(PlaybackSlot::Fullscreen) == PlaybackState::Closed);
	BOOST_CHECK(runtime.State(PlaybackSlot::Cameo) == PlaybackState::Closed);
}

BOOST_AUTO_TEST_CASE(runtime_closes_sessions_before_source_owners_are_destroyed)
{
	EmptySource::destruction_count = 0;
	{
		Runtime runtime;
		std::unique_ptr<Source> source(new EmptySource());
		BOOST_CHECK(!runtime.Open(PlaybackSlot::Cameo, std::move(source)));
	}
	BOOST_CHECK_EQUAL(EmptySource::destruction_count, 1);
}

BOOST_AUTO_TEST_CASE(runtime_rejects_invalid_session_operations)
{
	Runtime runtime;
	BOOST_CHECK(!runtime.Set_Visible(Invalid_Presentation, false));
	BOOST_CHECK(!runtime.Pause(Invalid_Presentation));
	BOOST_CHECK(!runtime.Play(Invalid_Presentation));
	BOOST_CHECK(!runtime.Seek(Invalid_Presentation, 0));
	BOOST_CHECK(runtime.Current_Frame(Invalid_Presentation) == nullptr);
	BOOST_CHECK(runtime.Info(Invalid_Presentation).width == 0);
}

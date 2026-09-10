module;

#define BOOST_TEST_MODULE VideoPlaybackTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

export module Video.Playback.Tests;

import Video.Playback;

using namespace Engine::Video;

class TestDecoder final : public Decoder
{
public:
	bool Open(std::string_view source) override
	{
		opened = source == "movie.vp6";
		cursor = 0;
		return opened;
	}

	bool Open(Source &) override
	{
		return false;
	}

	void Close() noexcept override
	{
		++close_count;
		opened = false;
	}

	StreamInfo Info() const noexcept override
	{
		return {1, 1, static_cast<std::uint64_t>(frames.size()), 0.1};
	}

	DecodeResult Decode_Next(DecodedVideoFrame &frame) override
	{
		if (!opened)
			return DecodeResult::Error;
		if (cursor >= frames.size())
			return DecodeResult::EndOfStream;

		frame = {
			1,
			1,
			4,
			PixelFormat::RGBA8,
			cursor,
			cursor * 100000,
			std::span<const std::byte>(frames[cursor])
		};
		++cursor;
		++decode_count;
		return DecodeResult::Frame;
	}

	bool Seek(std::uint64_t frame_index) override
	{
		if (!opened || frame_index >= frames.size())
			return false;
		cursor = static_cast<std::size_t>(frame_index);
		return true;
	}

	std::array<std::array<std::byte, 4>, 3> frames = {{
		{{std::byte{0x10}, std::byte{0x20}, std::byte{0x30}, std::byte{0xff}}},
		{{std::byte{0x40}, std::byte{0x50}, std::byte{0x60}, std::byte{0xff}}},
		{{std::byte{0x70}, std::byte{0x80}, std::byte{0x90}, std::byte{0xff}}}
	}};
	std::size_t cursor = 0;
	std::size_t decode_count = 0;
	std::size_t close_count = 0;
	bool opened = false;
};

class TestAudioSink final : public AudioSink
{
public:
	bool Submit(const DecodedAudioChunk &chunk) override
	{
		last_chunk = chunk;
		return chunk.Is_Valid();
	}

	void Start() noexcept override { ++start_count; }
	void Pause() noexcept override { ++pause_count; }
	void Resume() noexcept override { ++resume_count; }
	void Stop() noexcept override { ++stop_count; }
	void Reset() noexcept override { ++reset_count; }

	DecodedAudioChunk last_chunk{};
	std::size_t start_count = 0;
	std::size_t pause_count = 0;
	std::size_t resume_count = 0;
	std::size_t stop_count = 0;
	std::size_t reset_count = 0;
};

BOOST_AUTO_TEST_CASE(playback_owns_state_and_decoder_lifecycle)
{
	TestDecoder decoder;
	{
		Playback playback(decoder);
		BOOST_CHECK(playback.State() == PlaybackState::Closed);
		BOOST_CHECK(playback.Open("movie.vp6"));
		BOOST_CHECK(playback.State() == PlaybackState::Playing);
		BOOST_CHECK(decoder.opened);
		BOOST_CHECK(playback.Current_Frame() == nullptr);

		BOOST_CHECK(playback.Update(0.0));
		BOOST_REQUIRE(playback.Current_Frame() != nullptr);
		BOOST_CHECK(playback.Current_Frame()->frame_index == 0);
		BOOST_CHECK(playback.Is_Frame_Ready());
		BOOST_CHECK(playback.Frame_Sequence() == 1);
		playback.Clear_Frame_Ready();
		BOOST_CHECK(!playback.Is_Frame_Ready());

		BOOST_CHECK(playback.Pause());
		BOOST_CHECK(playback.State() == PlaybackState::Paused);
		BOOST_CHECK(!playback.Update(1.0));
		BOOST_CHECK(playback.Current_Frame()->frame_index == 0);
		BOOST_CHECK(playback.Play());
		BOOST_CHECK(playback.State() == PlaybackState::Playing);

		BOOST_CHECK(playback.Update(0.1));
		BOOST_REQUIRE(playback.Current_Frame() != nullptr);
		BOOST_CHECK(playback.Current_Frame()->frame_index == 1);
		BOOST_CHECK(playback.Current_Frame()->presentation_time_us == 100000);
		BOOST_CHECK(playback.Frame_Sequence() == 2);

		BOOST_CHECK(playback.Update(0.1));
		BOOST_REQUIRE(playback.Current_Frame() != nullptr);
		BOOST_CHECK(playback.Current_Frame()->frame_index == 2);
		BOOST_CHECK(playback.State() == PlaybackState::Playing);

		BOOST_CHECK(!playback.Update(0.1));
		BOOST_CHECK(playback.State() == PlaybackState::Finished);
		BOOST_CHECK(playback.Current_Frame()->frame_index == 2);
		BOOST_CHECK(decoder.decode_count == 3);
	}
	BOOST_CHECK(decoder.close_count == 1);
}

BOOST_AUTO_TEST_CASE(playback_delivers_frame_views_without_copying)
{
	TestDecoder decoder;
	Playback playback(decoder);
	BOOST_REQUIRE(playback.Open("movie.vp6"));
	BOOST_REQUIRE(playback.Update(0.0));
	const DecodedVideoFrame *frame = playback.Current_Frame();
	BOOST_REQUIRE(frame != nullptr);
	BOOST_CHECK(frame->pixels.data() == decoder.frames[0].data());
	BOOST_CHECK(std::to_integer<std::uint8_t>(frame->pixels[0]) == 0x10);

	BOOST_CHECK(playback.Seek(1));
	BOOST_CHECK(playback.Current_Frame() == nullptr);
	BOOST_REQUIRE(playback.Update(0.0));
	BOOST_REQUIRE(playback.Current_Frame() != nullptr);
	BOOST_CHECK(playback.Current_Frame()->frame_index == 1);
}

BOOST_AUTO_TEST_CASE(playback_loop_mode_restarts_inside_engine_video)
{
	TestDecoder decoder;
	Playback playback(decoder);
	playback.Set_Mode(PlaybackMode::Loop);
	BOOST_REQUIRE(playback.Open("movie.vp6"));

	BOOST_REQUIRE(playback.Update(0.0));
	BOOST_CHECK(playback.Current_Frame()->frame_index == 0);
	BOOST_REQUIRE(playback.Update(0.1));
	BOOST_CHECK(playback.Current_Frame()->frame_index == 1);
	BOOST_REQUIRE(playback.Update(0.1));
	BOOST_CHECK(playback.Current_Frame()->frame_index == 2);
	BOOST_REQUIRE(playback.Update(0.1));
	BOOST_CHECK(playback.Current_Frame()->frame_index == 0);
	BOOST_CHECK(playback.State() == PlaybackState::Playing);
}

BOOST_AUTO_TEST_CASE(playback_hold_last_frame_keeps_the_completion_frame)
{
	TestDecoder decoder;
	Playback playback(decoder);
	playback.Set_Mode(PlaybackMode::Hold_Last_Frame);
	BOOST_REQUIRE(playback.Open("movie.vp6"));

	BOOST_REQUIRE(playback.Update(0.0));
	BOOST_REQUIRE(playback.Update(0.1));
	BOOST_REQUIRE(playback.Update(0.1));
	BOOST_CHECK(!playback.Update(0.1));
	BOOST_CHECK(playback.State() == PlaybackState::Finished);
	BOOST_REQUIRE(playback.Current_Frame() != nullptr);
	BOOST_CHECK(playback.Current_Frame()->frame_index == 2);
}

BOOST_AUTO_TEST_CASE(playback_controls_audio_sink_with_video_lifecycle)
{
	TestDecoder decoder;
	TestAudioSink audio;
	Playback playback(decoder);
	playback.Set_Audio_Sink(&audio);

	BOOST_REQUIRE(playback.Open("movie.vp6"));
	BOOST_CHECK_EQUAL(audio.reset_count, 1);
	BOOST_CHECK_EQUAL(audio.start_count, 1);

	BOOST_REQUIRE(playback.Pause());
	BOOST_CHECK_EQUAL(audio.pause_count, 1);
	BOOST_REQUIRE(playback.Play());
	BOOST_CHECK_EQUAL(audio.resume_count, 1);
	BOOST_REQUIRE(playback.Seek(1));
	BOOST_CHECK_EQUAL(audio.reset_count, 2);
	BOOST_CHECK_EQUAL(audio.start_count, 2);

	playback.Close();
	BOOST_CHECK_EQUAL(audio.stop_count, 1);
}

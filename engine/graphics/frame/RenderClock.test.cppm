module;

#define BOOST_TEST_MODULE GraphicsRenderClockTests

#include <boost/test/included/unit_test.hpp>

#include <cstdint>

export module Graphics.Frame.RenderClock.Tests;

import Graphics.Frame.RenderClock;

using Graphics::RenderClock;

BOOST_AUTO_TEST_CASE(clock_starts_at_thirty_logic_frames_per_second)
{
	RenderClock clock;

	BOOST_CHECK_CLOSE_FRACTION(clock.Logic_Frame_Time_Milliseconds(),
		1000.0f / 30.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(clock.Logic_Frame_Time_Seconds(),
		(1000.0f / 30.0f) * 0.001f, 0.000001f);
	BOOST_CHECK_EQUAL(clock.Sync_Time(), 0u);
	BOOST_CHECK_EQUAL(clock.Sync_Delta(), 0u);
	BOOST_CHECK_EQUAL(clock.Fractional_Sync_Milliseconds(), 0u);
	BOOST_CHECK_EQUAL(clock.Logic_Time_Milliseconds(), 0u);
}

BOOST_AUTO_TEST_CASE(clock_carries_fractional_time_until_a_logic_step)
{
	RenderClock clock;
	const float step = 1000.0f / 30.0f;

	clock.Update_Logic_Frame_Time(step);
	clock.Sync(false);
	BOOST_CHECK_EQUAL(clock.Sync_Time(), 0u);
	BOOST_CHECK_EQUAL(clock.Sync_Delta(), 0u);
	BOOST_CHECK_EQUAL(clock.Fractional_Sync_Milliseconds(), 33u);
	BOOST_CHECK_EQUAL(clock.Logic_Time_Milliseconds(), 33u);

	clock.Sync(true);
	BOOST_CHECK_EQUAL(clock.Sync_Time(), 33u);
	BOOST_CHECK_EQUAL(clock.Sync_Delta(), 33u);
	BOOST_CHECK_EQUAL(clock.Fractional_Sync_Milliseconds(), 0u);
	BOOST_CHECK_EQUAL(clock.Logic_Time_Milliseconds(), 33u);

	clock.Update_Logic_Frame_Time(16.5f);
	clock.Sync(false);
	BOOST_CHECK_EQUAL(clock.Sync_Delta(), 0u);
	BOOST_CHECK_EQUAL(clock.Sync_Time(), 33u);
	BOOST_CHECK_EQUAL(clock.Fractional_Sync_Milliseconds(), 16u);
	BOOST_CHECK_EQUAL(clock.Logic_Time_Milliseconds(), 49u);

	clock.Sync(true);
	BOOST_CHECK_EQUAL(clock.Sync_Time(), 49u);
	BOOST_CHECK_EQUAL(clock.Sync_Delta(), 16u);
	BOOST_CHECK_EQUAL(clock.Fractional_Sync_Milliseconds(), 0u);
}

BOOST_AUTO_TEST_CASE(clock_accepts_multiple_updates_before_sync)
{
	RenderClock clock;

	clock.Update_Logic_Frame_Time(10.25f);
	clock.Update_Logic_Frame_Time(10.25f);
	clock.Update_Logic_Frame_Time(10.25f);
	BOOST_CHECK_EQUAL(clock.Sync_Time(), 0u);
	BOOST_CHECK_EQUAL(clock.Fractional_Sync_Milliseconds(), 30u);
	BOOST_CHECK_EQUAL(clock.Logic_Time_Milliseconds(), 30u);

	clock.Sync(true);
	BOOST_CHECK_EQUAL(clock.Sync_Time(), 30u);
	BOOST_CHECK_EQUAL(clock.Sync_Delta(), 30u);
	BOOST_CHECK_EQUAL(clock.Fractional_Sync_Milliseconds(), 0u);
}


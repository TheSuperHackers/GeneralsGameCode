module;
#define BOOST_TEST_MODULE FrameSubmissionStatisticsTests
#include <boost/test/included/unit_test.hpp>
#include <cstdint>
export module Graphics.Frame.SubmissionStatistics.Tests;
import Graphics.Frame.SubmissionStatistics;
import Graphics.RHI;

using namespace Graphics;

BOOST_AUTO_TEST_CASE(completed_frame_includes_offscreen_and_main_submissions_once)
{
    FrameSubmissionStatistics statistics;
    int source;
    BOOST_REQUIRE(statistics.Begin(&source, {10,20,60}));
    BOOST_CHECK_EQUAL(statistics.Last_Frame().draw_calls,0);
    // Three offscreen draws followed by five main-view draws share one source.
    const RHISubmissionCounts offscreen{13,26,78};
    const RHISubmissionCounts all{offscreen.draw_calls+5,offscreen.triangles+10,offscreen.vertex_invocations+30};
    BOOST_REQUIRE(statistics.Complete(&source,all));
    BOOST_CHECK_EQUAL(statistics.Last_Frame().draw_calls,8);
    BOOST_CHECK_EQUAL(statistics.Last_Frame().triangles,16);
    BOOST_CHECK_EQUAL(statistics.Last_Frame().vertex_invocations,48);
    BOOST_CHECK(!statistics.Complete(&source,all));
    BOOST_REQUIRE(statistics.Begin(&source,all));
    BOOST_REQUIRE(statistics.Complete(&source,all));
    BOOST_CHECK_EQUAL(statistics.Last_Frame().draw_calls,0);
    BOOST_CHECK_EQUAL(statistics.Last_Frame().triangles,0);
    BOOST_CHECK_EQUAL(statistics.Last_Frame().vertex_invocations,0);
}

BOOST_AUTO_TEST_CASE(cancelled_or_foreign_frames_preserve_the_completed_snapshot)
{
    FrameSubmissionStatistics statistics;
    int first,second;
    BOOST_CHECK(!statistics.Begin(nullptr,{}));
    BOOST_REQUIRE(statistics.Begin(&first,{}));
    BOOST_REQUIRE(statistics.Complete(&first,{1,2,6}));
    BOOST_REQUIRE(statistics.Begin(&first,{1,2,6}));
    BOOST_CHECK(!statistics.Begin(&second,{}));
    BOOST_CHECK(!statistics.Complete(&second,{100,200,600}));
    BOOST_CHECK_EQUAL(statistics.Last_Frame().draw_calls,1);
    // A failed draw/presentation iteration is discarded, not published.
    statistics.Cancel();
    BOOST_CHECK(!statistics.Complete(&first,{8,16,48}));
    BOOST_CHECK_EQUAL(statistics.Last_Frame().draw_calls,1);
    BOOST_REQUIRE(statistics.Begin(&second,{}));
    BOOST_REQUIRE(statistics.Complete(&second,{2,4,12}));
    BOOST_CHECK_EQUAL(statistics.Last_Frame().draw_calls,2);
    statistics.Reset();
    BOOST_CHECK_EQUAL(statistics.Last_Frame().draw_calls,0);
    BOOST_CHECK_EQUAL(statistics.Last_Frame().triangles,0);
    BOOST_CHECK_EQUAL(statistics.Last_Frame().vertex_invocations,0);
}

BOOST_AUTO_TEST_CASE(counter_recreation_is_rejected_and_wide_totals_keep_precision)
{
    FrameSubmissionStatistics statistics;
    int source;
    constexpr std::uint64_t wide=std::uint64_t{1}<<40;
    BOOST_REQUIRE(statistics.Begin(&source,{wide,wide,wide}));
    BOOST_CHECK(!statistics.Complete(&source,{0,0,0}));
    BOOST_CHECK_EQUAL(statistics.Last_Frame().draw_calls,0);
    BOOST_REQUIRE(statistics.Complete(&source,{wide+7,wide+11,wide+33}));
    BOOST_CHECK_EQUAL(statistics.Last_Frame().draw_calls,7);
    BOOST_CHECK_EQUAL(statistics.Last_Frame().triangles,11);
    BOOST_CHECK_EQUAL(statistics.Last_Frame().vertex_invocations,33);
    BOOST_REQUIRE(statistics.Begin(&source,{10,20,60}));
    BOOST_CHECK(!statistics.Complete(&source,{11,19,66}));
    BOOST_CHECK(!statistics.Complete(&source,{11,22,59}));
    BOOST_REQUIRE(statistics.Complete(&source,{11,22,66}));
    BOOST_CHECK_EQUAL(statistics.Last_Frame().draw_calls,1);
}

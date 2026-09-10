module;
#define BOOST_TEST_MODULE DazzleLayerTests
#include <boost/test/included/unit_test.hpp>
#include <memory>
#include <vector>
export module Graphics.Scene.Dazzles.Layer.Tests;
import Graphics.Scene.Dazzles.Layer;
namespace {
struct Source {
    unsigned value;
    Graphics::DazzleMembership membership;
    std::vector<unsigned>& released;
    Source(unsigned id,std::vector<unsigned>& log):value(id),released(log) {}
    ~Source() { BOOST_CHECK(!membership.queued); released.push_back(value); }
};
}
BOOST_AUTO_TEST_CASE(layer_keeps_type_order_reverse_insertion_latest_state_and_source_lifetimes) {
    std::vector<unsigned> released,drawn;
    Graphics::DazzleLayer<std::shared_ptr<Source>> layer(2),other(2);
    auto first=std::make_shared<Source>(1,released),second=std::make_shared<Source>(2,released),third=std::make_shared<Source>(3,released);
    unsigned retained=0;
    layer.Queue(1,first->membership,[&] { ++retained; return first; });
    layer.Queue(1,second->membership,[&] { ++retained; return second; });
    layer.Queue(0,third->membership,[&] { ++retained; return third; });
    layer.Queue(1,first->membership,[&] { ++retained; return first; });
    other.Queue(1,first->membership,[&] { ++retained; return first; });
    BOOST_CHECK_EQUAL(retained,3);
    second->value=7;
    first.reset(); second.reset(); third.reset(); BOOST_CHECK(released.empty());
    other.Draw_All([&](const auto&) { BOOST_FAIL("a source cannot belong to two queued layers"); });
    layer.Draw_All([&](const auto& source) { BOOST_CHECK(source->membership.queued); drawn.push_back(source->value); });
    BOOST_CHECK((drawn==std::vector<unsigned>{3,7,1})); BOOST_CHECK((released==std::vector<unsigned>{3,7,1}));
    layer.Draw_All([&](const auto&) { BOOST_FAIL("flush must empty the layer"); });
}
BOOST_AUTO_TEST_CASE(cancellation_clears_membership_before_final_release_and_allows_requeue) {
    std::vector<unsigned> released;
    auto source=std::make_shared<Source>(4,released);
    {
        Graphics::DazzleLayer<std::shared_ptr<Source>> layer(1);
        layer.Queue(0,source->membership,[&] { return source; });
        layer.Clear(); BOOST_CHECK(!source->membership.queued); BOOST_CHECK(released.empty());
        layer.Queue(0,source->membership,[&] { return source; });
        source.reset();
    }
    BOOST_CHECK((released==std::vector<unsigned>{4}));
}

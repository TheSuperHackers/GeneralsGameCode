module;
#define BOOST_TEST_MODULE VertexChannelsTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>
#include <Utility/CppMacros.h>
#include "WWMath/vector2.h"
export module Graphics.Scene.Models.VertexChannels.Tests;
import Graphics.Scene.Models.VertexChannels;

namespace {
std::array<std::uint32_t, 2> Object_Bits(const Vector2& value)
{
    static_assert(sizeof(Vector2) == sizeof(std::array<std::uint32_t, 2>));
    std::array<std::uint32_t, 2> bits{};
    std::memcpy(bits.data(), &value, sizeof(bits));
    return bits;
}
}

BOOST_AUTO_TEST_CASE(vector2_installed_channels_compare_all_bytes_and_signed_zero)
{
    using Channels = Graphics::VertexChannels<Vector2>;
    const Vector2 positive_zero(0.0f, 0.5f);
    const Vector2 negative_zero(-0.0f, 0.5f);
    const auto positive_bits = Object_Bits(positive_zero);
    const auto negative_bits = Object_Bits(negative_zero);
    BOOST_CHECK_EQUAL(positive_bits[0], std::bit_cast<std::uint32_t>(0.0f));
    BOOST_CHECK_EQUAL(negative_bits[0], std::bit_cast<std::uint32_t>(-0.0f));
    BOOST_CHECK_NE(positive_bits[0], negative_bits[0]);
    BOOST_CHECK_EQUAL(positive_bits[1], negative_bits[1]);

    Channels channels;
    const std::array<Vector2, 2> positive{{positive_zero, Vector2(1.0f, 0.75f)}};
    const std::array<Vector2, 2> negative{{negative_zero, Vector2(1.0f, 0.75f)}};
    const auto positive_index = channels.Install(positive);
    const auto negative_index = channels.Install(negative);
    BOOST_CHECK_NE(positive_index, negative_index);
    BOOST_CHECK_EQUAL(channels.Install(positive), positive_index);
    BOOST_CHECK_EQUAL(channels.Get(positive_index)[0].X, 0.0f);
    BOOST_CHECK_EQUAL(channels.Get(negative_index)[0].X, -0.0f);
    BOOST_CHECK_EQUAL(Object_Bits(channels.Get(positive_index)[0])[0], positive_bits[0]);
    BOOST_CHECK_EQUAL(Object_Bits(channels.Get(negative_index)[0])[0], negative_bits[0]);
}

BOOST_AUTO_TEST_CASE(default_and_alternate_channels_share_then_detach_and_survive_source_release)
{
    using Channels = Graphics::VertexChannels<Vector2>;
    Channels defaults;
    const std::array<Vector2, 2> default_primary{{Vector2(0.25f, 0.5f), Vector2(0.75f, 0.5f)}};
    const std::array<Vector2, 2> default_secondary{{Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f)}};
    const auto primary = defaults.Install(default_primary);
    const auto secondary = defaults.Install(default_secondary);

    Channels alternate;
    const auto alternate_primary = alternate.Import(defaults, primary);
    const auto alternate_secondary = alternate.Import(defaults, secondary);
    BOOST_CHECK_EQUAL(alternate_primary, 0u);
    BOOST_CHECK_EQUAL(alternate_secondary, 1u);
    BOOST_CHECK(alternate.Get(alternate_primary) == defaults.Get(primary));
    BOOST_CHECK(alternate.Get(alternate_secondary) == defaults.Get(secondary));

    alternate.Make_Unique(alternate_primary);
    alternate.Get(alternate_primary)[0].X = 0.5f;
    BOOST_CHECK_EQUAL(alternate.Get(alternate_primary)[0].X, 0.5f);
    BOOST_CHECK_EQUAL(defaults.Get(primary)[0].X, 0.25f);
    BOOST_CHECK(alternate.Get(alternate_primary) != defaults.Get(primary));
    BOOST_CHECK(alternate.Get(alternate_secondary) == defaults.Get(secondary));

    // An explicit alternate mapping can share a source channel directly.
    alternate.Share(2, defaults, secondary);
    BOOST_CHECK(alternate.Get(2) == defaults.Get(secondary));
    BOOST_CHECK_EQUAL(alternate.Count(), 3u);
    defaults.Clear();
    BOOST_CHECK_EQUAL(alternate.Get(alternate_primary)[0].X, 0.5f);
    BOOST_CHECK_EQUAL(alternate.Get(alternate_secondary)[1].X, 1.0f);
    BOOST_CHECK_EQUAL(alternate.Get(2)[0].Y, 1.0f);
}

BOOST_AUTO_TEST_CASE(import_reuses_equal_content_but_preserves_unpublished_identity)
{
    using Channels = Graphics::VertexChannels<Vector2>;
    const std::array<Vector2, 2> values{{Vector2(0.125f, 0.25f), Vector2(0.875f, 0.75f)}};

    Channels source;
    const auto source_index = source.Install(values);
    Channels equivalent;
    const auto equivalent_index = equivalent.Install(values);
    BOOST_REQUIRE_EQUAL(equivalent.Import(source, source_index), equivalent_index);
    BOOST_CHECK(equivalent.Get(equivalent_index) != source.Get(source_index));
    BOOST_CHECK_EQUAL(Object_Bits(equivalent.Get(equivalent_index)[1])[0],
        Object_Bits(source.Get(source_index)[1])[0]);

    Channels identity;
    identity.Share(0, source, source_index);
    BOOST_CHECK_EQUAL(identity.Import(source, source_index), 0u);
    BOOST_CHECK(identity.Get(0) == source.Get(source_index));

    Channels writable_source;
    auto* source_values = writable_source.Create(0, values.size());
    source_values[0] = values[0];
    source_values[1] = values[1];
    Channels writable_destination;
    auto* destination_values = writable_destination.Create(0, values.size());
    destination_values[0] = values[0];
    destination_values[1] = values[1];
    const auto imported_index = writable_destination.Import(writable_source, 0);
    BOOST_CHECK_EQUAL(imported_index, 1u);
    BOOST_CHECK(writable_destination.Get(0) != writable_source.Get(0));
    BOOST_CHECK(writable_destination.Get(imported_index) == writable_source.Get(0));
}


BOOST_AUTO_TEST_CASE(channel_revisions_follow_shared_origins_and_writable_escapes)
{
    using Channels=Graphics::VertexChannels<Vector2>;
    Channels first,second,combined;
    const std::array<Vector2,1> first_values{{Vector2(0.25f,0.5f)}};
    const std::array<Vector2,1> second_values{{Vector2(0.75f,1)}};
    const auto a=first.Install(first_values);
    const auto b=second.Install(second_values);
    combined.Share(0,first,a);
    const auto imported=combined.Import(second,b);
    const auto a_revision=first.Revision(a);
    const auto b_revision=second.Revision(b);
    BOOST_REQUIRE_NE(a_revision,0u);
    BOOST_REQUIRE_NE(b_revision,0u);
    BOOST_CHECK_NE(a_revision,b_revision);
    BOOST_CHECK_EQUAL(combined.Revision(0),a_revision);
    BOOST_CHECK_EQUAL(combined.Revision(imported),b_revision);
    BOOST_CHECK_EQUAL(combined.Peek(0)[0].X,0.25f);
    BOOST_CHECK_EQUAL(first.Revision(a),a_revision);
    auto* retained=first.Get(a);
    BOOST_CHECK_EQUAL(combined.Revision(0),0u);
    BOOST_CHECK_EQUAL(combined.Revision(imported),b_revision);
    retained[0].X=0.125f;
    BOOST_CHECK_EQUAL(combined.Peek(0)[0].X,0.125f);
    combined.Make_Unique(0);
    combined.Get(0)[0].X=0.625f;
    BOOST_CHECK_EQUAL(first.Peek(a)[0].X,0.125f);
    first.Clear(); second.Clear();
    BOOST_CHECK_EQUAL(combined.Peek(imported)[0].X,0.75f);
    BOOST_CHECK_EQUAL(combined.Revision(imported),b_revision);
    combined.Clear();
    combined.Install(first_values);
    BOOST_CHECK_NE(combined.Revision(0),a_revision);
    BOOST_CHECK_NE(combined.Revision(0),0u);
}

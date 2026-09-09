#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

import engine.navigation.world_fixture;

using Map = std::vector<std::string>;
using navigation::testing::JpsStats;
using navigation::testing::Timing;

namespace {

constexpr int warmSamples = 3;

struct Mode {
    const char* name;
    bool macroEdges;
    bool jps;
};

constexpr std::array<Mode, 3> modes{{
    {"macrooff_jpsoff", false, false},
    {"macroon_jpsoff", true, false},
    {"macroon_jpson", true, true},
}};

struct CounterDelta {
    std::uint64_t attempted = 0;
    std::uint64_t accepted = 0;
    std::uint64_t validationRejected = 0;
    std::uint64_t unavailableFallback = 0;
    std::uint64_t topologyRebuilds = 0;
};

CounterDelta delta(const JpsStats& after, const JpsStats& before)
{
    return {
        after.attempted - before.attempted,
        after.accepted - before.accepted,
        after.validationRejected - before.validationRejected,
        after.unavailableFallback - before.unavailableFallback,
        after.topologyRebuilds - before.topologyRebuilds,
    };
}

template<class Measure>
void run(const char* scenario, const char* api, const Map& map,
         int x1, int y1, int x2, int y2, const Mode& mode,
         int requests, Measure measure)
{
    navigation::testing::World world(map);
    world.setMacroEdgesEnabled(mode.macroEdges);
    world.setJpsAdapterEnabled(mode.jps);

    // Isolate the first topology preparation and allocation from steady-state
    // samples. The same World and query then exercise the reusable adapter and
    // native search storage in one executable.
    const auto beforeCold = world.jpsStats();
    const auto cold = measure(world, x1, y1, x2, y2, 1);
    const auto afterCold = world.jpsStats();
    const auto coldCounters = delta(afterCold, beforeCold);

    std::vector<double> samples;
    samples.reserve(warmSamples);
    Timing lastWarm;
    std::uint64_t warmFoundTotal = 0;
    for (int sample = 0; sample < warmSamples; ++sample) {
        lastWarm = measure(world, x1, y1, x2, y2, requests);
        samples.push_back(double(lastWarm.nanoseconds) / requests / 1000.0);
        warmFoundTotal += static_cast<std::uint64_t>(lastWarm.found);
    }
    std::sort(samples.begin(), samples.end());
    const auto afterWarm = world.jpsStats();
    const auto warmCounters = delta(afterWarm, afterCold);

    std::cout << scenario << ',' << api << ',' << mode.name << ','
              << requests << ',' << warmSamples << ','
              << double(cold.nanoseconds) / 1000.0 << ',' << cold.work << ','
              << cold.found << ',' << coldCounters.attempted << ','
              << coldCounters.accepted << ',' << coldCounters.validationRejected << ','
              << coldCounters.unavailableFallback << ','
              << coldCounters.topologyRebuilds << ',' << samples[samples.size() / 2] << ','
              << double(lastWarm.work) / requests << ',' << warmFoundTotal << ','
              << warmCounters.attempted << ',' << warmCounters.accepted << ','
              << warmCounters.validationRejected << ',' << warmCounters.unavailableFallback << ','
              << warmCounters.topologyRebuilds << '\n';
    std::cout << "# " << scenario << ',' << api << ',' << mode.name
              << " cell_bytes=" << world.cellBytes()
              << " search_storage_bytes=" << world.searchStorageBytes() << '\n';
}

Map openMap(int size)
{
    return Map(static_cast<std::size_t>(size),
        std::string(static_cast<std::size_t>(size), '.'));
}

Map mazeMap(int size, int spacing, int gap)
{
    Map map = openMap(size);
    for (int x = spacing; x < size - spacing; x += spacing) {
        for (int y = 0; y < size; ++y) {
            const bool gapAtBottom = ((x / spacing) % 2) != 0;
            if (gapAtBottom ? y < size - gap : y >= gap)
                map[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = '#';
        }
    }
    return map;
}

Map obstacleMap(int size)
{
    Map map = openMap(size);
    for (int y = 12; y < size - 12; y += 16) {
        for (int x = 12; x < size - 12; x += 16) {
            for (int dy = 0; dy < 8 && y + dy < size; ++dy)
                for (int dx = 0; dx < 8 && x + dx < size; ++dx)
                    map[static_cast<std::size_t>(y + dy)]
                       [static_cast<std::size_t>(x + dx)] = '#';
        }
    }
    return map;
}

} // namespace

int main()
{
    std::cout << std::unitbuf;
    std::cout << "scenario,api,mode,warm_requests,warm_samples,cold_us,cold_work,cold_found,"
                 "cold_attempted,cold_accepted,cold_validation_rejected,cold_unavailable_fallback,"
                 "cold_topology_rebuilds,warm_median_us,warm_work_per_request,warm_found_total,"
                 "warm_attempted,warm_accepted,warm_validation_rejected,warm_unavailable_fallback,"
                 "warm_topology_rebuilds\n";

    const auto runScenario = [](const char* name, const Map& map,
                                int x1, int y1, int x2, int y2,
                                int groundRequests, int normalRequests) {
        for (const auto& mode : modes) {
            run(name, "ground", map, x1, y1, x2, y2, mode, groundRequests,
                [](auto& world, int fromX, int fromY, int toX, int toY, int count) {
                    return world.benchmarkGround(fromX, fromY, toX, toY, count);
                });
            run(name, "normal", map, x1, y1, x2, y2, mode, normalRequests,
                [](auto& world, int fromX, int fromY, int toX, int toY, int count) {
                    return world.benchmarkNormal(fromX, fromY, toX, toY, count);
                });
        }
    };

    const Map open96 = openMap(96);
    const Map maze96 = mazeMap(96, 16, 12);
    const Map obstacles96 = obstacleMap(96);
    runScenario("open96", open96, 2, 2, 90, 87, 100, 50);
    runScenario("maze96", maze96, 2, 2, 90, 87, 50, 20);
    runScenario("obstacle96", obstacles96, 2, 2, 90, 87, 50, 20);

    const Map open512 = openMap(512);
    const Map maze512 = mazeMap(512, 32, 64);
    const Map obstacles512 = obstacleMap(512);
    runScenario("open512", open512, 2, 2, 500, 487, 50, 20);
    // The native 512 maze query is intentionally sampled lightly: each cold
    // baseline normal search is materially slower than the JPS route.
    runScenario("maze512", maze512, 2, 2, 500, 487, 20, 10);
    runScenario("obstacle512", obstacles512, 2, 2, 500, 487, 20, 10);

    // Keep the existing queued-ground characterization in the same process.
    std::cout << "scenario,budget,requests,total_ms,batches,max_batch_ms,found,work,attempted,"
                 "accepted,validation_rejected,unavailable_fallback,topology_rebuilds\n";
    for (const int budget : {5000, 25000, 50000}) {
        for (const bool complex : {false, true}) {
            for (const bool jps : {false, true}) {
                navigation::testing::World world(complex ? maze96 : open96);
                world.setMacroEdgesEnabled(true);
                world.setJpsAdapterEnabled(jps);
                const auto before = world.jpsStats();
                const auto result = world.benchmarkQueuedGround(10000, budget);
                const auto counters = delta(world.jpsStats(), before);
                const char* mapName = complex ? "queued_maze_varied" : "queued_open_varied";
                const char* modeName = jps ? "_jpson" : "_jpsoff";
                std::cout << mapName << modeName << ',' << budget << ",10000,"
                          << result.nanoseconds / 1000000.0 << ',' << result.batches << ','
                          << result.maxBatchNanoseconds / 1000000.0 << ',' << result.found << ','
                          << result.work << ',' << counters.attempted << ','
                          << counters.accepted << ',' << counters.validationRejected << ','
                          << counters.unavailableFallback << ',' << counters.topologyRebuilds
                          << '\n';
            }
        }
    }
}

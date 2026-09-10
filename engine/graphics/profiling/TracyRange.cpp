// Offline Tracy analysis: emit CPU totals for matching shellmap logic frames.
// Build against the same TracyServer library as the capture tool.
#include <cstdint>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <algorithm>
#include <span>
#include <thread>
#include <vector>
#include "server/TracyFileRead.hpp"
#include "server/TracyWorker.hpp"

struct ChildTotals {
    int64_t duration = 0;
    uint64_t count = 0;
};

static ChildTotals Child_Totals(const tracy::Worker& worker, const tracy::ZoneEvent& zone,
    std::span<const int16_t> sources = {})
{
    if (!zone.HasChildren()) return {};
    const auto& children = worker.GetZoneChildren(zone.Child());
    ChildTotals total;
    const auto add = [&](const tracy::ZoneEvent& child) {
        if (!sources.empty() && std::find(sources.begin(),sources.end(),child.SrcLoc()) == sources.end()) return;
        total.duration += child.End() - child.Start();
        ++total.count;
    };
    if (children.is_magic()) {
        const auto& values = reinterpret_cast<const tracy::Vector<tracy::ZoneEvent>&>(children);
        for (const auto& child : values) add(child);
    } else {
        for (const auto& child : children) add(*child);
    }
    return total;
}

int main(int argc, char** argv)
{
    if (argc != 4 && argc != 5) {
        std::fprintf(stderr,"Usage: TracyRange capture.tracy first_logic_frame last_logic_frame [child_zone_name]\n");
        return 1;
    }
    const int first = std::atoi(argv[2]), last = std::atoi(argv[3]);
    auto file = std::unique_ptr<tracy::FileRead>(tracy::FileRead::Open(argv[1]));
    if (!file || first >= last) return 1;
    tracy::Worker worker(*file);
    while (!worker.AreSourceLocationZonesReady())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    int64_t begin = -1, end = -1;
    for (const auto& plot : worker.GetPlots()) {
        if (std::strcmp(worker.GetString(plot->name),"Graphics.LogicFrame") != 0) continue;
        for (const auto& point : plot->data) {
            if (begin < 0 && point.val >= first) begin = point.time.Val();
            if (begin >= 0 && point.val >= last) { end = point.time.Val(); break; }
        }
    }
    if (begin < 0 || end <= begin) return 2;
    std::fprintf(stderr,"Range: %lld..%lld ns\n",static_cast<long long>(begin),static_cast<long long>(end));
    std::vector<int16_t> child_sources;
    if (argc == 5) {
        for (const auto& entry : worker.GetSourceLocationZones()) {
            const auto& source = worker.GetSourceLocation(entry.first);
            const auto* name = worker.GetString(source.name.active ? source.name : source.function);
            if (std::strcmp(name,argv[4]) == 0) child_sources.push_back(entry.first);
        }
        if (child_sources.empty()) return 3;
    }
    std::puts(argc == 5 ? "parent|calls|total_ns" : "name|calls|total_ns|self_ns");
    for (const auto& entry : worker.GetSourceLocationZones()) {
        int64_t total = 0, self = 0;
        uint64_t count = 0;
        for (const auto& item : entry.second.zones) {
            const auto& zone = *item.Zone();
            if (zone.Start() < begin || zone.Start() >= end || zone.End() < zone.Start()) continue;
            if (argc == 5) {
                const auto children = Child_Totals(worker,zone,child_sources);
                total += children.duration;
                count += children.count;
                continue;
            }
            const auto duration = zone.End() - zone.Start();
            total += duration;
            self += duration - Child_Totals(worker,zone).duration;
            ++count;
        }
        if (!count) continue;
        const auto& source = worker.GetSourceLocation(entry.first);
        const auto* name = worker.GetString(source.name.active ? source.name : source.function);
        if (argc == 5)
            std::printf("%s|%llu|%lld\n",name,static_cast<unsigned long long>(count),static_cast<long long>(total));
        else
            std::printf("%s|%llu|%lld|%lld\n",name,static_cast<unsigned long long>(count),
                static_cast<long long>(total),static_cast<long long>(self));
    }
}

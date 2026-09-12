// main cannot belong to a named C++ module. All host/game behavior is modular;
// this translation unit only validates process arguments and reports results.
#include <charconv>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>
import games.generalszh.hosts.headless.headless_match;
import games.generalszh.hosts.headless.content_match;
int main(int argc, char **argv)
{
    std::size_t workers = 1;
    if (argc > 3) { std::cerr << "Usage: generalszh_headless [workers:1..64] [extracted-INI-directory]\n"; return 2; }
    if (argc >= 2)
    {
        const std::string_view argument(argv[1]); const auto parsed = std::from_chars(argument.data(),argument.data()+argument.size(),workers);
        if (parsed.ec != std::errc{} || parsed.ptr != argument.data()+argument.size() || !workers || workers > 64)
        { std::cerr << "Worker count must be an integer in 1..64\n"; return 2; }
    }
    try
    {
        generalszh::headless::MatchReport report;
        if(argc==3)
        {
            const auto content=generalszh::headless::RunContentMatch(std::filesystem::path(argv[2]),workers); report=content.match;
            std::cout << "Loaded unit=" << content.unitName << " omitted_behavior_modules=" << content.omittedBehaviors.size() << '\n';
            for(const auto &name:content.omittedBehaviors) std::cout << "  not implemented: " << name << '\n';
            std::cout << "Partial content adapter: no armor, upgrades, original-map geometry or full locomotion/projectile fidelity\n";
        }
        else report=generalszh::headless::RunMatch(workers);
        const bool won = report.outcome.status == generalszh::MatchStatus::Won;
        std::cout << "Authored modern headless match (not original-map content)\nworkers=" << workers
            << " result=" << (won ? "victory" : "unfinished") << " tick=" << report.outcome.tick
            << " winner=" << report.outcome.winner.index << ':' << report.outcome.winner.generation
            << " target_health=" << report.targetHealth << " surviving_units=" << report.survivingUnits << '\n';
        return won ? 0 : 1;
    }
    catch (const std::exception &error) { std::cerr << "Simulation failed: " << error.what() << '\n'; return 1; }
}

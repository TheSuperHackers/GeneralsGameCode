module;
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
export module engine.gameplay.concealment.algorithms.detection_lease;

export namespace engine::gameplay::concealment
{
inline std::uint64_t ConcealmentDeadline(std::uint64_t tick, std::uint64_t delay)
{
    if (delay > (std::numeric_limits<std::uint64_t>::max)() - tick)
        throw std::overflow_error("Concealment deadline exceeds simulation tick range");
    return tick + delay;
}

// StealthDetectorUpdate passes updateRate + 1 to markAsDetected. The extra
// tick preserves the legacy exclusive expiry boundary: tick < detectedUntil.
inline std::uint64_t DetectionLeaseDeadline(std::uint64_t tick, std::uint64_t detectionRate)
{
    if (detectionRate == (std::numeric_limits<std::uint64_t>::max)())
        throw std::overflow_error("Detection lease exceeds simulation tick range");
    return ConcealmentDeadline(tick, detectionRate + 1);
}

inline bool IsDetected(std::uint64_t tick, std::uint64_t detectedUntil) noexcept
{
    return detectedUntil != 0 && tick < detectedUntil;
}

inline std::uint64_t ExtendDetectionLease(std::uint64_t current, std::uint64_t tick,
    std::uint64_t detectionRate)
{
    return (std::max)(current, DetectionLeaseDeadline(tick, detectionRate));
}
}

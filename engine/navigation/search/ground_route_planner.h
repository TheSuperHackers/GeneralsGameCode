#pragma once
#include "engine/navigation/movement/movement_validator.h"
#include "engine/navigation/path/ground_path_builder.h"
namespace navigation {
// Selects and validates direct/JPS routes. Native fallback remains the caller's policy.
class GroundRoutePlanner {
    struct State;
    std::unique_ptr<State> state_;
    Pathfinder& world_;
    MovementValidator movement_;
    GroundPathBuilder builder_;
    Pathfinder::JpsAdapterStats stats_;
    Bool enabled_ = true, directEnabled_ = true;
    Bool prepareTopology(Int radius, Bool center);
public:
    explicit GroundRoutePlanner(Pathfinder&);
    ~GroundRoutePlanner();
    void reset();
    void setEnabled(Bool value) { enabled_ = value; }
    Bool enabled() const { return enabled_; }
    void setDirectEnabled(Bool value) { directEnabled_ = value; }
    void invalidate();
    void invalidate(const IRegion2D&);
    Pathfinder::JpsAdapterStats stats() const;
    Path* find(const Pathfinder::JpsGroundQuery&, const Coord3D*, const Coord3D*);
};
}

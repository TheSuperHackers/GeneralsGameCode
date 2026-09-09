#pragma once
#include "engine/navigation/legacy/AIPathfind.h"
namespace navigation {
// Restores raw avoidance/rejoin cells without changing smoothed path links.
class GroundPathBuilder {
    Pathfinder& world_;
    static Int visit(Pathfinder*, PathfindCell*, PathfindCell*, Int, Int, void*);
public:
    explicit GroundPathBuilder(Pathfinder& world) : world_(world) {}
    void materialize(Path*, Bool center);
    Path* buildGround(Bool isCrusher, const Coord3D* fromPos, PathfindCell* goalCell,
        Bool center, Int pathDiameter);
    Path* buildHierarchical(const Coord3D* fromPos, PathfindCell* goalCell);
    Path* buildActual(const Object* obj, LocomotorSurfaceTypeMask acceptableSurfaces,
        const Coord3D* fromPos, PathfindCell* goalCell, Bool center, Bool blocked);
    void prepend(Path* path, const Coord3D* fromPos, PathfindCell* goalCell, Bool center);
    void coordinate(Int cellX, Int cellY, Bool centerInCell, Coord3D& pos, PathfindLayerEnum layer);
};
}

#pragma once

#include "WWMath/aabox.h"
#include "WWMath/matrix3d.h"
#include "W3DDevice/GameClient/W3DQueryBounds.h"
#include "W3DDevice/GameClient/W3DSceneQueryMask.h"
#include "WWMath/obbox.h"
#include "WWMath/tri.h"
#include "WWMath/colmath.h"

struct W3DIntersectionQuery
{
    explicit W3DIntersectionQuery(int mask) : CollisionType(mask) {}
    int CollisionType;
};

class W3DBoxIntersectionQuery : public W3DIntersectionQuery
{
public:
    W3DBoxIntersectionQuery(const AABoxClass &box, int mask) : W3DIntersectionQuery(mask), Box(box) {}
    bool Cull(const Vector3 &minimum, const Vector3 &maximum) const
    {
        return Geometry::Bounds_Are_Disjoint(W3DQueryBounds::Read(Box), W3DQueryBounds::Read(minimum, maximum));
    }
    bool Cull(const AABoxClass &box) const { return W3DQueryBounds::Disjoint(Box, box); }
    bool Intersect_Triangle(const TriClass &triangle) const { return CollisionMath::Intersection_Test(Box, triangle); }
    AABoxClass Box;
};

class W3DOrientedBoxIntersectionQuery : public W3DIntersectionQuery
{
public:
    W3DOrientedBoxIntersectionQuery(const OBBoxClass &box, int mask) : W3DIntersectionQuery(mask), Box(box)
    {
        Update_Bounds();
    }
    W3DOrientedBoxIntersectionQuery(const W3DOrientedBoxIntersectionQuery &source)
        : W3DIntersectionQuery(source), Box(source.Box) { Update_Bounds(); }
    W3DOrientedBoxIntersectionQuery(const W3DOrientedBoxIntersectionQuery &source, const Matrix3D &transform)
        : W3DIntersectionQuery(source)
    {
        OBBoxClass::Transform(transform, source.Box, &Box);
        Update_Bounds();
    }
    W3DOrientedBoxIntersectionQuery(const W3DBoxIntersectionQuery &source, const Matrix3D &transform)
        : W3DIntersectionQuery(source)
    {
        Matrix3D::Transform_Vector(transform, source.Box.Center, &Box.Center);
        Box.Extent = source.Box.Extent;
        Box.Basis = transform;
        Update_Bounds();
    }
    bool Cull(const Vector3 &minimum, const Vector3 &maximum) const
    {
        return Geometry::Bounds_Are_Disjoint(W3DQueryBounds::Read(BoundingBox), W3DQueryBounds::Read(minimum, maximum));
    }
    bool Cull(const AABoxClass &box) const { return W3DQueryBounds::Disjoint(BoundingBox, box); }
    bool Intersect_Triangle(const TriClass &triangle) const { return CollisionMath::Intersection_Test(Box, triangle); }
    OBBoxClass Box;
    AABoxClass BoundingBox;

private:
    void Update_Bounds()
    {
        BoundingBox.Center = Box.Center;
        BoundingBox.Extent = W3DQueryBounds::Write(Geometry::Oriented_Box_Extent(
            W3DQueryBounds::Read_Matrix<3>(Box.Basis), W3DQueryBounds::Read(Box.Extent)));
    }
};


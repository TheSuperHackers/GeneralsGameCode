#pragma once

#include <utility>

#include "W3DDevice/GameClient/W3DQueryBounds.h"
#include "W3DDevice/GameClient/W3DSceneQueryMask.h"
#include "WWMath/aabox.h"
#include "WWMath/castres.h"
#include "WWMath/lineseg.h"
#include "WWMath/matrix3d.h"
#include "WWMath/obbox.h"
#include "WWMath/tri.h"
#include "WWMath/colmath.h"

class W3DRenderObject;

// Transformed queries share their caller's result and hit-object identity.
struct W3DCastQuery
{
    W3DCastQuery(CastResultStruct *result, int mask) : Result(result), CollisionType(mask) {}
    W3DCastQuery(const W3DCastQuery &) = default;
    CastResultStruct *Result;
    int CollisionType;
    W3DRenderObject *CollidedRenderObj = nullptr;
};

class W3DRayCastQuery : public W3DCastQuery
{
public:
    W3DRayCastQuery(const LineSegClass &ray, CastResultStruct *result,
        int mask = SCENE_QUERY_0, bool translucent = false, bool hidden = false)
        : W3DCastQuery(result, mask), Ray(ray), CheckTranslucent(translucent), CheckHidden(hidden) {}
    W3DRayCastQuery(const W3DRayCastQuery &source, const Matrix3D &transform)
        : W3DCastQuery(source), Ray(source.Ray, transform),
          CheckTranslucent(source.CheckTranslucent), CheckHidden(source.CheckHidden) {}
    W3DRayCastQuery(const W3DRayCastQuery &) = delete;
    W3DRayCastQuery &operator=(const W3DRayCastQuery &) = delete;
    bool Cull(const Vector3 &minimum, const Vector3 &maximum) const
    {
        return CollisionMath::Overlap_Test(minimum, maximum, Ray) == CollisionMath::POS;
    }
    bool Cull(const AABoxClass &box) const { return CollisionMath::Overlap_Test(box, Ray) == CollisionMath::POS; }
    bool Cast_To_Triangle(const TriClass &triangle) const { return CollisionMath::Collide(Ray, triangle, Result); }
    LineSegClass Ray;
    bool CheckTranslucent;
    bool CheckHidden;
};

class W3DBoxCastQuery : public W3DCastQuery
{
public:
    enum ROTATION_TYPE { ROTATE_NONE, ROTATE_Z90, ROTATE_Z180, ROTATE_Z270 };
    W3DBoxCastQuery(const AABoxClass &box, const Vector3 &movement, CastResultStruct *result,
        int mask = SCENE_QUERY_0) : W3DCastQuery(result, mask), Box(box), Move(movement)
    {
        W3DQueryBounds::Write(Geometry::Swept_Box_Bounds(W3DQueryBounds::Read(Box.Center),
            W3DQueryBounds::Read(Box.Extent), W3DQueryBounds::Read(Move)), SweepMin, SweepMax);
    }
    W3DBoxCastQuery(const W3DBoxCastQuery &) = default;
    W3DBoxCastQuery &operator=(const W3DBoxCastQuery &) = delete;
    bool Cull(const Vector3 &minimum, const Vector3 &maximum) const
    {
        return Geometry::Bounds_Are_Disjoint(W3DQueryBounds::Read(SweepMin, SweepMax),
            W3DQueryBounds::Read(minimum, maximum));
    }
    bool Cull(const AABoxClass &box) const { return Cull(box.Center - box.Extent, box.Center + box.Extent); }
    bool Cast_To_Triangle(const TriClass &triangle) const { return CollisionMath::Collide(Box, Move, triangle, Result); }
    void Translate(const Vector3 &translation)
    {
        Box.Center += translation;
        SweepMin += translation;
        SweepMax += translation;
    }
    void Rotate(ROTATION_TYPE rotation)
    {
        Box.Center = W3DQueryBounds::Write(Geometry::Rotate_Z(W3DQueryBounds::Read(Box.Center), rotation));
        Move = W3DQueryBounds::Write(Geometry::Rotate_Z(W3DQueryBounds::Read(Move), rotation));
        if (rotation == ROTATE_Z90 || rotation == ROTATE_Z270)
            std::swap(Box.Extent.X, Box.Extent.Y);
        W3DQueryBounds::Write(Geometry::Rotate_Bounds_Z(W3DQueryBounds::Read(SweepMin, SweepMax), rotation),
            SweepMin, SweepMax);
    }
    void Transform(const Matrix3D &transform)
    {
        const Vector3 center = Box.Center, extent = Box.Extent;
        transform.Transform_Center_Extent_AABox(center, extent, &Box.Center, &Box.Extent);
        Move = transform.Rotate_Vector(Move);
        W3DQueryBounds::Write(Geometry::Transform_Bounds_Corners(W3DQueryBounds::Read(SweepMin, SweepMax),
            W3DQueryBounds::Read_Matrix<4>(transform)), SweepMin, SweepMax);
    }
    AABoxClass Box;
    Vector3 Move;
    Vector3 SweepMin, SweepMax;
};

class W3DOrientedBoxCastQuery : public W3DCastQuery
{
public:
    W3DOrientedBoxCastQuery(const OBBoxClass &box, const Vector3 &movement, CastResultStruct *result,
        int mask = SCENE_QUERY_0) : W3DCastQuery(result, mask), Box(box), Move(movement)
    {
        const auto extent = Geometry::Oriented_Box_Extent(W3DQueryBounds::Read_Matrix<3>(Box.Basis),
            W3DQueryBounds::Read(Box.Extent), .01f);
        W3DQueryBounds::Write(Geometry::Swept_Box_Bounds(W3DQueryBounds::Read(Box.Center), extent,
            W3DQueryBounds::Read(Move)), SweepMin, SweepMax);
    }
    W3DOrientedBoxCastQuery(const W3DOrientedBoxCastQuery &) = default;
    W3DOrientedBoxCastQuery &operator=(const W3DOrientedBoxCastQuery &) = delete;
    W3DOrientedBoxCastQuery(const W3DOrientedBoxCastQuery &source, const Matrix3D &transform)
        : W3DCastQuery(source)
    {
        transform.Transform_Min_Max_AABox(source.SweepMin, source.SweepMax, &SweepMin, &SweepMax);
        Matrix3D::Rotate_Vector(transform, source.Move, &Move);
        OBBoxClass::Transform(transform, source.Box, &Box);
    }
    W3DOrientedBoxCastQuery(const W3DBoxCastQuery &source, const Matrix3D &transform)
        : W3DCastQuery(source)
    {
        transform.Transform_Min_Max_AABox(source.SweepMin, source.SweepMax, &SweepMin, &SweepMax);
        Matrix3D::Rotate_Vector(transform, source.Move, &Move);
        Matrix3D::Transform_Vector(transform, source.Box.Center, &Box.Center);
        Box.Extent = source.Box.Extent;
        Box.Basis = transform;
    }
    bool Cull(const Vector3 &minimum, const Vector3 &maximum) const
    {
        return Geometry::Bounds_Are_Disjoint(W3DQueryBounds::Read(SweepMin, SweepMax),
            W3DQueryBounds::Read(minimum, maximum));
    }
    bool Cull(const AABoxClass &box) const { return Cull(box.Center - box.Extent, box.Center + box.Extent); }
    bool Cast_To_Triangle(const TriClass &triangle) const
    {
        return CollisionMath::Collide(Box, Move, triangle, Vector3(0, 0, 0), Result);
    }
    OBBoxClass Box;
    Vector3 Move;
    Vector3 SweepMin, SweepMax;
};


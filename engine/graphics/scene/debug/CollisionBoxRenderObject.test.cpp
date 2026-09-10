#define BOOST_TEST_MODULE CollisionBoxRenderObjectQueryTests
#include <boost/test/included/unit_test.hpp>

#include <cmath>

#include "W3DDevice/GameClient/CollisionBoxRenderObject.h"
#include "W3DDevice/GameClient/W3DSegmentedLineRenderObject.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DSceneQueryMask.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include "WWMath/aabox.h"
#include "WWMath/castres.h"
#include "WWMath/lineseg.h"
#include "WWMath/matrix3d.h"
#include "WWMath/obbox.h"
#include "WWLib/ref_ptr.h"

namespace
{

constexpr float kTolerance = 0.0001f;

void Check_Vector(const Vector3 &actual, const Vector3 &expected)
{
	BOOST_CHECK_SMALL(actual.X - expected.X, kTolerance);
	BOOST_CHECK_SMALL(actual.Y - expected.Y, kTolerance);
	BOOST_CHECK_SMALL(actual.Z - expected.Z, kTolerance);
}

void Check_Fraction(const CastResultStruct &result, float expected)
{
	BOOST_CHECK_SMALL(result.Fraction - expected, kTolerance);
}

Matrix3D Rotated_Translated_Transform()
{
	Matrix3D transform = Matrix3D::RotateZ90;
	transform.Set_Translation(Vector3(10, 20, 30));
	return transform;
}

Matrix3x3 Rotated_Basis()
{
	return Matrix3x3(Matrix3D::RotateZ90);
}

void Check_Ray_Hit(CollisionBoxRenderObject &object,
	const LineSegClass &line,
	int collision_type,
	float expected_fraction,
	const Vector3 &expected_normal,
	const Vector3 &expected_contact)
{
	CastResultStruct result;
	result.ComputeContactPoint = true;
	W3DRayCastQuery ray(line, &result, collision_type);
	BOOST_REQUIRE(object.Cast_Ray(ray));
	Check_Fraction(result, expected_fraction);
	Check_Vector(result.Normal, expected_normal);
	Check_Vector(result.ContactPoint, expected_contact);
	BOOST_CHECK(ray.CollidedRenderObj == &object);
}

void Check_Swept_Hit(CollisionBoxRenderObject &object,
	W3DBoxCastQuery &test,
	float expected_fraction,
	const Vector3 &expected_normal)
{
	BOOST_REQUIRE(object.Cast_AABox(test));
	Check_Fraction(*test.Result, expected_fraction);
	Check_Vector(test.Result->Normal, expected_normal);
	BOOST_CHECK(test.CollidedRenderObj == &object);
}

void Check_Swept_Hit(CollisionBoxRenderObject &object,
	W3DOrientedBoxCastQuery &test,
	float expected_fraction,
	const Vector3 &expected_normal)
{
	BOOST_REQUIRE(object.Cast_OBBox(test));
	Check_Fraction(*test.Result, expected_fraction);
	Check_Vector(test.Result->Normal, expected_normal);
	BOOST_CHECK(test.CollidedRenderObj == &object);
}

}

BOOST_AUTO_TEST_CASE(moved_adapter_preserves_aa_and_ob_bounds_after_rotation_and_translation)
{
	const Vector3 local_center(1, 2, 3);
	const Vector3 local_extent(2, 3, 4);
	const Matrix3D transform = Rotated_Translated_Transform();

	RefCountPtr<CollisionBoxRenderObject> aa_object =
		Create_No_Add_Ref(new CollisionBoxRenderObject);
	aa_object->Set_Local_Center_Extent(local_center, local_extent);
	aa_object->Set_Transform(transform);
	const AABoxClass &aa_box = aa_object->Get_AA_Box();
	Check_Vector(aa_box.Center, Vector3(11, 22, 33));
	Check_Vector(aa_box.Extent, local_extent);

	RefCountPtr<CollisionBoxRenderObject> ob_object =
		Create_No_Add_Ref(new CollisionBoxRenderObject(
			OBBoxClass(Vector3(0, 0, 0), local_extent, Rotated_Basis())));
	BOOST_CHECK_EQUAL(ob_object->Class_ID(), W3DRenderObject::CLASSID_OBBOX);
	ob_object->Set_Local_Center_Extent(local_center, local_extent);
	ob_object->Set_Transform(transform);
	const OBBoxClass &ob_box = ob_object->Get_OB_Box();
	Check_Vector(ob_box.Center, Vector3(8, 21, 33));
	Check_Vector(ob_box.Extent, local_extent);
	Check_Vector(Vector3(ob_box.Basis[0][0], ob_box.Basis[1][0], ob_box.Basis[2][0]),
		Vector3(0, 1, 0));
	Check_Vector(Vector3(ob_box.Basis[0][1], ob_box.Basis[1][1], ob_box.Basis[2][1]),
		Vector3(-1, 0, 0));
	Check_Vector(Vector3(ob_box.Basis[0][2], ob_box.Basis[1][2], ob_box.Basis[2][2]),
		Vector3(0, 0, 1));

	// Set_Position keeps the oriented basis while moving the transformed center.
	ob_object->Set_Position(Vector3(-4, 6, 7));
	Check_Vector(ob_object->Get_OB_Box().Center, Vector3(-6, 7, 10));
}

BOOST_AUTO_TEST_CASE(query_masks_animation_hidden_and_preexisting_startbad_are_rejected)
{
	RefCountPtr<CollisionBoxRenderObject> object =
		Create_No_Add_Ref(new CollisionBoxRenderObject(
			AABoxClass(Vector3(0, 0, 0), Vector3(1, 1, 1))));
	object->Set_Collision_Type(SCENE_QUERY_PHYSICAL);
	const LineSegClass line(Vector3(-3, 0, 0), Vector3(3, 0, 0));

	CastResultStruct masked_result;
	masked_result.Fraction = 0.25f;
	masked_result.Normal = Vector3(0, 1, 0);
	W3DRayCastQuery masked(line, &masked_result, SCENE_QUERY_PROJECTILE);
	BOOST_CHECK(!object->Cast_Ray(masked));
	Check_Fraction(masked_result, 0.25f);
	Check_Vector(masked_result.Normal, Vector3(0, 1, 0));
	BOOST_CHECK(masked.CollidedRenderObj == nullptr);

	object->Set_Animation_Hidden(true);
	CastResultStruct hidden_result;
	W3DRayCastQuery hidden(line, &hidden_result, SCENE_QUERY_PHYSICAL);
	BOOST_CHECK(!object->Cast_Ray(hidden));
	BOOST_CHECK(hidden.CollidedRenderObj == nullptr);
	BOOST_CHECK(!hidden_result.StartBad);

	object->Set_Animation_Hidden(false);
	CastResultStruct startbad_result;
	startbad_result.StartBad = true;
	startbad_result.Fraction = 0.5f;
	startbad_result.Normal = Vector3(0, 0, 1);
	W3DRayCastQuery startbad(line, &startbad_result, SCENE_QUERY_PHYSICAL);
	BOOST_CHECK(!object->Cast_Ray(startbad));
	BOOST_CHECK(startbad_result.StartBad);
	Check_Fraction(startbad_result, 0.5f);
	Check_Vector(startbad_result.Normal, Vector3(0, 0, 1));
	BOOST_CHECK(startbad.CollidedRenderObj == nullptr);
}

BOOST_AUTO_TEST_CASE(ray_queries_return_known_fraction_normal_contact_and_object)
{
	const int collision_type = SCENE_QUERY_PROJECTILE;

	RefCountPtr<CollisionBoxRenderObject> aa_object =
		Create_No_Add_Ref(new CollisionBoxRenderObject(
			AABoxClass(Vector3(5, 0, 0), Vector3(1, 2, 3))));
	aa_object->Set_Collision_Type(collision_type);
	Check_Ray_Hit(*aa_object,
		LineSegClass(Vector3(-1, 0, 0), Vector3(9, 0, 0)),
		collision_type, 0.5f, Vector3(-1, 0, 0), Vector3(4, 0, 0));

	const OBBoxClass oriented_box(Vector3(5, 0, 0), Vector3(1, 2, 3), Rotated_Basis());
	RefCountPtr<CollisionBoxRenderObject> ob_object =
		Create_No_Add_Ref(new CollisionBoxRenderObject(oriented_box));
	ob_object->Set_Collision_Type(collision_type);
	Check_Ray_Hit(*ob_object,
		LineSegClass(Vector3(5, -5, 0), Vector3(5, 5, 0)),
		collision_type, 0.4f, Vector3(0, -1, 0), Vector3(5, -1, 0));
}

BOOST_AUTO_TEST_CASE(segmented_line_ray_queries_preserve_mask_miss_transform_and_order)
{
	W3DSegmentedLineRenderObject object;
	object.Set_Collision_Type(SCENE_QUERY_PHYSICAL);
	object.Set_Width(0.5f);
	const Vector3 points[] = {
		Vector3(-2, -1, 0), Vector3(2, -1, 0), Vector3(2, 1, 0), Vector3(-2, 1, 0)};
	object.Set_Points(4, points);

	Matrix3D transform = Matrix3D::RotateZ90;
	transform.Set_Translation(Vector3(10, 20, 30));
	object.Set_Transform(transform);

	CastResultStruct masked_result;
	masked_result.Fraction = 0.25f;
	const LineSegClass world_ray(
		Vector3(5, 20, 30), Vector3(15, 20, 30));
	W3DRayCastQuery masked(world_ray, &masked_result, SCENE_QUERY_PROJECTILE);
	BOOST_CHECK(!object.Cast_Ray(masked));
	BOOST_CHECK_EQUAL(masked_result.Fraction, 0.25f);
	BOOST_CHECK(masked.CollidedRenderObj == nullptr);

	CastResultStruct miss_result;
	W3DRayCastQuery miss(
        LineSegClass(Vector3(5, 20, 31), Vector3(15, 20, 31)),
		&miss_result, SCENE_QUERY_PHYSICAL);
	BOOST_CHECK(!object.Cast_Ray(miss));
	BOOST_CHECK_EQUAL(miss_result.Fraction, 1.0f);
	BOOST_CHECK(miss.CollidedRenderObj == nullptr);

	CastResultStruct hit_result;
	W3DRayCastQuery hit(world_ray, &hit_result, SCENE_QUERY_PHYSICAL);
	BOOST_REQUIRE(object.Cast_Ray(hit));
	// The first transformed segment is at x=11 (fraction .6). The later
	// segment is at x=9 (fraction .4), but queries stop at the first accepted hit.
	BOOST_CHECK_CLOSE(hit_result.Fraction, 0.6f, 0.001f);
	BOOST_CHECK_EQUAL(hit_result.SurfaceType, 13u);
	BOOST_CHECK(hit.CollidedRenderObj == &object);

	// Picking uses the full authored width, independently of the drawn radius.
	CastResultStruct width_result;
	W3DRayCastQuery within_width(
		LineSegClass(Vector3(5, 20, 30.4f), Vector3(15, 20, 30.4f)),
		&width_result, SCENE_QUERY_PHYSICAL);
	BOOST_REQUIRE(object.Cast_Ray(within_width));
	Check_Fraction(width_result, 0.6f);

	CastResultStruct nearer_result;
	nearer_result.Fraction = 0.5f;
	W3DRayCastQuery nearer(world_ray, &nearer_result, SCENE_QUERY_PHYSICAL);
	BOOST_REQUIRE(object.Cast_Ray(nearer));
	Check_Fraction(nearer_result, 0.4f);

	CastResultStruct limited_result;
	limited_result.Fraction = 0.3f;
	W3DRayCastQuery limited(world_ray, &limited_result, SCENE_QUERY_PHYSICAL);
	BOOST_CHECK(!object.Cast_Ray(limited));
	Check_Fraction(limited_result, 0.3f);
	BOOST_CHECK(limited.CollidedRenderObj == nullptr);
}

BOOST_AUTO_TEST_CASE(swept_aa_and_ob_queries_preserve_known_hits_and_normals)
{
	const int collision_type = SCENE_QUERY_PHYSICAL;

	RefCountPtr<CollisionBoxRenderObject> aa_target =
		Create_No_Add_Ref(new CollisionBoxRenderObject(
			AABoxClass(Vector3(5, 0, 0), Vector3(1, 1, 1))));
	aa_target->Set_Collision_Type(collision_type);

	CastResultStruct aa_result;
	W3DBoxCastQuery moving_aa(
		AABoxClass(Vector3(-5, 0, 0), Vector3(1, 1, 1)),
		Vector3(10, 0, 0), &aa_result, collision_type);
	Check_Swept_Hit(*aa_target, moving_aa, 0.8f, Vector3(-1, 0, 0));

	CastResultStruct ob_result;
	const OBBoxClass moving_ob_box(Vector3(-5, 0, 0), Vector3(1, 1, 1));
	W3DOrientedBoxCastQuery moving_ob(moving_ob_box,
		Vector3(10, 0, 0), &ob_result, collision_type);
	Check_Swept_Hit(*aa_target, moving_ob, 0.8f, Vector3(-1, 0, 0));

	const OBBoxClass oriented_target_box(Vector3(5, 0, 0), Vector3(1, 2, 1), Rotated_Basis());
	RefCountPtr<CollisionBoxRenderObject> oriented_target =
		Create_No_Add_Ref(new CollisionBoxRenderObject(oriented_target_box));
	oriented_target->Set_Collision_Type(collision_type);

	CastResultStruct oriented_aa_result;
	W3DBoxCastQuery moving_aa_again(
		AABoxClass(Vector3(5, -5, 0), Vector3(1, 1, 1)),
		Vector3(0, 10, 0), &oriented_aa_result, collision_type);
	Check_Swept_Hit(*oriented_target, moving_aa_again, 0.3f, Vector3(0, -1, 0));

	CastResultStruct oriented_ob_result;
	const OBBoxClass moving_ob_box_again(Vector3(5, -5, 0), Vector3(1, 1, 1));
	W3DOrientedBoxCastQuery moving_ob_again(moving_ob_box_again,
		Vector3(0, 10, 0), &oriented_ob_result, collision_type);
	Check_Swept_Hit(*oriented_target, moving_ob_again, 0.3f, Vector3(0, -1, 0));

	CastResultStruct miss_result;
	W3DBoxCastQuery moving_away(
		AABoxClass(Vector3(-5, 0, 0), Vector3(1, 1, 1)),
		Vector3(-10, 0, 0), &miss_result, collision_type);
	BOOST_CHECK(!aa_target->Cast_AABox(moving_away));
	BOOST_CHECK(moving_away.CollidedRenderObj == nullptr);
	Check_Fraction(miss_result, 1.0f);
}

BOOST_AUTO_TEST_CASE(intersection_queries_cover_aa_ob_hit_miss_and_masks)
{
	const int collision_type = SCENE_QUERY_PHYSICAL;
	RefCountPtr<CollisionBoxRenderObject> aa_target =
		Create_No_Add_Ref(new CollisionBoxRenderObject(
			AABoxClass(Vector3(0, 0, 0), Vector3(1, 1, 1))));
	aa_target->Set_Collision_Type(collision_type);

	W3DBoxIntersectionQuery aa_hit(
		AABoxClass(Vector3(1.5f, 0, 0), Vector3(1, 1, 1)), collision_type);
	W3DBoxIntersectionQuery aa_miss(
		AABoxClass(Vector3(2.1f, 0, 0), Vector3(1, 1, 1)), collision_type);
	BOOST_CHECK(aa_target->Intersect_AABox(aa_hit));
	BOOST_CHECK(!aa_target->Intersect_AABox(aa_miss));

	W3DOrientedBoxIntersectionQuery ob_hit(
		OBBoxClass(Vector3(0, 1.5f, 0), Vector3(1, 1, 1), Rotated_Basis()), collision_type);
	W3DOrientedBoxIntersectionQuery ob_miss(
		OBBoxClass(Vector3(0, 2.1f, 0), Vector3(1, 1, 1), Rotated_Basis()), collision_type);
	BOOST_CHECK(aa_target->Intersect_OBBox(ob_hit));
	BOOST_CHECK(!aa_target->Intersect_OBBox(ob_miss));

	const OBBoxClass oriented_target_box(Vector3(0, 0, 0), Vector3(1, 2, 1), Rotated_Basis());
	RefCountPtr<CollisionBoxRenderObject> oriented_target =
		Create_No_Add_Ref(new CollisionBoxRenderObject(oriented_target_box));
	oriented_target->Set_Collision_Type(collision_type);

	W3DBoxIntersectionQuery oriented_aa_hit(
		AABoxClass(Vector3(0, 1.5f, 0), Vector3(1, 1, 1)), collision_type);
	W3DBoxIntersectionQuery oriented_aa_miss(
		AABoxClass(Vector3(0, 2.1f, 0), Vector3(1, 1, 1)), collision_type);
	BOOST_CHECK(oriented_target->Intersect_AABox(oriented_aa_hit));
	BOOST_CHECK(!oriented_target->Intersect_AABox(oriented_aa_miss));

	W3DOrientedBoxIntersectionQuery oriented_ob_hit(
		OBBoxClass(Vector3(1.5f, 0, 0), Vector3(1, 1, 1)), collision_type);
	W3DOrientedBoxIntersectionQuery oriented_ob_miss(
		OBBoxClass(Vector3(3.1f, 0, 0), Vector3(1, 1, 1)), collision_type);
	BOOST_CHECK(oriented_target->Intersect_OBBox(oriented_ob_hit));
	BOOST_CHECK(!oriented_target->Intersect_OBBox(oriented_ob_miss));

	W3DBoxIntersectionQuery masked(
		AABoxClass(Vector3(0, 0, 0), Vector3(1, 1, 1)), SCENE_QUERY_PROJECTILE);
	BOOST_CHECK(!oriented_target->Intersect_AABox(masked));
}

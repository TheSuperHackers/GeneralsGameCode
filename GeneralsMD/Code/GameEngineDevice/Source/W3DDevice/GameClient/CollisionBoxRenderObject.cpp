#include "WWMath/matrix4.h"
#include "W3DDevice/GameClient/W3DRenderServices.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

import Assets.Adapters.W3D.Box;
import Graphics.Scene.Debug.CollisionBox;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Views.CameraMatrices;
import Graphics.Scene.Views.View;

#include "W3DDevice/GameClient/CollisionBoxRenderObject.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"

#include "WWLib/chunkio.h"
#include "WWMath/matrix4.h"

namespace
{

template <typename Matrix>
std::array<float, 16> Copy_Matrix(const Matrix &matrix) noexcept
{
	std::array<float, 16> result{};
	for (std::size_t row = 0; row < 4; ++row)
		for (std::size_t column = 0; column < 4; ++column)
			result[row * 4 + column] = matrix[row][column];
	return result;
}

Graphics::CollisionBoxDrawData Make_Draw_Data(const CollisionBoxRenderObject &object)
{
	Graphics::CollisionBoxDrawData data;
	data.center = {object.Get_Local_Center().X, object.Get_Local_Center().Y, object.Get_Local_Center().Z};
	data.extent = {object.Get_Local_Extent().X, object.Get_Local_Extent().Y, object.Get_Local_Extent().Z};
	data.color = {object.Get_Color().X, object.Get_Color().Y, object.Get_Color().Z, object.Get_Opacity()};
	data.collision_type = static_cast<std::uint32_t>(object.Get_Collision_Type());
	data.front_counter_clockwise = !Get_W3D_Render_Services().Is_Reflection_Render_Pass();

	const auto &camera = Graphics::Get_Camera_Matrices();
	data.view_projection = Graphics::Compose_Matrices(camera.projection, camera.view).values;

	Matrix3D world_transform(true);
	if (object.Is_Oriented())
		world_transform = object.Get_Transform();
	else
		world_transform.Set_Translation(object.Get_Transform().Get_Translation());
	data.world = Copy_Matrix(Matrix4x4(world_transform));
	return data;
}

}

CollisionBoxRenderObject::CollisionBoxRenderObject()
{
	Set_Collision_Type(0);
	Update_Cached_Box();
}

CollisionBoxRenderObject::CollisionBoxRenderObject(const Assets::W3D::W3DBoxDescription &description)
	: m_name(description.name),
	  m_color(description.color[0] / 255.0f, description.color[1] / 255.0f, description.color[2] / 255.0f),
	  m_local_center(description.center.x, description.center.y, description.center.z),
	  m_local_extent(description.extent.x, description.extent.y, description.extent.z),
	  m_oriented(description.Is_Oriented())
{
	Set_Collision_Type(static_cast<int>(description.Collision_Type()));
	Update_Cached_Box();
}

CollisionBoxRenderObject::CollisionBoxRenderObject(const AABoxClass &box)
	: m_local_extent(box.Extent)
{
	Set_Collision_Type(0);
	Set_Position(box.Center);
	Update_Cached_Box();
}

CollisionBoxRenderObject::CollisionBoxRenderObject(const OBBoxClass &box)
	: m_local_extent(box.Extent),
	  m_oriented(true)
{
	Set_Collision_Type(0);
	Set_Transform(Matrix3D(box.Basis, box.Center));
	Update_Cached_Box();
}

CollisionBoxRenderObject::CollisionBoxRenderObject(const CollisionBoxRenderObject &source)
	: W3DRenderObject(source),
	  m_name(source.m_name),
	  m_color(source.m_color),
	  m_local_center(source.m_local_center),
	  m_local_extent(source.m_local_extent),
	  m_opacity(source.m_opacity),
	  m_oriented(source.m_oriented),
	  m_cached_aa_box(source.m_cached_aa_box),
	  m_cached_ob_box(source.m_cached_ob_box)
{
}

CollisionBoxRenderObject &CollisionBoxRenderObject::operator=(const CollisionBoxRenderObject &source)
{
	if (this != &source) {
		m_graphics.Release(Graphics::Get_Prop_Renderer());
		W3DRenderObject::operator=(source);
		m_name = source.m_name;
		m_color = source.m_color;
		m_local_center = source.m_local_center;
		m_local_extent = source.m_local_extent;
		m_opacity = source.m_opacity;
		m_oriented = source.m_oriented;
		m_cached_aa_box = source.m_cached_aa_box;
		m_cached_ob_box = source.m_cached_ob_box;
	}
	return *this;
}

W3DRenderObject *CollisionBoxRenderObject::Clone() const
{
	return W3DNEW CollisionBoxRenderObject(*this);
}

int CollisionBoxRenderObject::Class_ID() const
{
	return m_oriented ? W3DRenderObject::CLASSID_OBBOX : W3DRenderObject::CLASSID_AABOX;
}

void CollisionBoxRenderObject::Set_Name(const char *name)
{
	if (name != nullptr)
		m_name = name;
}

void CollisionBoxRenderObject::Render(W3DRenderContext &rinfo)
{
	(void)rinfo;
	const auto data = Make_Draw_Data(*this);
	m_graphics.Submit(Graphics::Get_Prop_Renderer(), Graphics::Get_Prop_Submission(), data);
}

void CollisionBoxRenderObject::Set_Transform(const Matrix3D &transform)
{
	W3DRenderObject::Set_Transform(transform);
	Update_Cached_Box();
}

void CollisionBoxRenderObject::Set_Position(const Vector3 &position)
{
	W3DRenderObject::Set_Position(position);
	Update_Cached_Box();
}

void CollisionBoxRenderObject::Set_Local_Center_Extent(const Vector3 &center, const Vector3 &extent)
{
	m_local_center = center;
	m_local_extent = extent;
	Update_Cached_Box();
}

void CollisionBoxRenderObject::Set_Local_Min_Max(const Vector3 &minimum, const Vector3 &maximum)
{
	m_local_center = (maximum + minimum) / 2.0f;
	m_local_extent = (maximum - minimum) / 2.0f;
	Update_Cached_Box();
}

void CollisionBoxRenderObject::Update_Cached_Box()
{
	m_cached_aa_box.Center = Get_Transform_No_Validity_Check().Get_Translation() + m_local_center;
	m_cached_aa_box.Extent = m_local_extent;
	Matrix3D::Transform_Vector(Get_Transform_No_Validity_Check(), m_local_center, &m_cached_ob_box.Center);
	m_cached_ob_box.Extent = m_local_extent;
	m_cached_ob_box.Basis.Set(Get_Transform_No_Validity_Check());
}

const AABoxClass &CollisionBoxRenderObject::Get_AA_Box() const
{
	Validate_Transform();
	const_cast<CollisionBoxRenderObject *>(this)->Update_Cached_Box();
	return m_cached_aa_box;
}

const OBBoxClass &CollisionBoxRenderObject::Get_OB_Box() const
{
	Validate_Transform();
	const_cast<CollisionBoxRenderObject *>(this)->Update_Cached_Box();
	return m_cached_ob_box;
}

bool CollisionBoxRenderObject::Cast_Ray(W3DRayCastQuery &raytest)
{
	if ((Get_Collision_Type() & raytest.CollisionType) == 0)
		return false;
	if (Is_Animation_Hidden() || raytest.Result->StartBad)
		return false;
	const bool collided = m_oriented
		? CollisionMath::Collide(raytest.Ray, m_cached_ob_box, raytest.Result)
		: CollisionMath::Collide(raytest.Ray, m_cached_aa_box, raytest.Result);
	if (collided)
		raytest.CollidedRenderObj = this;
	return collided;
}

bool CollisionBoxRenderObject::Cast_AABox(W3DBoxCastQuery &boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0 || boxtest.Result->StartBad)
		return false;
	const bool collided = m_oriented
		? CollisionMath::Collide(boxtest.Box, boxtest.Move, m_cached_ob_box, Vector3(0, 0, 0), boxtest.Result)
		: CollisionMath::Collide(boxtest.Box, boxtest.Move, m_cached_aa_box, boxtest.Result);
	if (collided)
		boxtest.CollidedRenderObj = this;
	return collided;
}

bool CollisionBoxRenderObject::Cast_OBBox(W3DOrientedBoxCastQuery &boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0 || boxtest.Result->StartBad)
		return false;
	const bool collided = m_oriented
		? CollisionMath::Collide(boxtest.Box, boxtest.Move, m_cached_ob_box, Vector3(0, 0, 0), boxtest.Result)
		: CollisionMath::Collide(boxtest.Box, boxtest.Move, m_cached_aa_box, Vector3(0, 0, 0), boxtest.Result);
	if (collided)
		boxtest.CollidedRenderObj = this;
	return collided;
}

bool CollisionBoxRenderObject::Intersect_AABox(W3DBoxIntersectionQuery &boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0)
		return false;
	return m_oriented
		? CollisionMath::Intersection_Test(m_cached_ob_box, boxtest.Box)
		: CollisionMath::Intersection_Test(m_cached_aa_box, boxtest.Box);
}

bool CollisionBoxRenderObject::Intersect_OBBox(W3DOrientedBoxIntersectionQuery &boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0)
		return false;
	return m_oriented
		? CollisionMath::Intersection_Test(m_cached_ob_box, boxtest.Box)
		: CollisionMath::Intersection_Test(m_cached_aa_box, boxtest.Box);
}

void CollisionBoxRenderObject::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
	sphere.Init(m_local_center, m_local_extent.Length());
}

void CollisionBoxRenderObject::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
	box.Init(m_local_center, m_local_extent);
}

Graphics::ModelFactory<W3DRenderObject> *Load_Collision_Box_Factory(ChunkLoadClass &cload)
{
	std::array<std::byte, Assets::W3D::W3DBoxPayloadSize> bytes{};
	if (cload.Cur_Chunk_Length() != bytes.size()
		|| cload.Read(bytes.data(), static_cast<unsigned>(bytes.size())) != bytes.size())
		return nullptr;

	Assets::W3D::W3DBoxDescription description;
	std::string error;
	if (!Assets::W3D::W3DRead_Box(bytes, description, error))
		return nullptr;
	const int class_id = description.Is_Oriented()
		? W3DRenderObject::CLASSID_OBBOX : W3DRenderObject::CLASSID_AABOX;
	return new Graphics::ModelFactory<W3DRenderObject>(description.name, class_id,
		[description]() -> W3DRenderObject * {
			return NEW_REF(CollisionBoxRenderObject, (description));
		});
}

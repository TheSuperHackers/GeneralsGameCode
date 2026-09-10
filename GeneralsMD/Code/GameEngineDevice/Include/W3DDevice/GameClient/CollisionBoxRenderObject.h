#pragma once

#include <string>

import Assets.Adapters.W3D.Box;
import Graphics.Scene.Debug.CollisionBox;
import Graphics.Scene.Models.Factory;

#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include "WWMath/obbox.h"

class ChunkLoadClass;
class W3DRenderContext;

class CollisionBoxRenderObject final : public W3DRenderObject
{
public:
	CollisionBoxRenderObject();
	explicit CollisionBoxRenderObject(const Assets::W3D::W3DBoxDescription &description);
	explicit CollisionBoxRenderObject(const AABoxClass &box);
	explicit CollisionBoxRenderObject(const OBBoxClass &box);
	CollisionBoxRenderObject(const CollisionBoxRenderObject &source);
	CollisionBoxRenderObject &operator=(const CollisionBoxRenderObject &source);
	~CollisionBoxRenderObject() override = default;

	W3DRenderObject *Clone() const override;
	int Class_ID() const override;
	int Get_Num_Polys() const override { return 12; }
	const char *Get_Name() const override { return m_name.c_str(); }
	void Set_Name(const char *name) override;
	void Render(W3DRenderContext &rinfo) override;

	void Set_Transform(const Matrix3D &transform) override;
	void Set_Position(const Vector3 &position) override;

	bool Cast_Ray(W3DRayCastQuery &raytest) override;
	bool Cast_AABox(W3DBoxCastQuery &boxtest) override;
	bool Cast_OBBox(W3DOrientedBoxCastQuery &boxtest) override;
	bool Intersect_AABox(W3DBoxIntersectionQuery &boxtest) override;
	bool Intersect_OBBox(W3DOrientedBoxIntersectionQuery &boxtest) override;

	void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
	void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;

	void Set_Color(const Vector3 &color) noexcept { m_color = color; }
	const Vector3 &Get_Color() const noexcept { return m_color; }
	void Set_Opacity(float opacity) noexcept { m_opacity = opacity; }
	float Get_Opacity() const noexcept { return m_opacity; }
	void Set_Local_Center_Extent(const Vector3 &center, const Vector3 &extent);
	void Set_Local_Min_Max(const Vector3 &minimum, const Vector3 &maximum);

	const Vector3 &Get_Local_Center() const noexcept { return m_local_center; }
	const Vector3 &Get_Local_Extent() const noexcept { return m_local_extent; }
	const AABoxClass &Get_AA_Box() const;
	const OBBoxClass &Get_OB_Box() const;
	bool Is_Oriented() const noexcept { return m_oriented; }

private:
	void Update_Cached_Box();

	std::string m_name;
	Vector3 m_color{1, 1, 1};
	Vector3 m_local_center{};
	Vector3 m_local_extent{1, 1, 1};
	float m_opacity = .25f;
	bool m_oriented = false;
	AABoxClass m_cached_aa_box;
	OBBoxClass m_cached_ob_box;
	Graphics::CollisionBoxRenderer m_graphics;
};

Graphics::ModelFactory<W3DRenderObject> *Load_Collision_Box_Factory(ChunkLoadClass &cload);

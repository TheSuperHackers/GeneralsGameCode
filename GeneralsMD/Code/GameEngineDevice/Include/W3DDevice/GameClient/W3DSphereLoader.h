#pragma once

#include <string>

import Assets.Spheres;
import Graphics.Scene.Models.Factory;
import Graphics.Scene.Sphere;

#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"

class ChunkLoadClass;
class W3DRenderContext;

// Native W3D ownership and scene callbacks for an authored sphere. The
// graphics module owns animation and prop submission; this adapter translates
// the W3DRenderObject contract and resource lifetime.
class W3DSphereRenderObject final : public W3DRenderObject
{
public:
	W3DSphereRenderObject();
	explicit W3DSphereRenderObject(const Assets::SphereAssetDesc &description);
	W3DSphereRenderObject(const W3DSphereRenderObject &source);
	W3DSphereRenderObject &operator=(const W3DSphereRenderObject &source);
	~W3DSphereRenderObject() override = default;

	W3DRenderObject *Clone() const override;
	int Class_ID() const override { return W3DRenderObject::CLASSID_SPHERE; }
	void Render(W3DRenderContext &rinfo) override;
	void Set_Transform(const Matrix3D &transform) override;
	void Set_Position(const Vector3 &position) override;

	void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
	void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;

	void Prepare_LOD(W3DCamera &camera) override;
	void Increment_LOD() override;
	void Decrement_LOD() override;
	float Get_Cost() const override;
	float Get_Value() const override;
	float Get_Post_Increment_Value() const override;
	void Set_LOD_Level(int lod) override;
	int Get_LOD_Level() const override;
	int Get_LOD_Count() const override;
	void Set_LOD_Bias(float bias) override;
	int Calculate_Cost_Value_Arrays(float screen_area, float *values,
		float *costs) const override;

	void Scale(float scale) override;
	void Scale(float scalex, float scaley, float scalez) override;

	void Set_Hidden(int onoff) override;
	void Set_Visible(int onoff) override;
	void Set_Animation_Hidden(int onoff) override;
	void Set_Force_Visible(int onoff) override;

	int Get_Num_Polys() const override;
	const char *Get_Name() const override { return m_name.c_str(); }
	void Set_Name(const char *name) override;

	void Set_Texture(W3DTextureHandle *texture);
	W3DTextureHandle *Peek_Texture() noexcept { return m_texture.Peek(); }

protected:
	void Update_Cached_Bounding_Volumes() const override;

private:
	static Assets::SphereAssetDesc With_Load_Fog(const Assets::SphereAssetDesc &description);

	std::string m_name;
	Graphics::SphereSceneObject m_sphere;
	RefCountPtr<W3DTextureHandle> m_texture;
	Graphics::SphereRenderer m_graphics;
};

Graphics::ModelFactory<W3DRenderObject> *Load_Sphere_Factory(ChunkLoadClass &cload);

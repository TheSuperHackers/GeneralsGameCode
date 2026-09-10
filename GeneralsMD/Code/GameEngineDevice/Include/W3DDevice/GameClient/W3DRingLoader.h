#pragma once

#include <string>

import Assets.Rings;
import Graphics.Scene.Models.Factory;
import Graphics.Scene.Ring.Renderer;
import Graphics.Scene.Ring.Runtime;

#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"

class ChunkLoadClass;
class W3DRenderContext;

// Native W3D ownership and scene callbacks for an authored ring. Geometry,
// animation, LOD selection, and draw submission stay in the graphics modules.
class W3DRingRenderObject final : public W3DRenderObject
{
public:
	W3DRingRenderObject();
	explicit W3DRingRenderObject(const Assets::RingAssetDesc &description);
	W3DRingRenderObject(const W3DRingRenderObject &source);
	W3DRingRenderObject &operator=(const W3DRingRenderObject &source);
	~W3DRingRenderObject() override = default;

	W3DRenderObject *Clone() const override;
	int Class_ID() const override { return W3DRenderObject::CLASSID_RING; }
	const char *Get_Name() const override { return m_name.c_str(); }
	void Set_Name(const char *name) override;
	int Get_Num_Polys() const override { return static_cast<int>(m_runtime.Num_Polys()); }
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

private:
	std::string m_name;
	Graphics::AuthoredRingRuntime m_runtime;
	RefCountPtr<W3DTextureHandle> m_texture;
	Graphics::AuthoredRingRenderer m_graphics;
};

Graphics::ModelFactory<W3DRenderObject> *Load_Ring_Factory(ChunkLoadClass &cload);

#pragma once

#include "WWLib/refcount.h"
#include "WWMath/vector3.h"

import Graphics.Scene.ObjectList;
import Graphics.Scene.Pass;
import Graphics.Scene.Traversal;

class W3DRenderObject;
class W3DRenderContext;
class W3DCamera;
class ChunkLoadClass;
class ChunkSaveClass;

// The iterator is deliberately small. The old only-visible argument was
// accepted but ignored, and callers still depend on traversing the complete
// render list when they pass true.
class W3DSceneIterator
{
public:
	virtual ~W3DSceneIterator() = default;
	virtual void First() = 0;
	virtual void Next() = 0;
	virtual bool Is_Done() = 0;
	virtual W3DRenderObject *Current_Item() = 0;

protected:
	W3DSceneIterator() = default;
};

// Game-facing scene identity and state. Object traversal and generic pass
// policy live in Graphics; this class only translates the existing game API
// and supplies the virtual scene lifecycle hooks used by derived scenes.
class W3DScene : public RefCountClass
{
public:
	W3DScene();
	~W3DScene() override;

	enum
	{
		SCENE_ID_UNKNOWN = 0xFFFFFFFF,
		SCENE_ID_SCENE = 0,
		SCENE_ID_SIMPLE,
		SCENE_ID_LAST = 0x0000FFFF,
	};

	virtual int Get_Scene_ID() const { return SCENE_ID_SCENE; }

	virtual void Add_Render_Object(W3DRenderObject *object);
	virtual void Remove_Render_Object(W3DRenderObject *object);

	virtual W3DSceneIterator *Create_Iterator(bool only_visible = false) = 0;
	virtual void Destroy_Iterator(W3DSceneIterator *iterator) = 0;

	virtual void Set_Ambient_Light(const Vector3 &color) { AmbientLight = color; }
	virtual const Vector3 &Get_Ambient_Light() { return AmbientLight; }

	virtual void Set_Fog_Enable(bool enabled) { FogEnabled = enabled; }
	virtual bool Get_Fog_Enable() { return FogEnabled; }
	virtual void Set_Fog_Color(const Vector3 &color) { FogColor = color; }
	virtual const Vector3 &Get_Fog_Color() { return FogColor; }
	virtual void Set_Fog_Range(float start, float end) { FogStart = start; FogEnd = end; }
	virtual void Get_Fog_Range(float *start, float *end) { *start = FogStart; *end = FogEnd; }

	enum PolyRenderType
	{
		POINT,
		LINE,
		FILL
	};

	void Set_Polygon_Mode(PolyRenderType mode) { PolyRenderMode = mode; }
	PolyRenderType Get_Polygon_Mode() { return PolyRenderMode; }

	enum ExtraPassPolyRenderType
	{
		EXTRA_PASS_DISABLE,
		EXTRA_PASS_LINE,
		EXTRA_PASS_CLEAR_LINE
	};

	void Set_Extra_Pass_Polygon_Mode(ExtraPassPolyRenderType mode)
	{
		ExtraPassPolyRenderMode = mode;
	}
	ExtraPassPolyRenderType Get_Extra_Pass_Polygon_Mode() { return ExtraPassPolyRenderMode; }

	enum RegType
	{
		ON_FRAME_UPDATE = 0,
		LIGHT,
		RELEASE,
	};

	virtual void Register(W3DRenderObject *object, RegType registration) = 0;
	virtual void Unregister(W3DRenderObject *object, RegType registration) = 0;

	virtual float Compute_Point_Visibility(W3DRenderContext &, const Vector3 &) { return 1.0f; }

	virtual void Save(ChunkSaveClass &save);
	virtual void Load(ChunkLoadClass &load);

	// Concrete game scene types can expose this entry point publicly. Keeping
	// it protected here preserves the original friend-based lifecycle boundary.
	virtual void Render(W3DRenderContext &context);

protected:
	virtual void Customized_Render(W3DRenderContext &context) = 0;
	virtual void Pre_Render_Processing(W3DRenderContext &) {}
	virtual void Post_Render_Processing(W3DRenderContext &) {}

	// The fields remain the serialized scene representation and are also read
	// by the game scene implementation while its larger renderer is migrated.
	Vector3 AmbientLight;
	PolyRenderType PolyRenderMode;
	ExtraPassPolyRenderType ExtraPassPolyRenderMode;
	bool FogEnabled;
	Vector3 FogColor;
	float FogStart;
	float FogEnd;

	Graphics::ScenePass &Scene_Pass() noexcept { return Pass; }
	const Graphics::ScenePass &Scene_Pass() const noexcept { return Pass; }

private:
	void Apply_Pass_Settings() noexcept;
	void Apply_Fog_Settings() noexcept;
	void Apply_Extra_Pass_Settings() noexcept;

	Graphics::ScenePass Pass;

	W3DScene(const W3DScene &) = delete;
	W3DScene &operator=(const W3DScene &) = delete;
};

// The simple scene adapter owns the four native registration lists and maps
// the old object API to Graphics.Scene.Traversal callbacks. The list
// references are views into that one owner; they do not duplicate storage.
class W3DSimpleScene : public W3DScene
{
public:
	W3DSimpleScene();
	~W3DSimpleScene() override;

	int Get_Scene_ID() const override { return SCENE_ID_SIMPLE; }

	void Add_Render_Object(W3DRenderObject *object) override;
	void Remove_Render_Object(W3DRenderObject *object) override;
	void Remove_All_Render_Objects();

	void Register(W3DRenderObject *object, RegType registration) override;
	void Unregister(W3DRenderObject *object, RegType registration) override;
	void Set_Ambient_Light(const Vector3 &color) override;

	W3DSceneIterator *Create_Iterator(bool only_visible = false) override;
	void Destroy_Iterator(W3DSceneIterator *iterator) override;

	virtual void Visibility_Check(W3DCamera *camera);
	float Compute_Point_Visibility(W3DRenderContext &context, const Vector3 &point) override;

protected:
	void Customized_Render(W3DRenderContext &context) override;
	void Post_Render_Processing(W3DRenderContext &context) override;

	Graphics::SceneTraversal<W3DRenderObject> Traversal;
	Graphics::SceneObjectList<W3DRenderObject> &RenderList;
	Graphics::SceneObjectList<W3DRenderObject> &UpdateList;
	Graphics::SceneObjectList<W3DRenderObject> &LightList;
	Graphics::SceneObjectList<W3DRenderObject> &ReleaseList;
};


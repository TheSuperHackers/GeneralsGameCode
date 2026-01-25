/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "always.h"
#include "vector3.h"
#include "matrix3d.h"
#include "rendobj.h"
#include "wwstring.h"
#include "proto.h"
#include "w3derr.h"
#include "shader.h"
#include "matrix4.h"

class CameraClass;
class DazzleVisibilityClass;
struct VertexFormatXYZNDUV2;

class DazzleInitClass
{
public:
	unsigned type;
	bool use_camera_translation;
	StringClass primary_texture_name;
	StringClass secondary_texture_name;
	StringClass lensflare_name;
	float halo_size_pow;
	float halo_intensity;
	float halo_area;
	float halo_intensity_pow;
	float halo_scale_x;
	float halo_scale_y;
	float dazzle_size_pow;
	float dazzle_intensity_pow;
	float dazzle_intensity;
	float dazzle_area;
	float dazzle_direction_area;
	Vector3 dazzle_direction;
	Vector3 dazzle_test_color;
	Vector3 dazzle_color;
	Vector3 halo_color;
	float dazzle_scale_x;
	float dazzle_scale_y;
	float fadeout_start;
	float fadeout_end;
	float size_optimization_limit;
	float history_weight;
	float radius;
	float blink_period;
	float blink_on_time;
};

class LensflareInitClass
{
public:
	LensflareInitClass()
		:
		flare_locations(0),
		flare_sizes(0),
		flare_colors(0),
		flare_uv(0)
	{
	}

	LensflareInitClass(const LensflareInitClass& lic)
		:
		type(lic.type),
		texture_name(lic.texture_name),
		flare_count(lic.flare_count),
		flare_locations(0),
		flare_sizes(0),
		flare_colors(0),
		flare_uv(0)
	{
		if (flare_count) {
			flare_locations=W3DNEWARRAY float[flare_count];
			memcpy(flare_locations,lic.flare_locations,sizeof(float)*flare_count);
			flare_sizes=W3DNEWARRAY float[flare_count];
			memcpy(flare_sizes,lic.flare_sizes,sizeof(float)*flare_count);
			flare_colors=W3DNEWARRAY Vector3[flare_count];
			memcpy(flare_colors,lic.flare_colors,sizeof(Vector3)*flare_count);
			flare_uv=W3DNEWARRAY Vector4[flare_count];
			memcpy(flare_uv,lic.flare_uv,sizeof(Vector4)*flare_count);
		}
	}

	~LensflareInitClass()
	{
		delete[] flare_locations;
		delete[] flare_sizes;
		delete[] flare_colors;
		delete[] flare_uv;
	}

	unsigned type;
	StringClass texture_name;
	int flare_count;
	float* flare_locations;
	float* flare_sizes;
	Vector3* flare_colors;
	Vector4* flare_uv;

};

class DazzleRenderObjClass;
class DazzleLayerClass;
class DazzleTypeClass
{
	friend DazzleRenderObjClass;
	friend DazzleLayerClass;

	TextureClass* primary_texture;
	TextureClass* secondary_texture;
	DazzleInitClass ic;
	float fadeout_end_sqr;
	float fadeout_start_sqr;
	StringClass name;
	unsigned dazzle_test_color_integer;
	unsigned dazzle_test_mask_integer;
	unsigned lensflare_id;

	ShaderClass dazzle_shader;
	ShaderClass halo_shader;

	float radius;

	DazzleTypeClass(const DazzleInitClass& is);
	virtual ~DazzleTypeClass();
public:

	virtual void Calculate_Intensities(
		float& dazzle_intensity,
		float& dazzle_size,
		float& halo_intensity,
		const Vector3& camera_dir,
		const Vector3& dazzle_dir,
		float distance) const;

	void Set_Dazzle_Shader(const ShaderClass& s);	// Set shader for the dazzle type
	void Set_Halo_Shader(const ShaderClass& s);	// Set shader for the dazzle type

	TextureClass* Get_Dazzle_Texture();
	TextureClass* Get_Halo_Texture();

};


// The DazzleLayerClass is for all the dazzles being rendered with a given
// group of camera settings: for example, different scenes may use different
// z-buffer settings and in such a case each scene should have a dazzle layer
// associated with it. (In some special cases a scene may have more than one
// dazzle layer). A dazzle layer contains visible and invisible lists for
// each dazzle type. During rendering each dazzle is put on the correct list
// (a "current dazzle layer" static variable is set before rendering the
// appropriate scenes to ensure this). After all scenes are rendered, the
// dazzle layers are rendered one by one with the correct camera settings.
// NOTE: dazzle layers must be constructed AFTER all the dazzle types have
// been initialized, since the constructor needs to know how many dazzle types
// there are.
class DazzleLayerClass {

	friend DazzleRenderObjClass;

	public:

		DazzleLayerClass(void);
		~DazzleLayerClass(void);

		// Render all dazzles in this layer (DazzleRenderObj::Render() only sets visibility)
		void Render(CameraClass* camera);

	private:

		virtual int Get_Visible_Item_Count(unsigned int type) const;	// Return visible item count
//		virtual void Get_Visible_Item_Locations(unsigned int type, Vector3* locations) const;	// Copy locations of visible items to buffer
		virtual void Clear_Visible_List(unsigned int type);

		// We have an array of visible lists (one for each dazzle type).
		DazzleRenderObjClass** visible_lists;

};

class LensflareTypeClass
{
	friend DazzleLayerClass;
	friend DazzleRenderObjClass;

	TextureClass* texture;
	LensflareInitClass lic;
	StringClass name;

	LensflareTypeClass(const LensflareInitClass& is);
	virtual ~LensflareTypeClass();
public:
	TextureClass* Get_Texture();

	void Generate_Vertex_Buffers(
		VertexFormatXYZNDUV2* vertex,
		int& vertex_count,
		float screen_x_scale,
		float screen_y_scale,
		float dazzle_intensity,
		const Vector4& transformed_location);

	void Render_Arrays(
		const Vector4* vertex_coordinates,
		const Vector2* uv_coordinates,
		const Vector3* color,
		int vertex_count,
		int halo_vertex_count,
		const Vector2* texture_coordinates);

};

class INIClass;

class DazzleRenderObjClass : public RenderObjClass
{
	friend DazzleLayerClass;

	DazzleRenderObjClass * succ;
	unsigned type;
	float current_dazzle_intensity;
	float current_dazzle_size;
	float current_halo_intensity;
	float current_distance;
	Vector4 transformed_loc;
	Vector3 current_vloc;
	Vector3 current_dir;
	Vector3 dazzle_color;
	Vector3 halo_color;
	float	  lensflare_intensity;
	float visibility;
	bool on_list;	// This is used to avoid insterting a dazzle into a list twice.
	float radius;	// Used to cast rays against
	unsigned int creation_time;

	static bool	_dazzle_rendering_enabled;

//	static void Draw_Debug_Dazzle(int idx);
	void vis_render_dazzle(SpecialRenderInfoClass & rinfo);

	void Render_Dazzle(CameraClass* camera);

public:

	DazzleRenderObjClass(unsigned type);
	DazzleRenderObjClass(const char* type_name);
	DazzleRenderObjClass(const DazzleRenderObjClass & src);
	DazzleRenderObjClass & operator = (const DazzleRenderObjClass &);

	DazzleRenderObjClass* Succ() { return succ; }
	const DazzleRenderObjClass* Succ() const { return succ; }

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void)	const { return CLASSID_DAZZLE; }

	virtual void					Render(RenderInfoClass & rinfo);
	virtual void Special_Render(SpecialRenderInfoClass & rinfo);
	virtual void 					Set_Transform(const Matrix3D &m);
   virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
   virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const;
	virtual void					Scale(float scale) 															{ radius*=scale; };

	void Set_Dazzle_Color(const Vector3& col) { dazzle_color=col; }
	void Set_Halo_Color(const Vector3& col) { halo_color=col; }
	void Set_Lensflare_Intensity (float intensity) {lensflare_intensity=intensity;}

	unsigned int					Get_Dazzle_Type(void) { return type; }

	// Usually, a DazzleRenderObj adds itself to the appropriate visible list
	// (determined by the current layer) when it is rendered. This does not
	// work for dazzles with "camera transform off", since they are located in
	// camera space and the standard worldspace visibility algo will give
	// unpredictable results for them (they may never have a Render() call).
	// So for these dazzles, you need to call Set_Layer() after constructing
	// them (this is instead of putting them in a scene). This function adds
	// the dazzle to the appropriate visible list. NOTE: It is also called
	// internally by the Render() function.
	void Set_Layer(DazzleLayerClass *layer);

	// Persistent object save-load interface
	// Dazzles save their "dazzle-type" and transform
	virtual const PersistFactoryClass &	Get_Factory (void) const;

	// Set the static "current layer" variable. This variable is used in the
	// Render() call so that the dazzle knows which list to add itself to if
	// it is visible. This function must be called before rendering the
	// scene(s) in which the dazzles are present.
	static void Set_Current_Dazzle_Layer(DazzleLayerClass *layer);

	static void Init_Type(const DazzleInitClass& i);
	static void Init_Lensflare(const LensflareInitClass& i);
	static void Init_From_INI(const INIClass* ini);
	static unsigned Get_Type_ID(const char* name);	// Return the ID of type with given name, or INT_MAX if failed
	static const char * Get_Type_Name(unsigned int id);	// Return the name of the type with the given ID
	static DazzleTypeClass* Get_Type_Class(unsigned id);	// Return dazzle type class pointer, or null if not found
																			// The pointer is NOT refcounted - all types are deinitialised
																			// when exiting the level.
	static unsigned Get_Lensflare_ID(const char* name);	// Return the ID of lensflare with given name, or INT_MAX if failed
	static LensflareTypeClass* Get_Lensflare_Class(unsigned id);	// Return lensflare type class pointer, or null if not found

	static void Deinit();

	// Install a class derived from DazzleVisibilityClass to add app-specific
	// visibility determination.  The default behavior will ask the scene which
	// the dazzle is a member of to compute its visibility.
	static void Install_Dazzle_Visibility_Handler(const DazzleVisibilityClass * visibility_handler);

	// Globally disable/enable dazzle rendering
	static void Enable_Dazzle_Rendering(bool onoff) { _dazzle_rendering_enabled = onoff; }
	static bool Is_Dazzle_Rendering_Enabled(void) { return _dazzle_rendering_enabled; }
};


/**
** DazzleVisibilityClass
** The user should derive a class from DazzleVisibilityClass and implement an app-specific
** dazzle visibility test.  Renegade will use ray-casting to determine visibility.  The
** default visibility handler will query the scene which the dazzle is contained in.
*/
class DazzleVisibilityClass
{
public:
	virtual float Compute_Dazzle_Visibility(	RenderInfoClass & rinfo,
															DazzleRenderObjClass * dazzle,
															const Vector3 & point) const;
};


/**
** DazzlePrototypeClass
** This description object is generated when reading a W3D_CHUNK_DAZZLE.  It stores the
** information needed to construct a particular instance of a dazzle.  Prototypes are
** stored in the asset manager and used to construct render objects when needed.
*/
class DazzlePrototypeClass : public W3DMPO, public PrototypeClass
{
	W3DMPO_GLUE(DazzlePrototypeClass)
public:
	DazzlePrototypeClass(void) : DazzleType(0)				{ }

	virtual const char *			Get_Name(void) const			{ return Name; }
	virtual int								Get_Class_ID(void) const	{ return RenderObjClass::CLASSID_DAZZLE; }
	virtual RenderObjClass *	Create(void);
	virtual void							DeleteSelf()							{ delete this; }

	WW3DErrorType					Load_W3D(ChunkLoadClass & cload);

private:

	StringClass				Name;
	int								DazzleType;
};


/**
** DazzleLoaderClass
** An instance of this class is registered with the asset manager and handles loading W3D_CHUNK_DAZZLE.
** It creates DazzlePrototypes from the data in the chunk.
*/
class DazzleLoaderClass : public PrototypeLoaderClass
{
public:
	DazzleLoaderClass(void)														{ }
	~DazzleLoaderClass(void)													{ }

	virtual int						Chunk_Type(void)							{ return W3D_CHUNK_DAZZLE; }
	virtual PrototypeClass *	Load_W3D(ChunkLoadClass & cload);
};

extern DazzleLoaderClass		_DazzleLoader;

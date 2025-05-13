/*
**	Command & Conquer Generals Zero Hour(tm)
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

/*************************************************************************** 
 ***    C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S     *** 
 *************************************************************************** 
 *                                                                         * 
 *                 Project Name : G                                        * 
 *                                                                         * 
 *                     $Archive:: /VSS_Sync/ww3d2/part_emt.h              $* 
 *                                                                         * 
 *                      $Author:: Vss_sync                                $* 
 *                                                                         * 
 *                     $Modtime:: 8/29/01 7:29p                           $* 
 *                                                                         * 
 *                    $Revision:: 10                                      $* 
 *                                                                         * 
 *-------------------------------------------------------------------------* 
 * Functions:                                                              * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PART_EMT_H
#define PART_EMT_H

#include "rendobj.h"
#include "RANDOM.H"
#include "part_buf.h"
#include "quat.h"
#include "w3d_file.h"
#include "w3derr.h"
#include "v3_rnd.h"

// Forward declarations
class ParticleEmitterDefClass;
class ChunkSaveClass;
struct NewParticleStruct;

/*
** Properties / Keyframes: These are for properties of the particles which
** are a straight function of their age, such as color and size. These are
** typically defined by a series of "keyframes", which are property / age
** pairs. Linear interpolation is performed between the keyframes.
*/
template<class T> struct ParticlePropertyStruct {
	T					Start;
	T					Rand;
	unsigned int	NumKeyFrames;
	float *			KeyTimes;
	T *				Values;
};


/*
** Utility function for copying the contents of one keyframe struct into another.
*/
template<class T> __inline 
void Copy_Emitter_Property_Struct
(
	ParticlePropertyStruct<T> &dest,
	const ParticlePropertyStruct<T> &src	
)
{
	dest.Start			= src.Start;
	dest.Rand			= src.Rand;
	dest.NumKeyFrames = src.NumKeyFrames;
	dest.KeyTimes		= NULL;
	dest.Values			= NULL;

	if (dest.NumKeyFrames > 0) {
		dest.KeyTimes	=  W3DNEWARRAY float[dest.NumKeyFrames];
		dest.Values		=  W3DNEWARRAY T[dest.NumKeyFrames];
		::memcpy (dest.KeyTimes, src.KeyTimes, sizeof (float) * dest.NumKeyFrames);
		::memcpy (dest.Values, src.Values, sizeof (T) * dest.NumKeyFrames);
	}

	return ;
}


/**
** ParticleEmitterClass: This is a renderobject which emits particles
** regularly. The particles emitted go into a different renderobject, a
** ParticleBufferClass (there is one particle buffer per particle emitter).
** This separation is so that the bounding volumes of the particle group and
** the object containing the emitter (emitters will typically be inserted
** into a hierarchy object or some such) will remain separate.
*/
class ParticleEmitterClass : public RenderObjClass
{
	public:

		// Note: all time/velocity/acceleration quantities use seconds (converted to ms internally)
		ParticleEmitterClass(float emit_rate, unsigned int burst_size, Vector3Randomizer *pos_rnd,
			Vector3 base_vel, Vector3Randomizer *vel_rnd, float out_vel, float vel_inherit_factor, 
			ParticlePropertyStruct<Vector3> &color, 
			ParticlePropertyStruct<float> &opacity,
			ParticlePropertyStruct<float> &size, 
			ParticlePropertyStruct<float> &rotation, float orient_rnd,
			ParticlePropertyStruct<float> &frames,
			ParticlePropertyStruct<float> &blur_times,
			Vector3 accel, float max_age, float future_start, TextureClass *tex,
			ShaderClass shader = ShaderClass::_PresetAdditiveSpriteShader, 
			int max_particles = 0, int max_buffer_size = -1, bool pingpong = false,
			int render_mode = W3D_EMITTER_RENDER_MODE_TRI_PARTICLES,
			int frame_mode = W3D_EMITTER_FRAME_MODE_1x1,
			const W3dEmitterLinePropertiesStruct * line_props = NULL);
				
		ParticleEmitterClass(const ParticleEmitterClass & src);
		ParticleEmitterClass & operator = (const ParticleEmitterClass &);
		virtual ~ParticleEmitterClass(void);				
		
		// Creation/serialization methods
		virtual RenderObjClass *		Clone(void) const;
		static ParticleEmitterClass * Create_From_Definition (const ParticleEmitterDefClass &definition);
		ParticleEmitterDefClass *		Build_Definition (void) const;
		WW3DErrorType						Save (ChunkSaveClass &chunk_save) const;

		// Identification methods
		virtual int				Class_ID (void) const { return CLASSID_PARTICLEEMITTER; }
		virtual const char *	Get_Name (void) const { return NameString; }
		virtual void			Set_Name (const char *pname);

		virtual void			Notify_Added(SceneClass * scene);
		virtual void			Notify_Removed(SceneClass * scene);

		// Update particle state and draw the particles.
		virtual void			Render(RenderInfoClass & rinfo) { }
		virtual void			Restart(void);

		// Scales the size of all particles and effects positions/velocities of
		// particles emitted after the Scale() call (but not before)
		virtual void			Scale(float scale);

		// Put particle buffer in scene if this is the first time (clunky code
		// - hopefully can be rewritten more cleanly in future)...
		virtual void			On_Frame_Update(void);

      virtual void			Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const { sphere.Center.Set(0,0,0); sphere.Radius = 0; }
      virtual void			Get_Obj_Space_Bounding_Box(AABoxClass & box) const { box.Center.Set(0,0,0); box.Extent.Set(0,0,0); }
		virtual void			Set_Hidden(int onoff)				{ RenderObjClass::Set_Hidden (onoff); Update_On_Visibilty (); }
		virtual void			Set_Visible(int onoff)				{ RenderObjClass::Set_Visible (onoff); Update_On_Visibilty (); }
		virtual void			Set_Animation_Hidden(int onoff)	{ RenderObjClass::Set_Animation_Hidden (onoff); Update_On_Visibilty (); }
		virtual void			Set_Force_Visible(int onoff)		{ RenderObjClass::Set_Force_Visible (onoff); Update_On_Visibilty (); }

		virtual void			Set_LOD_Bias(float bias)			{ if (Buffer) Buffer->Set_LOD_Bias(bias); }


		// These are not part of the renderobject interface:
		// You can use Reset() to re-cycle a particle emitter.  Make sure its transform is
		// set up before you call Reset().
		void						Reset(void);
		void						Start(void);
		void						Stop(void);
		bool						Is_Stopped(void);

		// Yet another way to make an emitter stop rendering (besides setting Hidden or Animation_Hidden).
		// We added this because we needed a way independent from the other two. This would be appropriate
		// to set, for example, when setting RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY on the container.
		void						Set_Invisible(bool onoff)	{ IsInvisible = onoff; }
		bool						Is_Invisible(void)			{ return IsInvisible; }
		
		// Change starting position/velocity/acceleration parameters:
		void Set_Position_Randomizer(Vector3Randomizer *rand);
		void Set_Velocity_Randomizer(Vector3Randomizer *rand);
		void Set_Base_Velocity(const Vector3& base_vel);
		void Set_Outwards_Velocity(float out_vel);
		void Set_Velocity_Inheritance_Factor(float inh_factor);
		void Set_Acceleration (const Vector3 &acceleration)	{ if (Buffer != NULL) Buffer->Set_Acceleration (acceleration/1000000.0f); }
	
		// Change visual properties of emitter / buffer:
		void Reset_Colors(ParticlePropertyStruct<Vector3> &new_props)							{ if (Buffer) Buffer->Reset_Colors(new_props); }
		void Reset_Opacity(ParticlePropertyStruct<float> &new_props)							{ if (Buffer) Buffer->Reset_Opacity(new_props); }
		void Reset_Size(ParticlePropertyStruct<float> &new_props)								{ if (Buffer) Buffer->Reset_Size(new_props); }
		void Reset_Rotations(ParticlePropertyStruct<float> &new_props, float orient_rnd)	{ if (Buffer) Buffer->Reset_Rotations(new_props, orient_rnd); }
		void Reset_Frames(ParticlePropertyStruct<float> &new_props)								{ if (Buffer) Buffer->Reset_Frames(new_props); }
		void Reset_Blur_Times(ParticlePropertyStruct<float> &new_props)								{ if (Buffer) Buffer->Reset_Blur_Times(new_props); }

		// Change emission/burst rate, or tell the emitter to emit a one-time burst.
		// NOTE: default buffer size fits the emission/burst rate that the emitter was created with.
		// Setting the rates too high will cause particles to be prematurely killed (to make space
		// for new ones) - setting it too low will cause wasted space in the particle buffer.
		// These problems can be avoided by specifying the desired buffer size in the emitter CTor.
		void						Set_Emission_Rate (float rate)	{ EmitRate = rate > 0.0f ? (unsigned int)(1000.0f / rate) : 1000U; }
		void						Set_Burst_Size (int size)			{ BurstSize	= size != 0 ? size : 1; }
		void						Set_One_Time_Burst(int size)		{ OneTimeBurstSize = size != 0 ? size : 1; OneTimeBurst = true; }

		// Emit particles (put in particle buffer). This is called by the
		// particle buffer On_Frame_Update() function to avoid order dependence.
		virtual void Emit(void);

		// Buffer control
		ParticleBufferClass *Peek_Buffer( void )					{ return Buffer; }
		void						Buffer_Scene_Not_Needed( void )	{ BufferSceneNeeded = false; }
		void						Remove_Buffer_From_Scene (void)	{ Buffer->Remove (); FirstTime = true; BufferSceneNeeded = true; }

		// from RenderObj...
      virtual bool			Is_Complete(void)							{ return IsComplete; }
		
		// Auto deletion behavior controls
		bool						Is_Remove_On_Complete_Enabled(void)				{ return RemoveOnComplete; }
		void						Enable_Remove_On_Complete(bool onoff)			{ RemoveOnComplete = onoff; }
		static bool				Default_Remove_On_Complete (void)				{ return DefaultRemoveOnComplete; }
		static void				Set_Default_Remove_On_Complete (bool onoff)	{ DefaultRemoveOnComplete = onoff; }

		//
		//	Virtual accessors (used for type specific information)
		//
		virtual int				Get_User_Type (void) const			{ return EMITTER_TYPEID_DEFAULT; }
		virtual const char *	Get_User_String (void) const		{ return NULL; }

		//
		// Inline accessors.
		//	These methods are provided as a means to get the emitter's settings.
		//
		int						Get_Render_Mode (void) const		{ return Buffer->Get_Render_Mode(); }
		int						Get_Frame_Mode (void) const		{ return Buffer->Get_Frame_Mode(); }
		float						Get_Particle_Size (void) const	{ return Buffer->Get_Particle_Size(); }
		Vector3					Get_Acceleration (void) const		{ return Buffer->Get_Acceleration (); }
		float						Get_Lifetime (void) const			{ return Buffer->Get_Lifetime (); }
		float						Get_Future_Start_Time (void) const { return Buffer->Get_Future_Start_Time(); }
		Vector3					Get_End_Color (void) const			{ return Buffer->Get_End_Color (); }
		float						Get_End_Opacity (void) const		{ return Buffer->Get_End_Opacity (); }
		TextureClass *			Get_Texture (void) const			{ return Buffer->Get_Texture (); }
		void						Set_Texture (TextureClass *tex)  { Buffer->Set_Texture(tex); }
		float						Get_Fade_Time (void) const			{ return Buffer->Get_Fade_Time (); }
		Vector3					Get_Start_Color (void) const		{ return Buffer->Get_Start_Color(); }
		float						Get_Start_Opacity (void) const	{ return Buffer->Get_Start_Opacity(); }
		float						Get_Position_Random (void) const	{ return PosRand ? PosRand->Get_Maximum_Extent() : 0.0f; }
		//float						Get_Velocity_Random (void) const	{ return VelRand ? (VelRand->Get_Maximum_Extent() * 1000.0f) : 0.0f; }
		float						Get_Emission_Rate (void) const	{ return 1000.0f / float(EmitRate); }
		int						Get_Burst_Size (void) const		{ return BurstSize; }
		int						Get_Max_Particles (void) const	{ return MaxParticles; }
		Vector3					Get_Start_Velocity (void) const	{ return BaseVel * 1000.0F; }
		Vector3Randomizer *	Get_Creation_Volume (void) const;
		Vector3Randomizer *	Get_Velocity_Random (void) const;
		float						Get_Outwards_Vel (void) const		{ return OutwardVel * 1000.0F; }
		float						Get_Velocity_Inherit (void) const{ return VelInheritFactor; }
		ShaderClass				Get_Shader (void) const				{ return Buffer->Get_Shader (); }

		// Note: Caller IS RESPONSIBLE for freeing any memory allocated by these calls
		void						Get_Color_Key_Frames (ParticlePropertyStruct<Vector3>	&colors) const			{ Buffer->Get_Color_Key_Frames (colors); }
		void						Get_Opacity_Key_Frames (ParticlePropertyStruct<float>	&opacities) const		{ Buffer->Get_Opacity_Key_Frames (opacities); }
		void						Get_Size_Key_Frames (ParticlePropertyStruct<float>	&sizes) const				{ Buffer->Get_Size_Key_Frames (sizes); }
		void						Get_Rotation_Key_Frames (ParticlePropertyStruct<float> &rotations) const	{ Buffer->Get_Rotation_Key_Frames (rotations); }
		void						Get_Frame_Key_Frames (ParticlePropertyStruct<float> &frames) const			{ Buffer->Get_Frame_Key_Frames (frames); }
		void						Get_Blur_Time_Key_Frames (ParticlePropertyStruct<float> &blurtimes) const	{ Buffer->Get_Blur_Time_Key_Frames (blurtimes); }
		float						Get_Initial_Orientation_Random (void) const											{ return Buffer->Get_Initial_Orientation_Random(); }

		// Line rendering accessors
		int						Get_Line_Texture_Mapping_Mode(void) const	{ return Buffer->Get_Line_Texture_Mapping_Mode(); }
		int						Is_Merge_Intersections(void) const			{ return Buffer->Is_Merge_Intersections(); }
		int						Is_Freeze_Random(void) const					{ return Buffer->Is_Freeze_Random(); }
		int						Is_Sorting_Disabled(void) const				{ return Buffer->Is_Sorting_Disabled(); }
		int						Are_End_Caps_Enabled(void)	const				{ return Buffer->Are_End_Caps_Enabled(); }
		int						Get_Subdivision_Level(void) const			{ return Buffer->Get_Subdivision_Level(); }
		float						Get_Noise_Amplitude(void) const				{ return Buffer->Get_Noise_Amplitude(); }
		float						Get_Merge_Abort_Factor(void) const			{ return Buffer->Get_Merge_Abort_Factor(); }
		float						Get_Texture_Tile_Factor(void) const			{ return Buffer->Get_Texture_Tile_Factor(); }
		Vector2					Get_UV_Offset_Rate(void) const				{ return Buffer->Get_UV_Offset_Rate(); }

		// Global debugging option for disabling all particle emission
#ifdef WWDEBUG
		static void				Disable_All_Emitters(bool onoff)	{ DebugDisable = onoff; }
		static bool				Are_Emitters_Disabled(void)		{ return DebugDisable; }
#endif

	protected:

		// Used to build a list of filenames this emitter is dependent on
		virtual void			Add_Dependencies_To_List (DynamicVectorClass<StringClass> &file_list, bool textures_only = false);
		
		// This method is called each time the visiblity state of the emitter changes.
		virtual void			Update_On_Visibilty (void);

	private:		

		// Collision sphere is a point - emitter emits also when not visible,
      // so this is only important to avoid affecting the collision spheres
      // of heirarchy objects into which the emitter is inserted.
		virtual void Update_Cached_Bounding_Volumes(void) const;

      // Create new particles and pass them to the particle buffer. Receives
      // the end-of-interval quaternion and origin and interpolates between
      // them and PrevQ/PrevOrig to get the transform to apply to each
      // particle's position and velocity as it is created. New particles are
      // placed into a circular queue for processing by the particle buffer.
		void Create_New_Particles(const Quaternion & curr_quat, const Vector3 & curr_orig);

		// Initialize one new particle at the given NewParticleStruct
		// address, with the given age and emitter transform (expressed as a
		// quaternion and origin vector). (must check if address is NULL).
		void Initialize_Particle(NewParticleStruct * newpart, unsigned int age,
			const Quaternion & quat, const Vector3 & orig);

		unsigned int				EmitRate;			// Emission rate (1/milliseconds).
		unsigned int				BurstSize;			// Burst size (how many particles in each emission).
		unsigned int				OneTimeBurstSize;	// Burst size for a one-time burst.
		bool							OneTimeBurst;		// Do we need to do a one-time burst?
		Vector3Randomizer *		PosRand;				// Position randomizer pointer (may be NULL).
		Vector3						BaseVel;				// Base initial emission velocity.
		Vector3Randomizer *		VelRand;				// Velocity randomizer pointer (may be NULL).
		float							OutwardVel;			// Size of outwards velocity.
		float							VelInheritFactor;	// Affects emitter vel. inherited by particles.
		unsigned int				EmitRemain;			// Millisecond emitter remainder.
		Quaternion 					PrevQ;				// Previous quaternion (for interpolation).
		Vector3						PrevOrig;			// Previous origin (for interpolation).
		bool							Active;				// Is the emitter currently Active?
		bool							FirstTime;			// Has it been rendered before?
		bool							BufferSceneNeeded;// Does the buffer need a scene?
		int							ParticlesLeft;		// Particles left to emit
		int							MaxParticles;		// Total particles to emit
		bool							IsComplete;			// Completed Emissions		
		char *						NameString;
		char *						UserString;
		bool							RemoveOnComplete;	// Should this emitter destroy itself when it completes?
		bool							IsInScene;
		unsigned char				GroupID;				// The group ID of a particle. A start causes the group ID to increment.		

		// This pointer is used only for sending new particles to the particle
		// buffer and for informing the buffer when the emitter is destroyed.
		ParticleBufferClass *	Buffer;

		// See comments on Set/Is_Invisible
		bool							IsInvisible;

		// This is used to set the global behavior of emitters...
		// Should they be removed from the scene when they complete their
		// emissions, or should they stay in the scene.  (For editing purposes)
		static bool					DefaultRemoveOnComplete;

		// This setting is used in debug and profile builds to disable
		// all particle emitters.
		static bool					DebugDisable;
};

#endif // PART_EMT_H



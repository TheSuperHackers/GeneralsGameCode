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
 *                     $Archive:: /VSS_Sync/ww3d2/part_buf.h              $*
 *                                                                         *
 *                      $Author:: Vss_sync                                $*
 *                                                                         *
 *                     $Modtime:: 8/29/01 7:29p                           $*
 *                                                                         *
 *                    $Revision:: 9                                       $*
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#pragma once

#ifndef PART_BUF_H
#define PART_BUF_H

#include "rendobj.h"
#include "pointgr.h"
#include "seglinerenderer.h"
#include "linegrp.h"

class ParticleEmitterClass;
template<class T> struct ParticlePropertyStruct;

/**
** NewParticleStruct: structure for passing new particles from the particle
** emitter to the particle buffer. Since the emitter always continues
** emitting (unless stopped) but the buffer may not update for long periods,
** the emitter may emit more particles than the buffer can contain. However,
** in this case the older particles can be ignored. Therefore
** ParticleBufferClass contains a circular buffer of NewParticleStructs, and
** new ones overwrite the oldest in the case of overflows.
*/
struct NewParticleStruct
{
	Vector3			Position;	// Particle position in worldspace.
	Vector3			Velocity;	// Particle velocity in worldspace.
	unsigned int	TimeStamp;	// Millisecond time at creation.
	unsigned char	GroupID;		// Group ID of the particle

	// These are needed by DynamicVectorClass (will probably never be used).
	bool operator != (const NewParticleStruct & p)
	{
		return (p.TimeStamp != TimeStamp) || (p.Position != Position);
	}
	bool operator == (const NewParticleStruct & p)
	{
		return (p.TimeStamp == TimeStamp) && (p.Position == Position);
	}
};

/**
** ParticleBufferClass: This is a renderobject which contains the particles
** emitted by a given renderer. The particle emitter is a different
** renderobject, a ParticleEmitterClass (there is one particle emitter per
** particle buffer). This separation is so that the bounding volumes of the
** particle group and the object containing the emitter (emitters will
** typically be inserted into a hierarchy object or some such) will remain
** separate.
*/
class ParticleBufferClass : public RenderObjClass
{
	public:

		ParticleBufferClass(ParticleEmitterClass *emitter, unsigned int buffer_size,
			ParticlePropertyStruct<Vector3> &color, ParticlePropertyStruct<float> &opacity,
			ParticlePropertyStruct<float> &size, ParticlePropertyStruct<float> &rotation,
			float orient_rnd, ParticlePropertyStruct<float> &frame,
			ParticlePropertyStruct<float> &blurtime, Vector3 accel,
			float max_age, float future_start, TextureClass *tex, ShaderClass shader, bool pingpong,
			int render_mode, int frame_mode, const W3dEmitterLinePropertiesStruct * line_props);

		ParticleBufferClass(const ParticleBufferClass & src);
		ParticleBufferClass & operator = (const ParticleBufferClass &);
		virtual ~ParticleBufferClass(void);

		/*
		** RenderObjClass Interface:
		*/
		virtual RenderObjClass * Clone(void) const;
		virtual int Class_ID(void) const { return CLASSID_PARTICLEBUFFER; }

		virtual int Get_Num_Polys(void) const;
		int			Get_Particle_Count(void) const;

		// Update particle state and draw the particles.
		virtual void Render(RenderInfoClass & rinfo);

		// Scales the size of the individual particles but doesn't affect their
		// position (and therefore the size of the particle system as a whole)
		virtual void Scale(float scale);

		// The particle buffer never receives a Set_Transform/Position call,
		// evem though its bounding volume changes. Since bounding volume
		// invalidations ordinarily occur when these functions are called,
		// the cached bounding volumes will not be invalidated unless we do
		// it elsewhere (such as here). We also need to call the particle
		// emitter's Emit() function (done here to avoid order dependence).
		virtual void On_Frame_Update(void);

		virtual void Notify_Added(SceneClass * scene);
		virtual void Notify_Removed(SceneClass * scene);

		virtual void Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
		virtual void Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

		/////////////////////////////////////////////////////////////////////////////
		// Render Object Interface - Predictive LOD
		/////////////////////////////////////////////////////////////////////////////

		virtual void	Prepare_LOD(CameraClass &camera);
		virtual void	Increment_LOD(void);
		virtual void	Decrement_LOD(void);
		virtual float	Get_Cost(void) const;
		virtual float	Get_Value(void) const;
		virtual float	Get_Post_Increment_Value(void) const;
		virtual void	Set_LOD_Level(int lod);
		virtual int		Get_LOD_Level(void) const;
		virtual int		Get_LOD_Count(void) const;
		virtual void	Set_LOD_Bias(float bias)	{ LodBias = MAX(bias, 0.0f); }
		virtual int		Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const;

		/*
		** These members are not part of the RenderObjClass Interface:
		*/
		void Reset_Colors(ParticlePropertyStruct<Vector3> &new_props);
		void Reset_Opacity(ParticlePropertyStruct<float> &new_props);
		void Reset_Size(ParticlePropertyStruct<float> &new_props);
		void Reset_Rotations(ParticlePropertyStruct<float> &new_rotations, float orient_rnd);
		void Reset_Frames(ParticlePropertyStruct<float> &new_frames);
		void Reset_Blur_Times(ParticlePropertyStruct<float> &new_blur_times);

		// This informs the buffer that the emitter is dead, so it can release
		// its pointer to it and be removed itself after all its particles dies
		// out.
		void Emitter_Is_Dead(void);

		// This set's the buffer's current emitter - this should usually be
		// called only by the emitter's copy constructor after it clones a
		// buffer.
		void Set_Emitter(ParticleEmitterClass *emitter);

		// from RenderObj...
      virtual bool	Is_Complete(void)		{ return IsEmitterDead && !NonNewNum && !NewNum; }

		// This adds an uninitialized NewParticleStuct to the new particle
		// buffer and returns its address so the particle emitter can
		// initialize it. This is how the emitter sends new particles to the
		// buffer - it is done this way to avoid needless copying.
		NewParticleStruct * Add_Uninitialized_New_Particle(void);

		//	Change the acceleration of the particles on the fly
		void Set_Acceleration (const Vector3 &acceleration)	{ Accel = acceleration;  HasAccel = ((Accel.X != 0) || (Accel.Y != 0) || (Accel.Z != 0)); }

		//
		// Inline accessors.
		//	These methods are provided as a means to get the emitter's settings.
		//
		int						Get_Render_Mode (void) const		{ return RenderMode; }
		int						Get_Frame_Mode (void) const		{ return FrameMode; }
		float						Get_Particle_Size (void) const	{ return SizeKeyFrameValues[0]; }
		Vector3					Get_Acceleration (void) const		{ return Accel * 1000000.0F; }
		float						Get_Lifetime (void) const			{ return (float(MaxAge)) / 1000.0F; }
		float						Get_Future_Start_Time(void) const{ return (float(FutureStartTime)) / 1000.0f; }
		Vector3					Get_Start_Color (void) const		{ return ColorKeyFrameValues[0]; }
		float						Get_Start_Opacity (void) const	{ return AlphaKeyFrameValues[0]; }
		Vector3					Get_End_Color (void) const			{ return (NumColorKeyFrames > 1) ? ColorKeyFrameValues[NumColorKeyFrames - 1] : ColorKeyFrameValues[0]; }
		float						Get_End_Opacity (void) const		{ return (NumAlphaKeyFrames > 1) ? AlphaKeyFrameValues[NumAlphaKeyFrames - 1] : AlphaKeyFrameValues[0]; }
		TextureClass *			Get_Texture (void) const;
		void						Set_Texture (TextureClass *tex);
		float						Get_Fade_Time (void) const			{ return (NumColorKeyFrames > 1) ? (((float)ColorKeyFrameTimes[1]) / 1000.0f) : 0.0f; }
		ShaderClass				Get_Shader (void) const;

		//
		// Line rendering properties.  These functions will always return
		// a default value if line rendering is not enabled.
		//
		int						Get_Line_Texture_Mapping_Mode(void) const;
		int						Is_Merge_Intersections(void) const;
		int						Is_Freeze_Random(void) const;
		int						Is_Sorting_Disabled(void) const;
		int						Are_End_Caps_Enabled(void)	const;
		int						Get_Subdivision_Level(void) const;
		float						Get_Noise_Amplitude(void) const;
		float						Get_Merge_Abort_Factor(void) const;
		float						Get_Texture_Tile_Factor(void) const;
		Vector2					Get_UV_Offset_Rate(void) const;


		// This is a utility function only meant to be called by the particle emitter.
		unsigned int			Get_Buffer_Size(void) const		{ return MaxNum; }

		// Note: Caller IS RESPONSIBLE for freeing any memory allocated by these calls
		void						Get_Color_Key_Frames (ParticlePropertyStruct<Vector3>	&colors) const;
		void						Get_Opacity_Key_Frames (ParticlePropertyStruct<float>	&opacities) const;
		void						Get_Size_Key_Frames (ParticlePropertyStruct<float>	&sizes) const;
		void						Get_Rotation_Key_Frames (ParticlePropertyStruct<float> &rotations) const;
		void						Get_Frame_Key_Frames (ParticlePropertyStruct<float> &frames) const;
		void						Get_Blur_Time_Key_Frames (ParticlePropertyStruct<float> &blurtimes) const;
		float						Get_Initial_Orientation_Random (void) const { return InitialOrientationRandom; }

		void						Set_Current_GroupID(unsigned char grp) { CurrentGroupID = grp; }

		// Total Active Particle Buffer Count
		static unsigned int	Get_Total_Active_Count( void )	{ return TotalActiveCount; }

		// Global control of particle LOD.
		static void				Set_LOD_Max_Screen_Size(int lod_level,float max_screen_size);
		static float			Get_LOD_Max_Screen_Size(int lod_level);

	protected:

		virtual void			Update_Cached_Bounding_Volumes(void) const;

		// render the particle system as a collection of particles
		void						Render_Particles(RenderInfoClass & rinfo);

		// render the particle system as a line
		void						Render_Line(RenderInfoClass & rinfo);

		// render the particle system as a line group
		void						Render_Line_Group(RenderInfoClass & rinfo);

		// Update the kinematic particle state. This includes getting new
		// particles from the new particle queue, updating velocity/position
		// for any existing particles, killing old ones, and updating
		// LastUpdateTime.
		void Update_Kinematic_Particle_State(void);

		// Update the visual particle state. This includes updating color/size
		// for all existing particles. Only needs to happen at rendering time.
		void Update_Visual_Particle_State(void);

		// Update the bounding box. (Updates the particle state if it needs to).
		void Update_Bounding_Box(void);

		// Helper function for Render_Particles and Render_LineGroup
		void Generate_APT(ShareBufferClass <unsigned int> **apt,unsigned int &active_point_count);
		void Combine_Color_And_Alpha();

		// Get new particles from the emitter and write them into the circular
		// particle buffer, possibly overwriting older particles. Perform
		// partial-interval upddate on them as well.
		void Get_New_Particles(void);

		// Kill all remaining particles which will be above their maxage at the
		// end of this time interval.
		void Kill_Old_Particles(void);

		// Update all living non-new particles according to time elapsed since
		// last update.
		void Update_Non_New_Particles(unsigned int elapsed);

		// Seperate circular buffer used by the emitter to pass new particles.
		// It is implemented as an array, start and end indices and a count (to
		// differentiate between completely full and completely empty).
		NewParticleStruct *	NewParticleQueue;
		unsigned int			NewParticleQueueStart;
		unsigned int			NewParticleQueueEnd;
		int						NewParticleQueueCount;

		// State global to the entire particle buffer.
		int				RenderMode;		// rendering mode being used (settings found in w3d_file.h)
		int				FrameMode;		// frame mode (settings found in w3d_file.h - 1x1..16x16)
		Vector3			Accel;			// Worldspace acceleration per ms^2.
		bool				HasAccel;		// Is the acceleration non-zero?
		unsigned int	MaxAge;			// Maximum age in milliseconds.
		unsigned int	FutureStartTime;// Future start time in milliseconds
		unsigned int	LastUpdateTime;// Time at last update.
		bool				IsEmitterDead;
		float				MaxSize;			// Used for BBox calculations

		// Circular buffer implementation. This is actually 2 sequential
		// circular buffers: one for non-new particles and one for new
		// particles (the distinction is needed because the two types of
		// particles are updated differently).
		// Besides the head/tail indices, a count is used for each buffer to
		// distinguish between full and empty.
		unsigned int 	MaxNum;		// Maximum number of particles.
		unsigned int	Start;		// Start of existing (non-new) particles.
		unsigned int	End;			// End of existing (non-new) particles.
		unsigned int	NewEnd;		// End of new particles.
		int				NonNewNum;  // Non-new entry count (to know when empty).
		int				NewNum;     // New entry count (to know when empty).

		// Worldspace-aligned bounding box:
		AABoxClass		BoundingBox;
		bool				BoundingBoxDirty;

		// At least one keyframe must exist for each property (time 0).
		// If a randomizer is zero and there are no additional keyframes for
		// that property (or the keyframes are all equal), all the arrays for
		// that property are NULL (since they will never be used), except for
		// the Values array which will have one entry (the constant value).
		// Note that the rotation and orientation properties are different -
		// only orientation is used in rendering. The rotation data is only
		// used to compute the orientations. So the condition is different -
		// if rotation and orientation randomizers, and all rotation keyframes
		// are all zero, then all of the arrays will be NULL (including the
		// Values array).
		unsigned int	NumColorKeyFrames;
		unsigned int *	ColorKeyFrameTimes;		// 0th entry is always 0
		Vector3 *		ColorKeyFrameValues;
		Vector3 *		ColorKeyFrameDeltas;
		unsigned int	NumAlphaKeyFrames;
		unsigned int *	AlphaKeyFrameTimes;		// 0th entry is always 0
		float *			AlphaKeyFrameValues;
		float *			AlphaKeyFrameDeltas;
		unsigned int	NumSizeKeyFrames;
		unsigned int *	SizeKeyFrameTimes;		// 0th entry is always 0
		float *			SizeKeyFrameValues;
		float *			SizeKeyFrameDeltas;
		unsigned int	NumRotationKeyFrames;
		unsigned int * RotationKeyFrameTimes;	// 0th entry is always 0
		float *			RotationKeyFrameValues;	// In rotations per millisecond
		float *			HalfRotationKeyFrameDeltas; // (* 0.5f)
		float *			OrientationKeyFrameValues;	// Rotation preintegrated to keyframe times
		unsigned int	NumFrameKeyFrames;
		unsigned int * FrameKeyFrameTimes;		// 0th entry is always 0
		float *			FrameKeyFrameValues;
		float *			FrameKeyFrameDeltas;
		unsigned int	NumBlurTimeKeyFrames;
		unsigned int * BlurTimeKeyFrameTimes;		// 0th entry is always 0
		float *			BlurTimeKeyFrameValues;
		float *			BlurTimeKeyFrameDeltas;

		Vector4			DefaultTailDiffuse;	// For line group mode, when all the tails are the same color

		// These tables are indexed by the array position in the particle buffer.
		// The table size is either the smallest power of two equal or larger
		// than the buffer size, or MAX_RANDOM_ENTRIES (defined in the .cpp
		// file - MUST be a power of two), whichever is smaller. Note that if a
		// randomizer is zero, the table will have one entry (containing zero),
		// which is why each property has its own NumXXXRandomEntries variable.
		// If a randomizer is zero and the property has no keyframes, the table
		// will be NULL since it will never be used (property is constant)).
		unsigned int	NumRandomColorEntriesMinus1;			// 2^n - 1 so can be used as a mask also
		Vector3 *		RandomColorEntries;
		unsigned int	NumRandomAlphaEntriesMinus1;			// 2^n - 1 so can be used as a mask also
		float *			RandomAlphaEntries;
		unsigned int	NumRandomSizeEntriesMinus1;			// 2^n - 1 so can be used as a mask also
		float *			RandomSizeEntries;
		unsigned int	NumRandomRotationEntriesMinus1;		// 2^n - 1 so can be used as a mask also
		float *			RandomRotationEntries;
		unsigned int	NumRandomOrientationEntriesMinus1;	// 2^n - 1 so can be used as a mask also
		float *			RandomOrientationEntries;
		unsigned int	NumRandomFrameEntriesMinus1;			// 2^n - 1 so can be used as a mask also
		float *			RandomFrameEntries;
		unsigned int	NumRandomBlurTimeEntriesMinus1;		// 2^n - 1 so can be used as a mask also
		float *			RandomBlurTimeEntries;

		Vector3			ColorRandom;
		float				OpacityRandom;
		float				SizeRandom;
		float				RotationRandom;
		float				FrameRandom;
		float				BlurTimeRandom;
		float				InitialOrientationRandom;

		// This object implements particle rendering
		PointGroupClass *	PointGroup;

		// This object implements line rendering
		SegLineRendererClass	* LineRenderer;

		// This object implements line group rendering
		LineGroupClass * LineGroup;

		// These are shared with the point group. The position, color and alpha
		// arrays serve double duty: they are used to store and update particle
		// state and also to pass point information to the point group. The
		// active point table is used to communicate to the point group which
		// points are active (it is only used if all are not active)..
		ShareBufferClass<Vector3> *	Position[2];	// Only [0] used unless pingpong enabled
		ShareBufferClass<Vector4> *	Diffuse;			// passed into point group
		ShareBufferClass<Vector3> *	Color;
		ShareBufferClass<float> *		Alpha;
		ShareBufferClass<float> *		Size;
		ShareBufferClass<uint8> *		Frame;
		ShareBufferClass<float> *		UCoord;			// Only used for line groups, uses Frame keyframes
		ShareBufferClass<Vector3> *	TailPosition;	// Only used for line groups
		ShareBufferClass<Vector4> *	TailDiffuse;	// Only used for line groups
		ShareBufferClass<uint8> *		Orientation;
		ShareBufferClass<unsigned int> *	APT;
		ShareBufferClass<unsigned char> * GroupID;	// Only used for lines

		// Do we keep two ping-pong position buffers (for collision and possibly other effects
		// which need the previous frames position as well as this frames)
		bool									PingPongPosition;

		// Additional per-particle state:
		Vector3 *							Velocity;	// World units per millisecond.
		unsigned int *						TimeStamp;	// Millisecond time at creation.

		// This pointer is used for synchronization - the emitter is called to
		// add new particles at the start of the buffers render function - to
		// prevent behavior which is dependent on the relative time order of
		// the emitter and buffer in the rendering list.
		ParticleEmitterClass *			Emitter;

		// These are used for decimating particles for LOD purposes. The
		// threshold is compared vs. an array filled with a random permutation
		// of the numbers 0 to 15, and any particle whose entry (modulo 16) is
		// less than the threshold is not rendered. So if DecimationThreshold
		// is 0 (the minimum value), all particles are rendered - if it is 16
		// (the maximum value) none are rendered.
		unsigned int						DecimationThreshold;
		static const unsigned int		PermutationArray[16];

		// LOD values
		unsigned int						LodCount;
		float									Cost[17];	// Cost array needs one entry for each LOD level
		float									Value[18];	// Value array needs one more entry than # of LODs
		float									LodBias;

		// Projected area, used for LOD purposes
		float									ProjectedArea;

		// Total Active Particle Buffer Count
		static unsigned int				TotalActiveCount;

		// Static array of screen-size clamps for the 17 possible LOD levels a
		// particle buffer can have. We can change these from being global to
		// being per-buffer later if we wish. Default is NO_MAX_SCREEN_SIZE.
		static float						LODMaxScreenSizes[17];

		enum TailDiffuseTypeEnum {
			BLACK,
			WHITE,
			SAME_AS_HEAD,
			SAME_AS_HEAD_ALPHA_ZERO
		};

		// Determine based on shader and texture
		// what the tail color should be
		TailDiffuseTypeEnum Determine_Tail_Diffuse();

		unsigned char						CurrentGroupID;
};

#endif // PART_BUF_H


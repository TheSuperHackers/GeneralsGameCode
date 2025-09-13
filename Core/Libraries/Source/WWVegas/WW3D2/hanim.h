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

/* $Header: /Commando/Code/ww3d2/hanim.h 3     12/13/01 7:01p Patrick $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/hanim.h                                $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 12/13/01 6:54p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef HANIM_H
#define HANIM_H

#include "always.h"
#include "quat.h"
#include "refcount.h"
#include "w3d_file.h"
#include "hash.h"
#include "mempool.h"
#include <refcount.h>
#include <SLIST.H>
#include <Vector.H>

struct NodeMotionStruct;
class MotionChannelClass;
class BitChannelClass;
class HTreeClass;
class ChunkLoadClass;
class ChunkSaveClass;
class HTreeClass;



#define EMBEDDED_SOUND_BONE_INDEX_NOT_SET -1
/**********************************************************************************

	HAnimClass

	This is the base class for all animation formats used in W3D.  It
	contains the virtual interface that all animations must support.

**********************************************************************************/
class HAnimClass : public RefCountClass, public	HashableClass
{
public:
	enum
	{
		CLASSID_UNKNOWNANIM	= 0xFFFFFFFF,
		CLASSID_HRAWANIM		= 0,
		CLASSID_LASTANIM		= 0x0000FFFF
	};

	HAnimClass(void)	:
		EmbeddedSoundBoneIndex (EMBEDDED_SOUND_BONE_INDEX_NOT_SET)	{ }
	virtual ~HAnimClass(void)		{ }

	virtual const char *		Get_Name(void) const = 0;
	virtual const char *		Get_HName(void) const = 0;

	virtual const char *		Get_Key( void )						{ return Get_Name(); }

	virtual int					Get_Num_Frames(void) = 0;
	virtual float				Get_Frame_Rate() = 0;
	virtual float				Get_Total_Time() = 0;

	// Jani: Changed to pass in reference of destination to avoid copying
	virtual void				Get_Translation(Vector3& translation, int pividx,float frame) const = 0;
	virtual void				Get_Orientation(Quaternion& orientation, int pividx,float frame) const = 0;
	virtual void				Get_Transform(Matrix3D&, int pividx, float frame) const = 0;
	virtual bool				Get_Visibility(int pividx,float frame) = 0;

	virtual int					Get_Num_Pivots(void) const = 0;
	virtual bool				Is_Node_Motion_Present(int pividx) = 0;

	// Methods that test the presence of a certain motion channel.
	virtual bool				Has_X_Translation (int pividx)	{ return true; }
	virtual bool				Has_Y_Translation (int pividx)	{ return true; }
	virtual bool				Has_Z_Translation (int pividx)	{ return true; }
	virtual bool				Has_Rotation (int pividx)			{ return true; }
	virtual bool				Has_Visibility (int pividx)		{ return true; }
	virtual int					Class_ID(void)	const															{ return CLASSID_UNKNOWNANIM; }

	// Animated sound-triggering support
	virtual bool				Has_Embedded_Sounds (void) const			{ if (EmbeddedSoundBoneIndex < 0) return false; return true;}
	virtual void				Set_Embedded_Sound_Bone_Index (int bone)	{ EmbeddedSoundBoneIndex = bone; }
	virtual int					Get_Embedded_Sound_Bone_Index() {return EmbeddedSoundBoneIndex;}

protected:
	int EmbeddedSoundBoneIndex;
};


/*
** The PivotMapClass is used by the HAnimComboDataClass (sometimes) to keep track of animation
** weights per-pivot point.
*/
class NamedPivotMapClass;

class PivotMapClass : public DynamicVectorClass<float>, public RefCountClass
{
public:
	virtual NamedPivotMapClass * As_Named_Pivot_Map() { return 0; }
};


/*
**	The NamedPivotMapClass allows us to create HAnimComboDataClass objects with pivot maps without
**	having to have the HAnim available. Later, when an HAnim is retrieved from the asset manager,
** the pivot map can be updated to produce the correct map.
*/
class NamedPivotMapClass : public PivotMapClass
{
public:
	~NamedPivotMapClass(void);

	virtual NamedPivotMapClass * As_Named_Pivot_Map() { return this; }

	// add a name & weight to the arrays
	void Add(const char *Name, float Weight);

	// configure the base pivot map using the specified tree
	void Update_Pivot_Map(const HTreeClass *Tree);

private:

	// This info is packaged into a struct to minimize DynamicVectorClass overhead
	struct WeightInfoStruct {
		WeightInfoStruct() : Name(NULL) {}
		~WeightInfoStruct() { delete [] Name; }

		char *Name;
		float Weight;

		// operators required for the DynamicVectorClass
		WeightInfoStruct & operator = (WeightInfoStruct const &that);
		bool operator == (WeightInfoStruct const &that) const { return &that == this; }
		bool operator != (WeightInfoStruct const &that) const { return &that != this; }
	};

	DynamicVectorClass<WeightInfoStruct>	WeightInfo;
};


/*
**	The HAnimComboDataClass is used by the new HAnimComboClass to allow for a mix of shared/unshared data
**	which will allow us to have the anim combo refer to data whereever we wish to put it.
*/
class HAnimComboDataClass : public AutoPoolClass<HAnimComboDataClass,256> {
	public:
		HAnimComboDataClass(bool shared = false);
		HAnimComboDataClass(const HAnimComboDataClass &);
		~HAnimComboDataClass(void);

		void Copy(const HAnimComboDataClass *);

		void Clear(void);
		void Set_HAnim(HAnimClass *motion);
		void Give_HAnim(HAnimClass *motion) { if(HAnim) HAnim->Release_Ref(); HAnim = motion; }	// used for giving this object the reference

		void Set_Frame(float frame)		{ PrevFrame = Frame; Frame = frame; }
		void Set_Prev_Frame(float frame)	{ PrevFrame = frame; }
		void Set_Weight(float weight)		{ Weight = weight; }
		void Set_Pivot_Map(PivotMapClass *map);


		HAnimClass * Peek_HAnim(void)				const { return HAnim; }	// note: does not add reference
		HAnimClass * Get_HAnim(void)				const { if(HAnim) HAnim->Add_Ref(); return HAnim; }	// note: does not add reference
		float Get_Frame(void)						const { return Frame; }
		float Get_Prev_Frame(void)					const { return PrevFrame; }
		float Get_Weight(void)						const { return Weight; }
		PivotMapClass * Peek_Pivot_Map(void)	const { return PivotMap; }
		PivotMapClass * Get_Pivot_Map(void)		const { if(PivotMap) PivotMap->Add_Ref(); return PivotMap; }
		bool Is_Shared(void)							const { return Shared; }

		void Build_Active_Pivot_Map(void);

	private:

		HAnimClass *HAnim;
		float Frame;
		float PrevFrame;
		float Weight;
		PivotMapClass * PivotMap;
		bool Shared;			// this is set to false when the HAnimCombo allocates it
};

/*
** defines a combination of animations for blending/mixing
*/
class HAnimComboClass {

public:

	HAnimComboClass(void);
	HAnimComboClass( int num_animations ); // allocates specified number of channels and sets then all to not-shared
	~HAnimComboClass(void);

	void	Clear( void );		// zeros all data

	void	Reset( void );		// empties the dynamic vector

 	bool	Normalize_Weights(void);	// Normalizes all weights (returns true if succeeded)
	int	Get_Num_Anims( void ) { return HAnimComboData.Count(); }

	void	Set_Motion( int indx, HAnimClass *motion );
	HAnimClass *Get_Motion( int indx );
	HAnimClass *Peek_Motion( int indx );

	void	Set_Frame( int indx, float frame );
	void	Set_Prev_Frame( int indx, float frame );
	float	Get_Frame( int indx );
	float	Get_Prev_Frame( int indx );

	void	Set_Weight( int indx, float weight );
	float	Get_Weight( int indx );

	void	Set_Pivot_Weight_Map( int indx, PivotMapClass * map );
	PivotMapClass	* Get_Pivot_Weight_Map( int indx );
	PivotMapClass	* Peek_Pivot_Weight_Map( int indx );


	// add an entry to the dynamic vector
	void Append_Anim_Combo_Data(HAnimComboDataClass * AnimComboData);

	// remove an entry from the vector
	void Remove_Anim_Combo_Data(HAnimComboDataClass * AnimComboData);

	// retrieve a specific combo data
	HAnimComboDataClass * Peek_Anim_Combo_Data(int index) { return HAnimComboData[index]; }

protected:

	DynamicVectorClass<HAnimComboDataClass *> HAnimComboData;

};

#endif

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

/* $Header: /Commando/Code/Tools/max2w3d/gamemtl.h 38    8/22/01 7:56a Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D engine                                       *
 *                                                                                             *
 *                    File Name : GAMEMTL.H                                                    *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 06/26/97                                                     *
 *                                                                                             *
 *                  Last Update : June 26, 1997 [GH]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <max.h>
#include "w3dmtl.h"
#include "w3d_file.h"


// GameMtl flags values
#define GAMEMTL_CONVERTED_TO_NOLOD		(1<<0)	// this material has been converted to use the NO_LOD flag on textures

#define GAMEMTL_DISPLACEMENT_ROLLUP_OPEN (1<<25)
#define GAMEMTL_SURFACE_ROLLUP_OPEN		(1<<26)
#define GAMEMTL_PASSCOUNT_ROLLUP_OPEN	(1<<27)
#define GAMEMTL_PASS0_ROLLUP_OPEN		(1<<28)
#define GAMEMTL_PASS1_ROLLUP_OPEN		(1<<29)
#define GAMEMTL_PASS2_ROLLUP_OPEN		(1<<30)
#define GAMEMTL_PASS3_ROLLUP_OPEN		(1<<31)

#define GAMEMTL_ROLLUP_FLAGS				(	GAMEMTL_SURFACE_ROLLUP_OPEN | \
														GAMEMTL_DISPLACEMENT_ROLLUP_OPEN | \
														GAMEMTL_PASSCOUNT_ROLLUP_OPEN | \
														GAMEMTL_PASS0_ROLLUP_OPEN | \
														GAMEMTL_PASS1_ROLLUP_OPEN | \
														GAMEMTL_PASS2_ROLLUP_OPEN | \
														GAMEMTL_PASS3_ROLLUP_OPEN )

#define GAMEMTL_ID_PARTA		0x29397211
#define GAMEMTL_ID_PARTB		0x28c016c2

///////////////////////////////////////////////////////////////////////////
//
// Mapping types
//
///////////////////////////////////////////////////////////////////////////
#define GAMEMTL_MAPPING_UV							0
#define GAMEMTL_MAPPING_ENV						1
#define GAMEMTL_MAPPING_CHEAP_ENV				2
#define GAMEMTL_MAPPING_SCREEN               3
#define GAMEMTL_MAPPING_LINEAR_OFFSET        4
#define GAMEMTL_MAPPING_SILHOUETTE		      5
#define	GAMEMTL_MAPPING_SCALE					6
#define	GAMEMTL_MAPPING_GRID						7
#define	GAMEMTL_MAPPING_ROTATE					8
#define	GAMEMTL_MAPPING_SINE_LINEAR_OFFSET	9
#define	GAMEMTL_MAPPING_STEP_LINEAR_OFFSET	10
#define	GAMEMTL_MAPPING_ZIGZAG_LINEAR_OFFSET 11
#define	GAMEMTL_MAPPING_WS_CLASSIC_ENV		12
#define	GAMEMTL_MAPPING_WS_ENVIRONMENT		13
#define	GAMEMTL_MAPPING_GRID_CLASSIC_ENV		14
#define	GAMEMTL_MAPPING_GRID_ENVIRONMENT		15
#define	GAMEMTL_MAPPING_RANDOM					16
#define	GAMEMTL_MAPPING_EDGE						17
#define	GAMEMTL_MAPPING_BUMPENV					18


///////////////////////////////////////////////////////////////////////////
//
// PSX Translucency Type
//
///////////////////////////////////////////////////////////////////////////
#define GAMEMTL_PSX_TRANS_NONE					0
#define GAMEMTL_PSX_TRANS_100						1
#define GAMEMTL_PSX_TRANS_50						2
#define GAMEMTL_PSX_TRANS_25						3
#define GAMEMTL_PSX_TRANS_MINUS_100				4

class GameMtlDlg;
class GameMapsClass;

extern Class_ID GameMaterialClassID;
extern ClassDesc * Get_Game_Material_Desc();

// MLL
// For Playstation 2 materials.
extern Class_ID PS2GameMaterialClassID;
extern ClassDesc * Get_PS2_Game_Material_Desc();

extern Class_ID PCToPS2MaterialClassID;
extern ClassDesc * Get_PS2_Material_Conversion();

///////////////////////////////////////////////////////////////////////////
//
//		Game Material
//		This is a plugin-material which attempts to emulate the material
//		used in our in-game 3D engine.  It has varying degrees of success
//		at making Max render things like our game but it gives us the
//		ability to have control over all parameters in our material.
//
///////////////////////////////////////////////////////////////////////////
class GameMtl: public Mtl
{

public:

		enum ShaderTypeEnum
		{
			STE_PC_SHADER,
			STE_PS2_SHADER,
		};

		GameMtl(BOOL loading = FALSE);
		~GameMtl(void);

		Class_ID				ClassID();
		SClass_ID			SuperClassID();

		// From Animatable
		void					GetClassName(TSTR& s);
		void					DeleteThis()													{ delete this; }
		int					NumSubs();
		Animatable *		SubAnim(int i);
		TSTR					SubAnimName(int i);

		// References
		int					NumRefs()														{ return REF_COUNT; }
		RefTargetHandle	Clone(RemapDir &remap = NoRemap());
		RefResult			NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
		void					SetReference(int i, RefTargetHandle rtarg);
		RefTargetHandle	GetReference(int i);

		// From MtlBase and Mtl
		void					SetAmbient(Color c, TimeValue t)							{ Set_Ambient(0,t,c); }
		void					SetDiffuse(Color c, TimeValue t)							{ Set_Diffuse(0,t,c); }
		void					SetSpecular(Color c, TimeValue t)						{ Set_Specular(0,t,c); }
		void					SetShininess(float v, TimeValue t)						{ Set_Shininess(0,t,v); }
		Color					GetAmbient(int mtlNum=0, BOOL backFace=FALSE)		{ return Get_Ambient(0,0); }
		Color					GetDiffuse(int mtlNum=0, BOOL backFace=FALSE)		{ return Get_Diffuse(0,0); }
		Color					GetSpecular(int mtlNum=0, BOOL backFace=FALSE)		{ return Get_Specular(0,0); }
		float					GetXParency(int mtlNum=0, BOOL backFace=FALSE)		{ return 0.0f; }
		float					GetShininess(int mtlNum=0, BOOL backFace=FALSE)		{ return Get_Shininess(0,0); }
		float					GetShinStr(int mtlNum=0, BOOL backFace=FALSE)		{ return 1.0f; }
		void					Reset(void);
		void					Update(TimeValue t, Interval& validr);
		Interval				Validity(TimeValue t);

		int					NumSubTexmaps(void);
		void					SetSubTexmap(int i, Texmap * m);
		Texmap *				GetSubTexmap(int i);

		float					EvalDisplacement(ShadeContext& sc);
		Interval				DisplacementValidity(TimeValue t);

		// Rendering
		void					Shade(ShadeContext& sc);
		ULONG					Requirements(int subMtlNum);
		//ULONG					LocalRequirements(int subMtlNum);

		// Material editor
		void					SetParamDlg(GameMtlDlg * pd)								{ MaterialDialog = pd; }
		ParamDlg*			CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);

		// IO stuff
		IOResult				Save(ISave* iSave);
		IOResult				Load(ILoad* iLoad);

		// Accessors...
		void					Set_Flag(ULONG f, ULONG val)								{ if (val) Flags|=f;	else Flags &= ~f; }
		int					Get_Flag(ULONG f)		 										{ return ((Flags&f) ? 1 : 0); }

		void					Set_Surface_Type(unsigned int type)						{ SurfaceType = type; }
		unsigned int		Get_Surface_Type(void) const								{ return SurfaceType; }

		void					Set_Sort_Level(int level)									{ SortLevel = level; }
		int					Get_Sort_Level(void) const									{ return SortLevel; }

		void					Set_Pass_Count(int passcount);
		int					Get_Pass_Count(void);

		IParamBlock *		Get_Parameter_Block(int pass);
		int					Get_Current_Page(int pass)									{ return CurPage[pass]; }
		Color					Get_Ambient(int pass,TimeValue t);
		Color					Get_Diffuse(int pass,TimeValue t);
		Color					Get_Specular(int pass,TimeValue t);
		Color					Get_Emissive(int pass,TimeValue t);
		float					Get_Shininess(int pass,TimeValue t);
		float					Get_Opacity(int pass,TimeValue t);
		float					Get_Translucency(int pass,TimeValue t);
		int					Get_Copy_Specular_To_Diffuse(int pass);
		int					Get_Mapping_Type(int pass, int stage=0);
		int					Get_PSX_Translucency(int pass);
		int					Get_PSX_Lighting(int pass);

		int					Get_Depth_Compare(int pass);
		int					Get_Depth_Mask(int pass);
		int					Get_Alpha_Test(int pass);
		int					Get_Dest_Blend(int pass);
		int					Get_Pri_Gradient(int pass);
		int					Get_Sec_Gradient(int pass);
		int					Get_Src_Blend(int pass);
		int					Get_Detail_Color_Func(int pass);
		int					Get_Detail_Alpha_Func(int pass);
		int					Get_PS2_Shader_Param_A(int pass);
		int					Get_PS2_Shader_Param_B(int pass);
		int					Get_PS2_Shader_Param_C(int pass);
		int					Get_PS2_Shader_Param_D(int pass);

		int					Get_Texture_Enable(int pass,int stage);
		int					Get_Texture_Publish(int pass,int stage);
		int					Get_Texture_Resize(int pass,int stage);		// NOTE: obsolete, replaced by Get_Texture_No_LOD
		int					Get_Texture_No_Mipmap(int pass,int stage);	// NOTE: obsolete, replaced by Get_Texture_No_LOD
		int					Get_Texture_Clamp_U(int pass,int stage);
		int					Get_Texture_Clamp_V(int pass,int stage);
		int					Get_Texture_No_LOD(int pass,int stage);
		int					Get_Texture_Alpha_Bitmap(int pass,int stage);
		int					Get_Texture_Hint(int pass,int stage);
		int					Get_Texture_Display(int pass,int stage);
		float					Get_Texture_Frame_Rate(int pass,int stage);
		int					Get_Texture_Frame_Count(int pass,int stage);
		int					Get_Texture_Anim_Type(int pass,int stage);
		Texmap *				Get_Texture(int pass,int stage);
		Texmap *				Get_Displacement_Map(void) const								{ return DisplacementMap; }
		float					Get_Displacement_Amount(void) const							{ return DisplacementAmt; }
		int					Get_Displacement_Map_Index(void) const;
		int					Get_Map_Channel(int pass,int stage);

		void					Set_Current_Page(int pass,int page)							{ CurPage[pass] = page; }
		void					Set_Ambient(int pass,TimeValue t,Color color);
		void					Set_Diffuse(int pass,TimeValue t,Color color);
		void					Set_Specular(int pass,TimeValue t,Color color);
		void					Set_Emissive(int pass,TimeValue t,Color color);
		void					Set_Shininess(int pass,TimeValue t,float val);
		void					Set_Opacity(int pass,TimeValue t,float val);
		void					Set_Translucency(int pass,TimeValue t,float val);
		void					Set_Copy_Specular_To_Diffuse(int pass,bool val);
		void					Set_Mapping_Type(int pass,int stage,int val);
		void					Set_PSX_Translucency(int pass,int val);
		void					Set_PSX_Lighting(int pass,bool val);

		void					Set_Depth_Compare(int pass,int val);
		void					Set_Depth_Mask(int pass,int val);
		void					Set_Alpha_Test(int pass,int val);
		void					Set_Dest_Blend(int pass,int val);
		void					Set_Pri_Gradient(int pass,int val);
		void					Set_Sec_Gradient(int pass,int val);
		void					Set_Src_Blend(int pass,int val);
		void					Set_Detail_Color_Func(int pass,int val);
		void					Set_Detail_Alpha_Func(int pass,int val);
		void					Set_PS2_Shader_Param_A(int pass,int val);
		void					Set_PS2_Shader_Param_B(int pass,int val);
		void					Set_PS2_Shader_Param_C(int pass,int val);
		void					Set_PS2_Shader_Param_D(int pass,int val);

		void					Set_Texture_Enable(int pass,int stage,bool val);
		void					Set_Texture_Publish(int pass,int stage,bool val);
		void					Set_Texture_Resize(int pass,int stage,bool val);		// NOTE: obsolete: replaced by Set_Texture_No_LOD
		void					Set_Texture_No_Mipmap(int pass,int stage,bool val);	// NOTE: obsolete: replaced by Set_Texture_No_LOD
		void					Set_Texture_Clamp_U(int pass,int stage,bool val);
		void					Set_Texture_Clamp_V(int pass,int stage,bool val);
		void					Set_Texture_No_LOD(int pass,int stage,bool val);
		void					Set_Texture_Alpha_Bitmap(int pass,int stage,bool val);
		void					Set_Texture_Hint(int pass,int stage,int val);
		void					Set_Texture_Display(int pass,int stage,bool val);
		void					Set_Texture_Frame_Rate(int pass,int stage,float val);
		void					Set_Texture_Frame_Count(int pass,int stage,int val);
		void					Set_Texture_Anim_Type(int pass,int stage,int val);
		void					Set_Texture(int pass,int stage,Texmap * tex);
		void					Set_Displacement_Amount(float amount)					{ DisplacementAmt = amount; Notify_Changed (); }

		void					Set_Map_Channel(int pass,int stage,int channel);

		void					Notify_Changed(void);

		// This returns the mapping args string buffer for that pass after
		// assuring that it is at least of length 'len'.
		char *				Get_Mapping_Arg_Buffer(int pass, int stage=0, unsigned int len = 0U);

		int					pass_stage_to_texmap_index(int pass,int stage);
		void					texmap_index_to_pass_stage(int index,int * set_pass,int * set_stage);

		// Set and get the type of shader, PC or PS2.
		// MLL
		int					Get_Shader_Type()						{return (ShaderType);}
		void					Set_Shader_Type(int shadertype)	{ShaderType = shadertype;}

		// Approximate the PS2 shader on the PC.
		int					Compute_PC_Shader_From_PS2_Shader(int pass);
		int					Compute_PS2_Shader_From_PC_Shader(int pass);

		// IML: Get/set substitute material.
		Mtl*					Substitute_Material()				{return (SubstituteMaterial);}
		void					Set_Substitute_Material (Mtl *m) {SubstituteMaterial = m;}

private:

		int					texture_ref_index(int pass,int stage)			{ return REF_TEXTURE + pass*W3dMaterialClass::MAX_STAGES + stage; }
		int					pass_ref_index(int pass)							{ return REF_PASS_PARAMETERS + pass; }

		void					update_viewport_display();

		// Do the shade functions specific to the Playstation 2.
		void					ps2_shade(ShadeContext& sc);


		unsigned int		SurfaceType;
		int					SortLevel;

		Interval				Ivalid;				// Valid interval
		GameMtlDlg *		MaterialDialog;	// Dialog
		ULONG					Flags;				// Flags
		int					RollScroll;			// Rollup scroll position
		int					CurPage[W3dMaterialClass::MAX_PASSES];		// which page was last open for each pass

		GameMapsClass *	Maps;					// ref 0 (obsolete...)
		IParamBlock *		MainParameterBlock;
		IParamBlock *		PassParameterBlock[W3dMaterialClass::MAX_PASSES];
		Texmap *				Texture[W3dMaterialClass::MAX_PASSES][W3dMaterialClass::MAX_STAGES];
		char *				MapperArg[W3dMaterialClass::MAX_PASSES][W3dMaterialClass::MAX_STAGES];
		unsigned int		MapperArgLen[W3dMaterialClass::MAX_PASSES][W3dMaterialClass::MAX_STAGES];
		Texmap *				DisplacementMap;
		float					DisplacementAmt;

		// MLL
		int					ShaderType;
		enum
		{
			REF_MAPS = 0,					// obsolete, gamemaps object
			REF_MAIN	= 1,					// main parameter block is ref 1
			REF_PASS_PARAMETERS = 2,	// pass parameter blocks are refs 2,3,4,5
			REF_TEXTURE = 6,				// textures are refs 6,7,8,9,10,11,12,13,14 (14 is the displacement map)
			REF_COUNT = 15
		};

		Mtl *SubstituteMaterial;		// IML: Temporary material used during game to standard material conversion and vice-versa.

		friend class GameMtlDlg;
		friend class GameMtlPostLoad;
};

Mtl * CreateGameMtl();

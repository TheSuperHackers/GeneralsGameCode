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

/* $Header: /Commando/Code/Tools/max2w3d/GameMtl.cpp 52    8/10/01 2:18p Ian_l $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D engine                                       * 
 *                                                                                             * 
 *                    File Name : GAMEMTL.CPP                                                  * 
 *                                                                                             * 
 *                   Programmer : Greg Hjelstrom                                               * 
 *                                                                                             * 
 *                   Start Date : 06/26/97                                                     * 
 *                                                                                             * 
 *                  Last Update : 10/26/1999997 [GH]                                           * 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   GameMtl::GameMtl -- constructor                                                           *
 *   GameMtl::~GameMtl -- destructor                                                           *
 *   GameMtl::ClassID -- returns the max ClassID of the material plugin                        *
 *   GameMtl::SuperClassID -- returns the super class ID                                       *
 *   GameMtl::GetClassName -- returns the name of this plugin clas                             *
 *   GameMtl::NumSubs -- returns the number of sub animations                                  *
 *   GameMtl::SubAnimName -- returns the name of the i'th sub animation                        *
 *   GameMtl::SubAnim -- returns the i'th sub-anim                                             *
 *   GameMtl::Clone -- clones this material                                                    *
 *   GameMtl::NotifyRefChanged -- NotifyRefChanged handler                                     *
 *   GameMtl::SetReference -- set the i'th reference                                           *
 *   GameMtl::GetReference -- returnst the i'th reference                                      *
 *   GameMtl::NumSubTexmaps -- returns the number of texture maps in this material             *
 *   GameMtl::SetSubTexmap -- set the i'th texture map                                         *
 *   GameMtl::GetSubTexmap -- returns the i'th texture map                                     *
 *   GameMtl::CreateParamDlg -- creates the material editor dialog box                         *
 *   GameMtl::Notify_Changed -- someone has changed this material                              *
 *   GameMtl::Reset -- reset this material to default settings                                 *
 *   GameMtl::Update -- time has changed                                                       *
 *   GameMtl::Validity -- return the validity of the material at time t                        *
 *   GameMtl::Requirements -- what requirements does this material have?                       *
 *   GameMtl::Load -- loading from a MAX file                                                  *
 *   GameMtl::Save -- Saving into a MAX file                                                   *
 *   GameMtl::Shade -- evaluate the material for the renderer.                                 *
 *   GameMtl::ps2_shade -- Emulate the PS2 shader.                                             *
 *   GameMtl::Compute_PC_Shader_From_PS2_Shader -- Determine if a PC shader can be created.    *
 *   GameMtl::Compute_PS2_Shader_From_PC_Shader -- Change a W3D material to a PS2 W3D material.*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gamemtl.h"
#include <max.h>
#include <gport.h>
#include <hsv.h>
#include "GameMtlDlg.h"
#include "dllmain.h"
#include "resource.h"
#include "util.h"
#include "meshsave.h"
#include "gamemaps.h"


/*****************************************************************
*
*		GameMtl Class Descriptor
*
*****************************************************************/
Class_ID GameMaterialClassID(GAMEMTL_ID_PARTA, GAMEMTL_ID_PARTB);

class GameMaterialClassDesc:public ClassDesc {

public:
	int				IsPublic()					{ return 1; }
	void *			Create(BOOL loading)		{ return new GameMtl(loading); }
	const TCHAR *	ClassName()					{ return Get_String(IDS_GAMEMTL); }
	SClass_ID		SuperClassID()				{ return MATERIAL_CLASS_ID; }
	Class_ID 		ClassID()					{ return GameMaterialClassID; }
	const TCHAR* 	Category()					{ return _T("");  }
};

static GameMaterialClassDesc _GameMaterialCD;

ClassDesc * Get_Game_Material_Desc() { return &_GameMaterialCD;  }


/******************************************************************************
*
*  PostLoadCallback for GameMtl
*
******************************************************************************/
class GameMtlPostLoad : public PostLoadCallback 
{
public:
	GameMtl *m;
	
	GameMtlPostLoad(GameMtl *b)		{ m=b; IsOld = false;}
	void proc(ILoad *iload);

	/*
	** Data from the previous version of GameMtl
	*/
	bool					IsOld;
	ULONG					Attributes;		
	Color					Diffuse;
	Color					Specular;
	Color					AmbientCoeff;
	Color					DiffuseCoeff;
	Color					SpecularCoeff;
	Color					EmissiveCoeff;
	float					FogCoeff;
	int					DCTFrames;
	int					DITFrames;
	int					SCTFrames;
	int					SITFrames;
	float					DCTFrameRate;
	float					DITFrameRate;
	float					SCTFrameRate;
	float					SITFrameRate;
	int					DCTMappingType;
	int					DITMappingType;
	int					SCTMappingType;
	int					SITMappingType;
	float					Opacity;
	float					Translucency;
	float					Shininess;
};



/******************************************************************************
*
*  GameMtl
*
*	Notes:
*  Prior to Nov, 1998 I was storing everything about the material in custom
*  chunks.  As part of the upgrade to surrender 1.4x, I'm going to use
*  a parameter block for each pass (combination of shader and vertex material)
*
******************************************************************************/
#define MTL_HDR_CHUNK							0x4000

/*
** Custom Chunk IDs
*/
#define GAMEMTL_FLAGS_CHUNK					0x0000		 
#define GAMEMTL_GAMEFLAGS_CHUNK				0x0001		// OBSOLETE!
#define GAMEMTL_COLORS_CHUNK					0x0002		// OBSOLETE!

#define GAMEMTL_TEXTURE_FRAMES_CHUNK		0x0003
#define GAMEMTL_ENVMAP_FRAMES_CHUNK			0x0004
#define GAMEMTL_DCT_FRAMES_CHUNK				GAMEMTL_TEXTURE_FRAMES_CHUNK
#define GAMEMTL_DIT_FRAMES_CHUNK				GAMEMTL_ENVMAP_FRAMES_CHUNK
#define GAMEMTL_SCT_FRAMES_CHUNK				0x0005
#define GAMEMTL_SIT_FRAMES_CHUNK				0x0006

#define GAMEMTL_DCT_FRAME_RATE_CHUNK		0x0010
#define GAMEMTL_DIT_FRAME_RATE_CHUNK		0x0011
#define GAMEMTL_SCT_FRAME_RATE_CHUNK		0x0012
#define GAMEMTL_SIT_FRAME_RATE_CHUNK		0x0013

#define GAMEMTL_DCT_MAPPING_CHUNK			0x0020
#define GAMEMTL_DIT_MAPPING_CHUNK			0x0021
#define GAMEMTL_SCT_MAPPING_CHUNK			0x0022
#define GAMEMTL_SIT_MAPPING_CHUNK			0x0023

#define GAMEMTL_ATTRIBUTES_CHUNK				0x0030
#define GAMEMTL_DIFFUSE_COLOR_CHUNK			0x0031
#define GAMEMTL_SPECULAR_COLOR_CHUNK		0x0032

#define GAMEMTL_AMBIENT_COEFF_CHUNK			0x0040
#define GAMEMTL_DIFFUSE_COEFF_CHUNK			0x0041
#define GAMEMTL_SPECULAR_COEFF_CHUNK		0x0042
#define GAMEMTL_EMISSIVE_COEFF_CHUNK		0x0043

#define GAMEMTL_OPACITY_CHUNK					0x0050
#define GAMEMTL_TRANSLUCENCY_CHUNK			0x0051
#define GAMEMTL_SHININESS_CHUNK				0x0052
#define GAMEMTL_FOG_CHUNK						0x0053

#define GAMEMTL_PASS0_CUR_PAGE				0x0060
#define GAMEMTL_PASS1_CUR_PAGE				0x0061
#define GAMEMTL_PASS2_CUR_PAGE				0x0062
#define GAMEMTL_PASS3_CUR_PAGE				0x0064

#define GAMEMTL_PASS0_STAGE0_MAPPER_ARGS	0x0070
#define GAMEMTL_PASS1_STAGE0_MAPPER_ARGS	0x0071
#define GAMEMTL_PASS2_STAGE0_MAPPER_ARGS	0x0072
#define GAMEMTL_PASS3_STAGE0_MAPPER_ARGS	0x0073

#define GAMEMTL_SURFACE_TYPE_CHUNK			0x0080

#define GAMEMTL_SORT_LEVEL_CHUNK				0x0090

#define GAMEMTL_PASS0_STAGE1_MAPPER_ARGS	0x0100
#define GAMEMTL_PASS1_STAGE1_MAPPER_ARGS	0x0101
#define GAMEMTL_PASS2_STAGE1_MAPPER_ARGS	0x0102
#define GAMEMTL_PASS3_STAGE1_MAPPER_ARGS	0x0103


/*
** Main Parameter Block Definition
*/
static ParamBlockDescID MainParameterBlockDesc[] = 
{
	{ TYPE_INT,			NULL,	FALSE,	0 },		// Pass Count		
};


/*
** ID numbers for parameter block entries. As of version 4 of the
** parameter block definitions, these enums should be used directly
** in the pblock defs to avoid accidental ID mismatches.
**
** Note: You should DO NOTHING TO CHANGE THE VALUES OF THE ENUM
** (except adding at the end of course). Following these constraints
** will preserve backwards compatibility with all pblock versions.
**
** Since this enum is now just a series of ID numbers, we must
** manually keep the PASS_PARAMBLOCK_LENGTH up to date. Believe it
** or not, but this is less error-prone than the previous way of
** doing things.
**
** (gth) Aug 6, 2000
** IMPORTANT -PLEASE READ- 
** The following enums are basically the index into the array of
** ParamBlockDescID's that we are using.  It is critical that this
** enumeration does not skip indexes and that the value for each
** symbol in this list matches the position in the param block
** for that variable.  This enumeration is does not define the
** id for the new variable, only its position in the array.
**
** When you add a new variable to the parameter block, you should
** find an id for it higher than any existing id (look through the
** rest of the entries in the parameter block).  That id will not
** usually be able to be the same as its array index due to the
** evolution of this structure.  The id I'm referring to is the
** number in the 4th element of each of the ParamBlockDescIDs.
** 
** If you remove an entry from our array of ParamBlockDescIDs you
** will need to update all of the PB_xxx enumeration below so that 
** they match their position in the array again.
*/

enum
{
	PB_AMBIENT										= 0,
	PB_DIFFUSE										= 1,
	PB_SPECULAR										= 2,
	PB_EMISSIVE										= 3,
	PB_SHININESS									= 4,
	PB_OPACITY										= 5,
	PB_TRANSLUCENCY								= 6,
	PB_COPY_SPECULAR_TO_DIFFUSE				= 7,
	PB_STAGE0_MAPPING_TYPE						= 8,
	PB_PSX_TRANSLUCENCY							= 9,
	PB_PSX_LIGHTING								= 10,

	PB_DEPTH_COMPARE								= 11,
	PB_DEPTH_MASK									= 12,
	PB_COLOR_MASK									= 13,	// obsolete (ignored)
	PB_DEST_BLEND									= 14,
	PB_FOG_FUNC										= 15,	// obsolete (ignored)
	PB_PRI_GRADIENT								= 16,
	PB_SEC_GRADIENT								= 17,
	PB_SRC_BLEND									= 18,
	PB_DETAIL_COLOR_FUNC							= 19,
	PB_DETAIL_ALPHA_FUNC							= 20,

	PB_STAGE0_TEXTURE_ENABLE					= 21,
	PB_STAGE0_TEXTURE_PUBLISH					= 22,
	PB_STAGE0_TEXTURE_RESIZE					= 23,	// obsolete (ignored)
	PB_STAGE0_TEXTURE_NO_MIPMAP				= 24,	// obsolete (ignored)
	PB_STAGE0_TEXTURE_CLAMP_U					= 25,
	PB_STAGE0_TEXTURE_CLAMP_V					= 26,
	PB_STAGE0_TEXTURE_HINT						= 27,
	PB_STAGE0_TEXTURE_DISPLAY					= 28,
	PB_STAGE0_TEXTURE_FRAME_RATE				= 29,
	PB_STAGE0_TEXTURE_FRAME_COUNT				= 30,
	PB_STAGE0_TEXTURE_ANIM_TYPE				= 31,

	PB_STAGE1_TEXTURE_ENABLE					= 32,
	PB_STAGE1_TEXTURE_PUBLISH					= 33,
	PB_STAGE1_TEXTURE_RESIZE					= 34,	// obsolete (ignored)
	PB_STAGE1_TEXTURE_NO_MIPMAP				= 35,	// obsolete (ignored)
	PB_STAGE1_TEXTURE_CLAMP_U					= 36,
	PB_STAGE1_TEXTURE_CLAMP_V					= 37,
	PB_STAGE1_TEXTURE_HINT						= 38,
	PB_STAGE1_TEXTURE_DISPLAY					= 39,
	PB_STAGE1_TEXTURE_FRAME_RATE				= 40,
	PB_STAGE1_TEXTURE_FRAME_COUNT				= 41,
	PB_STAGE1_TEXTURE_ANIM_TYPE				= 42,

	PB_STAGE0_TEXTURE_ALPHA_BITMAP			= 43,
	PB_STAGE1_TEXTURE_ALPHA_BITMAP			= 44,

	PB_ALPHA_TEST									= 45,

	PB_SHADER_PRESET								= 46,	// obsolete (ignored)

	// For the Playstation 2.
	PB_PS2_SHADER_PARAM_A						= 47,
	PB_PS2_SHADER_PARAM_B						= 48,
	PB_PS2_SHADER_PARAM_C						= 49,
	PB_PS2_SHADER_PARAM_D						= 50,

	// UV Channel selection
	PB_STAGE0_MAP_CHANNEL						= 51,
	PB_STAGE1_MAP_CHANNEL						= 52,

	PB_STAGE1_MAPPING_TYPE						= 53,	// (gth) this can't be 55, it is used as an array index	// = 55,	// yes, 55

	// textures can now disable LOD
	PB_STAGE0_TEXTURE_NO_LOD					= 54,
	PB_STAGE1_TEXTURE_NO_LOD					= 55,
};


/*
** Per-Pass Parameter Block Definition
*/

// Version 0 (old version)
static ParamBlockDescID PassParameterBlockDescVer0[] = 
{ 
	{ TYPE_POINT3,		NULL, TRUE,		0 },		// Ambient
	{ TYPE_POINT3,		NULL, TRUE,		1 },		// Diffuse
	{ TYPE_POINT3,		NULL, TRUE,		2 },		// Specular
	{ TYPE_POINT3,		NULL,	TRUE,		3 },		// Emissive
	{ TYPE_FLOAT,		NULL, TRUE,		4 },		// Shininess
	{ TYPE_FLOAT,		NULL, TRUE,		5 },		// Opacity
	{ TYPE_FLOAT,		NULL, TRUE,		6 },		// Translucency
	{ TYPE_INT,			NULL,	FALSE,	7 },		// Mapping Type		
	{ TYPE_INT,			NULL, FALSE,	8 },		// PSX Translucency Type
	{ TYPE_BOOL,		NULL,	FALSE,	9 },		// PSX Lighting Flag

	{ TYPE_INT,			NULL,	FALSE,	10},		// Depth Compare
	{ TYPE_INT,			NULL,	FALSE,	11},		// Depth Mask
	{ TYPE_INT,			NULL,	FALSE,	12},		// Color Mask
	{ TYPE_INT,			NULL,	FALSE,	13},		// Dest Blend
	{ TYPE_INT,			NULL,	FALSE,	14},		// FogFunc
	{ TYPE_INT,			NULL,	FALSE,	15},		// PriGradient
	{ TYPE_INT,			NULL,	FALSE,	16},		// SecGradient
	{ TYPE_INT,			NULL,	FALSE,	17},		// SrcBlend
	{ TYPE_INT,			NULL,	FALSE,	18},		// DetailColorFunc
	{ TYPE_INT,			NULL,	FALSE,	19},		// DetailAlphaFunc
	{ TYPE_INT,			NULL,	FALSE,	20},		// DitherMask
	{ TYPE_INT,			NULL,	FALSE,	21},		// Shade Model

	{ TYPE_BOOL,		NULL,	FALSE,	22},		// Stage0 Texture Enable
	{ TYPE_BOOL,		NULL,	FALSE,	23},		// Stage0 Texture Publish
	{ TYPE_BOOL,		NULL,	FALSE,	24},		// Stage0 Texture Display (in viewport...)
	{ TYPE_FLOAT,		NULL,	FALSE,	25},		// Stage0 Frame Rate
	{ TYPE_INT,			NULL,	FALSE,	26},		// Stage0 Frame Count
	{ TYPE_INT,			NULL, FALSE,	27},		// Stage0 Animation Type
		
	{ TYPE_BOOL,		NULL,	FALSE,	28},		// Stage1 Texture Enable
	{ TYPE_BOOL,		NULL,	FALSE,	29},		// Stage1 Texture Publish
	{ TYPE_BOOL,		NULL,	FALSE,	30},		// Stage1 Texture Display (in viewport...)
	{ TYPE_FLOAT,		NULL,	FALSE,	31},		// Stage1 Frame Rate
	{ TYPE_INT,			NULL,	FALSE,	32},		// Stage1 Frame Count
	{ TYPE_INT,			NULL, FALSE,	33},		// Stage1 Animation Type

};

// Version 1 
static ParamBlockDescID PassParameterBlockDescVer1[] = 
{ 
	{ TYPE_POINT3,		NULL, TRUE,		0 },		// Ambient
	{ TYPE_POINT3,		NULL, TRUE,		1 },		// Diffuse
	{ TYPE_POINT3,		NULL, TRUE,		2 },		// Specular
	{ TYPE_POINT3,		NULL,	TRUE,		3 },		// Emissive
	{ TYPE_FLOAT,		NULL, TRUE,		4 },		// Shininess
	{ TYPE_FLOAT,		NULL, TRUE,		5 },		// Opacity
	{ TYPE_FLOAT,		NULL, TRUE,		6 },		// Translucency
	{ TYPE_BOOL,		NULL,	FALSE,	34},		// Copy specular to diffuse (new to version 1)
	{ TYPE_INT,			NULL,	FALSE,	7 },		// Mapping Type		
	{ TYPE_INT,			NULL, FALSE,	8 },		// PSX Translucency Type
	{ TYPE_BOOL,		NULL,	FALSE,	9 },		// PSX Lighting Flag

	{ TYPE_INT,			NULL,	FALSE,	10},		// Depth Compare
	{ TYPE_INT,			NULL,	FALSE,	11},		// Depth Mask
	{ TYPE_INT,			NULL,	FALSE,	12},		// Color Mask (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	13},		// Dest Blend
	{ TYPE_INT,			NULL,	FALSE,	14},		// FogFunc (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	15},		// PriGradient
	{ TYPE_INT,			NULL,	FALSE,	16},		// SecGradient
	{ TYPE_INT,			NULL,	FALSE,	17},		// SrcBlend
	{ TYPE_INT,			NULL,	FALSE,	18},		// DetailColorFunc
	{ TYPE_INT,			NULL,	FALSE,	19},		// DetailAlphaFunc

	{ TYPE_BOOL,		NULL,	FALSE,	22},		// Stage0 Texture Enable
	{ TYPE_BOOL,		NULL,	FALSE,	23},		// Stage0 Texture Publish
	{ TYPE_BOOL,		NULL, FALSE,	35},		// Stage0 Texture Resize (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	36},		// Stage0 Texture No Mipmap (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	37},		// Stage0 Texture Clamp U (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	38},		// Stage0 Texture Clamp V (new to version 1)
	{ TYPE_INT,			NULL, FALSE,	39},		// Stage0 Texture Hint (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	24},		// Stage0 Texture Display (in viewport...)
	{ TYPE_FLOAT,		NULL,	FALSE,	25},		// Stage0 Frame Rate
	{ TYPE_INT,			NULL,	FALSE,	26},		// Stage0 Frame Count
	{ TYPE_INT,			NULL, FALSE,	27},		// Stage0 Animation Type
		
	{ TYPE_BOOL,		NULL,	FALSE,	28},		// Stage1 Texture Enable
	{ TYPE_BOOL,		NULL,	FALSE,	29},		// Stage1 Texture Publish
	{ TYPE_BOOL,		NULL, FALSE,	40},		// Stage1 Texture Resize (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	41},		// Stage1 Texture No Mipmap (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	42},		// Stage1 Texture Clamp U (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	43},		// Stage1 Texture Clamp V (new to version 1)
	{ TYPE_INT,			NULL, FALSE,	44},		// Stage1 Texture Hint (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	30},		// Stage1 Texture Display (in viewport...)
	{ TYPE_FLOAT,		NULL,	FALSE,	31},		// Stage1 Frame Rate
	{ TYPE_INT,			NULL,	FALSE,	32},		// Stage1 Frame Count
	{ TYPE_INT,			NULL, FALSE,	33},		// Stage1 Animation Type

	{ TYPE_BOOL,		NULL,	FALSE,	45},		// Stage0 Texture Alpha Bitmap (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	46},		// Stage1 Texture Alpha Bitmap (new to version 1)

	{ TYPE_BOOL,		NULL,	FALSE,	47},		// Alpha Test (new to version 1)
	{ TYPE_INT,			NULL,	FALSE,	48},		// Shader preset (new to version 1) (now obsolete and ignored)
};

// Version 2 (old version)
static ParamBlockDescID PassParameterBlockDescVer2[] = 
{ 
	{ TYPE_POINT3,		NULL, TRUE,		0 },		// Ambient
	{ TYPE_POINT3,		NULL, TRUE,		1 },		// Diffuse
	{ TYPE_POINT3,		NULL, TRUE,		2 },		// Specular
	{ TYPE_POINT3,		NULL,	TRUE,		3 },		// Emissive
	{ TYPE_FLOAT,		NULL, TRUE,		4 },		// Shininess
	{ TYPE_FLOAT,		NULL, TRUE,		5 },		// Opacity
	{ TYPE_FLOAT,		NULL, TRUE,		6 },		// Translucency
	{ TYPE_BOOL,		NULL,	FALSE,	34},		// Copy specular to diffuse (new to version 1)
	{ TYPE_INT,			NULL,	FALSE,	7 },		// Mapping Type		
	{ TYPE_INT,			NULL, FALSE,	8 },		// PSX Translucency Type
	{ TYPE_BOOL,		NULL,	FALSE,	9 },		// PSX Lighting Flag

	{ TYPE_INT,			NULL,	FALSE,	10},		// Depth Compare
	{ TYPE_INT,			NULL,	FALSE,	11},		// Depth Mask
	{ TYPE_INT,			NULL,	FALSE,	12},		// Color Mask (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	13},		// Dest Blend
	{ TYPE_INT,			NULL,	FALSE,	14},		// FogFunc (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	15},		// PriGradient
	{ TYPE_INT,			NULL,	FALSE,	16},		// SecGradient
	{ TYPE_INT,			NULL,	FALSE,	17},		// SrcBlend
	{ TYPE_INT,			NULL,	FALSE,	18},		// DetailColorFunc
	{ TYPE_INT,			NULL,	FALSE,	19},		// DetailAlphaFunc

	{ TYPE_BOOL,		NULL,	FALSE,	22},		// Stage0 Texture Enable
	{ TYPE_BOOL,		NULL,	FALSE,	23},		// Stage0 Texture Publish
	{ TYPE_BOOL,		NULL, FALSE,	35},		// Stage0 Texture Resize (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	36},		// Stage0 Texture No Mipmap (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	37},		// Stage0 Texture Clamp U (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	38},		// Stage0 Texture Clamp V (new to version 1)
	{ TYPE_INT,			NULL, FALSE,	39},		// Stage0 Texture Hint (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	24},		// Stage0 Texture Display (in viewport...)
	{ TYPE_FLOAT,		NULL,	FALSE,	25},		// Stage0 Frame Rate
	{ TYPE_INT,			NULL,	FALSE,	26},		// Stage0 Frame Count
	{ TYPE_INT,			NULL, FALSE,	27},		// Stage0 Animation Type
		
	{ TYPE_BOOL,		NULL,	FALSE,	28},		// Stage1 Texture Enable
	{ TYPE_BOOL,		NULL,	FALSE,	29},		// Stage1 Texture Publish
	{ TYPE_BOOL,		NULL, FALSE,	40},		// Stage1 Texture Resize (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	41},		// Stage1 Texture No Mipmap (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	42},		// Stage1 Texture Clamp U (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	43},		// Stage1 Texture Clamp V (new to version 1)
	{ TYPE_INT,			NULL, FALSE,	44},		// Stage1 Texture Hint (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	30},		// Stage1 Texture Display (in viewport...)
	{ TYPE_FLOAT,		NULL,	FALSE,	31},		// Stage1 Frame Rate
	{ TYPE_INT,			NULL,	FALSE,	32},		// Stage1 Frame Count
	{ TYPE_INT,			NULL, FALSE,	33},		// Stage1 Animation Type

	{ TYPE_BOOL,		NULL,	FALSE,	45},		// Stage0 Texture Alpha Bitmap (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	46},		// Stage1 Texture Alpha Bitmap (new to version 1)

	{ TYPE_BOOL,		NULL,	FALSE,	47},		// Alpha Test (new to version 1)
	{ TYPE_INT,			NULL,	FALSE,	48},		// Shader preset (new to version 1) (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	49},		// PS2 Shader Param A
	{ TYPE_INT,			NULL,	FALSE,	50},		// PS2 Shader Param B
	{ TYPE_INT,			NULL,	FALSE,	51},		// PS2 Shader Param C
	{ TYPE_INT,			NULL,	FALSE,	52},		// PS2 Shader Param D
};

// Version 3 (old version)
static ParamBlockDescID PassParameterBlockDescVer3[] = 
{ 
	{ TYPE_POINT3,		NULL, TRUE,		0 },		// Ambient
	{ TYPE_POINT3,		NULL, TRUE,		1 },		// Diffuse
	{ TYPE_POINT3,		NULL, TRUE,		2 },		// Specular
	{ TYPE_POINT3,		NULL,	TRUE,		3 },		// Emissive
	{ TYPE_FLOAT,		NULL, TRUE,		4 },		// Shininess
	{ TYPE_FLOAT,		NULL, TRUE,		5 },		// Opacity
	{ TYPE_FLOAT,		NULL, TRUE,		6 },		// Translucency
	{ TYPE_BOOL,		NULL,	FALSE,	34},		// Copy specular to diffuse (new to version 1)
	{ TYPE_INT,			NULL,	FALSE,	7 },		// Mapping Type		
	{ TYPE_INT,			NULL, FALSE,	8 },		// PSX Translucency Type
	{ TYPE_BOOL,		NULL,	FALSE,	9 },		// PSX Lighting Flag

	{ TYPE_INT,			NULL,	FALSE,	10},		// Depth Compare
	{ TYPE_INT,			NULL,	FALSE,	11},		// Depth Mask
	{ TYPE_INT,			NULL,	FALSE,	12},		// Color Mask (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	13},		// Dest Blend
	{ TYPE_INT,			NULL,	FALSE,	14},		// FogFunc (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	15},		// PriGradient
	{ TYPE_INT,			NULL,	FALSE,	16},		// SecGradient
	{ TYPE_INT,			NULL,	FALSE,	17},		// SrcBlend
	{ TYPE_INT,			NULL,	FALSE,	18},		// DetailColorFunc
	{ TYPE_INT,			NULL,	FALSE,	19},		// DetailAlphaFunc

	{ TYPE_BOOL,		NULL,	FALSE,	22},		// Stage0 Texture Enable
	{ TYPE_BOOL,		NULL,	FALSE,	23},		// Stage0 Texture Publish
	{ TYPE_BOOL,		NULL, FALSE,	35},		// Stage0 Texture Resize (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	36},		// Stage0 Texture No Mipmap (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	37},		// Stage0 Texture Clamp U (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	38},		// Stage0 Texture Clamp V (new to version 1)
	{ TYPE_INT,			NULL, FALSE,	39},		// Stage0 Texture Hint (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	24},		// Stage0 Texture Display (in viewport...)
	{ TYPE_FLOAT,		NULL,	FALSE,	25},		// Stage0 Frame Rate
	{ TYPE_INT,			NULL,	FALSE,	26},		// Stage0 Frame Count
	{ TYPE_INT,			NULL, FALSE,	27},		// Stage0 Animation Type
		
	{ TYPE_BOOL,		NULL,	FALSE,	28},		// Stage1 Texture Enable
	{ TYPE_BOOL,		NULL,	FALSE,	29},		// Stage1 Texture Publish
	{ TYPE_BOOL,		NULL, FALSE,	40},		// Stage1 Texture Resize (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	41},		// Stage1 Texture No Mipmap (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	42},		// Stage1 Texture Clamp U (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	43},		// Stage1 Texture Clamp V (new to version 1)
	{ TYPE_INT,			NULL, FALSE,	44},		// Stage1 Texture Hint (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	30},		// Stage1 Texture Display (in viewport...)
	{ TYPE_FLOAT,		NULL,	FALSE,	31},		// Stage1 Frame Rate
	{ TYPE_INT,			NULL,	FALSE,	32},		// Stage1 Frame Count
	{ TYPE_INT,			NULL, FALSE,	33},		// Stage1 Animation Type

	{ TYPE_BOOL,		NULL,	FALSE,	45},		// Stage0 Texture Alpha Bitmap (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	46},		// Stage1 Texture Alpha Bitmap (new to version 1)

	{ TYPE_BOOL,		NULL,	FALSE,	47},		// Alpha Test (new to version 1)
	{ TYPE_INT,			NULL,	FALSE,	48},		// Shader preset (new to version 1) (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	49},		// PS2 Shader Param A
	{ TYPE_INT,			NULL,	FALSE,	50},		// PS2 Shader Param B
	{ TYPE_INT,			NULL,	FALSE,	51},		// PS2 Shader Param C
	{ TYPE_INT,			NULL,	FALSE,	52},		// PS2 Shader Param D

	{ TYPE_INT,			NULL,	FALSE,	53},		// Stage0 UV Channel
	{ TYPE_INT,			NULL,	FALSE,	54},		// Stage1 UV Channel
};


// Version 4 (old version)
static ParamBlockDescID PassParameterBlockDescVer4[] = 
{ 
	{ TYPE_POINT3,		NULL, TRUE,		0 },		// Ambient
	{ TYPE_POINT3,		NULL, TRUE,		1 },		// Diffuse
	{ TYPE_POINT3,		NULL, TRUE,		2 },		// Specular
	{ TYPE_POINT3,		NULL,	TRUE,		3 },		// Emissive
	{ TYPE_FLOAT,		NULL, TRUE,		4 },		// Shininess
	{ TYPE_FLOAT,		NULL, TRUE,		5 },		// Opacity
	{ TYPE_FLOAT,		NULL, TRUE,		6 },		// Translucency
	{ TYPE_BOOL,		NULL,	FALSE,	34},		// Copy specular to diffuse (new to version 1)
	{ TYPE_INT,			NULL,	FALSE,	7 },		// Stage0 Mapping Type		
	{ TYPE_INT,			NULL, FALSE,	8 },		// PSX Translucency Type
	{ TYPE_BOOL,		NULL,	FALSE,	9 },		// PSX Lighting Flag

	{ TYPE_INT,			NULL,	FALSE,	10},		// Depth Compare
	{ TYPE_INT,			NULL,	FALSE,	11},		// Depth Mask
	{ TYPE_INT,			NULL,	FALSE,	12},		// Color Mask (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	13},		// Dest Blend
	{ TYPE_INT,			NULL,	FALSE,	14},		// FogFunc (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	15},		// PriGradient
	{ TYPE_INT,			NULL,	FALSE,	16},		// SecGradient
	{ TYPE_INT,			NULL,	FALSE,	17},		// SrcBlend
	{ TYPE_INT,			NULL,	FALSE,	18},		// DetailColorFunc
	{ TYPE_INT,			NULL,	FALSE,	19},		// DetailAlphaFunc

	{ TYPE_BOOL,		NULL,	FALSE,	22},		// Stage0 Texture Enable
	{ TYPE_BOOL,		NULL,	FALSE,	23},		// Stage0 Texture Publish
	{ TYPE_BOOL,		NULL, FALSE,	35},		// Stage0 Texture Resize (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	36},		// Stage0 Texture No Mipmap (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	37},		// Stage0 Texture Clamp U (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	38},		// Stage0 Texture Clamp V (new to version 1)
	{ TYPE_INT,			NULL, FALSE,	39},		// Stage0 Texture Hint (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	24},		// Stage0 Texture Display (in viewport...)
	{ TYPE_FLOAT,		NULL,	FALSE,	25},		// Stage0 Frame Rate
	{ TYPE_INT,			NULL,	FALSE,	26},		// Stage0 Frame Count
	{ TYPE_INT,			NULL, FALSE,	27},		// Stage0 Animation Type
		
	{ TYPE_BOOL,		NULL,	FALSE,	28},		// Stage1 Texture Enable
	{ TYPE_BOOL,		NULL,	FALSE,	29},		// Stage1 Texture Publish
	{ TYPE_BOOL,		NULL, FALSE,	40},		// Stage1 Texture Resize (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	41},		// Stage1 Texture No Mipmap (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	42},		// Stage1 Texture Clamp U (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	43},		// Stage1 Texture Clamp V (new to version 1)
	{ TYPE_INT,			NULL, FALSE,	44},		// Stage1 Texture Hint (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	30},		// Stage1 Texture Display (in viewport...)
	{ TYPE_FLOAT,		NULL,	FALSE,	31},		// Stage1 Frame Rate
	{ TYPE_INT,			NULL,	FALSE,	32},		// Stage1 Frame Count
	{ TYPE_INT,			NULL, FALSE,	33},		// Stage1 Animation Type

	{ TYPE_BOOL,		NULL,	FALSE,	45},		// Stage0 Texture Alpha Bitmap (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	46},		// Stage1 Texture Alpha Bitmap (new to version 1)

	{ TYPE_BOOL,		NULL,	FALSE,	47},		// Alpha Test (new to version 1)
	{ TYPE_INT,			NULL,	FALSE,	48},		// Shader preset (new to version 1) (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	49},		// PS2 Shader Param A
	{ TYPE_INT,			NULL,	FALSE,	50},		// PS2 Shader Param B
	{ TYPE_INT,			NULL,	FALSE,	51},		// PS2 Shader Param C
	{ TYPE_INT,			NULL,	FALSE,	52},		// PS2 Shader Param D

	{ TYPE_INT,			NULL,	FALSE,	53},		// Stage0 UV Channel
	{ TYPE_INT,			NULL,	FALSE,	54},		// Stage1 UV Channel

	{ TYPE_INT,			NULL,	FALSE,	9998},	// foo
	{ TYPE_INT,			NULL,	FALSE,	9999},	// bar

	{ TYPE_INT,			NULL,	FALSE,	55},		// Stage1 Mapping Type (new to version 4)
};


// Version 5 (current version)
static ParamBlockDescID PassParameterBlockDescVer5[] = 
{ 
	{ TYPE_POINT3,		NULL, TRUE,		0 },		// Ambient
	{ TYPE_POINT3,		NULL, TRUE,		1 },		// Diffuse
	{ TYPE_POINT3,		NULL, TRUE,		2 },		// Specular
	{ TYPE_POINT3,		NULL,	TRUE,		3 },		// Emissive
	{ TYPE_FLOAT,		NULL, TRUE,		4 },		// Shininess
	{ TYPE_FLOAT,		NULL, TRUE,		5 },		// Opacity
	{ TYPE_FLOAT,		NULL, TRUE,		6 },		// Translucency
	{ TYPE_BOOL,		NULL,	FALSE,	34},		// Copy specular to diffuse (new to version 1)
	{ TYPE_INT,			NULL,	FALSE,	7 },		// Stage0 Mapping Type		
	{ TYPE_INT,			NULL, FALSE,	8 },		// PSX Translucency Type
	{ TYPE_BOOL,		NULL,	FALSE,	9 },		// PSX Lighting Flag

	{ TYPE_INT,			NULL,	FALSE,	10},		// Depth Compare
	{ TYPE_INT,			NULL,	FALSE,	11},		// Depth Mask
	{ TYPE_INT,			NULL,	FALSE,	12},		// Color Mask (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	13},		// Dest Blend
	{ TYPE_INT,			NULL,	FALSE,	14},		// FogFunc (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	15},		// PriGradient
	{ TYPE_INT,			NULL,	FALSE,	16},		// SecGradient
	{ TYPE_INT,			NULL,	FALSE,	17},		// SrcBlend
	{ TYPE_INT,			NULL,	FALSE,	18},		// DetailColorFunc
	{ TYPE_INT,			NULL,	FALSE,	19},		// DetailAlphaFunc

	{ TYPE_BOOL,		NULL,	FALSE,	22},		// Stage0 Texture Enable
	{ TYPE_BOOL,		NULL,	FALSE,	23},		// Stage0 Texture Publish
	{ TYPE_BOOL,		NULL, FALSE,	35},		// Stage0 Texture Resize (new to version 1) OBSOLETE!
	{ TYPE_BOOL,		NULL, FALSE,	36},		// Stage0 Texture No Mipmap (new to version 1) OBSOLETE!
	{ TYPE_BOOL,		NULL, FALSE,	37},		// Stage0 Texture Clamp U (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	38},		// Stage0 Texture Clamp V (new to version 1)
	{ TYPE_INT,			NULL, FALSE,	39},		// Stage0 Texture Hint (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	24},		// Stage0 Texture Display (in viewport...)
	{ TYPE_FLOAT,		NULL,	FALSE,	25},		// Stage0 Frame Rate
	{ TYPE_INT,			NULL,	FALSE,	26},		// Stage0 Frame Count
	{ TYPE_INT,			NULL, FALSE,	27},		// Stage0 Animation Type
		
	{ TYPE_BOOL,		NULL,	FALSE,	28},		// Stage1 Texture Enable
	{ TYPE_BOOL,		NULL,	FALSE,	29},		// Stage1 Texture Publish
	{ TYPE_BOOL,		NULL, FALSE,	40},		// Stage1 Texture Resize (new to version 1)	 OBSOLETE! 
	{ TYPE_BOOL,		NULL, FALSE,	41},		// Stage1 Texture No Mipmap (new to version 1) OBSOLETE!
	{ TYPE_BOOL,		NULL, FALSE,	42},		// Stage1 Texture Clamp U (new to version 1)
	{ TYPE_BOOL,		NULL, FALSE,	43},		// Stage1 Texture Clamp V (new to version 1)
	{ TYPE_INT,			NULL, FALSE,	44},		// Stage1 Texture Hint (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	30},		// Stage1 Texture Display (in viewport...)
	{ TYPE_FLOAT,		NULL,	FALSE,	31},		// Stage1 Frame Rate
	{ TYPE_INT,			NULL,	FALSE,	32},		// Stage1 Frame Count
	{ TYPE_INT,			NULL, FALSE,	33},		// Stage1 Animation Type

	{ TYPE_BOOL,		NULL,	FALSE,	45},		// Stage0 Texture Alpha Bitmap (new to version 1)
	{ TYPE_BOOL,		NULL,	FALSE,	46},		// Stage1 Texture Alpha Bitmap (new to version 1)

	{ TYPE_BOOL,		NULL,	FALSE,	47},		// Alpha Test (new to version 1)
	{ TYPE_INT,			NULL,	FALSE,	48},		// Shader preset (new to version 1) (now obsolete and ignored)
	{ TYPE_INT,			NULL,	FALSE,	49},		// PS2 Shader Param A
	{ TYPE_INT,			NULL,	FALSE,	50},		// PS2 Shader Param B
	{ TYPE_INT,			NULL,	FALSE,	51},		// PS2 Shader Param C
	{ TYPE_INT,			NULL,	FALSE,	52},		// PS2 Shader Param D

	{ TYPE_INT,			NULL,	FALSE,	53},		// Stage0 UV Channel
	{ TYPE_INT,			NULL,	FALSE,	54},		// Stage1 UV Channel

	{ TYPE_INT,			NULL,	FALSE,	55	},		// Stage1 Mapping Type (new to version 4)

	{ TYPE_BOOL,		NULL,	FALSE,	56	},		// Stage0 no texture reduction (new to version 5)
	{ TYPE_BOOL,		NULL,	FALSE,	57	},		// Stage0 no texture reduction (new to version 5)
};

// Array of old pass parameter block versions (for backwards compatibility)
const int NUM_OLDVERSIONS = 5;
static ParamVersionDesc PassParameterBlockVersions[] = {
	ParamVersionDesc(PassParameterBlockDescVer0, 34, 0),
	ParamVersionDesc(PassParameterBlockDescVer1, 47, 1),
	ParamVersionDesc(PassParameterBlockDescVer2, 51, 2),
	ParamVersionDesc(PassParameterBlockDescVer3, 53, 3),
	ParamVersionDesc(PassParameterBlockDescVer4, 56, 4)
};


// Current pass parameter block version
const int CURRENT_VERSION = 5;
static ParamVersionDesc CurrentPassParameterBlockVersion(
							PassParameterBlockDescVer5, 
							sizeof(PassParameterBlockDescVer5) / sizeof(ParamBlockDescID),
							CURRENT_VERSION);


const int DISPLACEMENT_INDEX	= (W3dMaterialClass::MAX_PASSES * W3dMaterialClass::MAX_STAGES);

Color scale(const Color& a, const Color& b)
{
	return Color(a.r * b.r, a.g * b.g, a.b * b.b);
}

/***********************************************************************************************
 * GameMtl::GameMtl -- constructor                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
GameMtl::GameMtl(BOOL loading) 
{
	MaterialDialog = NULL;
	SurfaceType = SURFACE_TYPE_DEFAULT;
	SortLevel = SORT_LEVEL_NONE;

	Ivalid.SetEmpty();
	RollScroll = 0;
	Flags = 0;
	Flags |= GAMEMTL_PASSCOUNT_ROLLUP_OPEN;
	Set_Flag(GAMEMTL_CONVERTED_TO_NOLOD,true);

	DisplacementMap = NULL;
	DisplacementAmt = 0.0F;

	Maps = NULL;
	MainParameterBlock = NULL;
	for (int pass=0; pass<W3dMaterialClass::MAX_PASSES; pass++) {
		for (int stage=0; stage<W3dMaterialClass::MAX_STAGES; stage++) {
			Texture[pass][stage] = NULL;
			MapperArg[pass][stage] = NULL;
			MapperArgLen[pass][stage] = 0;
		}
		PassParameterBlock[pass] = NULL;
		CurPage[pass] = 0;
	}
	
	ShaderType = STE_PC_SHADER;
	SubstituteMaterial = NULL;
	
	if (!loading) {
		Reset();
	}

}


/***********************************************************************************************
 * GameMtl::~GameMtl -- destructor                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
GameMtl::~GameMtl(void)
{
	for (int pass=0; pass<W3dMaterialClass::MAX_PASSES; ++pass) {
		for (int stage=0; stage<W3dMaterialClass::MAX_STAGES; ++stage) {
			if (MapperArg[pass][stage]) {
				delete [] (MapperArg[pass][stage]);
				MapperArg[pass][stage] = NULL;
			}
		}
	}
}


/***********************************************************************************************
 * GameMtl::ClassID -- returns the max ClassID of the material plugin                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
Class_ID GameMtl::ClassID() 
{ 
	if (ShaderType == STE_PC_SHADER) {
		return GameMaterialClassID; 
	} else {
		return PS2GameMaterialClassID; 
	}
}


/***********************************************************************************************
 * GameMtl::SuperClassID -- returns the super class ID                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
SClass_ID GameMtl::SuperClassID() 
{
	return MATERIAL_CLASS_ID; 
}


/***********************************************************************************************
 * GameMtl::GetClassName -- returns the name of this plugin clas                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtl::GetClassName(TSTR& s)
{
	if (ShaderType == STE_PC_SHADER) {
		s = Get_String(IDS_GAMEMTL);
	}
	else {
		s = Get_String(IDS_PS2_GAMEMTL);
	}
}  


/***********************************************************************************************
 * GameMtl::NumSubs -- returns the number of sub animations                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
int GameMtl::NumSubs()
{
	return 0;
}


/***********************************************************************************************
 * GameMtl::SubAnimName -- returns the name of the i'th sub animation                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
TSTR GameMtl::SubAnimName(int i) 
{ 
	return _T("");
}		


/***********************************************************************************************
 * GameMtl::SubAnim -- returns the i'th sub-anim                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
Animatable* GameMtl::SubAnim(int i) 
{
	return NULL;
}


/***********************************************************************************************
 * GameMtl::Clone -- clones this material                                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
RefTargetHandle GameMtl::Clone(RemapDir &remap) 
{
	DebugPrint("GameMtl::Clone\n");

	GameMtl *mnew = new GameMtl(FALSE);
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff

	// replace the main parameter block
	mnew->ReplaceReference(REF_MAIN,remap.CloneRef(MainParameterBlock));

	// Clone the displacment map's settings
	mnew->ReplaceReference (REF_TEXTURE + DISPLACEMENT_INDEX, DisplacementMap);
	mnew->DisplacementAmt = DisplacementAmt;

	// Maintain the shader type.
	mnew->Set_Shader_Type(ShaderType);
	
	// replace each pass's parameter block and the textures
	for (int pass=0; pass < W3dMaterialClass::MAX_PASSES; pass++) {

		IParamBlock * pblock = NULL;
		if (PassParameterBlock[pass]) {
			pblock = (IParamBlock *)remap.CloneRef(PassParameterBlock[pass]);
		}
		mnew->ReplaceReference(pass_ref_index(pass), pblock);
		mnew->PassParameterBlock[pass] = pblock;

		for (int stage=0; stage < W3dMaterialClass::MAX_STAGES; stage++) {
			if (Texture[pass][stage]) {
				mnew->ReplaceReference(texture_ref_index(pass,stage),Texture[pass][stage]->Clone());
			} else {
				mnew->ReplaceReference(texture_ref_index(pass,stage),NULL);			
			}

			// Copy mapper arg strings and lengths
			if (MapperArgLen[pass][stage] > 0) {
				char *temp = new char[MapperArgLen[pass][stage] + 1];
				assert(strlen(MapperArg[pass][stage]) <= MapperArgLen[pass][stage]);
				strcpy(temp, MapperArg[pass][stage]);
				mnew->MapperArg[pass][stage] = temp;
				mnew->MapperArgLen[pass][stage] = MapperArgLen[pass][stage];
			}
		}

	}

	mnew->Ivalid = Ivalid;	
	return mnew;
}


/***********************************************************************************************
 * GameMtl::NotifyRefChanged -- NotifyRefChanged handler                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
RefResult GameMtl::NotifyRefChanged
(
	Interval					changeInt,
	RefTargetHandle		hTarget,
	PartID &					partID,
	RefMessage				message
) 
{
	switch (message) {
		case REFMSG_CHANGE:
		{
			Ivalid.SetEmpty();						// if any refs changed, clear the validity
			if (MaterialDialog && (MaterialDialog->TheMtl == this)) {
 				MaterialDialog->Invalidate();			// if the dialog is up, refresh it
		 	}
			break;
		}
		case REFMSG_GET_PARAM_DIM: 
		{
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = defaultDim; 
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: 
		{
			GetParamName *gpn = (GetParamName*)partID;
			bool pass_parameter = false;
			for(int pass = 0; pass < Get_Pass_Count(); pass++) {
				if(hTarget == (RefTargetHandle)PassParameterBlock[pass]) pass_parameter = true;
			}

			if (pass_parameter)	{
				switch (gpn->index) 
				{
					case PB_AMBIENT:			gpn->name = _T("Ambient");			break;
					case PB_DIFFUSE:			gpn->name = _T("Diffuse");			break;
					case PB_SPECULAR:			gpn->name = _T("Specular");		break;
					case PB_EMISSIVE:			gpn->name = _T("Emissive");		break;
					case PB_SHININESS:		gpn->name = _T("Shininess");		break;
					case PB_OPACITY:			gpn->name = _T("Opacity");			break;
					case PB_TRANSLUCENCY:	gpn->name = _T("Translucency");	break;
					default:						gpn->name = _T("");					break;
				}
			}
			return REF_STOP; 
		}
	}
	return(REF_SUCCEED);
}


/***********************************************************************************************
 * GameMtl::SetReference -- set the i'th reference                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtl::SetReference(int i, RefTargetHandle rtarg) 
{
	assert(i < REF_COUNT);

	if (i == REF_MAPS) {
		Maps = (GameMapsClass *)rtarg;
		return;
	}

	if (i == REF_MAIN) {
		MainParameterBlock = (IParamBlock*)rtarg;
		return;
	}

	if ((i >= REF_PASS_PARAMETERS) && (i < REF_PASS_PARAMETERS + W3dMaterialClass::MAX_PASSES)) {
		PassParameterBlock[i - REF_PASS_PARAMETERS] = (IParamBlock*)rtarg;
		return;
	}
	
	if ((i >= REF_TEXTURE) && (i < REF_TEXTURE + 9)) {
		if (i == REF_TEXTURE + DISPLACEMENT_INDEX) {
			DisplacementMap = (Texmap *)rtarg;
		} else {
			int offset = i - REF_TEXTURE;
			int pass = offset / 2;
			int stage = offset % 2;
			Texture[pass][stage] = (Texmap *)rtarg;
		}
		return;
	}
}


/***********************************************************************************************
 * GameMtl::GetReference -- returnst the i'th reference                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
RefTargetHandle GameMtl::GetReference(int i) 
{
	if (i == REF_MAPS) {
		return Maps;
	}
	if (i == REF_MAIN) {
		return MainParameterBlock;
	}
	if ((i >= REF_PASS_PARAMETERS) && (i < REF_PASS_PARAMETERS + W3dMaterialClass::MAX_PASSES)) {
		return PassParameterBlock[i - REF_PASS_PARAMETERS];
	}
	if ((i >= REF_TEXTURE) && (i < REF_TEXTURE + 9)) {
		if (i == REF_TEXTURE + DISPLACEMENT_INDEX) {
			return DisplacementMap;
		} else {
			int offset = i - REF_TEXTURE;
			int pass = offset / 2;
			int stage = offset % 2;
			return Texture[pass][stage];
		}
	}

	return NULL;
}


/***********************************************************************************************
 * GameMtl::NumSubTexmaps -- returns the number of texture maps in this material               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
int GameMtl::NumSubTexmaps(void)
{
	return (W3dMaterialClass::MAX_PASSES * W3dMaterialClass::MAX_STAGES) + 1;
}


/***********************************************************************************************
 * GameMtl::Get_Displacement_Map_Index -- returns the Sub-texmap index for the displacement map.
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/01/99   PDS : Created.                                                                 *
 *=============================================================================================*/
int GameMtl::Get_Displacement_Map_Index(void) const
{ 
	return DISPLACEMENT_INDEX;
}


/***********************************************************************************************
 * GameMtl::SetSubTexmap -- set the i'th texture map                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtl::SetSubTexmap(int i, Texmap * m)
{ 
	ReplaceReference(REF_TEXTURE + i, m);

	int pass,stage;
	texmap_index_to_pass_stage(i,&pass,&stage);
	if (Texture[pass][stage] != NULL) {
		UVGen * uvgen = Texture[pass][stage]->GetTheUVGen();
		if (uvgen != NULL) {
			uvgen->SetMapChannel(Get_Map_Channel(pass,stage));
		}
	}

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}


/***********************************************************************************************
 * GameMtl::GetSubTexmap -- returns the i'th texture map                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
Texmap * GameMtl::GetSubTexmap(int i) 
{ 
	if (i == DISPLACEMENT_INDEX) {
		return DisplacementMap;
	}

	int pass;
	int stage;
	texmap_index_to_pass_stage(i,&pass,&stage);
	return Texture[pass][stage]; 
}


/***********************************************************************************************
 * GameMtl::CreateParamDlg -- creates the material editor dialog box                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
ParamDlg * GameMtl::CreateParamDlg(HWND hwnd_mtl_edit, IMtlParams *imp) 
{
	GameMtlDlg *dlg = new GameMtlDlg(hwnd_mtl_edit, imp, this);
	SetParamDlg(dlg);
	return dlg;	
}


/***********************************************************************************************
 * GameMtl::Notify_Changed -- someone has changed this material                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtl::Notify_Changed(void)
{
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	if (MaterialDialog != NULL) {
		MaterialDialog->Update_Display();
	}
}


/***********************************************************************************************
 * GameMtl::Reset -- reset this material to default settings                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtl::Reset() 
{
	DebugPrint("GameMtl::Reset()\n");
	
	ReplaceReference(REF_MAIN,CreateParameterBlock(MainParameterBlockDesc,1,CURRENT_VERSION));

	for (int pass = 0; pass < W3dMaterialClass::MAX_PASSES; pass++) {
		
		// Install a parameter block for each pass
		IParamBlock * pblock = CreateParameterBlock(	PassParameterBlockDescVer5,
																	sizeof(PassParameterBlockDescVer5)/sizeof(ParamBlockDescID),
																	CURRENT_VERSION);

		ReplaceReference(pass_ref_index(pass), pblock);

		for (int stage = 0;stage < W3dMaterialClass::MAX_STAGES; stage++) {
			ReplaceReference(texture_ref_index(pass,stage), NULL);

			Set_Texture_Enable(pass,stage,false);
			Set_Texture_Publish(pass,stage,false);
			Set_Texture_Resize(pass,stage,false);
			Set_Texture_No_Mipmap(pass,stage,false);
			Set_Texture_Clamp_U(pass,stage,false);
			Set_Texture_Clamp_V(pass,stage,false);
			Set_Texture_No_LOD(pass,stage,false);
			Set_Texture_Alpha_Bitmap(pass,stage,false);
			Set_Texture_Hint(pass,stage,W3DTEXTURE_HINT_BASE);
			Set_Texture_Display(pass,stage,false);
			Set_Texture_Frame_Rate(pass,stage,15.0f);
			Set_Texture_Frame_Count(pass,stage,1);
			Set_Texture_Anim_Type(pass,stage,W3DTEXTURE_ANIM_LOOP);
			Set_Map_Channel(pass,stage,1);

			if (MapperArg[pass][stage]) {
				delete [] (MapperArg[pass][stage]);
				MapperArg[pass][stage] = NULL;
				MapperArgLen[pass][stage] = 0;
			}
		}

		Set_Sort_Level(SORT_LEVEL_NONE);
		
		Set_Ambient(pass,0,(pass == 0 ? Color(1.0,1.0,1.0) : Color(0,0,0)));
		Set_Diffuse(pass,0,(pass == 0 ? Color(1.0,1.0,1.0) : Color(0,0,0)));
		Set_Specular(pass,0,Color(0,0,0));
		Set_Emissive(pass,0,Color(0,0,0));
		Set_Shininess(pass,0,0.0f);
		Set_Opacity(pass,0,1.0f);
		Set_Translucency(pass,0,0.0f);
		Set_Copy_Specular_To_Diffuse(pass,false);
		Set_Mapping_Type(pass,0,W3DMAPPING_UV);
		Set_Mapping_Type(pass,1,W3DMAPPING_UV);
		Set_PSX_Translucency(pass,GAMEMTL_PSX_TRANS_NONE);
		Set_PSX_Lighting(pass,true);

		Set_Depth_Compare(pass,W3DSHADER_DEPTHCOMPARE_DEFAULT);
		Set_Depth_Mask(pass,W3DSHADER_DEPTHMASK_DEFAULT);
		Set_Alpha_Test(pass,W3DSHADER_ALPHATEST_DEFAULT);
		Set_Dest_Blend(pass,W3DSHADER_DESTBLENDFUNC_DEFAULT);
		Set_Pri_Gradient(pass,W3DSHADER_PRIGRADIENT_DEFAULT);
		Set_Sec_Gradient(pass,W3DSHADER_SECGRADIENT_DEFAULT);
		Set_Src_Blend(pass,W3DSHADER_SRCBLENDFUNC_DEFAULT);
		Set_Detail_Color_Func(pass,W3DSHADER_DETAILCOLORFUNC_DEFAULT);
		Set_Detail_Alpha_Func(pass,W3DSHADER_DETAILCOLORFUNC_DEFAULT);

		// Playstation 2 default for opaque.
		Set_PS2_Shader_Param_A(pass, PSS_SRC);
		Set_PS2_Shader_Param_B(pass, PSS_ZERO);
		Set_PS2_Shader_Param_C(pass, PSS_ONE);
		Set_PS2_Shader_Param_D(pass, PSS_ZERO);
	}

	Ivalid.SetEmpty();
	Set_Pass_Count(1);  // set default value for the main param block
}


/***********************************************************************************************
 * GameMtl::Update -- time has changed                                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtl::Update(TimeValue t, Interval &ivalid) 
{
	// This function is called by the system prior to rendering
	// Its purpose is to let you pre-calculate anything you can to
	// speed up the subsequent 'Shade' calls.
	Ivalid = FOREVER;
	ivalid &= Ivalid;
}


/***********************************************************************************************
 * GameMtl::Validity -- return the validity of the material at time t                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
Interval GameMtl::Validity(TimeValue t) 
{
	return FOREVER;	
}


/***********************************************************************************************
 * GameMtl::Requirements -- what requirements does this material have?                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
ULONG GameMtl::Requirements(int subMtlNum) 
{
	ULONG req = 0;
	
	for (int pass = 0; pass < W3dMaterialClass::MAX_PASSES; pass++) {
		for (int stage = 0; stage < W3dMaterialClass::MAX_STAGES; stage++) {
			if (Texture[pass][stage]) {
				req |= Texture[pass][stage]->Requirements(subMtlNum);
			}
		}
	}

	req |= MTLREQ_BGCOL;
	req |= MTLREQ_NOATMOS;

	#ifdef WANT_DISPLACEMENT_MAPS
		req |= MTLREQ_DISPLACEMAP;
	#endif //WANT_DISPLACEMENT_MAPS

	return req;
}


/***********************************************************************************************
 * GameMtl::Load -- loading from a MAX file                                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
IOResult GameMtl::Load(ILoad *iload) 
{
	ULONG nb;
	int id;
	int passindex = 0;
	int len = 0;
	unsigned char tmp8;
	unsigned short tmp16;
	char * tmpstring = NULL;
	float tmpfloat;
	IOResult res;

	GameMtlPostLoad * lc = new GameMtlPostLoad(this);
	iload->RegisterPostLoadCallback(lc);

	// Register automatic translation callbacks for the pass parameter blocks
	for (int i = 0; i < W3dMaterialClass::MAX_PASSES; i++) {
		iload->RegisterPostLoadCallback(
			new ParamBlockPLCB(PassParameterBlockVersions, NUM_OLDVERSIONS,
					&CurrentPassParameterBlockVersion, this, REF_PASS_PARAMETERS + i)
		);
	}
		
	while (IO_OK==(res=iload->OpenChunk())) {

		switch (id = iload->CurChunkID())  {

			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				Ivalid.SetEmpty();
				break;

			case GAMEMTL_FLAGS_CHUNK:
				res = iload->Read(&Flags,sizeof(Flags), &nb);
				break;

			case GAMEMTL_SURFACE_TYPE_CHUNK:
				res = iload->Read(&SurfaceType,sizeof(SurfaceType),&nb);
				break;

			case GAMEMTL_SORT_LEVEL_CHUNK:
				res = iload->Read(&SortLevel,sizeof(SortLevel),&nb);
				break;

			case GAMEMTL_PASS0_CUR_PAGE:
			case GAMEMTL_PASS1_CUR_PAGE:
			case GAMEMTL_PASS2_CUR_PAGE:
			case GAMEMTL_PASS3_CUR_PAGE:
				res = iload->Read(&tmp8,sizeof(tmp8),&nb);
				CurPage[id - GAMEMTL_PASS0_CUR_PAGE] = tmp8;
				break;
			
			case GAMEMTL_PASS0_STAGE0_MAPPER_ARGS:
			case GAMEMTL_PASS1_STAGE0_MAPPER_ARGS:
			case GAMEMTL_PASS2_STAGE0_MAPPER_ARGS:
			case GAMEMTL_PASS3_STAGE0_MAPPER_ARGS:
				len = iload->CurChunkLength();
				passindex = id - GAMEMTL_PASS0_STAGE0_MAPPER_ARGS;
				tmpstring = Get_Mapping_Arg_Buffer(passindex, 0, len);
				res = iload->Read(tmpstring, len, &nb);
				break;

			case GAMEMTL_PASS0_STAGE1_MAPPER_ARGS:
			case GAMEMTL_PASS1_STAGE1_MAPPER_ARGS:
			case GAMEMTL_PASS2_STAGE1_MAPPER_ARGS:
			case GAMEMTL_PASS3_STAGE1_MAPPER_ARGS:
				len = iload->CurChunkLength();
				passindex = id - GAMEMTL_PASS0_STAGE1_MAPPER_ARGS;
				tmpstring = Get_Mapping_Arg_Buffer(passindex, 1, len);
				res = iload->Read(tmpstring, len, &nb);
				break;
			
			/*
			** All chunks below here are for the obsolete material plugin...
			*/
			case GAMEMTL_GAMEFLAGS_CHUNK:
				break;

			case GAMEMTL_ATTRIBUTES_CHUNK:
				res = iload->Read(&lc->Attributes,sizeof(lc->Attributes),&nb);
				lc->IsOld = true;
				break;

			case GAMEMTL_COLORS_CHUNK:
				res = iload->Read(&lc->Diffuse,sizeof(lc->Diffuse),&nb);
				lc->IsOld = true;
				break;

			case GAMEMTL_DCT_FRAMES_CHUNK:
				res = iload->Read(&tmp16,sizeof(unsigned short),&nb);
				lc->DCTFrames = tmp16;
				lc->IsOld = true;
				break;

			case GAMEMTL_DCT_FRAME_RATE_CHUNK:
				res = iload->Read(&tmpfloat,sizeof(float),&nb);
				lc->DCTFrameRate = tmpfloat;
				lc->IsOld = true;
				break;

			case GAMEMTL_DIT_FRAMES_CHUNK:
				res = iload->Read(&tmp16,sizeof(unsigned short),&nb);
				lc->DITFrames = tmp16;
				lc->IsOld = true;
				break;

			case GAMEMTL_DIT_FRAME_RATE_CHUNK:
				res = iload->Read(&tmpfloat,sizeof(float),&nb);
				lc->DITFrameRate = tmpfloat;
				lc->IsOld = true;
				break;
			
			case GAMEMTL_SCT_FRAMES_CHUNK:
				res = iload->Read(&tmp16,sizeof(unsigned short),&nb);
				lc->SCTFrames = tmp16;
				lc->IsOld = true;
				break;
			
			case GAMEMTL_SCT_FRAME_RATE_CHUNK:
				res = iload->Read(&tmpfloat,sizeof(float),&nb);
				lc->SCTFrameRate = tmpfloat;
				lc->IsOld = true;
				break;

			case GAMEMTL_SIT_FRAMES_CHUNK:
				res = iload->Read(&tmp16,sizeof(unsigned short),&nb);
				lc->SITFrames = tmp16;
				lc->IsOld = true;
				break;
			
			case GAMEMTL_SIT_FRAME_RATE_CHUNK:
				res = iload->Read(&tmpfloat,sizeof(float),&nb);
				lc->SITFrameRate = tmpfloat;
				lc->IsOld = true;
				break;

			case GAMEMTL_DCT_MAPPING_CHUNK:
				res = iload->Read(&tmp8,sizeof(unsigned char),&nb);
				lc->DCTMappingType = tmp8;
				lc->IsOld = true;
				break;

			case GAMEMTL_DIT_MAPPING_CHUNK:
				res = iload->Read(&tmp8,sizeof(unsigned char),&nb);
				lc->DITMappingType = tmp8;
				lc->IsOld = true;
				break;

			case GAMEMTL_SCT_MAPPING_CHUNK:
				res = iload->Read(&tmp8,sizeof(unsigned char),&nb);
				lc->SCTMappingType = tmp8;
				lc->IsOld = true;
				break;

			case GAMEMTL_SIT_MAPPING_CHUNK:
				res = iload->Read(&tmp8,sizeof(unsigned char),&nb);
				lc->SITMappingType = tmp8;
				lc->IsOld = true;
				break;

			case GAMEMTL_DIFFUSE_COLOR_CHUNK:
				res = iload->Read(&lc->Diffuse,sizeof(lc->Diffuse),&nb);
				lc->IsOld = true;
				break;

			case GAMEMTL_SPECULAR_COLOR_CHUNK:
				res = iload->Read(&lc->Specular,sizeof(lc->Specular),&nb);
				lc->IsOld = true;
				break;

			case GAMEMTL_AMBIENT_COEFF_CHUNK:
				res = iload->Read(&lc->AmbientCoeff,sizeof(lc->AmbientCoeff),&nb);
				lc->IsOld = true;
				break;

			case GAMEMTL_DIFFUSE_COEFF_CHUNK:
				res = iload->Read(&lc->DiffuseCoeff,sizeof(lc->DiffuseCoeff),&nb);
				lc->IsOld = true;
				break;

			case GAMEMTL_SPECULAR_COEFF_CHUNK:
				res = iload->Read(&lc->SpecularCoeff,sizeof(lc->SpecularCoeff),&nb);
				lc->IsOld = true;
				break;

			case GAMEMTL_EMISSIVE_COEFF_CHUNK:
				res = iload->Read(&lc->EmissiveCoeff,sizeof(lc->EmissiveCoeff),&nb);
				lc->IsOld = true;
				break;
			
			case GAMEMTL_OPACITY_CHUNK:
				res = iload->Read(&lc->Opacity,sizeof(lc->Opacity),&nb);
				lc->IsOld = true;
				break;

			case GAMEMTL_TRANSLUCENCY_CHUNK:
				res = iload->Read(&lc->Translucency,sizeof(lc->Translucency),&nb);
				lc->IsOld = true;
				break;

			case GAMEMTL_SHININESS_CHUNK:
				res = iload->Read(&lc->Shininess,sizeof(lc->Shininess),&nb);
				lc->IsOld = true;
				break;

			case GAMEMTL_FOG_CHUNK:
				res = iload->Read(&lc->FogCoeff,sizeof(lc->FogCoeff),&nb);
				lc->IsOld = true;
				break;

			default:
				break;
		}

		iload->CloseChunk();
		if (res!=IO_OK) { 
			return res;
		}
	}
	
	return IO_OK;
}


/***********************************************************************************************
 * GameMtl::Save -- Saving into a MAX file                                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
IOResult GameMtl::Save(ISave *isave) 
{
	IOResult res;
	ULONG nb;

	/*
	** Save the base class stuff
	*/
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) 
		return res;
	isave->EndChunk();

	/*
	** Save the flags
	*/
	isave->BeginChunk(GAMEMTL_FLAGS_CHUNK);
	isave->Write(&Flags,sizeof(Flags),&nb);			
	isave->EndChunk();

	/*
	** Save the "cur-pages"
	*/
	uint8 tmp8;
	int pass;
	for (pass=0; pass < W3dMaterialClass::MAX_PASSES; pass++) {
		isave->BeginChunk(GAMEMTL_PASS0_CUR_PAGE + pass);
		tmp8 = CurPage[pass];
		isave->Write(&tmp8,sizeof(tmp8),&nb);
		isave->EndChunk();
	}
	
	/*
	** Save any Mapper Args
	*/
	for (pass=0; pass < W3dMaterialClass::MAX_PASSES; pass++) {
		char *buffer = Get_Mapping_Arg_Buffer(pass, 0);
		if (buffer) {
			isave->BeginChunk(GAMEMTL_PASS0_STAGE0_MAPPER_ARGS + pass);
			isave->Write(buffer, strlen(buffer) + 1, &nb);
			isave->EndChunk();
		}
		buffer = Get_Mapping_Arg_Buffer(pass, 1);
		if (buffer) {
			isave->BeginChunk(GAMEMTL_PASS0_STAGE1_MAPPER_ARGS + pass);
			isave->Write(buffer, strlen(buffer) + 1, &nb);
			isave->EndChunk();
		}
	}

	/*
	** Save the surface type
	*/
	isave->BeginChunk(GAMEMTL_SURFACE_TYPE_CHUNK);
	isave->Write(&SurfaceType,sizeof(SurfaceType),&nb);			
	isave->EndChunk();

	/*
	** Save the sort level
	*/
	isave->BeginChunk(GAMEMTL_SORT_LEVEL_CHUNK);
	isave->Write(&SortLevel,sizeof(SortLevel),&nb);
	isave->EndChunk();
	
	return IO_OK;
}


/***********************************************************************************************
 * GameMtl::Shade -- evaluate the material for the renderer.                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * This function could use a little work.  It doesn't always produce correct results.          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtl::Shade(ShadeContext& sc) 
{
	if (ShaderType == STE_PS2_SHADER) {
		ps2_shade(sc);
		return;
	}

	if (gbufID) sc.SetGBufferID(gbufID);

	/*
	** Shadowing, return black
	*/
	if (sc.mode==SCMODE_SHADOW) {
		sc.out.t = Color(0,0,0);
		return;
	}

	/*
	** Render each pass, initialize dest with the current background
	*/
	Color back_c;
	Color back_t;

	Color dest;
	Color src;
	float alpha;
	
	sc.GetBGColor(back_c,back_t, FALSE);
	dest = back_c;

	for (int pass=0; pass < Get_Pass_Count(); pass++) {

		/*
		** Computing the Primary and Secondary Gradients
		*/
		Color ambient = sc.ambientLight;
		Color diffuse(0,0,0);
		Color specular(0,0,0);

		for(int light_index = 0; light_index < sc.nLights; light_index++) {

			Color light_color;
			Point3 light_dir;
			float dot_nl;
			float diffuse_coef;

			LightDesc * light = sc.Light(light_index);
			if(light->Illuminate(sc, sc.Normal(), light_color, light_dir, dot_nl, diffuse_coef)) {
				
				//ambient += light_color;
				if (dot_nl > 0.0f) diffuse += dot_nl * light_color;
				
				float c = DotProd(light_dir, sc.ReflectVector());
				if(c > 0.f) {
					specular += (float)pow(c, Get_Shininess(pass, sc.CurTime())) * light_color;
				}
			}
		}
		
		ambient = ambient * Get_Ambient(pass,sc.CurTime());
		diffuse = diffuse * Get_Diffuse(pass,sc.CurTime());
		specular = specular * Get_Specular(pass,sc.CurTime());

		Color pri_gradient = ambient + diffuse + Get_Emissive(pass,sc.CurTime());
		if (pri_gradient.r > 1.0f) pri_gradient.r = 1.0f;
		if (pri_gradient.g > 1.0f) pri_gradient.g = 1.0f;
		if (pri_gradient.b > 1.0f) pri_gradient.b = 1.0f;

		Color sec_gradient = specular * Get_Specular(pass,sc.CurTime());
	
		/*
		** Sampling the Texture(s)
		*/
		AColor texel(1,1,1,1);
		if (Get_Texture_Enable(pass,0) && Texture[pass][0]) {
//			if (Get_Mapping_Type(pass) == GAMEMTL_MAPPING_UV) {
				texel = Texture[pass][0]->EvalColor(sc);
//			}
		}

		if (Get_Texture_Enable(pass,1) && Texture[pass][1]) {
			AColor detail_texel = Texture[pass][1]->EvalColor(sc);
			switch (Get_Detail_Color_Func(pass)) 
			{
				case W3DSHADER_DETAILCOLORFUNC_DISABLE:		
					break;
				case W3DSHADER_DETAILCOLORFUNC_DETAIL:			
					texel = detail_texel; 
					break;
				case W3DSHADER_DETAILCOLORFUNC_SCALE:			
					texel.r = detail_texel.r * texel.r;
					texel.g = detail_texel.g * texel.g;
					texel.b = detail_texel.b * texel.b;
					break;
				case W3DSHADER_DETAILCOLORFUNC_INVSCALE:
					texel.r = texel.r + (1.0f - texel.r) * detail_texel.r;
					texel.g = texel.g + (1.0f - texel.g) * detail_texel.g;
					texel.b = texel.b + (1.0f - texel.b) * detail_texel.b;
					break;
				case W3DSHADER_DETAILCOLORFUNC_ADD:
					texel.r = detail_texel.r + texel.r;
					texel.g = detail_texel.g + texel.g;
					texel.b = detail_texel.b + texel.b;
					break;
				case W3DSHADER_DETAILCOLORFUNC_SUB:			
					texel.r = texel.r - detail_texel.r;
					texel.g = texel.g - detail_texel.g;
					texel.b = texel.b - detail_texel.b;
					break;
				case W3DSHADER_DETAILCOLORFUNC_SUBR:
					texel.r = detail_texel.r - texel.r;
					texel.g = detail_texel.g - texel.g;
					texel.b = detail_texel.b - texel.b;
					break;
				case W3DSHADER_DETAILCOLORFUNC_BLEND:
					texel.r = (texel.a * texel.r) + ((1.0f - texel.a)*detail_texel.r);
					texel.g = (texel.a * texel.g) + ((1.0f - texel.a)*detail_texel.g);
					texel.b = (texel.a * texel.b) + ((1.0f - texel.a)*detail_texel.b);
					break;
				case W3DSHADER_DETAILCOLORFUNC_DETAILBLEND:
					texel.r = (detail_texel.a * texel.r) + ((1.0f - detail_texel.a)*detail_texel.r);
					texel.g = (detail_texel.a * texel.g) + ((1.0f - detail_texel.a)*detail_texel.g);
					texel.b = (detail_texel.a * texel.b) + ((1.0f - detail_texel.a)*detail_texel.b);
					break;
			}
			switch (Get_Detail_Alpha_Func(pass)) 
			{
				case W3DSHADER_DETAILALPHAFUNC_DISABLE:		
					break;
				case W3DSHADER_DETAILALPHAFUNC_DETAIL:
					texel.a = detail_texel.a;
					break;
				case W3DSHADER_DETAILALPHAFUNC_SCALE:
					texel.a = texel.a * detail_texel.a;
					break;
				case W3DSHADER_DETAILALPHAFUNC_INVSCALE:
					texel.a = texel.a + (1.0f - texel.a) * detail_texel.a;
					break;
			}
		}

		/*
		** Shader parameters define combination...
		*/
		src.r = texel.r;
		src.g = texel.g;
		src.b = texel.b;
		alpha = texel.a * Get_Opacity(pass,sc.CurTime());

		switch (Get_Pri_Gradient(pass)) 
		{
			case W3DSHADER_PRIGRADIENT_DISABLE:		break;
			case W3DSHADER_PRIGRADIENT_MODULATE:	src = src * pri_gradient; break;
			case W3DSHADER_PRIGRADIENT_ADD:			src = src + pri_gradient; break;
		}

		switch (Get_Sec_Gradient(pass)) 
		{
			case W3DSHADER_SECGRADIENT_DISABLE:		break;
			case W3DSHADER_SECGRADIENT_ENABLE:		src = src + sec_gradient; break;
		}

		if (src.r > 1.0f) src.r = 1.0f;
		if (src.g > 1.0f) src.g = 1.0f;
		if (src.b > 1.0f) src.b = 1.0f;

		if (src.r < 0.0f) src.r = 0.0f;
		if (src.g < 0.0f) src.g = 0.0f;
		if (src.b < 0.0f) src.b = 0.0f;

		Color dest_blend;
		switch (Get_Dest_Blend(pass))
		{
			case W3DSHADER_DESTBLENDFUNC_ZERO:						dest_blend = Color(0,0,0);	break;
			case W3DSHADER_DESTBLENDFUNC_ONE:						dest_blend = Color(1,1,1); break;
			case W3DSHADER_DESTBLENDFUNC_SRC_COLOR:				dest_blend = src;	break;
			case W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_COLOR:	dest_blend = Color(1.0f-src.r,1.0f-src.g, 1.0f-src.b); break;
			case W3DSHADER_DESTBLENDFUNC_SRC_ALPHA:				dest_blend = Color(alpha,alpha,alpha);	break;
			case W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA:	dest_blend = Color(1.0f-alpha,1.0f-alpha,1.0f-alpha); break;
			case W3DSHADER_DESTBLENDFUNC_SRC_COLOR_PREFOG:		dest_blend = src;	break;
		}
		
		Color src_blend;
		switch (Get_Src_Blend(pass))
		{
			case W3DSHADER_SRCBLENDFUNC_ZERO:						src_blend = Color(0,0,0); break;
			case W3DSHADER_SRCBLENDFUNC_ONE:							src_blend = Color(1,1,1); break;
			case W3DSHADER_SRCBLENDFUNC_SRC_ALPHA:					src_blend = Color(alpha,alpha,alpha); break;
			case W3DSHADER_SRCBLENDFUNC_ONE_MINUS_SRC_ALPHA:	src_blend = Color(1.0f-alpha,1.0f-alpha,1.0f-alpha); break;
		}
		
		src = scale(src_blend,src) + scale(dest_blend,dest);

		/*
		** Dest becomes Src and we repeat!
		*/
		dest = src;

	}

	sc.out.t = Color(0.0f, 0.0f, 0.0f);
	sc.out.c = dest;

}


/***********************************************************************************************
 * GameMtl::ps2_shade -- Emulate the PS2 shader.                                               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/12/1999MLL: Created.                                                                   *
 *=============================================================================================*/
void GameMtl::ps2_shade(ShadeContext& sc)
{
	if (gbufID) sc.SetGBufferID(gbufID);

	/*
	** Shadowing, return black
	*/
	if (sc.mode == SCMODE_SHADOW) {
		sc.out.t = Color(0,0,0);
		return;
	}

	/*
	** Render each pass, initialize dest with the current background
	*/

	// Background transparency. ???
	Color back_t;
	Color back_c;

	AColor dest;
	AColor src(1,1,1,1);
	
	sc.GetBGColor(back_c, back_t, FALSE);

	dest.r = back_c.r;
	dest.g = back_c.g;
	dest.b = back_c.b;
	dest.a = 0.0f;

	for (int pass=0; pass < Get_Pass_Count(); pass++) {

		// Compute the gradients.
		Color ambient = sc.ambientLight;
		Color diffuse(0,0,0);
		Color specular(0,0,0);

		for (int light_index = 0; light_index < sc.nLights; light_index++) {

			Color light_color;
			Point3 light_dir;
			float dot_nl;
			float diffuse_coef;

			LightDesc * light = sc.Light(light_index);

			assert(light);
			if(light->Illuminate(sc, sc.Normal(), light_color, light_dir, dot_nl, diffuse_coef)) {
				
				if (dot_nl > 0.0f) diffuse += dot_nl * light_color;
				
				float c = DotProd(light_dir, sc.ReflectVector());
				if (c > 0.f) {
					specular += (float)pow(c, Get_Shininess(pass, sc.CurTime())) * light_color;
				}
			}
		}
		
		ambient = ambient * Get_Ambient(pass,sc.CurTime());
		diffuse = diffuse * Get_Diffuse(pass,sc.CurTime());
		specular = specular * Get_Specular(pass,sc.CurTime());

		Color gradient = ambient + diffuse + Get_Emissive(pass,sc.CurTime());
		AColor pri_gradient;
		pri_gradient.r = gradient.r;
		pri_gradient.g = gradient.g;
		pri_gradient.b = gradient.b;
		pri_gradient.a = 1.0f;
		if (pri_gradient.r > 1.0f) pri_gradient.r = 1.0f;
		if (pri_gradient.g > 1.0f) pri_gradient.g = 1.0f;
		if (pri_gradient.b > 1.0f) pri_gradient.b = 1.0f;

		/*
		** Sampling the Texture(s)
		*/
		if (Get_Texture_Enable(pass,0) && Texture[pass][0]) {
			src = Texture[pass][0]->EvalColor(sc);
		}


		/*
		** Shader parameters define combination...
		*/
		src.a *= Get_Opacity(pass,sc.CurTime());

		switch (Get_Pri_Gradient(pass)) 
		{
			case PSS_PRIGRADIENT_MODULATE:
				src = src * pri_gradient;
				break;
			case PSS_PRIGRADIENT_HIGHLIGHT:
				src = src * pri_gradient;
				src += back_t; // ???
				break;
			case PSS_PRIGRADIENT_HIGHLIGHT2:
				src = src * pri_gradient;
				src += back_t; // ???
				break;
		}

		if (src.r > 1.0f) src.r = 1.0f;
		if (src.g > 1.0f) src.g = 1.0f;
		if (src.b > 1.0f) src.b = 1.0f;
		if (src.b > 1.0f) src.a = 1.0f;

		if (src.r < 0.0f) src.r = 0.0f;
		if (src.g < 0.0f) src.g = 0.0f;
		if (src.b < 0.0f) src.b = 0.0f;
		if (src.a < 0.0f) src.a = 0.0f;

		AColor param_a;
		AColor param_b;
		AColor param_c;
		AColor param_d;
		int a_value = Get_PS2_Shader_Param_A(pass);
		int b_value = Get_PS2_Shader_Param_B(pass);
		int c_value = Get_PS2_Shader_Param_C(pass);
		int d_value = Get_PS2_Shader_Param_D(pass);

		switch (a_value)
		{
			case PSS_SRC:
				param_a = src;
				break;
			case PSS_DEST:
				param_a = dest;
				break;
			case PSS_ZERO:
				param_a = AColor(0, 0, 0, 0);
				break;
		}

		switch (b_value)
		{
			case PSS_SRC:
				param_b = src;
				break;
			case PSS_DEST:
				param_b = dest;
				break;
			case PSS_ZERO:
				param_b = AColor(0, 0, 0, 0);
				break;
		}

		switch (c_value)
		{
			case PSS_SRC_ALPHA:
				param_c = AColor(src.a, src.a, src.a, src.a);
				break;
			case PSS_DEST_ALPHA:
				param_c = back_t; // ???
				break;
			case PSS_ONE:
				param_c = AColor(1, 1, 1, 1);
				break;
		}

		switch (d_value)
		{
			case PSS_SRC:
				param_d = src;
				break;
			case PSS_DEST:
				param_d = dest;
				break;
			case PSS_ZERO:
				param_d = AColor(0, 0, 0);
				break;
		}

		src = scale((param_a - param_b), param_c) + param_d;

		if (src.r > 1.0f) src.r = 1.0f;
		if (src.g > 1.0f) src.g = 1.0f;
		if (src.b > 1.0f) src.b = 1.0f;
		if (src.b > 1.0f) src.a = 1.0f;

		if (src.r < 0.0f) src.r = 0.0f;
		if (src.g < 0.0f) src.g = 0.0f;
		if (src.b < 0.0f) src.b = 0.0f;
		if (src.a < 0.0f) src.a = 0.0f;

		/*
		** Dest becomes Src and we repeat!
		*/
		dest = src;
	}

	sc.out.t = Color(0.0f, 0.0f, 0.0f);
	sc.out.c.r = dest.r;
	sc.out.c.g = dest.g;
	sc.out.c.b = dest.b;


}

// PS2 equation paramaters.  
// They are set to primes to avoid multiple equation solutions.
enum Shader_Translation {
	ST_ZERO,
	ST_ONE,
	ST_SRC = 7,
	ST_DEST = 31,
	ST_SRC_ALPHA = 101,
	ST_DEST_ALPHA = 2999,
};

/***********************************************************************************************
 * GameMtl::Compute_PC_Shader_From_PS2_Shader -- Determine if a PC shader can be created.      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/13/1999MLL: Created.                                                                   *
 *=============================================================================================*/
int GameMtl::Compute_PC_Shader_From_PS2_Shader(int pass)
{
	int param_value[4];

	// These match the PC equation paramaters.
	static const int src_blend[4] = {
		ST_ZERO, 
		ST_ONE, 
		ST_SRC_ALPHA, 
		1 - ST_SRC_ALPHA
	};
	static const int dest_blend[6] = {
		ST_ZERO,
		ST_ONE,
		ST_SRC,
		1 - ST_SRC,
		ST_SRC_ALPHA,
		1 - ST_SRC_ALPHA
	};

	int i = 0;
	int j = 0;
	int equation_value = 0;
	
	// Get the PS2 shader values.
	param_value[0] = Get_PS2_Shader_Param_A(pass);
	param_value[1] = Get_PS2_Shader_Param_B(pass);
	param_value[2] = Get_PS2_Shader_Param_D(pass);
	param_value[3] = Get_PS2_Shader_Param_C(pass);

	// Convert them to the enumeration.
	for (i = 0; i < 3; i++)	{
		switch(param_value[i]) 
		{
			case PSS_SRC:
				param_value[i] = ST_SRC;
				break;
			case PSS_DEST:
				param_value[i] = ST_DEST;
				break;
			case PSS_ZERO:
				param_value[i] = ST_ZERO;
				break;
		}
	}

	// The alpha paramater.
	switch(param_value[3]) 
	{
			case PSS_SRC_ALPHA:
				param_value[3] = ST_SRC_ALPHA;
				break;
			case PSS_DEST_ALPHA:
				param_value[3] = ST_DEST_ALPHA;
				break;
			case PSS_ONE:
				param_value[3] = ST_ONE;
				break;
	}

	// Calculate the PS2 shader.
	equation_value = ((param_value[0] - param_value[1]) * param_value[3]) + param_value[2];

	for (i = 0; i < 4; i++)	{
		for (j = 0; j < 6; j++)	{
			// Calculate the PC shader.  If equal, we have found a conversion.
			if (((src_blend[i] * ST_SRC) + (dest_blend[j] * ST_DEST)) == equation_value) {
				break;
			}
		}

		if (j != 6)	{
			break;
		}
	}

	if ((j == 6) && (i == 4)) {
		// Set the W3D shader to opaque.
		Set_Dest_Blend(pass, W3DSHADER_DESTBLENDFUNC_ZERO);
		Set_Src_Blend(pass, W3DSHADER_SRCBLENDFUNC_ONE);

		// No matches.
		return (FALSE);
	}

	// Set the PC shader to an equivalant of the PS2 shader.
	switch (dest_blend[j]) 
	{
		case ST_ZERO:
			Set_Dest_Blend(pass, W3DSHADER_DESTBLENDFUNC_ZERO);
			break;
		case ST_ONE:
			Set_Dest_Blend(pass, W3DSHADER_DESTBLENDFUNC_ONE);
			break;
		case ST_SRC:
			Set_Dest_Blend(pass, W3DSHADER_DESTBLENDFUNC_SRC_COLOR);
			break;
		case (1 - ST_SRC):
			Set_Dest_Blend(pass, W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_COLOR);
			break;
		case ST_SRC_ALPHA:
			Set_Dest_Blend(pass, W3DSHADER_DESTBLENDFUNC_SRC_ALPHA);
			break;
		case (1 - ST_SRC_ALPHA):
			Set_Dest_Blend(pass, W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA);
			break;
	}

	switch (src_blend[i]) 
	{
		case ST_ZERO:						
			Set_Src_Blend(pass, W3DSHADER_SRCBLENDFUNC_ZERO);
			break;
		case ST_ONE:						
			Set_Src_Blend(pass, W3DSHADER_SRCBLENDFUNC_ONE);
			break;
		case ST_SRC_ALPHA:						
			Set_Src_Blend(pass, W3DSHADER_SRCBLENDFUNC_SRC_ALPHA);
			break;
		case (1 - ST_SRC_ALPHA):						
			Set_Src_Blend(pass, W3DSHADER_SRCBLENDFUNC_ONE_MINUS_SRC_ALPHA);
			break;
	}

	// A match was made.
	return (TRUE);
}


/***********************************************************************************************
 * GameMtl::Compute_PS2_Shader_From_PC_Shader -- Change a W3D material to a PS2 W3D material.  *
W3DSHADER_PRIGRADIENT_ *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1999MLL: Created.                                                                   *
 *=============================================================================================*/
int GameMtl::Compute_PS2_Shader_From_PC_Shader(int pass)
{
  
	// These match the PC equation paramaters.
	static const int a_blend[3] = {
		ST_ZERO, 
		ST_SRC, 
		ST_DEST, 
	};

	static const int b_blend[3] = {
		ST_ZERO, 
		ST_SRC, 
		ST_DEST, 
	};

	static const int d_blend[3] = {
		ST_ZERO, 
		ST_SRC, 
		ST_DEST, 
	};

	static const int c_blend[6] = {
		ST_ONE,
		ST_SRC_ALPHA,
		ST_DEST_ALPHA,
	};

	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	int equation_value = 0;
	int param_value[2];
	
	// Get the PC shader values.
	param_value[0] = Get_Src_Blend(pass);
	param_value[1] = Get_Dest_Blend(pass);

	switch (param_value[0]) 
	{
		case W3DSHADER_SRCBLENDFUNC_ZERO: 
			param_value[0] = ST_ZERO;
			break;
		case W3DSHADER_SRCBLENDFUNC_ONE:
			param_value[0] = ST_ONE;
			break;
		case W3DSHADER_SRCBLENDFUNC_SRC_ALPHA:
			param_value[0] = ST_SRC_ALPHA;
			break;
		case W3DSHADER_SRCBLENDFUNC_ONE_MINUS_SRC_ALPHA:
			param_value[0] = 1 - ST_SRC_ALPHA;
			break;
	}

	// Convert them to the enumeration.
	switch (param_value[1]) 
	{
		case W3DSHADER_DESTBLENDFUNC_ZERO: 
			param_value[1] = ST_ZERO;
			break;
		case W3DSHADER_DESTBLENDFUNC_ONE:
			param_value[1] = ST_ONE;
			break;
		case W3DSHADER_DESTBLENDFUNC_SRC_COLOR:
			param_value[1] = ST_SRC;
			break;
		case W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_COLOR:
			param_value[1] = 1 - ST_SRC;
			break;
		case W3DSHADER_DESTBLENDFUNC_SRC_ALPHA:
			param_value[1] = ST_SRC_ALPHA;
			break;
		case W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA:
			param_value[1] = 1 - ST_SRC_ALPHA;
			break;
		case W3DSHADER_DESTBLENDFUNC_SRC_COLOR_PREFOG:
			assert(true);
			break;
	}


	// Calculate the PC shader.
	equation_value = param_value[0] * ST_SRC + param_value[1] * ST_DEST;

	// Set the shader to be a PS2 Shader.
	ShaderType = STE_PS2_SHADER;

	// Find a match for the depth compare test.
	switch (Get_Depth_Compare(pass))
	{
		case W3DSHADER_DEPTHCOMPARE_PASS_NEVER:
			Set_Depth_Compare(pass, PSS_DEPTHCOMPARE_PASS_NEVER);
			break;
		case W3DSHADER_DEPTHCOMPARE_PASS_LESS:			
			Set_Depth_Compare(pass, PSS_DEPTHCOMPARE_PASS_LESS);
			break;
		case W3DSHADER_DEPTHCOMPARE_PASS_LEQUAL:				
			Set_Depth_Compare(pass, PSS_DEPTHCOMPARE_PASS_LEQUAL);
			break;
		case W3DSHADER_DEPTHCOMPARE_PASS_ALWAYS:
			Set_Depth_Compare(pass, PSS_DEPTHCOMPARE_PASS_ALWAYS);
			break;
		default:
			Set_Depth_Compare(pass, PSS_DEPTHCOMPARE_PASS_LEQUAL);
			break;
	}

	// Only 1 gradient matches between the PC and PS2.
	switch (Get_Pri_Gradient(pass))
	{
		case W3DSHADER_PRIGRADIENT_MODULATE:
			Set_Pri_Gradient(pass, PSS_PRIGRADIENT_MODULATE);
			break;
		default:
			Set_Pri_Gradient(pass, PSS_PRIGRADIENT_MODULATE);
			break;
	}

	for (i = 0; i < 3; i++)	{
		for (j = 0; j < 3; j++)	{
			for (k = 0; k < 3; k++)	{
				for (l = 0; l < 3; l++)	{
					// Calculate the PS2 shader.  If equal, we have found a conversion.
					if (equation_value == ((a_blend[i] - b_blend[j]) * c_blend[k]) + d_blend[l]) {
						goto finished;
					}
				}
			}
		}
	}

 	// Set the PS2 W3D shader to opaque.
	Set_PS2_Shader_Param_A(pass, PSS_SRC);
	Set_PS2_Shader_Param_B(pass, PSS_ZERO);
	Set_PS2_Shader_Param_C(pass, PSS_ONE);
	Set_PS2_Shader_Param_D(pass, PSS_ZERO);
	Set_Depth_Mask(pass,W3DSHADER_DEPTHMASK_DEFAULT);

	// No matches.
	return (FALSE);

finished:
	// Set the PS2 shader to an equivalant of the PC shader.
	switch (a_blend[i]) 
	{
		case ST_ZERO:
			Set_PS2_Shader_Param_A(pass, PSS_ZERO);
			break;
		case ST_SRC:
			Set_PS2_Shader_Param_A(pass, PSS_SRC);
			break;
		case ST_DEST:
			Set_PS2_Shader_Param_A(pass, PSS_DEST);
			break;
	}

	switch (b_blend[j]) 
	{
		case ST_ZERO:
			Set_PS2_Shader_Param_B(pass, PSS_ZERO);
			break;
		case ST_SRC:
			Set_PS2_Shader_Param_B(pass, PSS_SRC);
			break;
		case ST_DEST:
			Set_PS2_Shader_Param_B(pass, PSS_DEST);
			break;
	}


	switch (d_blend[l]) 
	{
		case ST_ZERO:
			Set_PS2_Shader_Param_D(pass, PSS_ZERO);
			break;
		case ST_SRC:
			Set_PS2_Shader_Param_D(pass, PSS_SRC);
			break;
		case ST_DEST:
			Set_PS2_Shader_Param_D(pass, PSS_DEST);
			break;
	}

	switch (c_blend[k]) 
	{
		case ST_ONE:
			Set_PS2_Shader_Param_C(pass, PSS_ONE);
			break;
		case ST_SRC_ALPHA:
			Set_PS2_Shader_Param_C(pass, PSS_SRC_ALPHA);
			break;
		case ST_DEST_ALPHA:
			Set_PS2_Shader_Param_C(pass, PSS_DEST_ALPHA);
			break;
	}

	// A match was made.
	return (TRUE);
}

void GameMtl::Set_Pass_Count(int passcount)
{
	assert(MainParameterBlock);
	MainParameterBlock->SetValue(0, TimeValue(0), passcount);
}

int GameMtl::Get_Pass_Count(void)
{
	assert(MainParameterBlock);
	int pcount;
	MainParameterBlock->GetValue(0, TimeValue(0), pcount, FOREVER);
	return pcount;
}


Color	GameMtl::Get_Ambient(int pass,TimeValue t)
{
	Color val;	
	PassParameterBlock[pass]->GetValue(PB_AMBIENT,t,val,FOREVER);
	return val;
}
Color GameMtl::Get_Diffuse(int pass,TimeValue t)
{
	Color val;	
	PassParameterBlock[pass]->GetValue(PB_DIFFUSE,t,val,FOREVER);
	return val;
}
Color GameMtl::Get_Specular(int pass,TimeValue t)
{
	Color val;	
	PassParameterBlock[pass]->GetValue(PB_SPECULAR,t,val,FOREVER);
	return val;
}
Color GameMtl::Get_Emissive(int pass,TimeValue t)
{
	Color val;	
	PassParameterBlock[pass]->GetValue(PB_EMISSIVE,t,val,FOREVER);
	return val;
}
float GameMtl::Get_Shininess(int pass,TimeValue t)
{
	float val;
	PassParameterBlock[pass]->GetValue(PB_SHININESS,t,val,FOREVER);
	return val;
}
float GameMtl::Get_Opacity(int pass,TimeValue t)
{
	float val;
	PassParameterBlock[pass]->GetValue(PB_OPACITY,t,val,FOREVER);
	return val;
}
float GameMtl::Get_Translucency(int pass,TimeValue t)
{
	float val;
	PassParameterBlock[pass]->GetValue(PB_TRANSLUCENCY,t,val,FOREVER);
	return val;
}
int GameMtl::Get_Copy_Specular_To_Diffuse(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_COPY_SPECULAR_TO_DIFFUSE,0,val,FOREVER);
	return val;
}
int GameMtl::Get_Mapping_Type(int pass, int stage)
{
	int val = -1;
	if (stage == 0)
		PassParameterBlock[pass]->GetValue(PB_STAGE0_MAPPING_TYPE,0,val,FOREVER);
	else if (stage == 1)
		PassParameterBlock[pass]->GetValue(PB_STAGE1_MAPPING_TYPE,0,val,FOREVER);
	return val;
}
int GameMtl::Get_PSX_Translucency(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_PSX_TRANSLUCENCY,0,val,FOREVER);
	return val;
}
int GameMtl::Get_PSX_Lighting(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_PSX_LIGHTING,0,val,FOREVER);
	return val;
}
int GameMtl::Get_Depth_Compare(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_DEPTH_COMPARE,0,val,FOREVER);
	return val;
}
int GameMtl::Get_Depth_Mask(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_DEPTH_MASK,0,val,FOREVER);
	return val;
}
int GameMtl::Get_Alpha_Test(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_ALPHA_TEST,0,val,FOREVER);
	return val;
}
int GameMtl::Get_Dest_Blend(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_DEST_BLEND,0,val,FOREVER);
	return val;
}
int GameMtl::Get_Pri_Gradient(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_PRI_GRADIENT,0,val,FOREVER);
	return val;
}
int GameMtl::Get_Sec_Gradient(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_SEC_GRADIENT,0,val,FOREVER);
	return val;
}
int GameMtl::Get_Src_Blend(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_SRC_BLEND,0,val,FOREVER);
	return val;
}
int GameMtl::Get_Detail_Color_Func(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_DETAIL_COLOR_FUNC,0,val,FOREVER);
	return val;
}
int GameMtl::Get_Detail_Alpha_Func(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_DETAIL_ALPHA_FUNC,0,val,FOREVER);
	return val;
}
int GameMtl::Get_Texture_Enable(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_ENABLE,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_ENABLE,0,val,FOREVER);
	}
	return val;
}
int GameMtl::Get_Texture_Publish(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_PUBLISH,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_PUBLISH,0,val,FOREVER);
	}
	return val;
}
int GameMtl::Get_Texture_Resize(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_RESIZE,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_RESIZE,0,val,FOREVER);
	}
	return val;
}
int GameMtl::Get_Texture_No_Mipmap(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_NO_MIPMAP,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_NO_MIPMAP,0,val,FOREVER);
	}
	return val;
}
int GameMtl::Get_Texture_Clamp_U(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_CLAMP_U,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_CLAMP_U,0,val,FOREVER);
	}
	return val;
}
int GameMtl::Get_Texture_Clamp_V(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_CLAMP_V,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_CLAMP_V,0,val,FOREVER);
	}
	return val;
}
int GameMtl::Get_Texture_No_LOD(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_NO_LOD,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_NO_LOD,0,val,FOREVER);
	}
	return val;
}
int GameMtl::Get_Texture_Alpha_Bitmap(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_ALPHA_BITMAP,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_ALPHA_BITMAP,0,val,FOREVER);
	}
	return val;
}
int GameMtl::Get_Texture_Hint(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_HINT,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_HINT,0,val,FOREVER);
	}
	return val;
}
int GameMtl::Get_Texture_Display(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_DISPLAY,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_DISPLAY,0,val,FOREVER);
	}
	return val;
}
float GameMtl::Get_Texture_Frame_Rate(int pass,int stage)
{
	float val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_FRAME_RATE,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_FRAME_RATE,0,val,FOREVER);
	}
	return val;
}
int GameMtl::Get_Texture_Frame_Count(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_FRAME_COUNT,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_FRAME_COUNT,0,val,FOREVER);
	}
	return val;
}
int GameMtl::Get_Texture_Anim_Type(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_TEXTURE_ANIM_TYPE,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_TEXTURE_ANIM_TYPE,0,val,FOREVER);
	}
	return val;
}
Texmap * GameMtl::Get_Texture(int pass,int stage)
{ 
	return GetSubTexmap(pass_stage_to_texmap_index(pass,stage)); 
}

int GameMtl::Get_PS2_Shader_Param_A(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_PS2_SHADER_PARAM_A,0,val,FOREVER);
	return val;
}

int GameMtl::Get_PS2_Shader_Param_B(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_PS2_SHADER_PARAM_B,0,val,FOREVER);
	return val;
}

int GameMtl::Get_PS2_Shader_Param_C(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_PS2_SHADER_PARAM_C,0,val,FOREVER);
	return val;
}

int GameMtl::Get_PS2_Shader_Param_D(int pass)
{
	int val;
	PassParameterBlock[pass]->GetValue(PB_PS2_SHADER_PARAM_D,0,val,FOREVER);
	return val;
}

int GameMtl::Get_Map_Channel(int pass,int stage)
{
	int val;
	if (stage == 0) {
		PassParameterBlock[pass]->GetValue(PB_STAGE0_MAP_CHANNEL,0,val,FOREVER);
	} else {
		PassParameterBlock[pass]->GetValue(PB_STAGE1_MAP_CHANNEL,0,val,FOREVER);
	}
	return val;
}


void GameMtl::Set_Ambient(int pass,TimeValue t,Color val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_AMBIENT, t, val);
}
void GameMtl::Set_Diffuse(int pass,TimeValue t,Color val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_DIFFUSE, t, val);
}
void GameMtl::Set_Specular(int pass,TimeValue t,Color val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_SPECULAR, t, val);
}
void GameMtl::Set_Emissive(int pass,TimeValue t,Color val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_EMISSIVE, t, val);
}
void GameMtl::Set_Shininess(int pass,TimeValue t,float val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_SHININESS, t, val);
}
void GameMtl::Set_Opacity(int pass,TimeValue t,float val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_OPACITY, t, val);
}
void GameMtl::Set_Translucency(int pass,TimeValue t,float val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_TRANSLUCENCY, t, val);
}
void GameMtl::Set_Copy_Specular_To_Diffuse(int pass,bool val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_COPY_SPECULAR_TO_DIFFUSE, 0, val);
}
void GameMtl::Set_Mapping_Type(int pass,int stage,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0)
		PassParameterBlock[pass]->SetValue(PB_STAGE0_MAPPING_TYPE, 0, val);
	else if (stage == 1)
		PassParameterBlock[pass]->SetValue(PB_STAGE1_MAPPING_TYPE, 0, val);
}
void GameMtl::Set_PSX_Translucency(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_PSX_TRANSLUCENCY, 0, val);
}
void GameMtl::Set_PSX_Lighting(int pass,bool val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_PSX_LIGHTING, 0, val);
}
void GameMtl::Set_Depth_Compare(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_DEPTH_COMPARE, 0, val);
}
void GameMtl::Set_Depth_Mask(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_DEPTH_MASK, 0, val);
}
void GameMtl::Set_Alpha_Test(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_ALPHA_TEST, 0, val);
}
void GameMtl::Set_Dest_Blend(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_DEST_BLEND, 0, val);
}
void GameMtl::Set_Pri_Gradient(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_PRI_GRADIENT, 0, val);
}
void GameMtl::Set_Sec_Gradient(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_SEC_GRADIENT, 0, val);
}
void GameMtl::Set_Src_Blend(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_SRC_BLEND, 0, val);
}
void GameMtl::Set_Detail_Color_Func(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_DETAIL_COLOR_FUNC, 0, val);
}
void GameMtl::Set_Detail_Alpha_Func(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_DETAIL_ALPHA_FUNC, 0, val);
}
void GameMtl::Set_Texture_Enable(int pass,int stage,bool val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_ENABLE, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_ENABLE, 0, val);
	}
}
void GameMtl::Set_Texture_Publish(int pass,int stage,bool val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_PUBLISH, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_PUBLISH, 0, val);
	}
}
void GameMtl::Set_Texture_Resize(int pass,int stage,bool val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_RESIZE, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_RESIZE, 0, val);
	}
}
void GameMtl::Set_Texture_No_Mipmap(int pass,int stage,bool val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_NO_MIPMAP, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_NO_MIPMAP, 0, val);
	}
}
void GameMtl::Set_Texture_Clamp_U(int pass,int stage,bool val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_CLAMP_U, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_CLAMP_U, 0, val);
	}
}
void GameMtl::Set_Texture_Clamp_V(int pass,int stage,bool val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_CLAMP_V, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_CLAMP_V, 0, val);
	}
}
void GameMtl::Set_Texture_No_LOD(int pass,int stage,bool val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_NO_LOD, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_NO_LOD, 0, val);
	}
}
void GameMtl::Set_Texture_Alpha_Bitmap(int pass,int stage,bool val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_ALPHA_BITMAP, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_ALPHA_BITMAP, 0, val);
	}
}
void GameMtl::Set_Texture_Hint(int pass,int stage,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_HINT, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_HINT, 0, val);
	}
}
void GameMtl::Set_Texture_Display(int pass,int stage,bool val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);

	// clear all tex display flags
	for (int pi = 0; pi<Get_Pass_Count(); pi++) {
		PassParameterBlock[pi]->SetValue(PB_STAGE0_TEXTURE_DISPLAY, 0, false);
		PassParameterBlock[pi]->SetValue(PB_STAGE1_TEXTURE_DISPLAY, 0, false);
	}
	
	// set the one we want	
	if (val == true) {
	
		if (stage == 0) {
			PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_DISPLAY, 0, val);
		} else {
			PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_DISPLAY, 0, val);
		}
		SetMtlFlag( MTL_TEX_DISPLAY_ENABLED, TRUE );
		SetActiveTexmap(Texture[pass][stage]);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	} else {
		
		SetMtlFlag( MTL_TEX_DISPLAY_ENABLED, FALSE );
		SetActiveTexmap(NULL);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	
	}
	
	// tell dialog to refresh...
	if (MaterialDialog) {
		MaterialDialog->ReloadDialog();
	}

	if (IsMultiMtl()) {

		// Loop through all sub materials of the multi-material.
		for (unsigned mi = 0; mi < NumSubMtls(); mi++) {

			// Only change those that are W3D materials.
			if (GetSubMtl(mi)->ClassID() == GameMaterialClassID) {
				int pass;

				for (pass = 0; pass < ((GameMtl*)(GetSubMtl(mi)))->Get_Pass_Count(); pass++) {

					if (((GameMtl*)(GetSubMtl(mi)))->Get_Texture_Enable(pass, stage)) {
						(GetSubMtl(mi))->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);
					}
				}
			}
		}
	}
}

void GameMtl::Set_Texture_Frame_Rate(int pass,int stage,float val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_FRAME_RATE, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_FRAME_RATE, 0, val);
	}
}
void GameMtl::Set_Texture_Frame_Count(int pass,int stage,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_FRAME_COUNT, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_FRAME_COUNT, 0, val);
	}
}
void GameMtl::Set_Texture_Anim_Type(int pass,int stage,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_TEXTURE_ANIM_TYPE, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_TEXTURE_ANIM_TYPE, 0, val);
	}
}

void GameMtl::Set_Texture(int pass,int stage,Texmap * tex)
{ 
	SetSubTexmap(pass_stage_to_texmap_index(pass,stage),tex); 
}

void GameMtl::Set_PS2_Shader_Param_A(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_PS2_SHADER_PARAM_A, 0, val);
}

void GameMtl::Set_PS2_Shader_Param_B(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_PS2_SHADER_PARAM_B, 0, val);
}

void GameMtl::Set_PS2_Shader_Param_C(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_PS2_SHADER_PARAM_C, 0, val);
}

void GameMtl::Set_PS2_Shader_Param_D(int pass,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	PassParameterBlock[pass]->SetValue(PB_PS2_SHADER_PARAM_D, 0, val);
}

void GameMtl::Set_Map_Channel(int pass,int stage,int val)
{
	assert((pass >=0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert(PassParameterBlock[pass]);
	if (stage == 0) {
		PassParameterBlock[pass]->SetValue(PB_STAGE0_MAP_CHANNEL, 0, val);
	} else {
		PassParameterBlock[pass]->SetValue(PB_STAGE1_MAP_CHANNEL, 0, val);
	}

	if (Texture[pass][stage] != NULL) {
		UVGen * uvgen = Texture[pass][stage]->GetTheUVGen();
		if (uvgen != NULL) {
			uvgen->SetMapChannel(val);
		}
	}
	 
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}


// This returns the mapping args string buffer for that pass (and stage) after
// assuring that it can contain a string of length 'len' (if len is 0 no
// resizing will be performed)..
char * GameMtl::Get_Mapping_Arg_Buffer(int pass, int stage, unsigned int len)
{
	assert(pass >= 0);
	assert(pass < W3dMaterialClass::MAX_PASSES);
	assert(stage >= 0);
	assert(stage < W3dMaterialClass::MAX_STAGES);

	if (MapperArgLen[pass][stage] < len) {
		MapperArgLen[pass][stage] = len + 10;	// New length
		char *temp = new char[MapperArgLen[pass][stage] + 1];
		if (MapperArg[pass][stage]) {
			assert(strlen(MapperArg[pass][stage]) <= MapperArgLen[pass][stage]);
			strcpy(temp, MapperArg[pass][stage]);
			delete [] (MapperArg[pass][stage]);
			MapperArg[pass][stage] = NULL;
		}

		MapperArg[pass][stage] = temp;
	}

	return MapperArg[pass][stage];
}

int GameMtl::pass_stage_to_texmap_index(int pass,int stage)
{
	assert((pass >= 0) && (pass < W3dMaterialClass::MAX_PASSES));
	assert((stage >= 0) && (stage < W3dMaterialClass::MAX_STAGES));
	return pass * W3dMaterialClass::MAX_STAGES + stage;
}

void GameMtl::texmap_index_to_pass_stage(int index,int * set_pass,int * set_stage)
{
	*set_pass = index / W3dMaterialClass::MAX_STAGES;
	*set_stage = index % W3dMaterialClass::MAX_STAGES;
}

float GameMtl::EvalDisplacement(ShadeContext& sc)
{
	float displacement = 0.0F;
	if (DisplacementMap != NULL) {
		displacement = DisplacementMap->EvalMono(sc);
		displacement = displacement * DisplacementAmt;
	}
	return displacement;
}

Interval GameMtl::DisplacementValidity(TimeValue t)
{
	return FOREVER;
}

void GameMtlPostLoad::proc(ILoad *iload)
{ 
	if (IsOld) {

		m->Reset();
		m->Set_Pass_Count(1);
		m->Set_Ambient(0,0,AmbientCoeff);
		m->Set_Diffuse(0,0,Diffuse * DiffuseCoeff);
		m->Set_Specular(0,0,Specular * SpecularCoeff);
		m->Set_Emissive(0,0,EmissiveCoeff);
		m->Set_Opacity(0,0,Opacity);
		m->Set_Translucency(0,0,Translucency);
		m->Set_Shininess(0,0,Shininess);
		m->Set_Mapping_Type(0,0,DCTMappingType);
		
		Texmap * tex = (*(m->Maps))[ID_DI].Map;
		
		if ((tex) && (tex->ClassID() == Class_ID(BMTEX_CLASS_ID,0))) {
			m->Set_Texture(0,0,tex);
			m->Set_Texture_Enable(0,0,true);
			m->Set_Texture_Frame_Rate(0,0,DCTFrameRate);
			m->Set_Texture_Frame_Count(0,0,DCTFrames);
		
			if (m->TestMtlFlag(MTL_TEX_DISPLAY_ENABLED)) {
				m->Set_Texture_Display(0,0,true);
			}
		}
	
		m->ReplaceReference(GameMtl::REF_MAPS,NULL);
	}

	// older material formats did not save the map channel and will default to zero, 
	// we need to change the map channel to one in this case.
	for (int pass = 0; pass < W3dMaterialClass::MAX_PASSES; pass++) {
		for (int stage = 0; stage < W3dMaterialClass::MAX_STAGES; stage++) {
			if ((m->Get_Map_Channel(pass,stage) < 1) || (m->Get_Map_Channel(pass,stage) > 99)) {
				m->Set_Map_Channel(pass,stage,1);
			}
		}
	}

	// Now that I've removed the UI for the RESIZE and NO_MIPMAP options in the texture
	// pane, we initialize the No_LOD setting to the existing NO_MIPMAP setting
	// NOTE: I created a new flag for gamemtl which gets set when this conversion takes
	// place for the first time.  
	if (m->Get_Flag(GAMEMTL_CONVERTED_TO_NOLOD) == false) {
		for (int pass = 0; pass < W3dMaterialClass::MAX_PASSES; pass++) {
			for (int stage = 0; stage < W3dMaterialClass::MAX_STAGES; stage++) {
				bool no_lod = m->Get_Texture_No_Mipmap(pass,stage) != 0;
				m->Set_Texture_No_LOD(pass,stage,no_lod);
			}
		}
		m->Set_Flag(GAMEMTL_CONVERTED_TO_NOLOD,true);
	}

	delete this; 
} 


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

// FILE: Snow.h /////////////////////////////////////////////////////////

#pragma once
#ifndef _SNOW_H_
#define _SNOW_H_

#include "Lib/BaseType.h"
#include "Common/SubsystemInterface.h"
#include "Common/Overridable.h"
#include "Common/Override.h"
#include "WWMath/vector3.h"
#include "WWMath/vector4.h"

//-------------------------------------------------------------------------------------------------
/** This structure keeps the transparency and vertex settings, which are the same regardless of the
		time of day. They can be overridden on a per-map basis. */
//-------------------------------------------------------------------------------------------------
class WeatherSetting : public Overridable
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WeatherSetting, "WeatherSetting"  )

	public:
		AsciiString m_snowTexture;
		Real	m_snowFrequencyScaleX;	///<used to adjust snow position.
		Real	m_snowFrequencyScaleY;	///<used to adjust snow position.
		Real	m_snowAmplitude;		///<used to adjust amount of of snow movement. (in world units)
		Real	m_snowPointSize;		///<used to control hardware point-sprite size. (in arbitrary units - see DX SDK Docs).
		Real	m_snowMaxPointSize;		///<used to control maximum size (in pixels) of point sprite.
		Real	m_snowMinPointSize;		///<used to control the minimum size (in piexels) of point sprite.
		Real	m_snowQuadSize;			///<used to control quad size when no hardware point sprites. (world width/height of quad)
		Real	m_snowBoxDimensions;	///<used to set dimensions of box surrounding camera. (world units)
		Real	m_snowBoxDensity;		///<used to control how many emitters are present per world unit
		Real	m_snowVelocity;			///<used to set speed at which snow falls (world units/sec).
		Bool    m_usePointSprites;		///<used to disable hardware point-sprite support.
		Bool	m_snowEnabled;			///<enable/disable snow on the map.

	public:
		WeatherSetting()
		{
			m_snowTexture = "EXSnowFlake.tga";
			m_snowFrequencyScaleX=0.0533f;
			m_snowFrequencyScaleY=0.0275f;
			m_snowAmplitude=5.0f;
			m_snowPointSize=1.0f;
			m_snowQuadSize=0.5f;
			m_snowBoxDimensions=200;
			m_snowBoxDensity=1;
			m_snowVelocity=4;
			m_usePointSprites=TRUE;
			m_snowEnabled=FALSE;
			m_snowMaxPointSize=64.0f;
			m_snowMinPointSize=0.0f;
		}

		static const FieldParse m_weatherSettingFieldParseTable[];		///< the parse table for INI definition

		/// Get the INI parsing table for loading
		const FieldParse *getFieldParse( void ) const { return m_weatherSettingFieldParseTable; }
};

EMPTY_DTOR(WeatherSetting)

extern OVERRIDE<WeatherSetting> TheWeatherSetting;

class SnowManager : public SubsystemInterface
{
  public :
	  enum{
		 SNOW_NOISE_X=64,			//dimensions table holding noise function used for initial snow positions.
		 SNOW_NOISE_Y=64,			//dimensions table holding noise function used for initial snow positions.
	  };

	 SnowManager(void);
	~SnowManager(void);

	virtual void init( void );
	virtual void reset( void );
	virtual void updateIniSettings (void);
	void setVisible(Bool showWeather);	///<enable/disable rendering of weather - assuming it's available on map.

  protected :

	Real				*m_startingHeights;
	Real				m_time;	///<time elapsed since it started snowing.
	Real				m_velocity;	///<positive velocity of falling snow
	Real				m_fullTimePeriod;	///<time for snow to complete a full animation cycle.
	Real				m_frequencyScaleX;	///<used to adjust snow position.
	Real				m_frequencyScaleY;	///<used to adjust snow position.
	Real				m_amplitude;		///<used to adjust amount of of snow movement.
	Real				m_pointSize;		///<used to control hardware point-sprite size.
	Real				m_maxPointSize;		///<used to control maximum pixel size of sprites.
	Real				m_minPointSize;		///<used to control minimum pixel size of sprites.
	Real				m_quadSize;			///<used to control quad size when no hardware point sprites.
	Real				m_boxDimensions;	///<used to set dimensions of box surrounding camera.
	Real				m_emitterSpacing;		///<used to control how many emitters are present per world unit
	Bool				m_isVisible;		///<used to prevent map weather (if defined) from rendering.
};

extern SnowManager *TheSnowManager;  ///< the ray effects singleton external

#endif // _SNOW_H_


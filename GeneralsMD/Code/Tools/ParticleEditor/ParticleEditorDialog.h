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

#pragma once

#include <map>			// for std::pair
#include <string>		// for std::string
#include <vector>		// for std::vector
#include <list>			// for std::list

#define		DEFINE_PARTICLE_SYSTEM_NAMES	1
#include "GameClient/ParticleSys.h"

#include "CColorAlphaDialog.h"
#include "CSwitchesDialog.h"
#include "MoreParmsDialog.h"

#define		FORMAT_STRING		"%.2f"
#define		NONE_STRING "(None)"
struct RGBColorKeyframe;
struct RandomKeyframe;
interface ISwapablePanel;

#define NUM_EMISSION_TYPES	5
#define NUM_VELOCITY_TYPES	5
#define NUM_PARTICLE_TYPES	3
#define NUM_SHADER_TYPES		3

enum SwitchType
{
	ST_HOLLOW = 0,
	ST_ONESHOT,
	ST_ALIGNXY,
	ST_EMITABOVEGROUNDONLY,
	ST_PARTICLEUPTOWARDSEMITTER,
};

class DebugWindowDialog : public CDialog
{
	public:
		enum {IDD = IDD_PSEd};
		DebugWindowDialog(UINT nIDTemplate = DebugWindowDialog::IDD, CWnd* pParentWnd = NULL);
		virtual ~DebugWindowDialog();

		void InitPanel( void );
		HWND GetMainWndHWND( void );
		void addParticleSystem( IN const char *particleSystem );
		void addThingTemplate( IN const char *thingTemplate );
		void clearAllParticleSystems( void );
		void clearAllThingTemplates( void );
		Bool hasSelectionChanged( void );
		void getSelectedSystemName( OUT char *bufferToCopyInto ) const;
		void getSelectedParticleAsciiStringParm( IN int parmNum, OUT char *bufferToCopyInto ) const;
		void updateParticleAsciiStringParm( IN int parmNum, IN const char *bufferToCopyFrom );
		void updateCurrentParticleCap( IN int particleCap );
		void updateCurrentNumParticles( IN int particleCount );
		int getNewParticleCap( void );

		void updateCurrentParticleSystem( IN ParticleSystemTemplate *particleTemplate );
		void updateSystemUseParameters( IN ParticleSystemTemplate *particleTemplate );
		void signalParticleSystemEdit( void );


		// The purpose of these is to add as few friends as possible to the particle system classes.
		// Therefore, this class has ALL the access to ParticleSystems, and dances on the data directly.
		// Child panels make calls here
		void getColorValueFromSystem( IN Int systemNum,
																  OUT RGBColorKeyframe &colorFrame ) const;

		void updateColorValueToSystem( IN Int systemNum,
																	 IN const RGBColorKeyframe &colorFrame );

		void getAlphaRangeFromSystem( IN Int systemNum,
																  OUT ParticleSystemInfo::RandomKeyframe &randomVar ) const;

		void updateAlphaRangeToSystem( IN Int systemNum,
																	 IN const ParticleSystemInfo::RandomKeyframe &randomVar );

		void getHalfSizeFromSystem( IN Int coordNum, // 0:X, 1:Y, 2:Z
																OUT Real& halfSize ) const;

		void updateHalfSizeToSystem( IN Int coordNum, // 0:X, 1:Y, 2:Z
																 IN const Real &halfSize );

		void getSphereRadiusFromSystem( OUT Real &radius ) const;
		void updateSphereRadiusToSystem( IN const Real &radius );

		void getCylinderRadiusFromSystem( OUT Real &radius ) const;
		void updateCylinderRadiusToSystem( IN const Real &radius );

		void getCylinderLengthFromSystem( OUT Real &length ) const;
		void updateCylinderLengthToSystem( IN const Real &length );

		void getLineFromSystem( IN Int coordNum, // 0:X1, 1:Y1, 2:Z1, 3:X2, 4:Y2, 5:Z2
														OUT Real& linePoint ) const;

		void updateLineToSystem( IN Int coordNum, // 0:X, 1:Y, 2:Z, 3:X2, 4:Y2, 5:Z2
														 IN const Real &linePoint );

		void getVelSphereFromSystem( IN Int velNum, // 0:min 1:max
																 OUT Real &radius ) const;

		void updateVelSphereToSystem( IN Int velNum, // 0:min 1:max
																	IN const Real &radius );

		void getVelHemisphereFromSystem( IN Int velNum, // 0:min 1:max
																		 OUT Real &radius ) const;

		void updateVelHemisphereToSystem( IN Int velNum, // 0:min 1:max
																			IN const Real &radius );

		void getVelOrthoFromSystem( IN Int coordNum, // 0:Xmin, 1:Ymin, 2:Zmin, 3:Xmax, 4:Ymax, 5:Zmax
																OUT Real& ortho ) const;

		void updateVelOrthoToSystem( IN Int coordNum, // 0:Xmin, 1:Ymin, 2:Zmin, 3:Xmax, 4:Ymax, 5:Zmax
																 IN const Real& ortho );

		void getVelCylinderFromSystem( IN Int coordNum, // 0:Radialmin, 1:Lengthmin, 2:Radialmax, 3:Lengthmax
																	 OUT Real& ortho ) const;

		void updateVelCylinderToSystem( IN Int coordNum, // 0:Radialmin, 1:Lengthmin, 2:Radialmax, 3:Lengthmax
																		IN const Real& ortho );

		void getVelOutwardFromSystem( IN Int coordNum, // 0:Outwardmin, 1:Othermin, 2:Outwardmax, 3:Othermax
																	OUT Real& ortho ) const;

		void updateVelOutwardToSystem( IN Int coordNum, // 0:Outwardmin, 1:Othermin, 2:Outwardmax, 3:Othermax
																	 IN const Real& ortho );

		void getParticleNameFromSystem( OUT char *buffer, IN int buffLen ) const;
		void updateParticleNameToSystem( IN const char *buffer );
		void getDrawableNameFromSystem( OUT char *buffer, IN int buffLen ) const;
		void updateDrawableNameToSystem( IN const char *buffer );

		void getInitialDelayFromSystem( IN Int parmNum, // 0:min, 1:min
																	OUT Real& initialDelay ) const;

		void updateInitialDelayToSystem( IN Int parmNum, // 0:min, 1:min
																	 IN const Real& initialDelay );

		void getBurstDelayFromSystem( IN Int parmNum, // 0:min, 1:min
																	OUT Real& burstDelay ) const;

		void updateBurstDelayToSystem( IN Int parmNum, // 0:min, 1:min
																	 IN const Real& burstDelay );

		void getBurstCountFromSystem( IN Int parmNum, // 0:min, 1:min
																	OUT Real& burstCount ) const;

		void updateBurstCountToSystem( IN Int parmNum, // 0:min, 1:min
																	 IN const Real& burstCount );

		void getColorScaleFromSystem( IN Int parmNum, // 0:min, 1:min
																	OUT Real& colorScale ) const;

		void updateColorScaleToSystem( IN Int parmNum, // 0:min, 1:min
																	 IN const Real& colorScale );

		void getParticleLifetimeFromSystem( IN Int parmNum, // 0:min, 1:min
																	OUT Real& particleLifetime ) const;

		void updateParticleLifetimeToSystem( IN Int parmNum, // 0:min, 1:min
																	 IN const Real& particleLifetime );

		void getParticleSizeFromSystem( IN Int parmNum, // 0:min, 1:min
																	OUT Real& particleSize ) const;

		void updateParticleSizeToSystem( IN Int parmNum, // 0:min, 1:min
																	 IN const Real& particleSize );

		void getStartSizeRateFromSystem( IN Int parmNum, // 0:min, 1:min
																	OUT Real& startSizeRate ) const;

		void updateStartSizeRateToSystem( IN Int parmNum, // 0:min, 1:min
																	 IN const Real& startSizeRate );

		void getSizeRateFromSystem( IN Int parmNum, // 0:min, 1:min
																	OUT Real& sizeRate ) const;

		void updateSizeRateToSystem( IN Int parmNum, // 0:min, 1:min
																	 IN const Real& sizeRate );

		void getSizeDampingFromSystem( IN Int parmNum, // 0:min, 1:min
																	OUT Real& sizeDamping ) const;

		void updateSizeDampingToSystem( IN Int parmNum, // 0:min, 1:min
																	 IN const Real& sizeDamping );

		void getSystemLifetimeFromSystem( OUT Real& systemLifetime ) const;

		void updateSystemLifetimeToSystem( IN const Real& systemLifetime );

		void getSlaveOffsetFromSystem( IN Int parmNum, // 0:min, 1:min
																	OUT Real& slaveOffset ) const;

		void updateSlaveOffsetToSystem( IN Int parmNum, // 0:min, 1:min
																	 IN const Real& slaveOffset );

		void getDriftVelocityFromSystem( IN Int parmNum, // 0:X, 1:Y, 2:Z
																	OUT Real& driftVelocity ) const;

		void updateDriftVelocityToSystem( IN Int parmNum, // 0:min, 1:min
																	 IN const Real& driftVelocity );

		void getSwitchFromSystem( IN SwitchType switchType,
															OUT Bool& switchVal ) const;

		void updateSwitchToSystem( IN SwitchType switchType,
															 IN const Bool& switchVal );

		void getSlaveSystemFromSystem( OUT char *buffer, IN Int bufferSize) const;
		void updateSlaveSystemToSystem( IN const char *buffer );

		void getPerParticleSystemFromSystem( OUT char *buffer, IN Int bufferSize) const;
		void updatePerParticleSystemToSystem( IN const char *buffer );

		void getWindMotionFromSystem( OUT ParticleSystemInfo::WindMotion& windMotion ) const;
		void updateWindMotionToSystem( IN const ParticleSystemInfo::WindMotion& windMotion );

		void getPingPongStartAngleFromSystem( IN Int parmNum, OUT Real& angle ) const;
		void updatePingPongStartAngleToSystem( IN Int parmNum, IN const Real& angle );

		void getPingPongEndAngleFromSystem( IN Int parmNum, OUT Real& angle ) const;
		void updatePingPongEndAngleToSystem( IN Int parmNum, IN const Real& angle );

		void getWindAngleChangeFromSystem( IN Int parmNum, OUT Real& angle ) const;
		void updateWindAngleChangeToSystem( IN Int parmNum, IN const Real& angle );

		Bool shouldWriteINI( void );
		Bool hasRequestedReload( void );
		Bool shouldBusyWait( void );
		Bool shouldUpdateParticleCap( void );
		Bool shouldReloadTextures( void );
		Bool shouldKillAllParticleSystems( void );



		const std::list<std::string> &getAllThingTemplates( void ) const { return m_listOfThingTemplates; }
		const std::list<std::string> &getAllParticleSystems( void ) const { return m_listOfParticleSystems; }

		ParticleSystemTemplate *getCurrentParticleSystem( void ) { return m_particleSystem; }

	protected:
		HWND			mMainWndHWND;
		Bool			m_changeHasOcurred;
		ParticleSystemTemplate	*m_particleSystem;
		std::list<std::string> m_listOfThingTemplates;
		std::vector<std::string> m_particleParmValues;
		std::list<std::string> m_listOfParticleSystems;

		Bool			m_shouldWriteINI;
		Bool			m_showColorDlg;
		Bool			m_showSwitchesDlg;
		Bool			m_showMoreParmsDlg;
		Bool			m_shouldReload;
		Bool			m_shouldBusyWait;
		Bool			m_shouldUpdateParticleCap;
		Bool			m_shouldReloadTextures;
		Bool			m_shouldKillAllParticleSystems;

		CColorAlphaDialog	m_colorAlphaDialog;
		CSwitchesDialog m_switchesDialog;
		MoreParmsDialog m_moreParmsDialog;


		int m_activeEmissionPage;
		int m_activeVelocityPage;
		int m_activeParticlePage;

		ISwapablePanel *m_emissionTypePanels[NUM_EMISSION_TYPES];
		ISwapablePanel *m_velocityTypePanels[NUM_VELOCITY_TYPES];
		ISwapablePanel *m_particleTypePanels[NUM_PARTICLE_TYPES];

		void appendParticleSystemToList( IN const std::string &rString );
		void appendThingTemplateToList( IN const std::string &rString );

		// if true, updates the UI from the Particle System.
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );


	protected:
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnClose();
		afx_msg void OnParticleSystemChange();	// the current particle system isn't the same as the previous system
		afx_msg void OnParticleSystemEdit();		// this system has been edited
		afx_msg void OnKillAllParticleSystems();	// kill all particle systems in the world
		afx_msg void OnEditColorAlpha();
		afx_msg void OnEditMoreParms();
		afx_msg void OnEditSwitches();
		afx_msg void OnPushSave();
		afx_msg void OnReloadTextures();
		afx_msg void OnReloadSystem();
		afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
		afx_msg void OnReloadCurrent();
		afx_msg void OnReloadAll();
		afx_msg void OnSaveCurrent();
		afx_msg void OnSaveAll();
		afx_msg void OnParticleCapEdit();
		DECLARE_MESSAGE_MAP()
};

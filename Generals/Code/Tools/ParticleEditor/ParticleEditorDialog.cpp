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

#include "StdAfx.h"
#include "Resource.h"
#include "ParticleEditorDialog.h"

#include "CParticleEditorPage.h"
#include "EmissionTypePanels.h"
#include "GameClient/ParticleSys.h"
#include "ParticleEditorExport.h"
#include "ParticleTypePanels.h"
#include "VelocityTypePanels.h"


#define		ARBITRARY_BUFF_SIZE		128

DebugWindowDialog::DebugWindowDialog(UINT nIDTemplate, CWnd* pParentWnd) : CDialog(nIDTemplate, pParentWnd),
m_colorAlphaDialog(CColorAlphaDialog::IDD, this),
m_switchesDialog(CSwitchesDialog::IDD, this),
m_moreParmsDialog(MoreParmsDialog::IDD, this)
{
	mMainWndHWND = ::FindWindow(nullptr, "Command & Conquer: Generals");
	m_activeEmissionPage = 0;
	m_activeVelocityPage = 0;
	m_activeParticlePage = 0;
	m_particleSystem = nullptr;


	m_changeHasOcurred = false;
	m_shouldWriteINI = false;
	m_showColorDlg = false;
	m_showSwitchesDlg = false;
	m_showMoreParmsDlg = false;
	m_shouldBusyWait = false;
	m_shouldReload = false;
	m_shouldUpdateParticleCap = false;
	m_shouldReloadTextures = false;
	m_shouldKillAllParticleSystems = false;

	m_emissionTypePanels[0] = new EmissionPanelPoint;
	m_emissionTypePanels[1] = new EmissionPanelLine;
	m_emissionTypePanels[2] = new EmissionPanelBox;
	m_emissionTypePanels[3] = new EmissionPanelSphere;
	m_emissionTypePanels[4] = new EmissionPanelCylinder;

	m_velocityTypePanels[0] = new VelocityPanelOrtho;
	m_velocityTypePanels[1] = new VelocityPanelSphere;
	m_velocityTypePanels[2] = new VelocityPanelHemisphere;
	m_velocityTypePanels[3] = new VelocityPanelCylinder;
	m_velocityTypePanels[4] = new VelocityPanelOutward;

	m_particleTypePanels[0] = new ParticlePanelParticle;
	m_particleTypePanels[1] = new ParticlePanelDrawable;
	m_particleTypePanels[2] = new ParticlePanelStreak;
	m_particleParmValues.resize(PARM_NumParms);
}

DebugWindowDialog::~DebugWindowDialog()
{
	int i;
	for (i = 0; i < NUM_EMISSION_TYPES; ++i) {
		delete m_emissionTypePanels[i];
	}

	for (i = 0; i < NUM_VELOCITY_TYPES; ++i) {
		delete m_velocityTypePanels[i];
	}

	for (i = 0; i < NUM_PARTICLE_TYPES; ++i) {
		delete m_particleTypePanels[i];
	}
}

int DebugWindowDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return CDialog::OnCreate(lpCreateStruct);
}

void DebugWindowDialog::InitPanel( void )
{
	CComboBox* pCombo;

	pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_Priority);
	if (pCombo) {
	  for (int i = 1; ParticlePriorityNames[i]; ++i) {
		  pCombo->AddString(ParticlePriorityNames[i]);
		}
		pCombo->SetCurSel(0);
	}

	pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_EmissionType);
	if (pCombo) {
		for (int i = 1; EmissionVolumeTypeNames[i]; ++i) {
			pCombo->AddString(EmissionVolumeTypeNames[i]);
		}
		pCombo->SetCurSel(0);
	}

	pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_VelocityType);
	if (pCombo) {
		for (int i = 1; EmissionVelocityTypeNames[i]; ++i) {
			pCombo->AddString(EmissionVelocityTypeNames[i]);
		}
		pCombo->SetCurSel(0);
	}

	pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_ParticleType);
	if (pCombo) {
		for (int i = 1; ParticleTypeNames[i]; ++i) {
			pCombo->AddString(ParticleTypeNames[i]);
		}
		pCombo->SetCurSel(0);
	}

	pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_ShaderType);
	if (pCombo) {
		for (int i = 1; ParticleShaderTypeNames[i]; ++i) {
			pCombo->AddString(ParticleShaderTypeNames[i]);
		}
		pCombo->SetCurSel(0);
	}

	m_colorAlphaDialog.Create(CColorAlphaDialog::IDD, this);
	m_colorAlphaDialog.InitPanel();

	m_switchesDialog.Create(CSwitchesDialog::IDD, this);
	m_switchesDialog.InitPanel();

	m_moreParmsDialog.Create(MoreParmsDialog::IDD, this);
	m_moreParmsDialog.InitPanel();

	CWnd *pWnd;
	int j;

	pWnd = GetDlgItem(IDC_PSEd_EmissionPanel);
	if (pWnd) {
		CRect rect;
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		for (j = 0; j < NUM_EMISSION_TYPES; ++j) {
			m_emissionTypePanels[j]->Create(m_emissionTypePanels[j]->GetIDD(), this);
			m_emissionTypePanels[j]->InitPanel();
			m_emissionTypePanels[j]->ShowWindow(SW_HIDE);
			m_emissionTypePanels[j]->SetWindowPos(nullptr, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
		}
		pWnd->ShowWindow(SW_HIDE);
		m_emissionTypePanels[0]->ShowWindow(SW_SHOW);
	}

	pWnd = GetDlgItem(IDC_PSEd_VelocityPanel);
	if (pWnd) {
		CRect rect;
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		for (j = 0; j < NUM_VELOCITY_TYPES; ++j) {
			m_velocityTypePanels[j]->Create(m_velocityTypePanels[j]->GetIDD(), this);
			m_velocityTypePanels[j]->InitPanel();
			m_velocityTypePanels[j]->ShowWindow(SW_HIDE);
			m_velocityTypePanels[j]->SetWindowPos(nullptr, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
		}
		pWnd->ShowWindow(SW_HIDE);
		m_velocityTypePanels[0]->ShowWindow(SW_SHOW);
	}

	pWnd = GetDlgItem(IDC_PSEd_ParticlePanel);
	if (pWnd) {
		CRect rect;
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		for (j = 0; j < NUM_PARTICLE_TYPES; ++j) {
			m_particleTypePanels[j]->Create(m_particleTypePanels[j]->GetIDD(), this);
			m_particleTypePanels[j]->InitPanel();
			m_particleTypePanels[j]->ShowWindow(SW_HIDE);
			m_particleTypePanels[j]->SetWindowPos(nullptr, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
		}
		pWnd->ShowWindow(SW_HIDE);
		m_particleTypePanels[0]->ShowWindow(SW_SHOW);
	}
}

HWND DebugWindowDialog::GetMainWndHWND(void)
{
	return mMainWndHWND;
}

void DebugWindowDialog::addParticleSystem(const char *particleSystem)
{
	if (!particleSystem) {
		return;
	}

	std::string particleSystemName = particleSystem;
	appendParticleSystemToList(particleSystemName);
}

void DebugWindowDialog::addThingTemplate(const char *thingTemplate)
{
	if (!thingTemplate) {
		return;
	}

	std::string thingTemplateName = thingTemplate;
	appendThingTemplateToList(thingTemplateName);
}

void DebugWindowDialog::OnClose()
{
	ShowWindow(SW_MINIMIZE);
}

void DebugWindowDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (nType == SIZE_MINIMIZED) {
		if (mMainWndHWND) {
			::SetFocus(mMainWndHWND);
		}
	}
}

void DebugWindowDialog::clearAllParticleSystems(void)
{
	CComboBox* combo = (CComboBox*) GetDlgItem(IDC_PSEd_ParticleSystem);
	if (!combo) {
		return;
	}

	m_listOfParticleSystems.clear();
	combo->ResetContent();
}

void DebugWindowDialog::clearAllThingTemplates( void )
{
	m_listOfThingTemplates.clear();
	// this is a kindof(dirty_hack), because there's no way at runtime to verify that
	// particleTypePanels[1] is actually a PArticlePanelDrawable
	((ParticlePanelDrawable*)m_particleTypePanels[1])->clearAllThingTemplates();
}

void DebugWindowDialog::appendParticleSystemToList( const std::string &rString )
{
	CComboBox* combo = (CComboBox*) GetDlgItem(IDC_PSEd_ParticleSystem);
	if (!combo) {
		return;
	}

	m_listOfParticleSystems.push_front(rString);
	combo->AddString(rString.c_str());
}

void DebugWindowDialog::appendThingTemplateToList( IN const std::string &rString )
{
	m_listOfThingTemplates.push_back(rString);
}

void DebugWindowDialog::OnParticleSystemChange()
{
	m_changeHasOcurred = true;
}

void DebugWindowDialog::OnPushSave()
{
	m_shouldWriteINI = true;
}

void DebugWindowDialog::OnReloadTextures()
{
	// First, reload the textures
	ParticlePanelParticle* pParticle = (ParticlePanelParticle*) m_particleTypePanels[0];
	if (!pParticle) {
		return;
	}

	pParticle->InitPanel();

	// Then, force an update to the ui, to repick the appropriate texture.
	performUpdate(true);

	// Finally, signal a flag to the asset manager to reload the actual textures.
	m_shouldReloadTextures = true;
}

Bool DebugWindowDialog::hasSelectionChanged(void)
{
	if (m_changeHasOcurred) {
		m_changeHasOcurred = false;
		return true;
	}

	return false;
}

void DebugWindowDialog::getSelectedSystemName(char *bufferToCopyInto) const
{
	if (!bufferToCopyInto) {
		return;
	}

	CComboBox* combo = (CComboBox*) GetDlgItem(IDC_PSEd_ParticleSystem);
	if (!combo) {
		bufferToCopyInto[0] = '\0';
		return;
	}

	int ndx = combo->GetCurSel();
	CString string;

	if (ndx > -1) {
		combo->GetLBText(ndx, string);
		strcpy(bufferToCopyInto, string.GetBuffer(0));
	} else {
		combo->GetWindowText(string);
		strcpy(bufferToCopyInto, string.GetBuffer(0));
	}
}

void DebugWindowDialog::getSelectedParticleAsciiStringParm( IN int parmNum, OUT char *bufferToCopyInto ) const
{
	if (parmNum < 0 || parmNum >= PARM_NumParms || !bufferToCopyInto || !m_particleSystem) {
		return;
	}

	if (!m_particleParmValues[parmNum].empty()) {
		strcpy(bufferToCopyInto, m_particleParmValues[parmNum].c_str());
	} else {
		bufferToCopyInto[0] = 0;
	}
}

void DebugWindowDialog::updateParticleAsciiStringParm( IN int parmNum, OUT const char *bufferToCopyFrom )
{
	if (parmNum < 0 || parmNum >= PARM_NumParms || !bufferToCopyFrom) {
		return;
	}

	m_particleParmValues[parmNum] = bufferToCopyFrom;
}

void DebugWindowDialog::getColorValueFromSystem(IN Int systemNum, OUT RGBColorKeyframe &colorFrame) const
{
	if (systemNum >= MAX_KEYFRAMES || systemNum < 0 || !m_particleSystem) {
		return;
	}

	colorFrame = m_particleSystem->m_colorKey[systemNum];
}

void DebugWindowDialog::updateColorValueToSystem(IN Int systemNum, IN const RGBColorKeyframe &colorFrame)
{
	if (systemNum >= MAX_KEYFRAMES || systemNum < 0 || !m_particleSystem) {
		return;
	}

	m_particleSystem->m_colorKey[systemNum] = colorFrame;
}

void DebugWindowDialog::getAlphaRangeFromSystem(IN Int systemNum, OUT ParticleSystemInfo::RandomKeyframe &randomVar) const
{
	if (systemNum >= MAX_KEYFRAMES || systemNum < 0 || !m_particleSystem) {
		return;
	}

	randomVar = m_particleSystem->m_alphaKey[systemNum];
}

void DebugWindowDialog::updateAlphaRangeToSystem(IN Int systemNum, IN const ParticleSystemInfo::RandomKeyframe &randomVar)
{
	if (systemNum >= MAX_KEYFRAMES || systemNum < 0 || !m_particleSystem) {
		return;
	}

	m_particleSystem->m_alphaKey[systemNum] = randomVar;
}

void DebugWindowDialog::getHalfSizeFromSystem( IN Int coordNum, OUT Real& halfSize) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: halfSize = m_particleSystem->m_emissionVolume.box.halfSize.x; return;
		case 1: halfSize = m_particleSystem->m_emissionVolume.box.halfSize.y; return;
		case 2: halfSize = m_particleSystem->m_emissionVolume.box.halfSize.z; return;
		default: return;
	};
}

void DebugWindowDialog::updateHalfSizeToSystem( IN Int coordNum, IN const Real& halfSize)
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: m_particleSystem->m_emissionVolume.box.halfSize.x = halfSize; return;
		case 1: m_particleSystem->m_emissionVolume.box.halfSize.y = halfSize; return;
		case 2: m_particleSystem->m_emissionVolume.box.halfSize.z = halfSize; return;
		default: return;
	};
}

void DebugWindowDialog::getSphereRadiusFromSystem( OUT Real &radius ) const
{
	if (!m_particleSystem) {
		return;
	}

	radius = m_particleSystem->m_emissionVolume.sphere.radius;
}

void DebugWindowDialog::updateSphereRadiusToSystem( IN const Real &radius )
{
	if (!m_particleSystem) {
		return;
	}

	m_particleSystem->m_emissionVolume.sphere.radius = radius;
}

void DebugWindowDialog::getCylinderRadiusFromSystem( OUT Real &radius ) const
{
	if (!m_particleSystem) {
		return;
	}

	radius = m_particleSystem->m_emissionVolume.cylinder.radius;
}

void DebugWindowDialog::updateCylinderRadiusToSystem( IN const Real &radius )
{
	if (!m_particleSystem) {
		return;
	}

	m_particleSystem->m_emissionVolume.cylinder.radius = radius;
}

void DebugWindowDialog::getCylinderLengthFromSystem( OUT Real &length ) const
{
	if (!m_particleSystem) {
		return;
	}

	length = m_particleSystem->m_emissionVolume.cylinder.length;
}

void DebugWindowDialog::updateCylinderLengthToSystem( IN const Real &length )
{
	if (!m_particleSystem) {
		return;
	}

	m_particleSystem->m_emissionVolume.cylinder.length = length;
}

void DebugWindowDialog::getLineFromSystem( IN Int coordNum, OUT Real& linePoint) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: linePoint = m_particleSystem->m_emissionVolume.line.start.x; return;
		case 1: linePoint = m_particleSystem->m_emissionVolume.line.start.y; return;
		case 2: linePoint = m_particleSystem->m_emissionVolume.line.start.z; return;
		case 3: linePoint = m_particleSystem->m_emissionVolume.line.end.x; return;
		case 4: linePoint = m_particleSystem->m_emissionVolume.line.end.y; return;
		case 5: linePoint = m_particleSystem->m_emissionVolume.line.end.z; return;
		default: return;
	};
}


void DebugWindowDialog::updateLineToSystem( IN Int coordNum, IN const Real& linePoint)
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: m_particleSystem->m_emissionVolume.line.start.x = linePoint; return;
		case 1: m_particleSystem->m_emissionVolume.line.start.y = linePoint; return;
		case 2: m_particleSystem->m_emissionVolume.line.start.z = linePoint; return;
		case 3: m_particleSystem->m_emissionVolume.line.end.x = linePoint; return;
		case 4: m_particleSystem->m_emissionVolume.line.end.y = linePoint; return;
		case 5: m_particleSystem->m_emissionVolume.line.end.z = linePoint; return;
		default: return;
	};
}

void DebugWindowDialog::getVelSphereFromSystem( IN Int velNum, OUT Real &radius) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (velNum)
	{
		case 0: radius = m_particleSystem->m_emissionVelocity.spherical.speed.m_low; return;
		case 1: radius = m_particleSystem->m_emissionVelocity.spherical.speed.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateVelSphereToSystem( IN Int velNum, IN const Real &radius)
{
	if (!m_particleSystem) {
		return;
	}

	switch (velNum)
	{
		case 0: m_particleSystem->m_emissionVelocity.spherical.speed.m_low = radius; return;
		case 1: m_particleSystem->m_emissionVelocity.spherical.speed.m_high = radius; return;
		default: return;
	};
}

void DebugWindowDialog::getVelHemisphereFromSystem( IN Int velNum, OUT Real &radius) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (velNum)
	{
		case 0: radius = m_particleSystem->m_emissionVelocity.hemispherical.speed.m_low; return;
		case 1: radius = m_particleSystem->m_emissionVelocity.hemispherical.speed.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateVelHemisphereToSystem( IN Int velNum, IN const Real &radius)
{
	if (!m_particleSystem) {
		return;
	}

	switch (velNum)
	{
		case 0: m_particleSystem->m_emissionVelocity.hemispherical.speed.m_low = radius; return;
		case 1: m_particleSystem->m_emissionVelocity.hemispherical.speed.m_high = radius; return;
		default: return;
	};
}

void DebugWindowDialog::getVelOrthoFromSystem( IN Int coordNum, OUT Real& ortho) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: ortho = m_particleSystem->m_emissionVelocity.ortho.x.m_low; return;
		case 1: ortho = m_particleSystem->m_emissionVelocity.ortho.y.m_low; return;
		case 2: ortho = m_particleSystem->m_emissionVelocity.ortho.z.m_low; return;
		case 3: ortho = m_particleSystem->m_emissionVelocity.ortho.x.m_high; return;
		case 4: ortho = m_particleSystem->m_emissionVelocity.ortho.y.m_high; return;
		case 5: ortho = m_particleSystem->m_emissionVelocity.ortho.z.m_high; return;
		default: return;
	};
}


void DebugWindowDialog::updateVelOrthoToSystem( IN Int coordNum, IN const Real& ortho)
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: m_particleSystem->m_emissionVelocity.ortho.x.m_low = ortho; return;
		case 1: m_particleSystem->m_emissionVelocity.ortho.y.m_low = ortho; return;
		case 2: m_particleSystem->m_emissionVelocity.ortho.z.m_low = ortho; return;
		case 3: m_particleSystem->m_emissionVelocity.ortho.x.m_high = ortho; return;
		case 4: m_particleSystem->m_emissionVelocity.ortho.y.m_high = ortho; return;
		case 5: m_particleSystem->m_emissionVelocity.ortho.z.m_high = ortho; return;
		default: return;
	};
}

void DebugWindowDialog::getVelCylinderFromSystem( IN Int coordNum, OUT Real& cylinder) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: cylinder = m_particleSystem->m_emissionVelocity.cylindrical.radial.m_low; return;
		case 1: cylinder = m_particleSystem->m_emissionVelocity.cylindrical.normal.m_low; return;
		case 2: cylinder = m_particleSystem->m_emissionVelocity.cylindrical.radial.m_high; return;
		case 3: cylinder = m_particleSystem->m_emissionVelocity.cylindrical.normal.m_high; return;
		default: return;
	};
}


void DebugWindowDialog::updateVelCylinderToSystem( IN Int coordNum, IN const Real& cylinder)
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: m_particleSystem->m_emissionVelocity.cylindrical.radial.m_low = cylinder; return;
		case 1: m_particleSystem->m_emissionVelocity.cylindrical.normal.m_low = cylinder; return;
		case 2: m_particleSystem->m_emissionVelocity.cylindrical.radial.m_high = cylinder; return;
		case 3: m_particleSystem->m_emissionVelocity.cylindrical.normal.m_high = cylinder; return;
		default: return;
	};
}

void DebugWindowDialog::getVelOutwardFromSystem( IN Int coordNum, OUT Real& cylinder) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: cylinder = m_particleSystem->m_emissionVelocity.outward.speed.m_low; return;
		case 1: cylinder = m_particleSystem->m_emissionVelocity.outward.otherSpeed.m_low; return;
		case 2: cylinder = m_particleSystem->m_emissionVelocity.outward.speed.m_high; return;
		case 3: cylinder = m_particleSystem->m_emissionVelocity.outward.otherSpeed.m_high; return;
		default: return;
	};
}


void DebugWindowDialog::updateVelOutwardToSystem( IN Int coordNum, IN const Real& cylinder)
{
	if (!m_particleSystem) {
		return;
	}

	switch (coordNum)
	{
		case 0: m_particleSystem->m_emissionVelocity.outward.speed.m_low = cylinder; return;
		case 1: m_particleSystem->m_emissionVelocity.outward.otherSpeed.m_low = cylinder; return;
		case 2: m_particleSystem->m_emissionVelocity.outward.speed.m_high = cylinder; return;
		case 3: m_particleSystem->m_emissionVelocity.outward.otherSpeed.m_high = cylinder; return;
		default: return;
	};
}

void DebugWindowDialog::getParticleNameFromSystem( OUT char *buffer, IN int buffLen ) const
{
	if (!m_particleSystem) {
		return;
	}

	if (!buffer) {
		return;
	}

	getSelectedParticleAsciiStringParm( PARM_ParticleTypeName, buffer);
}

void DebugWindowDialog::updateParticleNameToSystem( IN const char *buffer )
{
	if (!m_particleSystem) {
		return;
	}

	if (!buffer) {
		return;
	}

	updateParticleAsciiStringParm( PARM_ParticleTypeName, buffer);
}

void DebugWindowDialog::getDrawableNameFromSystem( OUT char *buffer, IN int buffLen ) const
{
	getParticleNameFromSystem(buffer, buffLen);
}

void DebugWindowDialog::updateDrawableNameToSystem( IN const char* buffer )
{
	updateParticleNameToSystem(buffer);
}

void DebugWindowDialog::updateCurrentParticleSystem(ParticleSystemTemplate *particleTemplate )
{
	m_particleSystem = particleTemplate;
	performUpdate(true);
}

void DebugWindowDialog::updateSystemUseParameters(IN ParticleSystemTemplate *particleTemplate )
{
	m_particleSystem = particleTemplate;
	performUpdate(false);
}

void DebugWindowDialog::getInitialDelayFromSystem( IN Int parmNum, OUT Real& initialDelay ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: initialDelay = m_particleSystem->m_initialDelay.m_low; return;
		case 1: initialDelay = m_particleSystem->m_initialDelay.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateInitialDelayToSystem( IN Int parmNum, IN const Real& initialDelay )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_initialDelay.m_low = initialDelay; return;
		case 1: m_particleSystem->m_initialDelay.m_high = initialDelay; return;
		default: return;
	};
}

void DebugWindowDialog::getBurstDelayFromSystem( IN Int parmNum, OUT Real& burstDelay ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: burstDelay = m_particleSystem->m_burstDelay.m_low; return;
		case 1: burstDelay = m_particleSystem->m_burstDelay.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateBurstDelayToSystem( IN Int parmNum, IN const Real& burstDelay )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_burstDelay.m_low = burstDelay; return;
		case 1: m_particleSystem->m_burstDelay.m_high = burstDelay; return;
		default: return;
	};
}

void DebugWindowDialog::getBurstCountFromSystem( IN Int parmNum, OUT Real& burstCount ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: burstCount = m_particleSystem->m_burstCount.m_low; return;
		case 1: burstCount = m_particleSystem->m_burstCount.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateBurstCountToSystem( IN Int parmNum, IN const Real& burstCount )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_burstCount.m_low = burstCount; return;
		case 1: m_particleSystem->m_burstCount.m_high = burstCount; return;
		default: return;
	};
}

void DebugWindowDialog::getColorScaleFromSystem( IN Int parmNum, OUT Real& colorScale ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: colorScale = m_particleSystem->m_colorScale.m_low; return;
		case 1: colorScale = m_particleSystem->m_colorScale.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateColorScaleToSystem( IN Int parmNum, IN const Real& colorScale )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_colorScale.m_low = colorScale; return;
		case 1: m_particleSystem->m_colorScale.m_high = colorScale; return;
		default: return;
	};
}

void DebugWindowDialog::getParticleLifetimeFromSystem( IN Int parmNum, OUT Real& particleLifetime ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: particleLifetime = m_particleSystem->m_lifetime.m_low; return;
		case 1: particleLifetime = m_particleSystem->m_lifetime.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateParticleLifetimeToSystem( IN Int parmNum, IN const Real& particleLifetime )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_lifetime.m_low = particleLifetime; return;
		case 1: m_particleSystem->m_lifetime.m_high = particleLifetime; return;
		default: return;
	};
}

void DebugWindowDialog::getParticleSizeFromSystem( IN Int parmNum, OUT Real& particleSize ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: particleSize = m_particleSystem->m_startSize.m_low; return;
		case 1: particleSize = m_particleSystem->m_startSize.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateParticleSizeToSystem( IN Int parmNum, IN const Real& particleSize )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_startSize.m_low = particleSize; return;
		case 1: m_particleSystem->m_startSize.m_high = particleSize; return;
		default: return;
	};
}

void DebugWindowDialog::getStartSizeRateFromSystem( IN Int parmNum, OUT Real& startSizeRate ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: startSizeRate = m_particleSystem->m_startSizeRate.m_low; return;
		case 1: startSizeRate = m_particleSystem->m_startSizeRate.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateStartSizeRateToSystem( IN Int parmNum, IN const Real& startSizeRate )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_startSizeRate.m_low = startSizeRate; return;
		case 1: m_particleSystem->m_startSizeRate.m_high = startSizeRate; return;
		default: return;
	};
}

void DebugWindowDialog::getSizeRateFromSystem( IN Int parmNum, OUT Real& sizeRate ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: sizeRate = m_particleSystem->m_sizeRate.m_low; return;
		case 1: sizeRate = m_particleSystem->m_sizeRate.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateSizeRateToSystem( IN Int parmNum, IN const Real& sizeRate )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_sizeRate.m_low = sizeRate; return;
		case 1: m_particleSystem->m_sizeRate.m_high = sizeRate; return;
		default: return;
	};
}

void DebugWindowDialog::getSizeDampingFromSystem( IN Int parmNum, OUT Real& sizeDamping ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: sizeDamping = m_particleSystem->m_sizeRateDamping.m_low; return;
		case 1: sizeDamping = m_particleSystem->m_sizeRateDamping.m_high; return;
		default: return;
	};
}

void DebugWindowDialog::updateSizeDampingToSystem( IN Int parmNum, IN const Real& sizeDamping )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_sizeRateDamping.m_low = sizeDamping; return;
		case 1: m_particleSystem->m_sizeRateDamping.m_high = sizeDamping; return;
		default: return;
	};
}

void DebugWindowDialog::getSystemLifetimeFromSystem( OUT Real& systemLifetime ) const
{
	if (!m_particleSystem) {
		return;
	}

	systemLifetime = m_particleSystem->m_systemLifetime;
}

void DebugWindowDialog::updateSystemLifetimeToSystem( IN const Real& systemLifetime )
{
	if (!m_particleSystem) {
		return;
	}

	m_particleSystem->m_systemLifetime = systemLifetime;
}

void DebugWindowDialog::getSlaveOffsetFromSystem( IN Int parmNum, OUT Real& slaveOffset ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: slaveOffset = m_particleSystem->m_slavePosOffset.x; return;
		case 1: slaveOffset = m_particleSystem->m_slavePosOffset.y; return;
		case 2: slaveOffset = m_particleSystem->m_slavePosOffset.z; return;
		default: return;
	};
}

void DebugWindowDialog::updateSlaveOffsetToSystem( IN Int parmNum, IN const Real& slaveOffset )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_slavePosOffset.x = slaveOffset; return;
		case 1: m_particleSystem->m_slavePosOffset.y = slaveOffset; return;
		case 2: m_particleSystem->m_slavePosOffset.z = slaveOffset; return;
		default: return;
	};
}

void DebugWindowDialog::getDriftVelocityFromSystem( IN Int parmNum, OUT Real& driftVelocity ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: driftVelocity = m_particleSystem->m_driftVelocity.x; return;
		case 1: driftVelocity = m_particleSystem->m_driftVelocity.y; return;
		case 2: driftVelocity = m_particleSystem->m_driftVelocity.z; return;
		default: return;
	};
}

void DebugWindowDialog::updateDriftVelocityToSystem( IN Int parmNum, IN const Real& driftVelocity )
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: m_particleSystem->m_driftVelocity.x = driftVelocity; return;
		case 1: m_particleSystem->m_driftVelocity.y = driftVelocity; return;
		case 2: m_particleSystem->m_driftVelocity.z = driftVelocity; return;
		default: return;
	};
}

void DebugWindowDialog::getSlaveSystemFromSystem( OUT char *buffer, IN Int bufferSize) const
{
	if (!m_particleSystem) {
		return;
	}

	getSelectedParticleAsciiStringParm(PARM_SlaveSystemName, buffer);
}

void DebugWindowDialog::updateSlaveSystemToSystem( IN const char *buffer )
{
	if (!m_particleSystem) {
		return;
	}

	updateParticleAsciiStringParm(PARM_SlaveSystemName, buffer);
}

void DebugWindowDialog::getPerParticleSystemFromSystem( OUT char *buffer, IN Int bufferSize) const
{
	if (!m_particleSystem) {
		return;
	}

	getSelectedParticleAsciiStringParm(PARM_AttachedSystemName, buffer);
}

void DebugWindowDialog::updatePerParticleSystemToSystem( IN const char *buffer )
{
	if (!m_particleSystem) {
		return;
	}

	updateParticleAsciiStringParm(PARM_AttachedSystemName, buffer);
}

void DebugWindowDialog::getSwitchFromSystem( IN SwitchType switchType, OUT Bool& switchVal) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (switchType)
	{
		case ST_HOLLOW: switchVal = m_particleSystem->m_isEmissionVolumeHollow; break;
		case ST_ONESHOT: switchVal = m_particleSystem->m_isOneShot; break;
		case ST_ALIGNXY: switchVal = m_particleSystem->m_isGroundAligned; break;
		case ST_EMITABOVEGROUNDONLY: switchVal = m_particleSystem->m_isEmitAboveGroundOnly; break;
		case ST_PARTICLEUPTOWARDSEMITTER: switchVal = m_particleSystem->m_isParticleUpTowardsEmitter; break;
	};
}

void DebugWindowDialog::updateSwitchToSystem( IN SwitchType switchType, IN const Bool& switchVal)
{
	if (!m_particleSystem) {
		return;
	}

	switch (switchType)
	{
		case ST_HOLLOW: m_particleSystem->m_isEmissionVolumeHollow = switchVal; break;
		case ST_ONESHOT: m_particleSystem->m_isOneShot = switchVal; break;
		case ST_ALIGNXY: m_particleSystem->m_isGroundAligned = switchVal; break;
		case ST_EMITABOVEGROUNDONLY: m_particleSystem->m_isEmitAboveGroundOnly = switchVal; break;
		case ST_PARTICLEUPTOWARDSEMITTER: m_particleSystem->m_isParticleUpTowardsEmitter = switchVal; break;
	};
}

// ------------------------------------------------------------------------------------------------
static Real degreeToRadian( Real degree ) {	return (degree / 180.0f) * PI; }
static Real radianToDegree( Real radian ) { return (180.0f * radian) / PI; }

void DebugWindowDialog::getPingPongStartAngleFromSystem( IN Int parmNum, OUT Real& angle ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: angle = m_particleSystem->m_windMotionStartAngleMin; break;
		case 1: angle = m_particleSystem->m_windMotionStartAngleMax; break;
		default: return;
	};

	angle = radianToDegree( angle );

}

void DebugWindowDialog::updatePingPongStartAngleToSystem( IN Int parmNum, IN const Real& angle )
{
	if (!m_particleSystem) {
		return;
	}

	Real radian = degreeToRadian( angle );

	switch (parmNum)
	{
		case 0: m_particleSystem->m_windMotionStartAngleMin = radian; return;
		case 1: m_particleSystem->m_windMotionStartAngleMax = radian; return;
		default: return;
	};
}

void DebugWindowDialog::getPingPongEndAngleFromSystem( IN Int parmNum, OUT Real& angle ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: angle = m_particleSystem->m_windMotionEndAngleMin; break;
		case 1: angle = m_particleSystem->m_windMotionEndAngleMax; break;
		default: return;
	};

	angle = radianToDegree( angle );

}

void DebugWindowDialog::updatePingPongEndAngleToSystem( IN Int parmNum, IN const Real& angle )
{
	if (!m_particleSystem) {
		return;
	}

	Real radian = degreeToRadian( angle );

	switch (parmNum)
	{
		case 0: m_particleSystem->m_windMotionEndAngleMin = radian; return;
		case 1: m_particleSystem->m_windMotionEndAngleMax = radian; return;
		default: return;
	};
}

void DebugWindowDialog::getWindAngleChangeFromSystem( IN Int parmNum, OUT Real& angle ) const
{
	if (!m_particleSystem) {
		return;
	}

	switch (parmNum)
	{
		case 0: angle = m_particleSystem->m_windAngleChangeMin; break;
		case 1: angle = m_particleSystem->m_windAngleChangeMax; break;
		default: return;
	};

	angle = radianToDegree( angle );

}

void DebugWindowDialog::updateWindAngleChangeToSystem( IN Int parmNum, IN const Real& angle )
{
	if (!m_particleSystem) {
		return;
	}

	Real radian = degreeToRadian( angle );

	switch (parmNum)
	{
		case 0: m_particleSystem->m_windAngleChangeMin = radian; return;
		case 1: m_particleSystem->m_windAngleChangeMax = radian; return;
		default: return;
	};
}

void DebugWindowDialog::getWindMotionFromSystem( OUT ParticleSystemInfo::WindMotion& windMotion ) const
{
	if (!m_particleSystem) {
		return;
	}

	windMotion = m_particleSystem->m_windMotion;
}

void DebugWindowDialog::updateWindMotionToSystem( IN const ParticleSystemInfo::WindMotion& windMotion )
{
	if (!m_particleSystem) {
		return;
	}

	m_particleSystem->m_windMotion = windMotion;
}

// The reason I'm using this function is to prohibit me from forgetting to add an update
// one way or the other. The idea is that when you implement one, you might as well implement
// the other, and this way they're in one place.
void DebugWindowDialog::performUpdate( IN Bool toUI )
{
	if (!m_particleSystem) {
		return;
	}

	static char buff[ARBITRARY_BUFF_SIZE];

	{	// Update the emission type, velocity type, particle type and shader type.

		CComboBox *pCombo;

		pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_Priority);
		if (pCombo) {
			int selndx;
			if (toUI) {
			  selndx = pCombo->SelectString(-1, ParticlePriorityNames[(long) m_particleSystem->m_priority]);
			} else {
				selndx = pCombo->GetCurSel();
				if (selndx >= 0) {
				  m_particleSystem->m_priority = (ParticlePriorityType)(selndx + 1);
				}
			}
		}

		pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_EmissionType);
		if (pCombo) {
			int selndx;
			if (toUI) {
				selndx = pCombo->SelectString(-1, EmissionVolumeTypeNames[(long) m_particleSystem->m_emissionVolumeType]);
			} else {
				selndx = pCombo->GetCurSel();
				if (selndx >= 0) {
					m_particleSystem->m_emissionVolumeType = (ParticleSystemInfo::EmissionVolumeType) (selndx + 1);
				}
			}

			// do the page swap
			if (selndx != m_activeEmissionPage && selndx >= 0) {
				m_emissionTypePanels[m_activeEmissionPage]->ShowWindow(SW_HIDE);
				m_activeEmissionPage = selndx;
				m_emissionTypePanels[m_activeEmissionPage]->ShowWindow(SW_SHOW);
			}
		}

		pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_VelocityType);
		if (pCombo) {
			int selndx;
			if (toUI) {
				selndx = pCombo->SelectString(-1, EmissionVelocityTypeNames[(long) m_particleSystem->m_emissionVelocityType]);
			} else {
				selndx = pCombo->GetCurSel();
				if (selndx >= 0) {
					m_particleSystem->m_emissionVelocityType = (ParticleSystemInfo::EmissionVelocityType) (selndx + 1);
				}
			}

			// do the page swap
			if (selndx != m_activeVelocityPage && selndx >= 0) {
				m_velocityTypePanels[m_activeVelocityPage]->ShowWindow(SW_HIDE);
				m_activeVelocityPage = selndx;
				m_velocityTypePanels[m_activeVelocityPage]->ShowWindow(SW_SHOW);
			}
		}

		pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_ParticleType);
		if (pCombo) {
			int selndx;
			if (toUI) {
				selndx = pCombo->SelectString(-1, ParticleTypeNames[(long) m_particleSystem->m_particleType]);
			} else {
				selndx = pCombo->GetCurSel();
				if (selndx >= 0) {
					m_particleSystem->m_particleType = (ParticleSystemInfo::ParticleType) (selndx + 1);
				}
			}

			// do the swap
			if (selndx != m_activeParticlePage && selndx >= 0) {
				m_particleTypePanels[m_activeParticlePage]->ShowWindow(SW_HIDE);
				m_activeParticlePage = selndx;
				m_particleTypePanels[m_activeParticlePage]->ShowWindow(SW_SHOW);
			}
		}

		pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_ShaderType);
		if (pCombo) {
			if (toUI) {
				pCombo->SelectString(-1, ParticleShaderTypeNames[(long) m_particleSystem->m_shaderType]);
			} else {
				int selndx = pCombo->GetCurSel();
				if (selndx >= 0) {
					m_particleSystem->m_shaderType = (ParticleSystemInfo::ParticleShaderType) (selndx + 1);
				}
			}
		}
	}

	{	// update the minimum and maximum for Angle X, Y and Z, and Angular X, Y, and Z

		CWnd *pWnd;
		pWnd = GetDlgItem(IDC_PSEd_AngleXMin);
		if (pWnd) {
			if (toUI) {
#if PARTICLE_USE_XY_ROTATION
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angleX.getMinimumValue());
#else
				sprintf(buff, FORMAT_STRING, 0.0f);
#endif
				pWnd->SetWindowText(buff);
			} else {
#if PARTICLE_USE_XY_ROTATION
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angleX.m_low = atof(buff);
#endif
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_AngleYMin);
		if (pWnd) {
			if (toUI) {
#if PARTICLE_USE_XY_ROTATION
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angleY.getMinimumValue());
#else
				sprintf(buff, FORMAT_STRING, 0.0f);
#endif
				pWnd->SetWindowText(buff);
			} else {
#if PARTICLE_USE_XY_ROTATION
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angleY.m_low = atof(buff);
#endif
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_AngleZMin);
		if (pWnd) {
			if (toUI) {
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angleZ.getMinimumValue());
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angleZ.m_low = atof(buff);
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_AngleXMax);
		if (pWnd) {
			if (toUI) {
#if PARTICLE_USE_XY_ROTATION
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angleX.getMaximumValue());
#else
				sprintf(buff, FORMAT_STRING, 0.0f);
#endif
				pWnd->SetWindowText(buff);
			} else {
#if PARTICLE_USE_XY_ROTATION
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angleX.m_high = atof(buff);
#endif
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_AngleYMax);
		if (pWnd) {
			if (toUI) {
#if PARTICLE_USE_XY_ROTATION
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angleY.getMaximumValue());
#else
				sprintf(buff, FORMAT_STRING, 0.0f);
#endif
				pWnd->SetWindowText(buff);
			} else {
#if PARTICLE_USE_XY_ROTATION
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angleY.m_high = atof(buff);
#endif
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_AngleZMax);
		if (pWnd) {
			if (toUI) {
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angleZ.getMaximumValue());
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angleZ.m_high = atof(buff);
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_AngularRateXMin);
		if (pWnd) {
			if (toUI) {
#if PARTICLE_USE_XY_ROTATION
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angularRateX.getMinimumValue());
#else
				sprintf(buff, FORMAT_STRING, 0.0f);
#endif
				pWnd->SetWindowText(buff);
			} else {
#if PARTICLE_USE_XY_ROTATION
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angularRateX.m_low = atof(buff);
#endif
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_AngularRateYMin);
		if (pWnd) {
			if (toUI) {
#if PARTICLE_USE_XY_ROTATION
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angularRateY.getMinimumValue());
#else
				sprintf(buff, FORMAT_STRING, 0.0f);
#endif
				pWnd->SetWindowText(buff);
			} else {
#if PARTICLE_USE_XY_ROTATION
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angularRateY.m_low = atof(buff);
#endif
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_AngularRateZMin);
		if (pWnd) {
			if (toUI) {
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angularRateZ.getMinimumValue());
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angularRateZ.m_low = atof(buff);
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_AngularRateXMax);
		if (pWnd) {
			if (toUI) {
#if PARTICLE_USE_XY_ROTATION
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angularRateX.getMaximumValue());
#else
				sprintf(buff, FORMAT_STRING, 0.0f);
#endif
				pWnd->SetWindowText(buff);
			} else {
#if PARTICLE_USE_XY_ROTATION
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angularRateX.m_high = atof(buff);
#endif
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_AngularRateYMax);
		if (pWnd) {
			if (toUI) {
#if PARTICLE_USE_XY_ROTATION
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angularRateY.getMaximumValue());
#else
				sprintf(buff, FORMAT_STRING, 0.0f);
#endif
				pWnd->SetWindowText(buff);
			} else {
#if PARTICLE_USE_XY_ROTATION
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angularRateY.m_high = atof(buff);
#endif
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_AngularRateZMax);
		if (pWnd) {
			if (toUI) {
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angularRateZ.getMaximumValue());
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angularRateZ.m_high = atof(buff);
			}
		}
	}

	{	// update the damping values.
		CWnd *pWnd;
		pWnd = GetDlgItem(IDC_PSEd_AngleDampingMin);
		if (pWnd) {
			if (toUI) {
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angularDamping.getMinimumValue());
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angularDamping.m_low = atof(buff);
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_AngleDampingMax);
		if (pWnd) {
			if (toUI) {
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_angularDamping.getMaximumValue());
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_angularDamping.m_high = atof(buff);
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_VelocityDampingMin);
		if (pWnd) {
			if (toUI) {
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_velDamping.getMinimumValue());
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_velDamping.m_low = atof(buff);
			}
		}

		pWnd = GetDlgItem(IDC_PSEd_VelocityDampingMax);
		if (pWnd) {
			if (toUI) {
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_velDamping.getMaximumValue());
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_velDamping.m_high = atof(buff);
			}
		}
	}

	{ // update gravity
		CWnd *pWnd;
		pWnd = GetDlgItem(IDC_PSEd_Gravity);
		if (pWnd) {
			if (toUI) {
				sprintf(buff, FORMAT_STRING, m_particleSystem->m_gravity);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				m_particleSystem->m_gravity = atof(buff);
			}
		}
	}

	{	// all the kids need to update too.
		m_colorAlphaDialog.performUpdate(toUI);
		m_switchesDialog.performUpdate(toUI);
		m_moreParmsDialog.performUpdate(toUI);
		m_emissionTypePanels[m_activeEmissionPage]->performUpdate(toUI);
		m_velocityTypePanels[m_activeVelocityPage]->performUpdate(toUI);
		m_particleTypePanels[m_activeParticlePage]->performUpdate(toUI);
	}
}

void DebugWindowDialog::OnParticleSystemEdit()
{
	signalParticleSystemEdit();
}

void DebugWindowDialog::signalParticleSystemEdit( void )
{
	performUpdate(false);
}

void DebugWindowDialog::OnEditColorAlpha()
{
	CButton *pButton = (CButton*) GetDlgItem(IDC_PSEd_EditColorButton);
	if (!pButton) {
		return;
	}

	m_showColorDlg = !m_showColorDlg;

	if (m_showColorDlg) {
		m_colorAlphaDialog.ShowWindow(SW_SHOW);
		pButton->SetCheck(TRUE);
	} else {
		m_colorAlphaDialog.ShowWindow(SW_HIDE);
		pButton->SetCheck(FALSE);
	}
}

void DebugWindowDialog::OnEditSwitches()
{
	CButton *pButton = (CButton*) GetDlgItem(IDC_PSEd_EditSwitchesButton);
	if (!pButton) {
		return;
	}

	m_showSwitchesDlg = !m_showSwitchesDlg ;

	if (m_showSwitchesDlg) {
		m_switchesDialog.ShowWindow(SW_SHOW);
		pButton->SetCheck(TRUE);
	} else {
		m_switchesDialog.ShowWindow(SW_HIDE);
		pButton->SetCheck(FALSE);
	}
}

// kill all particle systems in the world -- one time effect
void DebugWindowDialog::OnKillAllParticleSystems()
{
	m_shouldKillAllParticleSystems = true;
}


void DebugWindowDialog::OnEditMoreParms()
{
	CButton *pButton = (CButton*) GetDlgItem(IDC_PSEd_Continued);
	if (!pButton) {
		return;
	}

	m_showMoreParmsDlg = !m_showMoreParmsDlg ;

	if (m_showMoreParmsDlg ) {
		m_moreParmsDialog.ShowWindow(SW_SHOW);
		pButton->SetCheck(TRUE);
	} else {
		m_moreParmsDialog.ShowWindow(SW_HIDE);
		pButton->SetCheck(FALSE);
	}
}

Bool DebugWindowDialog::shouldWriteINI( void )
{
	if (m_shouldWriteINI) {
		m_shouldWriteINI = false;
		return true;
	}

	return false;
}

Bool DebugWindowDialog::hasRequestedReload( void )
{
	if (m_shouldReload) {
		m_shouldReload = false;
		return true;
	}

	return false;
}

Bool DebugWindowDialog::shouldBusyWait( void )
{
	return m_shouldBusyWait;
}

Bool DebugWindowDialog::shouldUpdateParticleCap( void )
{
	if (m_shouldUpdateParticleCap) {
		m_shouldUpdateParticleCap = false;
		return true;
	}

	return false;
}

Bool DebugWindowDialog::shouldReloadTextures( void )
{
	if (m_shouldReloadTextures) {
		m_shouldReloadTextures = false;
		return true;
	}

	return false;
}

Bool DebugWindowDialog::shouldKillAllParticleSystems( void )
{
	if (m_shouldKillAllParticleSystems) {
		m_shouldKillAllParticleSystems = false;
		return true;
	}

	return false;
}

void DebugWindowDialog::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}

void DebugWindowDialog::OnReloadSystem( void )
{
	m_shouldReload = true;
}

void DebugWindowDialog::OnReloadCurrent()
{
	OnReloadSystem();
}

void DebugWindowDialog::OnReloadAll()
{
	OnReloadSystem();
}

void DebugWindowDialog::OnSaveCurrent()
{
	OnPushSave();
}

void DebugWindowDialog::OnSaveAll()
{
	OnPushSave();
}

void DebugWindowDialog::OnParticleCapEdit()
{
	m_shouldUpdateParticleCap = true;
}

void DebugWindowDialog::updateCurrentParticleCap( IN int particleCap )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	CWnd *pWnd = GetDlgItem(IDC_PSEd_CurrentParticleCap);
	if (!pWnd) {
		return;
	}

	sprintf(buff, "%d", particleCap);
	pWnd->SetWindowText(buff);
}

void DebugWindowDialog::updateCurrentNumParticles( IN int particleCount )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	CWnd *pWnd = GetDlgItem(IDC_PSEd_CurrentParticleCount);
	if (!pWnd) {
		return;
	}

	sprintf(buff, "%d", particleCount);
	pWnd->SetWindowText(buff);
}

int DebugWindowDialog::getNewParticleCap( void )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	CWnd *pWnd = GetDlgItem(IDC_PSEd_CurrentParticleCap);
	if (!pWnd) {
		return -1;
	}

	pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);

	return atoi(buff);
}

BEGIN_MESSAGE_MAP(DebugWindowDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_SYSCOMMAND()
	ON_CBN_SELCHANGE(IDC_PSEd_ParticleSystem, OnParticleSystemChange)
	ON_BN_CLICKED(IDC_PSEd_Go, OnParticleSystemChange)
	ON_BN_CLICKED(IDC_PSEd_EditColorButton, OnEditColorAlpha)
	ON_BN_CLICKED(IDC_PSEd_Continued, OnEditMoreParms)
	ON_BN_CLICKED(IDC_PSEd_Save, OnPushSave)
	ON_BN_CLICKED(IDC_PSEd_EditSwitchesButton, OnEditSwitches)
	ON_BN_CLICKED(IDC_PSEd_KillAll, OnKillAllParticleSystems)

	ON_CBN_SELCHANGE(IDC_PSEd_Priority, OnParticleSystemEdit)
	ON_CBN_SELCHANGE(IDC_PSEd_EmissionType, OnParticleSystemEdit)
	ON_CBN_SELCHANGE(IDC_PSEd_VelocityType, OnParticleSystemEdit)
	ON_CBN_SELCHANGE(IDC_PSEd_ParticleType, OnParticleSystemEdit)
	ON_CBN_SELCHANGE(IDC_PSEd_ShaderType, OnParticleSystemEdit)
	ON_COMMAND(ID_FILE_RELOADCURRENT, OnReloadCurrent)
	ON_COMMAND(ID_FILE_RELOADALL, OnReloadAll)
	ON_COMMAND(ID_FILE_SAVECURRENT, OnSaveCurrent)
	ON_COMMAND(ID_FILE_SAVEALL, OnSaveAll)
	ON_COMMAND(ID_FILE_RELOADTEXTURES, OnReloadTextures)


	ON_EN_KILLFOCUS(IDC_PSEd_CurrentParticleCap, OnParticleCapEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AngleXMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AngleYMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AngleZMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AngleXMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AngleYMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AngleZMax, OnParticleSystemEdit)

	ON_EN_KILLFOCUS(IDC_PSEd_AngularRateXMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AngularRateYMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AngularRateZMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AngularRateXMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AngularRateYMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AngularRateZMax, OnParticleSystemEdit)

	ON_EN_KILLFOCUS(IDC_PSEd_AngleDampingMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AngleDampingMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_VelocityDampingMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_VelocityDampingMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_Gravity, OnParticleSystemEdit)
END_MESSAGE_MAP()

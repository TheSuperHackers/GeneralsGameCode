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

#include "Lib/BaseType.h"

class ParticleSystemTemplate;

// Declared extern C to prevent name mangling, which makes life very unhappy
extern "C" {
	// Called to create the dialog
	void __declspec(dllexport) CreateParticleSystemDialog( void );

	// Called to (not surprisingly) destroy the dialog (and free the resources)
	void __declspec(dllexport) DestroyParticleSystemDialog( void );

	void __declspec(dllexport) RemoveAllParticleSystems( void );
	void __declspec(dllexport) AppendParticleSystem( const char* particleSystemName );
	void __declspec(dllexport) RemoveAllThingTemplates( void );
	void __declspec(dllexport) AppendThingTemplate( const char* thingTemplateName );

	Bool __declspec(dllexport) HasUpdatedSelectedParticleSystem( void );

	void __declspec(dllexport) GetSelectedParticleSystemName( char *bufferToCopyInto );

	void __declspec(dllexport) UpdateCurrentParticleCap( int currentParticleCap );
	void __declspec(dllexport) UpdateCurrentNumParticles( int currentParticleCount );
	int __declspec(dllexport) GetNewParticleCap( void );


#	define PARM_ParticleTypeName		0x00
# define PARM_SlaveSystemName			0x01
#	define PARM_AttachedSystemName	0x02

// Keep this one last
# define PARM_NumParms	0x03
	// parmNum can be exactly one of the above defines (PARM_*)
	void __declspec(dllexport) GetSelectedParticleAsciiStringParm( int parmNum, char *bufferToCopyInto, ParticleSystemTemplate **whichTemplate );
	void __declspec(dllexport) UpdateParticleAsciiStringParm( int parmNum, const char *bufferToCopyFrom, ParticleSystemTemplate **whichTemplate  );


	void __declspec(dllexport) UpdateCurrentParticleSystem( ParticleSystemTemplate *particleTemplate );
	void __declspec(dllexport) UpdateSystemUseParameters( ParticleSystemTemplate *particleTemplate );

	Bool __declspec(dllexport) ShouldWriteINI( void );
	Bool __declspec(dllexport) ShouldBusyWait( void );
	Bool __declspec(dllexport) ShouldUpdateParticleCap( void );
	Bool __declspec(dllexport) ShouldReloadTextures( void );


#	define PEB_Continue								0x00
#	define PEB_UpdateCurrentSystem		0x01
#	define PEB_ChangeToAnotherSystem	0x02
#	define PEB_SaveCurrentSystem			0x03
#	define PEB_SaveAllSystems					0x03
#	define PEB_ReloadCurrentSystem		0x04
#	define PEB_SetParticleCap					0x05
# define PEB_ReloadTextures					0x06
# define PEB_KillAllSystems					0x07
#	define PEB_BusyWait								0xFE
#	define PEB_Error									0xFF


	int __declspec(dllexport) NextParticleEditorBehavior( void );


}

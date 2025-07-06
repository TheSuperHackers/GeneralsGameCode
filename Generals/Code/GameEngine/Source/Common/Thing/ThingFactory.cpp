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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: ThingFactory.cpp /////////////////////////////////////////////////////////////////////////
// Created:   Colin Day, April 2001
// Desc:		This is how we go and make our things, we make our things, we make our things!	
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/FileSystem.h"
#include "Common/GameAudio.h"
#include "Common/MapObject.h"
#include "Common/ModuleFactory.h"
#include "Common/RandomValue.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/CreateModule.h"
#include "Common/ProductionPrerequisite.h"
#include "GameClient/GameClient.h"
#include "GameClient/Drawable.h"
#include "Common/INI.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
///#pragma optimize("", off)
///#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

enum { TEMPLATE_HASH_SIZE = 12288 };

// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
ThingFactory *TheThingFactory = NULL;  ///< Thing manager singleton declaration

// STATIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** Free all data loaded into this template database */
//-------------------------------------------------------------------------------------------------
void ThingFactory::freeDatabase( void )
{
	while (m_firstTemplate)
	{
		ThingTemplate* tmpl = m_firstTemplate;
		m_firstTemplate = m_firstTemplate->friend_getNextTemplate();
		deleteInstance(tmpl);
	}

	m_templateHashMap.clear();

}  // end freeDatabase

//-------------------------------------------------------------------------------------------------
/** add the thing template passed in, into the databse */
//-------------------------------------------------------------------------------------------------
void ThingFactory::addTemplate( ThingTemplate *tmplate )
{
	ThingTemplateHashMapIt tIt = m_templateHashMap.find(tmplate->getName());

	if (tIt != m_templateHashMap.end()) {
		DEBUG_CRASH(("Duplicate Thing Template name found: %s\n", tmplate->getName().str()));
	}

	// Link it to the list
	tmplate->friend_setNextTemplate(m_firstTemplate);
	m_firstTemplate = tmplate;

	// Add it to the hash table.
	m_templateHashMap[tmplate->getName()] = tmplate;
}  // end addTemplate

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ThingFactory::ThingFactory()
{
	m_firstTemplate = NULL;
	m_nextTemplateID = 1;	// not zero!

#ifdef USING_STLPORT
	m_templateHashMap.resize( TEMPLATE_HASH_SIZE );
#else
	m_templateHashMap.reserve( TEMPLATE_HASH_SIZE );
#endif
}  // end ThingFactory

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ThingFactory::~ThingFactory()
{

	// free all the template data
	freeDatabase();

}  // end ~ThingFactory

//-------------------------------------------------------------------------------------------------
/** Create a new template with name 'name' and add to our template list */
//-------------------------------------------------------------------------------------------------
ThingTemplate *ThingFactory::newTemplate( const AsciiString& name )
{
	ThingTemplate *newTemplate;

	// allocate template
	newTemplate = newInstance(ThingTemplate);

	// if the default template is present, get it and copy over any data to the new template
	const ThingTemplate *defaultT = findTemplate( AsciiString( "DefaultThingTemplate" ) );
	if( defaultT )
	{

		// copy over static data
		*newTemplate = *defaultT;
		newTemplate->setCopiedFromDefault();

	}  // end if

	// give template a unique identifier
	newTemplate->friend_setTemplateID( m_nextTemplateID++ );
	DEBUG_ASSERTCRASH( m_nextTemplateID != 0, ("m_nextTemplateID wrapped to zero") );

	// assign name
	newTemplate->friend_setTemplateName( name );

	// add to list
	addTemplate( newTemplate );

	// return the newly created template
	return newTemplate;

}

//-------------------------------------------------------------------------------------------------
/** Create newTemplate, copy data from final override of 'thingTemplate' to the newly created one,
	* and add newTemplate as the m_override of that final override.  NOTE that newTemplate
	* is *NOT* added to master template list, it is a hidden place to store
	* override values for 'thingTemplate' */
//-------------------------------------------------------------------------------------------------
ThingTemplate* ThingFactory::newOverride( ThingTemplate *thingTemplate )
{

	// sanity
	DEBUG_ASSERTCRASH( thingTemplate, ("newOverride(): NULL 'parent' thing template\n") );

	// sanity just for debuging, the weapon must be in the master list to do overrides
	DEBUG_ASSERTCRASH( findTemplate( thingTemplate->getName() ) != NULL,
										 ("newOverride(): Thing template '%s' not in master list\n", 
										 thingTemplate->getName().str()) );

	// find final override of the 'parent' template
	ThingTemplate *child = (ThingTemplate*) thingTemplate->friend_getFinalOverride();

	// allocate new template
	ThingTemplate *newTemplate = newInstance(ThingTemplate);

	// copy data from final override to 'newTemplate' as a set of initial default values
	*newTemplate = *child;
	newTemplate->setCopiedFromDefault();

	newTemplate->markAsOverride();
	child->setNextOverride(newTemplate);

	// return the newly created override for us to set values with etc
	return newTemplate;

}  // end newOverride

//-------------------------------------------------------------------------------------------------
/** Init */
//-------------------------------------------------------------------------------------------------
void ThingFactory::init( void )
{

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void ThingFactory::reset( void )
{
	ThingTemplate *t;
	// go through all templates and delete any overrides
	for( t = m_firstTemplate; t; /* empty */ )
	{
		Bool possibleAdjustment = FALSE;
		// t itself can be deleted if it is something created for this map only. Therefore, 
		// we need to store what the next item is so that we don't orphan a bunch of templates.
		ThingTemplate *nextT = t->friend_getNextTemplate();
		if (t == m_firstTemplate) {
			possibleAdjustment = TRUE;
		}

		// if stillValid is NULL after we delete the overrides, then this template was created for 
		// this map only. If it also happens to be m_firstTemplate, then we need to update m_firstTemplate
		// as well. Finally, if it was only created for this map, we need to remove the name from the 
		// hash map, to prevent any crashes.

		AsciiString templateName = t->getName();
		
		Overridable *stillValid = t->deleteOverrides();
		if (stillValid == NULL && possibleAdjustment) {
			m_firstTemplate = nextT;
		}
		
		if (stillValid == NULL) {
			// Also needs to be removed from the Hash map.
			m_templateHashMap.erase(templateName);
		}

		t = nextT;
	}
}  // end reset

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
void ThingFactory::update( void )
{

}  // end update

//-------------------------------------------------------------------------------------------------
/** Return the template with the matching database name */
//-------------------------------------------------------------------------------------------------
const ThingTemplate *ThingFactory::findByTemplateID( UnsignedShort id )
{
	for (ThingTemplate *tmpl = m_firstTemplate; tmpl; tmpl = tmpl->friend_getNextTemplate())
	{
		if (tmpl->getTemplateID() == id)
			return tmpl;
	}
	DEBUG_CRASH(("template %d not found\n",(Int)id));
	return NULL;
}

//-------------------------------------------------------------------------------------------------
/** Return the template with the matching database name */
//-------------------------------------------------------------------------------------------------
ThingTemplate *ThingFactory::findTemplateInternal( const AsciiString& name )
{
	ThingTemplateHashMapIt tIt = m_templateHashMap.find(name);

	if (tIt != m_templateHashMap.end()) {
		return tIt->second;
	}

#ifdef LOAD_TEST_ASSETS
	if (!strncmp(name.str(), TEST_STRING, strlen(TEST_STRING))) 
	{
		ThingTemplate *tmplate = newTemplate( AsciiString( "Un-namedTemplate" ) );

		// load the values
		tmplate->initForLTA( name );

		// Kinda lame, but necessary.
		m_templateHashMap.erase("Un-namedTemplate");
		m_templateHashMap[name] = tmplate;

		// add tmplate template to the database
		return findTemplateInternal( name );

	}
	
#endif
	
	//DEBUG_LOG(("*** Object template %s not found\n",name.str()));
	return NULL;

}  // end getTemplate

//=============================================================================
Object *ThingFactory::newObject( const ThingTemplate *tmplate, Team *team, ObjectStatusMaskType statusBits )
{
	if (tmplate == NULL)
		throw ERROR_BAD_ARG;

	const std::vector<AsciiString>& asv = tmplate->getBuildVariations();
	if (!asv.empty())
	{
		Int which = GameLogicRandomValue(0, asv.size()-1);
		const ThingTemplate* tmp = findTemplate( asv[which] );
		if (tmp != NULL)
			tmplate = tmp;
	}

	DEBUG_ASSERTCRASH(!tmplate->isKindOf(KINDOF_DRAWABLE_ONLY), ("You may not create Objects with the template %s, only Drawables\n",tmplate->getName().str()));

	// have the game logic create an object of the correct type.
	// (this will throw an exception on failure.)
	//Added ability to pass in optional statusBits. This is needed to be set prior to
	//the onCreate() calls... in the case of constructing.
	Object *obj = TheGameLogic->friend_createObject( tmplate, statusBits, team );

	// run the create function for the thing
	for (BehaviorModule** m = obj->getBehaviorModules(); *m; ++m)
	{
		CreateModuleInterface* create = (*m)->getCreate();
		if (!create)
			continue;
	
		create->onCreate();
	}

	//
	// all objects are part of the partition manager system, add it to that 
	// system now
	//
	ThePartitionManager->registerObject( obj );

	obj->initObject();

	return obj;

} 

//=============================================================================
Drawable *ThingFactory::newDrawable(const ThingTemplate *tmplate, DrawableStatus statusBits)
{
	if (tmplate == NULL)
		throw ERROR_BAD_ARG;

	Drawable *draw = TheGameClient->friend_createDrawable( tmplate, statusBits );

	/** @todo we should keep track of all the drawables we've allocated here
	but we'll wait until we have an drawable storage to do that cause it will
	all be tied together */

	return draw;

}  // end newDrawableByType

#if defined(RTS_DEBUG) || defined(RTS_INTERNAL) || defined(DEBUG_CRASHING)
AsciiString TheThingTemplateBeingParsedName;
#endif

//-------------------------------------------------------------------------------------------------
/** Parse Object entry */
//-------------------------------------------------------------------------------------------------
/*static*/ void ThingFactory::parseObjectDefinition( INI* ini, const AsciiString& name, const AsciiString& reskinFrom )
{
#if defined(RTS_DEBUG) || defined(RTS_INTERNAL) || defined(DEBUG_CRASHING)
	TheThingTemplateBeingParsedName = name;
#endif

	// find existing item if present
	ThingTemplate *thingTemplate = TheThingFactory->findTemplateInternal( name );
	if( !thingTemplate )
	{
		// no item is present, create a new one
		thingTemplate = TheThingFactory->newTemplate( name );
		if ( ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES )
		{
			// This ThingTemplate is actually an override, so we will mark it as such so that it properly
			// gets deleted on ::reset().
			thingTemplate->markAsOverride();
		}
	}
	else if( ini->getLoadType() != INI_LOAD_CREATE_OVERRIDES )
	{
		//Holy crap, this sucks to debug!!!
		//If you have two different objects, the previous code would simply 
		//allow you to define multiple objects with the same name, and just 
		//nuke the old one with the new one. So, I (KM) have added this 
		//assert to notify in case of two same-name objects.
		DEBUG_CRASH(( "[LINE: %d in '%s'] Duplicate factionunit %s found!", ini->getLineNum(), ini->getFilename().str(), name.str() ));
	}
	else
	{
		thingTemplate = TheThingFactory->newOverride( thingTemplate );
	}

	if (reskinFrom.isNotEmpty())
	{
		const ThingTemplate* reskinTmpl = TheThingFactory->findTemplate(reskinFrom);
		if (reskinTmpl)
		{
			thingTemplate->copyFrom(reskinTmpl);
			thingTemplate->setCopiedFromDefault();
			thingTemplate->setReskinnedFrom(reskinTmpl);
			ini->initFromINI( thingTemplate, thingTemplate->getReskinFieldParse() );
		}
		else
		{
			DEBUG_CRASH(("ObjectReskin must come after the original Object (%s, %s).\n",reskinFrom.str(),name.str()));
			throw INI_INVALID_DATA;
		}
	}
	else
	{
		ini->initFromINI( thingTemplate, thingTemplate->getFieldParse() );
	}

	thingTemplate->validate();

#if defined(RTS_DEBUG) || defined(RTS_INTERNAL) || defined(DEBUG_CRASHING)
	TheThingTemplateBeingParsedName.clear();
#endif
}

//#define CHECK_THING_NAMES
#ifdef CHECK_THING_NAMES

#include "Common/STLTypedefs.h"

const char *outFilenameINI				= "thing.txt";
const char *outFilenameStringFile	= "thingString.txt";

void resetReportFile( void )
{
	FILE *fp = fopen(outFilenameINI, "w");
	if (fp)
	{
		fprintf(fp, "-- ThingTemplate INI Report --\n\n");
		fclose(fp);
	}

	fp = fopen(outFilenameStringFile, "w");
	if (fp)
	{
		fprintf(fp, "-- ThingTemplate String File Report --\n\n");
		fclose(fp);
	}
}

AsciiStringList missingStrings;
void reportMissingNameInStringFile( AsciiString templateName )
{
	// see if we've seen it before
	AsciiStringListConstIterator cit = std::find(missingStrings.begin(), missingStrings.end(), templateName);
	if (cit != missingStrings.end())
		return;

	missingStrings.push_back(templateName);
}

void dumpMissingStringNames( void )
{
	missingStrings.sort();
	FILE *fp = fopen(outFilenameStringFile, "w");
	if (fp)
	{
		fprintf(fp, "-- ThingTemplate String File Report --\n\n");
		for (AsciiStringListConstIterator cit = missingStrings.begin(); cit!=missingStrings.end(); cit++)
		{
			fprintf(fp, "OBJECT:%s\n\"%s\"\nEND\n\n", cit->str(), cit->str());
		}
		fclose(fp);
	}
}

AsciiStringList missingNames;
void reportMissingNameInTemplate( AsciiString templateName )
{
	// see if we've seen it before
	AsciiStringListConstIterator cit = std::find(missingNames.begin(), missingNames.end(), templateName);
	if (cit != missingNames.end())
		return;

	missingNames.push_back(templateName);

	FILE *fp = fopen(outFilenameINI, "a+");
	if (fp)
	{
		fprintf(fp, "  DisplayName      = OBJECT:%s\n", templateName.str());
		fclose(fp);
	}

	//reportMissingNameInStringFile( templateName );
}

#endif

//-------------------------------------------------------------------------------------------------
/** Post process phase after loading the database files */
//-------------------------------------------------------------------------------------------------
void ThingFactory::postProcessLoad()
{
#ifdef CHECK_THING_NAMES
	//resetReportFile();
#endif

	// go through all thing templates
	for( ThingTemplate *thingTemplate = m_firstTemplate; 
			 thingTemplate; 
			 thingTemplate = thingTemplate->friend_getNextTemplate() )
	{

		// resolve the prerequisite names
		thingTemplate->resolveNames();

#ifdef CHECK_THING_NAMES
		if (thingTemplate->getDisplayName().isEmpty())
		{
			reportMissingNameInTemplate( thingTemplate->getName() );
		}
		else if (wcsstr(thingTemplate->getDisplayName().str(), L"MISSING:"))
		{
			AsciiString asciiName;
			asciiName.translate(thingTemplate->getDisplayName());
			asciiName.removeLastChar();
			asciiName = asciiName.str() + 17;
			reportMissingNameInStringFile( asciiName );
		}
#endif

	}  // end for 

#ifdef CHECK_THING_NAMES
	dumpMissingStringNames();
	exit(0);
#endif
}  // end postProcess

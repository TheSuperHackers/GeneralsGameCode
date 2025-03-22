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

// FILE: INI.h ////////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:   INI Reader
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __INI_H_
#define __INI_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include <stddef.h>	// for offsetof, which we don't use but everyone who includes us does
#include "Common/STLTypedefs.h"
#include "Common/AsciiString.h"
#include "Common/GameCommon.h"

//-------------------------------------------------------------------------------------------------
class INI;
class Xfer;
class File;
enum ScienceType;

//-------------------------------------------------------------------------------------------------
/** These control the behavior of loading the INI data into items */
//-------------------------------------------------------------------------------------------------
enum INILoadType
{	
	INI_LOAD_INVALID,						///< invalid load type
	INI_LOAD_OVERWRITE,					///< create new or load *over* existing data instance
	INI_LOAD_CREATE_OVERRIDES,	///< create new or load into *new* override data instance
	INI_LOAD_MULTIFILE					///< create new or continue loading into existing data instance.
};

//-------------------------------------------------------------------------------------------------
/** INI constant defines */
//-------------------------------------------------------------------------------------------------
enum
{
	INI_MAX_CHARS_PER_LINE = 1028,			///< max characters per line entry in any ini file
};

//-------------------------------------------------------------------------------------------------
/** Status return codes for the INI reader */
//-------------------------------------------------------------------------------------------------
enum
{
	// we map all of these to the same "real" error code, because
	// we generally don't care why it failed; but since the code distinguishes,
	// I didn't want to wipe out that intelligence. if we ever need to distinguish
	// failure modes at runtime, just put in real values for these.
	INI_CANT_SEARCH_DIR		= ERROR_BAD_INI,
	INI_INVALID_DIRECTORY = ERROR_BAD_INI,
	INI_INVALID_PARAMS		= ERROR_BAD_INI,
	INI_INVALID_NAME_LIST = ERROR_BAD_INI,
	INI_INVALID_DATA			= ERROR_BAD_INI,
	INI_MISSING_END_TOKEN = ERROR_BAD_INI,
	INI_UNKNOWN_TOKEN			= ERROR_BAD_INI,
	INI_BUFFER_TOO_SMALL  = ERROR_BAD_INI,
	INI_FILE_NOT_OPEN			= ERROR_BAD_INI,
	INI_FILE_ALREADY_OPEN = ERROR_BAD_INI,
	INI_CANT_OPEN_FILE		= ERROR_BAD_INI,
	INI_UNKNOWN_ERROR			= ERROR_BAD_INI,
	INI_END_OF_FILE				= ERROR_BAD_INI
};

//-------------------------------------------------------------------------------------------------
/** Function typedef for parsing data block fields.
	*
	* buffer - the character buffer of the line from INI that we are reading and parsing
	* instance - instance of what we're loading (for example a thingtemplate instance)
	* store - where to store the data parsed, this is a field in the *instance* 'instance'
	*/
//-------------------------------------------------------------------------------------------------
typedef void (*INIFieldParseProc)( INI *ini, void *instance, void *store, const void* userData );

//-------------------------------------------------------------------------------------------------
typedef const char* ConstCharPtr;
typedef ConstCharPtr* ConstCharPtrArray;

//-------------------------------------------------------------------------------------------------
struct LookupListRec
{
	const char* name;
	Int value;
};
typedef const LookupListRec *ConstLookupListRecArray;

//-------------------------------------------------------------------------------------------------
/** Parse tables for all fields of each data block are created using these */
//-------------------------------------------------------------------------------------------------
struct FieldParse
{
	const char*					token;						///< token of the field
	INIFieldParseProc		parse;						///< the parse function
	const void*					userData;					///< field-specific data
	Int									offset;						///< offset to data field

	inline void set(const char* t, INIFieldParseProc p, const void* u, Int o)
	{
		token = t;
		parse = p;
		userData = u;
		offset = o;
	}
};

//-------------------------------------------------------------------------------------------------
class MultiIniFieldParse
{
private:
	enum { MAX_MULTI_FIELDS = 16 };

	const FieldParse* m_fieldParse[MAX_MULTI_FIELDS];
	UnsignedInt				m_extraOffset[MAX_MULTI_FIELDS];
	Int								m_count;

public:
	MultiIniFieldParse() : m_count(0) 
	{ 
		//Added By Sadullah Nader
		//Initializations missing and needed
		for(Int i = 0; i < MAX_MULTI_FIELDS; i++)
			m_extraOffset[i] = 0;
		//

	}
	
	void add(const FieldParse* f, UnsignedInt e = 0);

	inline Int getCount() const { return m_count; }
	inline const FieldParse* getNthFieldParse(Int i) const { return m_fieldParse[i]; }
	inline UnsignedInt getNthExtraOffset(Int i) const { return m_extraOffset[i]; }
};

//-------------------------------------------------------------------------------------------------
/** Function typedef for parsing INI types blocks */
//-------------------------------------------------------------------------------------------------
typedef void (*INIBlockParse)( INI *ini );
typedef void (*BuildMultiIniFieldProc)(MultiIniFieldParse& p);

//-------------------------------------------------------------------------------------------------
/** INI Reader interface */
//-------------------------------------------------------------------------------------------------
class INI
{

public:

	INI();
	~INI();

	void loadDirectory( AsciiString dirName, Bool subdirs, INILoadType loadType, Xfer *pXfer );  ///< load directory of INI files
	void load( AsciiString filename, INILoadType loadType, Xfer *pXfer );		///< load INI file

	static Bool isDeclarationOfType( AsciiString blockType, AsciiString blockName, char *bufferToCheck );
	static Bool isEndOfBlock( char *bufferToCheck );

	// data type parsing (the highest level of what type of thing we're parsing)
	static void parseObjectDefinition( INI *ini );
	static void parseObjectReskinDefinition( INI *ini );
	static void parseWeaponTemplateDefinition( INI *ini );
	static void parseScienceDefinition( INI *ini );
	static void parseRankDefinition( INI *ini );
	static void parseCrateTemplateDefinition( INI *ini );
	static void parseLocomotorTemplateDefinition( INI *ini );
	static void parseLanguageDefinition( INI *ini );
	static void parsePlayerTemplateDefinition( INI *ini );
	static void parseGameDataDefinition( INI *ini );
	static void parseMapDataDefinition( INI *ini );
	static void parseAnim2DDefinition( INI *ini );
	static void parseAudioEventDefinition( INI *ini );
	static void parseDialogDefinition( INI *ini );
	static void parseMusicTrackDefinition( INI *ini );
	static void parseWebpageURLDefinition( INI *ini );
	static void parseHeaderTemplateDefinition( INI *ini );
	static void parseParticleSystemDefinition( INI *ini );
	static void parseWaterSettingDefinition( INI *ini );
	static void parseWaterTransparencyDefinition( INI *ini );
	static void parseMappedImageDefinition( INI *ini );
	static void parseArmorDefinition( INI *ini );
	static void parseDamageFXDefinition( INI *ini );
	static void parseDrawGroupNumberDefinition( INI *ini );
	static void parseTerrainDefinition( INI *ini );
	static void parseTerrainRoadDefinition( INI *ini );
	static void parseTerrainBridgeDefinition( INI *ini );
	static void parseMetaMapDefinition( INI *ini );
	static void parseFXListDefinition( INI *ini );
	static void parseObjectCreationListDefinition( INI* ini );
	static void parseMultiplayerSettingsDefinition( INI* ini );
	static void parseMultiplayerColorDefinition( INI* ini );
	static void parseOnlineChatColorDefinition( INI* ini );
	static void parseMapCacheDefinition( INI* ini );
	static void parseVideoDefinition( INI* ini );
	static void parseCommandButtonDefinition( INI *ini );
	static void parseCommandSetDefinition( INI *ini );
	static void parseUpgradeDefinition( INI *ini );
	static void parseMouseDefinition( INI* ini );
	static void parseMouseCursorDefinition( INI* ini );
	static void parseAIDataDefinition( INI *ini );
	static void parseSpecialPowerDefinition( INI *ini );
	static void parseInGameUIDefinition( INI *ini );
	static void parseControlBarSchemeDefinition( INI *ini );
	static void parseControlBarResizerDefinition( INI *ini );
	static void parseShellMenuSchemeDefinition( INI *ini );
	static void parseCampaignDefinition( INI *ini );
	static void parseAudioSettingsDefinition( INI *ini );
	static void parseMiscAudio( INI *ini );
	static void parseStaticGameLODDefinition( INI *ini);
	static void parseDynamicGameLODDefinition( INI *ini);
	static void parseStaticGameLODLevel( INI* ini, void * , void *store, const void*);
	static void parseDynamicGameLODLevel( INI* ini, void * , void *store, const void*);
	static void parseLODPreset( INI* ini);
	static void parseBenchProfile( INI* ini);
	static void parseEvaEvent( INI* ini );
	static void parseCredits( INI* ini );
	static void parseWindowTransitions( INI* ini );


	inline AsciiString getFilename( void ) const { return m_filename; }
	inline INILoadType getLoadType( void ) const { return m_loadType; }
	inline UnsignedInt getLineNum( void ) const { return m_lineNum; }
	inline const char *getSeps( void ) const { return m_seps; }
	inline const char *getSepsPercent( void ) const { return m_sepsPercent; }
	inline const char *getSepsColon( void ) const { return m_sepsColon; }
	inline const char *getSepsQuote( void ) { return m_sepsQuote; }
	inline Bool isEOF( void ) const { return m_endOfFile; }

	void initFromINI( void *what, const FieldParse* parseTable );
	void initFromINIMulti( void *what, const MultiIniFieldParse& parseTableList );
	void initFromINIMultiProc( void *what, BuildMultiIniFieldProc proc );
	
	static void parseUnsignedByte( INI *ini, void *instance, void *store, const void* userData );
	static void parseShort( INI *ini, void *instance, void *store, const void* userData );
	static void parseUnsignedShort( INI *ini, void *instance, void *store, const void* userData );
	static void parseInt( INI *ini, void *instance, void *store, const void* userData );
	static void parseUnsignedInt( INI *ini, void *instance, void *store, const void* userData );
	static void parseReal( INI *ini, void *instance, void *store, const void* userData );
	static void parsePositiveNonZeroReal( INI *ini, void *instance, void *store, const void* userData );
	static void parseBool( INI *ini, void *instance, void *store, const void* userData );
	static void parseBitInInt32( INI *ini, void *instance, void *store, const void* userData );
	static void parseAsciiString( INI *ini, void *instance, void *store, const void* userData );
	static void parseQuotedAsciiString( INI *ini, void *instance, void *store, const void* userData );
	static void parseAsciiStringVector( INI *ini, void *instance, void *store, const void* userData );
	static void parseAsciiStringVectorAppend( INI *ini, void *instance, void *store, const void* userData );
	static void parseAndTranslateLabel( INI *ini, void *instance, void *store, const void* userData );
	static void parseMappedImage( INI *ini, void *instance, void *store, const void *userData );
	static void parseAnim2DTemplate( INI *ini, void *instance, void *store, const void *userData );
	static void parsePercentToReal( INI *ini, void *instance, void *store, const void* userData );
	static void parseRGBColor( INI *ini, void *instance, void *store, const void* userData );
	static void parseRGBAColorInt( INI *ini, void *instance, void *store, const void* userData );
	static void parseColorInt( INI *ini, void *instance, void *store, const void* userData );
	static void parseCoord3D( INI *ini, void *instance, void *store, const void* userData );
	static void parseCoord2D( INI *ini, void *instance, void *store, const void *userData );
	static void parseICoord2D( INI *ini, void *instance, void *store, const void *userData );
	static void parseDynamicAudioEventRTS( INI *ini, void *instance, void *store, const void* userData );
	static void parseAudioEventRTS( INI *ini, void *instance, void *store, const void* userData );
	static void parseFXList( INI *ini, void *instance, void *store, const void* userData );
	static void parseParticleSystemTemplate( INI *ini, void *instance, void *store, const void *userData );
	static void parseObjectCreationList( INI *ini, void *instance, void *store, const void* userData );
	static void parseSpecialPowerTemplate( INI *ini, void *instance, void *store, const void *userData );
	static void parseUpgradeTemplate( INI *ini, void *instance, void *store, const void *userData );
	static void parseScience( INI *ini, void *instance, void *store, const void *userData );
	static void parseScienceVector( INI *ini, void *instance, void *store, const void *userData );
	static void parseGameClientRandomVariable( INI* ini, void *instance, void *store, const void* userData );
	static void parseBitString8( INI *ini, void *instance, void *store, const void* userData );
	static void parseBitString32( INI *ini, void *instance, void *store, const void* userData );
	static void parseByteSizedIndexList( INI *ini, void *instance, void *store, const void* userData );
	static void parseIndexList( INI *ini, void *instance, void *store, const void* userData );
	static void parseLookupList( INI *ini, void *instance, void *store, const void* userData );
	static void parseThingTemplate( INI *ini, void *instance, void *store, const void* userData );
	static void parseArmorTemplate( INI *ini, void *instance, void *store, const void* userData );
	static void parseDamageFX( INI *ini, void *instance, void *store, const void* userData );
	static void parseWeaponTemplate( INI *ini, void *instance, void *store, const void* userData );
	// parse a duration in msec and convert to duration in frames
	static void parseDurationReal( INI *ini, void *instance, void *store, const void* userData );
	// parse a duration in msec and convert to duration in integral number of frames, (unsignedint) rounding UP
	static void parseDurationUnsignedInt( INI *ini, void *instance, void *store, const void* userData );
	static void parseDurationUnsignedShort( INI *ini, void *instance, void *store, const void *userData );
	// parse acceleration in (dist/sec) and convert to (dist/frame)
	static void parseVelocityReal( INI *ini, void *instance, void *store, const void* userData );
	// parse acceleration in (dist/sec^2) and convert to (dist/frame^2)
	static void parseAccelerationReal( INI *ini, void *instance, void *store, const void* userData );
	// parse angle in degrees and convert to radians
	static void parseAngleReal( INI *ini, void *instance, void *store, const void *userData );
	// note that this parses in degrees/sec, and converts to rads/frame!
	static void parseAngularVelocityReal( INI *ini, void *instance, void *store, const void *userData );
	static void parseDamageTypeFlags(INI* ini, void* instance, void* store, const void* userData);
	static void parseDeathTypeFlags(INI* ini, void* instance, void* store, const void* userData);
	static void parseVeterancyLevelFlags(INI* ini, void* instance, void* store, const void* userData);
	static void parseSoundsList( INI* ini, void *instance, void *store, const void* /*userData*/ );
	
	
	/**
		return the next token. if seps is null (or omitted), the standard seps are used.
		
		this will *never* return null; if there are no more tokens, an exception will be thrown.
	*/
	const char* getNextToken(const char* seps = NULL);

	/**
		just like getNextToken(), except that null is returned if no more tokens are present
		(rather than throwing an exception). usually you should call getNextToken(),
		but for some cases this is handier (ie, parsing a variable-length number of tokens).
	*/
	const char* getNextTokenOrNull(const char* seps = NULL);

	/**
		This is called when the next thing you expect is something like:
			
			Tag:value

		pass "Tag" (without the colon) for 'expected', and you will have the 'value'
		token returned.

		If "Tag" is not the next token, an error is thrown.
	*/
	const char* getNextSubToken(const char* expected);

	/**
		return the next ascii string. this is usually the same the result of getNextToken(),
		except that it allows for quote-delimited strings (eg, "foo bar"), so you can
		get strings with spaces, and/or empty strings.
	*/
	AsciiString getNextAsciiString();
	AsciiString getNextQuotedAsciiString();	//fixed version of above.  We can't fix the regular one for fear of breaking existing code. :-(

	/**
		utility routine that does a sscanf() on the string to get the Science, and throws
		an exception if not of the right form.
	*/
	static ScienceType scanScience(const char* token);

	/**
		utility routine that does a sscanf() on the string to get the int, and throws
		an exception if not of the right form.
	*/
	static Int scanInt(const char* token);

	/**
		utility routine that does a sscanf() on the string to get the unsigned int, and throws
		an exception if not of the right form.
	*/
	static UnsignedInt scanUnsignedInt(const char* token);

	/**
		utility routine that does a sscanf() on the string to get the real, and throws
		an exception if not of the right form.
	*/
	static Real scanReal(const char* token);
	static Real scanPercentToReal(const char* token);

	static Int scanIndexList(const char* token, ConstCharPtrArray nameList);
	static Int scanLookupList(const char* token, ConstLookupListRecArray lookupList);

	static Bool scanBool(const char* token);

protected:

	static Bool isValidINIFilename( const char *filename ); ///< is this a valid .ini filename		

	void prepFile( AsciiString filename, INILoadType loadType );
	void unPrepFile();

	void readLine( void );

//	FILE *m_file;															///< file pointer of file currently loading
	File *m_file;															///< file pointer of file currently loading
	AsciiString m_filename;										///< filename of file currently loading
	INILoadType m_loadType;										///< load time for current file
	UnsignedInt m_lineNum;										///< current line number that's been read
	char m_buffer[ INI_MAX_CHARS_PER_LINE ];	///< buffer to read file contents into
	const char *m_seps;												///< for strtok parsing
	const char *m_sepsPercent;								///< m_seps with percent delimiter as well
	const char *m_sepsColon;									///< m_seps with colon delimiter as well
	const char *m_sepsQuote;									///< token to represent a quoted ascii string
	const char *m_blockEndToken;							///< token to represent end of data block
	Bool m_endOfFile;													///< TRUE when we've hit EOF
#if defined(_DEBUG) || defined(_INTERNAL)
	char m_curBlockStart[ INI_MAX_CHARS_PER_LINE ];	///< first line of cur block
#endif
};

#endif // __INI_H_


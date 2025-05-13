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

//----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright(C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: GameText.cpp
//
// Created:   11/07/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes                                                      
//----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <string>

#include <Lib/BaseType.h>
#include "GameText.h"

#define DEBUG_LOG(x) {}
#define DEBUG_ASSERTCRASH(x, y) {}

//#include <Common/Language.h>
//#include <Common/Debug.h>
//#include <Common/std::wstring.h>
//#include <Common/std::string.h>
//#include <Common/GlobalData.h>
#include "WSYS_file.h"
#include "WSYS_RAMFile.h"



//----------------------------------------------------------------------------
//         Externals                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines                                                         
//----------------------------------------------------------------------------

#define CSF_ID ( ('C'<<24) | ('S'<<16) | ('F'<<8) | (' ') )
#define CSF_LABEL ( ('L'<<24) | ('B'<<16) | ('L'<<8) | (' ') )
#define CSF_STRING ( ('S'<<24) | ('T'<<16) | ('R'<<8) | (' ') )
#define CSF_STRINGWITHWAVE ( ('S'<<24) | ('T'<<16) | ('R'<<8) | ('W') )
#define CSF_VERSION 3

#define STRING_FILE 0
#define CSF_FILE 1
#define MAX_UITEXT_LENGTH (10*1024)
//----------------------------------------------------------------------------
//         Private Types                                                     
//----------------------------------------------------------------------------

//===============================
// StringInfo 
//===============================

struct StringInfo
{
	std::string			label;
	std::wstring		text;
	std::string			speech;
};

struct StringLookUp
{
	std::string		*label;
	StringInfo		*info;
};

//===============================
// CSFHeader 
//===============================

struct CSFHeader
{
	Int id;
	Int version;
	Int num_labels;
	Int num_strings;
	Int skip;
	Int langid;

};

//===============================
// struct NoString 
//===============================

struct NoString
{
	struct NoString *next;
	std::wstring text;
};


//===============================
// GameTextManager 
//===============================

class GameTextManager : public GameTextInterface
{
	public:

		GameTextManager();
		virtual ~GameTextManager();

		virtual void					init( void );						///< Initlaizes the text system
		virtual void					deinit( void );					///< De-initlaizes the text system
		virtual void					update( void ) {};			///< update text manager
		virtual void					reset( void );					///< Resets the text system

		virtual const wchar_t * fetch( const Char *label );		///< Returns the associated labeled unicode text
	protected:

		Int							m_textCount;
		Int							m_maxLabelLen;
		Char						m_buffer[MAX_UITEXT_LENGTH];
		Char						m_buffer2[MAX_UITEXT_LENGTH];
		Char						m_buffer3[MAX_UITEXT_LENGTH];
		WideChar				m_tbuffer[MAX_UITEXT_LENGTH*2];
		
		StringInfo			*m_stringInfo;
		StringLookUp		*m_stringLUT;
		Bool						m_initialized;
		Bool						m_jabberWockie;
		Bool						m_munkee;
		NoString				*m_noStringList;
		Int							m_useStringFile;
		std::wstring		m_failed;

		void						stripSpaces ( WideChar *string );
		void						removeLeadingAndTrailing ( Char *m_buffer );
		void						readToEndOfQuote( File *file, Char *in, Char *out, Char *wavefile, Int maxBufLen );
		void						reverseWord ( Char *file, Char *lp );
		void						translateCopy( WideChar *outbuf, Char *inbuf );
		Bool						getStringCount( Char *filename);
		Bool						getCSFInfo ( Char *filename );
		Bool						parseCSF(  Char *filename );
		Bool						parseStringFile( char *filename );
		Bool						readLine( char *buffer, Int max, File *file );
		Char						readChar( File *file );
};

static int _cdecl			compareLUT ( const void *,  const void*);
//----------------------------------------------------------------------------
//         Private Data                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data                                                      
//----------------------------------------------------------------------------

GameTextInterface *TheGameText = NULL;

//----------------------------------------------------------------------------
//         Private Prototypes                                               
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions                                               
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Functions                                                
//----------------------------------------------------------------------------

//============================================================================
// CreateGameTextInterface
//============================================================================

GameTextInterface* CreateGameTextInterface( void )
{
	return new GameTextManager;
}


//============================================================================
// GameTextManager::GameTextManager
//============================================================================

GameTextManager::GameTextManager()
:	m_textCount(0),
	m_maxLabelLen(0),
	m_stringInfo(NULL),
	m_stringLUT(NULL),
	m_initialized(FALSE),
	m_jabberWockie(FALSE),
	m_munkee(FALSE),
	m_noStringList(NULL),
	m_useStringFile(TRUE),
	m_failed(L"***FATAL*** String Manager failed to initilaized properly")
{
}

//============================================================================
// GameTextManager::~GameTextManager
//============================================================================

GameTextManager::~GameTextManager()
{
	deinit();
}							

//============================================================================
// GameTextManager::init
//============================================================================

extern char szArgvPath[];

void GameTextManager::init( void )
{
	const Char *strFile = "autorun.str";
	const Char *csfFile = "autorun.csf";
	Int format;

	Char realStrFile[_MAX_PATH];
	Char realCsfFile[_MAX_PATH];

	strncpy(realStrFile, szArgvPath, _MAX_PATH);
	strncpy(realCsfFile, szArgvPath, _MAX_PATH);

	strncat(realStrFile, strFile, _MAX_PATH - strlen(realStrFile));
	strncat(realCsfFile, csfFile, _MAX_PATH - strlen(realCsfFile));

	if ( m_initialized )
	{
		return;
	}

	m_initialized = TRUE;

	m_maxLabelLen = 0;
	m_jabberWockie = FALSE;
	m_munkee = 	FALSE;

	if ( m_useStringFile && getStringCount( realStrFile) )
	{
		format = STRING_FILE;
	}
	else if ( getCSFInfo ( realCsfFile ) )
	{
		format = CSF_FILE;
	}
	else
	{
		return;
	}

	if( (m_textCount == 0) )
	{
		return;
	}

	//Allocate StringInfo Array

	m_stringInfo = new StringInfo[m_textCount];

	if( m_stringInfo == NULL )
	{
		deinit();
		return;
	}

	if ( format == STRING_FILE )
	{
		if( parseStringFile( realStrFile ) == FALSE )
		{
			deinit();
			return;
		}
	}
	else
	{
		if ( !parseCSF ( realCsfFile ) )
		{
			deinit();
			return;
		}
	}

	m_stringLUT = new StringLookUp[m_textCount];

	StringLookUp *lut = m_stringLUT;
	StringInfo *info = m_stringInfo;

	for ( Int i = 0; i < m_textCount; i++ )
	{
		lut->info = info;
		lut->label = &info->label;
		lut++;
		info++;
	}

	qsort( m_stringLUT, m_textCount, sizeof(StringLookUp), compareLUT  );

}

//============================================================================
// GameTextManager::deinit
//============================================================================

void GameTextManager::deinit( void )
{

	if( m_stringInfo != NULL )
	{
		delete [] m_stringInfo;
		m_stringInfo = NULL;
	}

	if( m_stringLUT != NULL )
	{
		delete [] m_stringLUT;
		m_stringLUT = NULL;
	}

	m_textCount = 0;

	NoString *noString = m_noStringList;

	DEBUG_LOG(("\n*** Missing strings ***\n"));
	while ( noString )
	{
		DEBUG_LOG(("*** %ls ***\n", noString->text.str()));
		NoString *next = noString->next;
		delete noString;
		noString = next;
	}
	DEBUG_LOG(("*** End missing strings ***\n\n"));

	m_noStringList = NULL;

	m_initialized = FALSE;
}

//============================================================================
// GameTextManager::reset
//============================================================================

void GameTextManager::reset( void )
{
}


//============================================================================
// GameTextManager::stripSpaces 
//============================================================================

void GameTextManager::stripSpaces ( WideChar *string )
{
	WideChar *str, *ptr;
	WideChar ch, last = 0;
	Int skipall = TRUE;

	str = ptr = string;

	while ( (ch = *ptr++) != 0 )
	{
		if ( ch == ' '  )
		{
			if ( last == ' ' || skipall )
			{
				continue;
			}
		}

		if ( ch == '\n' || ch == '\t' )
		{
				// remove last space
				if ( last == ' ' )
				{
					str--;
				}

				skipall = TRUE;		// skip all spaces
				last = *str++ = ch;
				continue;
		}

		last = *str++ = ch;
		skipall = FALSE;
	}

	if ( last == ' ' )
	{
		str--;
	}

	*str = 0;
}

//============================================================================
// GameTextManager::removeLeadingAndTrailing 
//============================================================================

void GameTextManager::removeLeadingAndTrailing ( Char *buffer )
{
	Char *first, *ptr;
	Char ch;

	ptr = first = buffer;

	while ( (ch = *first) != 0 && iswspace ( ch ))
	{
			first++;
	}

	while ( (*ptr++ = *first++) != 0 );

	ptr -= 2;;

	while ( (ptr > buffer) && (ch = *ptr) != 0 && iswspace ( ch ) )
	{
		ptr--;
	}

	ptr++;
	*ptr = 0;
}

//============================================================================
// GameTextManager::readToEndOfQuote
//============================================================================

void GameTextManager::readToEndOfQuote( File *file, Char *in, Char *out, Char *wavefile, Int maxBufLen )
{
	Int slash = FALSE;
	Int state = 0;
	Int line_start = FALSE;
	Char ch;
	Int ccount = 0;
	Int len = 0;
	Int done = FALSE;

	while ( maxBufLen )
	{
		// get next Char

		if ( in )
		{
			if ( (ch = *in++) == 0 )
			{
				in = NULL; // have exhausted the input m_buffer
				ch = readChar ( file );
			}
		}
		else
		{
			ch = readChar ( file );
		}

		if ( ch == EOF )
		{
			return ;
		}

		if ( ch == '\n' )
		{
			line_start = TRUE;
			slash = FALSE;
			ccount = 0;
			ch = ' ';
		}
		else if ( ch == '\\' && !slash)
		{
			slash = TRUE;
		}
		else if ( ch == '\\' && slash)
		{
			slash = FALSE;
		}
		else if ( ch == '"' && !slash )
		{
			break; // done
		}
		else
		{
			slash = FALSE;
		}

		if ( iswspace ( ch ))
		{
			ch = ' ';
		}

		*out++ = ch;
		maxBufLen--;
	}

	*out = 0;

	while ( !done )
	{
		// get next Char

		if ( in )
		{
			if ( (ch = *in++) == 0 )
			{
				in = NULL; // have exhausted the input m_buffer
				ch = readChar ( file );
			}
		}
		else
		{
			ch = readChar ( file );
		}

		if ( ch == '\n' || ch == EOF )
		{
			break;
		}

		switch ( state )
		{

			case 0:
				if ( iswspace ( ch ) || ch == '=' )
				{
					break;
				}

				state = 1;
			case 1:
				if ( ( ch >= 'a' && ch <= 'z') || ( ch >= 'A' && ch <='Z') || (ch >= '0' && ch <= '9') || ch == '_' )
				{
					*wavefile++ = ch;
					len++;
					break;
				}
				state = 2;
			case 2:
				break;
		}
	}

	*wavefile = 0;

	if ( len )
	{
		if ( ( ch = *(wavefile-1)) >= '0' && ch <= '9' )
		{
			*wavefile++ = 'e';
			*wavefile = 0;
		}
	}

}


//============================================================================
// GameTextManager::reverseWord 
//============================================================================

void GameTextManager::reverseWord ( Char *file, Char *lp )
{
	Int first = TRUE;
	Char f, l;
	Int ok = TRUE	;

	while ( ok )
	{
		if ( file >= lp )
		{
			return;
		}

		f = *file;
		l = *lp;

		if ( first )
		{
			if ( f >= 'A' && f <= 'Z' )
			{
				if ( l >= 'a' && l <= 'z' )
				{
					f = (f - 'A') + 'a';
					l = (l - 'a') + 'A';
				}
			}

			first = FALSE;
		}

		*lp-- = f;
		*file++ = l;

	}

}

//============================================================================
// GameTextManager::translateCopy
//============================================================================

void GameTextManager::translateCopy( WideChar *outbuf, Char *inbuf )
{
	Bool slash = FALSE;

	if ( m_jabberWockie )
	{
		static Char buffer[MAX_UITEXT_LENGTH*2];
		Char *firstLetter = NULL, *lastLetter;
		Char *b = buffer;
		Int formatWord = FALSE;
		Char ch;

		while ( (ch = *inbuf++) != 0 )
		{
			if ( ! (( ch >= 'a' && ch <= 'z') || ( ch >= 'A' && ch <= 'Z' )))
			{
				if ( firstLetter )
				{
					if ( !formatWord )
					{
						lastLetter = b-1;
						reverseWord ( firstLetter, lastLetter );
					}
					firstLetter = NULL;
					formatWord = FALSE;
				}
				*b++ = ch;
				if ( ch == '\\' )
				{
					*b++ = *inbuf++;
				}
				if ( ch == '%' )
				{
					while ( (ch = *inbuf++) != 0 && !( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')))
					{
						*b++ = ch;
					}
					*b++ = ch;
				}
			}
			else
			{
				if ( !firstLetter )
				{
					firstLetter = b;
				}

				*b++ = ch;

			}
		}

		if ( firstLetter )
		{
			lastLetter = b-1;
			reverseWord ( firstLetter, lastLetter );
		}

		*b++ = 0;
		inbuf = buffer;
	}
	else if( m_munkee )
	{
		wcscpy(outbuf, L"Munkee");
		return;
	}

	while( *inbuf != '\0' )
	{
		if( slash == TRUE )
		{
			slash = FALSE;

			switch( *inbuf )
			{
				// in case end of string is reached
				// should never happen!!!
				case '\0':
					return;

				case '\\':
					*outbuf++ = '\\';
					break;

				case '\'':
					*outbuf++ = '\'';
					break;

				case '\"':
					*outbuf++ = '\"';
					break;

				case '\?':
					*outbuf++ = '\?';
					break;

				case 't':
					*outbuf++ = '\t';
					break;

				case 'n':
					*outbuf++ = '\n';
					break;

				default:
					*outbuf++ = *inbuf & 0x00FF;
					break;
			}
		}
		else if( *inbuf != '\\' )
		{
			*outbuf++ = *inbuf & 0x00FF;
		}
		else
			slash = TRUE;

		inbuf++;
	}
	*outbuf= 0;
}

//============================================================================
// GameTextManager::getStringCount
//============================================================================

Bool GameTextManager::getStringCount( char *filename )
{
	Int ok = TRUE;

	m_textCount = 0;

	RAMFile file;
	
	if ( !file.open( filename, File::READ | File::TEXT ))
	{
		return FALSE;
	}

	while(ok)
	{
		if( !readLine( m_buffer, sizeof( m_buffer) -1, &file ) )
			break;
		removeLeadingAndTrailing ( m_buffer );

		if( m_buffer[0] == '"' )
		{
				Int len = strlen(m_buffer);
				m_buffer[ len ] = '\n';
				m_buffer[ len+1] = 0;
			readToEndOfQuote( &file, &m_buffer[1], m_buffer2, m_buffer3, MAX_UITEXT_LENGTH );
		}
		else if( !stricmp( m_buffer, "END") )
		{
			m_textCount++;
		}
	}

	m_textCount += 500;
	file.close();
	return TRUE;
}

//============================================================================
// GameTextManager::getCSFInfo 
//============================================================================

Bool GameTextManager::getCSFInfo ( Char *filename )
{
	CSFHeader header;
	Int ok = FALSE;
	RAMFile file;

	if ( file.open( filename, File::READ | File::BINARY ) )
	{
		if ( file.read( &header, sizeof ( header )) == sizeof ( header ) )
		{
			if ( header.id == CSF_ID )
			{
				m_textCount = header.num_labels;

				ok = TRUE;
			}
		}

		file.close();
	}

	return ok;
}

//============================================================================
// GameTextManager::parseCSF
//============================================================================

Bool GameTextManager::parseCSF( Char *filename )
{
	RAMFile file;
	Int id;
	Int len;
	Int listCount = 0;
	Bool ok = FALSE;
	CSFHeader header;

	if ( !file.open( filename, File::READ | File::BINARY ) )
	{
		return FALSE;
	}

	if (  file.read ( &header, sizeof ( CSFHeader)) != sizeof ( CSFHeader) )
	{
		return FALSE;
	}

	while( file.read ( &id, sizeof (id)) == sizeof ( id) )
	{
		Int num;
		Int num_strings;

		if ( id != CSF_LABEL )
		{
			goto quit;
		}

		file.read ( &num_strings, sizeof ( Int ));

		file.read ( &len, sizeof ( Int ) );

		if ( len )
		{
			file.read ( m_buffer, len );
		}

		m_buffer[len] = 0;

		m_stringInfo[listCount].label = m_buffer;


		if ( len > m_maxLabelLen )
		{
			m_maxLabelLen = len;
		}

		num = 0;

		while ( num < num_strings )
		{
		 	file.read ( &id, sizeof ( Int ) );

			if ( id != CSF_STRING && id != CSF_STRINGWITHWAVE )
			{
				goto quit;
			}

		 	file.read ( &len, sizeof ( Int ) );

			if ( len )
			{
				file.read ( m_tbuffer, len*sizeof(WideChar) );
			}

			if ( num == 0 )
			{
				// only use the first string found
				m_tbuffer[len] = 0;
				
				{
					WideChar *ptr;
				
					ptr = m_tbuffer;
				
					while ( *ptr )
					{
						*ptr = ~*ptr;
						ptr++;
					}
				}
				
				stripSpaces ( m_tbuffer );
				m_stringInfo[listCount].text = m_tbuffer;
			}

			if ( id == CSF_STRINGWITHWAVE )
			{
			 	file.read ( &len, sizeof ( Int ) );
				if ( len )
				{
					file.read ( m_buffer, len );
				}
				m_buffer[len] = 0;

				if ( num == 0 && len )
				{
					// only use the first string found
					m_stringInfo[listCount].speech = m_buffer;
				}

			}

			num++;
		}

		listCount++;
	}

	ok = TRUE;

quit:

	file.close();

	return ok;
}


//============================================================================
// GameTextManager::parseStringFile
//============================================================================

Bool GameTextManager::parseStringFile( char *filename )
{
	Int listCount = 0;
	Int ok = TRUE;

	RAMFile file;
	
	if ( !file.open(  filename, File::READ | File::TEXT ) )
	{
		return FALSE;
	}

	while( ok )
	{
		Int len;
		if( !readLine( m_buffer, MAX_UITEXT_LENGTH, &file ))
		{
			break;
		}

		removeLeadingAndTrailing ( m_buffer );

		if( ( *(unsigned short *)m_buffer == 0x2F2F) || !m_buffer[0])			//	0x2F2F is Hex for //
			continue;

		// make sure label is unique

		for ( Int i = 0; i < listCount; i++ )
		{
			if ( !stricmp ( m_stringInfo[i].label.c_str(), m_buffer ))
			{
				DEBUG_ASSERTCRASH ( FALSE, ("String label '%s' multiply defined!", m_buffer ));
			}
		}

		m_stringInfo[listCount].label = m_buffer;
		len = strlen ( m_buffer );


		if ( len > m_maxLabelLen )
		{
			m_maxLabelLen = len;
		}

		Bool readString = FALSE;
		while( ok )
		{
			if (!readLine ( m_buffer, sizeof(m_buffer)-1, &file ))
			{
				DEBUG_ASSERTCRASH (FALSE, ("Unexpected end of string file"));
				ok = FALSE;
				goto quit;
			}

			removeLeadingAndTrailing ( m_buffer );

			if( m_buffer[0] == '"' )
			{
				len = strlen(m_buffer);
				m_buffer[ len ] = '\n';
				m_buffer[ len+1] = 0;
				readToEndOfQuote( &file, &m_buffer[1], m_buffer2, m_buffer3, MAX_UITEXT_LENGTH );


				if ( readString )
				{
					// only one string per label allows
						DEBUG_ASSERTCRASH ( FALSE, ("String label '%s' has more than one string defined!", m_stringInfo[listCount].label.str()));
				}
				else
				{
					// Copy string into new home
					translateCopy( m_tbuffer, m_buffer2 );
					stripSpaces ( m_tbuffer );
					
					m_stringInfo[listCount].text = m_tbuffer ;
					m_stringInfo[listCount].speech = m_buffer3;
					readString = TRUE;
				}
			}
			else if ( !stricmp ( m_buffer, "END" ))
			{
				break;
			}
		}

		listCount++;
	}

quit:

	file.close();

	return ok;
}

//============================================================================
// *GameTextManager::fetch
//============================================================================

const wchar_t * GameTextManager::fetch( const Char *label )
{
	DEBUG_ASSERTCRASH ( m_initialized, ("String Manager has not been m_initialized") );

	if( m_stringInfo == NULL )
	{
		return m_failed.c_str();
	}

	StringLookUp *lookUp;
	StringLookUp key;
	std::string lb;
	lb = label;
	key.info = NULL;
	key.label = &lb;

	lookUp = (StringLookUp *) bsearch( &key, (void*) m_stringLUT, m_textCount, sizeof(StringLookUp), compareLUT );

	if( lookUp == NULL )
	{
		// See if we already have the missing string
		wchar_t tmp[256];
		_snwprintf(tmp, 256, L"MISSING: '%hs'", label);
		tmp[255] = 0;
		std::wstring missingString = tmp;

		NoString *noString = m_noStringList;

		while ( noString )
		{
			if (noString->text == missingString)
				return missingString.c_str();

			noString = noString->next;
		}

		//DEBUG_LOG(("*** MISSING:'%s' ***\n", label));
		// Remember file could have been altered at this point.
		noString = new NoString;
		noString->text = missingString;
		noString->next = m_noStringList;
		m_noStringList = noString;
		return noString->text.c_str();
	}
	return lookUp->info->text.c_str();
}

//============================================================================
// GameTextManager::readLine
//============================================================================

Bool	GameTextManager::readLine( char *buffer, Int max, File *file )
{
	Int ok = FALSE;

	while ( max && file->read( buffer, 1 ) == 1 )
	{
		ok = TRUE;

		if ( *buffer == '\n' )
		{
			break;
		}

		buffer++;
		max--;
	}

	*buffer = 0;

	return ok;
}

//============================================================================
// GameTextManager::readChar
//============================================================================

Char	GameTextManager::readChar( File *file )
{
	Char ch;

	if ( file->read( &ch, 1 ) == 1 )
	{
		return ch;
	}

	return 0;
}

//============================================================================
// GameTextManager::compareLUT 
//============================================================================

static int __cdecl compareLUT ( const void *i1,  const void*i2)
{
	StringLookUp *lut1 = (StringLookUp*) i1;
	StringLookUp *lut2 = (StringLookUp*) i2;

	return stricmp( lut1->label->c_str(), lut2->label->c_str());
}

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

/* Copyright (C) Electronic Arts Canada Inc. 1998-1999.  All rights reserved. */

#include <string.h>
#include <assert.h>
#include "gimex.h"      /* for file and memory IO only */
#include "locale.h"
#include "Wnd_File.h"

/*************************************************************************/
/* File Format Structures                                                */
/*************************************************************************/

#define LOCALEFILE_HEADERCHUNKID    0x48434f4c /* 'LOCH' */
#define LOCALEFILE_INDEXCHUNKID     0x49434f4c /* 'LOCI' */
#define LOCALEFILE_LANGUAGECHUNKID  0x4c434f4c /* 'LOCL' */

typedef struct
{
    unsigned int   ChunkID;        /* 'LOCH' LOCALEFILE_HEADERCHUNKID */
    unsigned int   ChunkSize;      /* size of chunk in bytes */
    unsigned int   Flags;          /* 0=no index chunk present,1=index chunk present */
    unsigned int   LanguageCount;  /* number of language chunks in this file */
/*  unsigned int   LanguageOffset[LanguageCount]; \\ offsets in bytes from start of file to language chunk */
} LOCALEFILE_HEADERCHUNK;

/* offset LOCALEFILE_HEADERCHUNK_LANGUAGE_OFFSET bytes from the start of the chunk to the language offset table */
#define LOCALEFILE_HEADERCHUNK_LANGUAGE_OFFSET  sizeof(LOCALEFILE_HEADERCHUNK)


typedef struct
{
    unsigned int   ChunkID;        /* 'LOCI' LOCALEFILE_INDEXCHUNKID */
    unsigned int   ChunkSize;      /* size of chunk in bytes */
    unsigned int   StringCount;    /* number of string ids in this chunk (same value in all language chunks) */
    unsigned int   pad;            /* must be zero */
/*  STRINGID StringID[StringCount];  */
/*  { */
/*      unsigned short ID;          \\ id that user gives to look up value */
/*      unsigned short Index;       \\ index to look up value in language chunks */
/*  } */
} LOCALEFILE_INDEXCHUNK;

/* offset LOCALEFILE_INDEXCHUNK_STRINGID_OFFSET bytes from the start of the chunk to the string id table */
#define LOCALEFILE_INDEXCHUNK_STRINGID_OFFSET   sizeof(LOCALEFILE_INDEXCHUNK)


typedef struct
{
    unsigned int   ChunkID;        /* 'LOCL' LOCALEFILE_LANGUAGECHUNKID */
    unsigned int   ChunkSize;      /* size of chunk in bytes including this header and all string data */
    unsigned int   LanguageID;     /* language strings are in for this bank */
    unsigned int   StringCount;    /* number of strings in this chunk */
/*  unsigned int   StringOffset[StringCount];   \\ offsets in bytes from start of chunk to string */
/*  const char*    Data[StringCount];           \\ StringCount null terminated strings */
} LOCALEFILE_LANGUAGECHUNK;

/* offset LOCALEFILE_LANGUAGECHUNK_STRING_OFFSETbytes from the start of the chunk to the string offset table */
#define LOCALEFILE_LANGUAGECHUNK_STRING_OFFSET   sizeof(LOCALEFILE_LANGUAGECHUNK)


/*************************************************************************/
/* LOCALE_INSTANCE declaration                                           */
/*************************************************************************/

typedef LOCALEFILE_HEADERCHUNK   HEADER;
typedef LOCALEFILE_INDEXCHUNK    INDEX;
typedef LOCALEFILE_LANGUAGECHUNK BANK;

typedef struct
{
    int     BankIndex;     /* current language bank set (0..BANK_COUNT-1) */
    BANK*   pBank[LOCALE_BANK_COUNT];  /* array of string banks */
    INDEX*  pIndex[LOCALE_BANK_COUNT]; /* array of string indices */
} LOCALE_INSTANCE;

static LOCALE_INSTANCE	*lx			= nullptr;

/*************************************************************************/
/* initialization/restore                                                */
/*************************************************************************/

/* helper function to make assertions for initialization clearer */
int LOCALE_isinitialized( void )
{
    if ( lx == nullptr ) {
//        TRACE("LOCALE API is not initialized - call LOCALE_init before calling LOCALE functions\n");
	}
    return( lx != nullptr );
}

/*
;
; ABSTRACT
;
; LOCALE_init - Init the localization module
;
;
; SUMMARY
;
; #include "realfont.h"
;
; int LOCALE_init(void)
;
; DESCRIPTION
;
; Initilizes everything needed to use the locale API.  Can only be called
; once until LOCALE_restore is called.
;
; Returns non-zero if everything went ok.
;
; SEE ALSO
;
; LOCALE_restore
;
;
; EXAMPLE
;
; locale_eg.c
;
; <HTML A HREF="locale_eg.c">Download the source<HTML /A>
; <HTML A HREF="locale_eg.csv">locale_eg.csv<HTML /A> (example data)
;
;
; END ABSTRACT
;
*/

int LOCALE_init(void)
{
    int ok = 0;

    /* ensure locale module is NOT already initialized */
    ASSERT(lx == nullptr); /* can only call LOCALE_init after a restore or once, cannot double init locale API */

    /* allocate instance */
    lx = (LOCALE_INSTANCE*)galloc(sizeof(LOCALE_INSTANCE));
    if (lx != nullptr) {
        memset(lx, 0, sizeof(LOCALE_INSTANCE));
        ok = 1;
    }
    return ok;
}

/*
;
; ABSTRACT
;
; LOCALE_restore - Free resources used by the locale module
;
;
; SUMMARY
;
; #include "realfont.h"
;
; void LOCALE_restore(void)
;
; DESCRIPTION
;
; Restores all resources used by the locale API.  Can only be called after
; LOCALE_init, and only once.
;
; SEE ALSO
;
; LOCALE_init
;
;
; EXAMPLE
;
; locale_eg.c
;
; <HTML A HREF="locale_eg.c">Download the source<HTML /A>
; <HTML A HREF="locale_eg.csv">locale_eg.csv<HTML /A> (example data)
;
;
; END ABSTRACT
;
*/

void LOCALE_restore(void)
{
    int i;

	if( lx != nullptr ) {

	    ASSERT(LOCALE_isinitialized()); /* must call LOCALE_init before calling this function */
		ASSERT(lx != nullptr);

		/* free any language tables */
		for (i = 0; i < LOCALE_BANK_COUNT; i++) {
			if (lx->pBank[i]) {
				LOCALE_setbank(i);
				LOCALE_freetable();
			}
		}

		/* free instance */
		gfree(lx);
		lx = nullptr;
	}
}

/*************************************************************************/
/* attributes                                                            */
/*************************************************************************/

/*
;
; ABSTRACT
;
; LOCALE_setbank - Set the current bank
;
;
; SUMMARY
;
; #include "realfont.h"
;
; void LOCALE_setbank(BankIndex)
;   int BankIndex;         Number between 0 and LOCALE_BANK_COUNT - 1
;
; DESCRIPTION
;
; Sets the current bank to be active.  All functions will now use this
; bank for strings.  A bank is slot where a string table is loaded.
; More than one bank can have a table loaded but the locale functions
; only work on one bank at a time, the active bank set by this function.
;
; SEE ALSO
;
; LOCALE_getbank
;
;
; EXAMPLE
;
; locale_eg.c
;
; <HTML A HREF="locale_eg.c">Download the source<HTML /A>
; <HTML A HREF="locale_eg.csv">locale_eg.csv<HTML /A> (example data)
;
;
; END ABSTRACT
;
*/

void LOCALE_setbank(int BankIndex)
{
    ASSERT(LOCALE_isinitialized()); /* must call LOCALE_init before calling this function */
    lx->BankIndex = BankIndex;
}

/*
;
; ABSTRACT
;
; LOCALE_getbank - Get the current bank
;
;
; SUMMARY
;
; #include "realfont.h"
;
; int LOCALE_getbank(void)
;
; DESCRIPTION
;
; Returns the bank index of the current bank.
;
; SEE ALSO
;
; LOCALE_setbank
;
;
; EXAMPLE
;
; locale_eg.c
;
; <HTML A HREF="locale_eg.c">Download the source<HTML /A>
; <HTML A HREF="locale_eg.csv">locale_eg.csv<HTML /A> (example data)
;
;
; END ABSTRACT
;
*/

int LOCALE_getbank(void)
{
    ASSERT(LOCALE_isinitialized()); /* must call LOCALE_init before calling this function */
    return lx->BankIndex;
}


/*
;
; ABSTRACT
;
; LOCALE_getbanklanguageid - Get the language id for the current bank
;
;
; SUMMARY
;
; #include "realfont.h"
;
; int LOCALE_getbanklanguageid(void)
;
; DESCRIPTION
;
; Returns the language id of the current bank.  This id will match
; the lanugage id in the header file generated by Locomoto
;
; SEE ALSO
;
; LOCALE_loadtable
;
; EXAMPLE
;
; locale_eg.c
;
; <HTML A HREF="locale_eg.c">Download the source<HTML /A>
; <HTML A HREF="locale_eg.csv">locale_eg.csv<HTML /A> (example data)
;
;
; END ABSTRACT
;
*/

int LOCALE_getbanklanguageid(void)
{
    ASSERT(LOCALE_isinitialized()); /* must call LOCALE_init before calling this function */
    ASSERT(lx->pBank[lx->BankIndex]);       /* must load a table into bank before calling this function */
    return (int)(lx->pBank[lx->BankIndex]->LanguageID);
}

/*
;
; ABSTRACT
;
; LOCALE_getbankstringcount - Get the string count for the current bank
;
;
; SUMMARY
;
; #include "realfont.h"
;
; int LOCALE_getbankstringcount(void)
;
; DESCRIPTION
;
; Returns the number of strings in the current bank.  If zero is
; returned then this bank is empty.
;
; SEE ALSO
;
; LOCALE_loadtable
;
; EXAMPLE
;
; locale_eg.c
;
; <HTML A HREF="locale_eg.c">Download the source<HTML /A>
; <HTML A HREF="locale_eg.csv">locale_eg.csv<HTML /A> (example data)
;
;
; END ABSTRACT
;
*/

int LOCALE_getbankstringcount(void)
{
    int StringCount = 0;

    ASSERT(LOCALE_isinitialized()); /* must call LOCALE_init before calling this function */
    if (lx->pBank[lx->BankIndex]) {
        StringCount = lx->pBank[lx->BankIndex]->StringCount;
    }
    return StringCount;
}

/*************************************************************************/
/* operations                                                            */
/*************************************************************************/

/*
;
; ABSTRACT
;
; LOCALE_loadtable - Load a string table into the current bank
;
;
; SUMMARY
;
; #include "realfont.h"
;
; int LOCALE_loadtable(pathname, languageid)
;
;   const char* pathname; // pathname of .loc file to load
;   int languageid;       // language id to load (from .h file)
;
; DESCRIPTION
;
; Loads the specified language from the string file into the
; current bank.  Returns non-zero if the operation was successful.
; The bank must be free before you can call LOCALE_loadtable.  To
; free a bank use the LOCALE_freetable function.  To determine
; if the bank is free use the LOCALE_getbankstringcount function.
;
; The languageid value is available in the .h file created by
; locomoto.
;
; Returns non-zero if everthing is ok.
;
; SEE ALSO
;
; LOCALE_freetable, LOCALE_getbankstringcount
;
; EXAMPLE
;
; locale_eg.c
;
; <HTML A HREF="locale_eg.c">Download the source<HTML /A>
; <HTML A HREF="locale_eg.csv">locale_eg.csv<HTML /A> (example data)
;
;
; END ABSTRACT
;
*/

static int readheader( GSTREAM* g )
{
    int ok = 0;

    /* read file header */
    LOCALEFILE_HEADERCHUNK header;
    int HeaderChunkSize = sizeof(LOCALEFILE_HEADERCHUNK);

//	VERIFY(gread(g, &header, HeaderChunkSize) == HeaderChunkSize);
    if( gread(g, &header, HeaderChunkSize) != HeaderChunkSize ) {
		return ok;
	}

	Msg( __LINE__, __FILE__, "readheader - HeaderChunkSize = %d.", HeaderChunkSize );
	Msg( __LINE__, __FILE__, "readheader - header.LanguageCount = %d.", header.LanguageCount );
	Msg( __LINE__, __FILE__, "readheader - header.Flags = %d.", header.Flags );

    ASSERT( header.ChunkID == LOCALEFILE_HEADERCHUNKID ); /* ensure that this is a valid .loc file */

    /* read index chunk if present */
    if ( header.Flags == 1 ) {

        int IndexChunkSize;
        int IndexChunkPos = header.ChunkSize;

        /* read index chunk size */
//		VERIFY(gseek(g, IndexChunkPos + 4));
        if( !gseek( g, IndexChunkPos + 4)) {
			return ok;
		}

		Msg( __LINE__, __FILE__, "readheader - seek to = %d.", IndexChunkPos + 4 );

//		VERIFY(gread(g, &IndexChunkSize, 4) == 4);
        if( gread( g, &IndexChunkSize, 4) != 4 ) {
			return ok;
		}

		Msg( __LINE__, __FILE__, "readheader - IndexChunkSize = %d.", IndexChunkSize );

        /* alloc and read index chunk */
        lx->pIndex[lx->BankIndex] = (LOCALEFILE_INDEXCHUNK *)galloc((long)IndexChunkSize );
        if (lx->pIndex[lx->BankIndex]) {

//			VERIFY(gseek(g, IndexChunkPos));
            gseek( g, IndexChunkPos );

			Msg( __LINE__, __FILE__, "readheader - seek to = %d.", IndexChunkPos );

//			VERIFY(gread(g, lx->pIndex[lx->BankIndex], IndexChunkSize) == IndexChunkSize);
            if ( gread(g, lx->pIndex[lx->BankIndex], IndexChunkSize ) != IndexChunkSize ) {
				return ok;
			}
			Msg( __LINE__, __FILE__, "readheader - IndexChunkSize = %d.", IndexChunkSize );

            ASSERT( lx->pIndex[lx->BankIndex]->ChunkID == LOCALEFILE_INDEXCHUNKID );

			ok = 1;
        }
    }
	Msg( __LINE__, __FILE__, "readheader - exiting." );

	return ok;
}

static int readstrings( GSTREAM* g, int LanguageID )
{
	Msg( __LINE__, __FILE__, "readstrings:: g ok? %d.", ((g!= nullptr)?1:0));

    int ok = 0;

    int LanguageChunkOffsetPos = 16 + LanguageID*4;
    int LanguageChunkPos = 0;
    int LanguageChunkSize = -1;

    /* read offset to language chunk */
//	VERIFY(gseek(g, (int)LanguageChunkOffsetPos));
//	VERIFY(gread(g, &LanguageChunkPos, 4) == 4);
    if( !gseek( g, (int)LanguageChunkOffsetPos )) {
		return ok;
	}
    if( gread( g, &LanguageChunkPos, 4 ) != 4 ) {
		return ok;
	}

    /* read language chunk size */
//	VERIFY(gseek(g, LanguageChunkPos + 4));
//	VERIFY(gread(g, &LanguageChunkSize, 4) == 4);
    if( !gseek( g, LanguageChunkPos + 4 )) {
		return ok;
	}
	if( gread( g, &LanguageChunkSize, 4 ) != 4 ) {
		return ok;
	}

	Msg( __LINE__, __FILE__, "readstrings::LanguageChunkOffsetPos = %d.", LanguageChunkOffsetPos );
	Msg( __LINE__, __FILE__, "readstrings::LanguageChunkPos = %d.", LanguageChunkPos );
	Msg( __LINE__, __FILE__, "readstrings::LanguageChunkSize = %d.", LanguageChunkSize );

    /* alloc and read language chunk */
    lx->pBank[lx->BankIndex] = (LOCALEFILE_LANGUAGECHUNK *)galloc((long)LanguageChunkSize);
    if (lx->pBank[lx->BankIndex]) {

		Msg( __LINE__, __FILE__, "readstrings:: A." );

//		VERIFY(gseek(g, LanguageChunkPos));
//		VERIFY(gread(g, lx->pBank[lx->BankIndex], LanguageChunkSize) == LanguageChunkSize);
		if( !gseek( g, LanguageChunkPos )) {
			return ok;
		}
		if( gread( g, lx->pBank[lx->BankIndex], LanguageChunkSize ) != LanguageChunkSize ) {
			return ok;
		}

		ASSERT(lx->pBank[lx->BankIndex]->ChunkID == LOCALEFILE_LANGUAGECHUNKID);
        ok = 1;
    }
    return ok;
}

int LOCALE_loadtable(const char* PathName, int LanguageID)
{
    int ok = 0;
    GSTREAM* g;

    ASSERT(LOCALE_isinitialized());             /* must call LOCALE_init before calling this function */
    ASSERT(lx->pBank[lx->BankIndex] == nullptr);   /* bank must be empty before loading a new table */
    ASSERT(lx->pIndex[lx->BankIndex] == nullptr);  /* bank must be empty before loading a new table */

    g = gopen( PathName );
    if( g != nullptr ) {

		Msg( __LINE__, __FILE__, "LOCALE_loadtable-- file opened." );

        if( readheader(g)) {

			Msg( __LINE__, __FILE__, "LOCALE_loadtable-- readstrings." );

		    ok = readstrings( g, LanguageID );

			Msg( __LINE__, __FILE__, "LOCALE_loadtable-- ok = %d ).", ok );
		}
        gclose(g);
    }
    return ok;
}

/*
;
; ABSTRACT
;
; LOCALE_purgetable - OBSOLETE
;
; Make all references to LOCALE_freetable
;
; END ABSTRACT
;
*/

/*
;
; ABSTRACT
;
; LOCALE_freetable - Free the string table in the current bank
;
;
; SUMMARY
;
; #include "realfont.h"
;
; void LOCALE_freetable(void)
;
; DESCRIPTION
;
; Frees the table loaded in the current bank.  There must be a
; table loaded in the current bank.  Use LOCALE_getbankstringcount
; to determine if the bank is free or not.
;
; SEE ALSO
;
; LOCALE_loadtable, LOCALE_getbankstringcount
;
; EXAMPLE
;
; locale_eg.c
;
; <HTML A HREF="locale_eg.c">Download the source<HTML /A>
; <HTML A HREF="locale_eg.csv">locale_eg.csv<HTML /A> (example data)
;
;
; END ABSTRACT
;
*/

void LOCALE_freetable(void)
{
	if( lx != nullptr ) {

		ASSERT(LOCALE_isinitialized());			/* must call LOCALE_init before calling this function */
		ASSERT(lx->pBank[lx->BankIndex]);       /* table must be loaded before calling this function  */

	    /* free string bank */
		gfree(lx->pBank[lx->BankIndex]);
	    lx->pBank[lx->BankIndex] = nullptr;

		/* if the bank has an index loaded, free that as well */
	    if (lx->pIndex[lx->BankIndex]) {
		    gfree(lx->pIndex[lx->BankIndex]);
			lx->pIndex[lx->BankIndex] = nullptr;
	    }
	}
}

/*
;
; ABSTRACT
;
; LOCALE_getstring - Return the specified string from the current bank
;
; SUMMARY
;
; #include "realfont.h"
;
; const char* LOCALE_getstring( StringID )
;   int StringID;   ID of string to return
;
; DESCRIPTION
;
; Returns the string specified from the current bank.  There must
; be a string table loaded into the current bank.  Use the String
; ID specified in the header file created by Locomoto.  Note that
; the string pointer is a const pointer.  Do not modify the string
; or the Locale library may return invalid results.
;
; If the .loc file was created with an index StringID can be any
; valid integer in the range 0..65535.  If no index was created
; with the .loc file StringID will be a zero based array index.
;
; String is returned by const for a reason.  Bad things will happen
; if you modify it.  You have been warned.
;
; SEE ALSO
;
; LOCALE_loadtable, LOCALE_getbankstringcount
;
; EXAMPLE
;
; locale_eg.c
;
; <HTML A HREF="locale_eg.c">Download the source<HTML /A>
; <HTML A HREF="locale_eg.csv">locale_eg.csv<HTML /A> (example data)
;
;
; END ABSTRACT
;
*/

#include <stdlib.h>     // for bsearch function

static int compare ( const void* arg1, const void* arg2 )
{
    const unsigned short* s1 = (const unsigned short*)(arg1);
    const unsigned short* s2 = (const unsigned short*)(arg2);
    return (*s1) - (*s2);
}

static int getstringbyindex( unsigned short key, const INDEX* pIndex )
{
    int index = 0;
    unsigned short* result;
    unsigned char*  base;   /* pointer to base of string id table */

    ASSERT(LOCALE_isinitialized()); /* must call LOCALE_init before calling this function */
    ASSERT(pIndex != nullptr); /* index not loaded - .loc file must have index created (use -i option) */

    base	= ((unsigned char*)pIndex) + LOCALEFILE_INDEXCHUNK_STRINGID_OFFSET;
    result	= (unsigned short*)bsearch((unsigned char *)&key, base, pIndex->StringCount, 4, compare);

    if (result != nullptr) {
        /* index is the second unsigned short */
        ++result;
        index = *result;
    } else {
        index = -1;
    }
    return index;
}

const char* LOCALE_getstring( int StringID )
{
    const char* p;							/* pointer to string, nullptr if string cannot be found */

	Msg( __LINE__, __FILE__, "Locale_getstring::( %d ).", StringID );

    ASSERT( LOCALE_isinitialized());		/* must call LOCALE_init before calling this function */

    /* get string array index from the index if it exists */
    if ( lx->pIndex[ lx->BankIndex ] != nullptr ) {
        StringID = getstringbyindex((unsigned short)StringID, lx->pIndex[lx->BankIndex]);
    }

	Msg( __LINE__, __FILE__, "Locale_getstring::( %d ).", StringID );
	Msg( __LINE__, __FILE__, "Locale_getstring::( lx->BankIndex = %d ).", lx->BankIndex );
	Msg( __LINE__, __FILE__, "Locale_getstring::( lx->pBank[lx->BankIndex]->StringCount = %d ).", lx->pBank[lx->BankIndex]->StringCount );

    if ((StringID >= 0) && (StringID < (int)(lx->pBank[lx->BankIndex]->StringCount ))) {

		Msg( __LINE__, __FILE__, "Locale_getstring:: A" );

        unsigned int offset;

        p = (const char*)(lx->pBank[lx->BankIndex]);

		Msg( __LINE__, __FILE__, "Locale_getstring:: B" );

        offset = *(unsigned int*)(p + LOCALEFILE_LANGUAGECHUNK_STRING_OFFSET + StringID*4);

		Msg( __LINE__, __FILE__, "Locale_getstring:: C" );

        p += offset;

		Msg( __LINE__, __FILE__, "Locale_getstring:: D" );


    } else {
        p = nullptr;
    }

	Msg( __LINE__, __FILE__, L"%s", 1252, (wchar_t *)p );

    return p;
}


/*
;
; ABSTRACT
;
; LOCALE_getstr - return selected string from the specified .loc file
;
;
; SUMMARY
;
; #include "realfont.h"
;
; const char* LOCALE_getstr(stringid)
;
;   int stringid;   // string id to return
;
; DESCRIPTION
;
; Returns the string identified by stringid from the specified
; .loc file.  Use the string ID specified in the header file created
; by Locomoto.  Note that the string pointer is a const pointer.  Do
; not modify the string or the Locale library may return invalid results.
;
; If your strings are Unicode strings, cast the result to a const USTR *.
;
; If the .loc file was created with an index stringid can be any
; valid integer in the range 0..65535.  If no index was created
; with the .loc file stringid will be a zero based array index.
;
; String is returned by const for a reason.  Bad things will happen
; if you modify it.  You have been warned.
;
; SEE ALSO
;
; EXAMPLE
;
; locale_eg.c
;
; <HTML A HREF="locale_eg.c">Download the source<HTML /A>
; <HTML A HREF="locale_eg.csv">locale_eg.csv<HTML /A> (example data)
;
; END ABSTRACT
;
*/

int LOCALElanguageid = 0;

const char* LOCALE_getstr( const void* pLocFile, int StringID )
{
	const char* p; /* pointer to string, nullptr if string cannot be found */

    HEADER* pHeader;
    BANK*   pBank;

    ASSERT(pLocFile != nullptr);

    pHeader = (LOCALEFILE_HEADERCHUNK*)(pLocFile);
    ASSERT(pHeader->ChunkID == LOCALEFILE_HEADERCHUNKID);
    ASSERT(pHeader->LanguageCount >= 1);

    if( pHeader->Flags == 1 ) {

        /* file has an index */
        INDEX* pIndex = (INDEX*)((unsigned char*)(pLocFile) + pHeader->ChunkSize);
        StringID = getstringbyindex((unsigned short)StringID, pIndex);
    }

    /* get pointer to string bank */
    {
        int offset = *((int*)(pLocFile) + 4 + LOCALElanguageid);
        pBank = (BANK*)((unsigned char*)(pLocFile) + offset);
    }

    if ((StringID >= 0) && (StringID < (int)(pBank->StringCount))) {

        unsigned int offset;

        p = (const char*)(pBank);
        offset = *(unsigned int*)(p + LOCALEFILE_LANGUAGECHUNK_STRING_OFFSET + StringID*4);
        p += offset;

    } else {
        p = nullptr;
    }

    return p;
}


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

//
//	ParseStr.h
//
//

#pragma once

#include "olestring.h"
#include "list.h"
#include "bin.h"

class CBabylonDlg;

typedef struct
{
	int numdialog;
	int missing;
	int unresolved;
	int resolved;
	int errors;

} DLGREPORT;

typedef struct
{
	int numlabels;
	int numstrings;
	int missing;
	int retranslate;
	int too_big;
	int errors;
	int bad_format;
	int translated;

} TRNREPORT;

typedef enum
{

	PMASK_NONE				= 0,
	PMASK_MISSING			=	0x00000001,
	PMASK_UNRESOLVED	=	0x00000002,
	PMASK_BADFORMAT		=	0x00000004,
	PMASK_TOOLONG			=	0x00000008,
	PMASK_RETRANSLATE	=	0x00000010,
	PMASK_ALL					=	0xffffffff
} PMASK;

typedef enum
{
	LANGID_US,
	LANGID_BRITISH,
	LANGID_GERMAN,
	LANGID_FRENCH,
	LANGID_SPANISH,
	LANGID_ITALIAN,
	LANGID_JAPANESE,
	LANGID_JABBER,
	LANGID_KOREAN,
	LANGID_CHINESE,
	LANGID_UNKNOWN,
	LANGID_UNUSED_1, // Maybe remove
	LANGID_BRAZILIAN,
	LANGID_POLISH,
	LANGID_RUSSIAN,
	LANGID_ARABIC,
	LANGID_UKRAINIAN,
	LANGID_SWEDISH,
	LANGID_ABKHAZIAN,
	LANGID_AFAR,
	LANGID_AFRIKAANS,
	LANGID_AKAN,
	LANGID_ALBANIAN,
	LANGID_AMHARIC,
	LANGID_ARAGONESE,
	LANGID_ARMENIAN,
	LANGID_ASSAMESE,
	LANGID_AVARIC,
	LANGID_AVESTAN,
	LANGID_AYMARA,
	LANGID_AZERBAIJANI,
	LANGID_BAMBARA,
	LANGID_BASHKIR,
	LANGID_BASQUE,
	LANGID_BELARUSIAN,
	LANGID_BENGALI,
	LANGID_BISLAMA,
	LANGID_BOSNIAN,
	LANGID_BRETON,
	LANGID_BULGARIAN,
	LANGID_BURMESE,
	LANGID_CATALAN,
	LANGID_CHAMORRO,
	LANGID_CHECHEN,
	LANGID_CHICHEWA,
	LANGID_SLAVONIC,
	LANGID_CHUVASH,
	LANGID_CORNISH,
	LANGID_CORSICAN,
	LANGID_CREE,
	LANGID_CROATIAN,
	LANGID_CZECH,
	LANGID_DANISH,
	LANGID_DIVEHI,
	LANGID_DUTCH,
	LANGID_DZONGKHA,
	LANGID_ESPERANTO,
	LANGID_ESTONIAN,
	LANGID_EWE,
	LANGID_FAROESE,
	LANGID_FIJIAN,
	LANGID_FINNISH,
	LANGID_FRISIAN,
	LANGID_FULAH,
	LANGID_GAELIC,
	LANGID_GALICIAN,
	LANGID_GANDA,
	LANGID_GEORGIAN,
	LANGID_GREEK,
	LANGID_KALAALLISUT,
	LANGID_GUARANI,
	LANGID_GUJARATI,
	LANGID_HAITIAN,
	LANGID_HAUSA,
	LANGID_HEBREW,
	LANGID_HERERO,
	LANGID_HINDI,
	LANGID_HIRIMOTU,
	LANGID_HUNGARIAN,
	LANGID_ICELANDIC,
	LANGID_IDO,
	LANGID_IGBO,
	LANGID_INDONESIAN,
	LANGID_INUKTITUT,
	LANGID_INUPIAQ,
	LANGID_IRISH,
	LANGID_JAVANESE,
	LANGID_KANNADA,
	LANGID_KANURI,
	LANGID_KASHMIRI,
	LANGID_KAZAKH,
	LANGID_KHMER,
	LANGID_KIKUYU,
	LANGID_KINYARWANDA,
	LANGID_KYRGYZ,
	LANGID_KOMI,
	LANGID_KONGO,
	LANGID_KUANYAMA,
	LANGID_KURDISH,
	LANGID_LAO,
	LANGID_LATIN,
	LANGID_LATVIAN,
	LANGID_LIMBURGAN,
	LANGID_LINGALA,
	LANGID_LITHUANIAN,
	LANGID_LUBAKATANGA,
	LANGID_LUXEMBOURGISH,
	LANGID_MACEDONIAN,
	LANGID_MALAGASY,
	LANGID_MALAY,
	LANGID_MALAYALAM,
	LANGID_MALTESE,
	LANGID_MANX,
	LANGID_MAORI,
	LANGID_MARATHI,
	LANGID_MARSHALLESE,
	LANGID_MONGOLIAN,
	LANGID_NAURU,
	LANGID_NAVAJO,
	LANGID_NORTHNDEBELE,
	LANGID_SOUTHNDEBELE,
	LANGID_NDONGA,
	LANGID_NEPALI,
	LANGID_NORWEGIAN,
	LANGID_NORWEGIANBOKMAL,
	LANGID_NORWEGIANNYNORSK,
	LANGID_OCCITAN,
	LANGID_OJIBWA,
	LANGID_ORIYA,
	LANGID_OROMO,
	LANGID_OSSETIAN,
	LANGID_PALI,
	LANGID_PASHTO,
	LANGID_PERSIAN,
	LANGID_PORTUGUESE,
	LANGID_PUNJABI,
	LANGID_QUECHUA,
	LANGID_ROMANIAN,
	LANGID_ROMANSH,
	LANGID_RUNDI,
	LANGID_SAMI,
	LANGID_SAMOAN,
	LANGID_SANGO,
	LANGID_SANSKRIT,
	LANGID_SARDINIAN,
	LANGID_SERBIAN,
	LANGID_SHONA,
	LANGID_SINDHI,
	LANGID_SINHALA,
	LANGID_SLOVAK,
	LANGID_SLOVENIAN,
	LANGID_SOMALI,
	LANGID_SOUTHERNSOTHO,
	LANGID_SUNDANESE,
	LANGID_SWAHILI,
	LANGID_SWATI,
	LANGID_TAGALOG,
	LANGID_TAHITIAN,
	LANGID_TAJIK,
	LANGID_TAMIL,
	LANGID_TATAR,
	LANGID_TELUGU,
	LANGID_THAI,
	LANGID_TIBETAN,
	LANGID_TIGRINYA,
	LANGID_TONGA,
	LANGID_TSONGA,
	LANGID_TSWANA,
	LANGID_TURKISH,
	LANGID_TURKMEN,
	LANGID_TWI,
	LANGID_UIGHUR,
	LANGID_URDU,
	LANGID_UZBEK,
	LANGID_VENDA,
	LANGID_VIETNAMESE,
	LANGID_VOLAPUK, 
	LANGID_WALLOON,
	LANGID_WELSH,
	LANGID_WOLOF,
	LANGID_XHOSA,
	LANGID_SICHUANYI,
	LANGID_YIDDISH,
	LANGID_YORUBA,
	LANGID_ZHUANG,
	LANGID_ZULU
} LangID;

typedef struct
{
	LangID langid;
	const char *name;
	const char *initials ;	// two character identifier
	const char *character;	// single character identifier

} LANGINFO;

LANGINFO*	GetLangInfo ( LangID langid );
const char*	GetLangName ( LangID langid );
LANGINFO*	GetLangInfo ( int index );
LANGINFO*	GetLangInfo ( char *language );

class CWaveInfo
{
	int						wave_valid;
	DWORD					wave_size_hi;
	DWORD					wave_size_lo;
	int						missing;

	public:

	CWaveInfo ( void );
	int						Valid		( void )									{ return wave_valid; };
	DWORD					Lo			( void )									{ return wave_size_lo; };
	DWORD					Hi			( void )									{ return wave_size_hi; };
	void					SetValid( int new_valid )					{ wave_valid = new_valid; };
	void					SetLo		( DWORD new_lo )					{ wave_size_lo = new_lo; };
	void					SetHi		( DWORD new_hi )					{ wave_size_hi = new_hi; };
	int						Missing ( void )									{ return missing; };
	void					SetMissing ( int val )						{ missing = val;  };
};

class DBAttribs
{
	DBAttribs	*parent;
	int changed;
	int processed;
	void *match;


	public:

	DBAttribs( void )													{ parent = nullptr; changed = FALSE; processed = FALSE; match = nullptr; };

	void	SetParent ( DBAttribs *new_parent )	{ parent = new_parent; };
	int		IsChanged ( void )									{ return changed; };
	void	Changed ( void )										{ changed = TRUE; if ( parent ) parent->Changed(); };
	void	NotChanged ( void )									{ changed = FALSE; };
	char	ChangedSymbol ( void )							{ return changed ? '*' :' '; };
	int		IsProcessed ( void )								{ return processed; };
	void	Processed ( void )									{ processed = TRUE; };
	void	NotProcessed ( void )								{ processed = FALSE; };
	void*	Matched ( void )										{ return match; };
	void	Match ( void* new_match )						{ match = new_match; };
	void	NotMatched ( void )									{ match = nullptr; };


};

class TransDB;
class BabylonLabel;
class BabylonText;

class Translation : public DBAttribs
{
	TransDB				*db;

	OLEString			*text;
	OLEString			*comment;
	LangID				langid;
	int						revision;
	int						sent;


	public:

	CWaveInfo			WaveInfo;

	Translation ( void );
	~Translation ( );

	void					SetDB				( TransDB *new_db );
	Translation*	Clone				( void );
	void					SetLangID		( LangID new_id )					{ langid = new_id; };
	TransDB*			DB					( void )									{ return db; };
	void					ClearChanges (void)										{ NotChanged(); };
	void					ClearProcessed (void)									{ NotProcessed(); };
	void					ClearMatched (void)										{ NotMatched(); };
	int						Clear				( void )									{ return 0;};
	void					Set					( OLECHAR *string )				{ text->Set ( string ); Changed();};
	void					Set					( char *string )					{ text->Set ( string ); Changed(); };
	OLECHAR*			Get					( void )									{ return text->Get (); };
	int						Len					( void )									{ return text->Len (); };
	char*					GetSB				( void )									{ return text->GetSB (); };
	void					SetComment	( OLECHAR *string )				{ comment->Set ( string ); Changed(); };
	void					SetComment	( char *string )					{ comment->Set ( string ); Changed(); };
	OLECHAR*			Comment			( void )									{ return comment->Get(); };
	char*					CommentSB		( void )									{ return comment->GetSB(); };
	int						Revision		( void )									{ return revision; };
	void					SetRevision	( int new_rev )						{ revision = new_rev; Changed(); };
	LangID				GetLangID		( void )									{ return langid; };
	const char*		Language		( void )									{ return GetLangName ( langid );};
	void					AddToTree		( CTreeCtrl *tc, HTREEITEM parent, int changes = FALSE );
	int						TooLong			( int maxlen );
	int						ValidateFormat ( BabylonText *text );
	int						IsSent ( void );
	void						Sent ( int val );
};

class BabylonText : public DBAttribs
{

	TransDB				*db;

	OLEString			*text;
	BabylonLabel			*label;
	OLEString			*wavefile;
	unsigned int	line_number;
	List					translations;
	int						revision;
	int						id;
	int						retranslate;
	int						sent;

	void init ( void );

	public:
	CWaveInfo			WaveInfo;

	BabylonText( void );
	~BabylonText( );

	void					AddTranslation ( Translation *trans );
	Translation*	FirstTranslation ( ListSearch &sh );
	Translation*	NextTranslation ( ListSearch &sh );
	Translation*	GetTranslation ( LangID langid );
	void					SetDB				( TransDB *new_db );
	void					ClearChanges ( void );
	void					ClearProcessed ( void );
	void					ClearMatched ( void );
	int						Clear				( void );
	BabylonText*			Clone				( void );
	void					Remove			( void );
	void					AssignID		( void );
	void					Set					( OLECHAR *string );
	void					Set					( char *string );
	void					SetID				( int new_id )						{ id = new_id; Changed(); };
	int						ID					( void )									{ return id; };
	void					LockText		( void )									{ text->Lock(); };
	TransDB*			DB					( void )									{ return db; };
	OLECHAR*			Get					( void )									{ return text->Get (); } ;
	int						Len					( void )									{ return text->Len (); };
	char*					GetSB				( void )									{ return text->GetSB (); } ;
	void					SetWave			( OLECHAR *string )				{ wavefile->Set ( string ); Changed(); InvalidateAllWaves (); };
	void					SetWave			( char *string )					{ wavefile->Set ( string ); Changed(); InvalidateAllWaves (); };
	void					SetLabel		( BabylonLabel *new_label )		{ label = new_label; };
	void					SetRetranslate ( int flag = TRUE )		{ retranslate = flag;};
	int						Retranslate ( void )									{ return retranslate; };
	OLECHAR*			Wave				( void )									{ return wavefile->Get (); } ;
	char*					WaveSB			( void )									{ return wavefile->GetSB (); } ;
	BabylonLabel*			Label				( void )									{ return label; } ;
	int						Revision		( void )									{ return revision; } ;
	void					SetRevision	( int new_rev )						{ revision = new_rev; Changed(); } ;
	void					IncRevision ( void )									{ revision++; Changed(); };
	void					AddToTree		( CTreeCtrl *tc, HTREEITEM parent, int changes = FALSE );
	int						LineNumber	( void )									{ return line_number; };
	void					SetLineNumber	( int line )						{ line_number = line; Changed(); };
	void					FormatMetaString ( void )							{ text->FormatMetaString (); Changed();};
	int						IsDialog ( void );
	int						DialogIsPresent ( const char *path, LangID langid = LANGID_US  );
	int						DialogIsValid ( const char *path, LangID langid = LANGID_US, int check = TRUE );
	int						ValidateDialog( const char *path, LangID langid = LANGID_US );
	void					InvalidateAllWaves ( void );
	void					InvalidateWave ( void );
	void					InvalidateWave ( LangID langid );
	int						IsSent ( void );
	void						Sent ( int val );

};


class BabylonLabel : public DBAttribs
{
	TransDB				*db;


	OLEString			*name;
	OLEString			*comment;
	OLEString			*context;
	OLEString			*speaker;
	OLEString			*listener;
	unsigned int	max_len;
	unsigned int	line_number;
	List					text;

	void init ( void );

	public:

	BabylonLabel ( void );
	~BabylonLabel ( );

	int						Clear				( void );
	void					ClearChanges ( void );
	void					ClearProcessed ( void );
	void					ClearMatched ( void );
	int						AllMatched	( void );
	void					Remove			( void );
	void					AddText			( BabylonText *new_text );
	void					RemoveText	( BabylonText *new_text );
	BabylonText*			FirstText		( ListSearch& sh );
	BabylonText*			NextText		( ListSearch& sh);
	BabylonText*			FindText		( OLECHAR *find_text );
	void					SetDB				( TransDB *new_db );
	BabylonLabel*			Clone				( void );
	int						NumStrings	( void )									{ return text.NumItems(); };
	void					SetMaxLen		( int max )								{ max_len = max; Changed(); };
	int						MaxLen			( void )									{ return max_len; };
	void					SetLineNumber( int line )							{ line_number = line; Changed(); };
	int						LineNumber	( void )									{ return line_number; };
	TransDB*			DB					( void )									{ return db;};
	void					LockName		( void )									{ name->Lock(); };
	void					SetName			( OLECHAR *string )				{ name->Set ( string ); Changed(); };
	void					SetName			( char *string )					{ name->Set ( string ); Changed(); };
	void					SetComment	( OLECHAR *string )				{ comment->Set ( string ); Changed(); };
	void					SetComment	( char *string )					{ comment->Set ( string ); Changed(); };
	void					SetContext	( OLECHAR *string )				{ context->Set ( string ); Changed(); };
	void					SetContext	( char *string )					{ context->Set ( string ); Changed(); };
	void					SetSpeaker	( char *string )					{ speaker->Set ( string ); Changed(); };
	void					SetSpeaker	( OLECHAR *string )				{ speaker->Set ( string ); Changed(); };
	void					SetListener	( char *string )					{ listener->Set ( string ); Changed(); };
	void					SetListener	( OLECHAR *string )				{ listener->Set ( string ); Changed(); };

	OLECHAR*			Name				( void )									{ return name->Get (); };
	OLECHAR*			Comment			( void )									{ return comment->Get(); };
	OLECHAR*			Context			( void )									{ return context->Get(); };
	OLECHAR*			Speaker			( void )									{ return speaker->Get(); };
	OLECHAR*			Listener		( void )									{ return listener->Get(); };


	char*					NameSB	 		( void )									{ return name->GetSB (); };
	char*					CommentSB		( void )									{ return comment->GetSB(); };
	char*					ContextSB		( void )									{ return context->GetSB(); };
	char*					SpeakerSB		( void )									{ return speaker->GetSB(); };
	char*					ListenerSB	( void )									{ return listener->GetSB(); };

	void					AddToTree		( CTreeCtrl *tc, HTREEITEM parent, int changes = FALSE );

};

#define TRANSDB_OPTION_NONE									00000000
#define TRANSDB_OPTION_DUP_TEXT							00000001	// strings can be dupilcated across labels
#define TRANSDB_OPTION_MULTI_TEXT						00000002	// labels can have more than 1 string

const int	START_STRING_ID	= 10000;
class TransDB : public DBAttribs
{
	ListNode			node;
	List					labels;
	List					obsolete;
	Bin						*label_bin;
	Bin						*text_bin;
	BinID					*text_id_bin;
	Bin						*obsolete_bin;
	char					name[100];
	int						num_obsolete;
	int						next_string_id;
	int						valid;
	int						checked_for_errors;
	int						last_error_count;
	int						flags;


	public:

	TransDB ( const char *name = "no name" );
	~TransDB ( );

	void					InvalidateDialog( LangID langid );
	void					VerifyDialog( LangID langid, void (*cb) ( void ) = nullptr  );
	int						ReportDialog( DLGREPORT *report, LangID langid, void (*print) ( const char *)= nullptr, PMASK pmask= PMASK_ALL );
	int						ReportTranslations( TRNREPORT *report, LangID langid, void (*print) ( const char *) = nullptr, PMASK pmask = PMASK_ALL );
	void					ReportDuplicates ( CBabylonDlg *dlg = nullptr );
	void					AddLabel		( BabylonLabel *label );
	void					AddText			( BabylonText *text );
	void					AddObsolete ( BabylonText *text );
	void					RemoveLabel ( BabylonLabel *label );
	void					RemoveText	( BabylonText *text );
	void					RemoveObsolete	( BabylonText *text );
	int						Errors		( CBabylonDlg *dlg = nullptr );
	int						HasErrors ( void ) { return checked_for_errors ? last_error_count != 0 : FALSE; };
	int						Warnings		( CBabylonDlg *dlg = nullptr );
	int						NumLabelsChanged	( void );
	int						NumLabels		( void );
	int						NumObsolete		( void ) { return num_obsolete; };
	BabylonLabel*			FirstLabel	( ListSearch& sh );
	BabylonLabel*			NextLabel		( ListSearch& sh);
	BabylonText*			FirstObsolete	( ListSearch& sh );
	BabylonText*			NextObsolete	( ListSearch& sh);
	BabylonLabel*			FindLabel		( OLECHAR *name );
	BabylonText*			FindText		( OLECHAR *text );
	BabylonText*			FindSubText	( OLECHAR *text, int item = 0 );
	BabylonText*			FindText		( int id );
	BabylonText*			FindNextText ( void );
	BabylonText*			FindObsolete		( OLECHAR *text );
	BabylonText*			FindNextObsolete ( void );
	int						Clear				( void );
	void					ClearChanges ( void );
	void					ClearProcessed ( void );
	void					ClearMatched ( void );
	TransDB*			Next				( void );
	void					AddToTree		( CTreeCtrl *tc, HTREEITEM parent, int changes = FALSE, void (*cb) ( void ) = nullptr );
	char*					Name				( void )							{ return name;};
	void					EnableIDs		( void )							{ next_string_id = START_STRING_ID; };
	int						NewID				( void )							{ if ( next_string_id != -1)  return next_string_id++; else return -1; };
	int						ID					( void )							{ return next_string_id; };
	void					SetID				( int new_id )				{ next_string_id = new_id; };
	int						IsValid			( void )							{ return valid; };
	void					InValid			( void )							{ valid = FALSE; };
	int						DuplicatesAllowed ( void )				{ return flags & TRANSDB_OPTION_DUP_TEXT;};
	int						MultiTextAllowed ( void )					{ return flags & TRANSDB_OPTION_MULTI_TEXT;};
	void					AllowDupiclates ( int yes = TRUE) { yes ? flags |= TRANSDB_OPTION_DUP_TEXT : flags &= ~(TRANSDB_OPTION_DUP_TEXT ); };
	void					AllowMultiText  ( int yes = TRUE) { yes ? flags |= TRANSDB_OPTION_MULTI_TEXT : flags &= ~(TRANSDB_OPTION_MULTI_TEXT ); };
};


class DupNode : public ListNode
{
	BabylonText *original;
	BabylonText *duplicate;

	public:
	DupNode ( BabylonText *dup, BabylonText *orig ) { original = orig; duplicate = dup, SetPriority ( orig->LineNumber ());};

	BabylonText *Duplicate ( void ) { return duplicate; };
	BabylonText *Original ( void ) { return original; };

};



extern TransDB* FirstTransDB ( void );

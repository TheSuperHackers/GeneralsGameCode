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
// TransDB.cpp
//

#include "StdAfx.h"
#include "TransDB.h"
#include "BabylonDlg.h"
#include "Babylon.h"
#include "assert.h"
#include "bin.h"
#include "list.h"

static char buffer[100*1024];

static List	DataBases;

static LANGINFO langinfo[] =
	{
		{	LANGID_US, "US", "us", "e"},
		{ LANGID_UK, "UK", "uk", "e" },
		{ LANGID_GERMAN, "German", "ge", "g" },
		{ LANGID_FRENCH, "French", "fr", "f" },
		{ LANGID_SPANISH, "Spanish", "sp", "s" },
		{ LANGID_ITALIAN, "Italian", "it", "i" },
		{ LANGID_JAPANESE, "Japanese", "ja", "j" },
		{ LANGID_KOREAN, "Korean", "ko", "k" },
		{ LANGID_CHINESE, "Chinese", "ch", "c" },
		{ LANGID_JABBER, "Jabberwockie", "jb", "e" },
		{ LANGID_UNKNOWN, "Unknown", NULL, NULL }
	};

LANGINFO *GetLangInfo ( int index )
{

	if ( (index >= 0) && (index < (sizeof ( langinfo ) / sizeof (LANGINFO )) -1) )
	{
		return &langinfo[index];
	}

	return NULL;
}

LANGINFO *GetLangInfo ( LangID langid )
{
	LANGINFO *item;

	item = langinfo;

	while ( item->langid != LANGID_UNKNOWN )
	{
		if ( item->langid == langid )
		{
			return item;
		}
		item++;
	}

	return NULL;
}

const char *GetLangName ( LangID langid )
{
	LANGINFO *item;

	if ( ( item = GetLangInfo ( langid )) )
	{
		return item->name;
	}

	return "unknown";
}

LANGINFO *GetLangInfo ( char *language )
{
	LANGINFO *item;

	item = langinfo;

	while ( item->langid != LANGID_UNKNOWN )
	{
		if ( stricmp ( language, item->name ) == 0 )
		{
			return item;
		}
		item++;
	}

	return NULL;
}

TransDB* FirstTransDB ( void )
{
	ListNode *first;

	first = DataBases.Next ();
	if ( first )
	{
		return (TransDB *) first->Item ();
	}
	return NULL;
}

TransDB::TransDB ( const char *cname )
{
	text_bin = new Bin ();
	text_id_bin = new BinID ();
	label_bin = new Bin ();
	obsolete_bin = new Bin ();
	strncpy ( name, cname, sizeof ( name ) -1 );
	name[sizeof(name)-1] = 0;
	node.SetItem ( this );
	DataBases.AddToTail ( &node );
	next_string_id = -1;
	valid = TRUE;
	num_obsolete = 0;
	checked_for_errors = FALSE;
	flags = TRANSDB_OPTION_NONE | TRANSDB_OPTION_DUP_TEXT;
}

TransDB::	~TransDB ( )
{
	Clear ();
	node.Remove ();
	delete text_bin;
	delete text_id_bin;
	delete label_bin;
	delete obsolete_bin;

}

void					TransDB::AddLabel		( BabylonLabel *label )
{
	ListNode	*node = new ListNode ();

	node->SetItem ( label );

	label_bin->Add ( label, label->Name() );

	labels.AddToTail ( node );
	label->SetDB ( this );
	Changed ();

}

void					TransDB::AddText		( BabylonText *text )
{

	text_bin->Add ( text, text->Get() );
	if ( text->ID () > 0 )
	{
		text_id_bin->Add ( text, text->ID ());
	}

}

void					TransDB::AddObsolete		( BabylonText *text )
{
	ListNode	*node = new ListNode ();

	node->SetItem ( text );

	obsolete_bin->Add ( text, text->Get() );
	if ( text->ID () > 0 )
	{
		text_id_bin->Add ( text, text->ID ());
	}

	num_obsolete++;
	text->SetParent ( (DBAttribs *)this );
	text->Changed ();

	obsolete.AddToTail ( node );
	Changed ();

}

void					TransDB::RemoveLabel ( BabylonLabel *label )
{
	ListNode *node;

	if ( (node = labels.Find ( label )) )
	{
		node->Remove ();
		label->SetDB ( NULL );
		label_bin->Remove ( label );
		delete node;
		Changed ();
	}
}

void					TransDB::RemoveText ( BabylonText *text )
{
	text_bin->Remove ( text );
	text_id_bin->Remove ( text );
}

void					TransDB::RemoveObsolete ( BabylonText *text )
{
	ListNode *node;

	if ( (node = obsolete.Find ( text )) )
	{
		node->Remove ();
		obsolete_bin->Remove ( text );
		text_id_bin->Remove ( text );
		num_obsolete--;
		delete node;
		Changed ();
	}
}

int					TransDB::NumLabelsChanged ( void )
{
	BabylonLabel	*label;
	ListSearch sh;
	int changed = 0;

	label = FirstLabel ( sh );

	while ( label )
	{
		if ( label->IsChanged ())
		{
			changed++;
		}

		label = NextLabel ( sh );
	}

	return changed;
}

int					TransDB::NumLabels ( void )
{

	return labels.NumItems();
}

BabylonLabel*			TransDB::FirstLabel	( ListSearch& sh )
{
	ListNode *node;

	if ( ( node = sh.FirstNode ( &labels )))
	{
		return (BabylonLabel *) node->Item ();
	}

	return NULL;
}

BabylonLabel*			TransDB::NextLabel		( ListSearch& sh)
{
	ListNode *node;

	if ( ( node = sh.Next ()))
	{
		return (BabylonLabel *) node->Item ();
	}

	return NULL;
}

BabylonText*			TransDB::FirstObsolete	( ListSearch& sh )
{
	ListNode *node;

	if ( ( node = sh.FirstNode ( &obsolete )))
	{
		return (BabylonText *) node->Item ();
	}

	return NULL;
}

BabylonText*			TransDB::NextObsolete		( ListSearch& sh)
{
	ListNode *node;

	if ( ( node = sh.Next ()))
	{
		return (BabylonText *) node->Item ();
	}

	return NULL;
}

BabylonLabel*			TransDB::FindLabel		( OLECHAR *name )
{
	return (BabylonLabel *) label_bin->Get ( name );
}

BabylonText*			TransDB::FindText		( OLECHAR *text )
{

	return (BabylonText *) text_bin->Get ( text );
}

BabylonText*			TransDB::FindSubText		( OLECHAR *pattern, int item )
{
	BabylonLabel	*label;
	ListSearch sh;
	BabylonText		*text;
	ListSearch sh_text;
	int plen = wcslen ( pattern );

	label = FirstLabel ( sh );

	while ( label )
	{
		text = label->FirstText ( sh_text );

		while ( text )
		{

			if ( wcsnicmp ( text->Get (), pattern, 15 ) == 0)
			{
				if ( !item )
				{
					return text;
				}

				item--;
			}

			text = label->NextText ( sh_text );
		}

		label = NextLabel ( sh );
	}

	return NULL;

}

BabylonText*			TransDB::FindText		( int id )
{

	return (BabylonText *) text_id_bin->Get ( id );
}

BabylonText*			TransDB::FindNextText		( void )
{

	return (BabylonText *) text_bin->GetNext ( );
}

BabylonText*			TransDB::FindObsolete		( OLECHAR *name )
{
	return (BabylonText *) obsolete_bin->Get ( name );
}

BabylonText*			TransDB::FindNextObsolete		( void )
{

	return (BabylonText *) obsolete_bin->GetNext ( );

}

int					TransDB::Clear				( void )
{
	ListSearch sh;
	BabylonLabel *label;
	BabylonText *text;
	ListNode *node;
	int count = 0;

	text_bin->Clear ();
	text_id_bin->Clear ();
	label_bin->Clear ();
	obsolete_bin->Clear ();

	while ( node = sh.FirstNode ( &labels ) )
	{
		node->Remove ();
		label = (BabylonLabel *) node->Item ();
		count++;
		delete label;
		delete node;
	}

	while ( node = sh.FirstNode ( &obsolete ) )
	{
		node->Remove ();
		text = (BabylonText *) node->Item ();
		count++;
		delete text;
		delete node;
	}

	num_obsolete = 0;

	if ( next_string_id != -1 )
	{
			next_string_id = START_STRING_ID;
	}

	if ( count )
	{
		Changed ();
	}

	valid = TRUE;

	return count;
}

void					TransDB::ClearChanges				( void )
{
	ListSearch sh;
	BabylonLabel *label;

	label = FirstLabel ( sh );
	while ( label )
	{
		label->ClearChanges ();
		label = NextLabel ( sh );
	}

	BabylonText *text = FirstObsolete ( sh );
	while ( text )
	{
		text->ClearChanges ();
		text = NextObsolete ( sh );
	}

	NotChanged ();
}

void					TransDB::ClearProcessed				( void )
{
	ListSearch sh;
	BabylonLabel *label;

	label = FirstLabel ( sh );
	while ( label )
	{
		label->ClearProcessed ();
		label = NextLabel ( sh );
	}
	NotProcessed ();
}

void					TransDB::ClearMatched				( void )
{
	ListSearch sh;
	BabylonLabel *label;

	label = FirstLabel ( sh );
	while ( label )
	{
		label->ClearMatched ();
		label = NextLabel ( sh );
	}
	NotMatched ();
}

void					TransDB::AddToTree		( CTreeCtrl *tc, HTREEITEM parent, int changes, void (*cb) ( void ) )
{
	HTREEITEM		item;
	HTREEITEM		ilabels, iobsolete;
	ListSearch	sh;
	BabylonLabel		*label;
	BabylonText			*txt;

	sprintf ( buffer, "%s%c  (%d/%d)",name, ChangedSymbol(), NumLabelsChanged(), NumLabels() );
	item = tc->InsertItem ( buffer, parent );
	ilabels = tc->InsertItem ( "Labels", item );

	label = FirstLabel ( sh );

	while ( label )
	{
		if ( !changes || label->IsChanged ())
		{
			label->AddToTree ( tc, ilabels, changes );
		}

		if ( cb )
		{
			cb ( );
		}

		label = NextLabel ( sh );
	}

	if ( num_obsolete )
	{
		iobsolete = tc->InsertItem ( "Obsolete Strings", item );

		txt = FirstObsolete ( sh );

		while ( txt )
		{
			if ( !changes || txt->IsChanged ())
			{
				txt->AddToTree ( tc, iobsolete );
			}

			if ( cb )
			{
				cb ( );
			}

			txt = NextObsolete ( sh );
		}
	}


}

TransDB*			TransDB::Next				( void )
{
	ListNode *next;

	next = node.Next ();

	if ( next )
	{
		return (TransDB *) next->Item ();
	}

	return NULL;

}

void BabylonLabel::init ( void )
{
	db = NULL;
	comment = NULL;
	line_number = -1;
	max_len = 0;
	name = NULL;
}

BabylonLabel::BabylonLabel ( void )
{
	init ();
	name = new OLEString ( );
	comment = new OLEString ( );
	context = new OLEString ( );
	speaker = new OLEString ( );
	listener = new OLEString ( );


}

BabylonLabel::~BabylonLabel ( )
{
	Clear ();
	delete name;
	delete comment;
	delete context;
	delete speaker;
	delete listener;
}

void					BabylonLabel::Remove			( void )
{
	if ( db )
	{
		db->RemoveLabel ( this );
	}
}

void					BabylonLabel::RemoveText ( BabylonText *txt )
{
	ListNode *node;

	if ( (node = text.Find ( txt )) )
	{
		node->Remove ();
		txt->SetDB ( NULL );
		txt->SetLabel ( NULL );
		txt->SetParent ( NULL );
		delete node;
		Changed ();
	}
}

void					BabylonLabel::AddText			( BabylonText *new_text )
{
	TransDB *db = DB();
	ListNode	*node = new ListNode ();

	node->SetItem ( new_text );

	text.AddToTail ( node );
	Changed ();
	new_text->SetDB ( db );
	new_text->SetParent ( (DBAttribs *) this );
	new_text->SetLabel ( this );
}

int					BabylonLabel::Clear				( void )
{
	ListSearch sh;
	BabylonText *txt;
	ListNode *node;
	int count = 0;

	while ( node = sh.FirstNode ( &text ) )
	{
		node->Remove ();
		txt = (BabylonText *) node->Item ();
		delete txt;
		delete node;
		count++;
	}

	if ( count )
	{
		Changed ();
	}

	return count;
}

BabylonLabel*			BabylonLabel::Clone				( void )
{
	BabylonLabel *clone = new BabylonLabel();
	BabylonText *txt;
	ListSearch sh;

	clone->SetName ( Name());
	clone->SetComment ( Comment ());
	clone->SetListener ( Listener ());
	clone->SetSpeaker ( Speaker ());
	clone->SetMaxLen ( MaxLen ());
	clone->SetContext ( Context ());

	txt = FirstText ( sh );

	while ( txt )
	{
		clone->AddText ( txt->Clone ());

		txt = NextText ( sh );
	}

	return clone;
}

BabylonText*			BabylonLabel::FirstText		( ListSearch& sh )
{
	ListNode *node;

	if ( ( node = sh.FirstNode ( &text )))
	{
		return (BabylonText *) node->Item ();
	}

	return NULL;
}

BabylonText*			BabylonLabel::NextText		( ListSearch& sh)
{
	ListNode *node;

	if ( ( node = sh.Next (  )))
	{
		return (BabylonText *) node->Item ();
	}

	return NULL;

}

BabylonText*			BabylonLabel::FindText ( OLECHAR *find_text )
{
	ListSearch sh;
	BabylonText *txt;

	txt = FirstText ( sh );

	while ( txt )
	{
		if ( wcscmp ( txt->Get(), find_text ) == 0)
		{
			return txt;
		}
		txt = NextText ( sh );
	}

	return NULL;
}


void					BabylonLabel::SetDB				( TransDB *new_db )
{
	BabylonText *ntext;
	ListSearch sh;

	db = new_db;
	SetParent ( (DBAttribs *) new_db );

	ntext = FirstText ( sh );

	while ( ntext )
	{
		ntext->SetDB ( new_db );

		ntext = NextText ( sh );
	}

}

void					BabylonLabel::ClearChanges				( void )
{
	BabylonText *ntext;
	ListSearch sh;

	ntext = FirstText ( sh );

	while ( ntext )
	{
		ntext->ClearChanges();

		ntext = NextText ( sh );
	}

	NotChanged();

}

void					BabylonLabel::ClearProcessed				( void )
{
	BabylonText *ntext;
	ListSearch sh;

	ntext = FirstText ( sh );

	while ( ntext )
	{
		ntext->ClearProcessed();

		ntext = NextText ( sh );
	}

	NotProcessed();

}

void					BabylonLabel::ClearMatched				( void )
{
	BabylonText *ntext;
	ListSearch sh;

	ntext = FirstText ( sh );

	while ( ntext )
	{
		ntext->ClearMatched();

		ntext = NextText ( sh );
	}

	NotMatched();

}

int					BabylonLabel::AllMatched				( void )
{
	BabylonText *ntext;
	ListSearch sh;

	ntext = FirstText ( sh );

	while ( ntext )
	{
		if ( !ntext->Matched() )
		{
			return FALSE;
		}

		ntext = NextText ( sh );
	}

	return TRUE;
}

BabylonText::BabylonText( void )
{
	init ();
	text = new OLEString (  );
	wavefile = new OLEString (  );

}

int BabylonText::IsSent ( void )
{
	return sent;
}

void BabylonText::Sent ( int val )
{
	sent = val;
}

void					BabylonLabel::AddToTree		( CTreeCtrl *tc, HTREEITEM parent, int changes )
{
	HTREEITEM		litem;
	ListSearch	sh;
	BabylonText			*txt;

	sprintf ( buffer, "%s%c", NameSB(), ChangedSymbol() );

	litem = tc->InsertItem ( buffer, parent );

	txt = FirstText ( sh );

	while ( txt )
	{
		if ( !changes || txt->IsChanged ())
		{
			txt->AddToTree ( tc, litem );
		}

		txt = NextText ( sh );
	}

	if ( strcmp ( CommentSB(), "" ) != 0 )
	{
		sprintf ( buffer, "COMMENT : %s", CommentSB() );
		tc->InsertItem ( buffer, litem );
	}

	if ( strcmp ( ContextSB(), "" ) != 0 )
	{
		sprintf ( buffer, "CONTEXT : %s", ContextSB() );
		tc->InsertItem ( buffer, litem );
	}

	if ( strcmp ( SpeakerSB(), "" ) != 0 )
	{
		sprintf ( buffer, "SPEAKER : %s", SpeakerSB() );
		tc->InsertItem ( buffer, litem );
	}

	if ( strcmp ( ListenerSB(), "" ) != 0 )
	{
		sprintf ( buffer, "LISTENER: %s", ListenerSB() );
		tc->InsertItem ( buffer, litem );
	}

	if ( line_number != -1 )
	{
		sprintf ( buffer, "LINE    : %d", line_number );
		tc->InsertItem ( buffer, litem );
	}

	if ( max_len )
	{
		sprintf ( buffer, "MAX LEN : %d", max_len );
		tc->InsertItem ( buffer, litem );
	}

}

void BabylonText::init ( void )
{
	db = NULL;
	label = NULL;
	line_number = -1;
	revision = 1;
	text = NULL;
	wavefile = NULL;
	id = -1;
	retranslate = FALSE;
	sent = FALSE;


}

BabylonText::~BabylonText( )
{
	Clear();
	delete text;
	delete wavefile;

}

void					BabylonText::SetDB				( TransDB *new_db )
{
	Translation *trans;
	ListSearch sh;

	if ( db )
	{
		db->RemoveText ( this );
	}


	if ( (db = new_db) )
	{
		AssignID ();
		db->AddText ( this );
	}

	trans = FirstTranslation ( sh );

	while ( trans )
	{
		trans->SetDB ( new_db );

		trans = NextTranslation ( sh );
	}

}

void					BabylonText::Remove			( void )
{
	if ( label )
	{
		label->RemoveText ( this );
	}
}

int						BabylonText::IsDialog ( void )
{

	return strcmp (WaveSB(), "" );

}

int						BabylonText::DialogIsValid ( const char *path, LangID langid, int check )
{
	LANGINFO	*linfo;
	CWaveInfo *winfo;
	DBAttribs *attribs;

	linfo = GetLangInfo ( langid );

	if ( langid == LANGID_US )
	{
		winfo = &WaveInfo;
		attribs = (DBAttribs *) this;
	}
	else
	{
		Translation *trans = GetTranslation ( langid );

		if ( !trans )
		{
			return FALSE;
		}

		attribs = (DBAttribs *) trans;
		winfo = &trans->WaveInfo;
	}


	if ( winfo->Valid () && check )
	{
		WIN32_FIND_DATA info;
		HANDLE	handle;

		winfo->SetValid ( FALSE );
		winfo->SetMissing ( TRUE );

		sprintf ( buffer, "%s%s\\%s%s.wav", path, linfo->character, WaveSB(), linfo->character );
		if ( (handle = FindFirstFile ( buffer, &info )) != INVALID_HANDLE_VALUE )
		{
			if ( ! (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				if ( winfo->Lo () == info.nFileSizeLow && winfo->Hi() == info.nFileSizeHigh )
				{
					winfo->SetValid ( TRUE );
				}
				winfo->SetMissing ( FALSE );
			}

			FindClose ( handle );
		}

	}

	return winfo->Valid ();
}

int						BabylonText::ValidateDialog ( const char *path, LangID langid )
{
	WIN32_FIND_DATA info;
	HANDLE	handle;
	CWaveInfo *winfo;
	LANGINFO *linfo;
	DBAttribs *attribs;

	linfo = GetLangInfo ( langid );

	if ( langid == LANGID_US )
	{
		winfo = &WaveInfo;
		attribs = (DBAttribs *) this;
	}
	else
	{
		Translation *trans = GetTranslation ( langid );

		if ( !trans )
		{
			return FALSE;
		}

		attribs = (DBAttribs *) trans;
		winfo = &trans->WaveInfo;
	}

	winfo->SetValid  ( FALSE );
	winfo->SetMissing ( TRUE );

	sprintf ( buffer, "%s%s\\%s%s.wav", path, linfo->character , WaveSB(), linfo->character );
	if ( (handle = FindFirstFile ( buffer, &info )) != INVALID_HANDLE_VALUE )
	{
		if ( ! (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			winfo->SetLo ( info.nFileSizeLow );
			winfo->SetHi ( info.nFileSizeHigh );
			winfo->SetValid  ( TRUE );
			winfo->SetMissing ( FALSE );
			attribs->Changed();
		}
		FindClose ( handle );
	}

	return winfo->Valid ();
}

int						BabylonText::DialogIsPresent ( const char *path, LangID langid )
{

	WIN32_FIND_DATA info;
	HANDLE	handle;
	int present = FALSE;
	LANGINFO	*linfo = GetLangInfo ( langid );

	sprintf ( buffer, "%s%s\\%s%s.wav", path, linfo->character , WaveSB(), linfo->character );
	if ( (handle = FindFirstFile ( buffer, &info )) != INVALID_HANDLE_VALUE )
	{
		if ( ! (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
				present = TRUE;
		}

		FindClose ( handle );
	}

	return present;
}

void					BabylonText::AddTranslation			( Translation *trans )
{
	ListNode	*node = new ListNode ();

	node->SetItem ( trans );

	translations.AddToTail ( node );
	Changed ();
	trans->SetDB ( DB() );
	trans->SetParent ( (DBAttribs *) this );

}

Translation*			BabylonText::FirstTranslation		( ListSearch& sh )
{
	ListNode *node;

	if ( ( node = sh.FirstNode ( &translations )))
	{
		return (Translation *) node->Item ();
	}

	return NULL;
}

Translation*			BabylonText::NextTranslation		( ListSearch& sh)
{
	ListNode *node;

	if ( ( node = sh.Next (  )))
	{
		return (Translation *) node->Item ();
	}

	return NULL;
}

Translation*			BabylonText::GetTranslation		( LangID langid )
{
	ListSearch sh;
	Translation *trans;

	trans = FirstTranslation ( sh );

	while ( trans )
	{
		if ( langid == trans->GetLangID())
		{
			break;
		}

		trans = NextTranslation ( sh );
	}


	return trans;
}

int					BabylonText::Clear				( void )
{
	ListSearch sh;
	Translation *trans;
	ListNode *node;
	int count = 0;

	while ( node = sh.FirstNode ( &translations ) )
	{
		node->Remove ();
		trans = (Translation *) node->Item ();
		delete trans;
		delete node;
		count++;
	}
	if ( count )
	{
		Changed ();
	}

	return count;
}

BabylonText*			BabylonText::Clone				( void )
{
	BabylonText *clone = new BabylonText();
	Translation *trans;
	ListSearch sh;

	clone->Set ( Get ());
	clone->SetWave ( Wave ());
	clone->SetRevision ( Revision ());

	trans = FirstTranslation ( sh );

	while ( trans )
	{
		clone->AddTranslation ( trans->Clone ());

		trans = NextTranslation ( sh );
	}

	return clone;
}

void					BabylonText::ClearChanges				( void )
{
	Translation *trans;
	ListSearch sh;

	trans = FirstTranslation ( sh );

	while ( trans )
	{
		trans->ClearChanges();

		trans = NextTranslation ( sh );
	}

	NotChanged();

}

void					BabylonText::ClearProcessed				( void )
{
	Translation *trans;
	ListSearch sh;

	trans = FirstTranslation ( sh );

	while ( trans )
	{
		trans->ClearProcessed();

		trans = NextTranslation ( sh );
	}

	NotProcessed();

}

void					BabylonText::ClearMatched				( void )
{
	Translation *trans;
	ListSearch sh;

	trans = FirstTranslation ( sh );

	while ( trans )
	{
		trans->ClearMatched();

		trans = NextTranslation ( sh );
	}

	NotMatched();

}

void					BabylonText::AssignID ( void )
{
	if ( id != -1 )
	{
		return;	// already assigned
	}
	if ( db )
	{
		SetID ( db->NewID ());
	}
}

void					BabylonText::Set ( OLECHAR *string )
{
	if ( db )
	{
		db->RemoveText ( this );
	}

	text->Set ( string );
	InvalidateWave ( );
	if ( db )
	{
		db->AddText ( this );
	}

	Changed ();
}

void					BabylonText::Set ( char *string )
{
	if ( db )
	{
		db->RemoveText ( this );
	}

	text->Set ( string );
	InvalidateWave ();
	if ( db )
	{
		db->AddText ( this );
	}

	Changed ();
}

void					BabylonText::InvalidateAllWaves			( void  )
{
	Translation *trans;
	ListSearch sh;

	WaveInfo.SetValid ( FALSE );

	trans = FirstTranslation ( sh );

	while ( trans )
	{
		trans->WaveInfo.SetValid ( FALSE );

		trans = NextTranslation ( sh );
	}

}

void					BabylonText::InvalidateWave			( void )
{

	WaveInfo.SetValid ( FALSE );

}

void					BabylonText::InvalidateWave			( LangID langid  )
{

	WaveInfo.SetValid ( FALSE );

	if ( langid == LANGID_US )
	{
		InvalidateWave ();
	}
	else
	{
		Translation *trans = GetTranslation ( langid );

		if ( trans )
		{
			trans->WaveInfo.SetValid ( FALSE );
		}

	}
}

void					BabylonText::AddToTree		( CTreeCtrl *tc, HTREEITEM parent, int changes )
{
	HTREEITEM		item;
	ListSearch	sh;
	Translation	*trans;

	sprintf ( buffer, "TEXT %c : %s", ChangedSymbol() ,GetSB ());
	item = tc->InsertItem ( buffer, parent );

	trans = FirstTranslation ( sh );

	while ( trans )
	{
		if ( !changes || trans->IsChanged ())
		{
			trans->AddToTree ( tc, item );
		}

		trans = NextTranslation ( sh );
	}

	if ( ID() != -1 )
	{
		sprintf ( buffer, "ID     : %d", ID ());
		tc->InsertItem ( buffer, item );
	}

	if ( strcmp ( WaveSB(), "" ) != 0 )
	{
		sprintf ( buffer, "WAVE   : %s", WaveSB() );
		tc->InsertItem ( buffer, item );
	}

	if ( line_number != -1 )
	{
		sprintf ( buffer, "LINE   : %d", line_number );
		tc->InsertItem ( buffer, item );
	}

	sprintf ( buffer, "REV    : %d", revision );
	tc->InsertItem ( buffer, item );

	sprintf ( buffer, "LEN    : %d", this->Len() );
	tc->InsertItem ( buffer, item );

}

Translation::Translation ( void )
{
	text = new OLEString (  );
	comment = new OLEString (  );
	revision = 0;
	sent = FALSE;

}

Translation::~Translation ( )
{
	delete text;
	delete comment;
}

int Translation::IsSent ( void )
{
	return sent;
}

void Translation::Sent ( int val )
{
	sent = val;
}

void					Translation::SetDB				( TransDB *new_db )
{
	db = new_db;
}

Translation*			Translation::Clone				( void )
{
	Translation *clone = new Translation();

	clone->Set ( Get ());
	clone->SetComment ( Comment ());
	clone->SetLangID ( GetLangID ());
	clone->SetRevision ( Revision ());

	return clone;
}

void					Translation::AddToTree		( CTreeCtrl *tc, HTREEITEM parent, int changes )
{
	HTREEITEM		item;

	sprintf ( buffer, "%s%c   : %s", Language(), ChangedSymbol(), GetSB ());

	item = tc->InsertItem ( buffer, parent );

	if ( strcmp ( CommentSB(), "" ) != 0 )
	{
		sprintf ( buffer, "COMMENT: %s", CommentSB() );
		tc->InsertItem ( buffer, item );
	}

	sprintf ( buffer, "REV    : %d", revision );
	tc->InsertItem ( buffer, item );

	sprintf ( buffer, "LEN    : %d", Len() );
	tc->InsertItem ( buffer, item );

}

int Translation::TooLong ( int maxlen )
{
	return maxlen != 0 && text->Len () > maxlen;

}

int Translation::ValidateFormat ( BabylonText *ntext )
{
	return SameFormat ( text->Get(), ntext->Get ());

}

int TransDB::Warnings ( CBabylonDlg *dlg )
{
	BabylonLabel *label;
	ListSearch sh_label;
	int count = 0;
	int warnings = 0;
	List	dups;

	if ( dlg )
	{
		dlg->InitProgress ( NumLabels ());
		dlg->Log ("");
		dlg->Log ("Generals.str Warnigs:");
		dlg->Status ( "Creating warnings report...", FALSE );
	}

	text_bin->Clear();

	label = FirstLabel ( sh_label );

	while ( label )
	{
		BabylonText *text;
		BabylonText *existing_text;
		ListSearch sh_text;

		text = label->FirstText ( sh_text );

		while ( text )
		{
			if ( text->Len ( ) == 0 )
			{
				if ( dlg )
				{
					sprintf ( buffer, "Warning:: text at line %5d is NULL",
								text->LineNumber());
					dlg->Log ( buffer );
				}
				warnings++;
			}
			else if ( !DuplicatesAllowed() && ( existing_text = FindText ( text->Get () )))
			{
				warnings++;
				if ( dlg )
				{
					DupNode *dup = new DupNode ( text, existing_text );
					dups.Add ( dup );
				}
			}
			else
			{
				text_bin->Add ( text, text->Get() );
			}

			text = label->NextText ( sh_text );
		}

		count++;
		if ( dlg )
		{
			dlg->SetProgress ( count );
		}
		label = NextLabel ( sh_label );
	}

	if ( dlg )
	{
		DupNode *dup;

		while ( (dup = (DupNode*)dups.LastNode ()))
		{
			sprintf ( buffer, "Warning:: text at line %5d is a duplicate of text on line %5d",
									dup->Duplicate()->LineNumber(), dup->Original()->LineNumber());
			dlg->Log ( buffer );

			dup->Remove ();
			delete dup;
		}
		sprintf ( buffer, "Total warnings: %d", warnings );
		dlg->Log ( buffer );
		dlg->Ready();
	}

	return warnings;
}

int TransDB::Errors ( CBabylonDlg *dlg )
{
	BabylonLabel *label;
	BabylonLabel *existing_label;
	ListSearch sh_label;
	Bin	*tbin = new Bin ();
	int count = 0;
	int errors = 0;

	if ( dlg )
	{
		dlg->InitProgress ( NumLabels ());
		dlg->Log ("");
		dlg->Log ("Generals.str Errors:");
		dlg->Status ( "Creating error report...", FALSE );
	}



	label_bin->Clear();

	label = FirstLabel ( sh_label );

	while ( label )
	{
		BabylonText *text;
		BabylonText *existing_text;
		ListSearch sh_text;

		if ( !MultiTextAllowed () && label->NumStrings () > 1 )
		{
			errors++;
			if ( dlg )
			{
				sprintf ( buffer, "Error  : Label \"%s\" at line %d is has more than 1 string defined",
							label->NameSB(), label->LineNumber());
				dlg->Log ( buffer );
			}

		}

		if ( ( existing_label = FindLabel ( label->Name () )))
		{
			errors++;
			if ( dlg )
			{
				sprintf ( buffer, "Error  : Label \"%s\" at line %d is already defined on line %d",
							label->NameSB(), label->LineNumber(), existing_label->LineNumber());
				dlg->Log ( buffer );
			}
		}

		label_bin->Add ( label, label->Name());

		tbin->Clear ();

		text = label->FirstText ( sh_text );

		while ( text )
		{
				if ( ( existing_text = (BabylonText *) tbin->Get ( text->Get () )))
				{
					errors++;
					if ( dlg )
					{
						sprintf ( buffer, "Error  : Label \"%s\" has duplicate text at line %d",
									label->NameSB(), text->LineNumber());
						dlg->Log ( buffer );
					}
				}

			tbin->Add ( text, text->Get() );

			// check string length against max len

			if ( label->MaxLen () )
			{
				if ( text->Len () > label->MaxLen ())
				{
					errors++;
					if ( dlg )
					{
						sprintf ( buffer, "Error  : The US text at line %d (for label \"%s\") exceeds the max length",
									text->LineNumber(), label->NameSB());
						dlg->Log ( buffer );
					}
				}
			}

			text = label->NextText ( sh_text );
		}

		count++;
		if ( dlg )
		{
			dlg->SetProgress ( count );
		}
		label = NextLabel ( sh_label );
	}

	if ( dlg )
	{
		sprintf ( buffer, "Total errors: %d", errors );
		dlg->Log ( buffer );

		dlg->Ready();
	}

	delete tbin;
	last_error_count = errors;
	checked_for_errors = TRUE;
	return errors;
}

CWaveInfo::CWaveInfo ( void )
{
	wave_valid = FALSE;
	missing = TRUE;
}

void TransDB::VerifyDialog( LangID langid, void (*cb) (void) )
{
	BabylonLabel *label;
	ListSearch sh_label;
	int count = 0;
	LANGINFO *linfo = GetLangInfo ( langid );

	label = FirstLabel ( sh_label );

	while ( label )
	{
		BabylonText *text;
		ListSearch sh_text;

		text = label->FirstText ( sh_text );

		while ( text )
		{
			if ( text->IsDialog ())
			{
				if ( text->DialogIsPresent ( DialogPath, langid ))
				{
					text->DialogIsValid ( DialogPath, langid );
				}
			}

			text = label->NextText ( sh_text );
		}

		if ( cb )
		{
			cb();
		}
		label = NextLabel ( sh_label );
	}

}

void TransDB::InvalidateDialog( LangID langid )
{
	BabylonLabel *label;
	ListSearch sh_label;

	label = FirstLabel ( sh_label );

	while ( label )
	{
		BabylonText *text;
		ListSearch sh_text;

		text = label->FirstText ( sh_text );

		while ( text )
		{
			if ( text->IsDialog ())
			{
				text->InvalidateWave ( langid );
			}

			text = label->NextText ( sh_text );
		}

		label = NextLabel ( sh_label );
	}

}

int TransDB::ReportDialog( DLGREPORT *report, LangID langid, void (*print) ( const char *), PMASK pmask )
{
	BabylonLabel *label;
	ListSearch sh_label;
	int count = 0;
	DLGREPORT _info;
	DLGREPORT *info = &_info;
	int skip_verify = FALSE;
	LANGINFO *linfo = GetLangInfo ( langid );

	if ( report )
	{
		info = report;
	}

	memset ( info, 0, sizeof ( DLGREPORT ));


	label = FirstLabel ( sh_label );

	while ( label )
	{
		BabylonText *text;
		ListSearch sh_text;

		text = label->FirstText ( sh_text );

		while ( text )
		{
			if ( text->IsDialog ())
			{
				if ( text->DialogIsPresent ( DialogPath, langid))
				{
					if ( !text->DialogIsValid ( DialogPath, langid, FALSE ) )
					{
						if ( print && pmask & PMASK_UNRESOLVED )
						{
							sprintf ( buffer, "%d: audio file \"%s%s.wav\" not verified", text->ID(), text->WaveSB (), linfo->character);

							print ( buffer );
						}
						info->unresolved++;
					}
					else
					{
						info->resolved++;
					}
				}
				else
				{
					if ( print && pmask & PMASK_MISSING )
					{
						sprintf ( buffer, "%d: audio file \"%s%s.wav\" missing", text->ID(), text->WaveSB (), linfo->character);

						print ( buffer );
					}
					info->missing++;
				}

				info->numdialog++;
			}

			text = label->NextText ( sh_text );
		}

		label = NextLabel ( sh_label );
	}

	return info->missing + info->unresolved + info->errors ;
}

int TransDB::ReportTranslations( TRNREPORT *report, LangID langid, void (*print) ( const char *buffer), PMASK pmask )
{
	BabylonLabel *label;
	ListSearch sh_label;
	int count = 0;
	int first_error = FALSE;
	TRNREPORT _info;
	TRNREPORT *info = &_info;

	if ( report )
	{
		info = report;
	}

	memset ( info, 0, sizeof ( TRNREPORT ));

	label = FirstLabel ( sh_label );

	while ( label )
	{
		BabylonText *text;
		ListSearch sh_text;
		int maxlen = label->MaxLen ();

		text = label->FirstText ( sh_text );

		while ( text )
		{
			int textnum = 0;
			Translation *trans;
			int too_big = FALSE;

			if ( text->Len ())
			{
				info->numstrings++;
				if ( langid != LANGID_US )
				{
					if ( (trans = text->GetTranslation ( langid ) ))
					{
						if ( maxlen && trans->Len() > maxlen )
						{
							if ( print && pmask & PMASK_TOOLONG )
							{
								sprintf ( buffer, "%d: translation is too long by %d characters", text->ID (), trans->Len() - maxlen);

								print ( buffer );
							}
							too_big = TRUE;
						}

						if ( text->Revision () > trans->Revision ())
						{
							if ( print && pmask & PMASK_RETRANSLATE )
							{
								sprintf ( buffer, "%d: needs re-translation", text->ID () );

								print ( buffer );
							}
							info->retranslate++;
						}
						else
						{
							info->translated++;
							if ( !trans->ValidateFormat ( text ) )
							{
								if ( print && pmask & PMASK_BADFORMAT )
								{
									sprintf ( buffer, "%d: translation has differring formating to original", text->ID () );

									print ( buffer );
								}
								info->bad_format++;
							}
						}
					}
					else
					{
						if ( print && pmask & PMASK_MISSING )
						{
							sprintf ( buffer, "%d: not translated", text->ID ());

							print ( buffer );
						}
						info->missing++;
					}
				}
				else
				{
					// check maxlen
					if ( maxlen )
					{
						if ( text->Len() > maxlen )
						{
							if ( print && pmask & PMASK_TOOLONG )
							{
								sprintf ( buffer, "%d: is too long by %d characters", text->ID (), text->Len() - maxlen);

								print ( buffer );
							}
							too_big = TRUE;
						}
					}
				}

			}

			if ( too_big )
			{
				info->too_big++;
			}
			text = label->NextText ( sh_text );
		}

		info->numlabels++;

		label = NextLabel ( sh_label );
	}

	info->errors = info->too_big + info->bad_format;

	return info->missing + info->too_big + info->retranslate + info->bad_format;
}

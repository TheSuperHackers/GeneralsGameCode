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
// loadsave.cpp
//

#include "StdAfx.h"
#include "iff.h"
#include "TransDB.h"
#include "BabylonDlg.h"

#define FORM_BABYLONDB			MakeID ('N','X','D','B')
#define FORM_LABEL			MakeID ('N','L','B','L')
#define FORM_TEXT				MakeID ('N','T','X','T')
#define FORM_TRANS			MakeID ('N','T','R','N')
#define CHUNK_COMMENT		MakeID ('C','M','N','T')
#define CHUNK_CONTEXT		MakeID ('C','T','X','T')
#define CHUNK_SPEAKER		MakeID ('S','P','K','R')
#define CHUNK_LISTENER	MakeID ('L','T','N','R')
#define CHUNK_TEXT			MakeID ('T','E','X','T')
#define CHUNK_WAVE			MakeID ('W','A','V','E')
#define CHUNK_WAVE_INFO	MakeID ('W','V','I','N')
#define CHUNK_INFO			MakeID ('I','N','F','O')
#define CHUNK_NAME			MakeID ('N','A','M','E')

#define MAX_BUFFER			(100*1024)

static OLECHAR	buffer[MAX_BUFFER];
typedef struct 
{
	int num_labels;
	int next_id;

} DBINFO;

typedef struct 
{
	int max_len;

} LBINFO;

typedef struct 
{
	int id;
	int revision;

} TXINFO;

typedef struct 
{
	LangID lang;
	int revision;

} TRINFO;

typedef struct 
{
	int valid;
	DWORD lo;
	DWORD hi;

} WVINFO;


static int writeString ( IFF_FILE *iff, OLECHAR *string, int chunk_id )
{
	int len = (wcslen ( string ) );
	int bytes = (len+1)*sizeof(OLECHAR);

	if ( len )
	{
		IFF_NEWCHUNK ( iff, chunk_id, error );
		IFF_WRITE ( iff, string, bytes, error);
		IFF_CloseChunk ( iff );

	}

	return TRUE;

error:

	return FALSE;
}

static int readString ( IFF_FILE *iff, OLECHAR *string )
{

	*string = 0;

	IFF_READ ( iff, string, iff->ChunkSize, error);

	return TRUE;

error:

	return FALSE;
}

static int writeTransForm ( IFF_FILE *iff, Translation *trans )
{
			TRINFO		trinfo;
			WVINFO		wvinfo;

			if ( !IFF_NewForm ( iff, FORM_TRANS ))
			{
				goto error;
			}
	
			trinfo.lang = trans->GetLangID ();
			trinfo.revision = trans->Revision ();
	
			if ( !IFF_NewChunk ( iff, CHUNK_INFO ))
			{
				goto error;
			}
			
			IFF_Write ( iff, &trinfo, sizeof ( trinfo ));
			
			IFF_CloseChunk ( iff );

			writeString ( iff, trans->Get (), CHUNK_TEXT );
			writeString ( iff, trans->Comment (), CHUNK_COMMENT );

			if ( (wvinfo.valid = trans->WaveInfo.Valid()) )
			{
				wvinfo.lo = trans->WaveInfo.Lo ();
				wvinfo.hi = trans->WaveInfo.Hi ();
				
				if ( !IFF_NewChunk ( iff, CHUNK_WAVE_INFO ))
				{
					goto error;
				}
				
				IFF_Write ( iff, &wvinfo, sizeof ( wvinfo ));
				
				IFF_CloseChunk ( iff );
			}

			IFF_CloseForm ( iff );

	return TRUE;
error:

	return FALSE;

}

static int writeTextForm ( IFF_FILE *iff, BabylonText *text )
{
			TXINFO		txinfo;
			WVINFO		wvinfo;

			if ( !IFF_NewForm ( iff, FORM_TEXT ))
			{
				goto error;
			}
	
			txinfo.id = text->ID ();
			txinfo.revision = text->Revision ();
	
			if ( !IFF_NewChunk ( iff, CHUNK_INFO ))
			{
				goto error;
			}
			
			IFF_Write ( iff, &txinfo, sizeof ( txinfo ));
			
			IFF_CloseChunk ( iff );
			writeString ( iff, text->Get (), CHUNK_TEXT );
			writeString ( iff, text->Wave (), CHUNK_WAVE );

			if ( (wvinfo.valid = text->WaveInfo.Valid()) )
			{
				wvinfo.lo = text->WaveInfo.Lo ();
				wvinfo.hi = text->WaveInfo.Hi ();
				
				if ( !IFF_NewChunk ( iff, CHUNK_WAVE_INFO ))
				{
					goto error;
				}
				
				IFF_Write ( iff, &wvinfo, sizeof ( wvinfo ));
				
				IFF_CloseChunk ( iff );
			}

			IFF_CloseForm ( iff );

	return TRUE;
error:

	return FALSE;

}

int WriteMainDB(TransDB *db, const char *filename, CBabylonDlg *dlg )
{
	BabylonText *text;
	BabylonLabel *label;
	ListSearch sh_label;
	ListSearch sh_text;
	int count = 0;
	IFF_FILE	*iff = NULL;
	DBINFO		dbinfo;
	int ok = FALSE;

	if ( dlg )
	{								 
		dlg->InitProgress ( db->NumLabels ());
	}


	if ( !( iff = IFF_New ( filename )))
	{
		goto error;
	}

	if ( !IFF_NewForm ( iff, FORM_BABYLONDB ) )
	{
		goto error;
	}
	
	dbinfo.next_id = db->ID ();
	dbinfo.num_labels = db->NumLabels ();

	if ( !IFF_NewChunk ( iff, CHUNK_INFO ))
	{
		goto error;
	}

	IFF_Write ( iff, &dbinfo, sizeof ( dbinfo ));

	IFF_CloseChunk ( iff );
	IFF_CloseForm ( iff );

	text = db->FirstObsolete ( sh_text );

	while ( text )
	{
		if ( !writeTextForm ( iff, text ) )
		{
				goto error;
		}	
		text = db->NextObsolete ( sh_text );
	}

	

	label = db->FirstLabel ( sh_label );

	while ( label )
	{
		LBINFO		lbinfo;

		if ( !IFF_NewForm ( iff, FORM_LABEL ))
		{
			goto error;
		}

		lbinfo.max_len = label->MaxLen ();

		if ( !IFF_NewChunk ( iff, CHUNK_INFO ))
		{
			goto error;
		}
		
		IFF_Write ( iff, &lbinfo, sizeof ( lbinfo ));
		
		IFF_CloseChunk ( iff );

		writeString ( iff, label->Name (), CHUNK_NAME );
		writeString ( iff, label->Comment (), CHUNK_COMMENT );
		writeString ( iff, label->Context (), CHUNK_CONTEXT );
		writeString ( iff, label->Speaker (), CHUNK_SPEAKER );
		writeString ( iff, label->Listener (), CHUNK_LISTENER );
		IFF_CloseForm ( iff );

		text = label->FirstText ( sh_text );

		while ( text )
		{
			Translation *trans;
			ListSearch sh_trans;
			if ( !writeTextForm ( iff, text ) )
			{
				goto error;
			}
			
			trans = text->FirstTranslation ( sh_trans );

			while ( trans )
			{
				if ( !writeTransForm ( iff, trans ) )
				{
					goto error;
				}

				trans = text->NextTranslation ( sh_trans );
			}

			text = label->NextText ( sh_text );
		}

		count++;
		if ( dlg )
		{
			dlg->SetProgress ( count );
		}
		label = db->NextLabel ( sh_label );
	}

	ok = TRUE;
	db->ClearChanges ();
		
error:
	if ( iff )
	{
		IFF_Close ( iff );
	}

	return ok;
}

int LoadMainDB(TransDB *db, const char *filename, void (*cb) (void) )
{
	BabylonLabel	*label = NULL;
	BabylonText		*text = NULL;
	Translation *trans = NULL;
	int count = 0;
	IFF_FILE	*iff = NULL;
	DBINFO		dbinfo;
	int ok = FALSE;


	if ( !(iff = IFF_Load ( filename ) ) )
	{
		goto error;
	}

	if ( !IFF_NextForm ( iff ) || iff->FormID != FORM_BABYLONDB )
	{
		goto error;
	}

	dbinfo.next_id = -1;
	dbinfo.num_labels = 0;

	while ( IFF_NextChunk ( iff ))
	{
		switch (iff->ChunkID )
		{
			case CHUNK_INFO:
				IFF_READ ( iff, &dbinfo, sizeof ( dbinfo ), error );
				break;
		}
	}

	db->SetID ( dbinfo.next_id );

	while ( IFF_NextForm ( iff ) )
	{
		switch ( iff->FormID )
		{
			case FORM_LABEL:
			{
				LBINFO		lbinfo;
				// new label

				if ( text )
				{
					if ( label )
					{
						label->AddText ( text );
					}
					else
					{
						db->AddObsolete ( text );
					}

					text = NULL;
				}

				if ( label )
				{
					count++;
					db->AddLabel ( label );
					label = NULL;
					if ( cb )
					{
						cb ();
					}
				}

				if ( ! (label = new BabylonLabel ()))
				{
					goto error;
				}

				while ( IFF_NextChunk ( iff ))
				{
					switch ( iff->ChunkID )
					{
						case CHUNK_INFO:
							IFF_READ ( iff, &lbinfo, sizeof (lbinfo), error );
							label->SetMaxLen ( lbinfo.max_len );
							break;
						case CHUNK_COMMENT:
							readString ( iff, buffer );
							label->SetComment ( buffer );
							break;
						case CHUNK_CONTEXT:
							readString ( iff, buffer );
							label->SetContext ( buffer );
							break;
						case CHUNK_SPEAKER:
							readString ( iff, buffer );
							label->SetSpeaker ( buffer );
							break;
						case CHUNK_LISTENER:
							readString ( iff, buffer );
							label->SetListener ( buffer );
							break;
						case CHUNK_NAME:
							readString ( iff, buffer );
							label->SetName ( buffer );
							break;
					}
				}
				break;
			}
			case FORM_TEXT:
			{
				TXINFO		txinfo;

				if ( text )
				{
					if ( label )
					{
						label->AddText ( text );
					}
					else
					{
						db->AddObsolete ( text );
					}

					text = NULL;
				}

				if ( ! (text = new BabylonText ()))
				{
					goto error;
				}

				while ( IFF_NextChunk ( iff ))
				{
					switch ( iff->ChunkID )
					{
						case CHUNK_INFO:
							IFF_READ ( iff, &txinfo, sizeof (txinfo), error );
							text->SetID ( txinfo.id );
							text->SetRevision ( txinfo.revision );
							break;
						case CHUNK_TEXT:
							readString ( iff, buffer );
							text->Set ( buffer );
							break;

						case CHUNK_WAVE:
							readString ( iff, buffer );
							text->SetWave ( buffer );
							break;
						case CHUNK_WAVE_INFO:
						{
							WVINFO		wvinfo;
							IFF_READ ( iff, &wvinfo, sizeof (wvinfo), error );
							text->WaveInfo.SetValid ( wvinfo.valid );
							text->WaveInfo.SetLo ( wvinfo.lo );
							text->WaveInfo.SetHi ( wvinfo.hi );
							break;
						}
					}

				}
				break;
			}
			case FORM_TRANS:
			{
				TRINFO		trinfo;

				if ( ! (trans = new Translation ()))
				{
					goto error;
				}

				while ( IFF_NextChunk ( iff ))
				{
					switch ( iff->ChunkID )
					{
						case CHUNK_INFO:
							IFF_READ ( iff, &trinfo, sizeof (trinfo), error );
							trans->SetLangID ( trinfo.lang );
							trans->SetRevision ( trinfo.revision );
							break;
						case CHUNK_TEXT:
							readString ( iff, buffer );
							trans->Set ( buffer );
							break;

						case CHUNK_COMMENT:
							readString ( iff, buffer );
							trans->SetComment ( buffer );
							break;
						case CHUNK_WAVE_INFO:
						{
							WVINFO		wvinfo;
							IFF_READ ( iff, &wvinfo, sizeof (wvinfo), error );
							trans->WaveInfo.SetValid ( wvinfo.valid );
							trans->WaveInfo.SetLo ( wvinfo.lo );
							trans->WaveInfo.SetHi ( wvinfo.hi );
						}
							break;

					}

				}
				if ( text )
				{
					text->AddTranslation ( trans );
				}
				else
				{
					delete trans;
				}
				trans = NULL;

				break;
			}
		}
	}

	if ( text )
	{
		if ( label )
		{
			label->AddText ( text );
		}
		else
		{
			db->AddObsolete ( text );
		}

		text = NULL;
	}

	if ( label )
	{
		count++;
		db->AddLabel ( label );
		label = NULL;
		if ( cb )
		{
			cb ();
		}
	}

	ok = TRUE;
		
error:

	if ( label )
	{
		delete label;
	}

	if ( text )
	{
		delete text;
	}

	if ( trans )
	{
		delete trans;
	}

	if ( iff )
	{
		IFF_Close ( iff );
	}

	db->ClearChanges ();

	if ( !ok )
	{
		db->Clear ();
	}

	return ok;
}


int	GetLabelCountDB ( char *filename )
{
	IFF_FILE	*iff = NULL;
	DBINFO		dbinfo;
	int count = 0;


	if ( !(iff = IFF_Open ( filename ) ) )
	{
		goto error;
	}

	if ( !IFF_NextForm ( iff ) || iff->FormID != FORM_BABYLONDB )
	{
		goto error;
	}

	dbinfo.next_id = -1;
	dbinfo.num_labels = 0;

	while ( IFF_NextChunk ( iff ))
	{
		switch (iff->ChunkID )
		{
			case CHUNK_INFO:
				IFF_READ ( iff, &dbinfo, sizeof ( dbinfo ), error );
				break;
		}
	}


	count = dbinfo.num_labels;

error:
	if ( iff )
	{
		IFF_Close ( iff );
	}
	return count;
}
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

//
// expimp.cpp
//

#include "StdAfx.h"
#include "TransDB.h"
#include "XLStuff.h"
#include "noxstringDlg.h"
#include "VerifyTextDlg.h"
#include "noxstring.h"
#include "expimp.h"
#include "direct.h"
#include "fileops.h"
#include "olestring.h"

static char buffer[100*1024];
static char buffer2[100*1024];
static OLECHAR olebuf[100*1024];
static OLECHAR olebuf2[100*1024];
static OLECHAR oletrans[100*1024];

static CNoxstringDlg *progress_dlg;
static int progress_count;

static void progress_cb ( void )
{
	progress_count ++;

	if ( progress_dlg )
	{
		progress_dlg->SetProgress ( progress_count );
	}
}

static FILE *cb_file;

static void print_to_file ( const char *text )
{

	fprintf ( cb_file, "\t\t\tString %s\n", text );

}

static void reverseWord ( OLECHAR *fp, OLECHAR *lp )
{
	int first = TRUE;
	OLECHAR f, l;

	while ( TRUE )
	{
		if ( fp >= lp )
		{
			return;
		}

		f = *fp;
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
		*fp++ = l;

	}

}


static void translateCopy( OLECHAR *outbuf, OLECHAR *inbuf )
{
	int slash = FALSE;

	{
		static OLECHAR buffer[100*1024];
		OLECHAR *firstLetter = NULL, *lastLetter;
		OLECHAR *b = buffer;
		int formatWord = FALSE;
		OLECHAR ch;

		while ( (ch = *inbuf++))
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
					while ( (ch = *inbuf++) && !IsFormatTypeChar ( ch ) && ch != '%')
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



	while( *inbuf != '\0' )
	{

			*outbuf++ = *inbuf++;
	}
	*outbuf= 0;
}


static void writeLabel ( NoxLabel *label, int row )
{
	PutCell ( row, CELL_LABEL, label->Name (), 0);
	wcscpy ( olebuf, label->Comment());
	EncodeFormat ( olebuf );
	PutCell ( row, CELL_COMMENT,  olebuf, 0);
	wcscpy ( olebuf, label->Context());
	EncodeFormat ( olebuf );
	PutCell ( row, CELL_CONTEXT, olebuf, 0);
	wcscpy ( olebuf, label->Speaker());
	EncodeFormat ( olebuf );
	PutCell ( row, CELL_SPEAKER, olebuf, 0);
	wcscpy ( olebuf, label->Listener());
	EncodeFormat ( olebuf );
	PutCell ( row, CELL_LISTENER, olebuf , 0);
}

static void writeText ( NoxText *text, int row )
{
	NoxLabel *label = text->Label ();
	int maxlen = label->MaxLen ();
	OLECHAR buffer[100];

	if ( !maxlen )
	{
		maxlen = 10000;
	}

	wcscpy ( olebuf, text->Get());
	EncodeFormat ( olebuf );
	PutCell ( row, CELL_ENGLISH, olebuf , 0);
	PutCell ( row, CELL_MAXLEN, NULL , maxlen );

	swprintf ( buffer, L"=LEN(%c%d)",'A' + CELL_LOCALIZED -1,  row );
	PutCell ( row, CELL_STRLEN, buffer , 0);
	swprintf ( buffer, L"=IF(%c%d>%c%d,\"Too long!\",\" \")", 'A' + CELL_STRLEN -1,  row, 'A' + CELL_MAXLEN -1,  row );
	PutCell ( row, CELL_LENCHECK ,buffer , 0);
	PutCell ( row, CELL_REVISION , 0, text->Revision ());
	PutCell ( row, CELL_STRINGID , 0, text->ID ());

	writeLabel ( label, row );
}

static int export_trans ( TransDB *db, LangID langid, TROPTIONS *options, void (*cb) (void ), int write )
{
	NoxLabel *label;
	NoxText *text;
	Translation *trans;
	ListSearch sh_label, sh_text;
	int count = 0;
	int limit = FALSE;
	int all = TRUE;
	int row;
	LANGINFO	*linfo;

	linfo = GetLangInfo ( langid );

	if ( options->filter == TR_SAMPLE )
	{
		limit = TRUE;
	}
	else if ( options->filter == TR_CHANGES )
	{
		all = FALSE;
	}

	if ( write )
	{
		OLECHAR buffer[100];

		swprintf ( buffer, L"%S", GetLangName ( langid ));
		PutCell ( ROW_LANGUAGE, COLUMN_LANGUAGE,buffer,0);

		swprintf ( buffer, L"%S Translation", GetLangName ( langid ));
		PutCell ( 2, CELL_LOCALIZED,buffer,0);
	}

	row = 3;
	label = db->FirstLabel ( sh_label );

	while ( label )
	{
		int label_written = FALSE;
		text = label->FirstText ( sh_text );
		
		while ( text )
		{
			int export;
			int bad_format = FALSE;
			int too_long = FALSE;

			trans = text->GetTranslation ( langid );

			if ( options->filter == TR_UNSENT )
			{
				export = !text->IsSent ();
			}
			else	if ( options->filter == TR_NONDIALOG )
			{
				export = !text->IsDialog ();
			}
			else if ( options->filter == TR_UNVERIFIED )
			{
					export = text->IsDialog() && text->DialogIsPresent( DialogPath, langid) && !text->DialogIsValid( DialogPath, langid);
			}
			else if ( options->filter == TR_MISSING_DIALOG )
			{
					export = text->IsDialog() && !text->DialogIsPresent( DialogPath, langid);
			}
			else if ( options->filter == TR_DIALOG )
			{
				export = text->IsDialog ();
			}
			else
			{
				if ( ! (export = all) )
				{
					if ( !trans )
					{
						export = TRUE;
					}
					else
					{
						if ( text->Revision () > trans->Revision ())
						{
							export = TRUE;
						}
						else if ( trans->TooLong ( label->MaxLen ()) )
						{
							export = TRUE;
							too_long = TRUE;
						}
						else if ( !trans->ValidateFormat ( text ) )
						{
							export = TRUE;
							bad_format = TRUE;
						}
					}
				}
			}

			if ( export && text->Len () )
			{
				count++;
				if ( cb )
				{
					cb ();
				}

				if ( write )
				{
					static OLECHAR buffer[100*1024];

					if ( !label_written )
					{
						// write out the lable
						//PutSeparator ( row++ );
						//writeLabel ( label, row );
						label_written = TRUE;
					}

					// write out text
					writeText ( text, row );
					if ( text->IsDialog ())
					{
						swprintf  ( buffer, L"%s%S.wav", text->Wave (), linfo->character );
						wcsupr ( buffer );
						PutCell ( row, CELL_WAVEFILE , buffer, 0);
					}

					{
						Translation *trans = text->GetTranslation ( langid );

						if ( langid == LANGID_JABBER || (trans && ( options->include_translations || too_long || bad_format )))
						{
						
							if ( langid == LANGID_JABBER )
							{
								translateCopy ( olebuf, text->Get() );
							}
							else
							{
								wcscpy ( olebuf, trans->Get());
							}

							EncodeFormat ( olebuf );
							PutCell ( row, CELL_LOCALIZED, olebuf, 0);
							if ( bad_format || too_long)
							{
									wcscpy ( olebuf, L"ERROR: " );
									if ( too_long )
									{
										wcscat ( olebuf, L"too long" );
										if ( bad_format )
										{
											wcscat ( olebuf, L"and " );
										}
									}

									if ( bad_format )
									{
										wcscat ( olebuf, L"bad format" );
									}

									PutCell ( row, CELL_COMMENT, olebuf , 0);
							}
						}
					}
					row++;
				}
			}

			text = label->NextText ( sh_text );
		}

		if ( limit && count > 50 )
		{
			break;
		}

		label = db->NextLabel ( sh_label );
	}

	if ( write )
	{
		PutCell ( row, CELL_STRINGID, NULL, -1 );
		PutCell ( ROW_COUNT, COLUMN_COUNT, NULL, count );
	}

	return count;
}


int ExportTranslations ( TransDB *db, const char *filename, LangID langid, TROPTIONS *options, CNoxstringDlg *dlg )
{
	int exports ;
	exports = export_trans ( db, langid, options, NULL, FALSE );

	if ( !exports )
	{
		if ( dlg )
		{
			AfxMessageBox ( "Nothing to export." );
			dlg->Ready ();
		}

		return 0;
	}

	if ( (progress_dlg = dlg) )
	{
		const char *format;
		dlg->InitProgress ( exports );

		dlg->Log ("");

		switch (options->filter )
		{
			case TR_ALL:
				format = "Exporting all strings";
				break;
			case TR_CHANGES:
				format = "Exporting all strings that require %s translation";
				break;
			case TR_SAMPLE:
				format = "Exporting a sample %s translation file";
				break;
			case TR_DIALOG:
				format = "Exporting dialog only %s translation file";
				break;
			case TR_NONDIALOG:
				format = "Exporting non-dialog %s translation file";
				break;
			case TR_UNVERIFIED:
				format = "Exporting all unverified %s dialog";
				break;
			case TR_MISSING_DIALOG:
				format = "Exporting all missing %s dialog";
				break;


			default:
				format = "Undefined switch";
				break;


		}
		strcpy ( buffer2, format );
		if ( options->include_comments && options->include_translations )
		{
			strcat ( buffer2, " with current %s translations and translator comments" );
		}
		else if ( options->include_comments )
		{
			strcat ( buffer2, " with %s translator comments" );
		}
		else if ( options->include_translations )
		{
			strcat ( buffer2, " with current %s translations" );
		}
		strcat ( buffer2, "..." );
		sprintf ( buffer, buffer2, GetLangName ( langid ), GetLangName ( langid ) );
		dlg->Status ( buffer );
	}

	_getcwd ( buffer, sizeof ( buffer ) -1 );

	strcat ( buffer, "\\babylon.xlt" );

	if ( !FileExists ( buffer ) )
	{
		if ( dlg )
		{
			dlg->Log ("FAILED", SAME_LINE );
			sprintf ( buffer2, "Template file \"%s\" is missing. Cannot export.", buffer );
			AfxMessageBox ( buffer2 );
			dlg->Log ( buffer2 );
			dlg->Ready();
		}
		return -1;
	}

	progress_count = 0;
	exports = -1;

	if ( NewWorkBook ( buffer ) )
	{
		if ( (exports = export_trans ( db, langid, options, progress_cb, TRUE )) != -1 )
		{
			if ( SaveWorkBook ( filename, TRUE ) )
			{
				if ( dlg )
				{
					dlg->Log ("DONE", SAME_LINE );
				}
			}
			else
			{
				if ( dlg )
				{
					dlg->Log ("FAILED", SAME_LINE );
					sprintf ( buffer2, "Failed to save export!");
					AfxMessageBox ( buffer2 );
					dlg->Log ( buffer2 );
				}
				exports = -1;
			}
		}
		CloseWorkBook ( );
	}
	else
	{
		if ( dlg )
		{
			dlg->Log ("FAILED", SAME_LINE );
			sprintf ( buffer2, "Failed to create new work book. File \"%s\" may be corrupt", buffer );
			AfxMessageBox ( buffer2 );
			dlg->Log ( buffer2 );
		}
	}

	if ( dlg )
	{
		dlg->Ready();
	}

	return exports;
}


static int import_trans ( TransDB *db, LangID langid, void (*cb) ( void ), CNoxstringDlg *dlg )
{
	int row = 3;
	int id;
	int count = 0;
	int new_count = 0;
	int changes_count = 0;
	int missing_count = 0;
	int mismatch_count = 0;
	int stale_count = 0;
	int	first_mismatch = TRUE;
	int bad_id = FALSE;
	int error_count = 0;
	int revision;

	while ( (id = GetInt ( row, CELL_STRINGID )) != -1)
	{
		if ( id == 0 )
		{
			goto skip;
		}

		NoxText *text;

		if ( (text = db->FindText ( id )) == NULL )
		{
			// string is no longer in database
			stale_count++;
			goto next;
		}

		revision = GetInt ( row, CELL_REVISION );
				
		if ( text->Revision() > revision )
		{
			// old translation
			stale_count++;
			goto next;
		}

		if ( text->Revision() < revision )
		{
			if ( dlg )
			{
				sprintf ( buffer, "ERROR: expecting revision %d for string ID %d but found revision %d. Possible bad ID!", text->Revision (), id, revision );
				dlg->Log ( buffer );
			}
			error_count++;
			goto next;
		}
				
		// first see if there is any translation there
		GetString ( row, CELL_LOCALIZED, oletrans );
		DecodeFormat ( oletrans );

		if ( !oletrans[0] )
		{
			missing_count++;
			goto next;
		}

		// verify that the translated engish is the same as the current english
		GetString ( row, CELL_ENGLISH, olebuf );
		DecodeFormat ( olebuf );
		
		if ( wcscmp ( text->Get(), olebuf ) )
		{
			// they are two possible reasons for the text to mismatch
			// 1. text was modified but not re-translated
			// 2. the IDs are wrong
			// 
			// to check for the first case we search for the label in the xl file
			// and make sure it is the same. If not then we have problems

			int nrow = row;

			olebuf2[0] = 0;
			while ( nrow > 0 )
			{
				GetString ( nrow, CELL_LABEL, olebuf2 );
				if ( olebuf2[0] )
				{
					break;
				}
				nrow--;
			}


			if ( !olebuf2[0] || wcscmp ( text->Label ()->Name(), olebuf2))
			{
				sprintf ( buffer, "%S", olebuf );
				CVerifyTextDlg dlg(buffer, text->GetSB());

				// didnt find label or label doesn't match 
				// It is possible that the xl was resorted so ask user to do a visual confirmation

				bad_id = dlg.DoModal ()==IDNO;
			} 
			else if ( text->Label()->FindText( olebuf ))
			{
				// we did find the label but text other than the current ID sourced text matches with the import text
				// this means the ID is definitely wrong
				bad_id = TRUE;
			}
			else
			{
				bad_id = FALSE;
			}

			if ( bad_id )
			{
				goto done;
			}

		}

		// ok import the translation

		Translation *trans;
						
		if ( ! (trans = text->GetTranslation ( langid )))
		{
				new_count++;
		
				trans = new Translation	 ();
				trans->SetLangID ( langid );
				text->AddTranslation ( trans );
		}
		
		
		if ( trans->Revision () == revision && !wcscmp ( trans->Get (), oletrans ))
		{
			// already up to date
			goto next;
		}

		trans->Set ( oletrans );
		trans->WaveInfo.SetValid ( FALSE );
		trans->SetRevision ( revision );
		changes_count++;
		

		next:				
				count++;
				
				if ( cb )
				{
					cb ();
				}

		skip:
			row++;
	}

done:

	if ( dlg )
	{
			sprintf ( buffer, "Total found : %d", count );
		dlg->Log ( buffer );

		{
			sprintf ( buffer, "New         : %d", new_count);
			dlg->Log ( buffer );
		}
		{
			sprintf ( buffer, "Updates     : %d", (changes_count - new_count));
			dlg->Log ( buffer );
		}
		if ( missing_count )
		{
			sprintf ( buffer, "Missing     : %d", missing_count );
			dlg->Log ( buffer );
		}
		if ( stale_count )
		{
			sprintf ( buffer, "Unmatched   : %d", stale_count);
			dlg->Log ( buffer );
		}

	}

		if ( bad_id )
		{
			if ( dlg )
			{
				sprintf ( buffer, "Aborting import: BAD IDs");
				dlg->Log ( buffer );
			}

			AfxMessageBox ("The imported translation file has bad string IDs! Fix the string IDs and re-import" );
		}
	return count;
}

static int update_sent_trans ( TransDB *db, LangID langid, void (*cb) ( void ), CNoxstringDlg *dlg )
{
	int row = 3;
	int id;
	int count = 0;
	int new_count = 0;
	int matched = 0;
	int unmatched = 0;
	int changed = 0;
	int	first_mismatch = TRUE;
	int bad_id = FALSE;
	int error_count = 0;
	int revision;

	while ( (id = GetInt ( row, CELL_STRINGID )) != -1)
	{
		if ( id == 0 )
		{
			goto skip;
		}

		NoxText *text;

		if ( (text = db->FindText ( id )) == NULL )
		{
			// string is no longer in database
			unmatched++;
			goto next;
		}

		revision = GetInt ( row, CELL_REVISION );
				
		if ( text->Revision() > revision )
		{
			// old translation
			changed++;
			goto next;
		}

		if ( text->Revision() < revision )
		{
			if ( dlg )
			{
				sprintf ( buffer, "ERROR: expecting revision %d for string ID %d but found revision %d. Possible bad ID!", text->Revision (), id, revision );
				dlg->Log ( buffer );
			}
			error_count++;
			goto next;
		}

				
		// verify that the translated engish is the same as the current english
		GetString ( row, CELL_ENGLISH, olebuf );
		DecodeFormat ( olebuf );
		
		if ( wcscmp ( text->Get(), olebuf ) )
		{
			// they are two possible reasons for the text to mismatch
			// 1. text was modified but not re-translated
			// 2. the IDs are wrong
			// 
			// to check for the first case we search for the label in the xl file
			// and make sure it is the same. If not then we have problems

			int nrow = row;

			olebuf2[0] = 0;
			while ( nrow > 0 )
			{
				GetString ( nrow, CELL_LABEL, olebuf2 );
				if ( olebuf2[0] )
				{
					break;
				}
				nrow--;
			}


			if ( !olebuf2[0] || wcscmp ( text->Label ()->Name(), olebuf2))
			{
				sprintf ( buffer, "%S", olebuf );
				CVerifyTextDlg dlg(buffer, text->GetSB());

				// didnt find label or label doesn't match 
				// It is possible that the xl was resorted so ask user to do a visual confirmation

				bad_id = dlg.DoModal ()==IDNO;
			} 
			else if ( text->Label()->FindText( olebuf ))
			{
				// we did find the label but text other than the current ID sourced text matches with the import text
				// this means the ID is definitely wrong
				bad_id = TRUE;
			}
			else
			{
				bad_id = FALSE;
			}

			if ( bad_id )
			{
				goto done;
			}

		}
		else
		{
			// text is still the same
			text->Sent ( TRUE );
			matched++;

		}

		next:				
				count++;
				
				if ( cb )
				{
					cb ();
				}

		skip:
			row++;
	}

done:

	if ( dlg )
	{
			sprintf ( buffer, "Total found : %d", count );
		dlg->Log ( buffer );

		{
			sprintf ( buffer, "Matched         : %d", matched);
			dlg->Log ( buffer );
		}
		{
			sprintf ( buffer, "Unmatched     : %d", unmatched);
			dlg->Log ( buffer );
		}

		if ( changed )
		{
			sprintf ( buffer, "changed   : %d", changed);
			dlg->Log ( buffer );
		}

	}

		if ( bad_id )
		{
			if ( dlg )
			{
				sprintf ( buffer, "Aborting import: BAD IDs");
				dlg->Log ( buffer );
			}

			AfxMessageBox ("The imported translation file has bad string IDs! Fix the string IDs and re-import" );
		}
	return count;
}

int ImportTranslations ( TransDB *db, const char *filename, CNoxstringDlg *dlg )
{
	int imports = -1;

	progress_dlg = dlg;
	if ( dlg )
	{
		dlg->Log ("");
		sprintf ( buffer, "Importing \"%s\"...", filename );
		dlg->Status ( buffer );
	}

	if ( OpenWorkBook ( filename ) )
	{
		int num_strings;
		LANGINFO *info;

		num_strings = GetInt ( ROW_COUNT, COLUMN_COUNT );
		GetString ( ROW_LANGUAGE, COLUMN_LANGUAGE, olebuf );
		sprintf ( buffer, "%S", olebuf );
		info = GetLangInfo ( buffer );

		if ( !info )
		{
			if ( dlg )
			{
				AfxMessageBox ( "Import file is of an unknown language or is not a translation file" );
				dlg->Log ( "FAILED", SAME_LINE );
				dlg->Ready();
			}
			CloseWorkBook ();
			return -1;
		}


		if ( dlg )
		{
			dlg->InitProgress ( num_strings );
			progress_count = 0;
			sprintf ( buffer, "...%s", info->name );
			dlg->Log ( buffer, SAME_LINE );
		}

		imports = import_trans ( db, info->langid, progress_cb, dlg );

		CloseWorkBook ( );
	}
	else
	{
		if ( dlg )
		{
			dlg->Log ("FAILED", SAME_LINE );
			sprintf ( buffer2, "Failed to open \"%s\"", buffer );
			AfxMessageBox ( buffer2 );
			dlg->Log ( buffer2 );
		}
	}

	if ( dlg )
	{
		dlg->Ready();
	}

	return imports;
}

static int generate_noxstr ( TransDB *db, const char *filename, LangID langid, GNOPTIONS *options )
{
	int ok = FALSE;
	FILE *file;

	if ( ! ( file = fopen ( filename, "wt" ) ))
	{
		goto error;
	}

	fprintf ( file, "// Generated by %s\n", AppTitle );
	fprintf ( file, "// Generated on %s %s\n\n\n", __DATE__, __TIME__ );

	{
		NoxLabel *label;
		NoxText *text;
		Translation *trans;
		ListSearch sh_label, sh_text;

		label = db->FirstLabel ( sh_label );
	
		while ( label )
		{
			text = label->FirstText ( sh_text );
		
			fprintf ( file, "\n\n%s\n", label->NameSB ());
				
			while ( text )
			{
				const char *string;

				trans = text->GetTranslation ( langid );

				if ( !trans )
				{
					if ( langid == LANGID_US )
					{
						string = text->GetSB ();
					}
					else
					{
						if ( text->Len ())
						{
							if ( options->untranslated == GN_USEIDS )
							{
								string = buffer2;
								sprintf (buffer2, "%d", text->ID ());
							}
							else
							{
								string = text->GetSB();
							}
						}
						else
						{
							string = "";
						}
					}
				}
				else
				{
					string = trans->GetSB ();
				}

				if ( text->Len() == 0 )
				{
					string = "";
				}

				fprintf ( file, "\"%s\" %s\n", string, text->WaveSB() );
				text = label->NextText ( sh_text );
			}
	
			fprintf ( file, "END\n" );
			label = db->NextLabel ( sh_label );
		}

		ok = TRUE;
	}
error:

	if ( file )
	{
		fclose ( file );
	}

	return ok;
}

static int writeCSFLabel ( FILE *file, NoxLabel *label )
{
	int id = CSF_LABEL;
	int len = strlen ( label->NameSB() );
	int strings = label->NumStrings ();

	if ( fwrite ( &id, sizeof ( int ), 1, file ) != 1 )
	{
		return FALSE;
	}

	if ( fwrite ( &strings, sizeof ( int ), 1, file ) != 1 )
	{
		return FALSE;
	}

	if ( fwrite ( &len, sizeof ( int ), 1, file ) != 1 )
	{
		return FALSE;
	}

	if ( !len )
	{
		return FALSE;
	}

	if ( fwrite ( label->NameSB(), len, 1, file ) != 1 )
	{
		return FALSE;
	}

	return TRUE;
}

static int writeCSFString ( FILE *file, const OLECHAR *string, char *wave, LANGINFO *linfo )
{
	int id = CSF_STRING;
	int len ;
	int wlen = strlen ( wave );

	if ( wlen )
	{
		id = CSF_STRINGWITHWAVE;
	}
	
	wcscpy ( olebuf, string );
	StripSpaces ( olebuf );
	ConvertMetaChars ( olebuf );
	len = wcslen ( olebuf );

	{
		OLECHAR *ptr = olebuf;

		while ( *ptr)
		{
			*ptr = ~*ptr++;
		}

	}

	if ( fwrite ( &id, sizeof ( int ), 1, file ) != 1 )
	{
		return FALSE;
	}

	if ( fwrite ( &len, sizeof ( int ), 1, file ) != 1 )
	{
		return FALSE;
	}

	if ( len )
	{
		if ( fwrite ( olebuf, len*sizeof(OLECHAR), 1, file ) != 1 )
		{
			return FALSE;
		}
	}
	if ( wlen )
	{
		wlen++;
		if ( fwrite ( &wlen, sizeof ( int ), 1, file ) != 1 )
		{
			return FALSE;
		}
		
		if ( fwrite ( wave, wlen-1, 1, file ) != 1 )
		{
			return FALSE;
		}
		if ( fwrite ( linfo->character, 1, 1, file ) != 1 )
		{
			return FALSE;
		}
	}
	return TRUE;
}

static int generate_csf ( TransDB *db, const char *filename, LangID langid, GNOPTIONS *options )
{
	CSF_HEADER header;
	int header_size;
	int ok = FALSE;
	FILE *file;
	LANGINFO	*linfo = GetLangInfo ( langid);

	if ( ! ( file = fopen ( filename, "w+b" ) ))
	{
		goto error;
	}

	header.id = CSF_ID;
	header.version = CSF_VERSION;
	header.skip = 0;
	header.num_labels = 0;
	header.num_strings = 0;
	header.langid = langid;
	header_size = sizeof ( header );

	fseek ( file, header_size, SEEK_SET );

	{
		NoxLabel *label;
		NoxText *text;
		Translation *trans;
		ListSearch sh_label, sh_text;

		label = db->FirstLabel ( sh_label );
	
		while ( label )
		{
			text = label->FirstText ( sh_text );
		
			if ( !writeCSFLabel ( file, label ) )
			{
				goto error;
			}
			header.num_labels++;
				
			while ( text )
			{
				const OLECHAR *string;

				trans = text->GetTranslation ( langid );

				if ( !trans )
				{
					if ( langid == LANGID_US )
					{
						string = text->Get ();
					}
					else
					{
						if ( text->Len ())
						{
							if ( options->untranslated == GN_USEIDS )
							{
								string = olebuf2;
								swprintf (olebuf2, L"%d", text->ID ());
							}
							else
							{
								string = text->Get();
							}
						}
						else
						{
							string = L"";
						}
					}
				}
				else
				{
					string = trans->Get ();
				}

				if ( !writeCSFString ( file, string, text->WaveSB (), linfo ) )
				{
					goto error;
				}
				header.num_strings ++;

				text = label->NextText ( sh_text );
			}
	
			label = db->NextLabel ( sh_label );
		}

		fseek ( file, 0, SEEK_SET );
		if ( fwrite ( &header, header_size, 1, file ) != 1 )
		{
			goto error;
		}

		fseek ( file, 0, SEEK_END );

		ok = TRUE;
	}

error:

	if ( file )
	{
		fclose ( file );
	}


	return ok;
}


int GenerateGameFiles ( TransDB *db, const char *filepattern, GNOPTIONS *options, LangID *languages, CNoxstringDlg *dlg)
{
	static char filename[2*1024];
	LangID langid;
	int count= 0 ;
	int num;

	if ( dlg )
	{
		LangID *temp = languages;
		num = 0;
		while ( *temp++ != LANGID_UNKNOWN )
		{
			num++;
		}

		dlg->Log ( "" );
		dlg->Status ( "Generating game files:" );
		dlg->InitProgress ( num );
		num = 0;
	}
	while ( (langid = *languages++) != LANGID_UNKNOWN )
	{
		LANGINFO *info;
		TRNREPORT trnreport;
		DLGREPORT dlgreport;
		int dlgwarning;
		int trnwarning;
		int done;

		info = GetLangInfo ( langid );

		sprintf ( filename, "%s_%s.%s", filepattern, info->initials, options->format == GN_NOXSTR ? "str" : "csf" );
		strlwr ( filename );

		if ( dlg )
		{
			sprintf ( buffer, "Writing: %s  - %s...", filename, GetLangName ( langid ));
			dlg->Status ( buffer );
			dlgwarning = db->ReportDialog ( &dlgreport, langid );
			trnwarning = db->ReportTranslations ( &trnreport, langid );
		}


		if ( options->format == GN_NOXSTR )
		{
			done = generate_noxstr ( db, filename, langid, options );
		}
		else
		{
			done = generate_csf ( db, filename, langid, options );
		}

		if ( done )
		{
			count++;
			if ( dlg )
			{
				if ( trnwarning || dlgwarning )
				{
					dlg->Log ( "WARNING", SAME_LINE );

					if ( trnwarning )
					{
						int missing;

						if ( (missing = trnreport.missing + trnreport.retranslate) )
						{
							sprintf ( buffer, "%d translation%s missing", missing, missing > 1 ? "s are" : " is" );
							dlg->Log ( buffer );
						}

						if ( trnreport.too_big )
						{
							sprintf ( buffer, "%d string%s too big", trnreport.too_big, trnreport.too_big > 1 ? "s are" : " is" );
							dlg->Log ( buffer );
						}

						if ( trnreport.bad_format )
						{
							sprintf ( buffer, "%d translation%s bad format", trnreport.bad_format, trnreport.bad_format > 1 ? "s have a" : " has a" );
							dlg->Log ( buffer );
						}
					}

					if ( dlgwarning )
					{
						if ( dlgreport.missing )
						{
							sprintf ( buffer, "%d dialog%s missing", dlgreport.missing, dlgreport.missing > 1 ? "s are" : " is" );
							dlg->Log ( buffer );
						}

						if ( dlgreport.unresolved )
						{
							sprintf ( buffer, "%d dialog%s not verified", dlgreport.unresolved, dlgreport.unresolved> 1 ? "s are" : " is" );
							dlg->Log ( buffer );
						}
					}
				}
				else
				{
					dlg->Log ( "OK", SAME_LINE );
				}
			}
		}
		else
		{
			if ( dlg )
			{
				dlg->Log ( "FAILED", SAME_LINE );
			}
		}

		if ( dlg )
		{
			dlg->SetProgress ( ++num );
		}

	}

	if ( dlg )
	{
		dlg->Ready ();
	}
	return count;
}

void ProcessWaves ( TransDB *db, const char *filename, CNoxstringDlg *dlg )
{
	int imports = -1;

	progress_dlg = dlg;

	if ( dlg )
	{
		dlg->Log ("");
		sprintf ( buffer, "Processing wavefile \"%s\"...", filename );
		dlg->Status ( buffer );
	}

	if ( OpenWorkBook ( filename ) )
	{
		int row = 1;
		int last_row = 1;
		int matches = 0;
		int unmatched = 0;
		FILE *file = NULL; 
		char *ptr;

		strcpy ( buffer, filename );

		if ( (ptr = strchr ( buffer, '.' )) )
		{
			*ptr = 0;
		}

		strcat ( buffer, ".txt" );

		if ( (file = fopen (buffer, "wt" )))
		{

			while ( row - last_row < 1000 )
			{
				NoxText *text;

				GetString ( row, 'J' - 'A' + 1, olebuf );

				wcslwr ( olebuf );

				if ( wcsstr ( olebuf, L".wav" ) )
				{
					last_row = row;

					fprintf ( file, "%S : ", olebuf );
					GetString ( row, 'K' -'A' + 1, olebuf );
					StripSpaces ( olebuf );

					if ( (text = db->FindSubText ( olebuf ) ))
					{
						fprintf ( file, "%6d", text->LineNumber () );
					}
					else
					{
						fprintf ( file, "??????" );
					}

					fprintf ( file, " - \"%S\"\n", olebuf );
				}

				row++;
			}

			fclose ( file );
		}

		CloseWorkBook ( );
	}
	else
	{
		if ( dlg )
		{
			dlg->Log ("FAILED", SAME_LINE );
			sprintf ( buffer2, "Failed to open \"%s\"", buffer );
			AfxMessageBox ( buffer2 );
			dlg->Log ( buffer2 );
		}
	}

	if ( dlg )
	{
		dlg->Ready();
	}

}


int GenerateReport ( TransDB *db, const char *filename, RPOPTIONS *options, LangID *languages, CNoxstringDlg *dlg)
{
	LangID langid;
	int count= 0 ;
	int num;
	FILE *file = NULL;

	if ( dlg )
	{
		LangID *temp = languages;
		num = 0;
		while ( *temp++ != LANGID_UNKNOWN )
		{
			num++;
		}

		dlg->Log ( "" );
		dlg->Status ( "Generating Report:" );
		dlg->InitProgress ( num );
		num = 0;
	}

	if ( ! ( file = fopen ( filename, "wt" )))
	{
		static char buffer[500];

		sprintf ( buffer, "Unable to open file \"%s\".\n\nCannot create report!", filename);
		AfxMessageBox ( buffer );

		if ( dlg )
		{
			dlg->Log ( "FAILED", SAME_LINE );
			dlg->Ready ();
		}
		return 0;
	}

	cb_file = file;

	{
		char date[50];
		char time[50];
		_strtime ( time );
		_strdate ( date );
		fprintf ( file, "Noxstring Report: %s %s\n", date, time);
	}



	while ( (langid = *languages++) != LANGID_UNKNOWN )
	{
		LANGINFO *info;
		TRNREPORT tr_report;
		DLGREPORT dlg_report;

		info = GetLangInfo ( langid );

		fprintf ( file, "\n\n%s Status:\n", info->name );


		if ( options->translations )
		{
			int count;

			count = db->ReportTranslations ( &tr_report, langid );
			
			fprintf ( file, "\n\tText Summary: %s\n", info->name );
			fprintf ( file,   "\t-------------\n\n");

			fprintf ( file, "\t\tErrors: %d\n", tr_report.errors);

			if ( langid != LANGID_US )
			{
				fprintf ( file, "\t\tNot translated: %d\n", tr_report.missing);
				fprintf ( file, "\t\tRetranslation: %d\n", tr_report.retranslate);
				fprintf ( file, "\t\tTranslated: %d\n", tr_report.translated );
			}
			fprintf ( file, "\t\tTotal text: %d\n", tr_report.numstrings );

			if ( count && count < options->limit )
			{
				fprintf ( file, "\n\tText Details: %s\n", info->name);
				fprintf ( file,   "\t------------\n\n" );
				db->ReportTranslations ( &tr_report, langid, print_to_file );
			}

		}

		if ( options->dialog )
		{
			int count;

			count = db->ReportDialog ( &dlg_report, langid );
			
			fprintf ( file, "\n\tDialog Summary: %s\n", info->name );
			fprintf ( file,   "\t-------------\n\n");

			fprintf ( file, "\t\tMissing Audio: %d\n", dlg_report.missing);
			fprintf ( file, "\t\tNot verified: %d\n", dlg_report.unresolved);
			fprintf ( file, "\t\tVerified: %d\n", dlg_report.resolved);
			fprintf ( file, "\t\tTotal dialog: %d\n", dlg_report.numdialog );

			if ( count && count < options->limit )
			{
				fprintf ( file, "\n\tDialog Details: %s\n", info->name );
				fprintf ( file,   "\t------------\n\n" );
				db->ReportDialog ( &dlg_report, langid, print_to_file );
			}
		}

		if ( dlg )
		{
			dlg->SetProgress ( ++num );
		}

	}

	if ( dlg )
	{
		dlg->Ready ();
	}

	fclose ( file );

	return count;
}

int UpdateSentTranslations ( TransDB *db, const char *filename, CNoxstringDlg *dlg )
{
	int imports = -1;

	progress_dlg = dlg;
	if ( dlg )
	{
		dlg->Log ("");
		sprintf ( buffer, "Importing \"%s\"...", filename );
		dlg->Status ( buffer );
	}

	if ( OpenWorkBook ( filename ) )
	{
		int num_strings;
		LANGINFO *info;

		num_strings = GetInt ( ROW_COUNT, COLUMN_COUNT );
		GetString ( ROW_LANGUAGE, COLUMN_LANGUAGE, olebuf );
		sprintf ( buffer, "%S", olebuf );
		info = GetLangInfo ( buffer );

		if ( !info )
		{
			if ( dlg )
			{
				AfxMessageBox ( "Import file is of an unknown language or is not a translation file" );
				dlg->Log ( "FAILED", SAME_LINE );
				dlg->Ready();
			}
			CloseWorkBook ();
			return -1;
		}


		if ( dlg )
		{
			dlg->InitProgress ( num_strings );
			progress_count = 0;
			sprintf ( buffer, "...%s", info->name );
			dlg->Log ( buffer, SAME_LINE );
		}

		imports = update_sent_trans ( db, info->langid, progress_cb, dlg );

		CloseWorkBook ( );
	}
	else
	{
		if ( dlg )
		{
			dlg->Log ("FAILED", SAME_LINE );
			sprintf ( buffer2, "Failed to open \"%s\"", buffer );
			AfxMessageBox ( buffer2 );
			dlg->Log ( buffer2 );
		}
	}

	if ( dlg )
	{
		dlg->Ready();
	}

	return imports;
}


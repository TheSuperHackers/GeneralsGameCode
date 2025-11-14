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


#include "StdAfx.h"
#include "sys/stat.h"
#include "iff.h"
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>

#define IFF_RAWREAD(iff,data,size,label)		{if ( IFF_rawread ( (iff), (data), (size)) != (size)) goto label;}


int		IFF_rawread ( IFF_FILE *iff, void *buffer, int bytes )
{
	if ( ! (iff->flags & mIFF_FILE_LOADED ) )
	{
		return	_read ( iff->fp, buffer, bytes );
	}

	if ( iff->file_size < (iff->file_pos + bytes) )
	{
		bytes = iff->file_size - iff->file_pos;
	}
	memcpy ( buffer, &iff->mem_file[iff->file_pos], bytes );
	iff->file_pos += bytes;
	return bytes;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		IFF_seek ( IFF_FILE *iff, int pos, int mode )
{

	if ( ! (iff->flags & mIFF_FILE_LOADED ))
	{
		return	lseek ( iff->fp, pos, mode );
	}

	switch ( mode )
	{
		case SEEK_CUR:
			iff->file_pos += pos;
			break;
		case SEEK_END:
			iff->file_pos = iff->file_size - pos;
			break;
		case SEEK_SET:
			iff->file_pos = pos;
			break;
	}

	if ( iff->file_pos < 0 )
	{
		iff->file_pos = 0;
	}
	else
	{
		if ( iff->file_pos > iff->file_size )
		{
			iff->file_pos = iff->file_size;
		}
	}

	return iff->file_pos;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

IFF_FILE	*IFF_Open ( const char *name )
{
	IFF_FILE *iff = NULL;


	if ( ! (iff = (IFF_FILE *) malloc ( sizeof (IFF_FILE))))
	{
		goto error;
	}

	iff->fp = -1;
	memset ( iff, 0, sizeof ( IFF_FILE ));

	if ((iff->fp = open ( name, _O_BINARY | _O_RDONLY )) == -1 )
	{
		goto error;
	}


	return iff;

error:

	if (iff)
	{
		IFF_Close ( iff );
	}

	return NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

IFF_FILE	*IFF_Load ( const char *name )
{
	IFF_FILE *iff = NULL;

	if ( ! (iff = (IFF_FILE *) malloc ( sizeof (IFF_FILE))))
	{
		goto error;
	}

	memset ( iff, 0, sizeof ( IFF_FILE ));
	iff->fp = -1;

	if ((iff->fp = open ( name, _O_BINARY | _O_RDONLY )) == -1 )
	{
		goto error;
	}

	iff->file_size = lseek ( iff->fp, 0, SEEK_END );
	lseek ( iff->fp, 0, SEEK_SET );

	if ( !(iff->mem_file = ( char *) malloc ( iff->file_size) ) )
	{
		goto error;
	}

	DO_READ ( iff->fp, iff->mem_file, iff->file_size, error );

	iff->flags |= mIFF_FILE_LOADED;

	return iff;

error:

	if (iff)
	{
		IFF_Close ( iff );
	}

	return NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	IFF_Reset ( IFF_FILE *iff )
{
	IFF_seek ( iff, 0, SEEK_SET );
	iff->pad_form = 0;
	iff->pad_chunk = 0;
	iff->FormSize = 0;
	iff->ChunkSize = 0;
	iff->next_byte = 0;
	iff->chunk_size_pos = 0;
	iff->form_size_pos = 0;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	IFF_goto_form_end ( IFF_FILE *iff )
{

	iff->FormSize += iff->pad_form;
	iff->pad_form = 0;
	if (iff->FormSize)
	{
		IFF_seek ( iff, iff->FormSize, SEEK_CUR );
		iff->next_byte += iff->FormSize;
		iff->FormSize = 0;
		iff->ChunkSize = 0;
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	IFF_goto_chunk_end ( IFF_FILE *iff )
{

	iff->ChunkSize += iff->pad_chunk;
	iff->pad_chunk = 0;
	if (iff->ChunkSize)
	{
		IFF_seek ( iff, iff->ChunkSize, SEEK_CUR );
		iff->next_byte += iff->ChunkSize;
		iff->FormSize -= iff->ChunkSize;
		iff->ChunkSize = 0;
	}
}



/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		IFF_NextForm ( IFF_FILE *iff )
{
	IFF_CHUNK chunk;
	int	form;

   	IFF_goto_form_end ( iff );

   	IFF_RAWREAD (iff, &chunk, sizeof( IFF_CHUNK ), error );

   	chunk.Size = 	BgEn32 (chunk.Size);
   	chunk.ID =  	BgEn32 (chunk.ID);

   	iff->pad_form = (int) (chunk.Size & 0x0001);

   	if (chunk.ID != vIFF_ID_FORM )
	{
   		goto error;
	}

   	IFF_RAWREAD (iff, &form, sizeof( int ), error);

   	iff->FormID = 	(int) BgEn32 (form);

   	iff->flags |= mIFF_FILE_FORMOPEN;
   	iff->next_byte += sizeof( int ) + sizeof ( IFF_CHUNK );
   	iff->FormSize = (int) chunk.Size - sizeof ( int );
   	iff->ChunkSize = 0;
   	iff->pad_chunk = 0;

   	return 	TRUE;

error:

   	return	FALSE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		IFF_NextChunk ( IFF_FILE *iff )
{
	IFF_CHUNK chunk;

	IFF_goto_chunk_end ( iff );

	if (iff->FormSize==0)
	{
		goto error;
	}

	IFF_RAWREAD ( iff, &chunk, sizeof( IFF_CHUNK ), error );

	chunk.Size = 	BgEn32 (chunk.Size);
	chunk.ID =  	BgEn32 (chunk.ID);

	iff->pad_chunk = (int) (chunk.Size & 0x0001);

	iff->flags |= mIFF_FILE_CHUNKOPEN;
	iff->ChunkID = (int) chunk.ID;
	iff->ChunkSize = (int) chunk.Size;
	iff->next_byte +=  sizeof ( IFF_CHUNK );
	iff->FormSize -= sizeof ( IFF_CHUNK );

	return 	TRUE;

error:

	return	FALSE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	IFF_Close ( IFF_FILE *iff )
{
	if (iff->fp != -1)
	{
		_close (iff->fp);
	}

	if ( iff->mem_file )
	{
		free ( iff->mem_file );
	}

	free ( iff );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		IFF_Read ( IFF_FILE *iff, void *buff, int size )
{
	int	read =0;

	if ( size>iff->ChunkSize )
	{
		size = iff->ChunkSize;
	}

	read = IFF_rawread ( iff, buff, size);

	if (read==-1)
	{
		read = 0;
	}

	iff->ChunkSize -= read;
	iff->FormSize -= read;
	iff->next_byte += read;


	return read;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

IFF_FILE		*IFF_New ( const char *name )
{
	IFF_FILE *iff = NULL;


	if ( ! (iff = (IFF_FILE *) malloc ( sizeof (IFF_FILE))))
	{
		goto error;
	}

	memset ( iff, 0, sizeof ( IFF_FILE ));
	iff->fp = -1;

	if ((iff->fp = _open ( name, _O_BINARY | _O_RDWR | _O_CREAT | _O_TRUNC, _S_IREAD | _S_IWRITE )) == -1 )
	{
		goto error;
	}

	return iff;

error:

	if (iff)
	{
		IFF_Close ( iff );
	}

	return NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		IFF_NewForm	( IFF_FILE *iff, int id )
{
	IFF_CHUNK chunk;

	chunk.ID = BgEn32 (vIFF_ID_FORM);
	chunk.Size = BgEn32 (90000);

	iff->FormSize = 0;
	iff->ChunkSize = 0;
	iff->FormID = id;

	DO_WRITE ( iff->fp, &chunk, sizeof (IFF_CHUNK), error);

	chunk.ID = BgEn32 ( (int) iff->FormID);
	DO_WRITE ( iff->fp, &chunk.ID, sizeof (int), error );

	iff->flags |= mIFF_FILE_FORMOPEN;
	iff->form_size_pos = iff->next_byte + sizeof (int);
	iff->next_byte += sizeof ( IFF_CHUNK ) + sizeof (int);

	return 	TRUE;

error:

	return	FALSE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			IFF_NewChunk ( IFF_FILE *iff, int id )
{
	IFF_CHUNK chunk;

	chunk.Size = 	BgEn32 (90000);
	chunk.ID =  	BgEn32 ( (int) id);


	DO_WRITE ( iff->fp, &chunk, sizeof ( IFF_CHUNK ), error );

	iff->flags |= mIFF_FILE_CHUNKOPEN;
	iff->chunk_size_pos = iff->next_byte + sizeof (int);
	iff->next_byte += sizeof ( IFF_CHUNK ) ;
	iff->FormSize += sizeof ( IFF_CHUNK ) ;
	iff->ChunkSize = 0;
	iff->ChunkID = id;

	return 	TRUE;

error:

	return	FALSE;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		IFF_Write ( IFF_FILE *iff, void *buff, int size )
{
	int	val =0;

	val = _write ( iff->fp, buff, size);

	if (val==-1)
	{
		val = 0;
	}

	iff->ChunkSize += val;
	iff->FormSize += val;
	iff->next_byte += val;


	return val;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		IFF_CloseForm ( IFF_FILE *iff )
{
	int	fp;
	int	off;
	int	size;
	int	pad = 0;

	if (iff && ((fp = iff->fp) != -1))
	{

		if (iff->FormSize&0x0001)
		{
			DO_WRITE (fp, &pad, 1, error );
			iff->next_byte++;
		}

		off = iff->next_byte - iff->form_size_pos;

		size = BgEn32 ( (int) (iff->FormSize+sizeof ( int )));

		if (lseek (fp, -off, SEEK_CUR)==iff->form_size_pos)
		{
			DO_WRITE ( fp, &size, sizeof (int), error );
			lseek ( fp, 0, SEEK_END);
			return TRUE;
		}
	}

error:

	return FALSE;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		IFF_CloseChunk ( IFF_FILE *iff )
{
	int		fp;
	int		off;
	int		size;
	int		pad = 0;

	if (iff && ((fp = iff->fp) != -1 ))
	{

		if (iff->ChunkSize&0x0001)
		{
			DO_WRITE (fp, &pad, 1, error );
			iff->next_byte++;
			iff->FormSize++;
		}

		off = iff->next_byte - iff->chunk_size_pos;

		size = BgEn32 ((int) iff->ChunkSize);

		if (lseek (fp, -off, SEEK_CUR)==iff->chunk_size_pos)
		{
			DO_WRITE ( fp, &size, sizeof (int), error );
			lseek ( fp, 0, SEEK_END);
			return TRUE;
		}
	}

error:

	return TRUE;
}


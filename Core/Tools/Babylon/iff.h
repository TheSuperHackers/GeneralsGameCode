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


#ifndef __IFF_H
#define __IFF_H

#define	MakeID(a,b,c,d)		( (int) ( ( (int) (a) ) << 24 | ( (int) (b) ) << 16 |	\
								( (int) (c) ) << 8 | ( (int) (d) ) ) )


#define 	vIFF_ID_FORM		MakeID('F','O','R','M')
#define 	vIFF_ID_CAT			MakeID('C','A','T',' ')
#define 	vIFF_ID_LIST		MakeID('L','I','S','T')
#define		vIFF_ID_PROP		MakeID('P','R','O','P')
#define		vIFF_ID_FILLER	W_MakeID(' ',' ',' ',' ')

/* IFF_FILE:
	IFF File handling structure
*/

/* defines for IFF_FILE->flags */
#define	mIFF_FILE_FORMOPEN	(1<<0)		/* in a form */
#define	mIFF_FILE_CHUNKOPEN	(1<<1)		/* in a chunk */
#define	mIFF_FILE_LOADED	(1<<2)		/* file is in memory */


typedef struct
{
	int			ID;			/* Chunk ID */
	int			Size;		/* Chunk size in bytes */

} IFF_CHUNK;

typedef	struct {

	/* public fields */
		int		FormID;
		int		ChunkID;
		int		FormSize;
		int		ChunkSize;

	/* private fields */

		int			fp;
		int			flags;
		int			next_byte;
		int			chunk_size_pos;
		int			form_size_pos;
		int			pad_form;
		int			pad_chunk;
		int			file_size;
		int			file_pos;
		char		*mem_file;


} IFF_FILE;

IFF_FILE*		IFF_Open ( const char * );
IFF_FILE*		IFF_Load ( const char * );
VOID				IFF_Reset ( IFF_FILE * );
VOID				IFF_Close ( IFF_FILE * );

VOID				IFF_goto_form_end ( IFF_FILE *);
VOID				IFF_goto_chunk_end ( IFF_FILE *);

int					IFF_NextForm ( IFF_FILE *);
int					IFF_NextChunk ( IFF_FILE *);

int					IFF_Read ( IFF_FILE *, void *, int );
int		   		IFF_Write ( IFF_FILE *, void *, int );

IFF_FILE*		IFF_New ( const char * );
int					IFF_NewForm	( IFF_FILE *, int );
int					IFF_NewChunk ( IFF_FILE *, int );
int					IFF_CloseForm ( IFF_FILE * );
int					IFF_CloseChunk ( IFF_FILE * );

#define DO_OPEN(P,N,F,E)	{if (!((P)=open(N,(F)))) goto E;}
#define DO_READ(P,B,S,E)	{if ( read((P),(B),(S))!=(S)) goto E;}
#define DO_WRITE(P,B,S,E)	{if ( write((P),(B),(S))!=(S)) goto E;}

#define IFF_READ(iff,data,size,label)		{if ( IFF_Read ( (iff), (data), (size)) != (size)) goto label;}
#define IFF_WRITE(iff,data,size,label)	{if ( IFF_Write ( (iff), (data), (size)) != (size)) goto label;}
#define IFF_NEWCHUNK(iff,id,label)			{ if ( !IFF_NewChunk ( (iff), (id))) goto label; }
#define IFF_NEWFORM(iff,id,label)				{ if ( !IFF_NewForm ( (iff), (id))) goto label; }

#define	Reverse32(L) ( (( (L)>>24 ) & 0xff) | (((L)>>8) &0xff00) | (((L)<<8)&0xff0000) | (((L)<<24)&0xff000000))
#define	Reverse16(L) ( (( (L)>>8 ) & 0xff) | (((L)<<8)&0xff00) )

#define __CPU_BIG_ENDIAN__ 0

#if	__CPU_BIG_ENDIAN__

	#define	BgEn32(L) 	(L)
	#define	BgEn16(L) 	(L)

	#define	LtEn32(L)		Reverse32(L)
	#define	LtEn16(L)		Reverse16(L)

#else

	#define	BgEn32(L) 	Reverse32(L)
	#define	BgEn16(L) 	Reverse16(L)

	#define	LtEn32(L)	(L)
	#define	LtEn16(L)	(L)

#endif


#endif	/* __IFF_H */

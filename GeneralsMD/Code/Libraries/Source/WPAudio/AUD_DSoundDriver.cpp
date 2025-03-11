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

/********************************************************************* Code **
**																			**
**			              Westwood Studios Pacific.			                **
**																		   	**
**						   Confidential Information						   	**
**				   Copyright (C) 1998 - All Rights Reserved				   	**
**																			**
******************************************************************************
**																			**
**	Project:		WAUDIO Library											**
**																			**
**	Module:			Audio (AUD_) 											**
**																			**
**	Version:		$ID$													**
**																			**
**	File name:		wn32/dsound/r2/drv_dsnd.c								**
**																			**
**	Created by:		06/26/96	TR											**
**																			**
**	Description:	Audio device driver for Win'95 Direct Sound				**
**																			**
**																			**
**	audioLoad              :   load system                                   **
**	audioUnload            :   unload system                                 **
**	audioOpen              :   open a unit                                   **
**	audioClose             :   close a unit                                  **
**                                                                          **
**	audioOpenChannel      :   open an audio channel                         **
**	audioCloseChannel     :   close an audio channel                        **
**	audioStart             :   satrt a sample                                **
**	audioStop              :   stop a sample                                 **
**	audioPause             :   pause a sample                                **
**	audioResume            :   resume a sample                               **
**	audioCheck             :   check if a sample is playing                  **
**	audioUpdate            :   update sample attributes                      **
**	audioQueueIt          :   queue up sample to play next                  **
**	AUD_command           :   driver specific command                       **
*****************************************************************************/

#include <wpaudio/altypes.h>						/*	Always include this header first. */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning ( disable: 4706 )
#pragma warning(disable:4201)
#include <mmsystem.h>
//
// Colin: I'm taking the INITGUID stuff out here because we are now linking
// the main RTS.exe to dxguid.lib, having this defined here causes duplicate
// symbols when linking
//
// #define INITGUID  // see comment above
#include <dsound.h>
// #undef INITGUID  // see comment above
#include <windows.h>

#include "asimp3/mss.h"
#include "asimp3/mp3dec.h"

#include <wpaudio/profiler.h>
#include <wpaudio/device.h>
#include <wpaudio/channel.h>
#include <wpaudio/thread.h>
#include <wpaudio/win32.h>


/*****************************************************************************
**							  	  Includes									**
*****************************************************************************/

#include <wpaudio/driver.h>       	/* prototypes for driver code */


/*****************************************************************************
**								   Externals								**
*****************************************************************************/


/*****************************************************************************
**								   Defines									**
*****************************************************************************/

#define		vAUD_DRV_MAX_CHANNELS			100	/* this is arbitrary */
#define		vAUD_DRV_POLL_INTERVAL		(SECONDS(1)/30)	/* this frequency will determine the buffer sizes */
#define		vAUD_DRV_OVER_SAMPLE			8
#define		vAUD_DRV_FRAMES						4                   // this must be at least 2
#define		vAUD_DRV_LAG_FRAMES				0					// follow behind play frame by this many frames
#define		vAUD_DRV_DBHALF				-1000				// -10dB

/*****************************************************************************
**								Private Types								**
*****************************************************************************/

#define STATE_READ_BLOCK							0
#define STATE_WRITE_BLOCK							1
#define STATE_DECODE_BLOCK						2
#define STATE_INIT_BLOCK							3

typedef struct
{
	int						previousValue;
	int						index;

} IMA_DATA;

typedef struct
{
	int						idelta;
	short					iCoef[2];
} MS_DATA;

typedef struct
{
	ASISTREAM *asi;
	int				in_pos;			// start of data in in buffer

} MP3_DATA;

typedef int (CB_DECODE) ( struct AudioDriverChannelTag *);

typedef struct _transfer
{
	int						state;							// current state
	int						next_state;					//
	uint8					*in;								// input buffer
	int						in_bytes;						// bytes to read
	int						in_bytes_left;			// bytes left to read
	int						in_size;						// current size of input buffer
	int						in_size_needed;			// required size of input buffer
	uint8					*out;								// output buffer
	int						out_bytes;					// bytes to write
	int						out_bytes_left;			// bytes left to write
	int						out_size;						// current size of output buffer
	int						out_size_needed;		// required size of output buffer
	int						channels;						// number of channels
	int						pending;						// there is buffered data that still needs servicing

	int						block_size;

	CB_DECODE			*decode;						// code to decode block

	IMA_DATA			ima[2];
	MS_DATA				ms[2];
	MP3_DATA			mp3;

} TRANSFER;

typedef int (CB_TRANSFER) ( struct AudioDriverChannelTag *, uint8 *dst, int *dst_bytes, uint8 *src , int *src_bytes );


typedef struct  AudioDriverChannelTag
{
						AudioChannel				*chan;			/* WCORE channel we belong to */
	volatile	uint								flags;

	/* play */
	volatile	int									pcm_pos;		/* pcm playback position relative to the end of the sample */
	volatile	uint								frame;			/* current frame position of play back */
	volatile	TimeStamp						last_poll;		/* time of last poll */
	volatile	TimeStamp						poll_interval;	/* time between polls */

	/* source data */
	volatile	uint8								*src_start;		/* start of source data */
	volatile	int									src_size;		/* size of source (bytes) */

	/* transfer */
	volatile	uint8								*src_pos;		/* source sample data read pos*/
	volatile	int									src_left;		/* number of bytes of source left */
	volatile	ulong								dst_pos;		/* current write posistion in buffer */
	volatile	ulong								write_pos;		/* current write posistion in buffer according to DS */
	volatile	int									dst_left;		/* bytes left to write befor wrap round */
	volatile	int									silence_left;	/* bytes of silence left to fill */
						int									min_fill;		/* debug */


	/* buffer */
						AudioFormat					format;			/* current buffer format */
						LPDIRECTSOUNDBUFFER dsbuf;			/* Direct sound buffer interface */
						int									buf_size;		/* size of active area in bytes*/
						int									frame_bytes;	/* frame size in bytes */
						
						CB_TRANSFER					*transfer;		/* code to read sample data */
						TRANSFER						transfer_data;

	volatile	int									next_frame_called;
	volatile	int									service_count;
	volatile	int									frames_played;
	volatile	int									original_frame;
	volatile	int									original_dst_pos;


} AUD_DRV_CHAN;



/*****************************************************************************
**								 Private Data								**
*****************************************************************************/


static AUD_Thread			*thread = NULL;
static Lock						drv_ref;

#ifdef _DEBUG
int mixer_skip = 0;
#endif


static 	AudioDriver _AUD_driver =
{
	NULL, /* use for extended data */
	audioOpenChannel,
	audioCloseChannel,
	audioStart,
	audioStop,
	audioPause,
	audioResume,
	audioCheck,
	audioUpdate,
	audioQueueIt,
	audioLock,
	audioUnlock
};

static		AudioSystemMaster  _AUD_system_master =
{
    "Direct Sound Driver V2.0",
    0x0,							/* reserved, do not change ! */
		mAUDIO_SYS_PROP_NONE,
		AUDIO_SYSTEM_MASTER_INIT_STATE,	//  do not change this !
		AUDIO_STAMP_SYSTEM_MASTER,		//  struct id, do not change this !
		audioLoad,
		audioUnload,
    audioOpen,
    audioClose,
		&_AUD_driver
};

/* direct sound */
LPDIRECTSOUND 			AUD_sound_object = NULL;
LPDIRECTSOUNDBUFFER	AUD_primary_buffer = NULL;
// Table was generated with the function 1000*LOG2(n/100) where n is the range 0..100
// 0 = no volume, 100 = full volume, and the range is linear ( 50 is half volume).
// LOG2() is log to the base 2
static	int	AUD_log_table[101] ={   //1....2.....3......4....5......6.....7....8.....9......0
										-10000,-6644,-5644,-5059,-4644,-4322,-4059,-3837,-3644,-3474,
										-3322,-3184,-3059,-2943,-2837,-2737,-2644,-2556,-2474,-2396,
										-2322,-2252,-2184,-2120,-2059,-2000,-1943,-1889,-1837,-1786,
										-1737,-1690,-1644,-1599,-1556,-1515,-1474,-1434,-1396,-1358,
										-1322,-1286,-1252,-1218,-1184,-1152,-1120,-1089,-1059,-1029,
										-1000, -971, -943, -916, -889, -862, -837, -811, -786, -761,
										-737,  -713, -690, -667, -644, -621, -599, -578, -556, -535,
										-515,  -494, -474, -454, -434, -415, -396, -377, -358, -340,
										-322,  -304, -286, -269, -252, -234, -218, -201, -184, -168,
										-152,  -136, -120, -105, -89,  -74,   -59,  -44,  -29,  -14,
										0
									};

/*****************************************************************************
**								 Public Data								**
*****************************************************************************/

AudioSystemMaster		*AudioSystemDSD = &_AUD_system_master;

/*****************************************************************************
**							  Private Prototypes							**
*****************************************************************************/

static		void 		AUD_pcm_service ( AudioChannel *chan );
static		void		AUD_init_buffer ( AUD_DRV_CHAN *ci );
static		int			AUD_create_pcm_buffer ( AUD_DRV_CHAN *ci, AudioFormat *format );
static		void		AUD_destroy_pcm_buffer ( AUD_DRV_CHAN *ci );
static		void		AUD_set_buffer_src ( AUD_DRV_CHAN *ci );
static		void		AUD_new_buffer_src ( AUD_DRV_CHAN *ci );
static		int			AUD_fill_buffer (  AUD_DRV_CHAN *ci, int bytes );
static		void		AUD_update_buffer (  AUD_DRV_CHAN *ci );
static		void		AUD_reset_buffer ( AUD_DRV_CHAN *ci );
static		void		AUD_channel_stop ( AudioChannel *chan );
static		uint		AUD_get_current_frame ( AUD_DRV_CHAN *ci );
static		void		AUD_set_wave_format ( WAVEFORMATEX *pcmwf, AudioFormat *format );
static		int			AUD_same_format ( AudioFormat *format1, AudioFormat *format2 );
static		AUD_ThreadCB		AUD_service_device;
static		int			AUD_service_device ( AudioDevice	*dev );
static		int			AUD_restore_primary ( void );
static		int			AUD_restore_buffer ( AUD_DRV_CHAN *ci );
static		CB_TRANSFER	PCM_transfer;
static		CB_DECODE		IMA_decode_block;
static		CB_DECODE		MS_decode_block;
//static		int			AUD_set_dev_format ( AudioDevice *dev, AudioFormat *format );


/*****************************************************************************
**							   Private Functions							**
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/


static void transfer_init ( TRANSFER *trans )
{
	memset ( trans, 0, sizeof (TRANSFER));
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void transfer_deinit ( TRANSFER *trans )
{
	if ( trans->in )
	{
		free ( trans->in );
		trans->in = NULL;
		trans->in_size = 0;
	}

	if ( trans->out )
	{
		free ( trans->out );
		trans->out = NULL;
		trans->out_size = 0;
	}

	if ( trans->mp3.asi )
	{
		ASI_stream_close ( (HASISTREAM) trans->mp3.asi );
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int	NULL_transfer ( AUD_DRV_CHAN *, uint8 *, int *, uint8 *, int *)
{
	return FALSE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int	PCM_transfer ( AUD_DRV_CHAN *, uint8 *dst, int *dst_bytes, uint8 *src, int *src_bytes )
{
	int transfer;

	if ( (transfer = *dst_bytes ) > *src_bytes )
	{
		transfer = *src_bytes;
	}

	if ( transfer )
	{
		memcpy ( dst, src, transfer );
		*dst_bytes = transfer;
		*src_bytes = transfer;
	}

	return TRUE;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int	ADPCM_transfer ( AUD_DRV_CHAN *ci, uint8 *dst, int *dst_bytes, uint8 *src, int *src_bytes )
{
	int sbytes = 0;
	int dbytes = 0;
	int stransfered = 0;
	int dtransfered = 0;
	int bytes;
	int done = FALSE;
	TRANSFER *data = &ci->transfer_data;

	if ( !data->out || !data->in )
	{
		return FALSE;
	}

	if ( src_bytes )
	{
		sbytes = *src_bytes;
	}

	if ( dst_bytes )
	{
		dbytes = *dst_bytes;
	}


	while ( !done )
	{
		switch ( data->state )
		{
			case STATE_READ_BLOCK:
			{
				if ( !data->in_bytes_left || src == NULL)
				{
					// have read in the src
					data->state = data->next_state;
					data->in_bytes_left = 0;
					break;
				}

				// ok, read in as much as we can

				if ( (bytes = data->in_bytes_left ) > sbytes )
				{
					bytes = sbytes;
				}

				if ( bytes == 0 )
				{
					// no more data to read at this time
					done = TRUE;
					break;
				}

				memcpy ( &data->in[ data->in_bytes - data->in_bytes_left], src, bytes );
				src += bytes;
				stransfered+= bytes;
				sbytes -= bytes;
				data->in_bytes_left -= bytes;
				data->pending = TRUE;
				break;
			}

			case STATE_WRITE_BLOCK:
			{
				if ( !data->out_bytes_left )
				{
					// have written out to dst
					data->state = data->next_state;
					data->pending = FALSE;
					break;
				}

				// ok, write out as much as we can

				if ( (bytes = data->out_bytes_left ) > dbytes )
				{
					bytes = dbytes;
				}

				if ( bytes == 0 )
				{
					// no more data to write at this time
					done = TRUE;
					break;
				}

				memcpy ( dst, &data->out[ data->out_bytes - data->out_bytes_left], bytes );
				dst += bytes;
				dtransfered+= bytes;
				dbytes -= bytes;
				data->out_bytes_left -= bytes;
				break;
			}

			case STATE_DECODE_BLOCK:
			{
				if ( !data->decode ( ci ) )
				{
					return FALSE;
				}

				data->state = STATE_WRITE_BLOCK;
				data->out_bytes_left = data->out_bytes;
				data->next_state = STATE_INIT_BLOCK;
				break;
			}

			case STATE_INIT_BLOCK:
			{
				if ( src != NULL )
				{
					data->in_bytes = data->block_size;
					data->in_bytes_left = data->block_size;
					data->state = STATE_READ_BLOCK;
					data->next_state = STATE_DECODE_BLOCK;
				}
				else
				{
					done = TRUE;
				}
				break;
			}
		}
	}

	if ( src_bytes )
	{
		*src_bytes = stransfered;
	}
	if ( dst_bytes )
	{
		*dst_bytes = dtransfered;
	}
	return TRUE;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static S32 AILCALLBACK fetch_CB ( U32 user, void FAR *dest, S32       bytes, S32 offset)
{
	TRANSFER *data =  (TRANSFER *) user ;

	if ( bytes)
	{
		if ( bytes > data->in_bytes_left )
		{
			bytes = data->in_bytes_left;
		}
		memcpy ( dest, &data->in[data->mp3.in_pos], bytes);
		data->mp3.in_pos += bytes;
		data->in_bytes_left -= bytes;
	}

	return bytes;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int	MP3_transfer ( AUD_DRV_CHAN *ci, uint8 *dst, int *dst_bytes, uint8 *src, int *src_bytes )
{
	int sbytes = 0;
	int dbytes = 0;
	int stransfered = 0;
	int dtransfered = 0;
	int bytes;
	//int done = FALSE;
	TRANSFER *data = &ci->transfer_data;

	if ( !data->in )
	{
		return FALSE;
	}

	if ( src_bytes )
	{
		sbytes = *src_bytes;
	}

	if ( dst_bytes )
	{
		dbytes = *dst_bytes;
		*dst_bytes = 0;
	}

	// make sure existing in data is at the start of the buffer

	if ( data->in_bytes_left && data->mp3.in_pos > 0 )
	{
		//copy the data down to the start of the buffer
		memcpy ( data->in, &data->in[data->mp3.in_pos], data->in_bytes_left );
	}

	data->mp3.in_pos = 0;

	// make sure the in buffer is full

	if ( data->in_bytes_left < data->in_size_needed && src )
	{
		// ok, read in as much as we can

		if ( (bytes = data->in_size_needed - data->in_bytes_left ) > sbytes )
		{
			bytes = sbytes;
		}

		if ( bytes == 0 )
		{
			// no more data to read at this time
			return TRUE;
		}

		memcpy ( &data->in[ data->in_bytes_left], src, bytes );
		src += bytes;
		stransfered+= bytes;
		sbytes -= bytes;
		data->in_bytes_left += bytes;
		data->pending = TRUE;

		if ( data->in_bytes_left < data->in_size_needed )
		{
			return TRUE; // not enough data to continue
		}
	}

	if ( !data->mp3.asi )
	{
		// start MP3 streamer
			if ( ! ( data->mp3.asi = (ASISTREAM *) ASI_stream_open ( (U32) data, fetch_CB, 0 )))
			{
				// bad MP3 stream
				return FALSE;
			}
	}

	// decode more of the stream
	if ( dbytes > 16*1024)
	{
		dbytes = 16*1024;
	}
	dtransfered = ASI_stream_process ( (HASISTREAM) data->mp3.asi, dst, dbytes );

	data->pending = data->in_bytes_left || (data->mp3.asi->output_cursor < data->mp3.asi->frame_size);

	if ( src_bytes )
	{
		*src_bytes = stransfered;
	}
	if ( dst_bytes )
	{
		*dst_bytes = dtransfered;
	}
	return dtransfered>0;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		int	AUD_service_device ( AUD_Thread *thread, void *data )
{
	AudioChannel	*chan;
	AudioSearch	sh;
	AudioDevice	*dev = (AudioDevice*) data;

	if ( !dev )
	{
		return TRUE;
	}

	#ifdef _DEBUG
		if ( mixer_skip )
		{
			mixer_skip--;
			if ( mixer_skip < 0 )
			{
				mixer_skip = 0;
			}

			return TRUE;
		}
	#endif


	if ( NotLocked ( &dev->channelAccess) )
	{
		TimeStamp now = AudioGetTime () ;
		AudioServicePerform ( &dev->mixerUpdate, now );
		chan = AudioDeviceFirstChannel ( dev, &sh );

		while ( chan )
		{
			if ( chan->Control.Status & mAUDIO_CTRL_PLAYING )
			{
				if ( audioCheck ( chan ))
				{
					chan->drvData->poll_interval = now - chan->drvData->last_poll;
					chan->drvData->last_poll = now;
					AUD_update_buffer (  chan->drvData );
				}
				else
				{
					AUD_channel_stop ( chan );
				}
			}
			chan = AudioDeviceNextChannel ( &sh );
		}

		AudioDeviceService ( dev );

		return TRUE;
	}

	return FALSE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		int			AUD_same_format ( AudioFormat *f1, AudioFormat *f2 )
{
	return (			f1->Channels == f2->Channels
						&&	f1->Rate == f2->Rate
						&&	f1->SampleWidth == f2->SampleWidth );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/


void		AUD_set_wave_format ( WAVEFORMATEX *pcmwf, AudioFormat *format )
{
    memset ( pcmwf, 0, sizeof(WAVEFORMATEX) );
    pcmwf->wFormatTag = WAVE_FORMAT_PCM;
    pcmwf->nChannels = (unsigned short ) format->Channels;
    pcmwf->nSamplesPerSec = format->Rate;
    pcmwf->nBlockAlign = (unsigned short ) format->Channels*format->SampleWidth;
    pcmwf->nAvgBytesPerSec = pcmwf->nSamplesPerSec * pcmwf->nBlockAlign;
    pcmwf->wBitsPerSample = (unsigned short ) format->SampleWidth<<3;
		pcmwf->cbSize = 0;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		int	AUD_create_pcm_buffer ( AUD_DRV_CHAN *ci,  AudioFormat *format )
{
	DSBUFFERDESC	desc;
  WAVEFORMATEX 	pcmwf;

	DBG_Function ("DSND2:AUD_create_pcm_buffer");


	AudioChannelSetFormat ( ci->chan, format );

	if ( ci->dsbuf )
	{
		if ( AUD_same_format ( &ci->format, format ) )
		{
			ci->chan->drv_format_changed = FALSE;
			/* we already have a buffer of the required format */
			return NO_ERROR;
		}
		ci->chan->drv_format_changed = TRUE;
	}


	/* we need to recreate the buffer */
	AUD_destroy_pcm_buffer ( ci );

    // Set up wave format structure.

	AUD_set_wave_format ( &pcmwf, format );
	ci->format = *format;

  // Set up DSBUFFERDESC structure.
  memset ( &desc, 0, sizeof(DSBUFFERDESC) );
  desc.dwSize = sizeof(DSBUFFERDESC);
  desc.dwFlags = (DSBCAPS_CTRLVOLUME |DSBCAPS_CTRLPAN|DSBCAPS_CTRLFREQUENCY) ; 	// Need default controls vol, pan, pitch

	/* now we work out the size that the buffer needs to be. This is based on the polling interval and
	the number of frames.

	A frame size is the that size that when played back will take X polling intervals ( over sampling). This ensures that
	the polling code will not miss any frames. A frame is also the minimum update size.

		frame size = ((bytes per second) * (poll interval per second) * over sample
	*/

	ci->frame_bytes = ((pcmwf.nAvgBytesPerSec * (vAUD_DRV_POLL_INTERVAL/10) )/(TIMER_HZ/10)) * vAUD_DRV_OVER_SAMPLE;
	ci->frame_bytes = ((ci->frame_bytes+1024-1)/1024) * 1024;

//	DBGPRINTF (("Sound frame size = %d bytes\n", ci->frame_bytes ));

  desc.dwBufferBytes = vAUD_DRV_FRAMES * ci->frame_bytes;

  desc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;

  // Create buffer.

  if ( AUD_sound_object->CreateSoundBuffer ( &desc, &ci->dsbuf, NULL) != DS_OK)
	{
		ci->dsbuf = NULL ;
		return ERROR_CODE_FAIL;
	}

#ifdef _DEBUG

	DSBCAPS caps = {0};

	caps.dwSize = sizeof(caps);

	if ( ci->dsbuf->GetCaps ( &caps ) != DS_OK )
	{
		msg_assert ( FALSE, ("Error trying to get caps of secondary buffer"));
	}

	msg_assert ( caps.dwBufferBytes == desc.dwBufferBytes , ("Error: Asked for buffer of %d bytes, but got %d bytes", desc.dwBufferBytes, caps.dwBufferBytes ));

#endif


	ci->buf_size = desc.dwBufferBytes;
//	DBGPRINTF (("Sound buffer size = %d bytes\n", ci->buf_size ));

	return vNO_ERROR;

}



/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void	AUD_destroy_pcm_buffer ( AUD_DRV_CHAN *ci )
{
	if ( ci->dsbuf )
	{
		ci->dsbuf->Release ();
		ci->dsbuf = NULL;
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		int		AUD_restore_primary ( void )
{
	if ( AUD_primary_buffer->Restore ( ) == DS_OK )
	{
		return TRUE;
	}

	return FALSE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		int			AUD_restore_buffer ( AUD_DRV_CHAN *ci )
{
	if ( ci->dsbuf->Restore () == DS_OK )
	{
		AUD_init_buffer ( ci );
		return TRUE;
	}

	return FALSE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		uint	AUD_get_current_frame ( AUD_DRV_CHAN *ci )
{
	ulong	pos;

	ci->dsbuf->GetCurrentPosition ( &pos, (ulong*) &ci->write_pos );

	return pos / ci->frame_bytes;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void	AUD_init_buffer ( AUD_DRV_CHAN *ci )
{

	ci->src_left = 0;
	ci->pcm_pos = 0;
	ci->silence_left = ci->frame_bytes*4;
	ci->dsbuf->SetCurrentPosition ( 0 );
	ci->frame = AUD_get_current_frame ( ci );
	ci->last_poll = AudioGetTime ( );
	ci->min_fill = ci->buf_size;

	ci->dsbuf->GetCurrentPosition ( (ulong*) &ci->dst_pos, (ulong*) &ci->write_pos );
	ci->dst_left = ci->buf_size - ci->dst_pos;
	ci->service_count = 0;
	ci->original_frame = ci->frame;
	ci->frames_played = 0;
	ci->original_dst_pos = ci->dst_pos;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void	AUD_reset_buffer ( AUD_DRV_CHAN *ci )
{
	ci->dst_pos = 0;
	ci->dst_left = ci->buf_size;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void	AUD_set_buffer_src ( AUD_DRV_CHAN *ci )
{
	ci->next_frame_called = FALSE;
	ci->src_pos = (uint8 *) ci->chan->frameData;
	ci->src_start = (uint8 *) ci->chan->frameData;
	ci->src_left = ci->src_size = ci->chan->bytesInFrame;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void	AUD_new_buffer_src ( AUD_DRV_CHAN *ci )
{
	AudioSample *sample = ci->chan->sample;
	AudioFormat *format;
	TRANSFER		*data = &ci->transfer_data;

	if ( !sample )
	{
		return;
	}


	if ( !( format = ci->chan->sample->Format ) )
	{
		format = &ci->chan->Device->DefaultFormat;
	}
#ifdef _DEBUG
	msg_assert ( AUD_same_format ( &ci->format, format ), ("Sample \"%s\" has incompatible format with the previous sample.\nCannot seamlessly merge samples of differing playback parameters!!", ci->chan->sample_name));
#endif	//_DEBUG

	data->out_bytes_left = 0;
	data->in_bytes_left = 0;
	data->in_bytes = 0;
	data->state = STATE_INIT_BLOCK;
	data->in_size_needed = 0;
	data->out_size_needed = 0;
	data->channels = format->Channels;
	data->pending = FALSE;

	if ( data->mp3.asi )
	{
		ASI_stream_close ( (HASISTREAM) data->mp3.asi );
	}
	data->mp3.asi = NULL;

	switch ( format->Compression )
	{

		case AUDIO_COMPRESS_NONE :
			ci->transfer = PCM_transfer;
			break;

		case AUDIO_COMPRESS_IMA_ADPCM:

			ci->transfer = ADPCM_transfer;
			data->decode = IMA_decode_block;
			data->block_size = format->cdata.adpcm.BlockSize;
			data->in_size_needed = data->block_size;
			data->out_size_needed = (data->block_size+32)*4;
			break;

		case AUDIO_COMPRESS_MS_ADPCM:
			ci->transfer = ADPCM_transfer;
			data->decode = MS_decode_block;
			data->block_size = format->cdata.adpcm.BlockSize;
			data->in_size_needed = data->block_size;
			data->out_size_needed = (data->block_size+32)*4;
			break;

		case AUDIO_COMPRESS_MP3:
			ci->transfer = MP3_transfer;
			data->in_size_needed = STREAM_BUFSIZE + 1024;
			data->mp3.in_pos = 0;
			data->in_bytes = data->in_size_needed;
			break;

#ifdef _DEBUG
		default:
			ci->transfer = NULL_transfer;
			msg_assert ( FALSE , ("Unknown compression format"));
			break;
#endif
	}

	if ( data->in_size < data->in_size_needed )
	{
		if ( data->in )
		{
			free ( data->in );
		}

		data->in = (uint8*) malloc ( data->in_size_needed );
		msg_assert ( data->in, ("Failed to create new in buffer"));
		if ( !data->in )
		{
			data->in_size = 0;
		}
		else
		{
			data->in_size = data->in_size_needed;
		}
	}

	if ( data->out_size < data->out_size_needed )
	{
		if ( data->out )
		{
			free ( data->out );
		}

		data->out = (uint8*) malloc ( data->out_size_needed );
		msg_assert ( data->out, ("Failed to create new out buffer"));

		if ( !data->out )
		{
			data->out_size = 0;
		}
		else
		{
			data->out_size = data->out_size_needed;
		}
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int AUD_lock_buffer ( AUD_DRV_CHAN *ci, ulong pos, ulong transfer, uint8 **dst, ulong *bytes1, uint8 **dst2, ulong *bytes2 )
{
	int retval;
	int retry_count = 3;

retry:

	if ( (retval = ci->dsbuf->Lock ( pos, transfer, (void **) dst, bytes1, (void **)dst2, bytes2, 0)) != DS_OK )
	{
		if ( retval = DSERR_BUFFERLOST )
		{
			if ( !AUD_restore_buffer ( ci ) )
			{
				DBGPRINTF (("Failed to restore buffer\n"));
				return FALSE;
			}
			DBGPRINTF (("Restored buffer\n"));
			if ( retry_count-- )
			{
				goto retry;
			}
		}
		DBGPRINTF (("Failed to lock buffer\n"));
		return FALSE;
	}

	return TRUE;
}

static		int	AUD_fill_buffer ( AUD_DRV_CHAN *ci,  int total_bytes )
{
	ulong	transfer, bytes_transfer;
	ulong		bytes;
	uint8	*lpos1 = NULL,*lpos2=NULL;
	ulong	lbytes1 = 0,lbytes2 = 0;
	uint8	*dst;
	int		locked =FALSE;
	int		result = FALSE;

	DBG_Function ("DSND2:AUD_fill_buffer");



	while ( total_bytes > 0  )
	{
		/* check for buffer wrap roun */
		if ( ci->dst_left <= 0)
		{
			AUD_reset_buffer ( ci );
		}

		bytes_transfer = 0;

		/* check for buffer overflow and adjust transfer accordingly */
		if ( (bytes = (ulong) total_bytes) > (ulong ) ci->dst_left )
		{
			bytes = ci->dst_left;
		}

		if ( locked )
		{
			ci->dsbuf->Unlock ( lpos1, lbytes1, lpos2, lbytes2 );
			locked = FALSE;
		}

		if ( !AUD_lock_buffer ( ci, ci->dst_pos, bytes, &lpos1, &lbytes1, &lpos2, &lbytes2) )
		{
				goto done;
		}

		locked = TRUE;

		if ( (lpos2 != NULL) || (lbytes1 != bytes) || (lbytes2 != 0))
		{
			DBGPRINTF (("Buffer management out by %d bytes\n", lbytes2));
			goto done;
		}

		dst = lpos1;


		while ( bytes )
		{

			transfer = bytes;

			/* is there any source data left */
			if ( ci->src_left == 0 )
			{
				/* call up to higher level to see if more source is going to be supplied */
				ci->src_left = -1;	/* unless the call back adds new source we assume no more data */

				if ( !ci->next_frame_called )
				{
					ci->chan->drvCBNextFrame ( ci->chan );
					ci->next_frame_called = TRUE;
				}

				if ( !ci->chan->bytesInFrame  )
				{
					// no more data for this sample
					if ( !ci->transfer_data.pending)
					{
						ci->chan->drvCBNextSample ( ci->chan );

						if ( ci->chan->bytesInFrame )
						{
							// we have a new sample
							AUD_new_buffer_src ( ci );
						}
					}

				}

				if ( ci->chan->bytesInFrame )
				{
					AUD_set_buffer_src ( ci );
				}

				if ( ci->src_left == 0 )
				{
					ci->src_left = -1;
				}
			}

			if ( ci->src_left <= 0  && !ci->transfer_data.pending )
			{
				if ( ci->chan->current_format.SampleWidth == 2)
				{
					memset ( dst, 0x0000, transfer );
				}
				else
				{
					memset ( dst, 0x80, transfer );
				}
				ci->silence_left -= transfer ;
			}
			else
			{
				int src_bytes;

				if ( (src_bytes = ci->src_left) <= 0 )
				{
					// flush buffered transfer
					if ( !ci->transfer ( ci, dst, (int*) &transfer, NULL, NULL ) )
					{
						goto done;
					}
					src_bytes = 0;
					ci->src_left = 0;	// try to read more source
				}
				else
				{
					if ( !ci->transfer ( ci, dst, (int*) &transfer, (uint8 *)ci->src_pos, &src_bytes ) )
					{
						goto done;
					}
				}
				ci->src_pos+= src_bytes;
				ci->src_left -= src_bytes;
				ci->pcm_pos += transfer;
			}

			dst += transfer;
			ci->dst_pos += transfer;
			ci->dst_left -= transfer;
			bytes -= transfer;
			bytes_transfer += transfer;
		}
		total_bytes -= bytes_transfer;
	}

	result = TRUE;

done:
	if ( locked )
	{
		ci->dsbuf->Unlock ( lpos1, lbytes1, lpos2, lbytes2 );
		locked = FALSE;
	}



	return result;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void	AUD_update_buffer ( AUD_DRV_CHAN *ci )
{
	uint	frame;
	int	dif;
	int filled;

	frame = AUD_get_current_frame ( ci );

	if ( ( dif = (frame - ci->frame )))
	{
		/* pcm address has moved on */
		if ( dif < 0 )
		{
			/* pcm has wraped round to start of buffer */
			dif = frame + (vAUD_DRV_FRAMES - ci->frame);
		}

		if ( (dif -= vAUD_DRV_LAG_FRAMES ) > 0 )
		{
			if ( (ci->frame += dif) >= vAUD_DRV_FRAMES)
			{
				ci->frame %= vAUD_DRV_FRAMES;
			}

			ci->frames_played += dif;

			dif = dif * ci->frame_bytes;
			filled = AUD_fill_buffer ( ci, dif );

			if ( ci->min_fill > dif )
			{
				ci->min_fill = dif;
			}

			if ( (ci->pcm_pos -= dif) <=0 || !filled )
			{
				ci->dsbuf->Stop();
			}
		}
	}
		ci->service_count++;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void	AUD_channel_stop ( AudioChannel *chan )
{
	AUD_DRV_CHAN	*ci;

	ci = (AUD_DRV_CHAN *) chan->drvData;
	/* put stop call here */

	if ( ci->dsbuf )
	{
		ci->dsbuf->Stop();
	}

	if ( chan->drvCBSampleDone )
	{
		chan->drvCBSampleDone ( chan );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int      audioLoad ( AudioSystem *system )
{
	int result;

	ASI_startup();
	result = DirectSoundCreate(NULL, &AUD_sound_object, NULL);

	if (result != DS_OK)
	{
		//Print_Sound_Error(Fetch_String(TXT_DSOUND_NO_COOP) );
		return ERROR_CODE_FAIL;
	}

	assert ( AUD_sound_object );

	result = AUD_sound_object->SetCooperativeLevel( AudioGetWindowsHandle () , DSSCL_PRIORITY);
	system->numUnits = 1;	/* tell WCORE that there is only 1 audio unit */


	if ( !thread )
	{
		LockInit ( &drv_ref );
		if (!(thread = AUD_ThreadCreate ( "Audio Service Thread", AUD_THREAD_PRI_REALTIME, AUD_service_device )) )
		{
			return ERROR_CODE_FAIL;
		}
		LockAcquire ( &drv_ref );
		AUD_ThreadSetInterval ( thread, vAUD_DRV_POLL_INTERVAL );
	}
	else
	{
		LockAcquire ( &drv_ref );
	}


	return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  void      audioUnload ( AudioSystem * )
{
    /* put in code to uninitialize audio system */

	if ( AUD_sound_object )
	{
		AUD_sound_object->Release();
		AUD_sound_object = NULL ;
	}

	if ( thread )
	{
		LockRelease ( &drv_ref );

		if ( NotLocked ( &drv_ref ))
		{
			AUD_ThreadDestroy ( thread );
			thread = NULL;
		}
	}

	ASI_shutdown ();
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int   audioOpen ( AudioDevice *dev )
{

    /* put in code to instance a device */

	dev->maxChannels = vAUD_DRV_MAX_CHANNELS;
	AudioServiceSetInterval ( &dev->mixerUpdate, vAUD_DRV_POLL_INTERVAL );
	AudioServiceSetMustServiceInterval ( &dev->mixerUpdate, vAUD_DRV_POLL_INTERVAL * vAUD_DRV_OVER_SAMPLE * vAUD_DRV_FRAMES  );
	AudioServiceSetResetInterval ( &dev->mixerUpdate, vAUD_DRV_POLL_INTERVAL * vAUD_DRV_OVER_SAMPLE * vAUD_DRV_FRAMES * 3  );

	AUD_ThreadSetData ( thread, dev);

	/* start the primary buffer playing */
	{
		DSBUFFERDESC	dsbdesc;
   	WAVEFORMATEX 	pcmwf;

		AUD_set_wave_format ( &pcmwf, &dev->DefaultFormat );

		memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out.
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    dsbdesc.dwBufferBytes = 0;

		if ( AUD_sound_object->CreateSoundBuffer ( &dsbdesc, &AUD_primary_buffer, NULL) != DS_OK )
		{
			AUD_primary_buffer = NULL;
			return ERROR_CODE_FAIL;
		}

		if ( AUD_primary_buffer->SetFormat ( &pcmwf ) != DS_OK )
		{
			DBGPRINTF (("Unable to set the desired primary format\n"));
		}

		DSCAPS caps = {0};
		caps.dwSize = sizeof( caps );

		AUD_sound_object->GetCaps ( &caps );
		if (caps.dwFlags & DSCAPS_EMULDRIVER)
		{
			//msg_assert ( FALSE, ("Audio Emulation"));

		}


		AUD_primary_buffer->Play ( 0, 0, DSBPLAY_LOOPING );
	}


		dev->frames = vAUD_DRV_FRAMES;
		dev->over_sample = vAUD_DRV_OVER_SAMPLE;
		dev->frame_lag = vAUD_DRV_LAG_FRAMES;

    return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  void        audioClose ( AudioDevice * )
{
    /* put in code to close up a device */

	AUD_ThreadSetData ( thread, NULL );

	if ( AUD_primary_buffer )
	{
		AUD_primary_buffer->Stop ();
		AUD_primary_buffer->Release ();
		AUD_primary_buffer = NULL;
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int		audioOpenChannel ( AudioChannel *chan )
{
	AUD_DRV_CHAN	*ci;

	DBG_Function ("DSND2:audioOpenChannel");
    /* put in code to instance a channel */

	MEM_ALLOC_STRUCT ( ci, AUD_DRV_CHAN, vMEM_ANY|mMEM_INTERNAL );

	chan->drvData = ci;

	ci->chan = chan;

	chan->time_min_frame = vAUD_DRV_POLL_INTERVAL * vAUD_DRV_OVER_SAMPLE;
	chan->time_buffer = vAUD_DRV_FRAMES * chan->time_min_frame;

	transfer_init ( &ci->transfer_data );

	AudioChannelSetFormat ( ci->chan, &ci->chan->Device->DefaultFormat );	/* initially create sound buffer with default format */
	return vNO_ERROR;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int		audioCloseChannel ( AudioChannel *chan )
{
	AUD_DRV_CHAN	*ci;

	DBG_Function ("DSND2:audioCloseChannel");
    /* put in code to close a channel*/
	if ( (ci = chan->drvData) )
	{
		transfer_deinit ( &ci->transfer_data );
		AUD_destroy_pcm_buffer ( ci );
		MEM_Free ( ci );
		chan->drvData = NULL;
	}

	return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int		audioStart ( AudioChannel *chan )
{
	AUD_DRV_CHAN	*ci;
	AudioFormat		*format;
	AudioSample		*sample;

	DBG_Function ("DSND2:audioStart");

	DBG_ASSERT ( !audioCheck ( chan ));
	ci = chan->drvData;
	sample = chan->sample;

	if ( !( format = sample->Format ) )
	{
		format = &chan->Device->DefaultFormat;
	}

	/* recreate buffer if format changed */
	if ( AUD_create_pcm_buffer ( ci, format ) != vNO_ERROR)
	{
		DBGPRINTF (("Failed to create playback buffer\n"));
		return ERROR_CODE_FAIL;
	}

	AUD_init_buffer  ( ci );					/* initialise buffer for play back */
	AUD_new_buffer_src ( ci );
	AUD_set_buffer_src ( ci  );		/* attach source to buffer */


	if ( !AUD_fill_buffer ( ci, ci->buf_size ))	/* pre-fill buffer to max */
	{
		DBGPRINTF (( "Failed to prefill buffer\n"));
		return ERROR_CODE_FAIL;
	}

	/* update buffer attributes */
	audioUpdate ( chan );

	/* start playing */

	if ( ci->dsbuf->Play (  0, 0, DSBPLAY_LOOPING ) != DS_OK)		/* must do looping or things will go horribly wrong */
	{
		DBGPRINTF (("Failed to start playback\n"));
		return ERROR_CODE_FAIL;
	}

	DBG_ASSERT ( audioCheck ( chan ));

	return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int		audioStop ( AudioChannel *chan )
{

	if ( chan->Control.Status & (mAUDIO_CTRL_ACTIVE) )
	{
		AUD_channel_stop ( chan );
	}
	return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int		audioPause ( AudioChannel *chan )
{
	AUD_DRV_CHAN	*ci = chan->drvData;

	if ( ci->dsbuf )
	{
		ci->dsbuf->Stop ();
	}


	return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int		audioResume ( AudioChannel *chan )
{
	AUD_DRV_CHAN	*ci = chan->drvData;

	if ( ci->dsbuf )
	{
		if ( ci->dsbuf->Play ( 0, 0, DSBPLAY_LOOPING ) == DS_OK)		/* must do looping or things will go horribly wrong */
		{
			return vNO_ERROR;
		}
	}
	return ERROR_CODE_FAIL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int		audioCheck ( AudioChannel *chan )
{
	AUD_DRV_CHAN	*ci = chan->drvData;
	DWORD status;

	if ( ci->dsbuf && (ci->dsbuf->GetStatus ( &status ) == DS_OK) )
	{
		if ( status & (DSBSTATUS_PLAYING|DSBSTATUS_LOOPING) == (DSBSTATUS_PLAYING|DSBSTATUS_LOOPING) )
		{
			return TRUE;
		}
		else
		{
				return FALSE;
		}	
	}

	return FALSE;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int		audioUpdate ( AudioChannel *chan )
{
	AUD_DRV_CHAN	*ci;
	int			vol;
	int			pitch;
	HRESULT			stat;

	ci = chan->drvData;


	vol = AudioLevelApply ( &chan->attribs.VolumeLevel, 100 );

	ci->dsbuf->SetVolume ( AUD_log_table[vol]);
#if 1
	{
		int	pos;

		pos = AudioLevelApply ( &chan->attribs.PanPosition, 200 ) - 100;

		if ( pos < 0 )
		{
			stat = ci->dsbuf->SetPan (  AUD_log_table[100+pos]);
		}
		else
		{
			stat = ci->dsbuf->SetPan ( -AUD_log_table[100-pos]);
		}

	}
#endif

	pitch = AudioAttribsCalcPitch ( &chan->attribs, ci->chan->current_format.Rate );
	ci->dsbuf->SetFrequency ( pitch);

	return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int		audioQueueIt ( AudioChannel *)
{
	return ERROR_CODE_FAIL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int		audioLock ( AudioChannel *)
{
	AUD_ThreadBeginCriticalSection ( thread );
	return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static  int		audioUnlock ( AudioChannel *)
{
	AUD_ThreadEndCriticalSection ( thread );
	return vNO_ERROR;
}

/* command functions */

#if 0
static		int		AUD_set_dev_format ( AudioDevice *, AudioFormat *format )
{
  WAVEFORMATEX 		wf;
	int		err = ERROR_CODE_FAIL;

		if ( AUD_primary_buffer )
		{
    	// Set up wave format structure.
			AUD_primary_buffer->Stop ();
			AUD_set_wave_format ( &wf, format );

			if ( AUD_primary_buffer->SetFormat ( &wf ) == DS_OK )
			{
				err = vNO_ERROR;
			}

			AUD_primary_buffer->Play ( 0, 0, DSBPLAY_LOOPING );
		}

		return err;

}
#endif

/*****************************************************************************
**							   Public Functions								**
*****************************************************************************/

/*						         :NOTICE:
	There are no public functions in a driver. If you have any then you are doing
	something wrong. -Tr
*/

void *GetDirectSoundObject ( void )
{
	return (void *) AUD_sound_object;
}

void *GetPrimaryBuffer ( void )
{
	return (void *) AUD_primary_buffer ;
}

void AudioLoseFocus ( void )
{
	if ( AUD_primary_buffer )
	{
		AUD_primary_buffer->Stop ();
	}
}

void AudioRegainFocus ( void )
{
	if ( AUD_primary_buffer )
	{
		DWORD status;

		AUD_primary_buffer->GetStatus ( &status );

		if ( (status & DSBSTATUS_BUFFERLOST) )
		{
			AUD_restore_primary ( );
		}

		if ( !(status & DSBSTATUS_PLAYING) )
		{
			AUD_primary_buffer->Play ( 0, 0, DSBPLAY_LOOPING );
		}
	}

}

		/* debug function. Not part of API */
#ifdef _DEBUG

void	AudioCauseMixerDelay ( void )
{

	mixer_skip = (vAUD_DRV_OVER_SAMPLE * vAUD_DRV_FRAMES -1) * 2;

}


#include <stdio.h>

void		AudioDeviceDumpDXSNDDriver ( AudioDevice *dev, void (*print) ( const char *) )
{
	char	string[200];
	AudioChannel	*chan;
	AudioSearch		sh;

	DBG_Function ("AudioDeviceDumpChannels");
	DBG_ASSERT_TYPE ( dev, AudioDevice );



	LockAcquire ( &dev->channelAccess);

	sprintf ( string, " \nDXSND Channels\n");
	print ( string );

	chan = AudioDeviceFirstChannel ( dev, &sh );

	while (chan)
	{

		sprintf ( string, "Channel 0x%0x, F=%6d, P=%d, I=%04d, T=%6d, B=%6d\n",
						chan,
						chan->drvData->frame_bytes,
						chan->drvData->frame,
						chan->drvData->poll_interval,
						chan->drvData->min_fill,
						chan->drvData->buf_size
				);
		print ( string );

		chan = AudioDeviceNextChannel( &sh );
	}

	LockRelease ( &dev->channelAccess);

}
#endif


// IMA ADPCM Support

/*
 *
 * Lookup tables for IMA ADPCM format
 *
 */
static int imaIndexAdjustTable[16] = {
   -1, -1, -1, -1,  /* +0 - +3, decrease the step size */
    2, 4, 6, 8,     /* +4 - +7, increase the step size */
   -1, -1, -1, -1,  /* -0 - -3, decrease the step size */
    2, 4, 6, 8,     /* -4 - -7, increase the step size */
};

static int imaStepSizeTable[89] = {
   7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34,
   37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143,
   157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494,
   544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552,
   1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026,
   4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442,
   11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623,
   27086, 29794, 32767
};

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

/****************************************************************************/
/* IMA ADPCM Support Functions Section                                      */
/****************************************************************************/

/*
 *
 * MsAdpcmDecode - Decode a given sample and update state tables
 *
 */

static short ImaAdpcmDecode( uint8 deltaCode, IMA_DATA *state )
{
    /* Get the current step size */
   int step;
   int difference;

   step = imaStepSizeTable[state->index];

   /* Construct the difference by scaling the current step size */
   /* This is approximately: difference = (deltaCode+.5)*step/4 */
   difference = step>>3;
   if ( deltaCode & 1 ) difference += step>>2;
   if ( deltaCode & 2 ) difference += step>>1;
   if ( deltaCode & 4 ) difference += step;

   if ( deltaCode & 8 ) difference = -difference;

   /* Build the new sample */
   state->previousValue += difference;

   if (state->previousValue > 32767) state->previousValue = 32767;
   else if (state->previousValue < -32768) state->previousValue = -32768;

   /* Update the step for the next sample */
   state->index += imaIndexAdjustTable[deltaCode];
   if (state->index < 0) state->index = 0;
   else if (state->index > 88) state->index = 88;

   return (short) state->previousValue;

}

static int IMA_decode_block (  AUD_DRV_CHAN *ci )
{
	int i,j;
	TRANSFER *data = &ci->transfer_data;
	int bytes = data->in_bytes - data->in_bytes_left;
	uint8 *in = data->in;

	// set up output
	data->out_bytes = 0;
	ushort *out = (ushort *) data->out;

	if ( bytes < data->channels*4 )
	{
		return FALSE;
	}

	for ( i=0; i < data->channels; i++)
	{
		data->ima[i].previousValue = (int) (in[1] <<8) + (int) in[0];

		if (data->ima[i].previousValue & 0x8000)
	    data->ima[i].previousValue -= 0x10000;

		if ((data->ima[i].index = in[2]) > 88)
		{
		    msg_assert(FALSE, ("IMA ADPCM Format Error (bad index value) in wav file %s",ci->chan->sample_name));
				return FALSE;
		}

		if (in[3])
		{
			msg_assert (FALSE, ("IMA ADPCM Format Error (synchronization error) in wav file %s ", ci->chan->sample_name));
			return FALSE;
		}
		in+=4;
		bytes -=4;

		*out++ = (unsigned short ) data->ima[i].previousValue;
		data->out_bytes += 2;
	}


	if ( data->channels == 1 )
	{
		// 99.99% of the samples being played are mono

		while ( bytes > 0 )
		{
				ushort *out = (ushort *) &data->out[data->out_bytes];
				j = 4;

				while ( j-- )
				{
					uint8 b = *in++;
					*out++ = (ushort ) ImaAdpcmDecode(b & 0x0f,&data->ima[0]);
					*out++ = (ushort ) ImaAdpcmDecode((b>>4) & 0x0f,&data->ima[0]);
				}

			data->out_bytes += 16;
			bytes -= 4;
		}

	}
	else
	{
		while ( bytes > 0 )
		{
			for ( i = 0; i < data->channels ; i++)
			{
				short *out = (short *) &data->out[data->out_bytes+(i<<1)];
				for (j=0;j<4;j++)
				{
					uint8 b = *in++;
					*out = ImaAdpcmDecode(b & 0x0f,&data->ima[i]);
					out += data->channels;
					*out = ImaAdpcmDecode((b>>4) & 0x0f,&data->ima[i]);
					out += data->channels;
				}
			}

			data->out_bytes += data->channels<<4;
			bytes -= data->channels<<2;
		}
	}

	if ( bytes != 0 )
	{
		msg_assert ( FALSE, ("bad block"));
		return FALSE;
	}

	return TRUE;
}

/****************************************************************************/
/* Microsoft ADPCM Support Functions Section                                      */
/****************************************************************************/


static const int adaptionTable[] = {
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230
};

static short MSAdpcmDecode( unsigned char nibble, MS_DATA *ms, int sample1, int sample2)
{
	int newsample;
	int predsample;

	predsample = ((sample1 * ms->iCoef[0]) + ( sample2 * ms->iCoef[1]))>>8;
	newsample = predsample + (ms->idelta * (nibble - ((nibble&0x08) << 1)));

	ms->idelta = (ms->idelta * adaptionTable[nibble])>>8;

	if ( ms->idelta < 16 )
	{
		ms->idelta = 16;
	}

	if (newsample > 0x7fff) 
	{
		newsample = 0x7fff;
	}
	else if (newsample < -0x8000)
	{
		newsample = -0x8000;
	}

	return (short) newsample;
}


static int MS_decode_block (  AUD_DRV_CHAN *ci )
{
	int i;
	TRANSFER *data = &ci->transfer_data;
	int bytes = data->in_bytes - data->in_bytes_left;
	uint8 *in = data->in;
	unsigned char bpredictor;
	int hsize = data->channels*(1+2+2+2);
	int channels = data->channels;
	int hi,lo;
	
	// set up output
	data->out_bytes = 0;
	short *out = (short *) data->out;

	if ( (bytes -= hsize) < 0 )
	{
		return FALSE;
	}

	for ( i=0; i < channels; i++)
	{
		bpredictor = *in++;
		if (bpredictor >= 7) {
	    msg_assert(FALSE, ("MS-ADPCM bpredictor out of range"));
			return FALSE;
		}
		data->ms[i].iCoef[0] = MSADPCM_StdCoef[bpredictor][0];
		data->ms[i].iCoef[1] = MSADPCM_StdCoef[bpredictor][1];
	}

	for ( i=0; i < channels; i++)
	{
		lo = *in++;
		hi = *in++;
		data->ms[i].idelta = (hi<<8) | lo;
	}

	for ( i=0; i < channels; i++)
	{
		lo = *in++;
		hi = *in++;
		out[channels+i] = (hi<<8) | lo;
	}

	for ( i=0; i < channels; i++)
	{
		lo = *in++;
		hi = *in++;
		out[i] = (hi<<8) | lo;
	}

	/* already have 1st 2 samples from block-header */
	out += 2*data->channels;
	data->out_bytes += 4*data->channels;

	if ( channels == 1 )
	{
		unsigned char byte;

		while ( bytes ) 
		{
			byte = *in++;
			*out++ = MSAdpcmDecode(byte >> 4, &data->ms[0], out[-1], out[-2]);
			*out++ = MSAdpcmDecode(byte&0x0f, &data->ms[0], out[-1], out[-2]);
			bytes--;
			data->out_bytes += 4;
		}

	}
	else
	{
		int chan = 0;
		unsigned char byte;
		int channels2 = 2*channels;
		int out_bytes = 4*channels;

		while ( bytes ) 
		{
			byte = *in++;

			*out++ = MSAdpcmDecode(byte >> 4, &data->ms[chan], out[-channels], out[-channels2]);
			if (++chan == channels ) 
			{
				chan = 0;
			}

			*out++ = MSAdpcmDecode(byte&0x0f, &data->ms[chan], out[-channels], out[-channels2]);
			if (++chan == channels) 
			{
				chan = 0;
			}

			bytes--;
			data->out_bytes += out_bytes;;
		}
	}

	return TRUE;
}


// Stubs

DXDEC  void FAR * AILCALL AIL_mem_alloc_lock(U32       size)
{
	return AudioMemAlloc ( size );
}

DXDEC  void       AILCALL AIL_mem_free_lock (void FAR *ptr)
{
	AudioMemFree ( ptr );
}

DXDEC  U32     AILCALL  AIL_MMX_available             (void)
{
	return 0;
}

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

/*****************************************************************************
**                                                                          **
**                       Westwood Studios Pacific.                          **
**                                                                          **
**                       Confidential Information					                  **
**                Copyright (C) 2000 - All Rights Reserved                  **
**                                                                          **
******************************************************************************
**                                                                          **
** Project:  Dune Emperor                                                   **
**                                                                          **
** Module:  <module> (<prefix>_)                                            **
**                                                                          **
** Version:  $ID$                                                           **
**                                                                          **
** File name:  audcache.cpp                                                 **
**                                                                          **
** Created by:  04/30/99 TR                                                 **
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/

#include <string.h>
#include <memory.h>

#include <wpaudio/altypes.h>						//  always include this header first 
#include <wpaudio/memory.h>
#include <wpaudio/list.h>
#include <wpaudio/source.h>
#include <wpaudio/cache.h>
#include <wpaudio/profiler.h>

#include <wsys/file.h>

// 'assignment within condition expression'.
#pragma warning(disable : 4706)


DBG_DECLARE_TYPE ( AudioCache );
DBG_DECLARE_TYPE ( AudioCacheItem );

/*****************************************************************************
**          Externals                                                       **
*****************************************************************************/



/*****************************************************************************
**           Defines                                                        **
*****************************************************************************/

#define DEBUG_CACHE 0

/*****************************************************************************
**        Private Types                                                     **
*****************************************************************************/


/*****************************************************************************
**         Private Data                                                     **
*****************************************************************************/



/*****************************************************************************
**         Public Data                                                      **
*****************************************************************************/



/*****************************************************************************
**         Private Prototypes                                               **
*****************************************************************************/



/*****************************************************************************
**          Private Functions                                               **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void audioCacheAssetClose ( AudioCache *cache )
{
	if ( cache->assetFile )
	{
		cache->assetFile->close();
		cache->assetFile = NULL;
	}

	cache->assetBytesLeft = 0;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int	audioCacheAssetOpen ( AudioCache *cache, const char *name )
{
	audioCacheAssetClose ( cache );

	if ( name == NULL || cache->openAssetCB == NULL )
	{
		return FALSE;
	}

	cache->assetFile = cache->openAssetCB( name );

	if ( !cache->assetFile )
	{
		return FALSE;
	}

	if ( !AudioFormatReadWaveFile ( cache->assetFile, &cache->assetFormat, &cache->assetBytesLeft ))
	{
		audioCacheAssetClose ( cache );
		return FALSE;
	}

	return TRUE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int audioCacheAssetRead ( AudioCache *cache, void *data, int bytes )
{
		if ( bytes > cache->assetBytesLeft )
		{
			bytes = cache->assetBytesLeft;
		}

		return cache->assetFile ? cache->assetFile->read ( data, bytes ) : 0 ;
}


/*****************************************************************************
**          Public Functions                                                **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioCache*			AudioCacheCreate ( int cacheSize, int maxItems, int frameSize )
{
	AudioCache	*cache;
	int					frameBytes;
	int					pages;

	ALLOC_STRUCT ( cache, AudioCache );

	DBG_SET_TYPE ( cache, AudioCache );

	cache->frameSize = frameSize;

	frameBytes = sizeof ( AudioFrame ) + frameSize;
	pages = cacheSize/frameBytes;

	cache->framePool = MemoryPoolCreate ( pages, frameBytes );

	cache->itemPool = MemoryPoolCreate ( maxItems, sizeof ( AudioCacheItem ) );

	ListInit ( &cache->items );
	cache->assetFile = NULL;
	AudioFormatInit ( &cache->assetFormat );

	ProfCacheInit ( &cache->profile, pages, frameSize );
	ProfCacheUpdateInterval ( &cache->profile, 10 ); // every ten milliseconds
	
	if ( !cache->framePool || !cache->itemPool )
	{
		AudioCacheDestroy ( cache );
		cache = NULL;
	}

	return cache;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioCacheDestroy ( AudioCache *cache )
{
	AudioCacheItem	*item;
	

	DBG_ASSERT_TYPE ( cache, AudioCache );

	while ( ( item = (AudioCacheItem *) ListNodeNext ( &cache->items )) )
	{
		AudioCacheItemFree ( item );
	}

	if ( cache->framePool )
	{
		MemoryPoolDestroy ( cache->framePool );
	}

	if ( cache->itemPool )
	{
		MemoryPoolDestroy ( cache->itemPool );
	}

	DBG_INVALIDATE_TYPE ( cache );

	AudioMemFree ( cache );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioCacheItem*		AudioCacheGetItem ( AudioCache *cache, const char *name )
{
	AudioCacheItem	*item, *head;
	

	DBG_ASSERT_TYPE ( cache, AudioCache );

	item = head = (AudioCacheItem *) &cache->items ;

	while ( (item = (AudioCacheItem *) item->nd.next ) != head )
	{
		if ( item->valid && item->name == name )
		{
			return item;
		}
	}

	return NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioCacheInvalidate ( AudioCache *cache )
{
	AudioCacheItem	*item, *head;
	

	DBG_ASSERT_TYPE ( cache, AudioCache );

	item = head = (AudioCacheItem *) &cache->items ;

	while ( (item = (AudioCacheItem *) item->nd.next ) != head )
	{
		item->valid = FALSE;
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioCacheOpenCB*		AudioCacheSetOpenCB ( AudioCache *cache, AudioCacheOpenCB *cb )
{
	DBG_ASSERT_TYPE ( cache, AudioCache );

	AudioCacheOpenCB *old = cache->openAssetCB;
	cache->openAssetCB = cb;
	return old;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioCacheItem*		AudioCacheLoadItem ( AudioCache *cache, const char *name )
{
	AudioCacheItem *item;
	int			error;
	

	DBG_ASSERT_TYPE ( cache, AudioCache );

	if ( (item = AudioCacheGetItem ( cache, name )))
	{
		#if DEBUG_CACHE
			DBGPRINTF (("ACACHE: %10s - Cached\n", item->name ));
		#endif

		ListNodeRemove ( &item->nd );
		ListNodeAppend ( &cache->items, &item->nd );
		ProfCacheHit ( &cache->profile );

		return item;
	}

	ProfCacheMiss ( &cache->profile );
	//  item is not in the cache so load it 
	//  see first if the sample exists 

	#if DEBUG_CACHE
		DBGPRINTF (("ACACHE: %10s - Loading\n", name ));
	#endif

	ProfCacheLoadStart ( &cache->profile, 0 );

	if ( !audioCacheAssetOpen( cache, name ))
	{
		#if DEBUG_CACHE
			DBGPRINTF (("does not exist\n"));
		#endif
		goto none;
	}

	item = (AudioCacheItem *) MemoryPoolGetItem ( cache->itemPool );

	if ( !item )
	{
		//  free the oldest item so that we can use it's item struct 
		AudioCacheFreeOldestItem ( cache );
		item = (AudioCacheItem *) MemoryPoolGetItem ( cache->itemPool );

		if ( !item )
		{
			//  the oldest item could not be freed because it was still playing 
			DBGPRINTF (("Audio cache overflow\n"));
			audioCacheAssetClose ( cache );
			goto none;
		}
	}

	//  prepare item for use 
	item->name = name;
	item->cache = cache;
	ListNodeInit ( &item->nd );
	LockInit ( &item->lock );
	AudioSampleInit ( &item->sample );
	AudioSampleSetName ( &item->sample, name );
	AudioFormatInit ( &item->format );
	item->sample.Format = &item->format;
	DBG_SET_TYPE ( item, AudioCacheItem );

	error = FALSE;

	//  ok load sample data in to cache 
	{
		int bytesToTransfer;
		int bytes;
		int bytesTransfered;

		bytesToTransfer = cache->assetBytesLeft;

		while ( bytesToTransfer )
		{
			AudioFrame	*frame;
			void		*data;

			if ( (bytes = cache->frameSize ) > bytesToTransfer )
			{
				bytes = bytesToTransfer;
			}

			while ( ! (frame = (AudioFrame *) MemoryPoolGetItem ( cache->framePool )))
			{
				if ( !AudioCacheFreeOldestItem ( cache ) )
				{
					break;
				}
			}

			if ( !frame )
			{
				error = TRUE;
				break;
			}

			data = (void *) ( (uint) frame + sizeof ( AudioFrame ));

			AudioFrameInit ( frame, data, bytes );
			AudioSampleAddFrame ( &item->sample, frame );

			bytesTransfered = audioCacheAssetRead ( cache, data, bytes );
			ProfCacheAddLoadBytes ( &cache->profile, bytesTransfered );
			ProfCacheAddPage ( &cache->profile );
			ProfCacheFill ( &cache->profile, bytesTransfered );

			if ( bytesTransfered != bytes )
			{
				error = TRUE;
				break;
			}

			bytesToTransfer -= bytesTransfered;
		}
	}

	if ( error )
	{
		#if DEBUG_CACHE
			DBGPRINTF (("FAILED\n"));
		#endif
		AudioCacheItemFree ( item );
		goto none;
	}

	#if DEBUG_CACHE
		DBGPRINTF (("done\n"));
	#endif

	//  update the format structure 
	memcpy ( &item->format, &cache->assetFormat, sizeof ( AudioFormat) );


	ListNodeAppend ( &cache->items, &item->nd );
	item->valid = TRUE;
	audioCacheAssetClose ( cache );

	ProfCacheLoadEnd ( &cache->profile );
	return item;

none:

	ProfCacheLoadEnd ( &cache->profile );
	return NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

//  AudioCacheFreeOldestItem()
//  Free the oldest UNUSED item in the list.  These should tend to be
//  towards the end of the list, as old events should expire and unlock.
//  But not always (e.g. loops), so don't count on it.

int				AudioCacheFreeOldestItem( AudioCache *cache )
{
	AudioCacheItem *item;

	DBG_ASSERT_TYPE( cache, AudioCache );

	item = (AudioCacheItem*) ListNodePrev( &cache->items );

	while ( item )
	{
		if ( !AudioCacheItemInUse( item ) )
		{
			AudioCacheItemFree( item );
			return TRUE;
		}

		item = (AudioCacheItem*) ListNodePrev( (ListNode*) item );
	}

	return FALSE;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioCacheItemLock ( AudioCacheItem *item )
{
	

	DBG_ASSERT_TYPE ( item, AudioCacheItem );

	LockAcquire ( &item->lock );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioCacheItemUnlock ( AudioCacheItem *item )
{
	

	DBG_ASSERT_TYPE ( item, AudioCacheItem );

	LockRelease ( &item->lock );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int				AudioCacheItemInUse ( AudioCacheItem *item )
{
	

	DBG_ASSERT_TYPE ( item, AudioCacheItem );

	return Locked ( &item->lock );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioCacheItemFree ( AudioCacheItem *item )
{
	

	DBG_ASSERT_TYPE ( item, AudioCacheItem );

	if ( ListNodeInList ( &item->nd ))
	{
		ListNodeRemove ( &item->nd );
	}

	DBG_MSGASSERT ( !AudioCacheItemInUse ( item ), ("cache item is still in use"));

	#if DEBUG_CACHE
		DBGPRINTF (("ACACHE: %10s - Freeing\n", AudioBagGetItemName ( item->cache->bag, item->id )));
	#endif

	//  return frames to frame pool 
	{
		AudioFrame *frame;

		while ( (frame = (AudioFrame *) ListNodeNext ( &item->sample.Frames )) )
		{
			ListNodeRemove ( &frame->nd );
			ProfCacheRemove ( &item->cache->profile, frame->Bytes );
			AudioFrameDeinit ( frame );
			ProfCacheRemovePage ( &item->cache->profile );
			MemoryPoolReturnItem ( item->cache->framePool, frame );
		}
	}

	AudioSampleDeinit ( &item->sample );
	DBG_INVALIDATE_TYPE ( item );
	MemoryPoolReturnItem ( item->cache->itemPool, item );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioSample*		AudioCacheItemSample ( AudioCacheItem *item )
{
	DBG_ASSERT_TYPE ( item, AudioCacheItem );

	return &item->sample;
}


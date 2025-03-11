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
// Bin.cpp
//

#include "StdAfx.h"
#include "bin.h"
#include "assert.h"
#include "list.h"

Bin::Bin ( int size )
{
	assert ( size > 0 );
	num_buckets = size;
	sh_item = NULL;

	bucket = new List[size];

}

Bin::~Bin ( )
{
	Clear ();

	delete [] bucket;

}



void Bin::Clear ( void )
{
	int count = num_buckets;
	sh_item = NULL;
	while ( count-- )
	{
		List *head = &bucket[count];
		BinItem *item;

		while ( ( item = (BinItem *) head->Next ()))
		{
			Remove ( item );
		}
	}

}

void*				Bin::Get					( OLECHAR *text1, OLECHAR *text2 )
{
	BinItem *item;

	if ( ( item = GetBinItem ( text1, text2 )) )
	{
		return item->Item();
	}

	return NULL;
}

void*				Bin::GetNext			( void )
{
	BinItem *item;

	if ( ( item = GetNextBinItem ( )) )
	{
		return item->Item();
	}

	return NULL;
}

void				Bin::Add					( void *data, OLECHAR *text1, OLECHAR *text2 )
{
	BinItem *item;
	List		*list;
	int			hash;

	sh_item = NULL;

	hash = calc_hash ( text1 );
	item = new BinItem ( data, hash, text1, text2 );

	list = &bucket[hash%num_buckets];

	list->AddToTail ( (ListNode *) item );

}

BinItem*		Bin::GetBinItem		( OLECHAR *text1, OLECHAR *text2)
{
	
	sh_size1 = sh_size2 = 0;
	sh_text1 = text1;
	sh_text2 = text2;

	sh_hash = calc_hash ( text1 );

	if ( sh_text1 )
	{
		sh_size1 = wcslen ( sh_text1 );
	}

	if ( sh_text2 )
	{
		sh_size2 = wcslen ( sh_text2 );
	}

	sh_item = (BinItem *) &bucket[sh_hash%num_buckets];


	return GetNextBinItem ();
}

BinItem*		Bin::GetNextBinItem ( void )
{
	if ( sh_item )
	{
		sh_item = (BinItem *) sh_item->Next ();
	}

	while ( sh_item )
	{
		if ( sh_item->Same ( sh_hash, sh_text1, sh_size1, sh_text2, sh_size2 ))
		{
			break;
		}
		sh_item = (BinItem *) sh_item->Next ();
	}

	return sh_item;
}

BinItem*		Bin::GetBinItem	( void *item )
{
	BinItem *bitem = NULL;
	int i;

	
	for ( i=0; i< num_buckets; i++)
	{

		if ( ( bitem = (BinItem *) bucket[i].Find ( item )))
		{
			break;
		}

	}

	return bitem;
								
}

void				Bin::Remove			( void *item )
{
	BinItem *bitem;

	if ( ( bitem = GetBinItem ( item ) ))
	{
		Remove ( bitem );
	}

}

void				Bin::Remove			( OLECHAR *text1, OLECHAR *text2 )
{
	BinItem *bitem;

	if ( ( bitem = GetBinItem ( text1, text2 ) ))
	{
		Remove ( bitem );
	}

}

void				Bin::Remove			( BinItem *item )
{
	sh_item = NULL;
	item->Remove ();
	delete item ;

}


BinItem::BinItem ( void *data, int new_hash, OLECHAR *new_text1, OLECHAR *new_text2 )
{
	SetItem ( data );
	hash = new_hash;

	if ( (text1 = new_text1) )
	{
		text1size = wcslen ( text1 );
	}
	else
	{
		text1size = 0;
	}

	if ( (text2 = new_text2) )
	{
		text2size = wcslen ( text2 );
	}
	else
	{
		text2size = 0;
	}
}

int BinItem::Same ( int chash, OLECHAR *ctext1, int size1, OLECHAR *ctext2, int size2 )
{
	if ( hash != chash || text1size != size1 || text2size != size2 )
	{
		return FALSE;
	}

	if ( wcsicmp ( text1, ctext1 ))
	{
		return FALSE;
	}

	if ( text2 && ctext2 && wcsicmp ( text2, ctext2 ))
	{
		return FALSE;
	}

	return TRUE;


}
int Bin::calc_hash ( OLECHAR *text )
{
	int hash = 0;

	while ( *text )
	{
		hash += *text++;
	}

	return hash;

}

// Bin ID code

BinID::BinID ( int size )
{
	assert ( size > 0 );
	num_buckets = size;

	bucket = new List[size];

}

BinID::~BinID ( )
{
	Clear ();

	delete [] bucket;

}



void BinID::Clear ( void )
{
	int count = num_buckets;

	while ( count-- )
	{
		List *head = &bucket[count];
		BinIDItem *item;

		while ( ( item = (BinIDItem *) head->Next ()))
		{
			Remove ( item );
		}
	}

}

void*				BinID::Get					( int id)
{
	BinIDItem *item;

	if ( ( item = GetBinIDItem ( id  )) )
	{
		return item->Item();
	}

	return NULL;
}

void				BinID::Add					( void *data, int id )
{
	BinIDItem *item;
	List		*list;


	item = new BinIDItem ( data, id );

	list = &bucket[id%num_buckets];

	list->AddToTail ( (ListNode *) item );

}

BinIDItem*		BinID::GetBinIDItem		( int id )
{
	BinIDItem *item;
	

	item = (BinIDItem *) bucket[id%num_buckets].Next();

	while ( item )
	{
		if ( item->Same ( id ))
		{
			break;
		}
		item = (BinIDItem *) item->Next ();
	}

	return item	;
}


BinIDItem*		BinID::GetBinIDItem	( void *item )
{
	BinIDItem *bitem = NULL;
	int i;

	
	for ( i=0; i< num_buckets; i++)
	{

		if ( ( bitem = (BinIDItem *) bucket[i].Find ( item )))
		{
			break;
		}

	}

	return bitem;
								
}

void				BinID::Remove			( void *item )
{
	BinIDItem *bitem;

	if ( ( bitem = GetBinIDItem ( item ) ))
	{
		Remove ( bitem );
	}

}

void				BinID::Remove			( int id  )
{
	BinIDItem *bitem;

	if ( ( bitem = GetBinIDItem ( id ) ))
	{
		Remove ( bitem );
	}

}

void				BinID::Remove			( BinIDItem *item )
{
	item->Remove ();
	delete item ;

}


BinIDItem::BinIDItem ( void *data, int new_id  )
{
	SetItem ( data );
	id = new_id;

}

int BinIDItem::Same ( int compare_id )
{
	return id == compare_id;
}


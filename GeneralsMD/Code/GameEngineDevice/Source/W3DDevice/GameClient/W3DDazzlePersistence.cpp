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
#include "W3DDevice/GameClient/W3DDazzleRenderObject.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"
#include "WWLib/chunkio.h"
#include "WWSaveLoad/persistfactory.h"
#include "WWSaveLoad/saveloadids.h"
#include "WWSaveLoad/saveload.h"
namespace
{

constexpr uint32 Dazzle_Persist_Chunk_Id = CHUNKID_WW3D_BEGIN + 2;

}

class DazzlePersistFactory : public PersistFactoryClass
{
	virtual uint32				Chunk_ID() const override;
	virtual PersistClass *	Load(ChunkLoadClass & cload) const override;
	virtual void				Save(ChunkSaveClass & csave,PersistClass * obj)	const override;

	enum
	{
		DAZZLEFACTORY_CHUNKID_VARIABLES		= 1212000336,
		DAZZLEFACTORY_VARIABLE_OBJPOINTER	= 0x00,
		OBSOLETE_DAZZLEFACTORY_VARIABLE_TYPE,
		DAZZLEFACTORY_VARIABLE_TRANSFORM,
		DAZZLEFACTORY_VARIABLE_TYPENAME,
	};
};

static DazzlePersistFactory _DazzleFactory;

uint32 DazzlePersistFactory::Chunk_ID() const
{
	return Dazzle_Persist_Chunk_Id;
}

PersistClass *	DazzlePersistFactory::Load(ChunkLoadClass & cload) const
{
	W3DDazzleRenderObject * old_obj = nullptr;
	Matrix3D tm(1);
	char dazzle_type[256];
	dazzle_type[0] = 0;


	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) {

			case DAZZLEFACTORY_CHUNKID_VARIABLES:

				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,DAZZLEFACTORY_VARIABLE_OBJPOINTER,old_obj);
						READ_MICRO_CHUNK(cload,DAZZLEFACTORY_VARIABLE_TRANSFORM,tm);
						READ_MICRO_CHUNK_STRING(cload,DAZZLEFACTORY_VARIABLE_TYPENAME,dazzle_type,sizeof(dazzle_type));
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d",__FILE__,__LINE__));
				break;
		};
		cload.Close_Chunk();
	}


	W3DRenderObject * new_obj = NEW_REF(W3DDazzleRenderObject,(dazzle_type));


	if (new_obj == nullptr) {
		static int count = 0;
		if ( count++ < 10 ) {
			WWDEBUG_SAY(("DazzlePersistFactory failed to create dazzle of type: %s!!",dazzle_type));
			WWDEBUG_SAY(("Replacing it with a null render object!"));
		}
		new_obj = W3DAssetCatalog::Get_Instance()->Create_Render_Obj("NULL");
	}

	WWASSERT(new_obj != nullptr);
	if (new_obj) {
		new_obj->Set_Transform(tm);
	}


	SaveLoadSystemClass::Register_Pointer(old_obj,new_obj);
	return new_obj;
}

void DazzlePersistFactory::Save(ChunkSaveClass & csave,PersistClass * obj)	const
{
	W3DDazzleRenderObject * robj = (W3DDazzleRenderObject *)obj;
	unsigned int dazzle_type = robj->Get_Dazzle_Type();
	const char * dazzle_type_name = W3DDazzleRenderObject::Get_Type_Name(dazzle_type);
	Matrix3D tm = robj->Get_Transform();

	csave.Begin_Chunk(DAZZLEFACTORY_CHUNKID_VARIABLES);
	WRITE_MICRO_CHUNK(csave,DAZZLEFACTORY_VARIABLE_OBJPOINTER,robj);
	WRITE_MICRO_CHUNK(csave,DAZZLEFACTORY_VARIABLE_TRANSFORM,tm);
	WRITE_MICRO_CHUNK_STRING(csave,DAZZLEFACTORY_VARIABLE_TYPENAME,dazzle_type_name);

	csave.End_Chunk();
}

const PersistFactoryClass & W3DDazzleRenderObject::Get_Factory () const
{
	return _DazzleFactory;
}


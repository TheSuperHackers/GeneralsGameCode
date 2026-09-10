#include "W3DDevice/GameClient/W3DRenderObject.h"

#include <cstring>

#include "WWLib/chunkio.h"
#include "WWSaveLoad/persistfactory.h"
#include "WWSaveLoad/saveload.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"

namespace
{

constexpr std::uint32_t Render_Object_Persist_Chunk = 0x00010000;
constexpr std::uint32_t Render_Object_Variables_Chunk = 0x00555040;
constexpr std::uint32_t Render_Object_Variable_Pointer = 0;
constexpr std::uint32_t Render_Object_Variable_Name = 1;
constexpr std::uint32_t Render_Object_Variable_Transform = 2;

class W3DRenderObjectPersistFactory final : public PersistFactoryClass
{
public:
    uint32 Chunk_ID() const override { return Render_Object_Persist_Chunk; }
    PersistClass *Load(ChunkLoadClass &load) const override;
    void Save(ChunkSaveClass &save, PersistClass *object) const override;
};

W3DRenderObjectPersistFactory g_w3d_render_object_persist_factory;

PersistClass *W3DRenderObjectPersistFactory::Load(ChunkLoadClass &load) const
{
    W3DRenderObject *old_object = nullptr;
    Matrix3D transform(true);
    char name[256]{};

    while (load.Open_Chunk()) {
        switch (load.Cur_Chunk_ID()) {
        case Render_Object_Variables_Chunk:
            while (load.Open_Micro_Chunk()) {
                switch (load.Cur_Micro_Chunk_ID()) {
                case Render_Object_Variable_Pointer:
                    load.Read(&old_object, sizeof(old_object));
                    break;
                case Render_Object_Variable_Name:
                    if (load.Cur_Micro_Chunk_Length() < sizeof(name))
                        load.Read(name, load.Cur_Micro_Chunk_Length());
                    else
                        load.Seek(load.Cur_Micro_Chunk_Length());
                    name[sizeof(name) - 1] = '\0';
                    break;
                case Render_Object_Variable_Transform:
                    load.Read(&transform, sizeof(transform));
                    break;
                default:
                    load.Seek(load.Cur_Micro_Chunk_Length());
                    break;
                }
                load.Close_Micro_Chunk();
            }
            break;
        default:
            load.Seek(load.Cur_Chunk_Length());
            break;
        }
        load.Close_Chunk();
    }

    if (name[0] == '\0')
        std::strcpy(name, "NULL");

    W3DRenderObject *new_object = nullptr;
    if (auto *asset_manager = W3DAssetCatalog::Get_Instance()) {
        new_object = asset_manager->Create_Render_Obj(name);
        if (new_object == nullptr)
            new_object = asset_manager->Create_Render_Obj("NULL");
    }
    if (new_object != nullptr)
        new_object->Set_Transform(transform);

    SaveLoadSystemClass::Register_Pointer(old_object, new_object);
    return new_object;
}

void W3DRenderObjectPersistFactory::Save(ChunkSaveClass &save, PersistClass *object) const
{
    auto *render_object = static_cast<W3DRenderObject *>(object);
    const char *name = render_object->Get_Name();
    if (name == nullptr)
        name = "";
    const Matrix3D transform = render_object->Get_Transform();

    save.Begin_Chunk(Render_Object_Variables_Chunk);
    save.Begin_Micro_Chunk(Render_Object_Variable_Pointer);
    save.Write(&render_object, sizeof(render_object));
    save.End_Micro_Chunk();
    save.Begin_Micro_Chunk(Render_Object_Variable_Name);
    save.Write(name, static_cast<std::uint32_t>(std::strlen(name) + 1));
    save.End_Micro_Chunk();
    save.Begin_Micro_Chunk(Render_Object_Variable_Transform);
    save.Write(&transform, sizeof(transform));
    save.End_Micro_Chunk();
    save.End_Chunk();
}

}

const PersistFactoryClass &W3DRenderObject::Get_Factory() const
{
    return g_w3d_render_object_persist_factory;
}

bool W3DRenderObject::Save(ChunkSaveClass &save)
{
    (void)save;
    return true;
}

bool W3DRenderObject::Load(ChunkLoadClass &load)
{
    (void)load;
    return true;
}

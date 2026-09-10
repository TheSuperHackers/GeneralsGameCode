#include <array>
#include <cstddef>
#include <string>
#include <utility>

import Assets.Adapters.W3D.Null;
import Graphics.Scene.Models.Factory;

#include "W3DDevice/GameClient/NullRenderObject.h"
#include "WWLib/chunkio.h"

namespace
{

constexpr std::size_t NullNameCapacity = Assets::W3D::W3DNullNameSize - 1;

std::string Copy_Name(const char *name)
{
	if (name == nullptr)
		return {};
	std::size_t length = 0;
	while (length < NullNameCapacity && name[length] != '\0')
		++length;
	return std::string(name, length);
}

}

NullRenderObject::NullRenderObject(const char *name)
	: m_name(Copy_Name(name))
{
}

NullRenderObject::NullRenderObject(
	const Assets::W3D::W3DNullDescription &description)
	: m_name(Copy_Name(description.name.c_str()))
{
}

NullRenderObject::NullRenderObject(const NullRenderObject &source)
	// The historical NULL copy constructor intentionally default-constructed
	// W3DRenderObject. Keep that clone contract: identity is copied while base
	// scene, visibility, collision, and transform state starts fresh.
	: m_name(source.m_name)
{
}

NullRenderObject &NullRenderObject::operator=(const NullRenderObject &source)
{
	if (this == &source)
		return *this;
	m_name = source.m_name;
	W3DRenderObject::operator=(source);
	return *this;
}

W3DRenderObject *NullRenderObject::Clone() const
{
	return NEW_REF(NullRenderObject, (*this));
}

void NullRenderObject::Render(W3DRenderContext &rinfo)
{
	(void)rinfo;
}

void NullRenderObject::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
	sphere.Center.Set(0, 0, 0);
	sphere.Radius = 0.1f;
}

void NullRenderObject::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
	box.Center.Set(0, 0, 0);
	box.Extent.Set(0.1f, 0.1f, 0.1f);
}

Graphics::ModelFactory<W3DRenderObject> *Load_Null_Factory(ChunkLoadClass &cload)
{
	std::array<std::byte, Assets::W3D::W3DNullPayloadSize> bytes{};
	if (cload.Cur_Chunk_Length() < bytes.size()
		|| cload.Read(bytes.data(), static_cast<unsigned>(bytes.size())) != bytes.size())
		return nullptr;

	Assets::W3D::W3DNullDescription description;
	std::string error;
	if (!Assets::W3D::W3DRead_Null(bytes, description, error))
		return nullptr;
	const std::string name = std::move(description.name);
	return new Graphics::ModelFactory<W3DRenderObject>(name,
		W3DRenderObject::CLASSID_NULL, [name] {
			return NEW_REF(NullRenderObject, (name.c_str()));
		});
}

Graphics::ModelFactory<W3DRenderObject> *Create_Null_Render_Object_Factory()
{
	return new Graphics::ModelFactory<W3DRenderObject>("NULL",
		W3DRenderObject::CLASSID_NULL, [] {
			return NEW_REF(NullRenderObject, ("NULL"));
		});
}

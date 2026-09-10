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
#include <array>
#include <cstdio>
#include "W3DDevice/GameClient/W3DAssetCatalog.h"
#include "W3DDevice/GameClient/W3DDazzleResources.h"
#include "WWLib/INI.h"
#include "WWLib/inisup.h"
#include "WWLib/Point.h"
#include "WWLib/WWFILE.h"
#include "WWLib/ffactory.h"
import Assets.Adapters.W3D.Dazzle;

namespace {
Graphics::DazzleResources<RefCountPtr<W3DTextureHandle>> resources;
struct DazzleINIReader final {
    const INIClass& ini;
    std::vector<std::string> List(const char* section) const {
        std::vector<std::string> names;
        const int count = ini.Entry_Count(section);
        names.reserve(count);
        for (int i = 0; i < count; ++i) {
            char name[80];
            ini.Get_String(section, ini.Get_Entry(section, i), "", name, sizeof(name));
            names.emplace_back(name);
        }
        return names;
    }
    std::string String(const std::string& section, const std::string& key) const {
        StringClass value;
        ini.Get_String(value, section.c_str(), key.c_str());
        return static_cast<const char*>(value);
    }
    float Float(const std::string& section, const std::string& key, float fallback) const {
        return ini.Get_Float(section.c_str(), key.c_str(), fallback);
    }
    int Integer(const std::string& section, const std::string& key, int fallback) const {
        return ini.Get_Int(section.c_str(), key.c_str(), fallback);
    }
    std::array<float, 3> Vector3(const std::string& section, const std::string& key, std::array<float, 3> fallback) const {
        const auto value = ini.Get_Point(section.c_str(), key.c_str(), TPoint3D<float>(fallback[0], fallback[1], fallback[2]));
        return {value.X, value.Y, value.Z};
    }
    std::array<float, 4> Vector4(const std::string& section, const std::string& key, std::array<float, 4> fallback) const {
        const auto* entry = ini.Find_Entry(section.c_str(), key.c_str());
        std::array<float, 4> value;
        if (entry && entry->Value && std::sscanf(entry->Value, "%f,%f,%f,%f", &value[0], &value[1], &value[2], &value[3]) == 4)
            return value;
        return fallback;
    }
};
}
Graphics::DazzleResources<RefCountPtr<W3DTextureHandle>>& Get_Dazzle_Resources() { return resources; }
bool Initialize_Dazzle_Resources() {
    FileClass* file = _TheFileFactory->Get_File("DAZZLE.INI");
    if (!file) return true;
    Assets::DazzleDefinitions definitions;
    bool decoded;
    {
        INIClass ini(*file);
        decoded = Assets::W3D::W3DRead_Dazzle_Definitions(DazzleINIReader{ini}, definitions);
    }
    _TheFileFactory->Return_File(file);
    if (!decoded) return false;
    resources.Initialize(std::move(definitions));
    return true;
}
void Shutdown_Dazzle_Resources() { resources.Clear(); }
RefCountPtr<W3DTextureHandle> Acquire_Dazzle_Texture(const std::string& name) {
    return RefCountPtr<W3DTextureHandle>::Create_No_Add_Ref(W3DAssetCatalog::Get_Instance()->Get_Texture(name.c_str()));
}

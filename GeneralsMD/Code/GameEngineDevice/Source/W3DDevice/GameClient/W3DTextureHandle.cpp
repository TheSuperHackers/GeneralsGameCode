import Graphics.Frame.RenderClock;
import Graphics.Frame.RenderSettings;
#include "W3DDevice/GameClient/W3DRenderServices.h"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "WWLib/ffactory.h"


import Assets.Images.PixelEncoding;
import Graphics.Frame.Runtime;
import Graphics.Resources.Loading.Queue;
import Graphics.Resources.Textures.Load;
import Graphics.Resources.Textures.CachePolicy;
import Graphics.Resources.Textures.Quality;
import Graphics.Resources.Textures.Storage;

namespace
{

unsigned unused_texture_id;

unsigned Next_Texture_Id() noexcept
{
    return unused_texture_id++;
}

Graphics::TextureResidencyClock Make_Game_Texture_Residency_Clock(
    Graphics::TextureResidencyClock clock)
{
    if (clock)
        return clock;
    return [] { return Graphics::Get_Render_Clock().Sync_Time(); };
}

Graphics::TextureImageReader Make_Archive_Reader(std::string path)
{
    return [path = std::move(path)](std::size_t prefix, std::vector<std::byte>& bytes,
        std::size_t& source_size) {
        if (!_TheFileFactory || path.empty())
            return false;

        file_auto_ptr file(_TheFileFactory, path.c_str());
        if (!file.get() || !file->Is_Available() || file->Open() == 0)
            return false;

        const int size = file->Size();
        if (size <= 0 || prefix > static_cast<std::size_t>(size))
            return false;

        const std::size_t read_size = prefix ? prefix : static_cast<std::size_t>(size);
        bytes.resize(read_size);
        if (file->Read(bytes.data(), static_cast<int>(read_size)) != static_cast<int>(read_size))
            return false;

        source_size = static_cast<std::size_t>(size);
        return true;
    };
}

bool Is_Compressed(Assets::PixelEncoding encoding) noexcept
{
    using E = Assets::PixelEncoding;
    return encoding == E::BC1 || encoding == E::BC2 || encoding == E::BC2Premultiplied
        || encoding == E::BC3 || encoding == E::BC3Premultiplied;
}

bool Is_Bump(Assets::PixelEncoding encoding) noexcept
{
    using E = Assets::PixelEncoding;
    return encoding == E::RG8_SNorm || encoding == E::RG5_SNorm_L6
        || encoding == E::RG8_SNorm_L8X8;
}

std::string To_String(const StringClass& value)
{
    const char* text = value.str();
    return text ? std::string(text) : std::string{};
}

}

struct W3DTextureHandle::LoadState final
{
    std::mutex mutex;
    W3DTextureHandle* owner = nullptr;
};

W3DTextureHandle::W3DTextureHandle(unsigned width, unsigned height,
    Assets::PixelEncoding format, MipCountType mip_level_count, PoolType pool,
    bool render_target, bool allow_reduction, Graphics::TextureResidencyClock clock)
    : W3DTextureHandle(width, height, format, mip_level_count, pool,
        render_target, allow_reduction, TEX_REGULAR, 1, std::move(clock))
{
}

W3DTextureHandle::W3DTextureHandle(unsigned width, unsigned height,
    Assets::PixelEncoding format, MipCountType mip_level_count, PoolType pool,
    bool render_target, bool allow_reduction, TexAssetType asset_type,
    unsigned depth, Graphics::TextureResidencyClock clock)
    : m_residency(Make_Game_Texture_Residency_Clock(std::move(clock))),
      m_sampling(Graphics::Make_Texture_Sampling(mip_level_count != MIP_LEVELS_1)),
      m_texture_format(format),
      m_mip_level_count(mip_level_count),
      m_pool(pool),
      m_asset_type(asset_type),
      m_depth(asset_type == TEX_VOLUME ? (std::max)(1u, depth) : 1u),
      m_texture_id(Next_Texture_Id()),
      m_width(static_cast<int>(width)),
      m_height(static_cast<int>(height)),
      m_allow_compression(Is_Compressed(format)),
      m_reducible(false),
      m_render_target(render_target),
      m_procedural_recreation_enabled(true)
{
    m_texture_description.width = width;
    m_texture_description.height = height;
    m_texture_description.mip_count = Mip_Count(mip_level_count);
    m_texture_description.usage = static_cast<unsigned>(Graphics::RHITextureUsage::ShaderResource);
    if (render_target)
        m_texture_description.usage |= static_cast<unsigned>(Graphics::RHITextureUsage::RenderTarget);

    if (pool == POOL_DEFAULT)
        m_dirty = true;

    Configure_Procedural_Recreation(true);
    auto* resource = Create_Procedural_Resource();
    if (resource)
        m_texture_description = resource->Description();
    m_residency.Set_Resource(resource);
    m_residency.Set_Initialized(m_residency.Is_Resident());
}

W3DTextureHandle::W3DTextureHandle(const char* name, const char* full_path,
    MipCountType mip_level_count, Assets::PixelEncoding texture_format,
    bool allow_compression, bool allow_reduction, TexAssetType asset_type,
    Graphics::TextureResidencyClock clock)
    : m_residency(Make_Game_Texture_Residency_Clock(std::move(clock))),
      m_sampling(Graphics::Make_Texture_Sampling(mip_level_count != MIP_LEVELS_1)),
      m_texture_format(texture_format),
      m_mip_level_count(mip_level_count),
      m_asset_type(asset_type),
      m_depth(asset_type == TEX_VOLUME ? 0u : 1u),
      m_texture_id(Next_Texture_Id()),
      m_allow_compression(allow_compression),
      m_reducible(allow_reduction),
      m_load_state(std::make_shared<LoadState>())
{
    if (Is_Compressed(m_texture_format))
        m_allow_compression = true;
    if (Is_Bump(m_texture_format)) {
        if (!Get_W3D_Render_Services().Is_Initialized()
            || Graphics::Texture_Storage_Format(m_texture_format) == Graphics::RHITextureFormat::Unknown)
            m_texture_format = Assets::PixelEncoding::Unknown;
        else {
            m_allow_compression = false;
            m_mip_level_count = MIP_LEVELS_1;
            m_sampling.mipmap = Graphics::SamplingFilter::Disabled;
        }
    }
    m_residency.Set_Inactivation_Time(Graphics::TextureCachePolicy::Default_Inactivation_Time);
    m_residency.Set_Resource(nullptr);

    if (name)
        m_name = name;
    if (full_path)
        m_full_path = full_path;

    for (const char* character = name; character && *character; ++character) {
        if (*character == '+') {
            m_is_lightmap = true;
            m_sampling.minification = Graphics::SamplingFilter::Fast;
            m_sampling.magnification = Graphics::SamplingFilter::Fast;
            if (mip_level_count != MIP_LEVELS_1)
                m_sampling.mipmap = Graphics::SamplingFilter::Fast;
            break;
        }
    }

    if (m_load_state)
        m_load_state->owner = this;

    if (!Graphics::Get_Render_Settings().Is_Texturing_Enabled())
        m_residency.Set_Initialized(true);

    Configure_File_Load();
}

W3DTextureHandle::W3DTextureHandle(Graphics::TextureEdit* surface,
    MipCountType mip_level_count, Graphics::TextureResidencyClock clock)
    : m_residency(Make_Game_Texture_Residency_Clock(std::move(clock))),
      m_sampling(Graphics::Make_Texture_Sampling(mip_level_count != MIP_LEVELS_1)),
      m_mip_level_count(mip_level_count),
      m_asset_type(TEX_REGULAR),
      m_texture_id(Next_Texture_Id()),
      m_reducible(false),
      m_procedural_recreation_enabled(false)
{
    if (surface) {
        const auto description = surface->Image().Description();
        m_texture_format = description.encoding;
        m_width = static_cast<int>(description.width);
        m_height = static_cast<int>(description.height);
        m_allow_compression = Is_Compressed(description.encoding);
        m_texture_description.width = description.width;
        m_texture_description.height = description.height;
        m_texture_description.mip_count = Mip_Count(mip_level_count);
    }

    m_residency.Set_Procedural(true);
    auto* resource = surface ? surface->Create_Texture(
        Graphics::Shared_Frame_Device(), Mip_Count(mip_level_count)) : nullptr;
    if (resource)
        m_texture_description = resource->Description();
    m_residency.Set_Resource(resource);
    m_residency.Set_Initialized(m_residency.Is_Resident());
}

W3DTextureHandle::W3DTextureHandle(Graphics::TextureResource* texture,
    Graphics::TextureResidencyClock clock)
    : m_residency(Make_Game_Texture_Residency_Clock(std::move(clock))),
      m_sampling(Graphics::Make_Texture_Sampling(texture && texture->Description().mip_count != 1)),
      m_asset_type(TEX_REGULAR),
      m_texture_id(Next_Texture_Id()),
      m_reducible(false),
      m_procedural_recreation_enabled(false)
{
    m_residency.Set_Procedural(true);
    m_residency.Set_Resource(Graphics::Retain_Texture_Resource(texture));
    m_residency.Set_Initialized(m_residency.Is_Resident());
    if (!texture)
        return;

    const auto& description = texture->Description();
    if (description.dimension == Graphics::RHITextureDimension::Cube)
        m_asset_type = TEX_CUBEMAP;
    else if (description.dimension == Graphics::RHITextureDimension::Volume)
        m_asset_type = TEX_VOLUME;
    m_texture_description = description;
    m_texture_format = texture->Encoding();
    m_width = static_cast<int>(description.width);
    m_height = static_cast<int>(description.height);
    m_depth = description.depth;
    m_allow_compression = Is_Compressed(m_texture_format);
    m_mip_level_count = static_cast<MipCountType>(description.mip_count);
    m_render_target = texture->Is_Render_Target();
}

W3DTextureHandle::W3DTextureHandle(unsigned width, unsigned height,
    Graphics::RHITextureFormat depth_format, MipCountType mip_level_count,
    PoolType pool, Graphics::TextureResidencyClock clock)
    : m_residency(Make_Game_Texture_Residency_Clock(std::move(clock))),
      m_mip_level_count(mip_level_count),
      m_pool(pool),
      m_asset_type(TEX_DEPTH),
      m_texture_id(Next_Texture_Id()),
      m_width(static_cast<int>(width)),
      m_height(static_cast<int>(height)),
      m_reducible(false),
      m_procedural_recreation_enabled(true)
{
    m_texture_description = {width, height, 1, depth_format,
        static_cast<unsigned>(Graphics::RHITextureUsage::DepthStencil), 1, 1,
        Graphics::RHITextureDimension::Texture2D, false, 0};
    if (pool == POOL_DEFAULT)
        m_dirty = true;

    Configure_Procedural_Recreation(pool == POOL_DEFAULT);
    auto* resource = Create_Procedural_Resource();
    if (resource)
        m_texture_description = resource->Description();
    m_residency.Set_Resource(resource);
    m_residency.Set_Initialized(m_residency.Is_Resident());
}

W3DTextureHandle::~W3DTextureHandle()
{
    Detach_Load_State();
}

void W3DTextureHandle::Detach_Load_State() noexcept
{
    auto state = std::move(m_load_state);
    if (!state)
        return;

    std::lock_guard lock(state->mutex);
    state->owner = nullptr;
}

unsigned W3DTextureHandle::Mip_Count(MipCountType count) noexcept
{
    return count == MIP_LEVELS_ALL ? 0u : static_cast<unsigned>(count);
}

void W3DTextureHandle::Configure_Procedural_Recreation(bool register_for_recreation)
{
    m_residency.Set_Procedural(true);
    m_residency.Set_Recreate_Callback([this] { return Recreate_Procedural_Texture(); });
    if (register_for_recreation)
        m_residency.Register_For_Recreation();
}

Graphics::TextureResource* W3DTextureHandle::Create_Procedural_Resource() const noexcept
{
    Graphics::Device* device = Graphics::Shared_Frame_Device();
    if (!device || m_width <= 0 || m_height <= 0)
        return nullptr;

    auto description = m_texture_description;
    description.width = static_cast<unsigned>(m_width);
    description.height = static_cast<unsigned>(m_height);
    description.mip_count = Mip_Count(m_mip_level_count);

    if (m_asset_type == TEX_CUBEMAP) {
        description.dimension = Graphics::RHITextureDimension::Cube;
        description.array_size = 6;
    } else if (m_asset_type == TEX_VOLUME) {
        description.dimension = Graphics::RHITextureDimension::Volume;
        description.depth = (std::max)(1u, m_depth);
    } else if (m_asset_type == TEX_DEPTH) {
        description.usage = static_cast<unsigned>(Graphics::RHITextureUsage::DepthStencil);
        description.format = m_texture_description.format;
        description.mip_count = 1;
        return Graphics::TextureResource::Create(device, description,
            Assets::PixelEncoding::Unknown);
    }

    const auto attachment = m_render_target
        ? Graphics::RHITextureFormat::D24_UNorm_S8
        : Graphics::RHITextureFormat::Unknown;
    return Graphics::TextureResource::Create(device, description, m_texture_format, attachment);
}

bool W3DTextureHandle::Recreate_Procedural_Texture()
{
    if (!m_procedural_recreation_enabled)
        return false;
    m_residency.Set_Resource(Create_Procedural_Resource());
    return m_residency.Is_Resident();
}

void W3DTextureHandle::Init()
{
    if (m_residency.Is_Procedural()) {
        if (m_residency.Ensure())
            m_residency.Set_Initialized(true);
    } else {
        m_residency.Initialize_If_Needed();
    }
}

bool W3DTextureHandle::Ensure_Render_Backend_Texture()
{
    return m_residency.Ensure();
}

void W3DTextureHandle::Invalidate() noexcept
{
    m_residency.Invalidate();
}

void W3DTextureHandle::Apply_New_Surface(Graphics::TextureResource* texture,
    bool initialized, bool disable_auto_invalidation) noexcept
{
    m_residency.Publish(texture, initialized, disable_auto_invalidation);
    if (!initialized || !texture)
        return;

    const auto& description = texture->Description();
    m_texture_description = description;
    m_texture_format = texture->Encoding();
    m_width = static_cast<int>(description.width);
    m_height = static_cast<int>(description.height);
    m_depth = description.depth;
}

void W3DTextureHandle::Set_HSV_Shift(const Vector3& hsv_shift) noexcept
{
    Invalidate();
    m_hsv_shift = hsv_shift;
}

bool W3DTextureHandle::Is_Missing_Texture() const noexcept
{
    const auto* texture = m_residency.Resource();
    return texture && texture->Is_Placeholder();
}

Graphics::TextureEdit* W3DTextureHandle::Get_Surface_Level(unsigned level)
{
    if (!Ensure_Render_Backend_Texture())
        return nullptr;
    auto* texture = m_residency.Resource();
    return texture ? Graphics::TextureEdit::Readback(*texture, level) : nullptr;
}

void W3DTextureHandle::Get_Level_Description(Assets::ImageDescription& description,
    unsigned level)
{
    description = {};
    if (!Ensure_Render_Backend_Texture())
        return;
    const auto* texture = m_residency.Resource();
    if (!texture || level >= texture->Description().mip_count)
        return;
    const auto& source = texture->Description();
    description = {texture->Encoding(), (std::max)(1u, source.width >> level),
        (std::max)(1u, source.height >> level)};
}

unsigned W3DTextureHandle::Texture_Level_Size(const Graphics::TextureResource& texture,
    unsigned level) noexcept
{
    const auto& description = texture.Description();
    const unsigned width = (std::max)(1u, description.width >> level);
    const unsigned height = (std::max)(1u, description.height >> level);
    const auto encoding = texture.Encoding();
    if (Assets::Is_Block_Compressed(encoding)) {
        const unsigned blocks_x = (std::max)(1u, (width + 3) / 4);
        const unsigned blocks_y = (std::max)(1u, (height + 3) / 4);
        return blocks_x * blocks_y * (encoding == Assets::PixelEncoding::BC1 ? 8u : 16u);
    }
    if (encoding == Assets::PixelEncoding::Unknown) {
        if (description.format == Graphics::RHITextureFormat::D16_UNorm)
            return width * height * 2u;
        return description.format == Graphics::RHITextureFormat::Unknown ? 0u : width * height * 4u;
    }
    return width * height * Assets::Pixel_Size(encoding);
}

unsigned W3DTextureHandle::Get_Texture_Memory_Usage() const noexcept
{
    const auto* texture = m_residency.Resource();
    if (!texture)
        return 0;
    unsigned result = 0;
    for (unsigned level = 0; level < texture->Description().mip_count; ++level)
        result += Texture_Level_Size(*texture, level);
    return result;
}

std::unique_ptr<Graphics::ResourceLoadJob> W3DTextureHandle::Make_Load_Job(
    RefCountPtr<W3DTextureHandle> owner) const
{
    if (!owner || m_asset_type == TEX_DEPTH)
        return {};

    Graphics::TextureLoadRequest request;
    request.device = Graphics::Shared_Frame_Device();
    if (!request.device)
        return {};
    request.dimension = m_asset_type == TEX_CUBEMAP ? Graphics::RHITextureDimension::Cube
        : m_asset_type == TEX_VOLUME ? Graphics::RHITextureDimension::Volume
        : Graphics::RHITextureDimension::Texture2D;
    request.encoding = m_texture_format;
    const auto& quality = Graphics::Get_Texture_Quality_Settings();
    request.mips = {Mip_Count(m_mip_level_count),
        static_cast<unsigned>((std::max)(0, quality.mip_reduction)),
        static_cast<unsigned>((std::max)(1, quality.minimum_dimension)), m_reducible};
    request.allow_compression = m_allow_compression;
    request.prefer_16_bits = quality.prefer_16_bits;
    request.hsv_shift = {m_hsv_shift.X, m_hsv_shift.Y, m_hsv_shift.Z};

    std::string path = To_String(Get_Full_Path());
    request.read_tga = Make_Archive_Reader(path);
    if (path.size() >= 4) {
        const auto extension = path.size() - 3;
        path.replace(extension, 3, "dds");
        request.read_dds = Make_Archive_Reader(std::move(path));
    }

    return std::make_unique<Graphics::TextureLoadJob>(std::move(request),
        [owner = std::move(owner)](Graphics::TextureResource* resource) noexcept {
            if (resource)
                owner->Apply_New_Surface(resource, true);
            else
                owner->Set_Render_Backend_Texture(nullptr);
        });
}

void W3DTextureHandle::Configure_File_Load()
{
    if (!m_load_state)
        return;

    std::weak_ptr<LoadState> weak_state = m_load_state;
    auto source = std::make_shared<const Graphics::ResourceLoadSource>(
        [weak_state]() -> std::unique_ptr<Graphics::ResourceLoadJob> {
            const auto state = weak_state.lock();
            if (!state)
                return {};
            std::lock_guard lock(state->mutex);
            if (!state->owner || state->owner->Is_Initialized())
                return {};
            state->owner->Add_Ref();
            RefCountPtr<W3DTextureHandle> owner = RefCountPtr<W3DTextureHandle>::Create_No_Add_Ref(
                state->owner);
            return state->owner->Make_Load_Job(std::move(owner));
        });
    m_residency.Set_Load_Source(std::move(source));
    m_residency.Set_Initialize_Callback([this] {
        if (m_residency.Is_Initialized() || !m_residency.Load_Source())
            return;
        auto& queue = Graphics::Get_Resource_Load_Queue();
        const auto& source = m_residency.Load_Source();
        if (!m_residency.Resource())
            queue.Request(source, Graphics::ResourceLoadPriority::Immediate);
        if (!m_residency.Is_Initialized())
            queue.Request(source, Graphics::ResourceLoadPriority::Background);
    });

    if (Graphics::Get_Resource_Load_Queue().Is_Owner_Thread())
        Init();
}

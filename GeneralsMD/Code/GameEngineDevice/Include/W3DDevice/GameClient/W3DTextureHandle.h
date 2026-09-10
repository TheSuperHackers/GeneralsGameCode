#pragma once

#include <cstdint>
#include <memory>

#include "WWLib/always.h"
#include "WWLib/refcount.h"
#include "WWLib/ref_ptr.h"
#include "WWLib/wwstring.h"
#include "WWMath/vector3.h"
import Assets.Images.Buffer;
import Assets.Images.PixelEncoding;
import Graphics.RHI;
import Graphics.Resources.Textures.Edit;
import Graphics.Resources.Textures.Residency;
import Graphics.Resources.Textures.Sampling;

enum MipCountType
{
    MIP_LEVELS_ALL = 0,
    MIP_LEVELS_1,
    MIP_LEVELS_2,
    MIP_LEVELS_3,
    MIP_LEVELS_4,
    MIP_LEVELS_5,
    MIP_LEVELS_6,
    MIP_LEVELS_7,
    MIP_LEVELS_8,
    MIP_LEVELS_10,
    MIP_LEVELS_11,
    MIP_LEVELS_12,
    MIP_LEVELS_MAX
};

class W3DTextureHandle : public RefCountClass
{
    W3DMPO_CODE(W3DTextureHandle)

public:
    enum PoolType
    {
        POOL_DEFAULT = 0,
        POOL_MANAGED,
        POOL_SYSTEMMEM
    };

    enum TexAssetType
    {
        TEX_REGULAR,
        TEX_CUBEMAP,
        TEX_VOLUME,
        TEX_DEPTH
    };

    W3DTextureHandle(
        unsigned width,
        unsigned height,
        Assets::PixelEncoding format,
        MipCountType mip_level_count = MIP_LEVELS_ALL,
        PoolType pool = POOL_MANAGED,
        bool render_target = false,
        bool allow_reduction = true,
        Graphics::TextureResidencyClock clock = {});

    // The asset type is explicit for callers replacing the historical cube
    // and volume subclasses. A volume uses the supplied depth; regular and
    // cube resources ignore it.
    W3DTextureHandle(
        unsigned width,
        unsigned height,
        Assets::PixelEncoding format,
        MipCountType mip_level_count,
        PoolType pool,
        bool render_target,
        bool allow_reduction,
        TexAssetType asset_type,
        unsigned depth = 1,
        Graphics::TextureResidencyClock clock = {});

    W3DTextureHandle(
        const char* name,
        const char* full_path = nullptr,
        MipCountType mip_level_count = MIP_LEVELS_ALL,
        Assets::PixelEncoding texture_format = Assets::PixelEncoding::Unknown,
        bool allow_compression = true,
        bool allow_reduction = true,
        TexAssetType asset_type = TEX_REGULAR,
        Graphics::TextureResidencyClock clock = {});

    W3DTextureHandle(
        Graphics::TextureEdit* surface,
        MipCountType mip_level_count = MIP_LEVELS_ALL,
        Graphics::TextureResidencyClock clock = {});

    explicit W3DTextureHandle(
        Graphics::TextureResource* texture,
        Graphics::TextureResidencyClock clock = {});

    W3DTextureHandle(
        unsigned width,
        unsigned height,
        Graphics::RHITextureFormat depth_format,
        MipCountType mip_level_count = MIP_LEVELS_ALL,
        PoolType pool = POOL_MANAGED,
        Graphics::TextureResidencyClock clock = {});

    ~W3DTextureHandle() override;

    W3DTextureHandle(const W3DTextureHandle&) = delete;
    W3DTextureHandle& operator=(const W3DTextureHandle&) = delete;

    TexAssetType Get_Asset_Type() const noexcept { return m_asset_type; }

    void Init();
    bool Ensure_Render_Backend_Texture();
    void Invalidate() noexcept;

    Graphics::TextureResource* Peek_Render_Backend_Texture() const noexcept
    {
        return m_residency.Resource();
    }

    Graphics::RHITextureHandle Peek_Graphics_Texture() const noexcept
    {
        return m_residency.Handle();
    }

    // Takes ownership of texture and releases the currently published
    // generation.
    void Set_Render_Backend_Texture(Graphics::TextureResource* texture) noexcept
    {
        m_residency.Set_Resource(texture);
    }

    void Apply_New_Surface(Graphics::TextureResource* texture, bool initialized,
        bool disable_auto_invalidation = false) noexcept;

    const StringClass& Get_Texture_Name() const noexcept { return m_name; }
    const StringClass& Get_Full_Path() const noexcept
    {
        return m_full_path.Is_Empty() ? m_name : m_full_path;
    }
    void Set_Texture_Name(const char* name) { m_name = name; }
    void Set_Full_Path(const char* path) { m_full_path = path; }

    unsigned Get_ID() const noexcept { return m_texture_id; }
    MipCountType Get_Mip_Level_Count() const noexcept { return m_mip_level_count; }
    int Get_Width() const noexcept { return m_width; }
    int Get_Height() const noexcept { return m_height; }
    unsigned Get_Depth() const noexcept { return m_depth; }

    void Set_Inactivation_Time(unsigned milliseconds) noexcept
    {
        m_residency.Set_Inactivation_Time(milliseconds);
    }
    unsigned Get_Inactivation_Time() const noexcept
    {
        return m_residency.Inactivation_Time();
    }

    bool Is_Initialized() const noexcept { return m_residency.Is_Initialized(); }
    bool Is_Procedural() const noexcept { return m_residency.Is_Procedural(); }
    bool Is_Reducible() const noexcept { return m_reducible; }
    bool Is_Lightmap() const noexcept { return m_is_lightmap; }
    bool Is_Compression_Allowed() const noexcept { return m_allow_compression; }
    PoolType Get_Pool() const noexcept { return m_pool; }

    bool Is_Missing_Texture() const noexcept;

    bool Is_Dirty() const noexcept { return m_dirty; }
    void Set_Dirty() noexcept { m_dirty = true; }
    void Clean() noexcept { m_dirty = false; }

    void Set_HSV_Shift(const Vector3& hsv_shift) noexcept;
    const Vector3& Get_HSV_Shift() const noexcept { return m_hsv_shift; }

    const std::shared_ptr<const Graphics::ResourceLoadSource>& Loading_Source() const noexcept
    {
        return m_residency.Load_Source();
    }

    Graphics::TextureEdit* Get_Surface_Level(unsigned level = 0);
    void Get_Level_Description(Assets::ImageDescription& description, unsigned level = 0);
    unsigned Get_Texture_Memory_Usage() const noexcept;

    Graphics::TextureSampling& Get_Sampling() noexcept { return m_sampling; }
    const Graphics::TextureSampling& Get_Sampling() const noexcept { return m_sampling; }
    Assets::PixelEncoding Get_Texture_Format() const noexcept { return m_texture_format; }
    Graphics::RHITextureFormat Get_Depth_Texture_Format() const noexcept
    {
        return m_texture_description.format;
    }

    Graphics::TextureResidency& Residency() noexcept { return m_residency; }
    const Graphics::TextureResidency& Residency() const noexcept { return m_residency; }

protected:
    virtual bool Recreate_Procedural_Texture();
    void Set_Procedural_Texture_Recreation_Enabled(bool enabled) noexcept
    {
        m_procedural_recreation_enabled = enabled;
    }

    Graphics::TextureResource* Create_Procedural_Resource() const noexcept;
    void Configure_Procedural_Recreation(bool register_for_recreation);
    void Configure_File_Load();

private:
    struct LoadState;

    std::unique_ptr<Graphics::ResourceLoadJob> Make_Load_Job(
        RefCountPtr<W3DTextureHandle> owner) const;
    void Detach_Load_State() noexcept;

    static unsigned Mip_Count(MipCountType count) noexcept;
    static unsigned Texture_Level_Size(const Graphics::TextureResource& texture,
        unsigned level) noexcept;

    Graphics::TextureResidency m_residency;
    Graphics::TextureSampling m_sampling;
    Graphics::RHITexture m_texture_description{};
    Assets::PixelEncoding m_texture_format = Assets::PixelEncoding::Unknown;
    Vector3 m_hsv_shift{};
    StringClass m_name;
    StringClass m_full_path;
    std::shared_ptr<LoadState> m_load_state;
    MipCountType m_mip_level_count = MIP_LEVELS_ALL;
    PoolType m_pool = POOL_MANAGED;
    TexAssetType m_asset_type = TEX_REGULAR;
    unsigned m_depth = 1;
    unsigned m_texture_id = 0;
    int m_width = 0;
    int m_height = 0;
    bool m_allow_compression = false;
    bool m_reducible = true;
    bool m_is_lightmap = false;
    bool m_dirty = false;
    bool m_render_target = false;
    bool m_procedural_recreation_enabled = false;
};

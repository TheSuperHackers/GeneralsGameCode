export module Graphics.Resources.Textures.Quality;

namespace Graphics
{
// Rendering preferences, sampled when a texture loading request is created.
// Existing resources change only when their owner explicitly invalidates them.
export struct TextureQualitySettings final
{
    int mip_reduction = 0;
    int minimum_dimension = 1;
    bool prefer_16_bits = true;
};

export TextureQualitySettings& Get_Texture_Quality_Settings() noexcept
{
    static TextureQualitySettings settings;
    return settings;
}
}

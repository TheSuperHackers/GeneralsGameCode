module;
#include <array>
export module Graphics.Scene.Props.Material;
export import Graphics.Scene.Props.Geometry;
namespace Graphics
{
export enum class PropColorSource { Material, PrimaryColor, SecondaryColor };
export struct PropMaterial final
{
    std::array<float,3> ambient{1,1,1};
    std::array<float,3> diffuse{1,1,1};
    std::array<float,3> emissive{};
    std::array<float,3> specular{};
    float opacity = 1;
    float shininess = 1;
    bool lighting = false;
    PropColorSource ambient_source = PropColorSource::Material;
    PropColorSource diffuse_source = PropColorSource::Material;
    PropColorSource emissive_source = PropColorSource::Material;
};

inline std::array<float,3> Select_Prop_Color(const PropVertex& vertex,
    PropColorSource source,const std::array<float,3>& material) noexcept
{
    if (source == PropColorSource::PrimaryColor)
        return {vertex.color[0],vertex.color[1],vertex.color[2]};
    if (source == PropColorSource::SecondaryColor)
        return {vertex.secondary_color[0],vertex.secondary_color[1],vertex.secondary_color[2]};
    return material;
}

export inline void Apply_Prop_Material(PropVertex& vertex,const PropMaterial& material) noexcept
{
    // Prelit geometry modulates its vertex color in the shader. Material color
    // source selection applies to lit diffuse, so do not apply that color twice.
    const auto diffuse = material.lighting
        ? Select_Prop_Color(vertex,material.diffuse_source,material.diffuse) : material.diffuse;
    float opacity = material.opacity;
    if (material.lighting && material.diffuse_source == PropColorSource::PrimaryColor)
        opacity = vertex.color[3];
    if (material.lighting && material.diffuse_source == PropColorSource::SecondaryColor)
        opacity = vertex.secondary_color[3];
    const auto ambient = Select_Prop_Color(vertex,material.ambient_source,material.ambient);
    const auto emissive = Select_Prop_Color(vertex,material.emissive_source,material.emissive);
    vertex.material_diffuse = {diffuse[0],diffuse[1],diffuse[2],opacity};
    vertex.material_ambient = {ambient[0],ambient[1],ambient[2],material.lighting ? 1.0f : 0.0f};
    vertex.material_emissive = {emissive[0],emissive[1],emissive[2],0};
    vertex.material_specular = {material.specular[0],material.specular[1],material.specular[2],material.shininess};
}
}

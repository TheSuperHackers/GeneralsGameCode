module;
#include <cmath>
export module Graphics.Scene.Props.LightingParameters;
import Graphics.Scene.Lighting.Local;
import Graphics.Scene.Props.Renderer;

namespace Graphics
{
export void Set_Prop_Lighting(PropParameters& parameters, const LocalLighting* lighting)
{
    if (!lighting) return;
    parameters.light_direction={}; parameters.light_diffuse={}; parameters.light_specular={};
    parameters.light_position={}; parameters.light_attenuation={};
    parameters.light_ambient={}; parameters.light_spot={};
    const auto& ambient = lighting->ambient;
    parameters.scene_ambient={ambient[0],ambient[1],ambient[2],0};
    for (std::size_t i=0;i<lighting->count;++i) {
        const auto& light = lighting->lights[i];
        const auto& diffuse = light.point ? light.source_diffuse : light.diffuse;
        parameters.light_direction[i]={light.direction[0],light.direction[1],light.direction[2],1};
        parameters.light_diffuse[i]={diffuse[0],diffuse[1],diffuse[2],0};
        if (light.point) {
            parameters.light_position[i]={light.position[0],light.position[1],light.position[2],1};
            parameters.light_ambient[i]={light.source_ambient[0],light.source_ambient[1],light.source_ambient[2],0};
            const float inner=light.inner_radius, outer=light.outer_radius;
            parameters.light_attenuation[i]={1,
                std::abs(inner-outer)<0.00001f || inner<=0.00001f ? 0 : 0.1f/inner,
                outer>0.00001f ? 8/(outer*outer) : 0,outer};
        } else if (i==0) parameters.light_specular[i]={1,1,1,0};
    }
}
}

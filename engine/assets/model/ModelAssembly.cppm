module;
#include <cmath>
#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>
export module Assets.ModelAssembly;
export import Assets.ModelRig;
export import Assets.Math;
namespace Assets {
export struct ModelAssemblyLevel final {
    float maximum_screen_size=std::numeric_limits<float>::max();
    std::vector<ModelAttachmentDesc> children;
};
export struct ModelDetailLevelDesc final {
    std::string name;
    float minimum_distance=0;
    float maximum_distance=0;
};
export struct ModelLevelSetDesc final {
    std::string name;
    // Highest detail first, matching authored dependency order.
    std::vector<ModelDetailLevelDesc> levels;
};
export struct ModelNamedAttachmentDesc final {
    std::string model_name;
    std::string bone_name;
};
export struct ModelAggregateDesc final {
    std::string name;
    std::string base_model;
    std::vector<ModelNamedAttachmentDesc> attachments;
    bool match_detail_levels=false;
};
// Assembly descriptions identify child assets and their attachment points.
// They contain no instantiated objects, mutable poses, or GPU resources.
export struct ModelAssemblyDesc final {
    std::string name;
    std::string skeleton_name;
    std::vector<ModelAssemblyLevel> levels;
    std::vector<ModelAttachmentDesc> aggregates;
    std::vector<ModelAttachmentDesc> proxies;
    std::vector<Vector3f> snap_points;
};
export struct ModelProxyDesc final {
    std::string name;
    // Row-major affine matrix, including translation in each row's last entry.
    std::array<float,12> transform{1,0,0,0, 0,1,0,0, 0,0,1,0};
};
export struct ModelCollectionDesc final {
    std::string name;
    std::vector<std::string> children;
    std::vector<ModelProxyDesc> proxies;
    std::vector<Vector3f> snap_points;
};
export bool Validate_Model_Collection(const ModelCollectionDesc& description,std::string& error) {
    const auto limit=std::size_t(std::numeric_limits<int>::max());
    const auto fail=[&](const char* message) { error=message;return false; };
    if(description.name.empty() || description.children.size()>limit ||
        description.proxies.size()>limit || description.snap_points.size()>limit)
        return fail("invalid model collection identity or dimensions");
    for(const auto& child:description.children)
        if(child.empty())return fail("empty model collection child identity");
    for(const auto& proxy:description.proxies) {
        if(proxy.name.empty())return fail("empty model proxy identity");
        for(float value:proxy.transform)
            if(!std::isfinite(value))return fail("nonfinite model proxy transform");
    }
    for(const auto& point:description.snap_points)
        if(!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
            return fail("nonfinite model collection snap point");
    error.clear();return true;
}
export bool Validate_Model_Assembly(const ModelAssemblyDesc& description,std::string& error) {
    const auto limit=std::size_t(std::numeric_limits<int>::max());
    const auto fail=[&](const char* message) { error=message;return false; };
    if(description.name.empty() || description.levels.empty() || description.levels.size()>limit)
        return fail("invalid model assembly identity or level count");
    const auto valid_children=[&](const auto& children) {
        if(children.size()>limit)return false;
        for(const auto& child:children)
            if(child.object_name.empty() || child.bone>limit)return false;
        return true;
    };
    for(const auto& level:description.levels) {
        if(!std::isfinite(level.maximum_screen_size) || level.maximum_screen_size<0 || !valid_children(level.children))
            return fail("invalid model assembly level");
    }
    if(!valid_children(description.aggregates) || !valid_children(description.proxies))
        return fail("invalid model assembly attachment");
    for(const auto& point:description.snap_points)
        if(!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
            return fail("nonfinite model snap point");
    error.clear();return true;
}
}

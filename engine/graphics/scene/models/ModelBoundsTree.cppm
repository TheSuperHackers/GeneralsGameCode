module;
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>
export module Graphics.Scene.Models.BoundsTree;
import Assets.Math;
import Assets.MeshBoundsTree;

namespace Graphics {
// Runtime copies own independent bounds. Construction and format decoding are
// asset responsibilities; query-specific intersection math belongs to callers.
export class ModelBoundsTree final
{
public:
    ModelBoundsTree() = default;
    explicit ModelBoundsTree(Assets::MeshBoundsTree source) : tree(std::move(source)) {}

    void Scale(float scale)
    {
        for(auto& node:tree.nodes) {
            node.bounds.minimum.x*=scale;node.bounds.minimum.y*=scale;node.bounds.minimum.z*=scale;
            node.bounds.maximum.x*=scale;node.bounds.maximum.y*=scale;node.bounds.maximum.z*=scale;
        }
    }

    // Visit front before back, culling each node when reached. A leaf callback
    // may finish its own query early, but cannot suppress visits to other leaves.
    // Callbacks must not replace or mutate this tree during traversal.
    template<class Cull,class Leaf>
    void Visit(Cull&& cull,Leaf&& leaf) const
    {
        if(!tree.nodes.empty()) Visit_Node(0,cull,leaf);
    }

    template<class Cull,class Intersect,class StartBad,class Surface>
    bool Cast(Cull&& cull,Intersect&& intersect,StartBad&& start_bad,Surface&& surface) const
    {
        bool hit=false;
        Visit(cull,[&](std::span<const std::uint32_t> polygons) {
            bool leaf_hit=false;
            std::uint32_t last_hit=0;
            for(const auto polygon:polygons) {
                if(intersect(polygon)) { leaf_hit=true;last_hit=polygon; }
                if(start_bad()) { hit=true;return; }
            }
            if(leaf_hit) { surface(last_hit);hit=true; }
        });
        return hit;
    }

    template<class Cull,class Intersect>
    bool Intersects(Cull&& cull,Intersect&& intersect) const
    {
        bool hit=false;
        Visit(cull,[&](std::span<const std::uint32_t> polygons) {
            for(const auto polygon:polygons)
                if(intersect(polygon)) { hit=true;return; }
        });
        return hit;
    }

private:
    template<class Cull,class Leaf>
    void Visit_Node(std::uint32_t index,Cull& cull,Leaf& leaf) const
    {
        const auto& node=tree.nodes[index];
        if(cull(node.bounds)) return;
        if(node.leaf) {
            leaf(std::span<const std::uint32_t>(tree.polygon_indices).subspan(node.first,node.second));
            return;
        }
        Visit_Node(node.first,cull,leaf);
        Visit_Node(node.second,cull,leaf);
    }

    Assets::MeshBoundsTree tree;
};
}

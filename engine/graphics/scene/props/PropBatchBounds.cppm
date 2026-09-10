module;
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
export module Graphics.Scene.Props.BatchBounds;
import Graphics.RHI;
import Graphics.Scene.Props.Geometry;

namespace Graphics {
// Unknown bounds overlap everything. This is a proof of disjoint raster
// coverage for batching, rather than a visibility or clipping decision.
export struct PropBatchBounds final {
    std::array<double,2> minimum{},maximum{};
    bool known=false;
    bool Disjoint(const PropBatchBounds& other) const noexcept {
        return known && other.known && (maximum[0]<other.minimum[0] || other.maximum[0]<minimum[0]
            || maximum[1]<other.minimum[1] || other.maximum[1]<minimum[1]);
    }
    void Include(const PropBatchBounds& other) noexcept {
        known=known && other.known;
        if (!known) return;
        for (unsigned axis=0; axis<2; ++axis) {
            minimum[axis]=(std::min)(minimum[axis],other.minimum[axis]);
            maximum[axis]=(std::max)(maximum[axis],other.maximum[axis]);
        }
    }
};

namespace BatchBoundsDetail {
struct Interval { double low,high; };
constexpr double roundoff=8.0*std::numeric_limits<float>::epsilon();
constexpr double smallest=std::numeric_limits<float>::min();

// Interval arithmetic includes intermediate float rounding and flush-to-zero
// error in each four-term shader dot product. Overflow is deliberately unknown.
bool Transform(const std::array<float,16>& matrix,const std::array<Interval,4>& input,
    std::array<Interval,4>& output,unsigned rows) noexcept {
    for (unsigned row=0; row<rows; ++row) {
        double low=0,high=0,magnitude=0;
        for (unsigned column=0; column<4; ++column) {
            const double coefficient=matrix[row*4+column];
            if (!std::isfinite(coefficient) || (coefficient!=0 && std::abs(coefficient)<smallest)) return false;
            const double a=coefficient*input[column].low,b=coefficient*input[column].high;
            low+=(std::min)(a,b); high+=(std::max)(a,b);
            magnitude+=(std::max)(std::abs(a),std::abs(b));
        }
        const double error=roundoff*magnitude+8*smallest;
        if (!std::isfinite(magnitude) || magnitude+error>(std::numeric_limits<float>::max)()) return false;
        output[row]={low-error,high+error};
    }
    return true;
}
}

export PropBatchBounds Project_Prop_Batch_Bounds(const PropGeometry& geometry,
    const std::array<float,16>& world,const std::array<float,16>& view_projection,
    const RHIViewport& viewport) noexcept {
    using namespace BatchBoundsDetail;
    if (viewport.width==0 || viewport.height==0) return {};
    std::array<Interval,4> local{},world_bounds{},clip{};
    const auto& minimum=geometry.Minimum_Position();
    const auto& maximum=geometry.Maximum_Position();
    for (unsigned axis=0; axis<3; ++axis) {
        if (!std::isfinite(minimum[axis]) || !std::isfinite(maximum[axis])) return {};
        // Source floats may be flushed when consumed by a shader instruction.
        local[axis]={double(minimum[axis])-smallest,double(maximum[axis])+smallest};
    }
    local[3]={1,1};
    if (!Transform(world,local,world_bounds,3)) return {};
    world_bounds[3]={1,1};
    if (!Transform(view_projection,world_bounds,clip,4) || clip[3].low<=0) return {};
    PropBatchBounds result;
    result.known=true;
    for (unsigned axis=0; axis<2; ++axis) {
        const std::array ratios{clip[axis].low/clip[3].low,clip[axis].low/clip[3].high,
            clip[axis].high/clip[3].low,clip[axis].high/clip[3].high};
        const double extent=axis==0 ? viewport.width : viewport.height;
        const double origin=axis==0 ? viewport.x : viewport.y;
        // One pixel covers raster quantization and clipping boundaries. Also
        // include float viewport-transform error for large origins/extents.
        const double padding=2.0/extent+roundoff*(origin+extent+1)*2.0/extent;
        result.minimum[axis]=*std::min_element(ratios.begin(),ratios.end())-padding;
        result.maximum[axis]=*std::max_element(ratios.begin(),ratios.end())+padding;
        if (!std::isfinite(result.minimum[axis]) || !std::isfinite(result.maximum[axis])) return {};
    }
    return result;
}
}

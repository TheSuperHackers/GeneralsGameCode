module;
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
export module Graphics.Materials.TextureMapping;
import Assets.Math;
import Assets.Materials.TextureMapping;
import Graphics.Materials.TextureCoordinates;
import Graphics.Materials.TextureProjection;

namespace Graphics {
namespace {
constexpr float mapping_pi=3.14159265358979323846f;
float Bounded_Offset(float value,float scale,bool clamp) noexcept
{
    if (!clamp) return value-std::floor(value);
    // Retain the ordered bounds checks even for a negative authored scale.
    if (value < -scale) return -scale;
    if (value > scale) return scale;
    return value;
}
}

export struct TextureMappingResult final {
    std::array<float,16> transform=Make_Affine_Texture_Transform({1,0,0,0,1,0});
    TextureCoordinateMode coordinates{};
    std::optional<std::array<float,4>> bump;
};

// Exposed only for a linear scroll, whose rate/offset may be controlled by
// vehicle treads or temporarily overridden while extracting a mesh material.
export struct TextureScrollState final {
    Assets::TextureScrollMapping parameters;
    Assets::Vector2f offset{};
    Assets::Vector2f rate_per_millisecond{};
    std::uint32_t last_time=0;

    explicit TextureScrollState(const Assets::TextureScrollMapping& description={},std::uint32_t now=0)
        : parameters(description),offset(description.start_offset),
          rate_per_millisecond{description.rate_per_second.x*-0.001f,description.rate_per_second.y*-0.001f},last_time(now) {}
    void Advance(std::uint32_t now) noexcept
    {
        const float delta=static_cast<float>(now-last_time);
        last_time=now;
        offset.x=Bounded_Offset(offset.x+rate_per_millisecond.x*delta,parameters.scale.x,parameters.clamp);
        offset.y=Bounded_Offset(offset.y+rate_per_millisecond.y*delta,parameters.scale.y,parameters.clamp);
    }
    void Reset(std::uint32_t now) noexcept { last_time=now; offset={}; }
};

namespace {
struct GridState final {
    Assets::TextureGridMapping parameters;
    std::uint32_t last_time=0,remainder=0,frame=0,width=1,frame_milliseconds=1;
    int direction=0;
    explicit GridState(Assets::TextureGridMapping description,std::uint32_t now)
        : parameters(description),last_time(now)
    {
        if (parameters.width_log2>=16 || !std::isfinite(parameters.frames_per_second)
            || std::fabs(parameters.frames_per_second)>1000.0f)
            throw std::invalid_argument("Invalid texture grid dimensions or frame rate");
        width=1u<<parameters.width_log2;
        if (!parameters.last_frame) parameters.last_frame=width*width;
        parameters.start_frame%=parameters.last_frame;
        const float fps=parameters.frames_per_second;
        direction=fps<0 ? -1 : fps>0 ? 1 : 0;
        if (direction) frame_milliseconds=static_cast<std::uint32_t>(1000.0f/std::fabs(fps));
        Reset(now);
    }
    void Reset(std::uint32_t now) noexcept {
        last_time=now; remainder=0;
        frame=direction<0 ? parameters.last_frame-1-parameters.start_frame : parameters.start_frame;
    }
    Assets::Vector2f Advance(std::uint32_t now) noexcept {
        remainder+=now-last_time;
        last_time=now;
        const auto steps=remainder/frame_milliseconds;
        // The original unsigned modulus is observable with negative FPS and
        // a last-frame count that is not a power of two.
        frame=(frame+steps*static_cast<std::uint32_t>(direction))%parameters.last_frame;
        remainder%=frame_milliseconds;
        const float inverse_width=1.0f/static_cast<float>(width);
        return {static_cast<float>(frame&(width-1))*inverse_width,
            static_cast<float>(frame>>parameters.width_log2)*inverse_width};
    }
};
template<class Parameters> struct TimedState final {
    Parameters parameters;
    std::uint32_t last_time=0;
    float phase=0;
    Assets::Vector2f offset{};
    float Delta(std::uint32_t now) noexcept {
        const float result=static_cast<float>(now-last_time); last_time=now; return result;
    }
    void Reset(std::uint32_t now) noexcept { last_time=now; phase=0; offset={}; }
};
struct RandomState final {
    Assets::TextureRandomMapping parameters;
    float (*sample)()=nullptr;
    std::uint32_t last_time=0;
    float remainder=0,angle=0;
    Assets::Vector2f center{};
    void Randomize() {
        angle=2*mapping_pi*sample(); center.x=sample(); center.y=sample();
    }
    void Reset(std::uint32_t now) noexcept { last_time=now; remainder=0; }
};
struct BumpState final {
    TextureScrollState scroll;
    float radians_per_second=0,scale=1,angle=0;
    std::uint32_t last_time=0;
    float (*sine)(float)=nullptr;
    float (*cosine)(float)=nullptr;
};
using MappingState=std::variant<Assets::TextureScaleMapping,TextureScrollState,
    Assets::TextureEnvironmentMapping,GridState,TimedState<Assets::TextureRotateMapping>,
    TimedState<Assets::TextureSineMapping>,TimedState<Assets::TextureStepMapping>,
    TimedState<Assets::TextureZigZagMapping>,TimedState<Assets::TextureEdgeMapping>,
    RandomState,BumpState,TextureProjectionState>;

TextureMappingResult Environment(const Assets::TextureEnvironmentMapping& parameters,
    const std::array<float,16>& view,float scale=0.5f,Assets::Vector2f offset={}) noexcept
{
    TextureMappingResult result;
    result.coordinates.source=parameters.source==Assets::TextureEnvironmentSource::Reflection
        ? TextureCoordinateSource::CameraReflection : TextureCoordinateSource::CameraNormal;
    auto& matrix=result.transform;
    matrix[0]=matrix[5]=0;
    const auto axis=parameters.world_space ? parameters.axis : Assets::TextureMappingAxis::Z;
    matrix[axis==Assets::TextureMappingAxis::X ? 1 : 0]=scale;
    matrix[axis==Assets::TextureMappingAxis::Z ? 5 : 6]=scale;
    matrix[3]=offset.x+scale; matrix[7]=offset.y+scale;
    if (parameters.world_space) {
        const auto source=matrix;
        for (unsigned row=0;row<3;++row)
            for (unsigned column=0;column<3;++column)
                matrix[row*4+column]=source[row*4]*view[column*4]
                    +source[row*4+1]*view[column*4+1]+source[row*4+2]*view[column*4+2];
    }
    return result;
}
TextureMappingResult Scroll(TextureScrollState& state,std::uint32_t now,
    const std::array<float,16>& projection) noexcept
{
    state.Advance(now);
    TextureMappingResult result;
    const auto scale=state.parameters.scale;
    if (state.parameters.screen_projection) {
        result.transform=projection;
        for (unsigned column=0;column<4;++column) {
            result.transform[column]=projection[column]*scale.x+projection[12+column]*state.offset.x;
            result.transform[4+column]=projection[4+column]*scale.y+projection[12+column]*state.offset.y;
        }
        result.coordinates={TextureCoordinateSource::CameraPosition,true};
    } else result.transform=Make_Affine_Texture_Transform({scale.x,0,state.offset.x,0,scale.y,state.offset.y});
    return result;
}
}

// Material slots share ownership; a material copy explicitly clones animation
// state. Evaluation allocates nothing and consumes only the supplied clock,
// camera matrices, and the authored random mapping's sampling callback.
export class TextureMapping final {
public:
    static std::shared_ptr<TextureMapping> Create(const Assets::TextureMappingDescription& description,
        std::uint32_t now,float (*random_sample)()=nullptr,
        float (*bump_sine)(float)=nullptr,float (*bump_cosine)(float)=nullptr)
    {
        auto state=std::visit([&](const auto& parameters)->MappingState {
            using P=std::decay_t<decltype(parameters)>;
            if constexpr (std::is_same_v<P,Assets::TextureScaleMapping>
                || std::is_same_v<P,Assets::TextureEnvironmentMapping>) return parameters;
            else if constexpr (std::is_same_v<P,Assets::TextureScrollMapping>) return TextureScrollState{parameters,now};
            else if constexpr (std::is_same_v<P,Assets::TextureGridMapping>) return GridState{parameters,now};
            else if constexpr (std::is_same_v<P,Assets::TextureRandomMapping>) {
                if (!random_sample) throw std::invalid_argument("Random texture mapping requires a sampling source");
                RandomState result{parameters,random_sample,now}; result.Randomize(); return result;
            } else if constexpr (std::is_same_v<P,Assets::TextureBumpMapping>) {
                if (!bump_sine || !bump_cosine) throw std::invalid_argument("Bump texture mapping requires trigonometric functions");
                return BumpState{TextureScrollState{parameters.scroll,now},2*mapping_pi*parameters.turns_per_second,
                    parameters.scale,0,now,bump_sine,bump_cosine};
            } else {
                TimedState<P> result{parameters,now};
                if constexpr (std::is_same_v<P,Assets::TextureEdgeMapping>) result.phase=parameters.start_offset;
                return result;
            }
        },description);
        return std::shared_ptr<TextureMapping>(new TextureMapping(std::move(state)));
    }
    static std::shared_ptr<TextureMapping> Create_Projection()
    {
        return std::shared_ptr<TextureMapping>(new TextureMapping(TextureProjectionState{}));
    }
    std::shared_ptr<TextureMapping> Clone(std::uint32_t now) const
    {
        auto result=std::shared_ptr<TextureMapping>(new TextureMapping(m_state));
        std::visit([now](auto& state) {
            using S=std::decay_t<decltype(state)>;
            if constexpr (std::is_same_v<S,TextureScrollState>) {
                state.offset=state.parameters.start_offset; state.last_time=now;
            } else if constexpr (std::is_same_v<S,BumpState>) {
                state.scroll.offset=state.scroll.parameters.start_offset; state.scroll.last_time=now;
                state.angle=0; state.last_time=now;
            } else if constexpr (std::is_same_v<S,RandomState>) { state.Reset(now); state.Randomize(); }
            else if constexpr (std::is_same_v<S,TimedState<Assets::TextureEdgeMapping>>) state.last_time=now;
            else if constexpr (requires { state.Reset(now); }) state.Reset(now);
        },result->m_state);
        return result;
    }
    void Reset(std::uint32_t now) noexcept
    {
        std::visit([now](auto& state) {
            using S=std::decay_t<decltype(state)>;
            // Resetting scroll does not restart the independent bump clock.
            if constexpr (std::is_same_v<S,BumpState>) state.scroll.Reset(now);
            else if constexpr (requires { state.Reset(now); }) state.Reset(now);
        },m_state);
    }
    TextureScrollState* Linear_Scroll() noexcept {
        auto* state=std::get_if<TextureScrollState>(&m_state);
        return state && !state->parameters.screen_projection ? state : nullptr;
    }
    TextureProjectionState* Projection() noexcept { return std::get_if<TextureProjectionState>(&m_state); }
    const TextureProjectionState* Projection() const noexcept { return std::get_if<TextureProjectionState>(&m_state); }
    bool Is_Time_Variant() const noexcept {
        return !std::holds_alternative<Assets::TextureScaleMapping>(m_state)
            && !std::holds_alternative<Assets::TextureEnvironmentMapping>(m_state)
            && !std::holds_alternative<TextureProjectionState>(m_state);
    }
    bool Needs_Normals() const noexcept {
        if (const auto* grid=std::get_if<GridState>(&m_state))
            return grid->parameters.environment.source!=Assets::TextureEnvironmentSource::None;
        return std::holds_alternative<Assets::TextureEnvironmentMapping>(m_state)
            || std::holds_alternative<TimedState<Assets::TextureEdgeMapping>>(m_state);
    }
    TextureMappingResult Evaluate(std::uint32_t now,
        const std::array<float,16>& view=Make_Affine_Texture_Transform({1,0,0,0,1,0}),
        const std::array<float,16>& projection=Make_Affine_Texture_Transform({1,0,0,0,1,0}))
    {
        return std::visit([&](auto& state)->TextureMappingResult {
            using S=std::decay_t<decltype(state)>;
            TextureMappingResult result;
            if constexpr (std::is_same_v<S,Assets::TextureScaleMapping>)
                result.transform=Make_Affine_Texture_Transform({state.scale.x,0,0,0,state.scale.y,0});
            else if constexpr (std::is_same_v<S,TextureScrollState>) return Scroll(state,now,projection);
            else if constexpr (std::is_same_v<S,Assets::TextureEnvironmentMapping>) return Environment(state,view);
            else if constexpr (std::is_same_v<S,TextureProjectionState>) {
                result.transform=state.Transform(); result.coordinates=state.Coordinates();
            } else if constexpr (std::is_same_v<S,GridState>) {
                const auto offset=state.Advance(now);
                if (state.parameters.environment.source!=Assets::TextureEnvironmentSource::None)
                    return Environment(state.parameters.environment,view,0.5f/static_cast<float>(state.width),offset);
                result.transform=Make_Affine_Texture_Transform({1,0,offset.x,0,1,offset.y});
            } else if constexpr (std::is_same_v<S,BumpState>) {
                result=Scroll(state.scroll,now,projection);
                state.angle+=state.radians_per_second*static_cast<float>(now-state.last_time)*0.001f;
                state.last_time=now;
                state.angle=std::fmod(state.angle,2*mapping_pi);
                const float c=state.scale*state.cosine(state.angle),s=state.scale*state.sine(state.angle);
                result.bump=std::array<float,4>{c,-s,s,c};
            } else if constexpr (std::is_same_v<S,RandomState>) {
                state.remainder+=static_cast<float>(now-state.last_time); state.last_time=now;
                const float fpms=state.parameters.frames_per_second/1000.0f;
                if (fpms!=0) {
                    const int frames=static_cast<int>(state.remainder*fpms);
                    if (frames!=0) { state.Randomize(); state.remainder-=frames/fpms; }
                }
                const float c=std::cos(state.angle),s=std::sin(state.angle);
                const auto& p=state.parameters;
                result.transform=Make_Affine_Texture_Transform({c*p.scale.x,-s*p.scale.y,
                    std::fmod(state.center.x+state.remainder*(p.rate_per_second.x/1000.0f),1.0f),
                    s*p.scale.x,c*p.scale.y,
                    std::fmod(state.center.y+state.remainder*(p.rate_per_second.y/1000.0f),1.0f)});
            } else {
                const float delta=state.Delta(now);
                const auto& p=state.parameters;
                if constexpr (std::is_same_v<S,TimedState<Assets::TextureRotateMapping>>) {
                    state.phase+=delta*(2*mapping_pi*p.turns_per_second/1000.0f);
                    state.phase=std::fmod(state.phase,2*mapping_pi);
                    if (state.phase<0) state.phase+=2*mapping_pi;
                    const float c=std::cos(state.phase),s=std::sin(state.phase);
                    result.transform=Make_Affine_Texture_Transform({p.scale.x*c,-p.scale.x*s,
                        -p.scale.x*(c*p.center.x-s*p.center.y-p.center.x),
                        p.scale.y*s,p.scale.y*c,-p.scale.y*(s*p.center.x+c*p.center.y-p.center.y)});
                } else if constexpr (std::is_same_v<S,TimedState<Assets::TextureSineMapping>>) {
                    state.phase+=delta*(2*mapping_pi/1000.0f);
                    result.transform=Make_Affine_Texture_Transform({p.scale.x,0,p.u.x*std::sin(p.u.y*state.phase+p.u.z*mapping_pi),
                        0,p.scale.y,p.v.x*std::sin(p.v.y*state.phase+p.v.z*mapping_pi)});
                } else if constexpr (std::is_same_v<S,TimedState<Assets::TextureStepMapping>>) {
                    state.phase+=delta;
                    const float spms=p.steps_per_second/1000.0f;
                    const int steps=static_cast<int>(spms*state.phase);
                    state.offset.x+=p.step.x*steps; state.offset.y+=p.step.y*steps;
                    if (spms!=0) state.phase-=steps/spms;
                    state.offset.x=Bounded_Offset(state.offset.x,p.scale.x,p.clamp);
                    state.offset.y=Bounded_Offset(state.offset.y,p.scale.y,p.clamp);
                    result.transform=Make_Affine_Texture_Transform({p.scale.x,0,state.offset.x,0,p.scale.y,state.offset.y});
                } else if constexpr (std::is_same_v<S,TimedState<Assets::TextureZigZagMapping>>) {
                    state.phase+=delta;
                    const float period=std::fabs(p.period_seconds*1000.0f);
                    float time=0;
                    if (period>0) {
                        state.phase-=static_cast<int>(state.phase/period)*period;
                        time=state.phase>period*0.5f ? period-state.phase : state.phase;
                    }
                    result.transform=Make_Affine_Texture_Transform({p.scale.x,0,(p.rate_per_second.x/1000.0f)*time,
                        0,p.scale.y,(p.rate_per_second.y/1000.0f)*time});
                } else if constexpr (std::is_same_v<S,TimedState<Assets::TextureEdgeMapping>>) {
                    state.phase+=delta*0.001f*p.rate_per_second;
                    state.phase-=std::floor(state.phase);
                    result.transform={0,0,0.5f,0.5f, 0,0,0,state.phase, 0,0,1,0, 0,0,0,1};
                    result.coordinates.source=p.reflection ? TextureCoordinateSource::CameraReflection : TextureCoordinateSource::CameraNormal;
                }
            }
            return result;
        },m_state);
    }
private:
    explicit TextureMapping(MappingState state): m_state(std::move(state)) {}
    MappingState m_state;
};
}

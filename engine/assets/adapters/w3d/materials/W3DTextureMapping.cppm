module;
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
export module Assets.Adapters.W3D.TextureMapping;
import Assets.Materials.TextureMapping;
import Assets.Math;

namespace Assets::W3D {
namespace {
std::string_view Trim_Mapping_Argument(std::string_view text) noexcept
{
    constexpr std::string_view whitespace=" \t\r\n\v\f";
    const auto first=text.find_first_not_of(whitespace);
    if (first==std::string_view::npos) return {};
    return text.substr(first,text.find_last_not_of(whitespace)-first+1);
}

// Borrow the bounded argument payload while decoding. Keys are case-sensitive,
// duplicate entries keep their first value, and semicolons begin comments.
class MappingArguments final {
public:
    explicit MappingArguments(std::string_view text)
    {
        text=text.substr(0,text.find('\0'));
        bool in_arguments=true;
        while (!text.empty()) {
            const auto end=text.find('\n');
            auto line=Trim_Mapping_Argument(text.substr(0,end));
            text=end==std::string_view::npos ? std::string_view{} : text.substr(end+1);
            if (line.starts_with('[') && line.find(']')!=std::string_view::npos) {
                if (!m_values.empty()) break;
                in_arguments=Trim_Mapping_Argument(line.substr(1,line.find(']')-1))=="Args";
                continue;
            }
            if (!in_arguments) continue;
            line=Trim_Mapping_Argument(line.substr(0,line.find(';')));
            const auto divider=line.find('=');
            if (divider==std::string_view::npos) continue;
            const auto key=Trim_Mapping_Argument(line.substr(0,divider));
            const auto value=Trim_Mapping_Argument(line.substr(divider+1));
            if (key.empty() || value.empty() || Find(key)) continue;
            m_values.emplace_back(key,value);
        }
    }
    float Float(std::string_view key,float fallback) const
    {
        const auto value=Find(key);
        if (!value) return fallback;
        const std::string input(*value);
        std::sscanf(input.c_str(),"%f",&fallback);
        if (input.find('%')!=std::string::npos) fallback/=100.0f;
        return fallback;
    }
    std::uint32_t Integer(std::string_view key,std::uint32_t fallback) const
    {
        const auto value=Find(key);
        if (!value) return fallback;
        const std::string input(*value);
        unsigned parsed=fallback;
        if (input.front()=='$') std::sscanf(input.c_str(),"$%x",&parsed);
        else if (input.back()=='h' || input.back()=='H') std::sscanf(input.c_str(),"%x",&parsed);
        else parsed=static_cast<unsigned>(std::atoi(input.c_str()));
        return parsed;
    }
    bool Boolean(std::string_view key,bool fallback) const
    {
        const auto value=Find(key);
        if (!value) return fallback;
        switch (value->front()) {
        case 'Y': case 'y': case 'T': case 't': case '1': return true;
        case 'N': case 'n': case 'F': case 'f': case '0': return false;
        default: return fallback;
        }
    }
    TextureMappingAxis Axis() const
    {
        const auto value=Find("Axis");
        if (value && (value->front()=='X' || value->front()=='x')) return TextureMappingAxis::X;
        if (value && (value->front()=='Y' || value->front()=='y')) return TextureMappingAxis::Y;
        return TextureMappingAxis::Z;
    }
    Vector2f Scale() const { return {Float("UScale",1),Float("VScale",1)}; }
    Vector2f Rate() const { return {Float("UPerSec",0),Float("VPerSec",0)}; }
    TextureScrollMapping Scroll(bool projected=false) const {
        return {Scale(),Rate(),{Float("UOffset",0),Float("VOffset",0)},Boolean("ClampFix",false),projected};
    }
private:
    std::optional<std::string_view> Find(std::string_view key) const {
        for (const auto& value:m_values) if (value.first==key) return value.second;
        return std::nullopt;
    }
    std::vector<std::pair<std::string_view,std::string_view>> m_values;
};
}

// Zero and unsupported mapping codes do not install a new mapper. This also
// preserves an existing mapping when an asset reapplies an ordinary UV stage.
export std::optional<TextureMappingDescription> W3DRead_Texture_Mapping(
    std::uint32_t attributes,unsigned stage,std::string_view text)
{
    if (stage>1) return std::nullopt;
    const auto code=(attributes >> (stage==0 ? 16 : 8)) & 255u;
    if (code==0 || code==5 || code>20) return std::nullopt;
    const MappingArguments args(text);
    using Source=TextureEnvironmentSource;
    switch (code) {
    case 1: return TextureEnvironmentMapping{Source::Reflection};
    case 2: return TextureEnvironmentMapping{Source::Normal};
    case 3: return args.Scroll(true);
    case 4: return args.Scroll();
    case 6: return TextureScaleMapping{args.Scale()};
    case 7: case 14: case 15: case 19: case 20: {
        TextureGridMapping grid;
        grid.frames_per_second=args.Float("FPS",1);
        grid.width_log2=args.Integer("Log2Width",1);
        grid.last_frame=args.Integer("Last",0);
        grid.start_frame=args.Integer("Offset",0);
        if (code!=7) grid.environment={code==14 || code==19 ? Source::Normal : Source::Reflection,
            code>=19,args.Axis()};
        return grid;
    }
    case 8: return TextureRotateMapping{args.Scale(),{args.Float("UCenter",0),args.Float("VCenter",0)},args.Float("Speed",0.1f)};
    case 9: return TextureSineMapping{args.Scale(),
        {args.Float("UAmp",1),args.Float("UFreq",1),args.Float("UPhase",0)},
        {args.Float("VAmp",1),args.Float("VFreq",1),args.Float("VPhase",0)}};
    case 10: return TextureStepMapping{args.Scale(),{args.Float("UStep",0),args.Float("VStep",0)},args.Float("SPS",0),args.Boolean("ClampFix",false)};
    case 11: return TextureZigZagMapping{args.Scale(),args.Rate(),args.Float("Period",0)};
    case 12: return TextureEnvironmentMapping{Source::Normal,true,args.Axis()};
    case 13: return TextureEnvironmentMapping{Source::Reflection,true,args.Axis()};
    case 16: return TextureRandomMapping{args.Scale(),args.Rate(),args.Float("FPS",0)};
    case 17: return TextureEdgeMapping{args.Float("VPerSec",0),args.Float("VStart",0),args.Boolean("UseReflect",false)};
    case 18: return TextureBumpMapping{args.Scroll(),args.Float("BumpRotation",0),args.Float("BumpScale",1)};
    default: return std::nullopt;
    }
}
}

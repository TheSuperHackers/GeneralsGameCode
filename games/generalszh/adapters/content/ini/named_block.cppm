module;
#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
export module games.generalszh.adapters.content.ini.named_block;
export namespace generalszh::content
{
inline std::string_view Trim(std::string_view text) noexcept
{
    const auto first=text.find_first_not_of(" \t\r\n"); if(first==text.npos) return {};
    return text.substr(first,text.find_last_not_of(" \t\r\n")-first+1);
}
inline bool EqualToken(std::string_view a,std::string_view b) noexcept
{
    if(a.size()!=b.size()) return false;
    for(std::size_t i=0;i!=a.size();++i)
    { auto upper=[](char c){return c>='a'&&c<='z'?char(c-'a'+'A'):c;}; if(upper(a[i])!=upper(b[i])) return false; }
    return true;
}
inline std::vector<std::string_view> Tokens(std::string_view value)
{
    std::vector<std::string_view> result;
    while(!(value=Trim(value)).empty())
    { const auto end=value.find_first_of(" \t"); result.push_back(value.substr(0,end)); if(end==value.npos) break; value.remove_prefix(end); }
    return result;
}
struct IniField { std::string key,value; };
struct IniBlock
{
    std::string kind,argument;
    std::vector<IniField> fields;
    std::vector<IniBlock> children;
    std::string_view Value(std::string_view key) const noexcept
    { for(auto it=fields.rbegin();it!=fields.rend();++it) if(EqualToken(it->key,key)) return it->value; return {}; }
    std::string_view Require(std::string_view key) const
    { const auto value=Value(key); if(value.empty()) throw std::invalid_argument("Missing INI field: "+std::string(key)); return value; }
};
inline std::string ReadIniFile(const std::filesystem::path &path)
{
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file) throw std::runtime_error("Cannot open content file: "+path.string());
    const auto length=file.tellg();
    if(length<0||length>32*1024*1024) throw std::length_error("INI file exceeds 32 MiB boundary");
    std::string text(static_cast<std::size_t>(length),'\0'); file.seekg(0);
    if(!text.empty()&&!file.read(text.data(),static_cast<std::streamsize>(text.size()))) throw std::runtime_error("Cannot read content file");
    return text;
}
// Decimal conversion without locale, floating point, exponent syntax or silent
// truncation. The requested scale must exactly represent the authored decimal.
inline std::uint64_t ScaledDecimal(std::string_view text,std::uint64_t scale)
{
    text=Trim(text); if(text.empty()||!scale) throw std::invalid_argument("Missing decimal value or scale");
    std::uint64_t whole=0,fraction=0,denominator=1; bool dot=false,digit=false;
    for(const char c:text)
    {
        if(c=='.'&&!dot) {dot=true;continue;}
        if(c<'0'||c>'9') throw std::invalid_argument("Expected nonnegative fixed decimal: "+std::string(text));
        digit=true; auto &part=dot?fraction:whole;
        if(part>((std::numeric_limits<std::uint64_t>::max)()-9)/10) throw std::out_of_range("Decimal overflow");
        part=part*10+static_cast<unsigned>(c-'0');
        if(dot) { if(denominator>100000000) throw std::invalid_argument("At most nine decimal places are supported"); denominator*=10; }
    }
    if(!digit||scale>((std::numeric_limits<std::uint64_t>::max)())/denominator||whole>((std::numeric_limits<std::uint64_t>::max)())/scale)
        throw std::out_of_range("Scaled decimal overflow");
    const auto fractional=fraction*scale;
    if(fractional%denominator) throw std::invalid_argument("Decimal cannot be represented at the selected scale");
    const auto integral=whole*scale;
    if(integral>(std::numeric_limits<std::uint64_t>::max)()-fractional/denominator) throw std::out_of_range("Scaled decimal overflow");
    return integral+fractional/denominator;
}
inline IniBlock ReadNamedBlock(std::string_view text,std::string_view kind,std::string_view name)
{
    if(text.size()>32*1024*1024) throw std::length_error("INI source exceeds 32 MiB boundary");
    std::vector<std::string_view> lines;
    while(!text.empty())
    {
        const auto end=text.find('\n'); auto line=text.substr(0,end); bool quote=false;
        for(std::size_t i=0;i<line.size();++i)
        {
            if(line[i]=='"') quote=!quote;
            if(!quote&&(line[i]==';'||(line[i]=='/'&&i+1<line.size()&&line[i+1]=='/'))) {line=line.substr(0,i);break;}
        }
        if(!(line=Trim(line)).empty()) lines.push_back(line);
        if(end==text.npos) break; text.remove_prefix(end+1);
    }
    std::size_t begin=lines.size(),end=lines.size();
    for(std::size_t i=0;i!=lines.size();++i)
    {
        if(lines[i].find('=')!=std::string_view::npos) continue;
        const auto words=Tokens(lines[i]);
        if(words.size()!=(name.empty()?1u:2u)||!EqualToken(words[0],kind)) continue;
        if(begin!=lines.size()&&end==lines.size()) end=i;
        if(name.empty()||words[1]==name)
        { if(begin!=lines.size()) throw std::invalid_argument("Duplicate named INI block"); begin=i; }
    }
    if(begin==lines.size()) throw std::invalid_argument("Missing INI block: "+std::string(kind)+" "+std::string(name));
    if(end<begin) end=lines.size();
    IniBlock root{std::string(kind),std::string(name)}; std::vector<IniBlock*> stack{&root};
    constexpr std::array assignmentBlocks{"Draw","Behavior","Body","ClientBehavior","ConditionState","TransitionState","AnimationState","Turret"};
    for(std::size_t i=begin+1;i<end;++i)
    {
        auto line=lines[i];
        if(stack.empty()) throw std::invalid_argument("Unsupported nesting or trailing content after INI End");
        if(EqualToken(line,"End")) {stack.pop_back();continue;}
        if(line.front()=='#') throw std::invalid_argument("INI preprocessing is not supported by the independent adapter yet");
        const auto equals=line.find('='); std::string_view key,value; bool block=equals==line.npos;
        if(block)
        {const auto split=line.find_first_of(" \t"); key=line.substr(0,split); value=split==line.npos?std::string_view{}:Trim(line.substr(split));}
        else
        {
            key=Trim(line.substr(0,equals)); value=Trim(line.substr(equals+1));
            for(const auto candidate:assignmentBlocks) block|=EqualToken(key,candidate);
        }
        if(block)
        {
            if(stack.size()==64) throw std::length_error("INI nesting exceeds 64 levels");
            stack.back()->children.push_back({std::string(key),std::string(value)}); stack.push_back(&stack.back()->children.back());
        }
        else stack.back()->fields.push_back({std::string(key),std::string(value)});
    }
    if(!stack.empty()) throw std::invalid_argument("Unterminated or unsupported INI block");
    return root;
}
}

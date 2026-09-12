module;
#include <charconv>
#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <system_error>
export module games.generalszh.adapters.content.progression.veterancy_decoder;
export import games.generalszh.gameplay.progression.definitions.veterancy_definition;
export import games.generalszh.adapters.content.ini.named_block;
export namespace generalszh::content
{
// Decode a resolved Object block. DefaultThingTemplate/inheritance resolution
// remains the caller's content responsibility, as with the existing unit decoder.
inline progression::VeterancyDefinition DecodeVeterancy(const IniBlock &object)
{
 progression::VeterancyDefinition result;
 std::array<std::int64_t,4> authoredSkillPointValues{};
 authoredSkillPointValues.fill(-999);
 bool skillPointValuesFound=false;
 auto integers=[](std::string_view value,auto &destination) {
  const auto words=Tokens(value);
  if(words.size()!=4) throw std::invalid_argument("Veterancy requires four integers");
  for(std::size_t i=0;i<4;++i) {
   std::uint32_t number{};
   const auto word=words[i];
   const auto parsed=std::from_chars(word.data(),word.data()+word.size(),number);
   if(parsed.ec!=std::errc{} || parsed.ptr!=word.data()+word.size() || number>2147483647u)
    throw std::invalid_argument("Invalid Zero Hour experience integer");
   destination[i]=number;
  }
 };
 auto skillIntegers=[&](std::string_view value) {
  const auto words=Tokens(value);
  if(words.size()!=4) throw std::invalid_argument("SkillPointValue requires four integers");
  for(std::size_t i=0;i<4;++i) {
   std::int64_t number{};
   const auto word=words[i];
   const auto parsed=std::from_chars(word.data(),word.data()+word.size(),number);
   if(parsed.ec != std::errc{} || parsed.ptr != word.data()+word.size() || number < -999 ||
      number > static_cast<std::int64_t>((std::numeric_limits<std::int32_t>::max)()))
    throw std::invalid_argument("Invalid Zero Hour skill-point integer");
   authoredSkillPointValues[i]=number;
  }
  skillPointValuesFound=true;
 };
 for(const auto &field:object.fields) {
  if(EqualToken(field.key,"ExperienceRequired")) integers(field.value,result.required);
  else if(EqualToken(field.key,"ExperienceValue")) integers(field.value,result.awardValues);
  else if(EqualToken(field.key,"SkillPointValue")) skillIntegers(field.value);
  else if(EqualToken(field.key,"IsTrainable")) {
   if(EqualToken(field.value,"Yes")) result.trainable=true;
   else if(EqualToken(field.value,"No")) result.trainable=false;
   else throw std::invalid_argument("Invalid IsTrainable boolean");
  }
 }
 // ThingTemplate initializes SkillPointValue entries to -999. Preserve the
 // inspected legacy fallback at the content boundary, while keeping the
 // runtime definition and producer independent of unit XP state.
 for(std::size_t i=0;i<4;++i)
  result.skillPointValues[i] = !skillPointValuesFound || authoredSkillPointValues[i] == -999
   ? result.awardValues[i] : static_cast<std::uint32_t>(authoredSkillPointValues[i]);
 (void)result.Progression();
 return result;
}
inline progression::VeterancyDefinition DecodeVeterancy(std::string_view text,std::string_view objectName)
{ return DecodeVeterancy(ReadNamedBlock(text,"Object",objectName)); }
}

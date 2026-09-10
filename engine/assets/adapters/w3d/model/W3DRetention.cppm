module;
#include <optional>
#include <string_view>
export module Assets.Adapters.W3D.Retention;

namespace Assets::W3D {
// Views borrow the supplied name. Membership remains an exact, case-sensitive
// comparison chosen by the consumer; these keys are not canonical asset paths.
export std::optional<std::string_view> W3D_Model_Retention_Key(std::string_view name) noexcept
{
    if(name.find('#')!=std::string_view::npos)return std::nullopt;
    return name.substr(0,name.find('.'));
}

export std::optional<std::string_view> W3D_Animation_Retention_Key(std::string_view name) noexcept
{
    const auto separator=name.find('.');
    if(separator==std::string_view::npos)return std::nullopt;
    return name.substr(separator+1);
}
}

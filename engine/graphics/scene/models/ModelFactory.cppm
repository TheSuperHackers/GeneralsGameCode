module;
#include <functional>
#include <string>
#include <utility>
export module Graphics.Scene.Models.Factory;

namespace Graphics {
// Immutable creation record. The callback owns any source data it needs and
// returns an independently owned instance; format decoding stays in adapters.
export template<class Object>
class ModelFactory final
{
public:
    template<class Creator>
    ModelFactory(std::string identity,int classification,Creator&& create)
        : name(std::move(identity)),class_id(classification),m_create(std::forward<Creator>(create)) {}
    ModelFactory(const ModelFactory&)=delete;
    ModelFactory& operator=(const ModelFactory&)=delete;
    Object* Instantiate() const { return m_create(); }
    const std::string name;
    const int class_id;
private:
    std::function<Object*()> m_create;
};
}

module;
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
export module Graphics.Scene.Models.FactoryStore;

namespace Graphics {
// Named factories belong to the graphics owner. Payload creation and destruction
// policy come from callers; this store knows no asset format or runtime class.
export template<class Factory,class Deleter=std::default_delete<Factory>>
class ModelFactoryStore final
{
public:
    using Owner=std::unique_ptr<Factory,Deleter>;
    ModelFactoryStore()=default;
    ModelFactoryStore(const ModelFactoryStore&)=delete;
    ModelFactoryStore& operator=(const ModelFactoryStore&)=delete;
    ~ModelFactoryStore() { Clear(); }

    std::size_t Size() const noexcept { return m_entries.size(); }
    Factory* At(std::size_t index) const noexcept { return index<Size()?m_entries[index].factory.get():nullptr; }
    Factory* Find(std::string_view name) const {
        const auto found=m_names.find(Key(name));
        return found==m_names.end()?nullptr:found->second.back();
    }
    void Insert(std::string_view name,Owner factory) {
        if(!factory)return;
        auto key=Key(name);
        m_entries.emplace_back();
        try {
            m_names[key].push_back(factory.get());
        } catch(...) {
            const auto found=m_names.find(key);
            if(found!=m_names.end() && found->second.empty())m_names.erase(found);
            m_entries.pop_back();
            throw;
        }
        m_entries.back().key=std::move(key);
        m_entries.back().factory=std::move(factory);
    }
    Owner Release(Factory* factory) {
        const auto found=std::find_if(m_entries.begin(),m_entries.end(),
            [&](const Entry& entry) { return entry.factory.get()==factory; });
        if(found==m_entries.end())return {};
        Unindex(*found);
        Owner owner=std::move(found->factory);
        m_entries.erase(found);
        return owner;
    }
    template<class Predicate>
    void Erase_If(Predicate&& remove) {
        std::vector<Owner> removed;
        removed.reserve(Size());
        std::vector<bool> erase;
        erase.reserve(Size());
        for(const auto& entry:m_entries)erase.push_back(remove(*entry.factory));
        std::size_t retained=0;
        for(std::size_t index=0;index<m_entries.size();++index) {
            auto& entry=m_entries[index];
            if(erase[index]) {
                Unindex(entry);
                removed.push_back(std::move(entry.factory));
            } else {
                if(retained!=index)m_entries[retained]=std::move(entry);
                ++retained;
            }
        }
        m_entries.resize(retained);
        // Detach before destruction so callbacks cannot find a dead entry.
        for(auto& owner:removed)owner.reset();
    }
    void Clear() {
        // Preserve reverse insertion destruction order and detach names first.
        m_names.clear();
        while(!m_entries.empty()) {
            auto owner=std::move(m_entries.back().factory);
            m_entries.pop_back();
        }
    }
private:
    struct Entry { std::string key;Owner factory; };
    static std::string Key(std::string_view name) {
        std::string key(name);
        for(char& value:key)value=static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
        return key;
    }
    void Unindex(const Entry& entry) {
        auto found=m_names.find(entry.key);
        auto& factories=found->second;
        factories.erase(std::find(factories.begin(),factories.end(),entry.factory.get()));
        if(factories.empty())m_names.erase(found);
    }
    std::vector<Entry> m_entries;
    std::unordered_map<std::string,std::vector<Factory*>> m_names;
};
}

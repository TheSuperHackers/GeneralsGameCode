module;
#include <cstddef>
#include <limits>
#include <vector>
export module Graphics.Scene.ObjectList;

namespace Graphics
{
namespace SceneListDetail
{
class Storage;
struct Link { Storage* owner=nullptr; std::size_t index=0; };
class Storage
{
public:
    virtual Link Next_Membership(std::size_t index) const noexcept = 0;
    virtual void Set_Next_Membership(std::size_t index,Link next) noexcept = 0;
    virtual void Unlink(std::size_t index) noexcept = 0;
protected:
    ~Storage() = default;
};
}

export template<class T,bool Retained> class SceneObjectList;

// Scene identities can belong to several ordered collections. Copying source
// data does not copy membership; destruction detaches borrowed entries before
// their storage can become dangling. No process-global node allocator is used.
export class SceneListMember
{
public:
    SceneListMember() = default;
    SceneListMember(const SceneListMember&) noexcept {}
    SceneListMember& operator=(const SceneListMember&) noexcept { return *this; }
    virtual ~SceneListMember()
    {
        while (m_first.owner) m_first.owner->Unlink(m_first.index);
    }
private:
    template<class T,bool Retained> friend class SceneObjectList;
    SceneListDetail::Link m_first;
};

// Entries occupy owner-local indexed storage. Order links survive vector growth;
// scoped cursors advance past erased entries before slots are reused. Retained
// collections use the caller's Add_Ref/Release_Ref contract.
export template<class T,bool Retained=true>
class SceneObjectList final : private SceneListDetail::Storage
{
    static constexpr std::size_t End=std::numeric_limits<std::size_t>::max();
    struct Entry
    {
        T* object=nullptr;
        std::size_t previous=End,next=End;
        SceneListDetail::Link membership;
    };
public:
    SceneObjectList() = default;
    SceneObjectList(const SceneObjectList&) = delete;
    SceneObjectList& operator=(const SceneObjectList&) = delete;
    ~SceneObjectList()
    {
        Reset_List();
        while (m_cursors) m_cursors->Detach();
    }

    bool Is_Empty() const noexcept { return m_head==End; }
    std::size_t Count() const noexcept { return m_count; }
    bool Contains(const T* object) const noexcept { return Find(object)!=End; }
    bool Is_In_List(const T* object) const noexcept { return Contains(object); }
    bool Add(T* object,bool unique=true) { return Insert(object,End,m_head,unique); }
    bool Add_Tail(T* object,bool unique=true) { return Insert(object,m_tail,End,unique); }
    bool Add_After(T* object,const T* existing,bool unique=true)
    {
        const auto index=Find(existing);
        return index!=End && Insert(object,index,m_entries[index].next,unique);
    }
    bool Remove(T* object)
    {
        const auto index=Find(object);
        if (index==End) return false;
        Unlink(index);
        if constexpr (Retained) object->Release_Ref();
        return true;
    }
    T* Peek_Head() const noexcept { return m_head==End ? nullptr : m_entries[m_head].object; }
    T* Get_Head() const
    {
        auto* object=Peek_Head();
        if constexpr (Retained) { if (object) object->Add_Ref(); }
        return object;
    }
    T* Remove_Head() noexcept
    {
        auto* object=Peek_Head();
        if (object) Unlink(m_head);
        return object; // Transfers the collection's reference to the caller.
    }
    bool Release_Head()
    {
        auto* object=Remove_Head();
        if constexpr (Retained) { if (object) object->Release_Ref(); }
        return object!=nullptr;
    }
    void Reset_List() { while (Release_Head()) {} }

    class Cursor final
    {
    public:
        explicit Cursor(SceneObjectList* list) { First(list); }
        Cursor(const Cursor&) = delete;
        Cursor& operator=(const Cursor&) = delete;
        ~Cursor() { Detach(); }
        void First(SceneObjectList* list) { Attach(list); First(); }
        void First() noexcept { m_reverse=false; Select(m_list ? m_list->m_head : End); }
        void Last(SceneObjectList* list) { Attach(list); Last(); }
        void Last() noexcept { m_reverse=true; Select(m_list ? m_list->m_tail : End); }
        bool Is_Done() const noexcept
        {
            return !m_list || m_index==End;
        }
        void Next() noexcept
        {
            m_reverse=false;
            if (m_removed) m_removed=false;
            else Select(Is_Done() ? End : m_list->m_entries[m_index].next);
        }
        void Prev() noexcept
        {
            m_reverse=true;
            if (m_removed) m_removed=false;
            else Select(Is_Done() ? End : m_list->m_entries[m_index].previous);
        }
        T* Peek_Obj() const noexcept { return Is_Done() ? nullptr : m_list->m_entries[m_index].object; }
        T* Get_Obj() const
        {
            auto* object=Peek_Obj();
            if constexpr (Retained) { if (object) object->Add_Ref(); }
            return object;
        }
        void Remove_Current_Object()
        {
            auto* object=Peek_Obj();
            if (!object) return;
            const auto index=m_index;
            Next();
            m_list->Unlink(index);
            if constexpr (Retained) object->Release_Ref();
        }
    private:
        friend class SceneObjectList;
        void Attach(SceneObjectList* list) noexcept
        {
            if (m_list==list) return;
            Detach();
            m_list=list;
            if (!list) return;
            m_next=list->m_cursors;
            if (m_next) m_next->m_previous=this;
            list->m_cursors=this;
        }
        void Detach() noexcept
        {
            if (m_previous) m_previous->m_next=m_next;
            else if (m_list) m_list->m_cursors=m_next;
            if (m_next) m_next->m_previous=m_previous;
            m_list=nullptr; m_previous=m_next=nullptr;
            Select(End);
        }
        void Select(std::size_t index) noexcept
        {
            m_index=index;
            m_removed=false;
        }
        SceneObjectList* m_list=nullptr;
        Cursor* m_previous=nullptr;
        Cursor* m_next=nullptr;
        std::size_t m_index=End;
        bool m_removed=false,m_reverse=false;
    };

private:
    std::size_t Find(const T* object) const noexcept
    {
        if (!object) return End;
        auto link=static_cast<const SceneListMember*>(object)->m_first;
        while (link.owner) {
            if (link.owner==this) return link.index;
            link=link.owner->Next_Membership(link.index);
        }
        return End;
    }
    bool Insert(T* object,std::size_t previous,std::size_t next,bool unique)
    {
        if (!object || (unique && Contains(object))) return false;
        std::size_t index=m_free;
        if (index==End) { index=m_entries.size(); m_entries.emplace_back(); }
        else m_free=m_entries[index].next;
        auto& entry=m_entries[index];
        entry.object=object; entry.previous=previous; entry.next=next;
        auto& member=*static_cast<SceneListMember*>(object);
        entry.membership=member.m_first;
        member.m_first={this,index};
        if (previous==End) m_head=index; else m_entries[previous].next=index;
        if (next==End) m_tail=index; else m_entries[next].previous=index;
        ++m_count;
        if constexpr (Retained) object->Add_Ref();
        return true;
    }
    SceneListDetail::Link Next_Membership(std::size_t index) const noexcept override
    {
        return m_entries[index].membership;
    }
    void Set_Next_Membership(std::size_t index,SceneListDetail::Link next) noexcept override
    {
        m_entries[index].membership=next;
    }
    void Unlink(std::size_t index) noexcept override
    {
        auto& entry=m_entries[index];
        auto& member=*static_cast<SceneListMember*>(entry.object);
        SceneListDetail::Link previous;
        auto link=member.m_first;
        while (link.owner && (link.owner!=this || link.index!=index)) {
            previous=link;
            link=link.owner->Next_Membership(link.index);
        }
        if (previous.owner) previous.owner->Set_Next_Membership(previous.index,entry.membership);
        else member.m_first=entry.membership;
        if (entry.previous==End) m_head=entry.next; else m_entries[entry.previous].next=entry.next;
        if (entry.next==End) m_tail=entry.previous; else m_entries[entry.next].previous=entry.previous;
        for (auto* cursor=m_cursors;cursor;cursor=cursor->m_next) {
            if (cursor->m_index==index) {
                cursor->Select(cursor->m_reverse ? entry.previous : entry.next);
                cursor->m_removed=true;
            }
        }
        entry.object=nullptr; entry.membership={}; entry.next=m_free;
        m_free=index;
        --m_count;
    }

    std::vector<Entry> m_entries;
    Cursor* m_cursors=nullptr;
    std::size_t m_head=End,m_tail=End,m_free=End,m_count=0;
};
}

module;
#include <array>
#include <cstdint>
#include <utility>
export module Graphics.Frame.AttachmentBindings;
export import Graphics.Resources.Textures.Resource;

namespace Graphics
{
export class AttachmentBindings;
export struct AttachmentSelection final
{
    RHITextureHandle color{},depth{};
    RHIViewport viewport{};
};

// Retains GPU generations independently of the CPU objects that published them.
// The device must outlive every snapshot and binding owner.
export class AttachmentSnapshot final
{
public:
    AttachmentSnapshot() = default;
    AttachmentSnapshot(const AttachmentSnapshot& other) { Acquire(other.m_device,other.m_selection); }
    AttachmentSnapshot& operator=(const AttachmentSnapshot& other)
    {
        if (this!=&other) { AttachmentSnapshot copy(other); Swap(copy); }
        return *this;
    }
    AttachmentSnapshot(AttachmentSnapshot&& other) noexcept { Swap(other); }
    AttachmentSnapshot& operator=(AttachmentSnapshot&& other) noexcept
    {
        if (this!=&other) { Reset(); Swap(other); }
        return *this;
    }
    ~AttachmentSnapshot() { Reset(); }
    const AttachmentSelection& Selection() const noexcept { return m_selection; }
    Device* Owner() const noexcept { return m_device; }
    void Reset() noexcept
    {
        if (m_device) {
            if (m_selection.color.Is_Valid()) m_device->Destroy_Texture(m_selection.color);
            if (m_selection.depth.Is_Valid()) m_device->Destroy_Texture(m_selection.depth);
        }
        m_device=nullptr; m_selection={};
    }
private:
    friend class AttachmentBindings;
    bool Acquire(Device* device,AttachmentSelection selection)
    {
        if (!device || (!selection.color.Is_Valid() && !selection.depth.Is_Valid())) return false;
        if (selection.color.Is_Valid() && !device->Retain_Texture(selection.color)) return false;
        if (selection.depth.Is_Valid() && !device->Retain_Texture(selection.depth)) {
            if (selection.color.Is_Valid()) device->Destroy_Texture(selection.color);
            return false;
        }
        m_device=device; m_selection=selection;
        return true;
    }
    void Swap(AttachmentSnapshot& other) noexcept
    {
        std::swap(m_device,other.m_device); std::swap(m_selection,other.m_selection);
    }
    Device* m_device=nullptr;
    AttachmentSelection m_selection{};
};

export class AttachmentBindings final
{
public:
    AttachmentBindings() = default;
    AttachmentBindings(const AttachmentBindings&) = delete;
    AttachmentBindings& operator=(const AttachmentBindings&) = delete;
    void Reset() noexcept { m_current.Reset(); m_default.Reset(); }
    bool Initialize(Device& device,AttachmentSelection defaults)
    {
        Reset();
        if (!m_default.Acquire(&device,defaults)) return false;
        if (!Bind(defaults)) { Reset(); return false; }
        return true;
    }
    const AttachmentSelection& Current() const noexcept { return m_current.Selection(); }
    const AttachmentSelection& Default() const noexcept { return m_default.Selection(); }
    bool Offscreen() const noexcept { return Current().color!=Default().color; }
    AttachmentSnapshot Capture() const { return m_current; }
    bool Restore(const AttachmentSnapshot& snapshot)
    {
        return snapshot.Owner()==m_default.Owner() && Bind(snapshot.Selection());
    }
    bool Restore_Default() { return Bind(Default()); }
    bool Bind(AttachmentSelection selection)
    {
        auto* device=m_default.Owner();
        if (!device || !selection.viewport.width || !selection.viewport.height) return false;
        AttachmentSnapshot next;
        if (!next.Acquire(device,selection)) return false;
        auto& commands=device->Immediate_Command_List();
        if (!commands.Set_Render_Targets(selection.color,selection.depth)) return false;
        if (!commands.Set_Viewport(selection.viewport)) {
            commands.Set_Render_Targets(Current().color,Current().depth);
            commands.Set_Viewport(Current().viewport);
            return false;
        }
        m_current=std::move(next);
        return true;
    }
    bool Bind(TextureResource* color,TextureResource* depth=nullptr)
    {
        if (!color && !depth) return Restore_Default();
        const auto* device=m_default.Owner();
        if ((color && (&color->Owner()!=device || !color->Is_Render_Target()))
            || (depth && (&depth->Owner()!=device || (depth->Description().usage
                & static_cast<unsigned>(RHITextureUsage::DepthStencil))==0))) return false;
        auto selection=Default();
        if (color) {
            selection.color=color->Handle(); selection.depth=color->Depth_Attachment();
            selection.viewport={0,0,color->Description().width,color->Description().height,0,1};
        }
        if (depth) selection.depth=depth->Handle();
        return Bind(selection);
    }
    bool Set_Viewport(RHIViewport viewport)
    {
        if (!m_default.Owner() || !m_default.Owner()->Immediate_Command_List().Set_Viewport(viewport)) return false;
        m_current.m_selection.viewport=viewport;
        return true;
    }
    bool Rebind()
    {
        auto* device=m_default.Owner();
        return device && device->Immediate_Command_List().Set_Render_Targets(Current().color,Current().depth)
            && device->Immediate_Command_List().Set_Viewport(Current().viewport);
    }
    void Clear(bool color,bool depth,std::array<float,4> value,float depth_value=1,std::uint8_t stencil=0)
    {
        if (!m_default.Owner()) return;
        auto& commands=m_default.Owner()->Immediate_Command_List();
        if (color) commands.Clear_Color_Target(Current().color,value);
        if (depth) commands.Clear_Depth_Stencil_Target(Current().depth,depth_value,stencil);
    }
private:
    AttachmentSnapshot m_default,m_current;
};

export class AttachmentScope final
{
public:
    AttachmentScope(AttachmentBindings& bindings,TextureResource* color,TextureResource* depth=nullptr)
        : m_bindings(bindings),m_saved(bindings.Capture()),m_active(bindings.Bind(color,depth)) {}
    AttachmentScope(const AttachmentScope&) = delete;
    AttachmentScope& operator=(const AttachmentScope&) = delete;
    ~AttachmentScope() { End(); }
    bool Active() const noexcept { return m_active; }
    void End()
    {
        if (!m_active) return;
        m_bindings.Restore(m_saved); m_active=false; m_saved.Reset();
    }
private:
    AttachmentBindings& m_bindings;
    AttachmentSnapshot m_saved;
    bool m_active=false;
};

export AttachmentBindings& Get_Attachment_Bindings()
{
    static AttachmentBindings bindings;
    return bindings;
}
}

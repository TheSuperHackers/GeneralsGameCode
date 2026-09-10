module;
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <limits>
#include <vector>
export module Graphics.Scene.Props.Constants;
export import Graphics.Scene.Props.Surface;
import Graphics.RHI;

namespace Graphics {
export struct PropParameters final
{
    // View data is contiguous for direct comparison and upload.
    std::array<float,16> view_projection{};
    std::array<float,4> shroud_projection{};
    std::array<float,4> fog_color{};
    std::array<float,4> fog_state{};
    std::array<float,4> camera_position{};
    std::array<float,16> view{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    // Lighting data is contiguous for direct comparison and upload.
    std::array<float,4> scene_ambient{};
    std::array<std::array<float,4>,4> light_direction{};
    std::array<std::array<float,4>,4> light_diffuse{};
    std::array<std::array<float,4>,4> light_specular{};
    std::array<std::array<float,4>,4> light_position{};
    std::array<std::array<float,4>,4> light_attenuation{};
    std::array<std::array<float,4>,4> light_ambient{};
    std::array<std::array<float,4>,4> light_spot{};
    // Material data is contiguous for direct comparison and upload.
    float textured = 1;
    float secondary_texture = 0;
    float alpha_cutoff = 0;
    float primary_gradient = 1;
    float detail_color = 0;
    float detail_alpha = 0;
    float secondary_gradient = 0;
    float shroud = 0;
    float shroud_only = 0;
    float opacity = 1;
    float texture_luminance = 0;
    float normal_in_world_space = 0;
    std::array<std::array<float,16>,2> uv_transform{{
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
        {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}}};
    std::array<float,4> uv_sources{};
    std::array<float,4> bump_matrix{};
    PropSurfaceParameters surface{};
    std::array<float,4> muzzle_flash_state{};
    // Object data is contiguous for direct comparison and upload.
    std::array<float,16> world{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
};
static_assert(sizeof(PropParameters) == 1008);
static_assert(offsetof(PropParameters,view_projection)==0);
static_assert(offsetof(PropParameters,scene_ambient)==192);
static_assert(offsetof(PropParameters,textured)==656);
static_assert(offsetof(PropParameters,world)==944);

export struct PropViewConstants final {
    std::array<float,16> view_projection{};
    std::array<float,4> shroud_projection{};
    std::array<float,4> fog_color{};
    std::array<float,4> fog_state{};
    std::array<float,4> camera_position{};
    std::array<float,16> view{};
};
struct PropLightingConstants final {
    std::array<float,4> scene_ambient{};
    std::array<std::array<float,4>,4> light_direction{};
    std::array<std::array<float,4>,4> light_diffuse{};
    std::array<std::array<float,4>,4> light_specular{};
    std::array<std::array<float,4>,4> light_position{};
    std::array<std::array<float,4>,4> light_attenuation{};
    std::array<std::array<float,4>,4> light_ambient{};
    std::array<std::array<float,4>,4> light_spot{};
};
export struct PropMaterialConstants final {
    float textured{};
    float secondary_texture{};
    float alpha_cutoff{};
    float primary_gradient{};
    float detail_color{};
    float detail_alpha{};
    float secondary_gradient{};
    float shroud{};
    float shroud_only{};
    float opacity{};
    float texture_luminance{};
    float normal_in_world_space{};
    std::array<std::array<float,16>,2> uv_transform{};
    std::array<float,4> uv_sources{};
    std::array<float,4> bump_matrix{};
    PropSurfaceParameters surface{};
    std::array<float,4> muzzle_flash_state{};
};
struct PropObjectConstants final {
    std::array<float,16> world{};
};
static_assert(sizeof(PropViewConstants)==192);
static_assert(sizeof(PropLightingConstants)==464);
static_assert(sizeof(PropMaterialConstants)==288);
static_assert(sizeof(PropObjectConstants)==64);

export struct PropSharedParameters final {
    PropViewConstants view;
    PropMaterialConstants material;
    explicit PropSharedParameters(const PropParameters& parameters) noexcept {
        std::memcpy(&view,&parameters,sizeof(view));
        std::memcpy(&material,&parameters.textured,sizeof(material));
    }
    bool Matches(const PropParameters& parameters) const noexcept {
        return std::memcmp(&view,&parameters,sizeof(view))==0
            && std::memcmp(&material,&parameters.textured,sizeof(material))==0;
    }
};
static_assert(sizeof(PropSharedParameters)==480);

// Each block owns its last successful upload. Command-list binding is still
// performed on each draw, so other renderers cannot leave stale slots.
template<class Value>
class PropConstantBlock final {
public:
    bool Initialize(Device& device) {
        m_buffer = device.Create_Buffer({sizeof(Value), RHIBufferUsage::Constant});
        m_uploaded = false;
        return m_buffer.Is_Valid();
    }
    void Shutdown(Device& device) noexcept {
        if (m_buffer.Is_Valid()) device.Destroy_Buffer(m_buffer);
        m_buffer = {}; m_uploaded = false;
    }
    bool Update(Device& device, std::span<const std::byte,sizeof(Value)> bytes) {
        if (m_uploaded && std::memcmp(m_bytes.data(), bytes.data(), bytes.size()) == 0) return true;
        if (!device.Update_Buffer(m_buffer, 0, bytes)) return false;
        std::memcpy(m_bytes.data(), bytes.data(), bytes.size()); m_uploaded = true;
        return true;
    }
    RHIBufferHandle Buffer() const noexcept { return m_buffer; }
private:
    RHIBufferHandle m_buffer{};
    std::array<std::byte,sizeof(Value)> m_bytes{};
    bool m_uploaded = false;
};

// A prepared mesh retains its material data independently of other meshes.
// Compare the complete effective block so animated mappings and draw overrides
// update it even when the source material itself has not changed.
export class PropMaterialBinding final {
public:
    bool Prepare(Device& device, const PropParameters& parameters) {
        if (!m_constants.Buffer().Is_Valid() && !m_constants.Initialize(device)) return false;
        const auto bytes = std::as_bytes(std::span(&parameters,1));
        return m_constants.Update(device, bytes.subspan<656,288>());
    }
    bool Prepare(Device& device, const PropMaterialConstants& parameters) {
        if (!m_constants.Buffer().Is_Valid() && !m_constants.Initialize(device)) return false;
        return m_constants.Update(device,std::as_bytes(std::span<const PropMaterialConstants,1>(&parameters,1)));
    }
    void Shutdown(Device& device) noexcept { m_constants.Shutdown(device); }
    RHIBufferHandle Buffer() const noexcept { return m_constants.Buffer(); }
private:
    PropConstantBlock<PropMaterialConstants> m_constants;
};

// A mesh keeps its instance stream across interleaved draws and frames.
// Publish the CPU snapshot only after the GPU upload succeeds.
export template<class Value>
class PropStorageBinding final {
public:
    bool Prepare(Device& device, std::span<const Value> values) {
        assert(!values.empty());
        assert(values.size() <= (std::numeric_limits<std::uint32_t>::max)()/sizeof(Value));
        const auto bytes = std::as_bytes(values);
        if (m_buffer.Is_Valid() && m_values.size() == values.size()
            && std::memcmp(m_values.data(),values.data(),bytes.size()) == 0) return true;
        if (values.size() > m_capacity) {
            auto capacity = std::bit_ceil(values.size());
            if (capacity > (std::numeric_limits<std::uint32_t>::max)()/sizeof(Value)) capacity = values.size();
            const auto buffer = device.Create_Buffer(
                {static_cast<std::uint32_t>(capacity*sizeof(Value)),RHIBufferUsage::Storage,sizeof(Value)});
            if (!buffer.Is_Valid()) return false;
            if (!device.Update_Buffer(buffer,0,bytes)) { device.Destroy_Buffer(buffer); return false; }
            if (m_buffer.Is_Valid()) device.Destroy_Buffer(m_buffer);
            m_buffer = buffer; m_capacity = capacity;
        } else if (!device.Update_Buffer(m_buffer,0,bytes)) return false;
        m_values.assign(values.begin(),values.end());
        return true;
    }
    void Shutdown(Device& device) noexcept {
        if (m_buffer.Is_Valid()) device.Destroy_Buffer(m_buffer);
        m_buffer = {}; m_capacity = 0; m_values.clear();
    }
    RHIBufferHandle Buffer() const noexcept { return m_buffer; }
private:
    RHIBufferHandle m_buffer{};
    std::size_t m_capacity = 0;
    std::vector<Value> m_values;
};

export using PropInstanceBinding = PropStorageBinding<std::array<float,16>>;
export using PropInstanceIndexBinding = PropStorageBinding<std::uint32_t>;

export class PropConstantBindings final {
public:
    bool Prepare_Lighting(Device& device,std::span<const std::byte,464> lighting) {
        return m_lighting.Update(device,lighting);
    }
    bool Initialize(Device& device) {
        return m_view.Initialize(device) && m_lighting.Initialize(device) && m_object.Initialize(device);
    }
    void Shutdown(Device& device) noexcept {
        m_view.Shutdown(device);
        m_lighting.Shutdown(device);
        m_object.Shutdown(device);
    }
    bool Prepare_Resources(Device& device, const PropParameters& parameters,
        PropMaterialBinding& material, std::span<RHIBindlessResource,4> resources, bool instance_records = false) {
        const auto bytes = std::as_bytes(std::span(&parameters,1));
        if (!m_view.Update(device, bytes.subspan<0,192>())
            || (!instance_records && !m_lighting.Update(device, bytes.subspan<192,464>()))
            || !material.Prepare(device, parameters)
            || (!instance_records && !m_object.Update(device, bytes.subspan<944,64>()))) return false;
        Bind(material,resources);
        return true;
    }
    bool Prepare_Resources(Device& device, const PropSharedParameters& parameters,
        PropMaterialBinding& material, std::span<RHIBindlessResource,4> resources, bool instance_records) {
        assert(instance_records);
        if (!m_view.Update(device,std::as_bytes(std::span<const PropViewConstants,1>(&parameters.view,1)))
            || !material.Prepare(device,parameters.material)) return false;
        Bind(material,resources);
        return true;
    }
private:
    void Bind(const PropMaterialBinding& material, std::span<RHIBindlessResource,4> resources) const {
        for (auto& resource : resources) resource={};
        resources[0].type = RHIResourceType::Material;
        resources[0].constant_buffer_slot = 0;
        resources[0].buffer = m_view.Buffer();
        resources[1].type = RHIResourceType::Material;
        resources[1].constant_buffer_slot = 1;
        resources[1].buffer = m_lighting.Buffer();
        resources[2].type = RHIResourceType::Material;
        resources[2].constant_buffer_slot = 2;
        resources[2].buffer = material.Buffer();
        resources[3].type = RHIResourceType::Material;
        resources[3].constant_buffer_slot = 3;
        resources[3].buffer = m_object.Buffer();
    }
    PropConstantBlock<PropViewConstants> m_view;
    PropConstantBlock<PropLightingConstants> m_lighting;
    PropConstantBlock<PropObjectConstants> m_object;
};
}

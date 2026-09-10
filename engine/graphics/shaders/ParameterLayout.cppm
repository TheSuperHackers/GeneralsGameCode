module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <type_traits>

export module Graphics.Shaders.ParameterLayout;

export import Graphics.Resources.Handles.ResourceHandle;

namespace Graphics
{

export using ShaderLayoutKey = std::uint64_t;

export inline constexpr std::uint16_t Invalid_Shader_Parameter_Offset = std::numeric_limits<std::uint16_t>::max();
export inline constexpr std::uint16_t Invalid_Shader_Parameter_Index = std::numeric_limits<std::uint16_t>::max();
export inline constexpr std::uint32_t Invalid_Shader_Resource_Index = std::numeric_limits<std::uint32_t>::max();

export enum class ShaderParameterBlock : std::uint8_t
{
	Frame,
	View,
	Material,
	Draw
};

export enum class ShaderParameterKind : std::uint8_t
{
	Constant,
	Buffer,
	BindlessResource
};

export enum class ShaderValueType : std::uint8_t
{
	UInt32,
	Float32,
	Float2,
	Float3,
	Float4,
	Matrix4x4
};

export enum class ShaderResourceClass : std::uint8_t
{
	Buffer,
	Texture,
	Sampler,
	Material
};

export struct ShaderParameterLocation final
{
	ShaderParameterBlock block = ShaderParameterBlock::Frame;
	ShaderParameterKind kind = ShaderParameterKind::Constant;
	ShaderValueType value_type = ShaderValueType::UInt32;
	ShaderResourceClass resource_class = ShaderResourceClass::Buffer;
	std::uint16_t parameter_index = Invalid_Shader_Parameter_Index;
	std::uint16_t constant_offset = Invalid_Shader_Parameter_Offset;
	std::uint16_t constant_size = 0;
	std::uint16_t resource_offset = Invalid_Shader_Parameter_Index;

	constexpr bool Is_Valid() const noexcept
	{
		return parameter_index != Invalid_Shader_Parameter_Index
			&& ((kind == ShaderParameterKind::Constant && constant_offset != Invalid_Shader_Parameter_Offset && constant_size != 0)
				|| (kind != ShaderParameterKind::Constant && resource_offset != Invalid_Shader_Parameter_Index));
	}

	friend constexpr bool operator==(const ShaderParameterLocation &left, const ShaderParameterLocation &right) noexcept
	{
		return left.block == right.block
			&& left.kind == right.kind
			&& left.value_type == right.value_type
			&& left.resource_class == right.resource_class
			&& left.parameter_index == right.parameter_index
			&& left.constant_offset == right.constant_offset
			&& left.constant_size == right.constant_size
			&& left.resource_offset == right.resource_offset;
	}

	friend constexpr bool operator!=(const ShaderParameterLocation &left, const ShaderParameterLocation &right) noexcept
	{
		return !(left == right);
	}
};

export struct alignas(16) ShaderParameterBlockData final
{
	static constexpr std::size_t Max_Constant_Bytes = 256;
	static constexpr std::size_t Max_Constant_Words = Max_Constant_Bytes / sizeof(std::uint32_t);
	static constexpr std::size_t Max_Resource_Indices = 32;

	std::array<std::uint32_t, Max_Constant_Words> constant_words{};
	std::array<std::uint32_t, Max_Resource_Indices> resource_indices{};

	ShaderParameterBlockData() noexcept
	{
		Clear();
	}

	void Clear() noexcept
	{
		constant_words.fill(0);
		resource_indices.fill(Invalid_Shader_Resource_Index);
	}

	bool Set_Constant(ShaderParameterLocation location, std::span<const std::byte> value) noexcept
	{
		if (!location.Is_Valid() || location.kind != ShaderParameterKind::Constant || location.constant_offset % sizeof(std::uint32_t) != 0 || value.size() != location.constant_size)
			return false;
		if (static_cast<std::size_t>(location.constant_offset) + value.size() > Max_Constant_Bytes)
			return false;

		const std::span<std::byte> destination = std::as_writable_bytes(std::span<std::uint32_t>(constant_words));
		std::memcpy(destination.data() + location.constant_offset, value.data(), value.size());
		return true;
	}

	template <typename Type>
	bool Set_Constant(ShaderParameterLocation location, const Type &value) noexcept
	{
		static_assert(std::is_trivially_copyable_v<Type>);
		return Set_Constant(location, std::as_bytes(std::span<const Type>(&value, 1)));
	}

	bool Set_Resource_Index(ShaderParameterLocation location, std::uint32_t index) noexcept
	{
		if (!location.Is_Valid() || location.kind == ShaderParameterKind::Constant || location.resource_offset >= Max_Resource_Indices)
			return false;

		resource_indices[location.resource_offset] = index;
		return true;
	}

	bool Set_Resource_Index(ShaderParameterLocation location, ResourceIndex index) noexcept
	{
		return Set_Resource_Index(location, index.Is_Valid() ? index.Get_Index() : Invalid_Shader_Resource_Index);
	}

	std::uint32_t Resource_Index(ShaderParameterLocation location) const noexcept
	{
		if (!location.Is_Valid() || location.kind == ShaderParameterKind::Constant || location.resource_offset >= Max_Resource_Indices)
			return Invalid_Shader_Resource_Index;

		return resource_indices[location.resource_offset];
	}

	std::span<const std::byte> Constant_Bytes(std::size_t byte_count) const noexcept
	{
		if (byte_count > Max_Constant_Bytes)
			return {};

		return std::as_bytes(std::span<const std::uint32_t>(constant_words)).first(byte_count);
	}

	std::span<const std::uint32_t> Resource_Indices() const noexcept
	{
		return resource_indices;
	}
};

static_assert(alignof(ShaderParameterBlockData) == 16);
static_assert(sizeof(ShaderParameterBlockData) == 384);

export using FrameParameterBlock = ShaderParameterBlockData;
export using ViewParameterBlock = ShaderParameterBlockData;
export using MaterialParameterData = ShaderParameterBlockData;
export using DrawParameterBlock = ShaderParameterBlockData;

export struct alignas(16) ShaderFloat4 final
{
	std::array<float, 4> values{};
};

export struct alignas(16) ShaderMatrix4x4 final
{
	std::array<float, 16> values{};
};

static_assert(sizeof(ShaderFloat4) == 16);
static_assert(sizeof(ShaderMatrix4x4) == 64);

constexpr std::size_t Value_Size(ShaderValueType type) noexcept
{
	switch (type) {
	case ShaderValueType::UInt32:
	case ShaderValueType::Float32:
		return 4;
	case ShaderValueType::Float2:
		return 8;
	case ShaderValueType::Float3:
		return 12;
	case ShaderValueType::Float4:
		return 16;
	case ShaderValueType::Matrix4x4:
		return 64;
	}

	return 0;
}

constexpr std::size_t Align_16(std::size_t value) noexcept
{
	return (value + 15u) & ~std::size_t(15u);
}

constexpr std::size_t Constant_Offset(std::size_t current, ShaderValueType type) noexcept
{
	const std::size_t size = Value_Size(type);
	if (type == ShaderValueType::Matrix4x4 || current % 16u + size > 16u)
		return Align_16(current);

	return current;
}

constexpr ShaderLayoutKey Hash_Value(ShaderLayoutKey hash, std::uint64_t value) noexcept
{
	hash ^= value;
	hash *= 1099511628211ull;
	return hash;
}

export class ShaderParameterLayout final
{
public:
	static constexpr std::size_t Max_Parameter_Count = 32;

	constexpr explicit ShaderParameterLayout(ShaderParameterBlock block = ShaderParameterBlock::Frame) noexcept
		: m_block(block)
	{
	}

	constexpr ShaderParameterBlock Block() const noexcept
	{
		return m_block;
	}

	constexpr std::size_t Size() const noexcept
	{
		return m_count;
	}

	constexpr std::size_t Constant_Byte_Size() const noexcept
	{
		return Align_16(m_constant_size);
	}

	constexpr std::size_t Resource_Count() const noexcept
	{
		return m_resource_count;
	}

	std::span<const ShaderParameterLocation> Parameters() const noexcept
	{
		return {m_parameters.data(), m_count};
	}

	ShaderParameterLocation Add_Constant(ShaderValueType type) noexcept
	{
		if (m_count >= Max_Parameter_Count)
			return {};

		const std::size_t size = Value_Size(type);
		const std::size_t offset = Constant_Offset(m_constant_size, type);
		if (size == 0 || offset + size > ShaderParameterBlockData::Max_Constant_Bytes || offset > Invalid_Shader_Parameter_Offset)
			return {};

		ShaderParameterLocation location;
		location.block = m_block;
		location.kind = ShaderParameterKind::Constant;
		location.value_type = type;
		location.parameter_index = m_count;
		location.constant_offset = static_cast<std::uint16_t>(offset);
		location.constant_size = static_cast<std::uint16_t>(size);
		m_parameters[m_count++] = location;
		m_constant_size = offset + size;
		return location;
	}

	ShaderParameterLocation Add_Buffer() noexcept
	{
		return Add_Resource(ShaderParameterKind::Buffer, ShaderResourceClass::Buffer);
	}

	ShaderParameterLocation Add_Bindless_Resource(ShaderResourceClass resource_class) noexcept
	{
		return Add_Resource(ShaderParameterKind::BindlessResource, resource_class);
	}

	bool Is_Compatible(ShaderParameterLocation location) const noexcept
	{
		return location.Is_Valid()
			&& location.parameter_index < m_count
			&& m_parameters[location.parameter_index] == location;
	}

	bool Is_Compatible(const ShaderParameterLayout &other) const noexcept
	{
		if (m_block != other.m_block || m_count != other.m_count || m_constant_size != other.m_constant_size || m_resource_count != other.m_resource_count)
			return false;

		for (std::size_t index = 0; index < m_count; ++index) {
			if (m_parameters[index] != other.m_parameters[index])
				return false;
		}

		return true;
	}

	constexpr ShaderLayoutKey Key() const noexcept
	{
		ShaderLayoutKey key = 1469598103934665603ull;
		key = Hash_Value(key, static_cast<std::uint8_t>(m_block));
		key = Hash_Value(key, m_count);
		key = Hash_Value(key, m_constant_size);
		key = Hash_Value(key, m_resource_count);
		for (std::size_t index = 0; index < m_count; ++index) {
			const ShaderParameterLocation &location = m_parameters[index];
			key = Hash_Value(key, static_cast<std::uint8_t>(location.kind));
			key = Hash_Value(key, static_cast<std::uint8_t>(location.value_type));
			key = Hash_Value(key, static_cast<std::uint8_t>(location.resource_class));
			key = Hash_Value(key, location.parameter_index);
			key = Hash_Value(key, location.constant_offset);
			key = Hash_Value(key, location.constant_size);
			key = Hash_Value(key, location.resource_offset);
		}
		return key;
	}

private:
	ShaderParameterLocation Add_Resource(ShaderParameterKind kind, ShaderResourceClass resource_class) noexcept
	{
		if (m_count >= Max_Parameter_Count || m_resource_count >= ShaderParameterBlockData::Max_Resource_Indices)
			return {};

		ShaderParameterLocation location;
		location.block = m_block;
		location.kind = kind;
		location.resource_class = resource_class;
		location.parameter_index = m_count;
		location.resource_offset = static_cast<std::uint16_t>(m_resource_count);
		m_parameters[m_count++] = location;
		++m_resource_count;
		return location;
	}

	ShaderParameterBlock m_block;
	std::array<ShaderParameterLocation, Max_Parameter_Count> m_parameters{};
	std::uint16_t m_count = 0;
	std::uint16_t m_constant_size = 0;
	std::uint16_t m_resource_count = 0;
};

export struct ShaderInterfaceLayout final
{
	ShaderParameterLayout frame{ShaderParameterBlock::Frame};
	ShaderParameterLayout view{ShaderParameterBlock::View};
	ShaderParameterLayout material{ShaderParameterBlock::Material};
	ShaderParameterLayout draw{ShaderParameterBlock::Draw};

	constexpr ShaderLayoutKey Key() const noexcept
	{
		ShaderLayoutKey key = 1469598103934665603ull;
		key = Hash_Value(key, frame.Key());
		key = Hash_Value(key, view.Key());
		key = Hash_Value(key, material.Key());
		key = Hash_Value(key, draw.Key());
		return key;
	}

	bool Is_Compatible(const ShaderInterfaceLayout &other) const noexcept
	{
		return frame.Is_Compatible(other.frame)
			&& view.Is_Compatible(other.view)
			&& material.Is_Compatible(other.material)
			&& draw.Is_Compatible(other.draw);
	}
};

}

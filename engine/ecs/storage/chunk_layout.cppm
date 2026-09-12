module;

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

export module engine.ecs.storage.chunk_layout;

export import engine.ecs.core.component_registry;

import engine.ecs.core.entity;

export namespace ecs
{

class ChunkLayout
{
public:
	static constexpr std::size_t DefaultTargetBytes = 16 * 1024;

	struct Column
	{
		ComponentId component{InvalidComponentId};
		std::size_t offset{0};
		std::size_t size{0};
		std::size_t alignment{1};
	};

	static ChunkLayout Build(const std::vector<const ComponentInfo *> &components,
		std::size_t targetBytes = DefaultTargetBytes);

	std::size_t Capacity() const noexcept { return m_capacity; }
	std::size_t TotalBytes() const noexcept { return m_totalBytes; }
	std::size_t Alignment() const noexcept { return m_alignment; }
	std::size_t EntityOffset() const noexcept { return m_entityOffset; }
	const std::vector<Column> &Columns() const noexcept { return m_columns; }

	const Column *Find(ComponentId component) const noexcept;

private:
	ChunkLayout(std::vector<Column> columns,
		std::size_t capacity,
		std::size_t totalBytes,
		std::size_t alignment,
		std::size_t entityOffset) noexcept;

	std::vector<Column> m_columns;
	std::size_t m_capacity{0};
	std::size_t m_totalBytes{0};
	std::size_t m_alignment{alignof(std::max_align_t)};
	std::size_t m_entityOffset{0};
};

} // namespace ecs

namespace ecs
{
namespace
{

std::size_t AlignUp(std::size_t value, std::size_t alignment)
{
	const std::size_t remainder = value % alignment;
	if (remainder == 0)
		return value;

	const std::size_t adjustment = alignment - remainder;
	if (value > std::numeric_limits<std::size_t>::max() - adjustment)
		throw std::length_error("ECS chunk layout size overflow");
	return value + adjustment;
}

struct Candidate
{
	std::vector<ChunkLayout::Column> columns;
	std::size_t totalBytes{0};
	std::size_t alignment{alignof(std::max_align_t)};
};

Candidate MakeCandidate(const std::vector<const ComponentInfo *> &components, std::size_t capacity)
{
	Candidate candidate;
	std::size_t offset = 0;
	if (capacity > std::numeric_limits<std::size_t>::max() / sizeof(Entity))
		throw std::length_error("ECS entity column size overflow");
	const std::size_t entityBytes = sizeof(Entity) * capacity;
	offset = AlignUp(offset, alignof(Entity));
	if (offset > std::numeric_limits<std::size_t>::max() - entityBytes)
		throw std::length_error("ECS entity column size overflow");
	offset += entityBytes;

	candidate.columns.reserve(components.size());
	for (const ComponentInfo *info : components)
	{
		if (info == nullptr || info->alignment == 0 || (info->alignment & (info->alignment - 1)) != 0)
			throw std::invalid_argument("ECS component alignment must be a power of two");

		candidate.alignment = std::max(candidate.alignment, info->alignment);
		offset = AlignUp(offset, info->alignment);
		if (info->size != 0 && capacity > std::numeric_limits<std::size_t>::max() / info->size)
			throw std::length_error("ECS component column size overflow");
		const std::size_t bytes = info->size * capacity;

		candidate.columns.push_back(ChunkLayout::Column{info->id, offset, info->size, info->alignment});
		if (offset > std::numeric_limits<std::size_t>::max() - bytes)
			throw std::length_error("ECS chunk layout size overflow");
		offset += bytes;
	}

	candidate.totalBytes = offset;
	return candidate;
}

} // namespace

ChunkLayout::ChunkLayout(std::vector<Column> columns,
	std::size_t capacity,
	std::size_t totalBytes,
	std::size_t alignment,
	std::size_t entityOffset) noexcept :
	m_columns(std::move(columns)),
	m_capacity(capacity),
	m_totalBytes(totalBytes),
	m_alignment(alignment),
	m_entityOffset(entityOffset)
{
}

ChunkLayout ChunkLayout::Build(const std::vector<const ComponentInfo *> &components, std::size_t targetBytes)
{
	if (targetBytes == 0)
		targetBytes = DefaultTargetBytes;

	std::size_t bytesPerEntity = sizeof(Entity);
	for (const ComponentInfo *info : components)
	{
		if (info == nullptr)
			throw std::invalid_argument("ECS chunk layout received a null component");
		if (info->size > std::numeric_limits<std::size_t>::max() - bytesPerEntity)
			throw std::length_error("ECS chunk layout size overflow");
		bytesPerEntity += info->size;
	}

	std::size_t capacity = std::max<std::size_t>(1, targetBytes / std::max<std::size_t>(1, bytesPerEntity));
	for (;;)
	{
		Candidate candidate = MakeCandidate(components, capacity);
		if (candidate.totalBytes <= targetBytes || capacity == 1)
		{
			return ChunkLayout(std::move(candidate.columns),
				capacity,
				candidate.totalBytes,
				candidate.alignment,
				0);
		}
		--capacity;
	}
}

const ChunkLayout::Column *ChunkLayout::Find(ComponentId component) const noexcept
{
	for (const Column &column : m_columns)
	{
		if (column.component == component)
			return &column;
	}
	return nullptr;
}

} // namespace ecs

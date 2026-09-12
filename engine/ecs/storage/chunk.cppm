module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>

export module engine.ecs.storage.chunk;

export import engine.ecs.core.entity;
export import engine.ecs.core.component_registry;
export import engine.ecs.storage.chunk_layout;

export namespace ecs
{

class Chunk
{
public:
	Chunk(ChunkLayout layout, std::vector<const ComponentInfo *> components);
	~Chunk();

	Chunk(const Chunk &) = delete;
	Chunk &operator=(const Chunk &) = delete;

	std::size_t Size() const noexcept { return m_size; }
	std::size_t Capacity() const noexcept { return m_layout.Capacity(); }
	bool IsFull() const noexcept { return m_size == Capacity(); }

	Entity *Entities() noexcept;
	const Entity *Entities() const noexcept;

	void *ComponentData(std::size_t column) noexcept;
	const void *ComponentData(std::size_t column) const noexcept;

	const ChunkLayout &Layout() const noexcept { return m_layout; }

	// Reservation keeps the row outside the published [0, Size()) range while
	// its component columns are constructed by the owning archetype.
	std::size_t ReserveRow() noexcept;
	void PublishRow(Entity entity, std::size_t row) noexcept;
	void CancelRow(std::size_t row) noexcept;

	void AddEntity(Entity entity);
	Entity RemoveSwap(std::size_t row);

private:
	static constexpr std::size_t InvalidRow = (std::numeric_limits<std::size_t>::max)();

	std::byte *DataAt(std::size_t offset) noexcept;
	const std::byte *DataAt(std::size_t offset) const noexcept;

	ChunkLayout m_layout;
	std::vector<const ComponentInfo *> m_components;
	std::byte *m_data{nullptr};
	std::size_t m_size{0};
	std::size_t m_reservedRow{InvalidRow};
};

} // namespace ecs

namespace ecs
{

Chunk::Chunk(ChunkLayout layout, std::vector<const ComponentInfo *> components) :
	m_layout(std::move(layout)),
	m_components(std::move(components))
{
	m_data = static_cast<std::byte *>(::operator new(m_layout.TotalBytes(), std::align_val_t(m_layout.Alignment())));
}

Chunk::~Chunk()
{
	assert(m_reservedRow == InvalidRow);
	for (std::size_t row = 0; row < m_size; ++row)
	{
		for (const ComponentInfo *info : m_components)
		{
			const ChunkLayout::Column *column = m_layout.Find(info->id);
			info->destroy(DataAt(column->offset + row * column->size));
		}
	}

	::operator delete(m_data, std::align_val_t(m_layout.Alignment()));
}

std::size_t Chunk::ReserveRow() noexcept
{
	assert(!IsFull());
	assert(m_reservedRow == InvalidRow);
	m_reservedRow = m_size;
	return m_reservedRow;
}

void Chunk::PublishRow(const Entity entity, const std::size_t row) noexcept
{
	assert(m_reservedRow == row);
	assert(row == m_size);
	Entities()[row] = entity;
	++m_size;
	m_reservedRow = InvalidRow;
}

void Chunk::CancelRow(const std::size_t row) noexcept
{
	assert(m_reservedRow == row);
	m_reservedRow = InvalidRow;
}

std::byte *Chunk::DataAt(std::size_t offset) noexcept
{
	return m_data + offset;
}

const std::byte *Chunk::DataAt(std::size_t offset) const noexcept
{
	return m_data + offset;
}

Entity *Chunk::Entities() noexcept
{
	return reinterpret_cast<Entity *>(DataAt(m_layout.EntityOffset()));
}

const Entity *Chunk::Entities() const noexcept
{
	return reinterpret_cast<const Entity *>(DataAt(m_layout.EntityOffset()));
}

void *Chunk::ComponentData(std::size_t column) noexcept
{
	return DataAt(m_layout.Columns()[column].offset);
}

const void *Chunk::ComponentData(std::size_t column) const noexcept
{
	return DataAt(m_layout.Columns()[column].offset);
}

void Chunk::AddEntity(Entity entity)
{
	const std::size_t row = ReserveRow();
	std::size_t constructed = 0;
	try
	{
		for (const ComponentInfo *info : m_components)
		{
			if (info->constructDefault == nullptr)
				throw std::logic_error("ECS component does not support default construction");
			const ChunkLayout::Column *column = m_layout.Find(info->id);
			info->constructDefault(DataAt(column->offset + row * column->size));
			++constructed;
		}
	}
	catch (...)
	{
		for (std::size_t i = 0; i < constructed; ++i)
		{
			const ComponentInfo *info = m_components[i];
			const ChunkLayout::Column *column = m_layout.Find(info->id);
			info->destroy(DataAt(column->offset + row * column->size));
		}
		CancelRow(row);
		throw;
	}

	PublishRow(entity, row);
}

Entity Chunk::RemoveSwap(std::size_t row)
{
	assert(m_reservedRow == InvalidRow);
	assert(row < m_size);
	const std::size_t last = m_size - 1;
	if (row == last)
	{
		for (const ComponentInfo *info : m_components)
		{
			const ChunkLayout::Column *column = m_layout.Find(info->id);
			info->destroy(DataAt(column->offset + last * column->size));
		}
		--m_size;
		return Entity::Null();
	}

	const Entity moved = Entities()[last];
	Entities()[row] = moved;
	for (const ComponentInfo *info : m_components)
	{
		const ChunkLayout::Column *column = m_layout.Find(info->id);
		void *destination = DataAt(column->offset + row * column->size);
		void *source = DataAt(column->offset + last * column->size);
		info->destroy(destination);
		info->constructMove(destination, source);
		info->destroy(source);
	}
	--m_size;
	return moved;
}

} // namespace ecs

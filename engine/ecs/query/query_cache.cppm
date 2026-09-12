module;

#include <cstdint>
#include <utility>
#include <vector>

export module engine.ecs.query.query_cache;

export import engine.ecs.core.world;

export namespace ecs
{

class QueryCache
{
public:
	template<typename Predicate>
	bool Refresh(World &world, Predicate &&predicate)
	{
		const std::uint64_t revision = world.ArchetypeRevision();
		if (m_initialized && m_revision == revision)
			return false;

		m_matches.clear();
		for (Archetype *archetype : world.GetArchetypes())
		{
			if (predicate(*archetype))
				m_matches.push_back(archetype);
		}
		m_revision = revision;
		m_initialized = true;
		return true;
	}

	const std::vector<Archetype *> &Matches() const noexcept { return m_matches; }
	std::uint64_t Revision() const noexcept { return m_revision; }

private:
	std::vector<Archetype *> m_matches;
	std::uint64_t m_revision{0};
	bool m_initialized{false};
};

} // namespace ecs

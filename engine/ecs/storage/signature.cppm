module;

#include <algorithm>
#include <vector>

export module engine.ecs.storage.signature;

export import engine.ecs.core.component_registry;

export namespace ecs
{

using Signature = std::vector<ComponentId>;

inline void CanonicalizeSignature(Signature &signature)
{
	std::sort(signature.begin(), signature.end());
	signature.erase(std::unique(signature.begin(), signature.end()), signature.end());
}

struct SignatureLess
{
	bool operator()(const Signature &left, const Signature &right) const noexcept
	{
		return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
	}
};

} // namespace ecs

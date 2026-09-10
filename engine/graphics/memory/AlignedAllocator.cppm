module;

#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>

export module Graphics.Memory.AlignedAllocator;

namespace Graphics
{

export template <typename Type, std::size_t Alignment>
class AlignedAllocator
{
public:
	using value_type = Type;
	using size_type = std::size_t;

	static_assert(Alignment >= alignof(Type));
	static_assert((Alignment & (Alignment - 1)) == 0);

	AlignedAllocator() noexcept = default;

	template <typename OtherType>
	AlignedAllocator(const AlignedAllocator<OtherType, Alignment> &) noexcept
	{
	}

	Type *allocate(size_type count)
	{
		if (count > std::numeric_limits<size_type>::max() / sizeof(Type))
			throw std::bad_array_new_length();

		return static_cast<Type *>(::operator new(count * sizeof(Type), std::align_val_t(Alignment)));
	}

	void deallocate(Type *pointer, size_type) noexcept
	{
		::operator delete(pointer, std::align_val_t(Alignment));
	}

	template <typename OtherType>
	struct rebind
	{
		using other = AlignedAllocator<OtherType, Alignment>;
	};
};

export template <typename LeftType, typename RightType, std::size_t Alignment>
constexpr bool operator==(const AlignedAllocator<LeftType, Alignment> &, const AlignedAllocator<RightType, Alignment> &) noexcept
{
	return true;
}

export template <typename LeftType, typename RightType, std::size_t Alignment>
constexpr bool operator!=(const AlignedAllocator<LeftType, Alignment> &, const AlignedAllocator<RightType, Alignment> &) noexcept
{
	return false;
}

export template <typename Type, std::size_t Alignment = 16>
using AlignedVector = std::vector<Type, AlignedAllocator<Type, Alignment>>;

}

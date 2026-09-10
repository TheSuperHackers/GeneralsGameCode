module;

#define BOOST_TEST_MODULE W3DNullTests

#include <array>
#include <boost/test/included/unit_test.hpp>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

export module Assets.Tests.Adapters.W3D.Null;

import Assets.Adapters.W3D.Null;

namespace
{

using Byte = std::byte;
// Keep fixture layout independent from the implementation constants. These
// literals describe the on-disk W3dNullObjectStruct contract.
constexpr std::size_t NullPayloadSize = 48;
constexpr std::size_t NullNameOffset = 16;
constexpr std::size_t NullNameSize = 32;
using Bytes = std::array<Byte, NullPayloadSize>;

void Store_U32(Bytes &bytes, std::size_t offset, std::uint32_t value)
{
	BOOST_REQUIRE(offset <= bytes.size() && bytes.size() - offset >= sizeof(value));
	bytes[offset] = static_cast<Byte>(value & 0xffu);
	bytes[offset + 1] = static_cast<Byte>((value >> 8) & 0xffu);
	bytes[offset + 2] = static_cast<Byte>((value >> 16) & 0xffu);
	bytes[offset + 3] = static_cast<Byte>((value >> 24) & 0xffu);
}

Bytes Null_Bytes()
{
	Bytes bytes{};
	Store_U32(bytes, 0, 0x00010000u);
	Store_U32(bytes, 4, 0xA5A5F00Du);
	Store_U32(bytes, 8, 0x11111111u);
	Store_U32(bytes, 12, 0x22222222u);
	const std::string name = "container.null";
	for (std::size_t index = 0; index < name.size(); ++index)
		bytes[NullNameOffset + index] =
			static_cast<Byte>(static_cast<unsigned char>(name[index]));
	return bytes;
}

}

BOOST_AUTO_TEST_CASE(null_decoder_reads_explicit_wire_offsets_and_accepts_trailing_payload)
{
	using namespace Assets::W3D;
	BOOST_CHECK_EQUAL(W3DNullPayloadSize, NullPayloadSize);
	BOOST_CHECK_EQUAL(W3DNullNameOffset, NullNameOffset);
	BOOST_CHECK_EQUAL(W3DNullNameSize, NullNameSize);
	const Bytes source = Null_Bytes();
	std::vector<Byte> bytes(source.begin(), source.end());
	bytes.push_back(Byte{0x7f});
	bytes.push_back(Byte{0x01});

	W3DNullDescription result;
	std::string error;
	BOOST_REQUIRE_MESSAGE(W3DRead_Null(bytes, result, error), error);
	BOOST_CHECK(error.empty());
	BOOST_CHECK_EQUAL(result.version, 0x00010000u);
	BOOST_CHECK_EQUAL(result.attributes, 0xA5A5F00Du);
	BOOST_CHECK_EQUAL(result.name, "container.null");
}

BOOST_AUTO_TEST_CASE(null_decoder_is_atomic_for_every_truncated_prefix)
{
	using namespace Assets::W3D;
	const Bytes bytes = Null_Bytes();
	for (std::size_t size = 0; size < bytes.size(); ++size) {
		W3DNullDescription result;
		result.version = 0xDEADBEEFu;
		result.attributes = 0xCAFEBABEu;
		result.name = "unchanged";
		std::string error = "stale";
		BOOST_CHECK(!W3DRead_Null(std::span<const Byte>(bytes).first(size), result, error));
		BOOST_CHECK_EQUAL(result.version, 0xDEADBEEFu);
		BOOST_CHECK_EQUAL(result.attributes, 0xCAFEBABEu);
		BOOST_CHECK_EQUAL(result.name, "unchanged");
		BOOST_CHECK(!error.empty());
	}
}

BOOST_AUTO_TEST_CASE(null_decoder_rejects_an_unterminated_fixed_name_atomically)
{
	using namespace Assets::W3D;
	Bytes bytes = Null_Bytes();
	for (std::size_t index = 0; index < NullNameSize; ++index)
		bytes[NullNameOffset + index] = Byte{0x4e};

	W3DNullDescription result;
	result.version = 7;
	result.attributes = 9;
	result.name = "unchanged";
	std::string error;
	BOOST_CHECK(!W3DRead_Null(bytes, result, error));
	BOOST_CHECK_EQUAL(result.version, 7u);
	BOOST_CHECK_EQUAL(result.attributes, 9u);
	BOOST_CHECK_EQUAL(result.name, "unchanged");
	BOOST_CHECK(!error.empty());
}

BOOST_AUTO_TEST_CASE(null_decoder_accepts_empty_fixed_name)
{
	using namespace Assets::W3D;
	Bytes bytes{};
	Store_U32(bytes, 0, 1);
	Store_U32(bytes, 4, 2);

	W3DNullDescription result;
	std::string error;
	BOOST_REQUIRE_MESSAGE(W3DRead_Null(bytes, result, error), error);
	BOOST_CHECK(result.name.empty());
}

BOOST_AUTO_TEST_CASE(null_decoder_accepts_the_longest_terminated_fixed_name)
{
	using namespace Assets::W3D;
	Bytes bytes{};
	Store_U32(bytes, 0, 1);
	Store_U32(bytes, 4, 2);
	for (std::size_t index = 0; index < NullNameSize - 1; ++index)
		bytes[NullNameOffset + index] = Byte{'X'};

	W3DNullDescription result;
	std::string error;
	BOOST_REQUIRE_MESSAGE(W3DRead_Null(bytes, result, error), error);
	BOOST_CHECK_EQUAL(result.name.size(), NullNameSize - 1);
	BOOST_CHECK_EQUAL(result.name, std::string(NullNameSize - 1, 'X'));
}

#pragma once
#include <cstdint>

namespace rb
{

	static constexpr uint32_t CECrc32(const char* data, size_t len);
	static __forceinline uint32_t Crc32(const char* data, size_t len);

} // namespace rb

#include "crc32.inl"

#define COMPILE_TIME(value) ((decltype(value))CompileTime<decltype(value), value>::ValueHolder::VALUE)

template<typename T, T Value>
struct CompileTime
{
	enum class ValueHolder : T
	{
		VALUE = Value
	};
};

#define hash(string)          (COMPILE_TIME(rb::CECrc32((char*)string, sizeof(string)-sizeof(*string))))
#define rt_hash(string, size) Crc32(string, size)
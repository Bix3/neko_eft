#pragma once

template <unsigned int x> struct compile_time {
	enum : unsigned int {
		value = x
	};
};
#define _(str) compile_time<OMG::crypto::hash(str)>::value

namespace OMG::crypto
{
	template <typename t>
	constexpr unsigned int hash(const t string)
	{
		auto hash = 0xffffffff;
		auto pointer = string;
		while (*pointer != '\0')
		{
			hash ^= *pointer++;
			hash ^= 0x4447bbee;
			hash = (hash >> 1);
			hash ^= 0x092cd4af;
			hash += (hash << 5);
		}
		return hash;
	}
}
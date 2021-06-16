#ifndef ENTITY
#define ENTITY
#pragma once
#include "../../kiface/kiface.hpp"
#include "../../utils/xorstr.hpp"

class entity {
private:

	uintptr_t matrix_list_base;
	uintptr_t dependency_index_table_base;


	struct TransformAccessReadOnly {
		uint64_t pTransformData;
		int index;
	};

	struct TransformData {
		uint64_t pTransformArray;
		uint64_t pTransformIndices;
	};

	struct Matrix34 {
		Vector4 vec0;
		Vector4 vec1;
		Vector4 vec2;
	};
public:
	struct unity_string
	{
		char buffer[256];
	};

	class game_object { // maybe outdated
	public:
		template<typename T>
		T get_class()
		{
			return OMG::KERNAL::read<T>((uintptr_t)this + 0x30);
		}

		template<typename T>
		T get_class(uint32_t second_offset)
		{
			const auto object = OMG::KERNAL::read<uintptr_t>((uintptr_t)this + 0x30);
			if (!object)
				return {};

			return OMG::KERNAL::read<T>(object + second_offset);
		}

		int   get_layer() { return OMG::KERNAL::read<int>((uintptr_t)this + 0x50); }
		std::string get_prefab_name() { return OMG::KERNAL::read<unity_string>((uintptr_t)this + 0x60).buffer; }
	};

	std::string read_unicode(uintptr_t addr) {
		try {
			uintptr_t pstring = addr + 0x14;
			int32_t nameLength = OMG::KERNAL::read<int32_t>(addr + 0x10);

			std::shared_ptr<char[]> buff(new char[nameLength]);
			for (int i = 0; i < nameLength; ++i) {
				buff[i] = OMG::KERNAL::read<UCHAR>(pstring + (i * 2));// x2 because its unicode.
			}
			return std::string(&buff[0], &buff[nameLength]);
		}
		catch (const std::exception& exc) {}
		return xorget("Error");
	}

	Vector3 location;
	uintptr_t transform, entity;

	Vector3 GetBonePosition()
	{
		__m128 result;

		const __m128 mulVec0 = { -2.000, 2.000, -2.000, 0.000 };
		const __m128 mulVec1 = { 2.000, -2.000, -2.000, 0.000 };
		const __m128 mulVec2 = { -2.000, -2.000, 2.000, 0.000 };

		TransformAccessReadOnly pTransformAccessReadOnly = OMG::KERNAL::read<TransformAccessReadOnly>(transform + 0x38);
		unsigned int index = OMG::KERNAL::read<unsigned int>(transform + 0x40);
		TransformData transformData = OMG::KERNAL::read<TransformData>(pTransformAccessReadOnly.pTransformData + 0x18);

		if (transformData.pTransformArray && transformData.pTransformIndices)
		{
			result = OMG::KERNAL::read<__m128>(transformData.pTransformArray + (uint64_t)0x30 * index);
			int transformIndex = OMG::KERNAL::read<int>(transformData.pTransformIndices + (uint64_t)0x4 * index);
			int pSafe = 0;
			while (transformIndex >= 0 && pSafe++ < 200)
			{
				Matrix34 matrix34 = OMG::KERNAL::read<Matrix34>(transformData.pTransformArray + (uint64_t)0x30 * transformIndex);

				__m128 xxxx = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_load_si128((__m128i*) & matrix34.vec1), 0x00));	// xxxx
				__m128 yyyy = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_load_si128((__m128i*) & matrix34.vec1), 0x55));	// yyyy
				__m128 zwxy = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_load_si128((__m128i*) & matrix34.vec1), 0x8E));	// zwxy
				__m128 wzyw = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_load_si128((__m128i*) & matrix34.vec1), 0xDB));	// wzyw
				__m128 zzzz = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_load_si128((__m128i*) & matrix34.vec1), 0xAA));	// zzzz
				__m128 yxwy = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_load_si128((__m128i*) & matrix34.vec1), 0x71));	// yxwy
				__m128 tmp7 = _mm_mul_ps(*(__m128*)(&matrix34.vec2), result);

				result = _mm_add_ps(
					_mm_add_ps(
						_mm_add_ps(
							_mm_mul_ps(
								_mm_sub_ps(
									_mm_mul_ps(_mm_mul_ps(xxxx, mulVec1), zwxy),
									_mm_mul_ps(_mm_mul_ps(yyyy, mulVec2), wzyw)),
								_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0xAA))),
							_mm_mul_ps(
								_mm_sub_ps(
									_mm_mul_ps(_mm_mul_ps(zzzz, mulVec2), wzyw),
									_mm_mul_ps(_mm_mul_ps(xxxx, mulVec0), yxwy)),
								_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x55)))),
						_mm_add_ps(
							_mm_mul_ps(
								_mm_sub_ps(
									_mm_mul_ps(_mm_mul_ps(yyyy, mulVec0), yxwy),
									_mm_mul_ps(_mm_mul_ps(zzzz, mulVec1), zwxy)),
								_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x00))),
							tmp7)), _mm_load_ps(&matrix34.vec0.x));

				transformIndex = OMG::KERNAL::read<int>(transformData.pTransformIndices + 0x4 * transformIndex);
			}
		}

		return Vector3(result.m128_f32[0], result.m128_f32[1], result.m128_f32[2]);
	}

	Vector3 GetBonePosition(uintptr_t transform)
	{
		__m128 result;

		const __m128 mulVec0 = { -2.000, 2.000, -2.000, 0.000 };
		const __m128 mulVec1 = { 2.000, -2.000, -2.000, 0.000 };
		const __m128 mulVec2 = { -2.000, -2.000, 2.000, 0.000 };

		TransformAccessReadOnly pTransformAccessReadOnly = OMG::KERNAL::read<TransformAccessReadOnly>(transform + 0x38);
		unsigned int index = OMG::KERNAL::read<unsigned int>(transform + 0x40);
		TransformData transformData = OMG::KERNAL::read<TransformData>(pTransformAccessReadOnly.pTransformData + 0x18);

		if (transformData.pTransformArray && transformData.pTransformIndices)
		{
			result = OMG::KERNAL::read<__m128>(transformData.pTransformArray + (uint64_t)0x30 * index);
			int transformIndex = OMG::KERNAL::read<int>(transformData.pTransformIndices + (uint64_t)0x4 * index);
			int pSafe = 0;
			while (transformIndex >= 0 && pSafe++ < 200)
			{
				Matrix34 matrix34 = OMG::KERNAL::read<Matrix34>(transformData.pTransformArray + (uint64_t)0x30 * transformIndex);

				__m128 xxxx = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_load_si128((__m128i*) & matrix34.vec1), 0x00));	// xxxx
				__m128 yyyy = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_load_si128((__m128i*) & matrix34.vec1), 0x55));	// yyyy
				__m128 zwxy = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_load_si128((__m128i*) & matrix34.vec1), 0x8E));	// zwxy
				__m128 wzyw = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_load_si128((__m128i*) & matrix34.vec1), 0xDB));	// wzyw
				__m128 zzzz = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_load_si128((__m128i*) & matrix34.vec1), 0xAA));	// zzzz
				__m128 yxwy = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_load_si128((__m128i*) & matrix34.vec1), 0x71));	// yxwy
				__m128 tmp7 = _mm_mul_ps(*(__m128*)(&matrix34.vec2), result);

				result = _mm_add_ps(
					_mm_add_ps(
						_mm_add_ps(
							_mm_mul_ps(
								_mm_sub_ps(
									_mm_mul_ps(_mm_mul_ps(xxxx, mulVec1), zwxy),
									_mm_mul_ps(_mm_mul_ps(yyyy, mulVec2), wzyw)),
								_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0xAA))),
							_mm_mul_ps(
								_mm_sub_ps(
									_mm_mul_ps(_mm_mul_ps(zzzz, mulVec2), wzyw),
									_mm_mul_ps(_mm_mul_ps(xxxx, mulVec0), yxwy)),
								_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x55)))),
						_mm_add_ps(
							_mm_mul_ps(
								_mm_sub_ps(
									_mm_mul_ps(_mm_mul_ps(yyyy, mulVec0), yxwy),
									_mm_mul_ps(_mm_mul_ps(zzzz, mulVec1), zwxy)),
								_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x00))),
							tmp7)), _mm_load_ps(&matrix34.vec0.x));

				transformIndex = OMG::KERNAL::read<int>(transformData.pTransformIndices + 0x4 * transformIndex);
			}
		}

		return Vector3(result.m128_f32[0], result.m128_f32[1], result.m128_f32[2]);
	}
};
#endif
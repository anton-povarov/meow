////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2011 Anton Povarov <anton.povarov@gmail.com>
// Adaptation of Murmur3 hash function by Austin Appleby
//  Murmur3 hash is Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
// most of the code and comments here are from: http://smhasher.googlecode.com/svn/trunk/MurmurHash3.cpp
////////////////////////////////////////////////////////////////////////////////////////////////

// Note - The x86 and x64 versions do _not_ produce the same results, as the
// algorithms are optimized for their respective platforms. You can still
// compile and run any of them on any platform, but your performance with the
// non-native version will be less than optimal.
//
// this header defines the following functions:
//
// x86 versions
// uint32_t MurmurHash3_x86_32(k, len, seed) // has own implementation
// uint64_t MurmurHash3_x86_64(k, len, seed) // uses MurmurHash3_x86_128() and slices the result
// void MurmurHash3_x86_128(k, len, seed, void* out) // own implementation
//
// x86_64 versions
// uint32_t MurmurHash3_x64_32(k, len, seed) // slices MurmurHash3_x64_128()
// uint64_t MurmurHash3_x64_64(k, len, seed) // slices MurmurHash3_x64_128()
// void MurmurHash3_x64_128(k, len, seed, void* out) // own implementation
//

#ifndef MEOW_HASH__MURMUR3_HPP_
#define MEOW_HASH__MURMUR3_HPP_

#include <stdint.h>
#include <meow/config/compiler_features.hpp> // MEOW_FORCE_INLINE

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace hash_fn {
////////////////////////////////////////////////////////////////////////////////////////////////

	inline uint32_t rotl32 ( uint32_t x, int8_t r )
	{
	  return (x << r) | (x >> (32 - r));
	}

	inline uint64_t rotl64 ( uint64_t x, int8_t r )
	{
	  return (x << r) | (x >> (64 - r));
	}

	inline uint32_t rotr32 ( uint32_t x, int8_t r )
	{
	  return (x >> r) | (x << (32 - r));
	}

	inline uint64_t rotr64 ( uint64_t x, int8_t r )
	{
	  return (x >> r) | (x << (64 - r));
	}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)
#define	ROTR32(x,y)	rotr32(x,y)
#define ROTR64(x,y)	rotr64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

////////////////////////////////////////////////////////////////////////////////////////////////
// x86
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

	MEOW_FORCE_INLINE inline uint32_t getblock ( const uint32_t * p, int i )
	{
		return p[i];
	}

	//----------

	MEOW_FORCE_INLINE inline void bmix32 ( uint32_t & h1, uint32_t & k1, uint32_t & c1, uint32_t & c2 )
	{
		c1 = c1*5+0x7b7d159c;
		c2 = c2*5+0x6bce6396;

		k1 *= c1; 
		k1 = ROTL32(k1,11); 
		k1 *= c2;

		h1 = ROTL32(h1,13);
		h1 = h1*5+0x52dce729;
		h1 ^= k1;
	}

	inline uint32_t MurmurHash3_x86_32  ( const void * key, int len, uint32_t seed )
	{
		const uint8_t * data = (const uint8_t*)key;
		const int nblocks = len / 4;

		uint32_t h1 = 0x971e137b ^ seed;

		uint32_t c1 = 0x95543787;
		uint32_t c2 = 0x2ad7eb25;

		// body
		const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);
		for(int i = -nblocks; i; i++)
		{
			uint32_t k1 = getblock(blocks,i);
			bmix32(h1,k1,c1,c2);
		}

		// tail
		const uint8_t * tail = (const uint8_t*)(data + nblocks*4);
		uint32_t k1 = 0;

		switch(len & 3)
		{
			case 3: k1 ^= tail[2] << 16;
			case 2: k1 ^= tail[1] << 8;
			case 1: k1 ^= tail[0];
				bmix32(h1,k1,c1,c2);
		};

		// finalization

		h1 ^= len;

		h1 *= 0x85ebca6b;
		h1 ^= h1 >> 13;
		h1 *= 0xc2b2ae35;
		h1 ^= h1 >> 16;

		h1 ^= seed;

		// result
		return h1;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

// This mix is large enough that VC++ refuses to inline it unless we use
// __forceinline. It's also not all that fast due to register spillage.

	MEOW_FORCE_INLINE inline void bmix32 ( uint32_t & h1, uint32_t & h2,
							   uint32_t & h3, uint32_t & h4, 
							   uint32_t & k1, uint32_t & k2, 
							   uint32_t & k3, uint32_t & k4, 
							   uint32_t & c1, uint32_t & c2 )
	{
		k1 *= c1; 
		k1  = ROTL32(k1,11); 
		k1 *= c2;
		h1 ^= k1;
		h1 += h2;
		h1 += h3;
		h1 += h4;

		h1 = ROTL32(h1,17);

		k2 *= c2; 
		k2  = ROTL32(k2,11);
		k2 *= c1;
		h2 ^= k2;
		h2 += h1;

		h1 = h1*3+0x52dce729;
		h2 = h2*3+0x38495ab5;

		c1 = c1*5+0x7b7d159c;
		c2 = c2*5+0x6bce6396;

		k3 *= c1; 
		k3  = ROTL32(k3,11); 
		k3 *= c2;
		h3 ^= k3;
		h3 += h1;

		k4 *= c2; 
		k4  = ROTL32(k4,11);
		k4 *= c1;
		h4 ^= k4;
		h4 += h1;

		h3 = h3*3+0x52dce729;
		h4 = h4*3+0x38495ab5;

		c1 = c1*5+0x7b7d159c;
		c2 = c2*5+0x6bce6396;
	}

	// Finalization mix - force all bits of a hash block to avalanche
	// avalanches all bits to within 0.25% bias
	MEOW_FORCE_INLINE inline uint32_t fmix32 ( uint32_t h )
	{
		h ^= h >> 16;
		h *= 0x85ebca6b;
		h ^= h >> 13;
		h *= 0xc2b2ae35;
		h ^= h >> 16;
		return h;
	}

	inline void MurmurHash3_x86_128 ( const void * key, int len, uint32_t seed, void * out )
	{
		const uint8_t * data = (const uint8_t*)key;
		const int nblocks = len / 16;

		uint32_t h1 = 0x8de1c3ac ^ seed;
		uint32_t h2 = 0xbab98226 ^ seed;
		uint32_t h3 = 0xfcba5b2d ^ seed;
		uint32_t h4 = 0x32452e3e ^ seed;

		uint32_t c1 = 0x95543787;
		uint32_t c2 = 0x2ad7eb25;

		// body
		const uint32_t * blocks = (const uint32_t *)(data);

		for(int i = 0; i < nblocks; i++)
		{
			uint32_t k1 = getblock(blocks,i*4+0);
			uint32_t k2 = getblock(blocks,i*4+1);
			uint32_t k3 = getblock(blocks,i*4+2);
			uint32_t k4 = getblock(blocks,i*4+3);

			bmix32(h1,h2,h3,h4, k1,k2,k3,k4, c1,c2);
		}

		// tail
		const uint8_t * tail = (const uint8_t*)(data + nblocks*16);

		uint32_t k1 = 0;
		uint32_t k2 = 0;
		uint32_t k3 = 0;
		uint32_t k4 = 0;

		switch(len & 15)
		{
			case 15: k4 ^= tail[14] << 16;
			case 14: k4 ^= tail[13] << 8;
			case 13: k4 ^= tail[12] << 0;

			case 12: k3 ^= tail[11] << 24;
			case 11: k3 ^= tail[10] << 16;
			case 10: k3 ^= tail[ 9] << 8;
			case  9: k3 ^= tail[ 8] << 0;

			case  8: k2 ^= tail[ 7] << 24;
			case  7: k2 ^= tail[ 6] << 16;
			case  6: k2 ^= tail[ 5] << 8;
			case  5: k2 ^= tail[ 4] << 0;

			case  4: k1 ^= tail[ 3] << 24;
			case  3: k1 ^= tail[ 2] << 16;
			case  2: k1 ^= tail[ 1] << 8;
			case  1: k1 ^= tail[ 0] << 0;
				bmix32(h1,h2,h3,h4, k1,k2,k3,k4, c1,c2);
		};

		// finalization

		h4 ^= len;

		h1 += h2; h1 += h3; h1 += h4;
		h2 += h1; h3 += h1; h4 += h1;

		h1 = fmix32(h1);
		h2 = fmix32(h2);
		h3 = fmix32(h3);
		h4 = fmix32(h4);

		h1 += h2; h1 += h3; h1 += h4;
		h2 += h1; h3 += h1; h4 += h1;

		((uint32_t*)out)[0] = h1;
		((uint32_t*)out)[1] = h2;
		((uint32_t*)out)[2] = h3;
		((uint32_t*)out)[3] = h4;
	}

	inline uint64_t MurmurHash3_x86_64 ( const void * key, int len, uint32_t seed )
	{
		uint64_t out_128[2];
		MurmurHash3_x86_128(key, len, seed, &out_128);
		*out = out_128[0];
	}

////////////////////////////////////////////////////////////////////////////////////////////////
// x64

	// Block read - if your platform needs to do endian-swapping or can only
	// handle aligned reads, do the conversion here

	MEOW_FORCE_INLINE inline uint64_t getblock ( const uint64_t * p, int i )
	{
		return p[i];
	}

	// Block mix - combine the key bits with the hash bits and scramble everything
	MEOW_FORCE_INLINE void bmix64 ( uint64_t & h1, uint64_t & h2, 
									uint64_t & k1, uint64_t & k2, 
									uint64_t & c1, uint64_t & c2 )
	{
		k1 *= c1;
		k1  = ROTL64(k1,23); 
		k1 *= c2;

		k2 *= c2; 
		k2  = ROTL64(k2,23);
		k2 *= c1;

		h1 = ROTL64(h1,17);
		h1 += h2;
		h1 ^= k1;

		h2 = ROTL64(h2,41);
		h2 += h1;
		h2 ^= k2;

		h1 = h1*3+0x52dce729;
		h2 = h2*3+0x38495ab5;

		c1 = c1*5+0x7b7d159c;
		c2 = c2*5+0x6bce6396;
	}

	// Finalization mix - avalanches all bits to within 0.05% bias
	MEOW_FORCE_INLINE uint64_t fmix64 ( uint64_t k )
	{
		k ^= k >> 33;
		k *= BIG_CONSTANT(0xff51afd7ed558ccd);
		k ^= k >> 33;
		k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
		k ^= k >> 33;
		return k;
	}

	inline void MurmurHash3_x64_128 ( const void * key, int len, uint32_t seed, void * out )
	{
		const uint8_t * data = (const uint8_t*)key;
		const int nblocks = len / 16;

		uint64_t h1 = BIG_CONSTANT(0x9368e53c2f6af274) ^ seed;
		uint64_t h2 = BIG_CONSTANT(0x586dcd208f7cd3fd) ^ seed;

		uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
		uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);

		//----------
		// body

		const uint64_t * blocks = (const uint64_t *)(data);

		for(int i = 0; i < nblocks; i++)
		{
		uint64_t k1 = getblock(blocks,i*2+0);
		uint64_t k2 = getblock(blocks,i*2+1);

		bmix64(h1,h2,k1,k2,c1,c2);
		}

		//----------
		// tail

		const uint8_t * tail = (const uint8_t*)(data + nblocks*16);

		uint64_t k1 = 0;
		uint64_t k2 = 0;

		switch(len & 15)
		{
		case 15: k2 ^= uint64_t(tail[14]) << 48;
		case 14: k2 ^= uint64_t(tail[13]) << 40;
		case 13: k2 ^= uint64_t(tail[12]) << 32;
		case 12: k2 ^= uint64_t(tail[11]) << 24;
		case 11: k2 ^= uint64_t(tail[10]) << 16;
		case 10: k2 ^= uint64_t(tail[ 9]) << 8;
		case  9: k2 ^= uint64_t(tail[ 8]) << 0;

		case  8: k1 ^= uint64_t(tail[ 7]) << 56;
		case  7: k1 ^= uint64_t(tail[ 6]) << 48;
		case  6: k1 ^= uint64_t(tail[ 5]) << 40;
		case  5: k1 ^= uint64_t(tail[ 4]) << 32;
		case  4: k1 ^= uint64_t(tail[ 3]) << 24;
		case  3: k1 ^= uint64_t(tail[ 2]) << 16;
		case  2: k1 ^= uint64_t(tail[ 1]) << 8;
		case  1: k1 ^= uint64_t(tail[ 0]) << 0;
			   bmix64(h1,h2,k1,k2,c1,c2);
		};

		//----------
		// finalization

		h2 ^= len;

		h1 += h2;
		h2 += h1;

		h1 = fmix64(h1);
		h2 = fmix64(h2);

		h1 += h2;
		h2 += h1;

		((uint64_t*)out)[0] = h1;
		((uint64_t*)out)[1] = h2;
	}

	inline uint32_t MurmurHash3_x64_32  ( const void * key, int len, uint32_t seed)
	{
		uint64_t out_128[2];
		MurmurHash3_x64_128(key, len, seed, &out_128);
		return out_128[0];
	}

	inline uint64_t MurmurHash3_x64_64  ( const void * key, int len, uint32_t seed)
	{
		uint64_t out_128[2];
		MurmurHash3_x64_128(key, len, seed, &out_128);
		return out_128[0];
	}

////////////////////////////////////////////////////////////////////////////////////////////////

#undef ROTL32
#undef ROTL64
#undef ROTR32
#undef ROTR64
#undef BIG_CONSTANT

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow { namespace hash_fn {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_HASH__MURMUR3_HPP_


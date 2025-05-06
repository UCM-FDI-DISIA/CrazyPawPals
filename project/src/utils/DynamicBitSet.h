
#pragma once

#include <cassert>
#include <cstdint>
#include <ctime>
#include <iostream>

using namespace std;

class DynamicBitSet {
	using blockType = uint8_t;
	static const std::size_t _BITS_PER_BLOCK = (sizeof(blockType) * 8);
public:

	class BlockRef {
	public:
		BlockRef(BlockRef&& o) {
			block_ = o.block_;
			mask_ = o.mask_;
		}
		BlockRef(const BlockRef& o) {
			block_ = o.block_;
			mask_ = o.mask_;
		}

		BlockRef(blockType *block, blockType mask) {
			block_ = block;
			mask_ = mask;
		}

		BlockRef& operator=(const bool v) {
			if (v) {
				*block_ = *block_ | mask_;
			} else {
				*block_ = *block_ & ~mask_;
			}
			return *this;
		}

		operator bool() {
			return (*block_ & mask_) > 0;
		}


	private:
		blockType mask_;
		blockType *block_;
	};


	DynamicBitSet(std::size_t size) {
		_size = size;
		_bits = new blockType[_size / _BITS_PER_BLOCK + 1]();
	}

	virtual ~DynamicBitSet() {
		delete[] _bits;
	}

	bool isSet(std::size_t n) {
		assert(n < _size);
		std::size_t blockIdx = n / _BITS_PER_BLOCK;
		blockType mask = 1 << (n % _BITS_PER_BLOCK);
		return (_bits[blockIdx] & mask) > 0;
	}

	void set(std::size_t n, bool v) {
		assert(n < _size);
		std::size_t blockIdx = n / _BITS_PER_BLOCK;
		blockType mask = 1 << (n % _BITS_PER_BLOCK);
		if (v) {
			_bits[blockIdx] = _bits[blockIdx] | mask;
		} else {
			_bits[blockIdx] = _bits[blockIdx] & ~mask;
		}
	}

	BlockRef operator[](std::size_t n) {
		BlockRef x(&_bits[n / _BITS_PER_BLOCK], 1 << (n % _BITS_PER_BLOCK));
		return x;
	}

private:
	std::size_t _size;
	blockType *_bits;
};


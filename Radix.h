#pragma once

#include "queue.h"

#include <climits>

namespace radix {

	// Bit i of value, in an order that sorts signed integers correctly.
	//
	// Flipping the sign bit maps the signed range onto the unsigned range while
	// preserving order (INT_MIN -> 0, ... , INT_MAX -> UINT_MAX). Without this
	// the final pass puts every negative number after every positive one, so
	// the previous version only worked on non-negative input.
	inline int bit_at(int value, int i) {
		unsigned key = static_cast<unsigned>(value) ^ (1u << (sizeof(int) * CHAR_BIT - 1));
		return static_cast<int>((key >> i) & 1u);
	}

	// LSD radix sort, one bit per pass, using two of our own queues as buckets.
	inline void sort(int* data, int size) {
		if (data == nullptr || size < 2) { return; }

		Queue<int> buckets[2];
		const int bits = static_cast<int>(sizeof(int) * CHAR_BIT);

		for (int pass = 0; pass < bits; ++pass) {
			for (int i = 0; i < size; ++i) {
				buckets[bit_at(data[i], pass)].push(data[i]);
			}
			int out = 0;
			for (int b = 0; b < 2; ++b) {
				while (!buckets[b].is_empty()) {
					data[out++] = buckets[b].front();
					buckets[b].pop();
				}
			}
		}
	}
}

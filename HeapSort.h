#pragma once

#include "HeapTree.h"

// In-place heap sort, ascending. Builds a max-heap and fills the array from
// the back, which is why the result comes out in ascending order.
template <typename T>
void HeapSort(T* data, int size) {
	if (data == nullptr || size < 2) { return; }

	HeapTree<T> heap;
	for (int i = 0; i < size; ++i) {
		heap.insert(data[i]);
	}
	for (int i = size - 1; i >= 0; --i) {
		data[i] = heap.max_value();
		heap.remove();
	}
}
